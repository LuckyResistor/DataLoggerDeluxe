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
#include "AdjustTimeView.h"


#include "ViewManager.h"
#include "SharpDisplay.h"
#include "DS3231.h"


namespace lr {
namespace AdjustTimeView {


// The labels for the date/time fields and the two actions.
static const char cLabelYear[] PROGMEM = "Year: ";
static const char cLabelMonth[] PROGMEM = "Month: ";
static const char cLabelDay[] PROGMEM = "Day: ";
static const char cLabelHour[] PROGMEM = "Hour: ";
static const char cLabelMinute[] PROGMEM = "Minute: ";
static const char cLabelCancel[] PROGMEM = "Cancel";
static const char cLabelSet[] PROGMEM = "Adjust Time";
static const char *cLabels[7] = {cLabelYear, cLabelMonth, cLabelDay, cLabelHour, cLabelMinute, cLabelCancel, cLabelSet};
    
static uint8_t gSelectedIndex; ///< The currently selected index.
static uint16_t gElements[5]; ///< The date/time elements as numbers.
static DateTime gDateTime; ///< The corresponding date/time value.
    
    
/// Create a date/time from the elements.
///
void dtFromElements()
{
    gDateTime.setDate(gElements[0], gElements[1], gElements[2]);
    gDateTime.setTime(gElements[3], gElements[4], 0);
}


/// Fill the elements from the date/time value.
/// 
void elementsFromDT()
{
    gElements[0] = gDateTime.getYear();
    gElements[1] = gDateTime.getMonth();
    gElements[2] = gDateTime.getDay();
    gElements[3] = gDateTime.getHour();
    gElements[4] = gDateTime.getMinute();
}
    
    
void viewWillAppear()
{
    gSelectedIndex = 0;
    gDateTime = DS3231::getDateTime();
    elementsFromDT();
}


void updateDisplay()
{
    SharpDisplay::setTextInverse(false);
    SharpDisplay::setLineText(0, PSTR("Adjust Time"));
    SharpDisplay::fillRow(1, '\x89');
    SharpDisplay::clearRows(2, 7);
    for (uint8_t i = 0; i < 7; ++i) {
        SharpDisplay::setCursorPosition(i+2, 0);
        if (i < 5) {
            SharpDisplay::writeText(cLabels[i]);
            SharpDisplay::setTextInverse(i == gSelectedIndex);
            SharpDisplay::writeText(String(gElements[i], DEC));
        } else {
            SharpDisplay::setTextInverse(i == gSelectedIndex);
            SharpDisplay::writeText(cLabels[i]);
        }
        SharpDisplay::setTextInverse(false);
    }
}
    
    
void handleKey(KeyPad::Key key)
{
    switch (key) {
        case KeyPad::Up:
            if (gSelectedIndex > 0) {
                --gSelectedIndex;
            }
            break;
            
        case KeyPad::Down:
            if (gSelectedIndex < 6) {
                ++gSelectedIndex;
            }
            break;
            
        case KeyPad::Left:
            if (gSelectedIndex < 5) {
                gElements[gSelectedIndex] -= 1;
                dtFromElements();
                elementsFromDT();
            }
            break;
            
        case KeyPad::Right:
            if (gSelectedIndex < 5) {
                gElements[gSelectedIndex] += 1;
                dtFromElements();
                elementsFromDT();
            }
            break;
            
        case KeyPad::Enter:
            if (gSelectedIndex == 6) {
                DS3231::setDateTime(gDateTime);
            }
            ViewManager::setNextView(ViewManager::MainMenuView);
            break;
            
        default:
            break;
    }
    ViewManager::setNeedsDisplayUpdate();
}
 
    
}
}
