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
#include "SharpDisplay.h"


#include <avr/interrupt.h>


namespace lr {
namespace {
    
    
// This counter are used to time the display refresh
static uint8_t gDisplayRefreshCounter;
static uint8_t gDisplayRefreshTrigger;

// This counter is used to protect the display from stuck pixels.
static uint32_t gDisplayProtectCounter;
static const uint32_t gDisplayProtectTrigger = 0x6B3A0; // Approximate after 2h
    
// A callback for the interrupt
static SharpDisplay::InterruptCallback gInterruptCallback;
    
// Use a number of global variables for time critical parts
// The following variables are used for the communication.
static uint8_t gChipSelectPin;
static uint8_t gClockPin;
static volatile uint8_t *gClockPort;
static uint8_t gClockMask;
static uint8_t gDataPin;
static volatile uint8_t *gDataPort;
static uint8_t gDataMask;
   
    
// This constants are bitmasks for the commands
static const uint8_t CMD_WRITE = 0x80;
static const uint8_t CMD_CLEAR = 0x20;
static const uint8_t CMD_VCOM = 0x40;
    
    
// This flag is used to toggle the required VCOM bit.
static uint8_t gVComBit;

    
// The array with the 12x12 screen.
static const uint8_t gScreenHeight = 12;
static const uint8_t gScreenWidth = 12;
static uint8_t gScreenCharacters[gScreenHeight*gScreenWidth];
static const uint8_t gScreenRowRequiresUpdateSize = gScreenHeight/8+1;
static uint8_t gScreenRowRequiresUpdate[gScreenRowRequiresUpdateSize];
    
// The height of a single character.
static const uint8_t gCharacterHeight = 8;

// A empty font, to prevent any crashes if no font is set.
static const uint8_t gNoFont[8] PROGMEM = {0, 0, 0, 0, 0, 0, 0, 0};

// The current font for the display
static const uint8_t *gTextFont;
    
// The current text flags
static const uint8_t gTextFlagInverse = 0x80;
static const uint8_t gTextCharacterMask = 0x7f;
static uint8_t gTextFlags;
    
// The cursor position
static uint8_t gCursorX;
static uint8_t gCursorY;
    
    
// Set the clock low
inline static void setClockLow()
{
    *gClockPort &= ~gClockMask;
}

    
// Set the clock high
inline static void setClockHigh()
{
    *gClockPort |= gClockMask;
}

    
// Set the data low
inline static void setDataLow()
{
    *gDataPort &= ~gDataMask;
}
    
    
// Set the data high
inline static void setDataHigh()
{
    *gDataPort |= gDataMask;
}

    
// This method sends a single byte MSB first to the display
static void sendByteMSB(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; ++i) {
        setClockLow();
        if (byte & 0x80) {
            setDataHigh();
        } else {
            setDataLow();
        }
        setClockHigh();
        byte <<= 1;
    }
    setClockLow();
}

    
// This method sends a single byte LSB first to the display
static void sendByteLSB(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; ++i) {
        setClockLow();
        if (byte & 0x01) {
            setDataHigh();
        } else {
            setDataLow();
        }
        setClockHigh();
        byte >>= 1;
    }
    setClockLow();
}

    
// This method toggles the VCOM bit
inline static void toggleVComBit()
{
    gVComBit ^= CMD_VCOM;
}

    
// This method calculates a pointer to a character on the screen
inline uint8_t* getCharacterPosition(uint8_t row, uint8_t column)
{
    return &gScreenCharacters[(row*gScreenWidth)+column];
}
    

// Clear the whole screen.
inline void clearScreen()
{
    memset(gScreenCharacters, 0x00, gScreenWidth*gScreenHeight);
}
    

// Check if a given row is maked for updates
inline bool isRowMarkedForUpdate(uint8_t row)
{
    return ((gScreenRowRequiresUpdate[row>>3] & (1<<(row&7))) != 0);
}
    
    
// Mark a given row for an update
inline void markRowForUpdate(uint8_t row)
{
    gScreenRowRequiresUpdate[row>>3] |= (1<<(row&7));
}

    
// Mark the whole screen for an update.
inline void markScreenForUpdate()
{
    memset(gScreenRowRequiresUpdate, 0xff, gScreenRowRequiresUpdateSize);
}

 
// Remove any row update requests from the screen.
inline void clearScreenUpdate()
{
    memset(gScreenRowRequiresUpdate, 0x00, gScreenRowRequiresUpdateSize);
}

// A simple class to lock the interrupt and make sure it is enabled if
// the method ends.
class LockInterrupt {
public:
    LockInterrupt() {
        cli();
    }
    ~LockInterrupt() {
        sei();
    }
};
    
    
void refreshDisplay()
{
    // Send the write command.
    digitalWrite(gChipSelectPin, HIGH);
    sendByteMSB(CMD_WRITE|gVComBit);
    toggleVComBit();
    // Update all rows which need a refresh.
    for (uint8_t row = 0; row < gScreenHeight; ++row) {
        if (isRowMarkedForUpdate(row)) {
            // Draw the row pixel row by pixel row
            for (uint8_t pixelRow = 0; pixelRow < gCharacterHeight; ++pixelRow) {
                sendByteLSB(row*gCharacterHeight+pixelRow+1);
                for (uint8_t column = 0; column < gScreenWidth; ++column) {
                    const uint8_t screenData = *(getCharacterPosition(row, column));
                    const uint8_t characterIndex = (screenData & gTextCharacterMask);
                    uint16_t characterStart = characterIndex;
                    characterStart *= gCharacterHeight;
                    characterStart += pixelRow;
                    uint8_t pixelMask = pgm_read_byte(gTextFont+characterStart);
                    if ((screenData & gTextFlagInverse) != 0) { // Inverse character?
                        pixelMask = ~pixelMask;
                    }
                    sendByteMSB(pixelMask);
                }
                sendByteLSB(0x00);
            }
        }
    }
    sendByteLSB(0x00);
    digitalWrite(gChipSelectPin, LOW);
    clearScreenUpdate();
}

    
void refreshFullDisplay(bool invert)
{
    // Send the write command.
    digitalWrite(gChipSelectPin, HIGH);
    sendByteMSB(CMD_WRITE|gVComBit);
    toggleVComBit();
    // Update all rows which need a refresh.
    for (uint8_t row = 0; row < gScreenHeight; ++row) {
        // Draw the row pixel row by pixel row
        for (uint8_t pixelRow = 0; pixelRow < gCharacterHeight; ++pixelRow) {
            sendByteLSB(row*gCharacterHeight+pixelRow+1);
            for (uint8_t column = 0; column < gScreenWidth; ++column) {
                const uint8_t screenData = *(getCharacterPosition(row, column));
                const uint8_t characterIndex = (screenData & gTextCharacterMask);
                uint16_t characterStart = characterIndex;
                characterStart *= gCharacterHeight;
                characterStart += pixelRow;
                uint8_t pixelMask = pgm_read_byte(gTextFont+characterStart);
                if ((screenData & gTextFlagInverse) != 0) { // Inverse character?
                    pixelMask = ~pixelMask;
                }
                if (invert) {
                    pixelMask = ~pixelMask;
                }
                sendByteMSB(pixelMask);
            }
            sendByteLSB(0x00);
        }
    }
    sendByteLSB(0x00);
    digitalWrite(gChipSelectPin, LOW);
    clearScreenUpdate();
}

    
inline void fastSetCharacter(uint8_t row, uint8_t column, uint8_t character)
{
    uint8_t* const cp = getCharacterPosition(row, column);
    const uint8_t newChar = ((static_cast<uint8_t>(character)-0x20) & gTextCharacterMask) | gTextFlags;
    if (*cp != newChar) {
        *cp = newChar;
        markRowForUpdate(row);
    }
}


void fastScrollScreen(SharpDisplay::ScrollDirection direction)
{
    switch (direction) {
        case SharpDisplay::ScrollUp:
            memmove(getCharacterPosition(0, 0), getCharacterPosition(1, 0), gScreenWidth*(gScreenHeight-1));
            memset(getCharacterPosition(gScreenHeight-1, 0), 0, gScreenWidth);
            break;
            
        case SharpDisplay::ScrollDown:
            memmove(getCharacterPosition(1, 0), getCharacterPosition(0, 0), gScreenWidth*(gScreenHeight-1));
            memset(getCharacterPosition(0, 0), 0, gScreenWidth);
            break;
            
        case SharpDisplay::ScrollLeft:
            memmove(getCharacterPosition(0, 0), getCharacterPosition(0, 1), (gScreenWidth*gScreenHeight)-1);
            for (uint8_t row = 0; row < gScreenHeight; ++row) {
                uint8_t *c = getCharacterPosition(row, gScreenWidth-1);
                *c = 0;
            }
            break;
            
        case SharpDisplay::ScrollRight:
            memmove(getCharacterPosition(0, 1), getCharacterPosition(0, 0), (gScreenWidth*gScreenHeight)-1);
            for (uint8_t row = 0; row < gScreenHeight; ++row) {
                uint8_t *c = getCharacterPosition(row, 0);
                *c = 0;
            }
            break;
    }
    markScreenForUpdate();
}
    
    
inline void fastWriteCharacter(uint8_t c)
{
    if (c == '\n') { // Add a line break
        gCursorX = 0;
        if (gCursorY < gScreenHeight) {
            ++gCursorY;
        } else {
            // Cursor is at the bottom. Scroll is required.
            fastScrollScreen(SharpDisplay::ScrollUp);
        }
    } else if (c >= 0x20) { // Ignore any other control characters.
        if (gCursorX == gScreenWidth) {
            gCursorX = 0;
            if (gCursorY < gScreenHeight) {
                ++gCursorY;
            }
        }
        if (gCursorY == gScreenHeight) {
            fastScrollScreen(SharpDisplay::ScrollUp);
            --gCursorY;
        }
        fastSetCharacter(gCursorY, gCursorX, c);
        ++gCursorX;
    }
}


} // end of anonymous namespace
} // end of lr namespace


// Create an interrupt for timer2 overflow.
// This interrupt will automatically refresh the display.
ISR(TIMER2_OVF_vect)
{
    // Check the protect counter, this counter will make sure the
    // whole display is refreshed and inverted every two hours
    // to prevent any stuck pixels.
    bool refreshDone = false;
    ++lr::gDisplayProtectCounter;
    if (lr::gDisplayProtectCounter >= (lr::gDisplayProtectTrigger+0x80)) {
        lr::gDisplayProtectCounter = 0;
        lr::markScreenForUpdate();
    } else if (lr::gDisplayProtectCounter >= (lr::gDisplayProtectTrigger+0x40)) {
        // Refresh the whole display, back normal.
        if ((lr::gDisplayProtectCounter&7) == 0) {
            lr::refreshFullDisplay(false);
        }
        refreshDone = true;
    } else if (lr::gDisplayProtectCounter >= lr::gDisplayProtectTrigger) {
        // Refresh the whole display, inverse.
        if ((lr::gDisplayProtectCounter&7) == 0) {
            lr::refreshFullDisplay(true);
        }
        refreshDone = true;
    }
    
    // The regular display refresh count.
    ++lr::gDisplayRefreshCounter;
    if (lr::gDisplayRefreshCounter > lr::gDisplayRefreshTrigger) {
        lr::gDisplayRefreshCounter = 0;
        if (!refreshDone) {
            lr::refreshDisplay();
        }
    }
    
    // Check if there is a interrupt callback.
    if (lr::gInterruptCallback != nullptr) {
        lr::gInterruptCallback();
    }
}


namespace lr {
namespace SharpDisplay {

    
void begin(uint8_t chipSelectPin, uint8_t clockPin, uint8_t dataPin)
{
    // Prepare the values for the SPI communication
    gChipSelectPin = chipSelectPin;
    gClockPin = clockPin;
    gClockPort = portOutputRegister(digitalPinToPort(clockPin));
    gClockMask = digitalPinToBitMask(clockPin);
    gDataPin = dataPin;
    gDataPort = portOutputRegister(digitalPinToPort(dataPin));
    gDataMask = digitalPinToBitMask(dataPin);
    
    // Initialize the screen memory
    clearScreen();
    clearScreenUpdate();
    gTextFlags = 0;
    gTextFont = gNoFont;
    gCursorX = 0;
    gCursorY = 0;
    
    // Initialize the counter.
    gDisplayRefreshCounter = 0;
    gDisplayRefreshTrigger = 6; // ~100ms
    gDisplayProtectCounter = 0;
    
    // No interrupt callback.
    gInterruptCallback = nullptr;

    // Prepare all communication ports
    digitalWrite(gChipSelectPin, HIGH);
    digitalWrite(gClockPin, LOW);
    digitalWrite(gDataPin, HIGH);
    pinMode(gChipSelectPin, OUTPUT);
    pinMode(gClockPin, OUTPUT);
    pinMode(gDataPin, OUTPUT);
    // Set the VCOM bit
    gVComBit = CMD_VCOM;
    // Clear the display.
    clear();
    // Initialize timer2 for the display refresh.
    ASSR = 0; // Synchronous internal clock.
    TCCR2A = _BV(WGM21)|_BV(WGM20); // Normal operation. Fast PWM.
    TCCR2B |= _BV(CS22)|_BV(CS21)|_BV(CS20); // Prescaler to 1024.
    OCR2A = 0; // Ignore the compare
    OCR2B = 0; // Ignore the compare
    TIMSK2 = _BV(TOIE2); // Interrupt on overflow.
    sei(); // Allow interrupts.    
}

    
void setRefreshInterval(RefreshInterval refreshInterval)
{
    LockInterrupt lock;
    if (refreshInterval == NormalRefresh) {
        gDisplayRefreshTrigger = 6;
    } else {
        gDisplayRefreshTrigger = 61;
    }
    gDisplayRefreshCounter = 0;
}
    
    
void setInterruptCallback(InterruptCallback interruptCallback)
{
    gInterruptCallback = interruptCallback;
}

    
void setFont(const uint8_t *fontData)
{
    LockInterrupt lock;
    gTextFont = fontData;
    markScreenForUpdate();
}

    
void clear()
{
    LockInterrupt lock;
    // Send the clear command.
    digitalWrite(gChipSelectPin, HIGH);
    sendByteMSB(CMD_CLEAR|gVComBit);
    sendByteMSB(0);
    toggleVComBit();
    digitalWrite(gChipSelectPin, LOW);

    // Clear the screen.
    clearScreen();
    clearScreenUpdate();
    gCursorX = 0;
    gCursorY = 0;
}

    
void clearRows(uint8_t startRow, uint8_t rowCount)
{
    LockInterrupt lock;
    for (uint8_t y = 0; y < rowCount; ++y) {
        const uint8_t row = startRow+y;
        memset(getCharacterPosition(row, 0), 0, gScreenWidth);
        markRowForUpdate(row);
    }
}

    
void setTextInverse(bool enable)
{
    if (enable) {
        gTextFlags |= gTextFlagInverse;
    } else {
        gTextFlags &= ~gTextFlagInverse;
    }
}

    
void setCharacter(uint8_t row, uint8_t column, uint8_t character)
{
    LockInterrupt lock;
    if (row < gScreenWidth && column < gScreenHeight) {
        fastSetCharacter(row, column, character);
    }
}
   
    
char getCharacter(uint8_t row, uint8_t column)
{
    LockInterrupt lock;
    if (row < gScreenWidth && column < gScreenHeight) {
        const uint8_t* const cp = getCharacterPosition(row, column);
        return (*cp & gTextCharacterMask) + 0x20;
    } else {
        return 0;
    }
}

    
void setLineText(uint8_t row, const String &text)
{
    LockInterrupt lock;
    if (row < gScreenHeight) {
        const uint8_t length = text.length();
        for (uint8_t column = 0; column < gScreenWidth; ++column) {
            if (column < length) {
                fastSetCharacter(row, column, text.charAt(column));
            } else {
                fastSetCharacter(row, column, ' ');
            }
        }
        markRowForUpdate(row);
    }
}

    
void setLineText(uint8_t row, const char *prgMemText)
{
    LockInterrupt lock;
    if (row < gScreenHeight) {
        const uint8_t length = strlen_P(prgMemText);
        for (uint8_t column = 0; column < gScreenWidth; ++column) {
            if (column < length) {
                fastSetCharacter(row, column, pgm_read_byte(prgMemText + column));
            } else {
                fastSetCharacter(row, column, ' ');
            }
        }
        markRowForUpdate(row);
    }
}
    
    
void fillRow(uint8_t row, char c)
{
    LockInterrupt lock;
    if (row < gScreenHeight) {
        for (uint8_t column = 0; column < gScreenWidth; ++column) {
            fastSetCharacter(row, column, c);
        }
    }
}

    
void setLineInverted(uint8_t row, bool inverted)
{
    LockInterrupt lock;
    if (row < gScreenHeight) {
        for (uint8_t column = 0; column < gScreenWidth; ++column) {
            uint8_t *c = getCharacterPosition(row, column);
            if (inverted) {
                *c |= gTextFlagInverse;
            } else {
                *c &= ~gTextFlagInverse;
            }
        }
        markRowForUpdate(row);
    }
}
    
    
void setCursorPosition(uint8_t row, uint8_t column)
{
    // Check the bounds.
    if (row > gScreenHeight) {
        row = gScreenHeight;
    }
    if (column > gScreenWidth) {
        column = gScreenWidth;
    }
    gCursorY = row;
    gCursorX = column;
    // If the cursor is below the last row, only X position 0 is valid.
    if (gCursorY == gScreenHeight) {
        gCursorX = 0;
    }
}
    
    
void getCursorPosition(uint8_t &row, uint8_t &column)
{
    row = gCursorY;
    column = gCursorX;
}
    
    
void writeCharacter(uint8_t c)
{
    LockInterrupt lock;
    fastWriteCharacter(c);
}

    
void writeText(const String &text)
{
    LockInterrupt lock;
    const unsigned int length = text.length();
    for (unsigned int i = 0; i < length; ++i) {
        fastWriteCharacter(text.charAt(i));
    }
}

    
void writeText(const char *prgMemText)
{
    LockInterrupt lock;
    char c;
    while ((c = pgm_read_byte(prgMemText++)) != 0) {
        fastWriteCharacter(c);
    }
}

    
void scrollScreen(ScrollDirection direction)
{
    LockInterrupt lock;
    fastScrollScreen(direction);
}
    

}
}






