#pragma once

#include <cstdint>

// ui
namespace LongUI {
    // type healper
    namespace type_helper {
        // base int type
        template<size_t> struct int_type;
        // 1 byte
        template<> struct int_type<sizeof(uint8_t)> {
            // signed 1 byte int
            using signed_t = int8_t;
            // unsigned 1 byte int
            using unsigned_t = uint8_t;
        };
        // 2 byte
        template<> struct int_type<sizeof(uint16_t)> {
            // signed 2 byte int
            using signed_t = int16_t;
            // unsigned 2 byte int
            using unsigned_t = uint16_t;
        };
        // 4 byte
        template<> struct int_type<sizeof(uint32_t)> {
            // signed 4 byte int
            using signed_t = int32_t;
            // unsigned 4 byte int
            using unsigned_t = uint32_t;
        };
        // 8 byte
        template<> struct int_type<sizeof(uint64_t)> {
            // signed 8 byte int
            using signed_t = int64_t;
            // unsigned 8 byte int
            using unsigned_t = uint64_t;
        };
    }
    // half ptr
    using halfptr_t = typename type_helper::int_type<sizeof(void*) / 2>::signed_t;
    // unsigned half ptr
    using uhalfptr_t = typename type_helper::int_type<sizeof(void*) / 2>::unsigned_t;
    // i8
    inline constexpr int32_t operator ""_i8(unsigned long long i) noexcept {
        return static_cast<int8_t>(i);
    }
    // ui8
    inline constexpr uint8_t operator ""_ui8(unsigned long long i) noexcept {
        return static_cast<uint8_t>(i);
    }
    // i16
    inline constexpr int16_t operator ""_i16(unsigned long long i) noexcept {
        return static_cast<int16_t>(i);
    }
    // ui16
    inline constexpr uint16_t operator ""_ui16(unsigned long long i) noexcept {
        return static_cast<uint16_t>(i);
    }
    // i32 
    inline constexpr int32_t operator ""_i32(unsigned long long i) noexcept {
        return static_cast<int32_t>(i);
    }
    // ui32 
    inline constexpr uint32_t operator ""_ui32(unsigned long long i) noexcept {
        return static_cast<uint32_t>(i);
    }
    // i64
    inline constexpr int64_t operator ""_i64(unsigned long long i) noexcept {
        return static_cast<int64_t>(i);
    }
    // ui64
    inline constexpr uint64_t operator ""_ui64(unsigned long long i) noexcept {
        return static_cast<uint64_t>(i);
    }
}