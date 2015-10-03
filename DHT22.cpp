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


namespace lr {
namespace DHT22 {


static uint8_t gPin; ///< The pin to read from.
static uint8_t gPinMask; ///< The bit for the pin.
static uint8_t gPinPort; ///< The port for the pin.
static uint32_t gPulseTimeout; ///< The maximum count to wait for a pulse.

    
/// The result for measuring a pulse.
///
enum PulseResult : uint8_t {
    PulseHigh,
    PulseLow,
    Timeout
};

    
void begin(uint8_t pin)
{
    gPin = pin;
    gPinMask = digitalPinToBitMask(gPin);
    gPinPort = digitalPinToPort(gPin);
    gPulseTimeout = microsecondsToClockCycles(1000); // > 1ms timeout.
    pinMode(gPin, INPUT);
    digitalWrite(gPin, HIGH);
}


/// Read a single pulse from the pin.
///
PulseResult getPulse()
{
    uint32_t lowCount = 0;
    uint32_t highCount = 0;
    while ((*portInputRegister(gPinPort) & gPinMask) == 0) {
        if (++lowCount > gPulseTimeout) {
            return Timeout;
        }
    }
    while ((*portInputRegister(gPinPort) & gPinMask) != 0) {
        if (++highCount > gPulseTimeout) {
            return Timeout;
        }
    }
    return (highCount>lowCount) ? PulseHigh : PulseLow;
}


Measurement readTemperatureAndHumidity()
{
    Measurement measurement = {NAN, NAN};
    
    // 5 bytes of read data.
    uint8_t readData[5];
    memset(readData, 0, 5);
    
    // Start time critical code
    noInterrupts();
    
    // Trigger the start of a new read.
    digitalWrite(gPin, HIGH);
    delay(250);
    pinMode(gPin, OUTPUT);
    digitalWrite(gPin, LOW);
    delay(20); // Data low for 20ms.
    digitalWrite(gPin, HIGH);
    delayMicroseconds(40);
    // Back observing the line for the data.
    pinMode(gPin, INPUT);
    delayMicroseconds(10);
    
    // Read and ignore the initial pulse from the sensor.
    if (getPulse() == Timeout) {
#ifdef LR_DHT22_DEBUG
        Serial.println(F("Timeout for initial pulse."));
#endif
        goto END_READ;
    }
    
    // Now read all 40 bits.
    for (uint8_t i = 0; i < 5; ++i) {
        for (uint8_t j = 0; j < 8; ++j) {
            switch (getPulse()) {
                case PulseHigh:
                    readData[i] <<= 1;
                    readData[i] |= 1;
                    break;
                case PulseLow:
                    readData[i] <<= 1;
                    break;
                case Timeout:
#ifdef LR_DHT22_DEBUG
                    Serial.print(F("Timeout for reading byte "));
                    Serial.print(i);
                    Serial.print(F(" bit "));
                    Serial.println(j);
#endif
                    goto END_READ;
            }
        }
    }

#ifdef LR_DHT22_DEBUG
    Serial.print(F("Read bytes: 0x"));
    Serial.print(readData[0], HEX);
    Serial.print(F(", 0x"));
    Serial.print(readData[1], HEX);
    Serial.print(F(", 0x"));
    Serial.print(readData[2], HEX);
    Serial.print(F(", 0x"));
    Serial.print(readData[3], HEX);
    Serial.print(F(", 0x"));
    Serial.print(readData[4], HEX);
#endif
    
    // Check the checksum
    if (readData[4] != ((readData[0]+readData[1]+readData[2]+readData[3])&0xff)) {
#ifdef LR_DHT22_DEBUG
        Serial.println(F("Checksum does not match."));
#endif
        goto END_READ;
    }
    
    // Convert the read bits into temperature and humidity
    measurement.temperature = (static_cast<uint16_t>(readData[2]&0x7f) << 8) + readData[3];
    measurement.temperature /= 10.0f;
    if ((readData[2] & 0x80) != 0) {
        measurement.temperature *= -1.0f;
    }
    measurement.humidity = (static_cast<uint16_t>(readData[0]&0x7f) << 8) + readData[1];
    measurement.humidity /= 10.0f;
    if ((readData[0] & 0x80) != 0) {
        measurement.humidity *= -1.0f;
    }
    
END_READ:
    // End time critical code
    interrupts();
    
    return measurement;
}


}
}

