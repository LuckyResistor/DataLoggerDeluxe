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
#include "DS3231.h"


#include <Wire.h>


namespace lr {
namespace DS3231 {


// The chip address in the I2C bus.
static const uint8_t cChipAddress = 0x68;
    
// The used registers
static const uint8_t cSecondsRegister = 0x00;
static const uint8_t cControlRegister = 0x0e;
static const uint8_t cTemperatureRegister = 0x11;

// The year base.
static uint16_t gYearBase;
 
    
// Function to convert BCD format into binary format.
static inline uint8_t convertBcdToBin(const uint8_t bcd)
{
    return (bcd&0xf)+((bcd>>4)*10);
}

    
// Function to convert binary to BCD format.
static inline uint8_t convertBinToBcd(const uint8_t bin)
{
    return (bin%10)+((bin/10)<<4);
}

    
void begin(uint16_t yearBase)
{
    gYearBase = yearBase;
}

    
DateTime getDateTime()
{
    // Address the seconds register to read this plus
    // all subsequent registers for the time.
    Wire.beginTransmission(cChipAddress);
    Wire.write(cSecondsRegister);
    Wire.endTransmission();
    // Read the initial 7 registers.
    Wire.requestFrom(cChipAddress, 7u);
    const uint8_t seconds = Wire.read();
    const uint8_t minutes = Wire.read();
    const uint8_t hours = Wire.read();
    const uint8_t dayOfWeek = Wire.read();
    const uint8_t day = Wire.read();
    const uint8_t month = Wire.read();
    const uint8_t year = Wire.read();
    // Convert these values into a date object.
    return DateTime::fromUncheckedValues(
        static_cast<uint16_t>(convertBcdToBin(year))+((month&_BV(7))!=0?(gYearBase+100):gYearBase),
        convertBcdToBin(month&0x1f),
        convertBcdToBin(day&0x3f),
        convertBcdToBin(hours&0x3f),
        convertBcdToBin(minutes&0x7f),
        convertBcdToBin(seconds&0x7f),
        dayOfWeek&0x7);
}

    
void setDateTime(const DateTime &dateTime)
{
    // Basic year check
    const uint16_t newYear = dateTime.getYear();
    if (newYear < gYearBase || newYear >= (gYearBase+200)) {
        return; // Ignore this call.
    }
    // Prepare all registers which will be written
    const uint8_t seconds = convertBinToBcd(dateTime.getSecond());
    const uint8_t minutes = convertBinToBcd(dateTime.getMinute());
    const uint8_t hours = convertBinToBcd(dateTime.getHour());
    const uint8_t dayOfWeek = dateTime.getDayOfWeek();
    const uint8_t day = convertBinToBcd(dateTime.getDay());
    const uint8_t month = convertBinToBcd(dateTime.getMonth()) |
        (dateTime.getYear()>=(gYearBase+100)?_BV(7):0);
    const uint8_t year = convertBinToBcd(dateTime.getYear()%100);
    // Write the prepared values into the register
    Wire.beginTransmission(cChipAddress);
    Wire.write(cSecondsRegister);
    Wire.write(seconds);
    Wire.write(minutes);
    Wire.write(hours);
    Wire.write(dayOfWeek);
    Wire.write(day);
    Wire.write(month);
    Wire.write(year);
    Wire.endTransmission();
}

    
bool isRunning()
{
    // Address the control register.
    Wire.beginTransmission(cChipAddress);
    Wire.write(cControlRegister);
    Wire.endTransmission();
    // Read this register (1 byte).
    Wire.requestFrom(cChipAddress, 1u);
    const uint8_t controlRegister = Wire.read();
    // If the 7th bit is zero, the RTC is running.
    return (controlRegister&_BV(7))==0;
}

    
float getTemperature()
{
    // Address the temperature register.
    Wire.beginTransmission(cChipAddress);
    Wire.write(cTemperatureRegister);
    Wire.endTransmission();
    // Read the temperature.
    Wire.requestFrom(cChipAddress, 2u);
    const int8_t temperatureMSB = Wire.read();
    const uint8_t temperatureLSB = Wire.read();
    // Create a float from this values.
    float result = static_cast<float>(temperatureMSB);
    const float fraction = static_cast<float>(temperatureLSB >> 6) * 0.25f;
    if (result < 0) {
        result -= fraction;
    } else {
        result += fraction;
    }
    return result;
}
    

}
}


