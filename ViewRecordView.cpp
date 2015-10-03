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
#include "ViewRecordView.h"


#include "Application.h"
#include "LogSystem.h"
#include "SharpDisplay.h"
#include "ViewManager.h"


namespace lr {
namespace ViewRecordView {


// The scroll speed.    
enum ScrollSpeed : uint8_t {
    Speed1,
    Speed10,
    Speed100
};


static uint32_t gTopRecord = 0; ///< The record at the top if the display.
static uint32_t gNumberOfRecords = 0; ///< The current number of records.
static uint8_t gCursorPosition = 0; ///< The position of the cursor.
static ScrollSpeed gScrollSpeed = Speed1; ///< The current scroll speed.


void viewWillAppear()
{
    Application::setOperationMode(Application::FullScreenMode);
    gNumberOfRecords = LogSystem::currentNumberOfRecords();
    gCursorPosition = 0;
    if ((gTopRecord+2) > gNumberOfRecords) {
        gTopRecord = 0;
    }
}

    
void updateDisplay()
{
    for (uint8_t i = 0; i < 3; ++i) {
        SharpDisplay::setTextInverse(i == gCursorPosition);
        const uint32_t index = gTopRecord + i;
        const uint8_t row = 3*i;
        if (index < gNumberOfRecords) {
            SharpDisplay::setLineText(row, String(index+1) + String(F(":")));
            LogRecord logRecord = LogSystem::getLogRecord(index);
            SharpDisplay::setLineText(row+1, logRecord.getDateTime().toString(DateTime::FormatShortDate) + " " + logRecord.getDateTime().toString(DateTime::FormatShortTime));
            String htText = String(logRecord.getHumidity(), 1);
            htText += String(F("% "));
            htText += String(logRecord.getTemperature(), 1);
            htText += String(F("\x7f""C"));
            SharpDisplay::setLineText(row+2, htText);
        } else {
            SharpDisplay::fillRow(row, ' ');
            SharpDisplay::fillRow(row+1, ' ');
            SharpDisplay::fillRow(row+2, ' ');
        }
    }
    SharpDisplay::setTextInverse(false);
    SharpDisplay::fillRow(9, '\x89');
    String recText = String(F("Record: "));
    switch (gScrollSpeed) {
        case Speed1: recText += F("\x81"); break;
        case Speed10: recText += F("\x81\x81"); break;
        case Speed100: recText += F("\x81\x81\x81"); break;
    }
    SharpDisplay::setLineText(10, recText);
    String position = String(gTopRecord+gCursorPosition+1, DEC);
    position += '/';
    position += String(gNumberOfRecords, DEC);
    SharpDisplay::setLineText(11, position);
}

    
void handleKey(KeyPad::Key key)
{
    switch (key) {
        case KeyPad::Up:
            if (gScrollSpeed == Speed1) {
                if (gCursorPosition > 0) {
                    --gCursorPosition;
                } else if (gTopRecord > 0) {
                    --gTopRecord;
                }
            } else if (gScrollSpeed == Speed10) {
                if (gTopRecord >= 10) {
                    gTopRecord -= 10;
                } else {
                    gTopRecord = 0;
                }
            } else if (gScrollSpeed == Speed100) {
                if (gTopRecord >= 100) {
                    gTopRecord -= 100;
                } else {
                    gTopRecord = 0;
                }
            }
            break;
            
        case KeyPad::Down:
            if (gScrollSpeed == Speed1) {
                if (gCursorPosition < min(2, gNumberOfRecords)) {
                    ++gCursorPosition;
                } else if (gNumberOfRecords > 3 && (gTopRecord+3) < gNumberOfRecords) {
                    ++gTopRecord;
                }
            } else if (gScrollSpeed == Speed10) {
                if (gNumberOfRecords > 3) {
                    gTopRecord += 10;
                    if ((gTopRecord+3) > gNumberOfRecords) {
                        gTopRecord = gNumberOfRecords-3;
                    }
                }
            } else if (gScrollSpeed == Speed100) {
                if (gNumberOfRecords > 3) {
                    gTopRecord += 100;
                    if ((gTopRecord+3) > gNumberOfRecords) {
                        gTopRecord = gNumberOfRecords-3;
                    }
                }
            }
            break;
            
        case KeyPad::Right:
            switch (gScrollSpeed) {
                case Speed1: gScrollSpeed = Speed10; break;
                case Speed10: gScrollSpeed = Speed100; break;
                case Speed100: gScrollSpeed = Speed1; break;
            }
            break;
            
        case KeyPad::Left:
            ViewManager::setNextView(ViewManager::MainMenuView);
            return;
            
        default:
            break;
    }
    ViewManager::setNeedsDisplayUpdate();
}
    

}
}


