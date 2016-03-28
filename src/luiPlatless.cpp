#include "luibase.h"
#include "luiconf.h"
#include "Platless/luiPlUtil.h"
#include "Platless/luiPlHlper.h"

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
        assert(IsHighSurrogate(lead) && "illegal utf-16 char");
        assert(IsLowSurrogate(trail) && "illegal utf-16 char");
        return char32_t((lead-0xD800) << 10 | (trail-0xDC00)) + (0x10000);
    };
    // 反弹渐出
    auto inline bounce_ease_out(float p) noexcept ->float {
        if (p < 4.f / 11.f) {
            return (121.f * p * p) / 16.f;
        }
        else if (p < 8.f / 11.f) {
            return (363.f / 40.f * p * p) - (99.f / 10.f * p) + 17.f / 5.f;
        }
        else if (p < 9.f / 10.f) {
            return (4356.f / 361.f * p * p) - (35442.f / 1805.f * p) + 16061.f / 1805.f;
        }
        else {
            return (54.f / 5.f * p * p) - (513.f / 25.f * p) + 268.f / 25.f;
        }
    }
    // 字符串转数字
    template<typename T> auto atoi(const T* str) noexcept ->int {
        assert(str && "bad argument");
        // 初始化
        bool negative = false; int value = 0; T ch = 0;
        // 遍历
        while (ch = *str) {
            // 空白?
            if (!white_space(ch)) {
                if (ch == '-') {
                    negative = true;
                }
                else if (valid_digit(ch)) {
                    value *= 10;
                    value += ch - static_cast<T>('0');
                }
                else {
                    break;
                }
            }
            ++str;
        }
        // 负数
        if (negative) {
            value = -value;
        }
        return value;
    }
    // 字符串转浮点
    template<typename T> auto atof(const T* p) noexcept ->float {
        assert(p && "bad argument");
        bool negative = false;
        float value, scale;
        // 跳过空白
        while (white_space(*p)) ++p;
        // 检查符号
        if (*p == '-') {
            negative = true;
            ++p;
        }
        else if (*p == '+') {
            ++p;
        }
        // 获取小数点或者指数之前的数字(有的话)
        for (value = 0.0f; valid_digit(*p); ++p) {
            value = value * 10.0f + static_cast<float>(*p - static_cast<T>('0'));
        }
        // 获取小数点或者指数之后的数字(有的话)
        if (*p == '.') {
            float pow10 = 10.0f; ++p;
            while (valid_digit(*p)) {
                value += (*p - static_cast<T>('0')) / pow10;
                pow10 *= 10.0f;
                ++p;
            }
        }
        // 处理指数(有的话)
        bool frac = false;
        scale = 1.0f;
        if ((*p == 'e') || (*p == 'E')) {
            // 获取指数的符号(有的话)
            ++p;
            if (*p == '-') {
                frac = true;
                ++p;
            }
            else if (*p == '+') {
                ++p;
            }
            unsigned int expon;
            // 获取指数的数字(有的话)
            for (expon = 0; valid_digit(*p); ++p) {
                expon = expon * 10 + (*p - static_cast<T>('0'));
            }
            // float 最大38 double 最大308
            if (expon > 38) expon = 38;
            // 计算比例因数
            while (expon >= 8) { scale *= 1E8f;  expon -= 8; }
            while (expon) { scale *= 10.0f; --expon; }
        }
        // 返回
        float returncoude = (frac ? (value / scale) : (value * scale));
        if (negative) {
            // float
            returncoude = -returncoude;
        }
        return returncoude;
    }
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
    // char8 转 char32
    auto LongUI::UTF8ChartoChar32(const char* src) -> char32_t {
        assert(src && "bad argument");
        // 初始数据
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
        // 返回
        return ch;
    }
    // 获取u16转换u8后u8所占长度(含0结尾符)
    auto UTF16toUTF8GetBufLen(const char16_t * src) noexcept -> uint32_t {
        static_assert(sizeof(char16_t) == sizeof(wchar_t), "bad action");
        const wchar_t* src2 = reinterpret_cast<const wchar_t*>(src);
        uint32_t length = 1;
        char16_t ch = 0;
        // 遍历字符串
        while ((ch = *src)) {
            src += 1;
            // 1字节区域 [0-7F]
            if (ch < 0x0080) {
                length += 1;
            }
            // 四字节区
            else if (LongUI::IsHighSurrogate(ch)) {
                assert(IsHighSurrogate(src[0]) && "illegal utf-8");
                src += 1;
                length += 4;
            }
            // 3字节区域 > 2^11
            else if (ch > 0x07FFF){
                length += 3;
            }
            // 2字节区域 [80-7FF]
            else /*if (ch < 0x0800) */{
                length += 2;
            }
        }
        return length;
    }
    // 获取u8转换u16后u16所占长度(含0结尾符)
    auto UTF8toUTF16GetBufLen(const char* src) noexcept -> uint32_t {
        uint32_t length = 1;
        unsigned char ch = 0;
        // 遍历字符串
        while ((ch = *src)) {
            auto blen = BYTES_FOR_UTF8[ch];
            assert(blen > 0 && blen < 5 && "illegal utf-8");
            src += blen;
            length += 1 + (blen >> 2);
        }
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
            // 初始数据
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
        buflen;
#endif
        // 遍历字符串
        while (*src) {
            // 初始数据
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
    /// <summary>
    /// string to float.字符串转浮点, std::atof自己实现版
    /// </summary>
    /// <param name="p">The string. in const char*</param>
    /// <returns></returns>
    auto AtoF(const char* __restrict p) noexcept -> float {
        if (!p) return 0.0f;
        return impl::atof(p);
    }
    /// <summary>
    /// string to float.字符串转浮点, std::atof自己实现版
    /// </summary>
    /// <param name="p">The string.in const wchar_t*</param>
    /// <returns></returns>
    auto AtoF(const wchar_t* __restrict p) noexcept -> float {
        if (!p) return 0.0f;
        return impl::atof(p);
    }
    /// <summary>
    /// string to int, 字符串转整型, std::atoi自己实现版
    /// </summary>
    /// <param name="str">The string.</param>
    /// <returns></returns>
    auto AtoI(const char* __restrict str) noexcept -> int {
        if (!str) return 0;
        return impl::atoi(str);
    }
    /// <summary>
    /// string to int, 字符串转整型, std::atoi自己实现版
    /// </summary>
    /// <param name="str">The string.</param>
    /// <returns></returns>
    auto AtoI(const wchar_t* __restrict str) noexcept -> int {
        if (!str) return 0;
        return impl::atoi(str);
    }
    // BKDR 哈希
    auto BKDRHash(const char* str) noexcept -> uint32_t {
        constexpr uint32_t seed = 131;
        uint32_t code = 0;
        auto p = reinterpret_cast<const unsigned char*>(str);
        while (*p) code = code * seed + (*p++);
        return code;
    }
    // BKDR 哈希
    auto BKDRHash(const wchar_t* str) noexcept -> uint32_t {
        constexpr uint32_t seed = 131;
        uint32_t code = 0;
        while (*str) code = code * seed + (*str++);
        return code;
    }
    // π
    constexpr float EZ_PI = 3.1415296F;
    // 二分之一π
    constexpr float EZ_PI_2 = 1.5707963F;
    // CUIAnimation 缓动函数
    auto EasingFunction(AnimationType type, float p) noexcept ->float {
        assert((p >= 0.f && p <= 1.f) && "bad argument");
        switch (type)
        {
        default:
            assert(!"type unknown");
            __fallthrough;
        case LongUI::AnimationType::Type_LinearInterpolation:
            // 线性插值     f(x) = x
            return p;
        case LongUI::AnimationType::Type_QuadraticEaseIn:
            // 平次渐入     f(x) = x^2
            return p * p;
        case LongUI::AnimationType::Type_QuadraticEaseOut:
            // 平次渐出     f(x) =  -x^2 + 2x
            return -(p * (p - 2.f));
        case LongUI::AnimationType::Type_QuadraticEaseInOut:
            // 平次出入
            // [0, 0.5)     f(x) = (1/2)((2x)^2)
            // [0.5, 1.f]   f(x) = -(1/2)((2x-1)*(2x-3)-1) ; 
            return p < 0.5f ? (p * p * 2.f) : ((-2.f * p * p) + (4.f * p) - 1.f);
        case LongUI::AnimationType::Type_CubicEaseIn:
            // 立次渐入     f(x) = x^3;
            return p * p * p;
        case LongUI::AnimationType::Type_CubicEaseOut:
            // 立次渐出     f(x) = (x - 1)^3 + 1
        {
            float f = p - 1.f;
            return f * f * f + 1.f;
        }
        case LongUI::AnimationType::Type_CubicEaseInOut:
            // 立次出入
            // [0, 0.5)     f(x) = (1/2)((2x)^3) 
            // [0.5, 1.f]   f(x) = (1/2)((2x-2)^3 + 2) 
            if (p < 0.5f) {
                return p * p * p * 2.f;
            }
            else {
                float f = (2.f * p) - 2.f;
                return 0.5f * f * f * f + 1.f;
            }
        case LongUI::AnimationType::Type_QuarticEaseIn:
            // 四次渐入     f(x) = x^4
        {
            float f = p * p;
            return f * f;
        }
        case LongUI::AnimationType::Type_QuarticEaseOut:
            // 四次渐出     f(x) = 1 - (x - 1)^4
        {
            float f = (p - 1.f); f *= f;
            return 1.f - f * f;
        }
        case LongUI::AnimationType::Type_QuarticEaseInOut:
            // 四次出入
            // [0, 0.5)     f(x) = (1/2)((2x)^4)
            // [0.5, 1.f]   f(x) = -(1/2)((2x-2)^4 - 2)
            if (p < 0.5f) {
                float f = p * p;
                return 8.f * f * f;
            }
            else {
                float f = (p - 1.f); f *= f;
                return 1.f - 8.f * f * f;
            }
        case LongUI::AnimationType::Type_QuinticEaseIn:
            // 五次渐入     f(x) = x^5
        {
            float f = p * p;
            return f * f * p;
        }
        case LongUI::AnimationType::Type_QuinticEaseOut:
            // 五次渐出     f(x) = (x - 1)^5 + 1
        {
            float f = (p - 1.f);
            return f * f * f * f * f + 1.f;
        }
        case LongUI::AnimationType::Type_QuinticEaseInOut:
            // 五次出入
            // [0, 0.5)     f(x) = (1/2)((2x)^5) 
            // [0.5, 1.f]   f(x) = (1/2)((2x-2)^5 + 2)
            if (p < 0.5) {
                float f = p * p;
                return 16.f * f * f * p;
            }
            else {
                float f = ((2.f * p) - 2.f);
                return  f * f * f * f * f * 0.5f + 1.f;
            }
        case LongUI::AnimationType::Type_SineEaseIn:
            // 正弦渐入     
            return std::sin((p - 1.f) * EZ_PI_2) + 1.f;
        case LongUI::AnimationType::Type_SineEaseOut:
            // 正弦渐出     
            return std::sin(p * EZ_PI_2);
        case LongUI::AnimationType::Type_SineEaseInOut:
            // 正弦出入     
            return 0.5f * (1.f - std::cos(p * EZ_PI));
        case LongUI::AnimationType::Type_CircularEaseIn:
            // 四象圆弧
            return 1.f - std::sqrt(1.f - (p * p));
        case LongUI::AnimationType::Type_CircularEaseOut:
            // 二象圆弧
            return std::sqrt((2.f - p) * p);
        case LongUI::AnimationType::Type_CircularEaseInOut:
            // 圆弧出入
            if (p < 0.5f) {
                return 0.5f * (1.f - std::sqrt(1.f - 4.f * (p * p)));
            }
            else {
                return 0.5f * (std::sqrt(-((2.f * p) - 3.f) * ((2.f * p) - 1.f)) + 1.f);
            }
        case LongUI::AnimationType::Type_ExponentialEaseIn:
            // 指数渐入     f(x) = 2^(10(x - 1))
            return (p == 0.f) ? (p) : (std::pow(2.f, 10.f * (p - 1.f)));
        case LongUI::AnimationType::Type_ExponentialEaseOut:
            // 指数渐出     f(x) =  -2^(-10x) + 1
            return (p == 1.f) ? (p) : (1.f - std::powf(2.f, -10.f * p));
        case LongUI::AnimationType::Type_ExponentialEaseInOut:
            // 指数出入
            // [0,0.5)      f(x) = (1/2)2^(10(2x - 1)) 
            // [0.5,1.f]    f(x) = -(1/2)*2^(-10(2x - 1))) + 1 
            if (p == 0.0f || p == 1.0f) return p;
            if (p < 0.5f) {
                return 0.5f * std::powf(2.f, (20.f * p) - 10.f);
            }
            else {
                return -0.5f * std::powf(2.f, (-20.f * p) + 1.f) + 1.f;
            }
        case LongUI::AnimationType::Type_ElasticEaseIn:
            // 弹性渐入
            return std::sin(13.f * EZ_PI_2 * p) * std::pow(2.f, 10.f * (p - 1.f));
        case LongUI::AnimationType::Type_ElasticEaseOut:
            // 弹性渐出
            return std::sin(-13.f * EZ_PI_2 * (p + 1.f)) * std::powf(2.f, -10.f * p) + 1.f;
        case LongUI::AnimationType::Type_ElasticEaseInOut:
            // 弹性出入
            if (p < 0.5f) {
                return 0.5f * std::sin(13.f * EZ_PI_2 * (2.f * p)) * std::pow(2.f, 10.f * ((2.f * p) - 1.f));
            }
            else {
                return 0.5f * (std::sin(-13.f * EZ_PI_2 * ((2.f * p - 1.f) + 1.f)) * std::pow(2.f, -10.f * (2.f * p - 1.f)) + 2.f);
            }
        case LongUI::AnimationType::Type_BackEaseIn:
            // 回退渐入
            return  p * p * p - p * std::sin(p * EZ_PI);
        case LongUI::AnimationType::Type_BackEaseOut:
            // 回退渐出
        {
            float f = (1.f - p);
            return 1.f - (f * f * f - f * std::sin(f * EZ_PI));
        }
        case LongUI::AnimationType::Type_BackEaseInOut:
            // 回退出入
            if (p < 0.5f) {
                float f = 2.f * p;
                return 0.5f * (f * f * f - f * std::sin(f * EZ_PI));
            }
            else {
                float f = (1.f - (2 * p - 1.f));
                return 0.5f * (1.f - (f * f * f - f * std::sin(f * EZ_PI))) + 0.5f;
            }
        case LongUI::AnimationType::Type_BounceEaseIn:
            // 反弹渐入
            return 1.f - impl::bounce_ease_out(1.f - p);
        case LongUI::AnimationType::Type_BounceEaseOut:
            // 反弹渐出
            return impl::bounce_ease_out(p);
        case LongUI::AnimationType::Type_BounceEaseInOut:
            // 反弹出入
            if (p < 0.5f) {
                return 0.5f * (1.f - impl::bounce_ease_out(1.f - (p*2.f)));
            }
            else {
                return 0.5f * impl::bounce_ease_out(p * 2.f - 1.f) + 0.5f;
            }
        }
    }

}
