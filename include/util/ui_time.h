#pragma once
#include <cstdint>
#include "../util/ui_unimacro.h"

namespace LongUI {
    // system time
    struct SystemTime;
    // file time
    struct FileTime {
        // time
        uint64_t        time;
        // to system time
        bool ToSystemTime(SystemTime& time) const noexcept;
    };
    // system time
    struct SystemTime {
        // from system time
        static void FromSystemTime(SystemTime& st) noexcept;
        // from system time
        static auto FromSystemTime() noexcept ->SystemTime {
            SystemTime st; FromSystemTime(st); return st;
        }
        // year
        uint16_t        year;
        // month
        uint16_t        month;
        // day of week
        uint16_t        day_of_week;
        // day
        uint16_t        day;
        // hour
        uint16_t        wHour;
        // minute
        uint16_t        minute;
        // second
        uint16_t        second;
        // ms
        uint16_t        milliseconds;
        // to file time
        bool ToFileTime(FileTime&) const noexcept;
    };
}