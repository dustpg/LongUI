#include <stdint.h>
#include <assert.h>
#include <util/ui_unimacro.h>

// HSVA
typedef struct {
    float h;       // ∈ [0, 360]
    float s;       // ∈ [0, 1]
    float v;       // ∈ [0, 1]
    float a;       // ∈ [0, 1]
} hsva_t;

// HSLA
typedef struct {
    float h;       // ∈ [0, 360]
    float s;       // ∈ [0, 1]
    float l;       // ∈ [0, 1]
    float a;       // ∈ [0, 1]
} hsla_t;

// RGBA
typedef struct {
    float r;       // ∈ [0, 1]
    float g;       // ∈ [0, 1]
    float b;       // ∈ [0, 1]
    float a;       // ∈ [0, 1]
} rgba_t;

PCN_NOINLINE
/// <summary>
/// UIs the function HSL to RGB float32.
/// </summary>
/// <param name="data">The data.</param>
void ui_function_hsla_to_rgba_float32(const float hsla[4], float rgba[4]) {
    // 将A弄过去
    rgba[3] = hsla[3];
    // 准备数据
    const float h = hsla[0];
    assert(0.f <= h && h < 360.f && "out of range");
    const float s = hsla[1];
    const float l = hsla[2];
    float *r = rgba + 0;
    float *g = rgba + 1;
    float *b = rgba + 2;
    // Q
    const float v = (l <= 0.5f) ? (l * (1.0f + s)) : (l + s - l * s);
    // 黑色
    if (v <= 0.0f) {
        *r = *g = *b = 0.f;
    }
    // 其他
    else {
        const float m = l + l - v;
        const float sv = (v - m) / v;
        const int sextant = (int)(h / 60.0f);
        const float fract = h / 60.0f - sextant;
        const float vsf = v * sv * fract;
        const float mid1 = m + vsf;
        const float  mid2 = v - vsf;
        switch (sextant) {
        case 0: *r = v; *g = mid1; *b = m; break;
        case 1: *r = mid2; *g = v; *b = m; break;
        case 2: *r = m; *g = v; *b = mid1; break;
        case 3: *r = m; *g = mid2; *b = v; break;
        case 4: *r = mid1; *g = m; *b = v; break;
        case 5: *r = v; *g = m; *b = mid2; break;
        }
    }
}