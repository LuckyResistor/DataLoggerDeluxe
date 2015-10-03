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


//#define LR_DHT22_DEBUG 1


namespace lr {


/// A own compact implementation of the DHT22 sensor library.
///
/// Using some timing values from the DHT library by Adafruit.
///
namespace DHT22 {
    

/// One single measurement from the sensor.
///
struct Measurement {
    float temperature;
    float humidity;
};

    
/// Initialize the library
///
void begin(uint8_t pin);

/// Read the temperature and humidity
///
/// The temperature is read in celsius.
///
Measurement readTemperatureAndHumidity();


}
}


