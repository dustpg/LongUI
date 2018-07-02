#include <algorithm>
#include <cstdarg>
#include <cwchar>
#include <core/ui_string.h>

PCN_NOINLINE
/// <summary>
/// Strings the assign.
/// </summary>
/// <param name="str">The string.</param>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <returns></returns>
void LongUI::detail::string_helper::string_assign(
    base_str& str,
    const char* begin, 
    const char* end) noexcept {
    // 计算长度
    const auto bytelen = str.m_uByteLen;
    const auto len = (end - begin) / bytelen;
    assert(len >= 0 && "bad string length");
    str.reserve(len);
    // 内存不足
    if (!str.is_ok()) return;
    // 部署数据
    str.assign(begin, end);
    // 添加NUL字符
    string_set_null_char(str.end(), bytelen);
}



PCN_NOINLINE
/// <summary>
/// Strings the insert.
/// </summary>
/// <param name="str">The string.</param>
/// <param name="pos">The position.</param>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <returns></returns>
void LongUI::detail::string_helper::string_insert(
    base_str& str,
    uintptr_t pos,
    const char* begin,
    const char* end) noexcept {
    const decltype(pos) endpos = str.size();
    assert(pos <= endpos || pos == uintptr_t(-1) && "out of range");
    // 计算长度
    const auto bytelen = str.m_uByteLen;
    const auto len = (end - begin) / bytelen;
    assert(len >= 0 && "bad string length");
    // 长度不够
    if (str.capacity() - 1 < (str.size() + len)) {
        // 尝试申请
        str.reserve((str.size() + len) * 3 / 2);
        // 内存不足
        if (!str.is_ok()) return;
    }
    // 插入数据
    str.insert(std::min(endpos, pos), begin, end);
    // 添加NUL字符
    string_set_null_char(str.end(), bytelen);
}

PCN_NOINLINE
/// <summary>
/// Strings the erase.
/// </summary>
/// <param name="str">The string.</param>
/// <param name="pos">The position.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
void LongUI::detail::string_helper::string_erase(
    base_str& str, size_t pos, size_t len) noexcept {
    // 内存有效
    if (str.is_ok()) {
        assert(pos <= str.size() && "out of range");
        // 删除指定范围数据
        str.erase(pos, (len == CUIStringU8::npos) ? str.size() : pos + len);
        // 设置NUL字符
        const auto bytelen = str.m_uByteLen;
        string_set_null_char(str.end(), bytelen);
    }
}



PCN_NOINLINE
/// <summary>
/// Strings the format.
/// </summary>
/// <param name="str">The string.</param>
/// <param name="fmt">The FMT.</param>
/// <param name="">The .</param>
/// <returns></returns>
bool LongUI::detail::string_helper::string_format(
    base_str& str, const char* fmt, ...) noexcept {
    // 内存不足
    if (!str.is_ok()) return false;
    // 初始化必要数据
    const auto bytesize = str.m_uByteLen;
    va_list args;
    va_start(args, fmt);
    int code = -1;
    // char
    if (bytesize == sizeof(char)) {
        code = std::vsnprintf(str.m_pData, str.m_uVecCap, fmt, args);
    }
    // wchar
    else if (bytesize == sizeof(wchar_t)) {
        const auto buffer = reinterpret_cast<wchar_t*>(str.m_pData);
        const auto fmtwch = reinterpret_cast<const wchar_t*>(fmt);
        code = std::vswprintf(nullptr, 0, fmtwch, args);
        if (code > 0 && static_cast<uint32_t>(code) < str.m_uVecCap)
            code = std::vswprintf(buffer, str.m_uVecCap, fmtwch, args);
    }
    else { assert(!"support char / wchar_t only"); }
    // 错误duang言
    assert(code > 0 && "bad format");
    // 错误检查: 直接清空字符串
    if (code < 0) goto on_error;
    // 检查缓存
    /*if (static_cast<uint32_t>(code) > str.m_uVecCap)*/

    // 申请内存
    str.reserve(static_cast<uint32_t>(code));
    // 内存不足
    if(!str.is_ok()) goto on_error;
    // char
    if (bytesize == sizeof(char)) {
        const auto newcode = std::vsnprintf(str.m_pData, str.m_uVecCap, fmt, args);
        newcode; assert(newcode == code);
    }
    // wchar
    else {
        const auto buffer = reinterpret_cast<wchar_t*>(str.m_pData);
        const auto fmtwch = reinterpret_cast<const wchar_t*>(fmt);
        const auto newcode = std::vswprintf(buffer, str.m_uVecCap, fmtwch, args);
        newcode; assert(newcode == code);
    }
    
    va_end(args);
    str.m_uVecLen = code;
    return true;
on_error:
    // 错误情况
    str.m_uVecLen = 0;
    string_set_null_char(str.m_pData, bytesize);
    return false;
}

// LongUI::detail
namespace LongUI { namespace detail {
    template<typename T>
    inline auto split_view(PodStringView<T>& view, T ch) noexcept ->PodStringView<T> {
        // 初始化
        auto token = view;  int offset = 0;
        token.first = nullptr;
        const auto*itr = view.first;
        const auto itr_end = view.second;
        // 遍历字符串
        while (itr != itr_end) {
            if (*itr == ch) { 
                if (token.first) {
                    offset = 1;
                    break;
                }
            }
            else if (!token.first) token.first = itr;
            ++itr;
        }
        // 写回数据
        token.second = itr;
        if (!token.first) token.first = token.second;
        view = { itr + offset, itr_end };
        return token;
    }
}}


/// <summary>
/// Strings the set null character.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="bytelen">The bytelen.</param>
/// <returns></returns>
void LongUI::detail::string_helper::string_set_null_char(
    void * pos, size_t bytelen) noexcept {
#if 0
    // utf-8?
    if (bytelen == 1) std::memset(pos, 0, 1);
    // utf-16?
    else if (bytelen == 2) std::memset(pos, 0, 2);
    // utf-32?
    else if (bytelen == 4) std::memset(pos, 0, 4);
    // utf-korea?
    else std::memset(pos, 0, bytelen);
#else
    std::memset(pos, 0, bytelen);
#endif
}


PCN_NOINLINE
/// <summary>
/// Copies as latin1.
/// </summary>
/// <param name="des">The DES.</param>
/// <param name="src_begin">The source begin.</param>
/// <param name="src_end">The source end.</param>
/// <returns></returns>
void LongUI::detail::string_helper::copy_from_latin1(
    wchar_t* des,
    const char* src_begin,
    const char* src_end) noexcept {
    [](wchar_t* __restrict des,
        const char* __restrict src_begin,
        const char* __restrict src_end) noexcept {
#if 1
        // 耗时: 1.00x
        auto len = (src_end - src_begin);
        while (len--) *des++ = (wchar_t)*src_begin++;
        /*const __m128i chunk = _mm_loadu_si128((const __m128i*)(src_begin + offset));
        const __m256i extended = _mm256_cvtepu8_epi16(chunk);
        _mm256_storeu_si256((__m256i*)(des + offset), extended);*/
#elif 1
        // 耗时: 1.33X
        while (src_begin != src_end) {
            *des = static_cast<wchar_t>(*src_begin);
            ++src_begin;
            ++des;
        }
#else
        // 耗时: 1.03X
        const auto len = (src_end - src_begin);
        using len_t = std::remove_const<decltype(len)>::type;
        for (len_t i = 0; i != len; ++i) {
            des[i] = src_begin[i];
        }
#endif
    }(des, src_begin, src_end);
}



template<>
PCN_NOINLINE
auto LongUI::PodStringView<char>::Split(char ch) noexcept -> PodStringView<char> {
    return detail::split_view(*this, ch);
}

template<>
PCN_NOINLINE
auto LongUI::PodStringView<wchar_t>::Split(wchar_t ch) noexcept -> PodStringView<wchar_t> {
    return detail::split_view(*this, ch);
}

PCN_NOINLINE
/// <summary>
/// Strings the split.
/// </summary>
/// <param name="str">The string.</param>
/// <param name="list">The list.</param>
/// <param name="tokenbegin">The tokenbegin.</param>
/// <param name="tokenend">The tokenend.</param>
/// <returns></returns>
void LongUI::detail::string_helper::string_split(
    const base_str& str, 
    POD::Vector<PodStringView<char>>& list,
    const char* tokenbegin, 
    const char* tokenend
) noexcept {
    using T = char;
    using view_t = PodStringView<T>;
    // 字符串有效
    if (str.is_ok()) {
        const auto*itr = str.m_pData;
        const auto itrend = itr + str.m_uVecLen * str.m_uByteLen;
        view_t view;
        view.first = itr;
        const size_t tokenlen = tokenend - tokenbegin;
        const auto step = str.m_uByteLen;
        // 遍历字符
        while (itr < itrend) {
            // 找到分割点
            if (!std::memcmp(itr, tokenbegin, tokenlen)) {
                view.second = itr;
                // 有效情况
                if (view.first != view.second) {
                    list.push_back(view);
                    if (!list.is_ok()) break;
                }
                itr += tokenlen;
                view.first = itr;
            }
            else {
                itr += step;
            }
        }
        // 最后一个字符
        assert(list.is_ok() && "ignore next assert");
        assert(itr == itrend && "bad string");
        view.second = itr;
        if (view.first != view.second) {
            list.push_back(view);
        }
    }
}


#ifdef OPTIMIZATION_STRING_SPLIT_WITHIN_SINGLE_CHAR
/// <summary>
/// Strings the split.
/// </summary>
/// <param name="ch">The ch.</param>
/// <param name="str">The string.</param>
/// <returns></returns>
template<typename T>
inline void LongUI::detail::string_helper::string_split(
    T ch, 
    base_str& str, 
    POD::Vector<PodStringView<T>>& list
) noexcept {
    using view_t = PodStringView<T>;
    // 字符串有效
    if (str.is_ok()) {
        auto itr = reinterpret_cast<const T*>(str.m_pData);
        const auto itrend = reinterpret_cast<const T*>(str.m_pData) + str.m_uVecLen;
        view_t view;
        view.first = itr;
        // 遍历字符
        while (itr < itrend) {
            // 找到分割点
            if (*itr == ch) {
                view.second = itr;
                // 有效情况
                if (view.first != view.second) {
                    list.push_back(view);
                    if (!list.is_ok()) break;
                }
                view.first = itr + 1;
            }
            ++itr;
        }
        // 最后一个字符
        view.second = itr;
        if (view.first != view.second) {
            list.push_back(view);
        }
    }
}

PCN_NOINLINE
/// <summary>
/// Strings the split.
/// </summary>
/// <param name="str">The string.</param>
/// <param name="ch">The ch.</param>
/// <returns></returns>
void LongUI::detail::string_helper::string_split(
    base_str& str, 
    POD::Vector<PodStringView<char>>& list,
    char ch) noexcept {
    string_split(ch, list, str);
}

PCN_NOINLINE
/// <summary>
/// Strings the split.
/// </summary>
/// <param name="str">The string.</param>
/// <param name="ch">The ch.</param>
/// <returns></returns>
void LongUI::detail::string_helper::string_split(
    base_str& str,
    POD::Vector<PodStringView<char>>& list,
    wchar_t ch) noexcept {
    string_split(ch, list, str);
}
#endif


extern "C" {
    // utf-8 -> utf-16
    uint32_t ui_utf8_to_utf16_get_buflen(const char* src, const char* end);
    // utf-8 -> utf-16
    uint32_t ui_utf8_to_utf16(
        char16_t* __restrict buf,
        uint32_t buflen,
        const char* __restrict src,
        const char* end
    );
}

/// <summary>
/// Strings the u8 u16.
/// </summary>
/// <param name="str">The string.</param>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <returns></returns>
void LongUI::detail::string_helper::string_u8_u16(
    base_str& str,
    const char* begin,
    const char* end) noexcept {
    assert(str.m_uByteLen == sizeof(char16_t));
    // 计算所需缓存大小
    const auto len = ::ui_utf8_to_utf16_get_buflen(begin, end);
    str.reserve(len);
    // 内存分配成功
    if (str.is_ok()) {
        const auto ptr = reinterpret_cast<char16_t*>(str.m_pData);
        ::ui_utf8_to_utf16(ptr, len, begin, end);
        ptr[str.m_uVecLen = len] = 0;
    }
}