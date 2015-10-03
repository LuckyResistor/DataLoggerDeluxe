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
#include "Settings.h"


#include <util/crc16.h>


namespace lr {
namespace Settings {
    
    
/// The stored data structure.
///
struct Data {
    Interval interval; ///< The recording interval.
    SerialSpeed serialSpeed; ///< The serial speed.
    uint16_t crc; ///< The CRC-16 of the data.
};

// The intervals in seconds.
static const uint32_t cIntervals[] PROGMEM = {10, 30, 60, 600, 3600, 28800, 86400};

// The optimal power save delays in seconds.
static const uint8_t cPowerSaveDurations[] PROGMEM = {2, 5, 10, 60, 60, 60, 60};

// The current representation of the stored data.
static Data gData;
    
  
void resetToDefault()
{
    gData.interval = I1h;
    gData.serialSpeed = S9600;
}


void saveToStorage()
{
    // Recalculate the CRC-16 from the values.
    gData.crc = 0;
    uint16_t crc = 0xffff;
    const uint8_t *dataPtr = reinterpret_cast<const uint8_t*>(&gData);
    for (uint8_t i = 0; i < sizeof(Data); ++i) {
        crc = _crc16_update(crc, dataPtr[i]);
    }
    gData.crc = crc;
    // Save the data block to the storage.
    Storage::writeBytes(0, dataPtr, sizeof(Data));
}


void begin()
{
    // First initialize the data struct from the storage.
    // This can be completely random data.
    Storage::readBytes(0, reinterpret_cast<uint8_t*>(&gData), sizeof(Data));
    // Keep the read CRC value.
    const uint16_t dataCRC = gData.crc;
    // Set the CRC in the data to 0 to calculate the actual CRC value.
    gData.crc = 0;
    // Calculate the CRC-16 from the data block.
    uint16_t crc = 0xffff;
    const uint8_t *dataPtr = reinterpret_cast<const uint8_t*>(&gData);
    for (uint8_t i = 0; i < sizeof(Data); ++i) {
        crc = _crc16_update(crc, dataPtr[i]);
    }
    // Compare the values. If the CRC does not match, use the default values.
    if (crc != dataCRC) {
        resetToDefault();
    }
}


uint16_t size()
{
    return sizeof(Data);
}

    
void setInterval(Interval interval)
{
    gData.interval = interval;
    saveToStorage();
}

    
Interval getInterval()
{
    return gData.interval;
}

    
uint32_t getIntervalInSeconds()
{
    return pgm_read_dword(&cIntervals[gData.interval]);
}


uint16_t getPowerSaveDuration()
{
    return pgm_read_byte(&cPowerSaveDurations[gData.interval]);
}

    
void setSerialSpeed(SerialSpeed speed)
{
    gData.serialSpeed = speed;
    saveToStorage();
}
    
    
SerialSpeed getSerialSpeed()
{
    return gData.serialSpeed;
}

    
}
}


