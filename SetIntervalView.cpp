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
#include "SetIntervalView.h"


#include "Application.h"
#include "Settings.h"
#include "SharpDisplay.h"
#include "ViewManager.h"


namespace lr {
namespace SetIntervalView {


// The intervals encoded as number, unit tuples.
static const uint8_t cIntervals[] PROGMEM = {
    10, 's',
    30, 's',
    1, 'm',
    10, 'm',
    1, 'h',
    8, 'h',
    24, 'h'
};

// The index from the settings.
static uint8_t gSettingsIndex;

// The currently selected index.
static uint8_t gSelectedIndex;
    

void viewWillAppear()
{
    gSettingsIndex = Settings::getInterval();
    gSelectedIndex = gSettingsIndex;
}


void updateDisplay()
{
    SharpDisplay::setLineText(0, PSTR("Interval"));
    SharpDisplay::fillRow(1, '\x89');
    for (uint8_t i = 0; i < 7; ++i) {
        String text;
        if (i == gSettingsIndex) {
            text = String(F("\x9e "));
        } else {
            text = String(F("  "));
        }
        text += String(pgm_read_byte(&cIntervals[i*2]), DEC);
        text += static_cast<char>(pgm_read_byte(&cIntervals[i*2+1]));
        SharpDisplay::setTextInverse(i == gSelectedIndex);
        SharpDisplay::setLineText(i+2, text);
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
            
        case KeyPad::Enter:
            Settings::setInterval(static_cast<Settings::Interval>(gSelectedIndex));
        case KeyPad::Left:
            ViewManager::setNextView(ViewManager::MainMenuView);
            break;
            
        default:
            break;
    }
    ViewManager::setNeedsDisplayUpdate();
}

    
}
}
