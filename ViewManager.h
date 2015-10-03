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


#include "DHT22.h"
#include "DateTime.h"


namespace lr {

    
/// The manager for all views.
///
namespace ViewManager {


/// The identifier for all views.
///
enum ViewIdentifier : int8_t {
    NoView = -1,
    MainMenuView = 0,
    RecordView,
    MemoryFullView,
    ViewRecordView,
    SendRecordView,
    SetIntervalView,
    EraseAllView,
    AdjustTimeView,
    VersionInfoView
};


/// Initialize the view manager.
///
void begin();

/// The loop code
///
void loop();

/// Request switch to a new view.
///
/// @param identifier The identifier of the requested view in flash ram.
///
void setNextView(ViewIdentifier nextView);

/// Request a display update.
///
void setNeedsDisplayUpdate();

/// Display an error/loop endless
///
void displayError(String message);

/// Update the time and sensor readings at the bottom of the display
///
void updateMeasurementDisplay(const DHT22::Measurement &measurement, const DateTime &dateTime, char modeDisplay);


}
}

