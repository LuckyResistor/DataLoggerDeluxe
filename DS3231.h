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

#include "DateTime.h"


namespace lr {
namespace DS3231 {

    
/// Initialize the real time clock driver.
///
/// @param yearBase The year base which is used for the RTC.
///    The RTC stores the year only with two digits, plus one
///    additional bit for the next century. If you set the
///    year base to 2000, the RTC will hold the correct time
///    for 200 years, starting from 2000-01-01 00:00:00.
///
void begin(uint16_t yearBase = 2000);

/// Get the current date/time.
///
DateTime getDateTime();

/// Set the date/time.
///
void setDateTime(const DateTime &dateTime);

/// Check if the RTC is running.
///
bool isRunning();

/// Get the temperature in degrees celsius.
///
float getTemperature();


}
}


