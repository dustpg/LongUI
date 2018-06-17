// std::atoi like funciton, but 3 args
#include <stdint.h>
#include <assert.h>
#include <util/ui_unimacro.h>

#define white_space(c) ((c) == ' ' || (c) == '\t')
#define valid_digit(c) ((c) >= '0' && (c) <= '9')
#define char_to_int(c) ((int32_t)(hex_lookup_table[(c) & 127]))

// digital lookup table for self
static const char hex_lookup_table[128]={
  // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 0
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 1
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 2
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,  // 3
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 4
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 5
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 6
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 7
};


/// <summary>
/// UIs the hexadecimal to int.
/// </summary>
/// <param name="ch">The ch.</param>
/// <returns></returns>
char ui_hex_to_int(char ch) {
    const char i = char_to_int(ch);
    assert(i != -1);
    return i;
}

PCN_NOINLINE
/// <summary>
/// UIs the function view atof.[little-endian]
/// </summary>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <returns></returns>
int32_t ui_function_view_atoi_le(const char* begin, const char* end, char step) {
    int32_t sign = 1, value, base = 10;
    const int cstep = step;
    // 跳过开头空白
    while (white_space(*begin) && begin < end) begin += cstep;
    // 结尾?
    if (begin == end) return 0;
    // 获取符号
    if (*begin == '-') { sign = -1; begin += cstep; }
    else if (*begin == '+') { begin += cstep; }
    // 0开头?
    else if (*begin == '0') { 
        begin += cstep;
        switch (*begin)
        {
        case 'x': case 'X': base = 16; begin += cstep; break;
        case 'b': case 'B': base = 2; begin += cstep; break;
        default: base = 8; break;
        }
    }
    // 跳过符号后空白
    while (white_space(*begin) && begin < end) begin += cstep;
    // 结尾?
    if (begin == end) return 0;
    // 获取小数点前面数字
    for (value = 0; valid_digit(*begin) && begin < end; begin += cstep) {
        // 十进制
        value = value * base + char_to_int(*begin);
        // 数据溢出
        if (value < 0) { value = 0; break; }
    }
    // 结尾
    return value * sign;
}

/// <summary>
/// UIs the function view atoi [big-endian]
/// </summary>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <param name="step">The step.</param>
/// <returns></returns>
int32_t ui_function_view_atoi_be(const char* begin, const char* end, char step) {
    return ui_function_view_atoi_le(begin + step - 1, end, step);
}

/// <summary>
/// UIs the function view atoi [same as cpu]
/// </summary>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <param name="step">The step.</param>
/// <returns></returns>
int32_t ui_function_view_atoi_cpu(const char* begin, const char* end, char step) {
    const int16_t endianness = 0x01;
    begin += (step - 1) * (!(*((const char*)&endianness)));
    return ui_function_view_atoi_be(begin , end, step);
}