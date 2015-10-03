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

/// This is an abstraction for the connected keypad.
///
namespace KeyPad {

    
/// The different keys.
/// This is also a mask for pressed keys.
///
enum Key : uint8_t {
    None  = 0, ///< If no key is pressed.
    Up    = _BV(0), ///< The up key
    Down  = _BV(1), ///< The down key
    Left  = _BV(2), ///< The left key
    Right = _BV(3), ///< The right key
    Enter = _BV(4) ///< The start/enter key
};


/// Initialize the keypad
///
void begin();

/// Check the keypad
///
void checkKeys();

/// Clear the key queue.
///
void clear();

/// Check if a key was pressed.
///
/// @return A pressed key or None if no key was pressed.
///
Key getNextKey();

/// Check if there is a recorded key press.
///
bool hasNextKey();



}
}

