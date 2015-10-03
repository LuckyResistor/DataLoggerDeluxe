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
#include "LogSystem.h"


#include "Storage.h"

#include <util/crc16.h>


namespace lr {


LogRecord::LogRecord()
    : _dateTime(), _temperature(0.0f), _humidity(0.0f)
{    
}


LogRecord::~LogRecord()
{
}


LogRecord::LogRecord(const DateTime &dateTime, float temperature, float humidity)
    : _dateTime(dateTime), _temperature(temperature), _humidity(humidity)
{
    if (_temperature > 100.0f) {
        _temperature = 100.0f;
    }
    if (_temperature < -273.15f) {
        _temperature = -273.15f;
    }
    if (_humidity > 100.0f) {
        _humidity = 100.0f;
    }
    if (_humidity < 0.0f) {
        _humidity = 0.0f;
    }
}


bool LogRecord::isNull() const
{
    return _dateTime.isFirst() && _humidity == 0.0f && _temperature == 0.0f;
}


void LogRecord::writeToSerial() const
{
    Serial.print(_dateTime.toString(DateTime::FormatLong));
    Serial.print(",");
    Serial.print(_temperature, 2);
    Serial.print(",");
    Serial.println(_humidity, 2);
}


namespace LogSystem {


static uint32_t gReservedForConfig; ///< The number of bytes reserved for the settings.
static uint32_t gCurrentNumberOfRecords; ///< The current number of records.
static uint32_t gMaximumNumberOfRecords; ///< The maximum number of records.


// The internal representation of a log record.
//
struct InternalLogRecord
{
    uint32_t time; // The time as seconds since 2000-01-01 00:00:00.
    float humidity; // The humidity value from the sensor.
    float temperature; // The humidity value from the sensor.
    uint16_t crc; // The CRC-16 of the record.
};

    
// Calculate the start of a record.
//
inline uint32_t getRecordStart(uint32_t index)
{
    return gReservedForConfig + (sizeof(InternalLogRecord) * index);
}

    
// Read one single internal record from the storage.
//
// @param index The index of the record.
// @return A copy of the internal record.
//
inline InternalLogRecord getInternalRecord(uint32_t index)
{
    InternalLogRecord record;
    Storage::readBytes(getRecordStart(index), reinterpret_cast<uint8_t*>(&record), sizeof(InternalLogRecord));
    return record;
}


// Write a single internal record to the storage.
//
// @param record The record to store.
// @param index The index of the record.
//
inline void setInternalRecord(const InternalLogRecord *record, uint32_t index)
{
    Storage::writeBytes(getRecordStart(index), reinterpret_cast<const uint8_t*>(record), sizeof(InternalLogRecord));
}
    

// Set a record in the storage to zero.
//
// @param index The index of the record to zero.
//
void zeroInternalRecord(uint32_t index)
{
    uint32_t storageIndex = getRecordStart(index);
    for (uint8_t i = 0; i < sizeof(InternalLogRecord); ++i) {
        Storage::writeByte(storageIndex, 0);
        ++storageIndex;
    }
}
    

// Check if an internal record is null.
//
// This is true if all bytes of the records are null.
//
// @param record The record to check.
// @return true if the record is null.
//
bool isInternalRecordNull(InternalLogRecord *record)
{
    uint8_t *recordPtr = reinterpret_cast<uint8_t*>(record);
    for (uint8_t i = 0; i < sizeof(InternalLogRecord); ++i) {
        if (*recordPtr != 0) {
            return false;
        }
        ++recordPtr;
    }
    return true;
}

    
// Calculate the CRC for the record.
//
// The CRC is calculated as CRC-16 while the CRC field is set to 0.
// All nibbles of the CRC-16 combined by XOR.
//
// @param record The record to calculate the CRC for.
// @return The 4-bit CRC
//
uint16_t getCRCForInternalRecord(InternalLogRecord *record)
{
    uint16_t crc = 0xFFFF;
    InternalLogRecord recordForCRC = *record;
    recordForCRC.crc = 0;
    uint8_t *recordPtr = reinterpret_cast<uint8_t*>(&recordForCRC);
    for (uint8_t i = 0; i < sizeof(InternalLogRecord); ++i) {
        crc = _crc16_update(crc, *recordPtr);
        ++recordPtr;
    }
    return crc;
}
    
    
// Check if an internal record is valid.
//
// This is true if all values of the record are in a valid range
// and the CRC code is valid.
//
// @param record The record to check.
// @return true if the record is valid.
//
bool isInternalRecordValid(InternalLogRecord *record)
{
    if (record->humidity < 0.0f ||
        record->humidity > 100.0f ||
        record->temperature < -273.15f ||
        record->temperature > 100.0f) {
        return false; // out of range.
    }
    const uint16_t crc = getCRCForInternalRecord(record);
    return crc == record->crc;
}
    
    
void begin(uint32_t reservedForConfig)
{
    gReservedForConfig = reservedForConfig;
    gCurrentNumberOfRecords = 0;
    gMaximumNumberOfRecords = 0;
    
    // Calculate the maximum number of records.
    gMaximumNumberOfRecords = (Storage::size() - gReservedForConfig) / sizeof(InternalLogRecord);
    // Scan the storage for valid records.
    uint32_t index = 0;
    InternalLogRecord record = getInternalRecord(index);
    while (!isInternalRecordNull(&record)) {
        if (!isInternalRecordValid(&record)) {
            break;
        }
        ++index;
        record = getInternalRecord(index);
    }
    gCurrentNumberOfRecords = index;
}


LogRecord getLogRecord(uint32_t index)
{
    if (index >= gCurrentNumberOfRecords) {
        return LogRecord();
    }
    const InternalLogRecord record = getInternalRecord(index);
    return LogRecord(DateTime::fromSecondsSince2000(record.time), record.temperature, record.humidity);
}


bool appendRecord(const LogRecord &logRecord)
{
    if (gCurrentNumberOfRecords >= gMaximumNumberOfRecords) {
        return false;
    }
    // zero the following record if possible
    if (gCurrentNumberOfRecords+1 < gMaximumNumberOfRecords) {
        zeroInternalRecord(gCurrentNumberOfRecords+1);
    }
    // convert the record into the internal structure.
    InternalLogRecord internalRecord;
    memset(&internalRecord, 0, sizeof(InternalLogRecord));
    internalRecord.time = logRecord.getDateTime().toSecondsSince2000();
    internalRecord.humidity = logRecord.getHumidity();
    internalRecord.temperature = logRecord.getTemperature();
    internalRecord.crc = getCRCForInternalRecord(&internalRecord);
    setInternalRecord(&internalRecord, gCurrentNumberOfRecords);
    gCurrentNumberOfRecords++;
    return true;
}


void format()
{
    zeroInternalRecord(0);
    zeroInternalRecord(1);
}

    
uint32_t maximumNumberOfRecords()
{
    return gMaximumNumberOfRecords;
}

    
uint32_t currentNumberOfRecords()
{
    return gCurrentNumberOfRecords;
}


}
}






