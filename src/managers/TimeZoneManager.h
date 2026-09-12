#pragma once

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <time.h>

class TimeZoneManager
{
public:

    static constexpr size_t MAX_ZONES = 256;
    static constexpr size_t MAX_TRANSITIONS = 1024;

    TimeZoneManager();

    bool begin();

    bool setZone(const char* zone);

    const String& getZone() const;

    const String& getDatabaseVersion() const;

    bool isLoaded() const;

    size_t getTransitionCount() const;

    int32_t getUTCOffset(time_t utc) const;

    bool isDST(time_t utc) const;

    bool getLocalTime(
        time_t utc,
        struct tm& result
    ) const;

private:

    // ========================================================
    // TZDB
    // ========================================================

    static constexpr const char* TZDB_PATH =
        "/timezone/zones.tzdb";

    static constexpr uint16_t TZDB_VERSION = 1;

    static constexpr size_t ZONE_NAME_SIZE = 48;

    struct ZoneEntry
    {
        char name[ZONE_NAME_SIZE];

        uint32_t offset;
        uint32_t size;
    };

    ZoneEntry _zones[MAX_ZONES];

    uint32_t _zoneCount;

    uint32_t _dataOffset;

    String _databaseVersion;

    String _zone;

    // ========================================================
    // TZIF
    // ========================================================

    struct Transition
    {
        int64_t timestamp;
        int32_t offset;
        bool isDst;
    };

    Transition _transitions[MAX_TRANSITIONS];

    size_t _transitionCount;

    int32_t _defaultOffset;

    bool _defaultIsDst;

    bool _initialized;

    bool _loaded;

    // ========================================================
    // POSIX footer
    // ========================================================

    enum class RuleType : uint8_t
    {
        NONE,
        MONTH_WEEK_DAY,
        JULIAN_NO_LEAP,
        DAY_OF_YEAR
    };

    struct PosixRule
    {
        RuleType type;

        uint8_t month;
        uint8_t week;
        uint8_t weekday;

        uint16_t day;

        int32_t seconds;

        bool valid;

        PosixRule()
            : type(RuleType::NONE),
              month(0),
              week(0),
              weekday(0),
              day(0),
              seconds(0),
              valid(false)
        {
        }
    };

    struct PosixInfo
    {
        bool valid;

        int32_t standardOffset;
        int32_t daylightOffset;

        PosixRule startRule;
        PosixRule endRule;

        PosixInfo()
            : valid(false),
              standardOffset(0),
              daylightOffset(0)
        {
        }
    };

    PosixInfo _posix;

    // ========================================================
    // Database
    // ========================================================

    bool loadDatabase();

    bool readZoneIndex(
        File& file
    );

    bool findZone(
        const char* zone,
        ZoneEntry& result
    ) const;

    bool loadZone(
        const char* zone
    );

    // ========================================================
    // TZif
    // ========================================================

    bool parseTZif(
        File& file,
        uint32_t offset,
        uint32_t size
    );

    bool parseHeader(
        File& file,
        uint8_t& version,
        uint32_t& ttisgmtcnt,
        uint32_t& ttisstdcnt,
        uint32_t& leapcnt,
        uint32_t& timecnt,
        uint32_t& typecnt,
        uint32_t& charcnt
    );

    bool parseDataBlock(
        File& file,
        uint32_t ttisgmtcnt,
        uint32_t ttisstdcnt,
        uint32_t leapcnt,
        uint32_t timecnt,
        uint32_t typecnt,
        uint32_t charcnt,
        bool is64bit
    );

    bool parseFooter(
        File& file
    );

    // ========================================================
    // POSIX
    // ========================================================

    bool parsePosixTZ(
        const char* tz
    );

    bool parseOffset(
        const char*& ptr,
        int32_t& seconds
    );

    bool parseRule(
        const char*& ptr,
        PosixRule& rule
    );

    bool parseRuleTime(
        const char*& ptr,
        int32_t& seconds
    );

    int32_t getPosixOffset(
        time_t utc,
        bool& dst
    ) const;

    bool getPosixTransitions(
        int year,
        time_t& startUtc,
        time_t& endUtc
    ) const;

    // ========================================================
    // Offset
    // ========================================================

    int32_t findOffset(
        time_t utc,
        bool& dst
    ) const;

    int32_t findOffsetFromTransitions(
        time_t utc,
        bool& dst
    ) const;

    // ========================================================
    // Calendar
    // ========================================================

    static bool isLeapYear(
        int year
    );

    static int daysInMonth(
        int year,
        int month
    );

    static int dayOfWeek(
        int year,
        int month,
        int day
    );

    static int dayOfYear(
        int year,
        int month,
        int day
    );

    static time_t makeUTC(
        int year,
        int month,
        int day,
        int hour,
        int minute,
        int second
    );

    static int getYearUTC(
        time_t timestamp
    );

    // ========================================================
    // Binary
    // ========================================================

    static bool readBE32(
        File& file,
        uint32_t& value
    );

    static bool readBE64(
        File& file,
        uint64_t& value
    );

    static bool skipBytes(
        File& file,
        uint64_t count
    );

    static bool readByte(
        File& file,
        uint8_t& value
    );

    void clear();
};