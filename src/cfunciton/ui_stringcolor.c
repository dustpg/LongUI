// web color string to argb32
#include <stdint.h>
#include <assert.h>
#include <limits.h>

// color-name to rgb32
extern uint32_t 
ui_web_color_name_hash_to_rgb32(uint32_t hash);
// hash
extern uint32_t
ui_hash_ignore_case(const char* begin, const char* end, char step);
// hex-char to int
extern char
ui_hex_to_int(char ch);

#define HEX16_BIT 4
typedef union { uint32_t pri; uint8_t rgba[4]; } color_t;

#define fallthrough

/// <summary>
/// web color string to RGBA32(RGBA byte-order).
/// </summary>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <param name="step">The step.</param>
/// <remarks>
/// color format support:
/// [O] keywords colors
/// [O] transparent
/// [O] rebeccapurple
/// [O] #RRGGBB, #RGB
/// [O] #RRGGBBAA, #RGBA
/// [X] rgb(), rgba()
/// [X] hsl(), hsla()
/// [X] system-color [context-sensitive]
/// [X] currentColor [context-sensitive]
/// </remarks>
/// <returns></returns>
uint32_t ui_web_color_string_to_rgba32(const char* begin, const char* end, char step) {
    assert((end - begin) % step == 0 && "bad string");
    // #开头的16进制
    if (*begin == '#') {
        const int len = (int)(end - begin) / step;
        //char R1, R2, G1, G2, B1, B2, A1, A2;
        color_t color;
        switch (len)
        {
        case 4:
            // #RGB
            color.rgba[3] = 0xff;
            fallthrough;
        case 5:
            // #RGBA
        {
            unsigned char r = ui_hex_to_int(begin[step * 1]);
            color.rgba[0] = (r << HEX16_BIT) | r;
            unsigned char g = ui_hex_to_int(begin[step * 2]);
            color.rgba[1] = (g << HEX16_BIT) | g;
            unsigned char b = ui_hex_to_int(begin[step * 3]);
            color.rgba[2] = (b << HEX16_BIT) | b;
            if (len == 5) {
                unsigned char a = ui_hex_to_int(begin[step * 3]);
                color.rgba[3] = (a << HEX16_BIT) | a;
            }
            break;
        }
        case 7:
            // #RRGGBB
            color.rgba[3] = 0xff;
            fallthrough;
        case 9:
            // #RRGGBBAA
        {
            unsigned char r1 = ui_hex_to_int(begin[step * 1]);
            unsigned char r2 = ui_hex_to_int(begin[step * 2]);
            color.rgba[0] = (r1 << HEX16_BIT) | r2;
            unsigned char g1 = ui_hex_to_int(begin[step * 3]);
            unsigned char g2 = ui_hex_to_int(begin[step * 4]);
            color.rgba[1] = (g1 << HEX16_BIT) | g2;
            unsigned char b1 = ui_hex_to_int(begin[step * 5]);
            unsigned char b2 = ui_hex_to_int(begin[step * 6]);
            color.rgba[2] = (b1 << HEX16_BIT) | b2;
            if (len == 9) {
                unsigned char a1 = ui_hex_to_int(begin[step * 7]);
                unsigned char a2 = ui_hex_to_int(begin[step * 8]);
                color.rgba[3] = (a1 << HEX16_BIT) | a2;
            }
            break;
        }
        default:
            assert(!"bad hexadecimal color");
            color.pri = 0;
            break;
        }
        return color.pri;
    }
    // TODO: 函数调用(百分号/数字 空格/逗号 百分号/数字)

    // 颜色名称
    else {
        // hash("transparent") = 0x062e7992
        // 无视大小写计算HASH
        const uint32_t hash = ui_hash_ignore_case(begin, end, step);
        // 透明不包括在RGB32里面
        if (hash == 0x062e7992ul) return 0;
        // 将RGB32的A置为FF
#if 1
        //color_t alpha; alpha.pri = 0; alpha.rgba[3] = 0xff;
        //const uint32_t mask = alpha.pri;
        return ui_web_color_name_hash_to_rgb32(hash) | 0xff000000ul;
#else
        color_t color;
        color.pri = ui_web_color_name_hash_to_rgb32(hash);
        color.rgba[3] = 0xff;
        return color.pri;
#endif
    }
    //return 0;
}

