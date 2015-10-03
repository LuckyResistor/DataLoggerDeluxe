#pragma once
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


#include <Arduino.h>


namespace lr {


/// The application
///
namespace Application {


/// The current operation mode
///
enum OperationMode : uint8_t {
    MenuMode, ///< The menu mode with fast refresh and reaction time.
    FullScreenMode, ///< The full screen, with fast refresh and reaction time.
    RecordingMode, ///< The power saving recording mode.
    PowerSave, ///< Just save power but keep menus running.
};

/// Call this in the setup() method.
///
void setup();

/// Call this in the loop() method.
///
void loop();

/// Set the next record time to the current time + 10 seconds.
///
void resetNextRecordTime();

/// Change the operation mode.
///
void setOperationMode(OperationMode mode);

/// Get the logo text.
///
const char* getLogoText();
    
    
}
}


