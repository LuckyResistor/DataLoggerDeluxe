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


#include "Storage.h"


namespace lr {
    

/// The settings for the device.
///
namespace Settings {
    
    
/// The recording interval.
///
enum Interval : uint8_t {
    I10s = 0, ///< 10 Seconds.
    I30s, ///< 30 Seconds.
    I1m, ///< 1 Minute.
    I10m, ///< 10 Minutes.
    I1h, ///< 1 Hour.
    I8h, ///< 8 Hours.
    I24h ///< 24 Hours/1 Day.
};


/// The serial speed
///
enum SerialSpeed : uint32_t {
    S300 = 300,
    S600 = 600,
    S1200 = 1200,
    S2400 = 2400,
    S4800 = 4800,
    S9600 = 9600,
    S14400 = 14400,
    S19200 = 19200,
    S28800 = 28800,
    S38400 = 38400,
    S57600 = 57600,
    S115200 = 115200
};


/// Initialize the settings, read them from the storage.
///
void begin();
    
/// The size required for the settings in bytes
///
uint16_t size();

/// Set a new interval
///
void setInterval(Interval interval);

/// Get the interval.
///
Interval getInterval();

/// The current inverval in seconds.
///
uint32_t getIntervalInSeconds();

/// Get the duration for the power save cycle.
///
uint16_t getPowerSaveDuration();

/// Set the serial speed.
///
void setSerialSpeed(SerialSpeed speed);

/// Get the current serial speed.
///
SerialSpeed getSerialSpeed();
    
    
}
}

