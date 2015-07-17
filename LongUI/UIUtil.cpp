
#include "LongUI.h"

// UIString 设置字符串
void LongUI::CUIString::Set(const wchar_t* str, uint32_t length) noexcept {
    assert(str && "<LongUI::CUIString::CUIString@const wchar_t*> str == null");
    // 未知则计算
    if (!length && *str) { length = ::wcslen(str); }
    // 超长的话
    if (length > m_cBufferLength) {
        m_cBufferLength = length + LongUIStringLength / 2;
        // 常试释放
        if (m_pString != m_aDataStatic) {
            LongUI::CtrlFree(m_pString);
        }
        // 申请内存
        m_pString = reinterpret_cast<wchar_t*>(
            LongUI::CtrlAlloc(sizeof(wchar_t) * (m_cBufferLength))
            );
    }
    // 复制数据
    assert(str && "<LongUI::CUIString::CUIString@const wchar_t*> m_pString == null");
    ::wcscpy(m_pString, str);
    m_cLength = length;
}

// UIString 字符串析构函数
LongUI::CUIString::~CUIString() noexcept {
    // 释放数据
    if (m_pString && m_pString != m_aDataStatic) {
        LongUI::CtrlFree(m_pString);
    }
    m_pString = nullptr;
    m_cLength = 0;
}

// 复制构造函数
LongUI::CUIString::CUIString(const LongUI::CUIString& obj) noexcept {
    // 构造
    if (obj.m_pString != obj.m_aDataStatic) {
        m_pString = reinterpret_cast<wchar_t*>(
            LongUI::CtrlAlloc(sizeof(wchar_t) * (obj.m_cLength + 1))
            );
    }
    // 复制数据
    assert(m_pString && "<LongUI::CUIString::CUIString@const CUIString&> m_pString == null");
    ::wcscpy(m_pString, obj.m_pString);
    m_cLength = obj.m_cLength;
}

// move构造函数
LongUI::CUIString::CUIString(LongUI::CUIString&& obj) noexcept {
    // 构造
    if (obj.m_pString != obj.m_aDataStatic) {
        m_pString = obj.m_pString;
    }
    else {
        // 复制数据
        ::wcscpy(m_aDataStatic, obj.m_aDataStatic);
    }
    m_cLength = obj.m_cLength;
    obj.m_cLength = 0;
    obj.m_pString = nullptr;
}

// += 操作
//const LongUI::CUIString& LongUI::CUIString::operator+=(const wchar_t*);

// DllControlLoader 构造函数
LongUI::DllControlLoader::DllControlLoader(
    const wchar_t* file, const wchar_t* name, const char* proc)
    noexcept : dll(::LoadLibraryW(file)){
    CreateControlFunction tmpfunc = nullptr;
    // 检查参数
    assert(file && name && proc && "bad argument");
    // 获取函数地址
    if (this->dll && (tmpfunc=reinterpret_cast<CreateControlFunction>(::GetProcAddress(dll, proc)))) {
        // 强制转换
        const_cast<CreateControlFunction&>(this->function) = tmpfunc;
        // 添加函数映射
        UIManager.RegisterControl(this->function, name);
    }
}

// DllControlLoader 析构函数
LongUI::DllControlLoader::~DllControlLoader() noexcept {
    if (this->dll) {
        ::FreeLibrary(this->dll);
        const_cast<HMODULE&>(this->dll) = nullptr;
    }
}

// Meta 渲染
void __fastcall LongUI::Meta_Render(
    const Meta& meta, LongUIRenderTarget* target, 
    const D2D1_RECT_F& des_rect, float opacity) noexcept {
    // 无效位图
    if (!meta.bitmap) {
        UIManager << DL_Warning << "bitmap->null" << LongUI::endl;
        return;
    }
    switch (meta.rule)
    {
    case LongUI::BitmapRenderRule::Rule_Scale:
        target->DrawBitmap(
            meta.bitmap,
            des_rect, opacity,
            static_cast<D2D1_INTERPOLATION_MODE>(meta.interpolation),
            meta.src_rect,
            nullptr
            );
        break;
    case LongUI::BitmapRenderRule::Rule_ButtonLike:
    {
        constexpr float MARKER = 0.25f;
        auto width = meta.src_rect.right - meta.src_rect.left;
        auto bilibili = width * MARKER / (meta.src_rect.bottom - meta.src_rect.top) *
            (des_rect.bottom - des_rect.top);
        D2D1_RECT_F des_rects[3]; D2D1_RECT_F src_rects[3]; D2D1_RECT_F clip_rects[3];
        // ---------------------------------------
        des_rects[0] = {
            des_rect.left, des_rect.top,
            des_rect.left + bilibili, des_rect.bottom
        };
        des_rects[1] = {
            des_rects[0].right, des_rect.top,
            des_rect.right - bilibili, des_rect.bottom
        };
        des_rects[2] = {
            des_rects[1].right, des_rect.top,
            des_rect.right, des_rect.bottom
        };
        // ---------------------------------------
        ::memcpy(clip_rects, des_rects, sizeof(des_rects));
        if (clip_rects[1].left > des_rects[1].right) {
            std::swap(clip_rects[1].right, des_rects[1].left);
            std::swap(des_rects[1].right, des_rects[1].left);
            clip_rects[0].right = des_rects[1].left;
            clip_rects[2].left = des_rects[1].right;
        }
        // ---------------------------------------
        src_rects[0] = {
            meta.src_rect.left, meta.src_rect.top,
            meta.src_rect.left + width * MARKER, meta.src_rect.bottom
        };
        src_rects[1] = {
            src_rects[0].right, meta.src_rect.top,
            meta.src_rect.right - width * MARKER, meta.src_rect.bottom
        };
        src_rects[2] = {
            src_rects[1].right, meta.src_rect.top,
            meta.src_rect.right, meta.src_rect.bottom
        };
        // 正式渲染
        for (auto i = 0u; i < lengthof(src_rects); ++i) {
            target->PushAxisAlignedClip(clip_rects + i, D2D1_ANTIALIAS_MODE_ALIASED);
            target->DrawBitmap(
                meta.bitmap,
                des_rects[i], opacity,
                static_cast<D2D1_INTERPOLATION_MODE>(meta.interpolation),
                src_rects[i],
                nullptr
                );
            target->PopAxisAlignedClip();
        }
    }
        break;
    }
}

#define white_space(c) ((c) == ' ' || (c) == '\t')
#define valid_digit(c) ((c) >= '0' && (c) <= '9')


// 自己实现LongUI::AtoI
auto __fastcall LongUI::AtoI(const char* __restrict str) -> int {
    if (!str) return 0;
    register bool negative = false;
    register int value = 0;
    register char ch = 0;
    while (ch = *str) {
        if (!white_space(ch)) {
            if (ch == '-') {
                negative = true;
            }
            else if (valid_digit(ch)) {
                value *= 10;
                value += ch - '0';
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


// 自己实现的LongUI::AtoF
auto __fastcall LongUI::AtoF(const char* __restrict p) -> float {
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
        value = value * 10.0f + static_cast<float>(*p - '0');
    }
    // 获取小数点或者指数之后的数字(有的话)
    if (*p == '.') {
        float pow10 = 10.0f;
        ++p;
        while (valid_digit(*p)) {
            value += (*p - '0') / pow10;
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
            expon = expon * 10 + (*p - '0');
        }
        // float 最大38 double 最大308
        if (expon > 38) expon = 38;
        // 计算比例因数
        while (expon >= 8) { scale *= 1E8f;  expon -= 8; }
        while (expon) { scale *= 10.0f; --expon; }
    }
    // 返回
    register float returncoude = (frac ? (value / scale) : (value * scale));
    if (negative) {
        // float
        returncoude = -returncoude;
    }
    return returncoude;
}



// 源: http://llvm.org/svn/llvm-project/llvm/trunk/lib/Support/ConvertUTF.c
// 有修改

static constexpr int halfShift = 10;

static constexpr char32_t halfBase = 0x0010000UL;
static constexpr char32_t halfMask = 0x3FFUL;

#define UNI_SUR_HIGH_START      (char32_t)0xD800
#define UNI_SUR_HIGH_END        (char32_t)0xDBFF
#define UNI_SUR_LOW_START       (char32_t)0xDC00
#define UNI_SUR_LOW_END         (char32_t)0xDFFF

#define UNI_REPLACEMENT_CHAR    (char32_t)0x0000FFFD
#define UNI_MAX_BMP             (char32_t)0x0000FFFF
#define UNI_MAX_UTF16           (char32_t)0x0010FFFF
#define UNI_MAX_UTF32           (char32_t)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32     (char32_t)0x0010FFFF

#define UNI_MAX_UTF8_BYTES_PER_CODE_POINT 4

#define UNI_UTF16_BYTE_ORDER_MARK_NATIVE  0xFEFF
#define UNI_UTF16_BYTE_ORDER_MARK_SWAPPED 0xFFFE

// 转换表
static constexpr char trailingBytesForUTF8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/*
* Magic values subtracted from a buffer value during UTF8 conversion.
* This table contains as many values as there might be trailing bytes
* in a UTF-8 sequence.
*/
static constexpr char32_t offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
* Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
* into the first byte, depending on how many bytes follow.  There are
* as many entries in this table as there are UTF-8 sequence types.
* (I.e., one byte sequence, two byte... etc.). Remember that sequencs
* for *legal* UTF-8 will be 4 or fewer bytes total.
*/
static constexpr char firstByteMark[7] = { (char)0x00, (char)0x00, (char)0xC0, (char)0xE0, (char)0xF0,(char)0xF8, (char)0xFC };


// 编码
auto __fastcall LongUI::Base64Encode(IN const uint8_t* __restrict bindata, IN size_t binlen, OUT char* __restrict const base64 ) noexcept -> char * {
    register uint8_t current;
    register auto base64_index = base64;
    // 
    for (size_t i = 0; i < binlen; i += 3) {
        current = (bindata[i] >> 2);
        current &= static_cast<uint8_t>(0x3F);
        *base64_index = Base64Chars[current]; ++base64_index;

        current = (static_cast<uint8_t>((bindata)[i] << 4)) & (static_cast<uint8_t>(0x30));
        if (i + 1 >= binlen) {
            *base64_index = Base64Chars[current]; ++base64_index;
            *base64_index = '='; ++base64_index;
            *base64_index = '='; ++base64_index;
            break;
        }
        current |= (static_cast<uint8_t>((bindata)[i + 1] >> 4)) & (static_cast<uint8_t>(0x0F));
        *base64_index = Base64Chars[current]; ++base64_index;

        current = (static_cast<uint8_t>((bindata)[i + 1] << 2)) & (static_cast<uint8_t>(0x3C));
        if (i + 2 >= binlen) {
            *base64_index = Base64Chars[current]; ++base64_index;
            *base64_index = '='; ++base64_index;
            break;
        }
        current |= (static_cast<uint8_t>((bindata)[i + 2] >> 6)) & (static_cast<uint8_t>(0x03));
        *base64_index = Base64Chars[current]; ++base64_index;

        current = (static_cast<uint8_t>(bindata[i + 2])) & (static_cast<uint8_t>(0x3F));
        *base64_index = Base64Chars[current]; ++base64_index;
    }
    *base64_index = 0;
    return base64;
}

// 解码
auto __fastcall LongUI::Base64Decode(IN const char * __restrict base64, OUT uint8_t * __restrict bindata) noexcept -> size_t{
    // 二进制长度
    register union { uint8_t temp[4]; uint32_t temp_u32; };
    register uint8_t* bindata_index = bindata;
    // 主循环
    while (*base64) {
        temp_u32 = uint32_t(-1);
        // 基本转换
        temp[0] = Base64Datas[base64[0]];  temp[1] = Base64Datas[base64[1]];
        temp[2] = Base64Datas[base64[2]];  temp[3] = Base64Datas[base64[3]];
        // 第一个二进制数据
        *bindata_index = ((temp[0] << 2) & uint8_t(0xFC)) | ((temp[1] >> 4) & uint8_t(0x03));
        ++bindata_index;
        if (base64[2] == '=') break;
        // 第三个二进制数据
        *bindata_index = ((temp[1] << 4) & uint8_t(0xF0)) | ((temp[2] >> 2) & uint8_t(0x0F));
        ++bindata_index;
        if (base64[3] == '=') break;
        // 第三个二进制数据
        *bindata_index = ((temp[2] << 6) & uint8_t(0xF0)) | ((temp[2] >> 0) & uint8_t(0x3F));
        ++bindata_index;
        base64 += 4;
    }
    return bindata_index - bindata;
}

// UTF-16 to UTF-8
// Return: UTF-8 string length, 0 maybe error
auto __fastcall LongUI::UTF16toUTF8(const char16_t* __restrict pUTF16String, char* __restrict pUTF8String) noexcept->uint32_t {
    UINT32 length = 0;
    const char16_t* source = pUTF16String;
    char* target = pUTF8String;
    //char* targetEnd = pUTF8String + uBufferLength;
    // 转换
    while (*source) {
        char32_t ch;
        unsigned short bytesToWrite = 0;
        const char32_t byteMask = 0xBF;
        const char32_t byteMark = 0x80;
        const char16_t* oldSource = source; /* In case we have to back up because of target overflow. */
        ch = *source++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            if (*source) {
                char32_t ch2 = *source;
                /* If it's a low surrogate, convert to UTF32. */
                if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
                    ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
                        + (ch2 - UNI_SUR_LOW_START) + halfBase;
                    ++source;
                }
            }
            else {
                --source;
                length = 0;
                assert(!"end of string");
                break;
            }
#ifdef STRICT_CONVERSION
        else { /* it's an unpaired high surrogate */
            --source; /* return to the illegal value itself */
            result = sourceIllegal;
            break;
        }
#endif
        }
#ifdef STRICT_CONVERSION
        else {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
                --source; /* return to the illegal value itself */
                result = sourceIllegal;
                break;
            }
        }
#endif
        /* Figure out how many bytes the result will require */
        if (ch < (char32_t)0x80) {
            bytesToWrite = 1;
        }
        else if (ch < (char32_t)0x800) {
            bytesToWrite = 2;
        }
        else if (ch < (char32_t)0x10000) {
            bytesToWrite = 3;
        }
        else if (ch < (char32_t)0x110000) {
            bytesToWrite = 4;
        }
        else {
            bytesToWrite = 3;
            ch = UNI_REPLACEMENT_CHAR;
        }

        target += bytesToWrite;
        /*if (target > targetEnd) {
            source = oldSource; // Back up source pointer!
            target -= bytesToWrite;
            length = 0; break;
        }*/
        switch (bytesToWrite) { /* note: everything falls through. */
        case 4: *--target = (char)((ch | byteMark) & byteMask); ch >>= 6;
        case 3: *--target = (char)((ch | byteMark) & byteMask); ch >>= 6;
        case 2: *--target = (char)((ch | byteMark) & byteMask); ch >>= 6;
        case 1: *--target = (char)(ch | firstByteMark[bytesToWrite]);
        }
        target += bytesToWrite;
        length += bytesToWrite;
    }
    return length;
}



// UTF-8 to UTF-16
// Return: UTF-16 string length, 0 maybe error
auto __fastcall LongUI::UTF8toUTF16(const char* __restrict pUTF8String, char16_t* __restrict pUTF16String) noexcept -> uint32_t {
    UINT32 length = 0;
    auto source = reinterpret_cast<const unsigned char*>(pUTF8String);
    char16_t* target = pUTF16String;
    //char16_t* targetEnd = pUTF16String + uBufferLength;
    // 遍历
    while (*source) {
        char32_t ch = 0;
        unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
        /*if (extraBytesToRead >= sourceEnd - source) {
        result = sourceExhausted; break;
        }*/
        /* Do this check whether lenient or strict */
        /*if (!isLegalUTF8(source, extraBytesToRead + 1)) {
        result = sourceIllegal;
        break;
        }*/
        /*
        * The cases all fall through. See "Note A" below.
        */
        switch (extraBytesToRead) {
        case 5: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
        case 4: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
        case 3: ch += *source++; ch <<= 6;
        case 2: ch += *source++; ch <<= 6;
        case 1: ch += *source++; ch <<= 6;
        case 0: ch += *source++;
        }
        ch -= offsetsFromUTF8[extraBytesToRead];

        /*if (target >= targetEnd) {
            source -= (extraBytesToRead + 1); // Back up source pointer!
            length = 0; break;
        }*/
        if (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
                                 /* UTF-16 surrogate values are illegal in UTF-32 */
            if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
#ifdef STRICT_CONVERSION
                source -= (extraBytesToRead + 1); /* return to the illegal value itself */
                length = 0;
                break;
#else
                *target++ = UNI_REPLACEMENT_CHAR;
                ++length;
#endif
            }
            else {
                *target++ = (char16_t)ch; /* normal case */
                ++length;
            }
        }
        else if (ch > UNI_MAX_UTF16) {
#ifdef STRICT_CONVERSION
            length = 0;
            source -= (extraBytesToRead + 1); /* return to the start */
            break; /* Bail out; shouldn't continue */
#else
            *target++ = UNI_REPLACEMENT_CHAR;
            ++length;
#endif
        }
        else {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            /*if (target + 1 >= targetEnd) {
                source -= (extraBytesToRead + 1); // Back up source pointer!
                length = 0; break;
            }*/
            ch -= halfBase;
            *target++ = (char16_t)((ch >> halfShift) + UNI_SUR_HIGH_START);
            *target++ = (char16_t)((ch & halfMask) + UNI_SUR_LOW_START);
            length += 2;
        }
    }
    // 最后修正
    return length;
}


// ----------------------------
#define UIText_NewAttribute(a) { ::strcpy(attribute_buffer, prefix); ::strcat(attribute_buffer, a); }
// UIText 构造函数
LongUI::UIText::UIText(pugi::xml_node node, const char * prefix) noexcept
    : m_pTextRenderer(nullptr){
    m_config = {
        ::SafeAcquire(UIManager_DWriteFactory),
        nullptr,
        UIManager.inline_handler,
        128.f, 64.f, 1.f, 0
    };
    // 检查参数
    assert(node && prefix && "bad arguments");
    register union { const char* str; const uint32_t* pui32; };
    str = nullptr;
    char attribute_buffer[256];
    // 获取进度
    {
        UIText_NewAttribute("progress");
        if ((str = node.attribute(attribute_buffer).value())) {
            m_config.progress = LongUI::AtoF(str);
        }
    }
    // 获取渲染器
    {
        int renderer_index = Type_NormalTextRenderer;
        UIText_NewAttribute("renderer");
        if ((str = node.attribute(attribute_buffer).value())) {
            renderer_index = LongUI::AtoI(str);
        }
        auto renderer = UIManager.GetTextRenderer(renderer_index);
        m_pTextRenderer = renderer;
        // 保证缓冲区
        if (renderer) {
            auto length = renderer->GetContextSizeInByte();
            if (length) {
                UIText_NewAttribute("context");
                if ((str = node.attribute(attribute_buffer).value())) {
                    m_buffer.NewSize(length);
                    renderer->CreateContextFromString(m_buffer.data, str);
                }
            }
        }
    }
    {
        // 检查基本颜色
        m_basicColor = D2D1::ColorF(D2D1::ColorF::Black);
        UIText_NewAttribute("color");
        UIControl::MakeColor(node.attribute(attribute_buffer).value(), m_basicColor);
    }
    {
        // 检查格式
        uint32_t format_index = 0;
        UIText_NewAttribute("format");
        if ((str = node.attribute(attribute_buffer).value())) {
            format_index = static_cast<uint32_t>(LongUI::AtoI(str));
        }
        m_config.text_format = UIManager.GetTextFormat(format_index);
    }
    {
        // 检查类型
        UIText_SetIsRich(false);
        UIText_NewAttribute("type");
        if (str = node.attribute(attribute_buffer).value()) {
            switch (*pui32)
            {
            case "xml"_longui32:
            case "XML"_longui32:
                UIText_SetIsRich(true);
                UIText_SetIsXML(true);
                break;
            case "core"_longui32:
            case "Core"_longui32:
            case "CORE"_longui32:
                UIText_SetIsRich(true);
                UIText_SetIsXML(false);
                break;
            default:
                UIText_SetIsRich(false);
                UIText_SetIsXML(false);
                break;
            }
        }
    }
    // 重建
    this->recreate(node.attribute(prefix).value());
}

// UIText = L"***"
LongUI::UIText& LongUI::UIText::operator=(const wchar_t* new_string) noexcept {
    // 不能是XML模式
    assert(UIText_GetIsXML() == false && "=(const wchar_t*) must be in core-mode, can't be xml-mode");
    m_text.Set(new_string);
    this->recreate();
    return *this;
}

// UIText = "***"
LongUI::UIText & LongUI::UIText::operator=(const char* str) noexcept {
    if (UIText_GetIsXML()) {
        this->recreate(str);
        return *this;
    }
    else {
        wchar_t buffer[LongUIStringBufferLength];
        buffer[LongUI::UTF8toWideChar(str, buffer)] = L'\0';
        return this->operator=(buffer);
    }
}


// UIText 析构
LongUI::UIText::~UIText() noexcept {
    m_pTextRenderer.SafeRelease();
    ::SafeRelease(m_pLayout);
    ::SafeRelease(m_config.dw_factory);
    ::SafeRelease(m_config.text_format);
}

void LongUI::UIText::recreate(const char* utf8) noexcept {
    wchar_t text_buffer[LongUIStringBufferLength];
    // 转换为核心模式
    if (UIText_GetIsXML() && UIText_GetIsRich()) {
        CUIManager::XMLToCoreFormat(utf8, text_buffer);
    }
    else if(utf8){
        // 直接转码
        register auto length = LongUI::UTF8toWideChar(utf8, text_buffer);
        text_buffer[length] = L'\0';
        m_text.Set(text_buffer, length);
    }
    // 创建布局
    ::SafeRelease(m_pLayout);
    // 富文本
    if (UIText_GetIsRich()) {
        m_pLayout = CUIManager::FormatTextCore(
            m_config,
            m_text.c_str(),
            nullptr
            );
    }
    // 平台文本
    else {
        register auto string_length_need = static_cast<uint32_t>(static_cast<float>(m_text.length() + 1) * 
            m_config.progress);
        LongUIClamp(string_length_need, 0, m_text.length());
        m_config.dw_factory->CreateTextLayout(
            m_text.c_str(),
            string_length_need,
            m_config.text_format,
            m_config.width,
            m_config.height,
            &m_pLayout
            );
        m_config.text_length = m_text.length();
    }
}


// UIFileLoader 读取文件
bool LongUI::CUIFileLoader::ReadFile(WCHAR* file_name) noexcept {
    // 打开文件
    FILE* file = nullptr;
    if (file = ::_wfopen(file_name, L"rb")) {
        // 获取文件大小
        ::fseek(file, 0L, SEEK_END);
        m_cLength = ::ftell(file);
        ::fseek(file, 0L, SEEK_SET);
        // 缓存不足?
        if (m_cLength > m_cLengthReal) {
            m_cLengthReal = m_cLength;
            if (m_pData) free(m_pData);
            m_pData = malloc(m_cLength);
        }
        // 读取文件
        if (m_pData) ::fread(m_pData, 1, m_cLength, file);
    }
    // 关闭文件
    if (file) ::fclose(file);
    return file && m_pData;
}


// --------------  CUIConsole ------------
// CUIConsole 构造函数
LongUI::CUIConsole::CUIConsole() noexcept {
    ::InitializeCriticalSection(&m_cs); m_name[0] = L'\0';
    { if (m_hConsole != INVALID_HANDLE_VALUE) this->WindUp(); }
}

// CUIConsole 析构函数
LongUI::CUIConsole::~CUIConsole() noexcept {
    this->WindUp();
    // 关闭
    if (m_hConsole != INVALID_HANDLE_VALUE) {
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
    }
    ::DeleteCriticalSection(&m_cs);
}

// CUIConsole 关闭
long LongUI::CUIConsole::WindUp() noexcept {
    if (!(*this))
        return -1;
    else
        return ::DisconnectNamedPipe(m_hConsole);
}

// CUIConsole 输出
long LongUI::CUIConsole::Output(const wchar_t * str, bool flush, long len) noexcept {
    if (len == -1) len = ::wcslen(str);
    // 过长则分批
    if (len > LongUIStringBufferLength) {
        // 直接递归
        while (len) {
            auto len_in = len > LongUIStringBufferLength ? LongUIStringBufferLength : len;
            this->Output(str, true, len_in);
            len -= len_in;
            str += len_in;
        }
        return 0;
    }
    // 计算目标
    if (m_length + len > LongUIStringBufferLength) {
        flush = true;
    }
    // 写入
    if (m_length + len < LongUIStringBufferLength) {
        ::memcpy(m_buffer + m_length, str, len * sizeof(wchar_t));
        m_length += len;
        str = nullptr;
        // 不flush
        if(!flush) return 0;
    }
    DWORD dwWritten = DWORD(-1);
    auto safe_write_file = [this](HANDLE hFile, LPCVOID lpBuffer,
        DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten,
        LPOVERLAPPED lpOverlapped) noexcept {
        ::EnterCriticalSection(&m_cs);
        BOOL bRet = ::WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
        ::LeaveCriticalSection(&m_cs);
        return bRet;
    };
    // 先写入缓冲区
    if (m_length) {
        safe_write_file(m_hConsole, m_buffer, m_length * sizeof(wchar_t), &dwWritten, nullptr);
        m_length = 0;
    }
    // 再写入目标
    if (str) {
        len *= sizeof(wchar_t);
        return (!safe_write_file(m_hConsole, str, len, &dwWritten, nullptr)
            || (int)dwWritten != len) ? -1 : (int)dwWritten;
    }
    return 0;
}

// CUIConsole 创建
long LongUI::CUIConsole::Create(const wchar_t* lpszWindowTitle, Config& config) noexcept {
    // 二次创建?
    if (m_hConsole != INVALID_HANDLE_VALUE) {
        ::DisconnectNamedPipe(m_hConsole);
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
    }
    // 先复制
    ::wcsncpy(m_name, LR"(\\.\pipe\)", 9);
    wchar_t logger_name_buffer[128];
    // 未给logger?
    if (!config.logger_name)  {
        ::swprintf(
            logger_name_buffer, lengthof(logger_name_buffer),
            L"logger_%f", float(::rand()) / float(RAND_MAX)
            );
        config.logger_name = logger_name_buffer;
    }
    ::wcscat(m_name, config.logger_name);
    // 创建管道
    m_hConsole = ::CreateNamedPipeW(
        m_name,
        PIPE_ACCESS_OUTBOUND,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1,
        4096,   // 输出缓存
        0,      // 输入缓存
        1,
        nullptr
    );
    // 无效
    if (m_hConsole == INVALID_HANDLE_VALUE) {
        ::MessageBoxW(nullptr, L"CreateNamedPipe failed", L"CUIConsole::Create failed", MB_ICONERROR);
        return -1;
    }
    // 创建控制台
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ::GetStartupInfoW(&si);

    const wchar_t* DEFAULT_HELPER_EXE = L"ConsoleHelper.exe";

    wchar_t cmdline[MAX_PATH];;
    if (!config.helper_executable)
        config.helper_executable = DEFAULT_HELPER_EXE;

    ::swprintf(cmdline, MAX_PATH, L"%ls %ls", config.helper_executable, config.logger_name);
    BOOL bRet = ::CreateProcessW(nullptr, cmdline, nullptr, nullptr, false, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi);
    if (!bRet)  {
        char *path = ::getenv("ConsoleLoggerHelper");
        if (path) {
            ::swprintf(cmdline, MAX_PATH, L"%s %ls", path, config.logger_name);
            bRet = ::CreateProcessW(nullptr, nullptr, nullptr, nullptr, false, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi);
        }
        if (!bRet) {
            ::MessageBoxW(nullptr, L"Helper executable not found", L"ConsoleLogger failed", MB_ICONERROR);
            ::CloseHandle(m_hConsole);
            m_hConsole = INVALID_HANDLE_VALUE;
            return -1;
        }
    }


    BOOL bConnected = ::ConnectNamedPipe(m_hConsole, nullptr) ?
        TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
    // 连接失败
    if (!bConnected) {
        ::MessageBoxW(nullptr, L"ConnectNamedPipe failed", L"ConsoleLogger failed", MB_ICONERROR);
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
        return -1;
    }

    DWORD cbWritten;

    // 特性

    wchar_t buffer[128];
    // 传送标题
    if (!lpszWindowTitle) lpszWindowTitle = m_name + 9;
    ::swprintf(buffer, lengthof(buffer), L"TITLE: %ls\r\n", lpszWindowTitle);
    auto len_in_byte = ::wcslen(buffer) * sizeof(wchar_t);
    ::WriteFile(m_hConsole, buffer, len_in_byte, &cbWritten, nullptr);
    if (cbWritten != len_in_byte) {
        ::MessageBoxW(nullptr, L"WriteFile failed(1)", L"ConsoleLogger failed", MB_ICONERROR);
        ::DisconnectNamedPipe(m_hConsole);
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
        return -1;
    }

    // 传送位置
    if (config.position_xy != -1) {
        ::swprintf(buffer, lengthof(buffer), L"POS: %d\r\n", config.position_xy);
        len_in_byte = ::wcslen(buffer) * sizeof(wchar_t);
        ::WriteFile(m_hConsole, buffer, len_in_byte, &cbWritten, nullptr);
        if (cbWritten != len_in_byte) {
            ::MessageBoxW(nullptr, L"WriteFile failed(1.1)", L"ConsoleLogger failed", MB_ICONERROR);
            ::DisconnectNamedPipe(m_hConsole);
            ::CloseHandle(m_hConsole);
            m_hConsole = INVALID_HANDLE_VALUE;
            return -1;
        }
    }
    // 传送属性
    if (config.atribute) {
        ::swprintf(buffer, lengthof(buffer), L"ATTR: %d\r\n", config.atribute);
        len_in_byte = ::wcslen(buffer) * sizeof(wchar_t);
        ::WriteFile(m_hConsole, buffer, len_in_byte, &cbWritten, nullptr);
        if (cbWritten != len_in_byte) {
            ::MessageBoxW(nullptr, L"WriteFile failed(1.2)", L"ConsoleLogger failed", MB_ICONERROR);
            ::DisconnectNamedPipe(m_hConsole);
            ::CloseHandle(m_hConsole);
            m_hConsole = INVALID_HANDLE_VALUE;
            return -1;
        }
    }

    // 传送缓存区大小
    if (config.buffer_size_x != -1 && config.buffer_size_y != -1)  {
        ::swprintf(buffer, lengthof(buffer), L"BUFFER-SIZE: %dx%d\r\n", config.buffer_size_x, config.buffer_size_y);
        len_in_byte = ::wcslen(buffer) * sizeof(wchar_t);
        ::WriteFile(m_hConsole, buffer, len_in_byte, &cbWritten, nullptr);
        if (cbWritten != len_in_byte) {
            ::MessageBoxW(nullptr, L"WriteFile failed(2)", L"ConsoleLogger failed", MB_ICONERROR);
            ::DisconnectNamedPipe(m_hConsole);
            ::CloseHandle(m_hConsole);
            m_hConsole = INVALID_HANDLE_VALUE;
            return -1;
        }
    }

    // 添加头
    if (false)  {
        ::DisconnectNamedPipe(m_hConsole);
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
        return -1;
    }

    // 传送完毕

    buffer[0] = 0;
    ::WriteFile(m_hConsole, buffer, 2, &cbWritten, nullptr);
    if (cbWritten != 2) {
        ::MessageBoxW(nullptr, L"WriteFile failed(3)", L"ConsoleLogger failed", MB_ICONERROR);
        ::DisconnectNamedPipe(m_hConsole);
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
        return -1;
    }
    return 0;
}

// --------------  CUIDefaultConfigure ------------
#ifdef LONGUI_WITH_DEFAULT_CONFIG

// CUIDefaultConfigure 析构函数
LongUI::CUIDefaultConfigure::~CUIDefaultConfigure() noexcept {
    ::SafeRelease(m_pXMLResourceLoader);
    ::SafeRelease(this->script);
}

// longui
namespace LongUI {
    // 创建XML资源读取器
    auto CreateResourceLoaderForXML(CUIManager& manager, const char* xml) noexcept->IUIResourceLoader*;
}

// 查询接口信息
auto LongUI::CUIDefaultConfigure::QueryInterface(const IID & riid, void ** ppvObject) noexcept -> HRESULT {
    // 非接口
    if (riid == LongUI::IID_InlineParamHandler) {
        if (handler) {
            *ppvObject = handler;
            return S_OK;
        }
        else {
            return E_NOINTERFACE;
        }
    }
    // 接口
    IUIInterface* pUIInterface = nullptr;
    // 资源读取器
    if (riid == LongUI::IID_IUIResourceLoader) {
        if (m_pXMLResourceLoader) {
            pUIInterface = (m_pXMLResourceLoader);
        }
        else {
            pUIInterface = m_pXMLResourceLoader = 
                LongUI::CreateResourceLoaderForXML(m_manager, this->resource);
        }
    }
    // 脚本
    else if (riid == LongUI::IID_IUIScript) {
        pUIInterface = (script);
    }
    // 增加计数
    *ppvObject = ::SafeAcquire(pUIInterface);
    // 检查
    return pUIInterface ? S_OK : E_NOINTERFACE;
}

auto LongUI::CUIDefaultConfigure::ChooseAdapter(IDXGIAdapter1 * adapters[], size_t const length) noexcept -> size_t {
    // 核显卡优先 
#ifdef LONGUI_NUCLEAR_FIRST
    for (size_t i = 0; i < length; ++i) {
        DXGI_ADAPTER_DESC1 desc;
        adapters[i]->GetDesc1(&desc);
        if (!::wcsncmp(L"NVIDIA", desc.Description, 6))
            return i;
    }
#endif
    return length;
}

// CUIDefaultConfigure 显示错误信息
auto LongUI::CUIDefaultConfigure::ShowError(const wchar_t * str_a, const wchar_t* str_b) noexcept -> void {
    assert(str_a && "bad argument!");
    if(!str_b) str_b = L"Error!";
    ::MessageBoxW(::GetForegroundWindow(), str_a, str_b, MB_ICONERROR);
}

#ifdef _DEBUG
// 输出调试字符串
auto LongUI::CUIDefaultConfigure::OutputDebugStringW(
    DebugStringLevel level, const wchar_t * string, bool flush) noexcept -> void {
    auto& console = this->consoles[level];
    // 无效就创建
    if (!console) {
        this->CreateConsole(level);
    }
    // 有效就输出
    if (console) {
        console.Output(string, flush);
    }
}

void LongUI::CUIDefaultConfigure::CreateConsole(DebugStringLevel level) noexcept {
    CUIConsole::Config config;
    config.x = -5;
    config.y = int16_t(level) * 128;
    switch (level)
    {
    case LongUI::DLevel_None:
        break;
    case LongUI::DLevel_Log:
        break;
    case LongUI::DLevel_Hint:
        break;
    case LongUI::DLevel_Warning:
        config.atribute = FOREGROUND_RED | FOREGROUND_GREEN;
        break;
    case LongUI::DLevel_Error:
    case LongUI::DLevel_Fatal:
        config.atribute = FOREGROUND_RED;
        break;
    }
    assert(level < LongUI::DLEVEL_SIZE);
    // 名称
    const wchar_t* strings[LongUI::DLEVEL_SIZE] = {
        L"None      Console", 
        L"Log       Console", 
        L"Hint      Console", 
        L"Warning   Console", 
        L"Error     Console", 
        L"Fatal     Console"
    };

    this->consoles[level].Create(strings[level], config);
}

#endif
#endif

//////////////////////////////////////////
// Video
//////////////////////////////////////////
#ifdef LONGUI_VIDEO_IN_MF
// CUIVideoComponent 事件通知
HRESULT LongUI::CUIVideoComponent::EventNotify(DWORD event, DWORD_PTR param1, DWORD param2) noexcept {
    switch (event)
    {
    case MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA:
        m_bEOS = false;
        break;
    case MF_MEDIA_ENGINE_EVENT_CANPLAY:
        this->Play();
        break;
    case MF_MEDIA_ENGINE_EVENT_PLAY:
        m_bPlaying = true;
        break;
    case MF_MEDIA_ENGINE_EVENT_PAUSE:
        m_bPlaying = false;
        break;
    case MF_MEDIA_ENGINE_EVENT_ENDED:
        m_bPlaying = false;
        m_bEOS = true;
        break;
    case MF_MEDIA_ENGINE_EVENT_TIMEUPDATE:
        break;
    case MF_MEDIA_ENGINE_EVENT_NOTIFYSTABLESTATE:
        ::SetEvent(reinterpret_cast<HANDLE>(param1));
        break;
    case MF_MEDIA_ENGINE_EVENT_ERROR:
    {
        auto err = MF_MEDIA_ENGINE_ERR(param1);
        auto hr = HRESULT(param2);
        int a = 9;
    }
        break;
    }
    return S_OK;
}

HRESULT LongUI::CUIVideoComponent::Init() noexcept {
    HRESULT hr = S_OK;
    IMFAttributes* attributes = nullptr;
    // 创建MF属性
    if (SUCCEEDED(hr)) {
        hr = ::MFCreateAttributes(&attributes, 1);
    }
    // 设置属性: DXGI管理器
    if (SUCCEEDED(hr)) {
        hr = attributes->SetUnknown(MF_MEDIA_ENGINE_DXGI_MANAGER, UIManager_MFDXGIDeviceManager);
    }
    // 设置属性: 事件通知
    if (SUCCEEDED(hr)) {
        hr = attributes->SetUnknown(MF_MEDIA_ENGINE_CALLBACK, this);
    }
    // 设置属性: 输出格式
    if (SUCCEEDED(hr)) {
        hr = attributes->SetUINT32(MF_MEDIA_ENGINE_VIDEO_OUTPUT_FORMAT, DXGI_FORMAT_B8G8R8A8_UNORM);
    }
    // 创建媒体引擎
    if (SUCCEEDED(hr)) {
        constexpr DWORD flags = MF_MEDIA_ENGINE_WAITFORSTABLE_STATE;
        hr = UIManager_MFMediaEngineClassFactory->CreateInstance(flags, attributes, &m_pMediaEngine);
    }
    // 获取Ex版
    /*if (SUCCEEDED(hr)) {
        hr = m_pMediaEngine->QueryInterface(LongUI_IID_PV_ARGS(m_pEngineEx));
    }*/
    assert(SUCCEEDED(hr));
    ::SafeRelease(attributes);
    return hr;
}

// 重建
HRESULT LongUI::CUIVideoComponent::Recreate(ID2D1RenderTarget* target) noexcept {
    ::SafeRelease(m_pRenderTarget);
    ::SafeRelease(m_pTargetSurface);
    ::SafeRelease(m_pDrawSurface);

    assert(target && "bad argument");
    m_pRenderTarget = ::SafeAcquire(target);
    return this->recreate_surface();
}

// 渲染
void LongUI::CUIVideoComponent::Render(D2D1_RECT_F* dst) const noexcept {
    /*const MFARGB bkColor = { 0,0,0,0 };
    assert(m_pMediaEngine);
    // 表面无效
    if (!m_pDrawSurface) {
        this->recreate_surface();
    }
    // 表面有效
    if (m_pDrawSurface) {
        LONGLONG pts;
        if ((m_pMediaEngine->OnVideoStreamTick(&pts)) == S_OK) {
            D3D11_TEXTURE2D_DESC desc;
            m_pTargetSurface->GetDesc(&desc);
            m_pMediaEngine->TransferVideoFrame(m_pTargetSurface, nullptr, &dst_rect, &bkColor);
            m_pDrawSurface->CopyFromBitmap(nullptr, m_pSharedSurface, nullptr);
        }
        D2D1_RECT_F src = { 0.f, 0.f,  float(dst_rect.right), float(dst_rect.bottom) };
        m_pRenderTarget->DrawBitmap(m_pDrawSurface, dst, 1.f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &src
            );
    }*/
}


// CUIVideoComponent 构造函数
LongUI::CUIVideoComponent::CUIVideoComponent() noexcept {
    force_cast(dst_rect) = { 0 };
}


// CUIVideoComponent 析构函数
LongUI::CUIVideoComponent::~CUIVideoComponent() noexcept {
    if (m_pMediaEngine) {
        m_pMediaEngine->Shutdown();
    }
    ::SafeRelease(m_pMediaEngine);
    //::SafeRelease(m_pEngineEx);
    ::SafeRelease(m_pTargetSurface);
    ::SafeRelease(m_pSharedSurface);
    ::SafeRelease(m_pDrawSurface);
    ::SafeRelease(m_pRenderTarget);
}

#define MakeAsUnit(a) (((a) + (LongUITargetBitmapUnitSize-1)) / LongUITargetBitmapUnitSize * LongUITargetBitmapUnitSize)


// 重建表面
HRESULT LongUI::CUIVideoComponent::recreate_surface() noexcept {
    // 有效情况下
    DWORD w, h; HRESULT hr = S_FALSE;
    if (this->HasVideo() && SUCCEEDED(hr = m_pMediaEngine->GetNativeVideoSize(&w, &h))) {
        force_cast(dst_rect.right) = w;
        force_cast(dst_rect.bottom) = h;
        // 获取规范大小
        w = MakeAsUnit(w); h = MakeAsUnit(h);
        // 检查承载大小
        D2D1_SIZE_U size = m_pDrawSurface ? m_pDrawSurface->GetPixelSize() : D2D1::SizeU();
        // 重建表面
        if (w > size.width || h > size.height) {
            size = { w, h };
            ::SafeRelease(m_pTargetSurface);
            ::SafeRelease(m_pSharedSurface);
            ::SafeRelease(m_pDrawSurface);
            IDXGISurface* surface = nullptr;
#if 0
            D3D11_TEXTURE2D_DESC desc = {
                w, h, 1, 1, DXGI_FORMAT_B8G8R8A8_UNORM, {1, 0}, D3D11_USAGE_DEFAULT, 
                D3D11_BIND_RENDER_TARGET, 0, 0
            };
            hr = UIManager_D3DDevice->CreateTexture2D(&desc, nullptr, &m_pTargetSurface);
            // 获取Dxgi表面
            if (SUCCEEDED(hr)) {
                hr = m_pTargetSurface->QueryInterface(LongUI_IID_PV_ARGS(surface));
            }
            // 从Dxgi表面创建位图
            if (SUCCEEDED(hr)) {
                hr = UIManager_RenderTaget->CreateBitmapFromDxgiSurface(
                    surface, nullptr, &m_pDrawSurface
                    );
            }
#else
            // 创建D2D位图
            D2D1_BITMAP_PROPERTIES1 prop = {
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                96.f, 96.f,
                D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_TARGET, nullptr
            };
            hr = UIManager_RenderTaget->CreateBitmap(size, nullptr, size.width * 4, &prop, &m_pSharedSurface);
            // 获取Dxgi表面
            if (SUCCEEDED(hr)) {
                hr = m_pSharedSurface->GetSurface(&surface);
            }
            // 获取D3D11 2D纹理
            if (SUCCEEDED(hr)) {
                hr = surface->QueryInterface(LongUI_IID_PV_ARGS(m_pTargetSurface));
            }
            // 创建刻画位图
            if (SUCCEEDED(hr)) {
                prop.bitmapOptions = D2D1_BITMAP_OPTIONS_NONE;
                hr = UIManager_RenderTaget->CreateBitmap(size, nullptr, size.width * 4, &prop, &m_pDrawSurface);
            }
#endif
            ::SafeRelease(surface);
        }
    }
    return hr;
}

#endif



// -----------------------------



// π
static constexpr float EZ_PI   = 3.1415296F;
// 二分之一π
static constexpr float EZ_PI_2 = 1.5707963F;

// 反弹渐出
float inline __fastcall BounceEaseOut(float p) noexcept {
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


// CUIAnimation 缓动函数
float __fastcall LongUI::EasingFunction(AnimationType type, float p) noexcept {
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
        register float f = p - 1.f;
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
            register float f = (2.f * p) - 2.f;
            return 0.5f * f * f * f + 1.f;
        }
    case LongUI::AnimationType::Type_QuarticEaseIn:
        // 四次渐入     f(x) = x^4
    {
        register float f = p * p;
        return f * f;
    }
    case LongUI::AnimationType::Type_QuarticEaseOut:
        // 四次渐出     f(x) = 1 - (x - 1)^4
    {
        register float f = (p - 1.f); f *= f;
        return 1.f - f * f;
    }
    case LongUI::AnimationType::Type_QuarticEaseInOut:
        // 四次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^4)
        // [0.5, 1.f]   f(x) = -(1/2)((2x-2)^4 - 2)
        if (p < 0.5f) {
            register float f = p * p;
            return 8.f * f * f;
        }
        else {
            register float f = (p - 1.f); f *= f;
            return 1.f - 8.f * f * f;
        }
    case LongUI::AnimationType::Type_QuinticEaseIn:
        // 五次渐入     f(x) = x^5
    {
        register float f = p * p;
        return f * f * p;
    }
    case LongUI::AnimationType::Type_QuinticEaseOut:
        // 五次渐出     f(x) = (x - 1)^5 + 1
    {
        register float f = (p - 1.f);
        return f * f * f * f * f + 1.f;
    }
    case LongUI::AnimationType::Type_QuinticEaseInOut:
        // 五次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^5) 
        // [0.5, 1.f]   f(x) = (1/2)((2x-2)^5 + 2)
        if (p < 0.5) {
            register float f = p * p;
            return 16.f * f * f * p;
        }
        else {
            register float f = ((2.f * p) - 2.f);
            return  f * f * f * f * f * 0.5f + 1.f;
        }
    case LongUI::AnimationType::Type_SineEaseIn:
        // 正弦渐入     
        return ::sin((p - 1.f) * EZ_PI_2) + 1.f;
    case LongUI::AnimationType::Type_SineEaseOut:
        // 正弦渐出     
        return ::sin(p * EZ_PI_2);
    case LongUI::AnimationType::Type_SineEaseInOut:
        // 正弦出入     
        return 0.5f * (1.f - ::cos(p * EZ_PI));
    case LongUI::AnimationType::Type_CircularEaseIn:
        // 四象圆弧
        return 1.f - ::sqrt(1.f - (p * p));
    case LongUI::AnimationType::Type_CircularEaseOut:
        // 二象圆弧
        return ::sqrt((2.f - p) * p);
    case LongUI::AnimationType::Type_CircularEaseInOut:
        // 圆弧出入
        if (p < 0.5f) {
            return 0.5f * (1.f - ::sqrt(1.f - 4.f * (p * p)));
        }
        else {
            return 0.5f * (::sqrt(-((2.f * p) - 3.f) * ((2.f * p) - 1.f)) + 1.f);
        }
    case LongUI::AnimationType::Type_ExponentialEaseIn:
        // 指数渐入     f(x) = 2^(10(x - 1))
        return (p == 0.f) ? (p) : (::pow(2.f, 10.f * (p - 1.f)));
    case LongUI::AnimationType::Type_ExponentialEaseOut:
        // 指数渐出     f(x) =  -2^(-10x) + 1
        return (p == 1.f) ? (p) : (1.f - ::powf(2.f, -10.f * p));
    case LongUI::AnimationType::Type_ExponentialEaseInOut:
        // 指数出入
        // [0,0.5)      f(x) = (1/2)2^(10(2x - 1)) 
        // [0.5,1.f]    f(x) = -(1/2)*2^(-10(2x - 1))) + 1 
        if (p == 0.0f || p == 1.0f) return p;
        if (p < 0.5f) {
            return 0.5f * ::powf(2.f, (20.f * p) - 10.f);
        }
        else {
            return -0.5f * ::powf(2.f, (-20.f * p) + 1.f) + 1.f;
        }
    case LongUI::AnimationType::Type_ElasticEaseIn:
        // 弹性渐入
        return ::sin(13.f * EZ_PI_2 * p) * ::pow(2.f, 10.f * (p - 1.f));
    case LongUI::AnimationType::Type_ElasticEaseOut:
        // 弹性渐出
        return ::sin(-13.f * EZ_PI_2 * (p + 1.f)) * ::powf(2.f, -10.f * p) + 1.f;
    case LongUI::AnimationType::Type_ElasticEaseInOut:
        // 弹性出入
        if (p < 0.5f) {
            return 0.5f * ::sin(13.f * EZ_PI_2 * (2.f * p)) * ::pow(2.f, 10.f * ((2.f * p) - 1.f));
        }
        else {
            return 0.5f * (::sin(-13.f * EZ_PI_2 * ((2.f * p - 1.f) + 1.f)) * ::pow(2.f, -10.f * (2.f * p - 1.f)) + 2.f);
        }
    case LongUI::AnimationType::Type_BackEaseIn:
        // 回退渐入
        return  p * p * p - p * ::sin(p * EZ_PI);
    case LongUI::AnimationType::Type_BackEaseOut:
        // 回退渐出
    {
        register float f = (1.f - p);
        return 1.f - (f * f * f - f * ::sin(f * EZ_PI));
    }
    case LongUI::AnimationType::Type_BackEaseInOut:
        // 回退出入
        if (p < 0.5f) {
            register float f = 2.f * p;
            return 0.5f * (f * f * f - f * ::sin(f * EZ_PI));
        }
        else {
            register float f = (1.f - (2 * p - 1.f));
            return 0.5f * (1.f - (f * f * f - f * ::sin(f * EZ_PI))) + 0.5f;
        }
    case LongUI::AnimationType::Type_BounceEaseIn:
        // 反弹渐入
        return 1.f - ::BounceEaseOut(1.f - p);
    case LongUI::AnimationType::Type_BounceEaseOut:
        // 反弹渐出
        return ::BounceEaseOut(p);
    case LongUI::AnimationType::Type_BounceEaseInOut:
        // 反弹出入
        if (p < 0.5f) {
            return 0.5f * (1.f - ::BounceEaseOut(1.f - (p*2.f)));
        }
        else {
            return 0.5f * ::BounceEaseOut(p * 2.f - 1.f) + 0.5f;
        }
    }
}
