#include <cassert>
#include <algorithm>
#include <core/ui_color.h>
#include <util/ui_aniamtion.h>


extern "C" {
    // easing
    double ui_easing_function(uint32_t type, double p);
}

/// <summary>
/// Easings the function.
/// </summary>
/// <param name="type">The type.</param>
/// <param name="x">The x.</param>
/// <returns></returns>
auto LongUI::EasingFunction(AnimationType type, float x) noexcept -> float {
    const double new_x = x;
    const auto id = static_cast<uint32_t>(type);
    const auto y = ::ui_easing_function(id, new_x);
    return static_cast<float>(y);
}


/// <summary>
/// Indeterminates the value.
/// </summary>
/// <param name="value">The value.</param>
/// <param name="p">The p.</param>
/// <returns></returns>
auto LongUI::IndeterminateValue(SSFromTo value, float p) noexcept -> SSValue {
    assert(p >= 0.f && p <= 1.f && "out of range");
    SSValue rv;
    rv.type = value.from.type;
    rv.data.u32 = 0;
    const auto x0_0 = p;
    const auto x0_1 = 1.f - p;
    const auto do_float = [=](float from, float to) noexcept {
        return from * x0_1 + to * x0_0;
    };
    // 分类讨论
    switch (LongUI::GetEasyType(rv.type))
    {
    default:
        // [NO-ANIMATION]
        return { ValueType::Type_Unknown };
    case ValueEasyType::Type_Float:
        // [FLOAT]
        rv.data.single = do_float(
            value.from.data.single,
            value.to.data.single
        );
        break;
    case ValueEasyType::Type_Color:
        // [COLOR]
    {
#if 0
        RGBA from, to, target;
        from.primitive = value.from.data.u32;
        to.primitive = value.to.data.u32;
        // R
        target.u8.r = uint8_t(do_float(float(from.u8.r), float(to.u8.r)));
        // G
        target.u8.g = uint8_t(do_float(float(from.u8.g), float(to.u8.g)));
        // B
        target.u8.b = uint8_t(do_float(float(from.u8.b), float(to.u8.b)));
        // A
        target.u8.a = uint8_t(do_float(float(from.u8.a), float(to.u8.a)));
        // RGBA
        rv.data.u32 = target.primitive;
#else
        ColorF from, to;
        ColorF::FromRGBA_RT(from, { value.from.data.u32 });
        ColorF::FromRGBA_RT(to, { value.to.data.u32 });
        rv.data.u32 = LongUI::Mix(from, to, p).ToRGBA().primitive;
#endif
        break;
    }
#if 0
    case ValueEasyType::Type_Uint32:
        // [UINT32]
    {
        const double x0_0 = p;
        const double x0_1 = 1. - p;
        const double from = value.from.data.u32;
        const double to = value.to.data.u32;
        rv.data.u32 = static_cast<uint32_t>(
            from * x0_1 + to * x0_0)
            ;
        break;
    }
#endif
    }
    return rv;
}

/// <summary>
/// Cubics the bezier ex.
/// </summary>
/// <param name="t">The t.</param>
/// <param name="arg">The argument.</param>
/// <returns></returns>
//auto LongUI::CubicBezierEx(float t, uint32_t arg) noexcept -> float {
//    const float p0 = 0.f;
//    const float p1 = 0.f;
//    const float p2 = 1.f;
//    const float p3 = 1.f;
//    return
//        (1.f - t)*(1.f - t)*(1.f - t) * p0
//        + 3.f * (1.f - t)*(1.f - t) * t * p1
//        + 3.f * (1.f - t) * t*t * p2
//        + t * t*t * p3;
//
//}

/*/// <summary>
/// Initializes the specified type.
/// </summary>
/// <param name="type">The type.</param>
/// <returns></returns>
void LongUI::PODAnimation::Init(AnimationType a_type) noexcept {
    this->time = 0.f;
    this->duration = 0.233f;
    this->type = a_type;
    this->count = 1;
    //this->write     = nullptr;
    this->starts[0] = 0.f;
    this->ends[0] = 0.f;
}

/// <summary>
/// Updates the specified t.
/// </summary>
/// <param name="t">The t.</param>
/// <returns></returns>
void LongUI::PODAnimation::Update(float ) noexcept {
    assert(!"NOT IMPL");
    //float* output = nullptr;
    // 到点
    if (this->time <= 0.f) {
        std::memcpy(output, ends, sizeof(float) * count);
        return;
    }
    // 计算
    for (uint32_t i = 0; i < count; ++i) {
        const auto p = static_cast<double>(time / duration);
        const auto v = ui_easing_function(type, p);
        const auto fv = static_cast<float>(v);
        output[i] = fv * (starts[i] - ends[i]) + ends[i];
    }
    // 减少时间
    this->time -= t;
}*/