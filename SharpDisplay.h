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


/// This is a compact driver for the LS013B4DN04 display from Sharp.
///
/// The driver uses fixed 8x8 pixel characters to show text on the display.
/// You can use special characters to draw simple boxes and lines.
///
/// See http://luckyresistor.me for details.
///
/// You can buy the display from Adafruit: http://www.adafruit.com/products/1393
///
/// Because there is only one instance of this class possible, it is
/// implemented as collection of functions to reduce the overall code size.
///
namespace SharpDisplay {
    
    
/// The scroll direction.
///
enum ScrollDirection : uint8_t {
    ScrollUp,
    ScrollDown,
    ScrollLeft,
    ScrollRight
};

/// The refresh interval
///
enum RefreshInterval : uint8_t {
    NormalRefresh, ///< Refresh every ~100ms
    SlowRefresh, ///< Refresh every second.
};

/// The type for the interrupt callback.
///
typedef void (*InterruptCallback)();

/// Call this method in the setup() method to initialize the driver.
///
/// This will start timer2 to automatically trigger the display refresh.
///
void begin(uint8_t chipSelectPin, uint8_t clockPin, uint8_t dataPin);

/// Change the refresh interval
///
void setRefreshInterval(RefreshInterval refreshInterval);

/// Set a function which is called for each interrupt.
///
void setInterruptCallback(InterruptCallback interruptCallback);

/// Set the font for the display.
///
/// You have to set a font initially.
/// The driver will not check if the font provides data for all 128
/// characters. To save space in the flash RAM, you can provide a font
/// which covers less than 128 characters. The behaviour for
/// higher characters is undefined in this case (random data is displayed).
///
/// The font has to be located in flash memory.
///
/// @param fontData A pointer to the font data in flash memory.
///
void setFont(const uint8_t *fontData);

/// Get the width of the display in characters.
///
inline uint8_t getScreenWidth() { return 12; }

/// Get the height of the display in characters.
///
inline uint8_t getScreenHeight() { return 12; }

/// Clears the display.
///
/// This will also clear the text buffer and fill the buffer with space (0x20).
/// It also sets the cursor at position 0, 0.
///
void clear();

/// Clears a range of rows.
///
void clearRows(uint8_t startRow, uint8_t rowCount = 1);

/// Enable/Disable inverse future text.
///
/// If you enable inverse text, any subsequent call of text writing methods
/// will write inverse text.
///
void setTextInverse(bool enable);

/// Set a single character on the screen.
///
/// The call of this method does not affect the cursor position.
///
/// @param row The row to set the character.
/// @param column The colum to set the character.
/// @param character The character to set at the given location.
///
void setCharacter(uint8_t row, uint8_t column, uint8_t character);

/// Get a single character from the screen.
///
/// @param row The row for the character.
/// @param column The column for the character.
/// @return The character the the given position.
///
char getCharacter(uint8_t row, uint8_t column);

/// Set the text for a single line.
///
/// If the text is longer than the line, the text is cut off.
/// If the text is shorter than the line, the rest of the line
/// is filled with spaces.
///
/// The call of this method does not affect the cursor position.
/// Any control characters are ignored.
///
/// @param row The row to write the text into.
/// @param text The text to write in the row.
///
void setLineText(uint8_t row, const String &text);

/// Set the text for a single line using null terminated text from flash memory.
///
void setLineText(uint8_t row, const char *prgMemText);

/// Fill a row with the given character.
///
void fillRow(uint8_t row, char c);

/// Set the inversion state of a row
///
/// @param row The row to set the inversion.
/// @param inverted true to set the row to inverse, false to set it not inverse.
///
void setLineInverted(uint8_t row, bool inverted);

/// Set the position of the cursor.
///
/// The cursor can be one column and row out of the
/// visible screen.
///
/// @param row The row for the cursor.
/// @param column The column for the cursor.
///
void setCursorPosition(uint8_t row, uint8_t column);

/// Get the current cursor position.
///
void getCursorPosition(uint8_t &row, uint8_t &column);

/// Write a single character at the cursor position on the screen.
///
/// @param character The character to write.
///
void writeCharacter(uint8_t c);

/// Write text at the cursor position on the screen.
///
/// The text will wrap around after the last charcater in the
/// column. The control character "newline" (\n) will add a
/// line break and continue on the next line. If the cursor
/// reaches the end of the screen, the screen contents are
/// scrolled up one row.
///
/// @param text The text to write to the screen.
///
void writeText(const String &text);

/// Write text at the cursor position on the screen.
///
/// The text will wrap around after the last charcater in the
/// column. The control character "newline" (\n) will add a
/// line break and continue on the next line. If the cursor
/// reaches the end of the screen, the screen contents are
/// scrolled up one row.
///
/// @param text The text to write to the screen in flash memory.
///
void writeText(const char *prgMemText);

/// Scroll the screen in the given direction.
///
/// The new areas are initialized with empty characters.
///
/// @param direction The scroll direction.
///
void scrollScreen(ScrollDirection direction);


}
}





