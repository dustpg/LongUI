#include <algorithm>
#include <cstdarg>
#include <cwchar>
#include <core/ui_string.h>

// TODO: ascii_offset 换成常量表达式
// TODO: C函数字符串处理函数全部换成LE(小端)后缀

PCN_NOINLINE
/// <summary>
/// Strlens the specified PTR.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <returns></returns>
auto LongUI::detail::strlen(const unwchar_t ptr[]) noexcept -> size_t {
    assert(ptr && "bad string pointer");
    return std::char_traits<unwchar_t>::length(ptr);
}


// atof view
extern "C" {
    // atof
    double ui_function_view_atof_le(const char* begin, const char* end, char step);
    // atoi
    int32_t ui_function_view_atoi_le(const char* begin, const char* end, char step);
    // rgb32
    uint32_t ui_web_color_name_to_rgb32(const char* begin, const char* end, char step);
    // argb32
    uint32_t ui_web_color_string_to_rgba32(const char* begin, const char* end, char step);
}


/// <summary>
/// Colors the RGB32.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <param name="c">The c.</param>
/// <returns></returns>
auto LongUI::detail::color_rgba32(const char * a, const char * b, char c) noexcept -> uint32_t {
    return ui_web_color_string_to_rgba32(a, b, c);
}

/// <summary>
/// color name to rgb32
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <param name="c">The c.</param>
/// <returns></returns>
auto LongUI::detail::name_rgb32(const char * a, const char * b, char c) noexcept -> uint32_t {
    return ui_web_color_name_to_rgb32(a, b, c);
}

/// <summary>
/// Implements the operator float.
/// </summary>
/// <returns>
/// The result of the operator.
/// </returns>
template<> LongUI::PodStringView<char>::operator float() const noexcept {
    const auto dvalue = ui_function_view_atof_le(this->first, this->second, sizeof(char));
    return static_cast<float>(dvalue);
}

/// <summary>
/// Implements the operator float.
/// </summary>
/// <returns>
/// The result of the operator.
/// </returns>
template<> LongUI::PodStringView<wchar_t>::operator float() const noexcept {
    const auto chfirst = reinterpret_cast<const char*>(this->first);
    const auto chsecond = reinterpret_cast<const char*>(this->second);
    const wchar_t ch = '\1';
    const auto chfirstle = chfirst + (sizeof(wchar_t)-1) * !(
        *reinterpret_cast<const char*>(&ch)
        );
    const auto dvalue = ui_function_view_atof_le(chfirstle, chsecond, sizeof(wchar_t));
    return static_cast<float>(dvalue);
}

/// <summary>
/// Implements the operator int32_t.
/// </summary>
/// <returns>
/// The result of the operator.
/// </returns>
template<> LongUI::PodStringView<char>::operator int32_t() const noexcept {
    return ui_function_view_atoi_le(this->first, this->second, sizeof(char));
}

/// <summary>
/// Implements the operator float.
/// </summary>
/// <returns>
/// The result of the operator.
/// </returns>
template<> LongUI::PodStringView<wchar_t>::operator int32_t() const noexcept {
    const auto chfirst = reinterpret_cast<const char*>(this->first);
    const auto chsecond = reinterpret_cast<const char*>(this->second);
    const wchar_t ch = '\1';
    const auto chfirstle = chfirst + (sizeof(wchar_t) - 1) * !(
        *reinterpret_cast<const char*>(&ch)
        );
    return ui_function_view_atoi_le(chfirstle, chsecond, sizeof(wchar_t));
}

