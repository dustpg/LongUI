#include "luibase.h"
#include "luiconf.h"
#include "Platonly/luiPoUtil.h"
#include "Platonly/luiPoHlper.h"
#include "Platless/luiPlUtil.h"
#include "Platless/luiPlHlper.h"
#include "Graphics/luiGrD2d.h"
#include "LongUI/luiUiXml.h"
#ifdef _DEBUG
#include "Core/luiManager.h"
#endif


// longui
namespace LongUI {
    /// <summary>
    /// Beeps the error.
    /// </summary>
    /// <returns></returns>
    void BeepError() noexcept {
         ::MessageBeep(MB_ICONERROR); 
    }
}

// 双击
LongUINoinline bool LongUI::Helper::DoubleClick::Click(long x, long y) noexcept {
    auto now = ::timeGetTime();
    bool result = ((now - this->last) <= time) && x == this->ptx && y == this->pty;
    this->last = result ? 0ui32 : now;
    this->ptx = x;
    this->pty = y;
    return result;
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
LongUINoinline bool LongUI::Helper::Timer::Update() noexcept {
    // 当前
    const uint32_t now = ::timeGetTime();
    // 结尾时间
    const uint32_t end = m_dwTime + m_dwLastCount;
    // 是否过了时间
    if (now > end) {
        // 添加上去
        m_dwLastCount = now;// (now - end > m_dwTime) ? now : end;
        // 确定
        return true;
    }
    return false;
}

// longui::impl 命名空间
namespace LongUI { namespace impl { 
    // 申请全局字符串
    template<typename T> 
    inline auto alloc_global_string(const T* src, size_t len) noexcept {
        // 申请
        auto global = ::GlobalAlloc(GMEM_MOVEABLE, sizeof(T)*(len + 1));
        // 有效?
        if (global) {
            // 锁住!
            auto* des = reinterpret_cast<T*>(::GlobalLock(global));
            // 申请全局内存成功
            if (des) {
                // 复制
                std::memcpy(des, src, sizeof(T)*(len));
                // null结尾
                des[len] = 0;
                // 解锁
                ::GlobalUnlock(global);
            }
#ifdef _DEBUG
            else {
                UIManager << DL_Error
                    << L" GlobalLock --> Failed"
                    << LongUI::endl;
            }
#endif
        }
#ifdef _DEBUG
        else {
            UIManager << DL_Error
                << L" GlobalAlloc --> Failed, try alloc from"
                << Formated(L"%p in %zu bytes", src, len)
                << LongUI::endl;
        }
#endif
        return global;
    }
}}

// 申请全局字符串
LongUINoinline auto LongUI::Helper::GlobalAllocString(const wchar_t* src, size_t len) noexcept ->HGLOBAL {
    return impl::alloc_global_string(src, len);
}

// 申请全局字符串
LongUINoinline auto LongUI::Helper::GlobalAllocString(const char* src, size_t len) noexcept ->HGLOBAL {
    return impl::alloc_global_string(src, len);
}

// 查找多个文件
LongUINoinline auto LongUI::Helper::FindFilesToBuffer(
    wchar_t* buf, size_t buf_len, 
    const wchar_t* folder,
    const wchar_t* name
    ) noexcept ->wchar_t* {
    // 初始化
    WIN32_FIND_DATAW fileinfo;
#ifdef _DEBUG
    std::memset(&fileinfo, 0, sizeof(fileinfo));
#endif
    HANDLE hFile = INVALID_HANDLE_VALUE;
    ::SetLastError(0);
    {
        wchar_t path[MAX_PATH * 2]; path[0] = 0;
        std::swprintf(path, lengthof(path), L"%ls\\%ls", folder, name);
        hFile = ::FindFirstFileW(path, &fileinfo);
    }
    // 有效
    if (hFile != INVALID_HANDLE_VALUE) {
        // 清空
        DWORD errorcode = ::GetLastError();
        // 遍历文件
        while (errorcode != ERROR_NO_MORE_FILES) {
            // 跳过.开头
            if (fileinfo.cFileName[0] != '.') {
                // 写入
                auto code = std::swprintf(buf, buf_len, L"%ls\\%ls", folder, fileinfo.cFileName);
                // 缓存不足
                if (code >= (decltype(code))(buf_len)) break;
                // 写入
                buf += code + 1; buf[0] = 0; buf_len -= code + 1;
            }
            // 推进
            ::FindNextFileW(hFile, &fileinfo);
            // 检查
            errorcode = ::GetLastError();
        }
    }
    ::FindClose(hFile);
    return buf;
}



// longui::impl 命名空间
namespace LongUI { namespace impl {
    template<size_t C> struct make_units_helper { };
    // 创建单元帮助器
    template<> struct make_units_helper<2> { 
        // 创建单元
        template<char32_t SEPARATOR, typename CHAR_TYPE, typename OUT_TYPE, typename Lam>
        static LongUIInline void make_units(Lam caster, const CHAR_TYPE* str, OUT_TYPE* units, uint32_t size) noexcept {
            // 参数检查
            assert(str && units && size && "bad arguments");
            // 数据
            CHAR_TYPE buf[LongUIStringBufferLength]; auto itr = buf;
            // 遍历
            while (size) {
                auto ch = (*str);
                // 获取到了分隔符号
                if (ch == static_cast<CHAR_TYPE>(SEPARATOR) || ch == 0) {
                    *itr = 0;
                    caster(units++, buf);
                    itr = buf; --size;
                    if (ch == 0) {
#ifdef _DEBUG
                        if (size != 0) {
                            UIManager << DL_Warning
                                << L"bad string given!"
                                << LongUI::endl;
                        }
#endif
                        break;
                    }
                }
                // 继续复制
                else {
                    *itr = ch;
                    ++itr;
                    assert(itr >= std::end(buf) && "buffer to small");
                }
                ++str;
            }
        }
    };
    // 创建单元帮助器
    template<> struct make_units_helper<3> { 
        // 创建单元
        template<char32_t SEPARATOR, typename CHAR_TYPE, typename OUT_TYPE, typename Lam>
        static LongUIInline void make_units(Lam caster, const CHAR_TYPE* str, OUT_TYPE* units, uint32_t size) noexcept {
            // 参数检查
            assert(str && units && size && "bad arguments");
            // 数据
            auto old = str;
            // 遍历
            while (size) {
                // 获取到了分隔符号
                auto ch = (*str);
                if (ch == static_cast<CHAR_TYPE>(SEPARATOR) || ch == 0) {
                    caster(units++, old, str);
                    old = str + 1; --size;
                    if (ch == 0) {
#ifdef _DEBUG
                        if (size != 0) {
                            UIManager << DL_Warning
                                << L"bad string given!"
                                << LongUI::endl;
                        }
#endif
                        break;
                    }
                }
                ++str;
            }
        }
    };
    // 创建单元
    template<char32_t SEPARATOR, typename CHAR_TYPE, typename OUT_TYPE, typename Lam>
    inline void make_units(Lam caster, const CHAR_TYPE* str, OUT_TYPE units[], uint32_t size) noexcept {
        using caster_type = Helper::type_helper<Lam>;
        return make_units_helper<caster_type::arity>::make_units<SEPARATOR>(caster, str, units, size);
    }
}}


// Outline Text Renderer : context
namespace LongUI {

}

// 命名空间
namespace LongUI { namespace Helper {
    // 创建浮点
    LongUINoinline auto MakeFloats(const char* str, float fary[], uint32_t size) noexcept -> const char*{
        // 检查字符串
        if (!str || !*str) return str;
        const char* rcode = str;
        impl::make_units<','>([&rcode](float* out, const char* begin, const char* end) noexcept {
            auto len = static_cast<size_t>(end - begin);
            char buf[128]; assert(len < lengthof(buf));
            std::memcpy(buf, begin, len); len[buf] = 0;
            *out = LongUI::AtoF(buf);
            rcode = end;
        }, str, fary, size);
        return rcode;
    }
    // 创建整数
    LongUINoinline auto MakeInts(const char* str, int iary[], uint32_t size) noexcept -> const char* {
        // 检查字符串
        if (!str || !*str) return str;
        const char* rcode = str;
        impl::make_units<','>([&rcode](int* out, const char* begin, const char* end) noexcept {
            auto len = static_cast<size_t>(end - begin);
            char buf[128]; assert(len < lengthof(buf));
            std::memcpy(buf, begin, len); len[buf] = 0;
            *out = LongUI::AtoI(buf);
            rcode = end;
        }, str, iary, size);
        return rcode;
    }
    // 颜色属性名字符串集
    const char* const COLOR_BUTTON[] = {
        "disabledcolor", "normalcolor", "hovercolor", "pushedcolor"
    };
    /// <summary>
    ///  创建基于状态的颜色数组
    /// </summary>
    /// <param name="node">pugixml 节点</param>
    /// <param name="prefix">颜色属性前缀</param>
    /// <param name="color">颜色数组</param>
    /// <returns></returns>
    LongUINoinline bool MakeStateBasedColor(pugi::xml_node node, const char* prefix, D2D1_COLOR_F color[4]) noexcept {
        // 初始值
        if (color->a < 0.0f || color->a > 1.f) {
            color[State_Disabled]   = D2D1::ColorF(0xDEDEDEDEui32);
            color[State_Normal]     = D2D1::ColorF(0xCDCDCDCDui32);
            color[State_Hover]      = D2D1::ColorF(0xA9A9A9A9ui32);
            color[State_Pushed]     = D2D1::ColorF(0x78787878ui32);
        }
        bool rc = false;
        // 循环设置
        for (int i = 0; i < STATE_COUNT; ++i) {
            rc = rc | Helper::MakeColor(Helper::XMLGetValue(node, COLOR_BUTTON[i], prefix), color[i]);
        }
        return rc;
    }
    /// <summary>
    /// 创建meta组
    /// </summary>
    /// <param name="node">xml node</param>
    /// <param name="prefix">属性名前缀</param>
    /// <param name="fary">float array of meta id</param>
    /// <param name="count">count of it</param>
    /// <returns>成功设置则返回true, 没有或者错误返回false</returns>
    bool MakeMetaGroup(pugi::xml_node node, const char* prefix, uint16_t fary[], uint32_t count) noexcept {
        // 检查
        constexpr int BUFFER_COUNT = 64; float tmp[BUFFER_COUNT];
        assert(count < BUFFER_COUNT && "out of buffer length");
        // 初始化
        std::memset(tmp, 0, sizeof(tmp));
        // 字符串
        auto str = Helper::XMLGetValue(node, "metagroup", prefix);
        // 计算结果
        auto result = Helper::MakeFloats(str, tmp, count);
        // 转换数据
        for (uint32_t i = 0; i < count; ++i) fary[i] = static_cast<uint16_t>(tmp[i]);
        // 返回结果
        return result != str;
    }
}}

// 16进制
unsigned int LongUI::Hex2Int(char c) noexcept {
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    else return c - '0';
}

#include <algorithm>

// longui namepsace
namespace LongUI {
    // color name
    struct ColorName { const char* name; uint32_t value; };
    // color array
    const ColorName COLOR_ARRAY[] = {
        { "aliceblue", 0xf0f8ff },
        { "antiquewhite", 0xfaebd7 },
        { "aqua", 0x00ffff },
        { "aquamarine", 0x7fffd4 },
        { "azure", 0xf0ffff },
        { "beige", 0xf5f5dc },
        { "bisque", 0xffe4c4 },
        { "black", 0x000000 },
        { "blanchedalmond", 0xffebcd },
        { "blue", 0x0000ff },
        { "blueviolet", 0x8a2be2 },
        { "brown", 0xa52a2a },
        { "burlywood", 0xdeb887 },
        { "cadetblue", 0x5f9ea0 },
        { "chartreuse", 0x7fff00 },
        { "chocolate", 0xd2691e },
        { "coral", 0xff7f50 },
        { "cornflowerblue", 0x6495ed },
        { "cornsilk", 0xfff8dc },
        { "crimson", 0xdc143c },
        { "cyan", 0x00ffff },
        { "darkblue", 0x00008b },
        { "darkcyan", 0x008b8b },
        { "darkgoldenrod", 0xb8860b },
        { "darkgray", 0xa9a9a9 },
        { "darkgreen", 0x006400 },
        { "darkkhaki", 0xbdb76b },
        { "darkmagenta", 0x8b008b },
        { "darkolivegreen", 0x556b2f },
        { "darkorange", 0xff8c00 },
        { "darkorchid", 0x9932cc },
        { "darkred", 0x8b0000 },
        { "darksalmon", 0xe9967a },
        { "darkseagreen", 0x8fbc8f },
        { "darkslateblue", 0x483d8b },
        { "darkslategray", 0x2f4f4f },
        { "darkturquoise", 0x00ced1 },
        { "darkviolet", 0x9400d3 },
        { "deeppink", 0xff1493 },
        { "deepskyblue", 0x00bfff },
        { "dimgray", 0x696969 },
        { "dodgerblue", 0x1e90ff },
        { "firebrick", 0xb22222 },
        { "floralwhite", 0xfffaf0 },
        { "forestgreen", 0x228b22 },
        { "fuchsia", 0xff00ff },
        { "gainsboro", 0xdcdcdc },
        { "ghostwhite", 0xf8f8ff },
        { "gold", 0xffd700 },
        { "goldenrod", 0xdaa520 },
        { "gray", 0x808080 },
        { "green", 0x008000 },
        { "greenyellow", 0xadff2f },
        { "honeydew", 0xf0fff0 },
        { "hotpink", 0xff69b4 },
        { "indianred", 0xcd5c5c },
        { "indigo", 0x4b0082 },
        { "ivory", 0xfffff0 },
        { "khaki", 0xf0e68c },
        { "lavender", 0xe6e6fa },
        { "lavenderblush", 0xfff0f5 },
        { "lawngreen", 0x7cfc00 },
        { "lemonchiffon", 0xfffacd },
        { "lightblue", 0xadd8e6 },
        { "lightcoral", 0xf08080 },
        { "lightcyan", 0xe0ffff },
        { "lightgoldenrodyellow", 0xfafad2 },
        { "lightgreen", 0x90ee90 },
        { "lightgray", 0xd3d3d3 },
        { "lightpink", 0xffb6c1 },
        { "lightsalmon", 0xffa07a },
        { "lightseagreen", 0x20b2aa },
        { "lightskyblue", 0x87cefa },
        { "lightslategray", 0x778899 },
        { "lightsteelblue", 0xb0c4de },
        { "lightyellow", 0xffffe0 },
        { "lime", 0x00ff00 },
        { "limegreen", 0x32cd32 },
        { "linen", 0xfaf0e6 },
        { "magenta", 0xff00ff },
        { "maroon", 0x800000 },
        { "mediumaquamarine", 0x66cdaa },
        { "mediumblue", 0x0000cd },
        { "mediumorchid", 0xba55d3 },
        { "mediumpurple", 0x9370db },
        { "mediumseagreen", 0x3cb371 },
        { "mediumslateblue", 0x7b68ee },
        { "mediumspringgreen", 0x00fa9a },
        { "mediumturquoise", 0x48d1cc },
        { "mediumvioletred", 0xc71585 },
        { "midnightblue", 0x191970 },
        { "mintcream", 0xf5fffa },
        { "mistyrose", 0xffe4e1 },
        { "moccasin", 0xffe4b5 },
        { "navajowhite", 0xffdead },
        { "navy", 0x000080 },
        { "oldlace", 0xfdf5e6 },
        { "olive", 0x808000 },
        { "olivedrab", 0x6b8e23 },
        { "orange", 0xffa500 },
        { "orangered", 0xff4500 },
        { "orchid", 0xda70d6 },
        { "palegoldenrod", 0xeee8aa },
        { "palegreen", 0x98fb98 },
        { "paleturquoise", 0xafeeee },
        { "palevioletred", 0xdb7093 },
        { "papayawhip", 0xffefd5 },
        { "peachpuff", 0xffdab9 },
        { "peru", 0xcd853f },
        { "pink", 0xffc0cb },
        { "plum", 0xdda0dd },
        { "powderblue", 0xb0e0e6 },
        { "purple", 0x800080 },
        { "red", 0xff0000 },
        { "rosybrown", 0xbc8f8f },
        { "royalblue", 0x4169e1 },
        { "saddlebrown", 0x8b4513 },
        { "salmon", 0xfa8072 },
        { "sandybrown", 0xf4a460 },
        { "seagreen", 0x2e8b57 },
        { "seashell", 0xfff5ee },
        { "sienna", 0xa0522d },
        { "silver", 0xc0c0c0 },
        { "skyblue", 0x87ceeb },
        { "slateblue", 0x6a5acd },
        { "slategray", 0x708090 },
        { "snow", 0xfffafa },
        { "springgreen", 0x00ff7f },
        { "steelblue", 0x4682b4 },
        { "tan", 0xd2b48c },
        { "teal", 0x008080 },
        { "thistle", 0xd8bfd8 },
        { "tomato", 0xff6347 },
        { "turquoise", 0x40e0d0 },
        { "violet", 0xee82ee },
        { "wheat", 0xf5deb3 },
        { "white", 0xffffff },
        { "whitesmoke", 0xf5f5f5 },
        { "yellow", 0xffff00 },
        { "yellowgreen", 0x9acd32 },
    };
    // hash table
    struct ColorTable { uint8_t a, b; };
    // table data
    struct ColorTableData {
        // length of hash buffer
        enum : uint32_t { HASH_LENGTH = 235 };
        // data
        ColorTable  data[HASH_LENGTH];
        // djb hash function, lower case version
        static auto DJBHash(const char* str) noexcept {
            uint32_t hash = 5381;
            while (*str) {
                auto lower_case = (uint32_t(*str) & 31) + 32*3;
                str++;
                hash += (hash << 5) + lower_case;
            }
            return (hash & 0x7FFFFFFF);
        }
        // is equal for string without case sensitivity
        static bool IsEqual(
            const char* __restrict a, 
            const char* __restrict b) noexcept {
            while (true) {
                char ca = ((*a) & 31) + 32 * 3;
                char cb = ((*b) & 31) + 32 * 3;
                if (ca != cb) return false;
                if (*a) return true;
                ++a; ++b;
            }
        }
        // get color
        auto GetColor(const char* str, uint32_t& color) noexcept {
            constexpr uint8_t bad_index = 255;
            auto index = this->DJBHash(str) % HASH_LENGTH;
            auto unit = data[index];
            if (unit.a == bad_index) return false;
            if (this->IsEqual(LongUI::COLOR_ARRAY[unit.a].name, str)) {
                color = LongUI::COLOR_ARRAY[unit.a].value;
                return true;
            }
            if (unit.b == bad_index) return false;
            color = LongUI::COLOR_ARRAY[unit.b].value;
            return true;
        }
        // ctor
        ColorTableData() noexcept {
            constexpr uint8_t bad_index = 255;
            std::memset(this->data, bad_index, sizeof(this->data));
            for (const auto& u : LongUI::COLOR_ARRAY) {
                auto index = this->DJBHash(u.name) % HASH_LENGTH;
                auto&table = data[index];
                assert(table.a == bad_index || table.b == bad_index && "bad hash");
                uint8_t pos = uint8_t(&u - LongUI::COLOR_ARRAY);
                (table.a == bad_index ? table.a : table.b) = pos;
            }
        }
    } g_table;
}

// 获取颜色表示
bool LongUI::Helper::MakeColor(const char* data, D2D1_COLOR_F& color) noexcept {
    if (!data || !*data) return false;
    uint32_t u32c;
    // 获取有效值
    while (white_space(*data)) ++data;
    // 以#开头?
    if (*data == '#') {
        color.a = 1.f;
        // #RGB
        if (white_space(data[4]) || !data[4]) {
            color.r = static_cast<float>(Hex2Int(*++data)) / 15.f;
            color.g = static_cast<float>(Hex2Int(*++data)) / 15.f;
            color.b = static_cast<float>(Hex2Int(*++data)) / 15.f;
        }
        // #RRGGBB
        else if (white_space(data[7]) || !data[7]) {
            color.r = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
            color.g = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
            color.b = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
        }
        // #AARRGGBB
        else {
            color.a = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
            color.r = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
            color.g = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
            color.b = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
        }
        return true;
    }
    // 颜色名称
    else if (LongUI::g_table.GetColor(data, u32c)) {
        LongUI::UnpackTheColorARGB(u32c, color);
        color.a = 1.f;
        return true;
    }
    // 浮点数组
    else {
        return Helper::MakeFloats(data, color) != data;
    }
}



/*/// <summary>
/// 创建字符串
/// </summary>
/// <param name="data">The data.</param>
/// <param name="str">The string.</param>
/// <returns></returns>
bool LongUI::Helper::MakeString(const char* data, CUIString& str) noexcept {
    if (!data || !*data) return false;
    LongUI::SafeUTF8toWideChar(
        data, [&str](const wchar_t* be, const wchar_t* ed) noexcept {
        str.Set(be, uint32_t(ed - be));
    });
    return true;
}*/


// longui::helper
LONGUI_NAMESPACE_BEGIN namespace Helper {
    // 边框
    const char* const BORDER_COLOR_ATTR[] = {
        "disabledbordercolor", "normalbordercolor",
        "hoverbordercolor",  "pushedbordercolor",
    };
}
LONGUI_NAMESPACE_END

// 设置边框颜色
void LongUI::Helper::SetBorderColor(pugi::xml_node node, D2D1_COLOR_F color[STATE_COUNT]) noexcept {
    Helper::SetBorderColor(color);
    // 必须有效
    assert(node && "no null");
    // 检查
    for (auto i = 0u; i < STATE_COUNT; ++i) {
        Helper::MakeColor(node.attribute(BORDER_COLOR_ATTR[i]).value(), color[i]);
    }
}

// 设置边框颜色
LongUINoinline void LongUI::Helper::SetBorderColor(D2D1_COLOR_F color[STATE_COUNT]) noexcept {
    // 边框颜色
    color[State_Disabled]   = D2D1::ColorF(0xBFBFBFui32);
    color[State_Normal]     = D2D1::ColorF(0xACACACui32);
    color[State_Hover]      = D2D1::ColorF(0x7EB4EAui32);
    color[State_Pushed]     = D2D1::ColorF(0x569DE5ui32);
}

