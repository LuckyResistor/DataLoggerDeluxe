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
#include "ViewManager.h"


#include "AdjustTimeView.h"
#include "EraseAllView.h"
#include "KeyPad.h"
#include "MainMenuView.h"
#include "MemoryFullView.h"
#include "RecordView.h"
#include "SendRecordView.h"
#include "SetIntervalView.h"
#include "SharpDisplay.h"
#include "VersionInfoView.h"
#include "ViewRecordView.h"


namespace lr {
namespace ViewManager {
    
    
// Type definition for the used function pointers.
typedef void (*UpdateDisplayFn)();
typedef void (*HandleKeyFn)(KeyPad::Key key);
typedef void (*HandleLoopFn)();


static UpdateDisplayFn gUpdateDisplayFn; ///< A pointer to the current update function.
static HandleKeyFn gHandleKeyFn; ///< A pointer to the current key handling function.
static HandleLoopFn gHandleLoopFn; ///< A pointer to the current loop handling function.
static bool gNeedsDisplayUpdate; ///< Flag if a display update is required.
static ViewIdentifier gNextView; ///< Flag if a new view should be shown.


// forward declaration
void setView(ViewIdentifier view);

    
void begin()
{
    gUpdateDisplayFn = nullptr;
    gHandleKeyFn = nullptr;
    gHandleLoopFn = nullptr;
    gNeedsDisplayUpdate = true;
    gNextView = NoView;
    
    setView(MainMenuView);
    setNeedsDisplayUpdate();
}


void loop()
{
    if (gHandleLoopFn != nullptr) {
        gHandleLoopFn();
    }
    if (gNeedsDisplayUpdate) {
        gUpdateDisplayFn();
        gNeedsDisplayUpdate = false;
    }
    KeyPad::Key key = KeyPad::getNextKey();
    if (key != KeyPad::None) {
        if (gHandleKeyFn != nullptr) {
            gHandleKeyFn(key);
        }
    }
    if (gNextView != NoView) {
        setView(gNextView);
        gNextView = NoView;
    }
}


void displayError(String message)
{
    while (true) {
        SharpDisplay::clear();
        SharpDisplay::writeText(PSTR("Error:\n"));
        SharpDisplay::writeText(message);
        delay(1000);
        SharpDisplay::clear();
        delay(200);
    }
}

    
void updateMeasurementDisplay(const DHT22::Measurement &measurement, const DateTime &dateTime, char modeDisplay)
{
    // Display this data on the screen
    SharpDisplay::setTextInverse(false);
    SharpDisplay::setLineText(10, dateTime.toString(DateTime::FormatShortDate) + " " + dateTime.toString(DateTime::FormatShortTime));
    String htText = String(measurement.humidity, 1);
    htText += String(F("%"));
    htText += modeDisplay;
    htText += String(measurement.temperature, 1);
    htText += String(F("\x7f""C"));
    SharpDisplay::setLineText(11, htText);
    SharpDisplay::fillRow(9, 0x89);
}

    
void setNeedsDisplayUpdate()
{
    gNeedsDisplayUpdate = true;
}

    
void setNextView(ViewIdentifier nextView)
{
    gNextView = nextView;
}


void setView(ViewIdentifier view)
{
    gHandleKeyFn = nullptr;
    gHandleLoopFn = nullptr;
    switch (view) {
        case MainMenuView:
            gUpdateDisplayFn = &MainMenuView::updateDisplay;
            gHandleKeyFn = &MainMenuView::handleKey;
            MainMenuView::viewWillAppear();
            break;
            
        case RecordView:
            gUpdateDisplayFn = &RecordView::updateDisplay;
            gHandleKeyFn = &RecordView::handleKey;
            RecordView::viewWillAppear();
            break;
            
        case MemoryFullView:
            gUpdateDisplayFn = &MemoryFullView::updateDisplay;
            gHandleKeyFn = &MemoryFullView::handleKey;
            MemoryFullView::viewWillAppear();
            break;
            
        case ViewRecordView:
            gUpdateDisplayFn = &ViewRecordView::updateDisplay;
            gHandleKeyFn = &ViewRecordView::handleKey;
            ViewRecordView::viewWillAppear();
            break;
            
        case SendRecordView:
            gUpdateDisplayFn = &SendRecordView::updateDisplay;
            gHandleLoopFn = &SendRecordView::handleLoop;
            SendRecordView::viewWillAppear();
            break;
            
        case EraseAllView:
            gUpdateDisplayFn = &EraseAllView::updateDisplay;
            gHandleKeyFn = &EraseAllView::handleKey;
            EraseAllView::viewWillAppear();
            break;
            
        case SetIntervalView:
            gUpdateDisplayFn = &SetIntervalView::updateDisplay;
            gHandleKeyFn = &SetIntervalView::handleKey;
            SetIntervalView::viewWillAppear();
            break;
            
        case AdjustTimeView:
            gUpdateDisplayFn = &AdjustTimeView::updateDisplay;
            gHandleKeyFn = &AdjustTimeView::handleKey;
            AdjustTimeView::viewWillAppear();
            break;
            
        case VersionInfoView:
            gUpdateDisplayFn = &VersionInfoView::updateDisplay;
            gHandleKeyFn = &VersionInfoView::handleKey;
            break;
            
        default:
            break;
    }
    setNeedsDisplayUpdate();
}


}
}



