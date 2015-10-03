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
#include "KeyPad.h"


namespace lr {
namespace KeyPad {


// The pins for the keys.
static const uint8_t cKeyUpPin = 6;
static const uint8_t cKeyDownPin = 8;
static const uint8_t cKeyLeftPin = 5;
static const uint8_t cKeyRightPin = 12;
static const uint8_t cKeyEnterPin = 4;
    
/// All key pins in an array
static uint8_t gKeyPins[5] = {cKeyUpPin, cKeyDownPin, cKeyLeftPin, cKeyRightPin, cKeyEnterPin};
static uint8_t gKeyMasks[5] = {Up, Down, Left, Right, Enter}; ///< All key masks in an array.
static uint8_t gLastKeyMask; ///< The last key mask.
static Key gKeyBuffer[16]; ///< The last pressed keys.
static uint8_t gKeyBufferSize; ///< The current size of the key buffer;

    
/// Get a mask with all currently pressed keys.
///
uint8_t getCurrentKeyMask()
{
    uint8_t result = 0;
    for (uint8_t i = 0; i < 5; ++i) {
        if (digitalRead(gKeyPins[i]) == LOW) {
            result |= gKeyMasks[i];
        }
    }
    return result;
}


/// Get the currently pressed key.
///
Key getPressedKey()
{
    const uint8_t currentKeyMask = getCurrentKeyMask();
    const uint8_t lastKeyMask = gLastKeyMask;
    gLastKeyMask = currentKeyMask;
    uint8_t mask = 1;
    for (uint8_t i = 0; i < 5; ++i) {
        if ((lastKeyMask & mask) == 0 && (currentKeyMask & mask) != 0) {
            return static_cast<Key>(mask);
        }
        mask <<= 1;
    }
    return None;
}


void begin()
{
    pinMode(cKeyUpPin, INPUT);
    pinMode(cKeyDownPin, INPUT);
    pinMode(cKeyLeftPin, INPUT);
    pinMode(cKeyRightPin, INPUT);
    pinMode(cKeyEnterPin, INPUT);
    
    // Set the current keymask, this will ignore initially pressed keys.
    gLastKeyMask = getCurrentKeyMask();
    gKeyBufferSize = 0;
}

    
void checkKeys()
{
    if (gKeyBufferSize < 16) {
        Key key = getPressedKey();
        if (key != None) {
            gKeyBuffer[gKeyBufferSize] = key;
            ++gKeyBufferSize;
        }
    }
}

    
void clear()
{
    cli();
    gKeyBufferSize = 0;
    sei();
}

    
Key getNextKey()
{
    cli();
    Key result = None;
    if (gKeyBufferSize > 0) {
        result = gKeyBuffer[0];
        memmove(gKeyBuffer, gKeyBuffer+1, 15);
        --gKeyBufferSize;
    }
    sei();
    return result;
}


bool hasNextKey()
{
    cli();
    bool result = gKeyBufferSize > 0;
    sei();
    return result;
}

    
}
}




