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
#include "SendRecordView.h"


#include "Application.h"
#include "Settings.h"
#include "LogSystem.h"
#include "ViewManager.h"
#include "SharpDisplay.h"
#include "config.h"


namespace lr {
namespace SendRecordView {


// The state of this view.    
enum State : uint8_t {
    StateInitialize,
    StateWelcome,
    StateWrite,
    StateDone
};

    
static State gState = StateInitialize; // The state of the view.
static uint32_t gTime; // The time to calculate delays.
static uint32_t gSentRecord; // The last sent record index.
static uint32_t gSerialSpeed; // The speed of the serial port.
    
    
void viewWillAppear()
{
    gState = StateInitialize;
    gTime = millis();
    gSentRecord = 0;
    gSerialSpeed = Settings::getSerialSpeed();
}

    
void handleLoop()
{
    if (gState == StateInitialize) {
        Serial.begin(gSerialSpeed);
        gState = StateWelcome;
        ViewManager::setNeedsDisplayUpdate();
    } else if (gState == StateWelcome) {
        if ((millis() - gTime) > 2000) {
            Serial.println(F("Data Logger - Version " APP_VERSION "\n"));
            gState = StateWrite;
        }
    } else if (gState == StateWrite) {
        gTime = millis();
        while ((millis() - gTime) < 100) {
            if (gSentRecord >= LogSystem::currentNumberOfRecords()) {
                gState = StateDone;
                gTime = millis();
                break;
            }
            LogRecord logRecord = LogSystem::getLogRecord(gSentRecord);
            logRecord.writeToSerial();
            ++gSentRecord;
        }
        ViewManager::setNeedsDisplayUpdate();
    } else if (gState == StateDone) {
        if ((millis() - gTime) > 2000) {
            ViewManager::setNextView(ViewManager::MainMenuView);
        }
    }
}

    
void updateDisplay()
{
    SharpDisplay::clearRows(0, 9);
    if (gState == StateInitialize || gState == StateWelcome) {
        SharpDisplay::setLineText(3, PSTR("Sending Data"));
        SharpDisplay::setLineText(4, PSTR("to Serial at"));
        SharpDisplay::setLineText(5, String(gSerialSpeed, DEC) + String(F(" baud")));
    } else if (gState == StateWrite) {
        SharpDisplay::setLineText(3, PSTR("Send Record:"));
        const uint32_t numberOfRecords = LogSystem::currentNumberOfRecords();
        SharpDisplay::setLineText(4, String(gSentRecord, DEC) + '/' + String(numberOfRecords, DEC));
    } else if (gState == StateDone) {
        SharpDisplay::setLineText(4, PSTR("  Success!  "));
    }
}

    
}
}

