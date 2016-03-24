#include <cstdint>
#include <cassert>
#include <Windows.h>

// longui::implnamespace
namespace LongUI { namespace impl {
    // char32 to char 16
    inline auto char32_to_char16(char32_t ch, char16_t* str) noexcept -> char16_t* {
        assert(str && "bad argment");
        // 检查是否需要转换
        if (ch > 0xFFFF) {
            str[0] = char16_t(0xD800 + (ch >> 10) - (0x10000 >> 10));
            str[1] = char16_t(0xDC00 + (ch & 0x3FF));
            return str + 2;
        }
        else {
            str[0] = char16_t(ch);
            return str + 1;
        }
    }
    // 2x char16 to char32
    inline auto char16x2_to_char32(char16_t lead, char16_t trail) noexcept -> char32_t {
        assert((lead & 0xD800) == 0xD800 && "illegal utf-16 char");
        assert((trail & 0xDC00) == 0xDC00 && "illegal utf-16 char");
        return char32_t((lead-0xD800) << 10 | (trail-0xDC00)) + (0x10000);
    };
}}

// longui namespace
namespace LongUI {
    // Base64 DataChar: Map 0~63 to visible char
    static const char BASE64_CHARS[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    // Base64 DataChar: Map visible char to 0~63
    static const uint8_t BASE64_DATA[128] = {
        // [  0, 16)
        0, 0, 0, 0,   0, 0 ,0, 0,       0, 0, 0, 0, 0, 0, 0, 0,
        // [ 16, 32)
        0, 0, 0, 0,   0, 0 ,0, 0,       0, 0, 0, 0, 0, 0, 0, 0,
        // [ 32, 48)                            43 44 45 46 47
        0, 0, 0, 0,   0, 0 ,0, 0,       0, 0, 0,62, 0, 0, 0,64,
        // [ 48, 64)
        52,53,54,55, 56,57,58,59,      60,61, 0, 0, 0, 0, 0, 0,
        // [ 64, 80)
        0, 0, 1, 2,   3, 4, 5, 6,       7, 8, 9,10,11,12,13,14,
        // [ 80, 96)
        15,16,17,18, 19,20,21,22,      23,24,25, 0, 0, 0, 0, 0,
        // [ 96,112)
        0,26,27,28,  29,30,31,32,      33,34,35,36,37,38,39,40,
        // [112,128)
        41,42,43,44, 45,46,47,48,      49,50,51, 0, 0, 0, 0, 0,
    };
    // UTF-8 字节长度 [5, 6已被弃用]
    static const char BYTES_FOR_UTF8[256] = {
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,5,5,5,5,6,6,6,6
    };
    // 偏移值
    static const char32_t OFFSETS_FROM_UTF8[6] = { 
        0x00000000UL, 0x00003080UL, 0x000E2080UL, 
        0x03C82080UL, 0xFA082080UL, 0x82082080UL 
    };
    // mark for first byte
    static const char32_t FIRST_BYTE_MARK[7] = { 
        0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC 
    };
    // Base64 Encode 编码
    auto Base64Encode(
        const uint8_t*  __restrict  bindata,
        size_t                      binlen,
        char* const     __restrict  base64
    ) noexcept -> char * {
        uint8_t current;
        auto base64_index = base64;
        // 遍历
        for (size_t i = 0; i < binlen; i += 3) {
            current = (bindata[i] >> 2);
            current &= static_cast<uint8_t>(0x3F);
            *base64_index = BASE64_CHARS[current]; ++base64_index;
            // 1
            current = (static_cast<uint8_t>((bindata)[i] << 4)) & (static_cast<uint8_t>(0x30));
            if (i + 1 >= binlen) {
                *base64_index = BASE64_CHARS[current]; ++base64_index;
                *base64_index = '='; ++base64_index;
                *base64_index = '='; ++base64_index;
                break;
            }
            current |= (static_cast<uint8_t>((bindata)[i + 1] >> 4)) & (static_cast<uint8_t>(0x0F));
            *base64_index = BASE64_CHARS[current]; ++base64_index;
            // 2
            current = (static_cast<uint8_t>((bindata)[i + 1] << 2)) & (static_cast<uint8_t>(0x3C));
            if (i + 2 >= binlen) {
                *base64_index = BASE64_CHARS[current]; ++base64_index;
                *base64_index = '='; ++base64_index;
                break;
            }
            current |= (static_cast<uint8_t>((bindata)[i + 2] >> 6)) & (static_cast<uint8_t>(0x03));
            *base64_index = BASE64_CHARS[current]; ++base64_index;
            // 3
            current = (static_cast<uint8_t>(bindata[i + 2])) & (static_cast<uint8_t>(0x3F));
            *base64_index = BASE64_CHARS[current]; ++base64_index;
        }
        *base64_index = 0;
        return base64;
    }
    // Base64 Decode 解码
    auto Base64Decode(
const char*     __restrict  base64,
uint8_t*        __restrict  bindata
) noexcept->size_t {
// 二进制长度
uint8_t* bindata_index = bindata;
// 主循环
while (*base64) {
    // 基本转换
    uint8_t a = BASE64_DATA[base64[0]];
    uint8_t b = BASE64_DATA[base64[1]];
    uint8_t c = BASE64_DATA[base64[2]];
    uint8_t d = BASE64_DATA[base64[3]];
    // 第一个二进制数据
    *bindata_index = ((a << 2) & 0xFCui8) | ((b >> 4) & 0x03ui8);
    ++bindata_index;
    if (base64[2] == '=') break;
    // 第三个二进制数据
    *bindata_index = ((b << 4) & 0xF0ui8) | ((c >> 2) & 0x0Fui8);
    ++bindata_index;
    if (base64[3] == '=') break;
    // 第三个二进制数据
    *bindata_index = ((c << 6) & 0xF0ui8) | ((d >> 0) & 0x3Fui8);
    ++bindata_index;
    base64 += 4;
}
return bindata_index - bindata;
    }
    // char32 转换 char16
    auto Char32toChar16(char32_t ch, char16_t* str) -> char16_t* {
        return impl::char32_to_char16(ch, str);
    }
    // 获取u16转换u8后u8所占长度(含0结尾符)
    auto UTF16toUTF8GetBufLen(const char16_t * src) noexcept -> uint32_t {
        static_assert(sizeof(char16_t) == sizeof(wchar_t), "bad action");
        const wchar_t* src2 = reinterpret_cast<const wchar_t*>(src);
#ifdef _DEBUG
        int result = ::WideCharToMultiByte(CP_UTF8, 0, src2, -1, nullptr, 0, nullptr, nullptr);
#endif
        uint32_t length = 1;
        char16_t ch = 0;
        // 遍历字符串
        while ((ch = *src)) {
            src += 1;
            // 1字节区域 [0-7F]
            if (ch < 0x0080) {
                length += 1;
            }
            // 2字节区域 [80-7FF]
            else if (ch < 0x0800) {
                length += 2;
            }
            // 四字节区
            else if ((ch & 0xD800) == 0xD800) {
                assert((src[0] & 0xDC00) == 0xDC00 && "illegal utf-8");
                src += 1;
                length += 4;
            }
            // 3字节区域 > 2^11
            else {
                length += 3;
            }
        }
#ifdef _DEBUG
        assert(length == result && "bad length for buffer");
#endif
        return length;
    }
    // 获取u8转换u16后u16所占长度(含0结尾符)
    auto UTF8toUTF16GetBufLen(const char* src) noexcept -> uint32_t {
#ifdef _DEBUG
        auto result = ::MultiByteToWideChar(CP_UTF8, 0, src, -1, nullptr, 0);
#endif
        uint32_t length = 1;
        unsigned char ch = 0;
        // 遍历字符串
        while ((ch = *src)) {
            auto blen = BYTES_FOR_UTF8[ch];
            assert(blen > 0 && blen < 5 && "illegal utf-8");
            src += blen;
            length += 1 + (blen >> 2);
        }
#ifdef _DEBUG
        assert(length == result && "bad length for buffer");
#endif
        return length;
    }
    // UTF16字符串 转 UTF8字符串
    auto UTF16toUTF8(
        const char16_t* __restrict src,
        char * __restrict des,
        uint32_t buflen
    ) noexcept -> char* {
#ifdef _DEBUG
        const auto olddes = des;
        auto len = LongUI::UTF16toUTF8GetBufLen(src);
        assert(buflen >= len && "buffer too small");
#endif
        // 遍历
        while (*src) {
            char32_t ch = 0;
            uint32_t move = 0;
            // 辅助平面
            if (((*src) & 0xD800) == 0xD800) {
                ch = impl::char16x2_to_char32(src[0], src[1]);
                assert(ch > 0xFFFF && ch <= 0x10FFFF && "bad utf32 char");
                // UTF-16 x2 --- UTF-8  x4
                src += 2;
                move = 4;
            }
            // 基本多语言平面
            else {
                ch = char32_t(*src);
                src += 1;
                // 单字节
                if (ch < char32_t(0x80)) {
                    move = 1;
                }
                // 双字节
                else if (ch < char32_t(0x0800)) {
                    move = 2;
                }
                // 三字节
                else {
                    move = 3;
                }
            }
            // 掩码
            constexpr char32_t byteMask = 0xBF;
            constexpr char32_t byteMark = 0x80;
            des += move;
            // 转换
            switch (move) {
            case 4: *--des = (char)((ch | byteMark) & byteMask); ch >>= 6;
            case 3: *--des = (char)((ch | byteMark) & byteMask); ch >>= 6;
            case 2: *--des = (char)((ch | byteMark) & byteMask); ch >>= 6;
            case 1: *--des = (char)(ch | FIRST_BYTE_MARK[move]);
            }
            des += move;
        }
        // 收尾检查
#ifdef _DEBUG
        auto utf16len1 = size_t(len - 1);
        auto utf16len2 = size_t(des - olddes);
        assert((utf16len1 == utf16len2) && "bug!");
#endif
        return des;
    }
    //  UTF8字符串 转 UTF16字符串
    auto UTF8toUTF16(
        const char * __restrict src, 
        char16_t * __restrict des, 
        uint32_t buflen) noexcept -> char16_t* {
#ifdef _DEBUG
        const auto olddes = des;
        auto len = LongUI::UTF8toUTF16GetBufLen(src);
        assert(buflen >= len && "buffer too small");
#else
        UNREFERENCED_PARAMETER(buflen);
#endif
        // 遍历字符串
        while (*src) {
            char32_t ch = 0;
            unsigned char read = BYTES_FOR_UTF8[(unsigned char)(*src)] - 1;
            // 读取
            switch (read)
            {
#ifdef _DEBUG
            default:assert(!"bug"); break;
            case 5: 
            case 4: assert(!"illegal utf-8");
#endif
            case 3: ch += (unsigned char)(*src++); ch <<= 6;
            case 2: ch += (unsigned char)(*src++); ch <<= 6;
            case 1: ch += (unsigned char)(*src++); ch <<= 6;
            case 0: ch += (unsigned char)(*src++);
            }
            // 减去偏移量
            ch -= OFFSETS_FROM_UTF8[read];
            // 写入
            des = impl::char32_to_char16(ch, des);
        }
        // 收尾检查
#ifdef _DEBUG
        auto utf16len1 = size_t(len - 1);
        auto utf16len2 = size_t(des - olddes);
        assert((utf16len1 == utf16len2) && "bug!");
#endif
        return des;
    }
}
