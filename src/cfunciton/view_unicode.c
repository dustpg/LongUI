#include "assert.h"
#include "stdint.h"
// C99 support
#if defined(_MSC_VER) && (_MSC_VER < 1900)
typedef int16_t char16_t;
typedef int32_t char32_t;
#define inline __inline
#else
#include "uchar.h"
#endif

#include <util/ui_unimacro.h>

// 0xD800 <= ch <= 0xDFFF
#define is_surrogate(ch) (((ch) & 0xF800) == 0xD800)

// 0xD800 <= ch <= 0xDBFF
#define is_high_surrogate(ch) (((ch) & 0xFC00) == 0xD800)

// 0xDC00 <= ch <= 0xDFFF
#define is_low_surrogate(ch) (((ch) & 0xFC00) == 0xDC00)

// UTF-8 字节长度 [5, 6已被弃用]
static const char ui_bytes_for_utf8[256] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,

    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,5,5,5,5,6,6,6,6
};

// mark for first byte
static const char32_t ui_first_byte_mark[7] = {
    0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC
};

// offset
static const char32_t ui_offsets_from_utf8[6] = {
    0x00000000UL, 0x00003080UL, 0x000E2080UL,
    0x03C82080UL, 0xFA082080UL, 0x82082080UL
};

/// <summary>
/// Implementations the char16x2 to char32.
/// </summary>
/// <param name="lead">The lead.</param>
/// <param name="trail">The trail.</param>
/// <returns></returns>
extern inline char32_t impl_char16x2_to_char32(char16_t lead, char16_t trail) {
    assert(is_high_surrogate(lead) && "illegal utf-16 char");
    assert(is_low_surrogate(trail) && "illegal utf-16 char");
    return (char32_t)((lead - 0xD800) << 10 | (trail - 0xDC00)) + (0x10000);
};

/// <summary>
/// Char32s to char16.
/// </summary>
/// <param name="ch">The ch.</param>
/// <param name="str">The string.</param>
/// <returns></returns>
static inline char16_t* impl_char32_to_char16(char32_t ch, char16_t* str) {
    assert(str && "bad argment");
    // 检查是否需要转换
    if (ch > 0xFFFF) {
        str[0] = (char16_t)(0xD800 + (ch >> 10) - (0x10000 >> 10));
        str[1] = (char16_t)(0xDC00 + (ch & 0x3FF));
        return str + 2;
    }
    else {
        str[0] = (char16_t)ch;
        return str + 1;
    }
}

PCN_NOINLINE
/// <summary>
/// UIs the UTF16 to UTF8 get buflen.
/// 获取u16转换u8后u8所占长度(不含0结尾符)
/// </summary>
/// <param name="src">The source.</param>
/// <returns></returns>
uint32_t ui_utf16_to_utf8_get_buflen(const char16_t* src, const char16_t* end) {
    assert(src && end && end >= src && "bad string");
    uint32_t length = 0;
    // 遍历字符串
    for (ptrdiff_t i = 0; i != (end - src); ++i) {
        char16_t ch = src[i];
        // 1字节区域 [0-7F]
        if (ch < 0x0080) {
            length += 1;
        }
        // 四字节区
        else if (is_high_surrogate(ch)) {
            assert(is_low_surrogate(src[i+1]) && "illegal utf-16 string");
            ++i;
            length += 4;
        }
        // 3字节区域 > 2^11
        else if (ch > 0x07FF) {
            length += 3;
        }
        // 2字节区域 [80-7FF]
        else /*if (ch < 0x0800) */ {
            length += 2;
        }
    }
    return length;
}

PCN_NOINLINE
/// <summary>
/// UIs the UTF16 to UTF32 get buflen.
/// 获取u16转换u32后u32所占长度(不含0结尾符)
/// </summary>
/// <param name="src">The source.</param>
/// <returns></returns>
uint32_t ui_utf16_to_utf32_get_buflen(const char16_t* src, const char16_t* end) {
    assert(src && end && end >= src && "bad string");
    uint32_t length = 0;
    // 遍历字符串
    for (ptrdiff_t i = 0; i < (end - src); ++i) {
        char16_t ch = src[i]; ++length;
        // 双字区
        if (is_high_surrogate(ch)) {
            assert(is_low_surrogate(src[i+1]) && "illegal utf-16 string");
            ++i;
            assert((end - src) != i && "end of string");
        }
    }
    return length;
}


PCN_NOINLINE
/// <summary>
/// UIs the UTF16 to UTF8 get buflen.
/// </summary>
/// <param name="src">The source.</param>
/// <returns></returns>
uint32_t ui_utf8_to_utf16_get_buflen(const char* src, const char* end) {
    uint32_t length = 0;
    // 遍历字符串
    while (src < end) {
        unsigned char ch = *src;
        const char blen = ui_bytes_for_utf8[ch];
        assert(blen > 0 && blen < 5 && "illegal utf-8 @ RFC 3629");
        src += blen;
        length += 1 + (blen >> 2);
    }
    assert(src == end && "end of string!");
    return length;
}

PCN_NOINLINE
/// <summary>
/// UIs the UTF16 to UTF8 get buflen.
/// </summary>
/// <param name="src">The source.</param>
/// <returns></returns>
uint32_t ui_utf8_to_utf32_get_buflen(const char* src, const char* end) {
    uint32_t length = 0;
    // 遍历字符串
    while (src < end) {
        unsigned char ch = *src;
        const char blen = ui_bytes_for_utf8[ch];
        assert(blen > 0 && blen < 5 && "illegal utf-8 @ RFC 3629");
        src += blen;
        length += 1;
    }
    assert(src == end && "end of string!");
    return length;
}


PCN_NOINLINE
/// <summary>
/// UIs the UTF16 to UTF32 get buflen.
/// 将u16转换成u32
/// </summary>
/// <param name="src">The source.</param>
/// <returns></returns>
uint32_t ui_utf16_to_utf32(
    char32_t* __restrict buf,
    uint32_t buflen,
    const char16_t* __restrict src,
    const char16_t* end) {
#ifndef NDEBUG
    const uint32_t needed = ui_utf16_to_utf32_get_buflen(src, end);
    assert(buflen >= needed && "buffer too small");
#endif
    assert(src && end && end >= src && "bad string");
    uint32_t bufrm = buflen;
    char32_t* __restrict des = buf;
    // 遍历字符串
    for (ptrdiff_t i = 0; i < (end - src); ++i) {
        if (!bufrm) break; --bufrm;
        char16_t ch = src[i];
        // 双字区
        if (is_high_surrogate(ch)) {
            const char16_t ch2 = src[i + 1];
            assert(is_low_surrogate(ch2) && "illegal utf-16 string");
            *des = impl_char16x2_to_char32(ch, ch2);
            ++i;
            assert((end - src) != i && "end of string");
        }
        else *des = (char32_t)ch;
        ++des;
    }
    // 收尾检查
    assert((uint32_t)(des - buf) == needed && "bug");
    return (uint32_t)(des - buf);
}

PCN_NOINLINE
/// <summary>
/// UIs the UTF16 to UTF8.
/// </summary>
/// <param name="buf">The buf.</param>
/// <param name="buflen">The buflen.</param>
/// <param name="src">The source.</param>
/// <param name="end">The end.</param>
/// <returns></returns>
uint32_t ui_utf16_to_utf8(
    char* __restrict buf,
    uint32_t buflen,
    const char16_t* __restrict src,
    const char16_t* end) {
    assert(buf && "bad buffer");
    assert(src && end >= end && "bad string");
#ifndef NDEBUG
    const uint32_t needed = ui_utf16_to_utf8_get_buflen(src, end);
    assert(buflen >= needed && "buffer too small");
#endif
    uint32_t bufrm = buflen;
    char* __restrict des = buf;
    // 遍历
    while (src < end) {
        // 初始数据
        char32_t ch = 0;
        uint32_t move = 0;
        // 辅助平面
        if (((*src) & 0xD800) == 0xD800) {
            ch = impl_char16x2_to_char32(src[0], src[1]);
            assert(ch > 0xFFFF && ch <= 0x10FFFF && "bad utf32 char");
            // UTF-16 x2 --- UTF-8  x4
            src += 2;
            move = 4;
        }
        // 基本多语言平面
        else {
            ch = (char32_t)(*src);
            src += 1;
            // 单字节
            if (ch < (char32_t)(0x80)) {
                move = 1;
            }
            // 双字节
            else if (ch < (char32_t)(0x0800)) {
                move = 2;
            }
            // 三字节
            else {
                move = 3;
            }
        }
        // [RT]缓存不足
        if (bufrm < move) break;
        bufrm -= move;
        des += move;
        // 掩码
#define byteMask ((char32_t)0xBF)
#define byteMark ((char32_t)0x80)
        // 转换
        switch (move) {
        case 4: *--des = (char)((ch | byteMark) & byteMask); ch >>= 6;
        case 3: *--des = (char)((ch | byteMark) & byteMask); ch >>= 6;
        case 2: *--des = (char)((ch | byteMark) & byteMask); ch >>= 6;
        case 1: *--des = (char)(ch | ui_first_byte_mark[move]);
        }
        des += move;
    }
    // 收尾检查
    assert((uint32_t)(des - buf) == needed && "bug");
    return (uint32_t)(des - buf);
}

PCN_NOINLINE
/// <summary>
///  the UTF8 to UTF16.
///  UTF8字符串 转 UTF16字符串
/// </summary>
/// <param name="buf">The buf.</param>
/// <param name="buflen">The buflen.</param>
/// <param name="src">The source.</param>
/// <param name="end">The end.</param>
/// <returns></returns>
uint32_t ui_utf8_to_utf16(
    char16_t* __restrict buf,
    uint32_t buflen,
    const char* __restrict src,
    const char* end) {
#ifndef NDEBUG
    const uint32_t needed = ui_utf8_to_utf16_get_buflen(src, end);
    assert(buflen >= needed && "buffer too small");
#endif
    uint32_t bufremain = buflen;
    char16_t* __restrict des = buf;
    // 遍历字符串
    while (src < end) {
        // 初始数据
        char32_t ch = 0;
        const unsigned char read = ui_bytes_for_utf8[(unsigned char)(*src)] - 1;
        // 读取
        switch (read)
        {
#ifndef NDEBUG
        default:assert(!"bug"); break;
        case 5:
        case 4: assert(!"illegal utf-8 @ RFC 3629");
#endif
        case 3: ch += (unsigned char)(*src++); ch <<= 6;
        case 2: ch += (unsigned char)(*src++); ch <<= 6;
        case 1: ch += (unsigned char)(*src++); ch <<= 6;
        case 0: ch += (unsigned char)(*src++);
        }
        // 减去偏移量
        ch -= ui_offsets_from_utf8[read];
        // 检查缓存是否富裕
        const uint32_t u16len = ch > 0xFFFF ? 2 : 1;
        if (bufremain < u16len) break;
        bufremain -= u16len;
        // 写入
        des = impl_char32_to_char16(ch, des);
    }
    // 收尾检查
#ifndef NDEBUG
    if (buflen >= needed) {
        assert((uint32_t)(des - buf) == needed && "bug");
    }
#endif
    return (uint32_t)(des - buf);
}


PCN_NOINLINE
/// <summary>
///  the UTF8 to UTF16.
///  UTF8字符串 转 UTF32字符串
/// </summary>
/// <param name="buf">The buf.</param>
/// <param name="buflen">The buflen.</param>
/// <param name="src">The source.</param>
/// <param name="end">The end.</param>
/// <returns></returns>
uint32_t ui_utf8_to_utf32(
    char32_t* __restrict buf,
    uint32_t buflen,
    const char* __restrict src,
    const char* end) {
#ifndef NDEBUG
    const uint32_t needed = ui_utf8_to_utf32_get_buflen(src, end);
    assert(buflen >= needed && "buffer too small");
#endif
    uint32_t bufremain = buflen;
    char32_t* __restrict des = buf;
    // 遍历字符串
    while (src < end) {
        // 初始数据
        char32_t ch = 0;
        const unsigned char read = ui_bytes_for_utf8[(unsigned char)(*src)] - 1;
        // 读取
        switch (read)
        {
#ifndef NDEBUG
        default:assert(!"bug"); break;
        case 5:
        case 4: assert(!"illegal utf-8 @ RFC 3629");
#endif
        case 3: ch += (unsigned char)(*src++); ch <<= 6;
        case 2: ch += (unsigned char)(*src++); ch <<= 6;
        case 1: ch += (unsigned char)(*src++); ch <<= 6;
        case 0: ch += (unsigned char)(*src++);
        }
        // 减去偏移量
        ch -= ui_offsets_from_utf8[read];
        // 检查缓存是否富裕
        if (!bufremain) break; --bufremain;
        // 写入
        des[0] = ch; des++;
    }
    // 收尾检查
#ifndef NDEBUG
    if (buflen >= needed) {
        assert((uint32_t)(des - buf) == needed && "bug");
    }
#endif
    return (uint32_t)(des - buf);
}
