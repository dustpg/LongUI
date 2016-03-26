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

#ifdef LONGUI_DOUBLECLICK_WITH_POINT
// 双击
LongUINoinline bool LongUI::Helper::DoubleClick::Click(float x, float y) noexcept {
    auto now = ::timeGetTime();
    bool result = ((now - this->last) <= time) && x == this->ptx && y == this->pty;
    this->last = result ? 0ui32 : now;
    this->ptx = pt.x;
    this->pty = pt.y;
    return result;
}
#else
// 双击
LongUINoinline bool LongUI::Helper::DoubleClick::Click() noexcept {
    auto now = ::timeGetTime();
    bool result = ((now - this->last) <= time) ;
    this->last = result ? 0ui32 : now;
    return result;
}
#endif

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
                auto code = std::swprintf(buf, buf_len - 1, L"%ls\\%ls", folder, fileinfo.cFileName);
                // 缓存不足
                if (code < 0) break;
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
                        assert(size == 0 && "bad string given!");
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
                        assert(size == 0 && "bad string given!");
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

// 命名空间
namespace LongUI { namespace Helper {
    // 创建浮点
    LongUINoinline bool MakeFloats(const char* str, float fary[], uint32_t size) noexcept {
        // 检查字符串
        if (!str || !*str) return false;
        impl::make_units<','>([](float* out, const char* begin, const char* end) noexcept {
            auto len = static_cast<size_t>(end - begin);
            char buf[128]; assert(len < lengthof(buf));
            std::memcpy(buf, begin, len); len[buf] = 0;
            *out = LongUI::AtoF(buf);
        }, str, fary, size);
        return true;
    }
    // 创建整数
    LongUINoinline bool MakeInts(const char* str, int iary[], uint32_t size) noexcept {
        // 检查字符串
        if (!str || !*str) return false;
        impl::make_units<','>([](int* out, const char* begin, const char* end) noexcept {
            auto len = static_cast<size_t>(end - begin);
            char buf[128]; assert(len < lengthof(buf));
            std::memcpy(buf, begin, len); len[buf] = 0;
            *out = LongUI::AtoI(buf);
        }, str, iary, size);
        return true;
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
        // 计算结果
        auto result = Helper::MakeFloats(Helper::XMLGetValue(node, "metagroup", prefix), tmp, count);
        // 转换数据
        for (uint32_t i = 0; i < count; ++i) fary[i] = static_cast<uint16_t>(tmp[i]);
        // 返回结果
        return result;
    }
}}

// 16进制
unsigned int LongUI::Hex2Int(char c) noexcept {
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    else return c - '0';
}

// 获取颜色表示
bool LongUI::Helper::MakeColor(const char* data, D2D1_COLOR_F& color) noexcept {
    if (!data || !*data) return false;
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
    // 浮点数组
    else {
        return Helper::MakeFloats(data, reinterpret_cast<float*>(&color), 4);
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

