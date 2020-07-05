#include <algorithm>
#include <cstdarg>
#include <cwchar>
#include <core/ui_string.h>
#include <util/ui_endian.h>

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
    double ui_function_view_atof_le(const char* begin, const char* end, char step, char decimal) noexcept;
    // atoi
    int32_t ui_function_view_atoi_le(const char* begin, const char* end, char step) noexcept;
    // rgb32
    uint32_t ui_web_color_name_to_rgb32(const char* begin, const char* end, char step) noexcept;
    // argb32
    uint32_t ui_web_color_string_to_rgba32(const char* begin, const char* end, char step) noexcept;
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
template<> auto LongUI::PodStringView<char>::ToDouble(char decimal) const noexcept ->double {
    return ui_function_view_atof_le(this->first, this->second, sizeof(char), decimal);
}

/// <summary>
/// Implements the operator float.
/// </summary>
/// <returns>
/// The result of the operator.
/// </returns>
template<> auto LongUI::PodStringView<char16_t>::ToDouble(char decimal) const noexcept ->double {
    const auto chfirst = reinterpret_cast<const char*>(this->first);
    const auto chsecond = reinterpret_cast<const char*>(this->second);
    // TODO: helper::ascii_offset;
    union { char16_t u16; char u8[2]; }; u16 = '\1';
    const auto chfirstle = chfirst + u8[1];
    const auto dvalue = ui_function_view_atof_le(chfirstle, chsecond, sizeof(char16_t), decimal);
    return static_cast<float>(dvalue);
}

/// <summary>
/// Implements the operator int32_t.
/// </summary>
/// <returns>
/// The result of the operator.
/// </returns>
template<> auto LongUI::PodStringView<char>::ToInt32()const noexcept ->int32_t {
    return ui_function_view_atoi_le(this->first, this->second, sizeof(char));
}

/// <summary>
/// Implements the operator float.
/// </summary>
/// <returns>
/// The result of the operator.
/// </returns>
template<> auto LongUI::PodStringView<char16_t>::ToInt32()const noexcept ->int32_t {
    const auto chfirst = reinterpret_cast<const char*>(this->first);
    const auto chsecond = reinterpret_cast<const char*>(this->second);
    union { char16_t u16; char u8[2]; }; u16 = '\1';
    const auto chfirstle = chfirst + u8[1];
    return ui_function_view_atoi_le(chfirstle, chsecond, sizeof(wchar_t));
}

/// <summary>
/// range assert
/// </summary>
/// <param name=""></param>
/// <param name=""></param>
/// <returns></returns>
void LongUI::detail::range_assert(uint32_t a, uint32_t b) noexcept {
    assert(a < b && "a must less than b");
}