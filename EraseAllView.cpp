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
#include "EraseAllView.h"


#include "SharpDisplay.h"
#include "LogSystem.h"
#include "ViewManager.h"


namespace lr {
namespace EraseAllView {

    
static uint8_t gCounter;
    

void viewWillAppear()
{
    gCounter = 10;
}


void updateDisplay()
{
    SharpDisplay::clearRows(0, 9);
    SharpDisplay::setCursorPosition(2, 0);
    SharpDisplay::setTextInverse(false);
    SharpDisplay::writeText(PSTR(" Erase All?\n\n  "));
    SharpDisplay::setTextInverse(true);
    SharpDisplay::writeText(PSTR(" "));
    SharpDisplay::writeText(String(gCounter, DEC));
    SharpDisplay::writeText(PSTR(" "));
}


void handleKey(KeyPad::Key key)
{
    switch (key) {
        case KeyPad::Left:
            ViewManager::setNextView(ViewManager::MainMenuView);
            break;
            
        case KeyPad::Enter:
            if (--gCounter == 0) {
                LogSystem::format();
                ViewManager::setNextView(ViewManager::MainMenuView);
            } else {
                ViewManager::setNeedsDisplayUpdate();
            }
            break;
        
        default:
            break;
    }
}

    
}
}

