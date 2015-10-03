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
#include "MainMenuView.h"


#include "Application.h"
#include "SharpDisplay.h"
#include "ViewManager.h"


namespace lr {
namespace MainMenuView {
    
    
// The items in the start menu.
static const char cItem1[] PROGMEM = "Start Rec.";
static const char cItem2[] PROGMEM = "View Records";
static const char cItem3[] PROGMEM = "Send Records";
static const char cItem4[] PROGMEM = "Set Interval";
static const char cItem5[] PROGMEM = "Erase All";
static const char cItem6[] PROGMEM = "Adjust Time";
static const char cItem7[] PROGMEM = "Version Info";
static const char *cItems[7] = {cItem1, cItem2, cItem3, cItem4, cItem5, cItem6, cItem7};
static const uint8_t cItemCount = 7;

// The currently selected item.
static uint8_t gSelectedItem = 0;

    
void updateDisplay()
{
    SharpDisplay::setTextInverse(false);
    SharpDisplay::setLineText(0, PSTR("Main Menu"));
    SharpDisplay::fillRow(1, '\x89');
    const uint8_t visibleLines = SharpDisplay::getScreenHeight()-5;
    for (uint8_t i = 0; i < visibleLines; ++i) {
        if (i < cItemCount) {
            SharpDisplay::setTextInverse(i == gSelectedItem);
            SharpDisplay::setLineText(i+2, cItems[i]);
        } else {
            SharpDisplay::setTextInverse(false);
            SharpDisplay::fillRow(i+2, ' ');
        }
    }
}
    
    
void handleKey(KeyPad::Key key)
{
    if (key == KeyPad::Down && gSelectedItem < (cItemCount-1)) {
        ++gSelectedItem;
        ViewManager::setNeedsDisplayUpdate();
    } else if (key == KeyPad::Up && gSelectedItem > 0) {
        --gSelectedItem;
        ViewManager::setNeedsDisplayUpdate();
    } else if (key == KeyPad::Enter || key == KeyPad::Right) {
        switch (gSelectedItem) {
            case 0: ViewManager::setNextView(ViewManager::RecordView); break;
            case 1: ViewManager::setNextView(ViewManager::ViewRecordView); break;
            case 2: ViewManager::setNextView(ViewManager::SendRecordView); break;
            case 3: ViewManager::setNextView(ViewManager::SetIntervalView); break;
            case 4: ViewManager::setNextView(ViewManager::EraseAllView); break;
            case 5: ViewManager::setNextView(ViewManager::AdjustTimeView); break;
            case 6: ViewManager::setNextView(ViewManager::VersionInfoView); break;
        }
    }
}

    
void viewWillAppear()
{
    KeyPad::clear();
    Application::setOperationMode(Application::MenuMode);
}

    
}
}


