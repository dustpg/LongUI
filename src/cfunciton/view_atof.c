// std::atof like funciton, but 3 args
#include <stdint.h>
#include <util/ui_unimacro.h>

#define white_space(c) ((c) == ' ' || (c) == '\t')
#define valid_digit(c) ((c) >= '0' && (c) <= '9')

PCN_NOINLINE
/// <summary>
/// UIs the function view atof.
/// </summary>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <returns></returns>
double ui_function_view_atof_le(const char* begin, const char* end, char step, char decimal) {
    int frac = 0; double sign = 1.0, value = 0.0, scale = 1.0;
    const int cstep = step;
    // 跳过开头空白
    while (white_space(*begin) && begin < end) begin += cstep;
    // 结尾?
    if (begin == end) return 0.0;
    // 获取符号
    if (*begin == '-') { sign = -1.0; begin += cstep; }
    else if (*begin == '+') { begin += cstep; }
    // 跳过符号后空白
    while (white_space(*begin) && begin < end) begin += cstep;
    // 结尾?
    if (begin == end) return 0.0;
    // 获取小数点前面数字
    for (value = 0.0; valid_digit(*begin) && begin < end; begin += cstep) {
        value = value * 10.0 + (*begin - '0');
    }
    // 结尾?
    if (begin == end) return value;
    // 获取小数点后面数据
    if (*begin == decimal) {
        double pow10 = 10.0;
        begin += cstep;
        while (valid_digit(*begin) && begin < end) {
            value += (*begin - '0') / pow10;
            pow10 *= 10.0;
            begin += cstep;
        }
    }
    // 结尾?
    if (begin < end) {
        // 指数?
        if ((*begin == 'e') || (*begin == 'E')) {
            unsigned int expon;
            // 获取指数符号
            begin += cstep;
            if (*begin == '-') { frac = 1; begin += cstep; }
            else if (*begin == '+') { begin += cstep; }
            // 获取指数数字
            for (expon = 0; valid_digit(*begin) && begin < end; begin += cstep) {
                expon = expon * 10 + (*begin - '0');
            }
            // 双精度浮点限制
            if (expon > 308) expon = 308;
            // 获取指数大小
            while (expon >= 50) { scale *= 1E50; expon -= 50; }
            while (expon >= 8) { scale *= 1E8;  expon -= 8; }
            while (expon > 0) { scale *= 10.0; expon -= 1; }
        }
    }
    // 返回数据
    return sign * (frac ? (value / scale) : (value * scale));
}


/// <summary>
/// UIs the function view atof [big-endian]
/// </summary>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <param name="step">The step.</param>
/// <param name="decimal">The decimal mark.</param>
/// <returns></returns>
double ui_function_view_atof_be(const char* begin, const char* end, char step, char decimal) {
    return ui_function_view_atof_le(begin + step - 1, end, step, decimal);
}

/// <summary>
/// UIs the function view atof [same as cpu]
/// </summary>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <param name="step">The step.</param>
/// <param name="decimal">The decimal mark.</param>
/// <returns></returns>
double ui_function_view_atof_cpu(const char* begin, const char* end, char step, char decimal) {
    const int16_t endianness = 0x01;
    if (!(*((const char*)&endianness))) begin += step - 1;
    return ui_function_view_atof_be(begin, end, step, decimal);
}