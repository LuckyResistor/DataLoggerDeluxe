//
// Lucky Resistor's Deluxe Data Logger
// ---------------------------------------------------------------------------
// (c)2015 by Lucky Resistor. See LICENSE for details.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "Application.h"


// Include the local classes
#include "config.h"
#include "DateTime.h"
#include "DHT22.h"
#include "DS3231.h"
#include "Fonts.h"
#include "KeyPad.h"
#include "LogSystem.h"
#include "Settings.h"
#include "SharpDisplay.h"
#include "Storage.h"
#include "ViewManager.h"

// Include Arduino libraries
#include <Wire.h>

// Include AVR libraries
#include <avr/sleep.h>


namespace lr {
namespace Application {


/// The initial logo displayed on the screen.
///
static const char cLogoText[] PROGMEM =
    " \x84\x85\x86\x89\x89\x89\x89\x86\x85\x84\n"
    "\x84\x85\x86 DATA \x86\x85\x84\n"
    "\x85\x86 LOGGER \x86\x85\n"
    "Version " APP_VERSION "\n"
    "\x84\x85\x86\x89\x89\x89\x89\x89\x89\x86\x85\x84\n";


static uint8_t gDisplayInfoRefreshCount; ///< A counter to delay the update of the info area.
static OperationMode gOperationMode; ///< The current operation mode of the application.
static DateTime gNextRecordTime; ///< The next time where a new record is created.
    
    
/// Wait a number of seconds in powersafe mode.
///
/// This function puts the microcontroller in power save mode and
/// waits a number of seconds. This only works, because the display
/// driver is using timer 2 as interrupt source. The interrupt for
/// the display wakes the microcontroller from the power save mode.
///
/// @param seconds The number of seconds to sleep.
///
void powerSave(uint16_t seconds)
{
    // Go to sleep (for 1/60s).
    SMCR = _BV(SM1)|_BV(SM0); // Power-save mode.
    const uint32_t waitIntervals = (seconds*61); // This is almost a second.
    for (uint32_t i = 0; i < waitIntervals; ++i) {
        TCNT2 = 0; // reset the timer.
        SMCR |= _BV(SE); // Enable sleep mode.
        sleep_cpu();
        SMCR &= ~_BV(SE); // Disable sleep mode.
        if (KeyPad::hasNextKey()) {
            break; // Stop waiting if there is a key press.
        }
    }
}


void setup()
{
    // Initialize the key pad.
    KeyPad::begin();

    // Initialize the display.
    SharpDisplay::begin(11, 9, 10);
    SharpDisplay::setFont(Fonts::getFontA());
    SharpDisplay::setInterruptCallback(&KeyPad::checkKeys);
    
    // Write some initial greeting.
    SharpDisplay::writeText(getLogoText());
    SharpDisplay::writeText(PSTR("\nStarting..."));

    // Initialize all libraries
    ViewManager::begin();
    Wire.begin();
    DHT22::begin(3);
    SharpDisplay::writeText(PSTR("\x9e\n"));

    // Initialize the log system.
    SharpDisplay::writeText(PSTR("Log Sys... "));
    if (!Storage::begin()) {
        ViewManager::displayError(F("Storage\nProblem"));
    }
    LogSystem::begin(Settings::size());
    SharpDisplay::writeText(PSTR("\x9e\n"));

    // Read all settings.
    Settings::begin();
    
    SharpDisplay::writeText(PSTR("RTC... "));
    DS3231::begin(2000); // Usage 2000-2199
    if (!DS3231::isRunning()) {
        ViewManager::displayError(F("RTC Problem"));
    }
    SharpDisplay::writeText(PSTR("\x9e\n"));
    delay(2000);
    SharpDisplay::clear();
    
    // Initialize the variables.
    gOperationMode = MenuMode;
    gDisplayInfoRefreshCount = 201;
}


void loop()
{
    // Always keep the last recorded values.
    static DHT22::Measurement measurement;
    static DateTime dateTime;
    
    if (gOperationMode == MenuMode) {
        // Manage the current view.
        ViewManager::loop();
        delay(50);
        
        if (++gDisplayInfoRefreshCount > 200) {
            gDisplayInfoRefreshCount = 0;
            // Read the time and sensor data.
            measurement = DHT22::readTemperatureAndHumidity();
            dateTime = DS3231::getDateTime();
            ViewManager::updateMeasurementDisplay(measurement, dateTime, ' ');
        }
    } else if (gOperationMode == FullScreenMode) {
        // Manage the current view.
        ViewManager::loop();
        delay(50);
    } else if (gOperationMode == RecordingMode) {
        // Power saving recording mode
        ViewManager::setNeedsDisplayUpdate(); // Update of the screen always required.
        ViewManager::loop();

        // Check if we shall store a new record.
        dateTime = DS3231::getDateTime();
        if (dateTime >= gNextRecordTime) {
            measurement = DHT22::readTemperatureAndHumidity();
            LogRecord logRecord(dateTime, measurement.temperature, measurement.humidity);
            if (!LogSystem::appendRecord(logRecord)) {
                ViewManager::setNextView(ViewManager::MemoryFullView);
                setOperationMode(Application::MenuMode);
            }
            gNextRecordTime = gNextRecordTime.addSeconds(Settings::getIntervalInSeconds());
        }
        ViewManager::updateMeasurementDisplay(measurement, dateTime, '\x85');
        const int32_t secondsToWait = min(dateTime.secondsTo(gNextRecordTime), Settings::getPowerSaveDuration());
        if (secondsToWait > 0) {
            powerSave(secondsToWait);
        } else {
            powerSave(1);
        }
    } else {
        // Display the menu, but save power.
        ViewManager::loop();
        
        dateTime = DS3231::getDateTime();
        measurement = DHT22::readTemperatureAndHumidity();
        ViewManager::updateMeasurementDisplay(measurement, dateTime, '\x84');
        powerSave(60); // Update in 1 minute intervals (except a key is pressed).
    }
}

    
void resetNextRecordTime()
{
    gNextRecordTime = DS3231::getDateTime().addSeconds(10);
}

    
void setOperationMode(OperationMode mode)
{
    gOperationMode = mode;
    gDisplayInfoRefreshCount = 201;
    switch (mode) {
        case MenuMode:
        case FullScreenMode:
            SharpDisplay::setRefreshInterval(SharpDisplay::NormalRefresh);
            break;
            
        case RecordingMode:
        case PowerSave:
            SharpDisplay::setRefreshInterval(SharpDisplay::SlowRefresh);
            break;
    }
}
    

const char* getLogoText()
{
    return cLogoText;
}
    

}
}











