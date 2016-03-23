#include <cstdint>
#include <cassert>
#include <Windows.h>

// longui::implnamespace
namespace LongUI { namespace impl {

}}

// longui namespace
namespace LongUI {
    // Base64 DataChar: Map 0~63 to visible char
    static const char Base64Chars[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    // Base64 DataChar: Map visible char to 0~63
    static const uint8_t Base64Datas[128] = {
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
            *base64_index = Base64Chars[current]; ++base64_index;
            // 1
            current = (static_cast<uint8_t>((bindata)[i] << 4)) & (static_cast<uint8_t>(0x30));
            if (i + 1 >= binlen) {
                *base64_index = Base64Chars[current]; ++base64_index;
                *base64_index = '='; ++base64_index;
                *base64_index = '='; ++base64_index;
                break;
            }
            current |= (static_cast<uint8_t>((bindata)[i + 1] >> 4)) & (static_cast<uint8_t>(0x0F));
            *base64_index = Base64Chars[current]; ++base64_index;
            // 2
            current = (static_cast<uint8_t>((bindata)[i + 1] << 2)) & (static_cast<uint8_t>(0x3C));
            if (i + 2 >= binlen) {
                *base64_index = Base64Chars[current]; ++base64_index;
                *base64_index = '='; ++base64_index;
                break;
            }
            current |= (static_cast<uint8_t>((bindata)[i + 2] >> 6)) & (static_cast<uint8_t>(0x03));
            *base64_index = Base64Chars[current]; ++base64_index;
            // 3
            current = (static_cast<uint8_t>(bindata[i + 2])) & (static_cast<uint8_t>(0x3F));
            *base64_index = Base64Chars[current]; ++base64_index;
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
            uint8_t a = Base64Datas[base64[0]];
            uint8_t b = Base64Datas[base64[1]];
            uint8_t c = Base64Datas[base64[2]];
            uint8_t d = Base64Datas[base64[3]];
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
    // 获取u16转换u8后u8所占长度(含0结尾符)
    auto UTF16toUTF8GetBufLen(const char16_t * src) noexcept -> uint32_t {
        static_assert(sizeof(char16_t) == sizeof(wchar_t), "bad action");
        const wchar_t* src2 = reinterpret_cast<const wchar_t*>(src);
        int result = ::WideCharToMultiByte(CP_UTF8, 0, src2, -1, nullptr, 0, nullptr, nullptr);
        return uint32_t(result);
    }
    // 获取u8转换u16后u16所占长度(含0结尾符)
    auto UTF8toUTF16GetBufLen(const char* src) noexcept -> uint32_t {
#ifdef _DEBUG
        auto result = ::MultiByteToWideChar(CP_UTF8, 0, src, -1, nullptr, 0);
#endif
        uint32_t length = 1;
        // 遍历字符串
        while (*src) {
            ++length;
            if ((*src & 0x80) == 0x0) {
                // 1 byte
                src += 1;
            }
            else if ((*src & 0xE0) == 0xC0) {
                // 2 bytes
                src += 2;
            }
            else if ((*src & 0xE0) == 0xE0) {
                // 3 bytes
                src += 3;
            }
        }
#ifdef _DEBUG
        assert(length == result && "bad length for buffer");
#endif
        return length;
    }
    // UTF16字符串 转 UTF8字符串
    auto UTF16toUTF8(
        const char16_t * __restrict src, 
        char * __restrict des, 
        uint32_t buflen
    ) noexcept -> uint32_t {
        assert(!"NOIMPL");
#ifdef _DEBUG
        auto len = LongUI::UTF16toUTF8GetBufLen(src);
        assert(buflen >= len && "buffer too small");
#endif
        return 0;
    }
    //  UTF8字符串 转 UTF16字符串
    auto UTF8toUTF16(
        const char * __restrict src, 
        char16_t * __restrict des, 
        uint32_t buflen) noexcept -> uint32_t {
        assert(!"NOIMPL");
        return uint32_t();
    }
}
