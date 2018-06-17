#include <cassert>
#include <util/ui_color_system.h>

// c function list
extern "C" {
    // hsla to rgba
    void ui_function_hsla_to_rgba_float32(const float hsla[4], float rgba[4]);
}

#if 0
/// <summary>
/// RGBA to HSLA
/// </summary>
/// <returns></returns>
LongUI::CS::HSLA LongUI::CS::RGBA::toHSLA() const noexcept {
    assert(!"NOT IMPL");
    return{};
}

/// <summary>
/// RGBA To the HSVA.
/// </summary>
/// <returns></returns>
LongUI::CS::HSVA LongUI::CS::RGBA::toHSVA() const noexcept {
    assert(!"NOT IMPL");
    return{};
}

/// <summary>
/// HSVA to HSLA
/// </summary>
/// <returns></returns>
LongUI::CS::HSLA LongUI::CS::HSVA::toHSLA() const noexcept {
    assert(!"NOT IMPL");
    /*hsla.h = this->h;
    hsla.s = this->s
    hsla.l = this->v;
    hsla.a = this->a;*/
    return{};
}

#endif


/// <summary>
/// HSLA to RGBA
/// </summary>
/// <returns></returns>
LongUI::CS::RGBA LongUI::CS::HSLA::toRGBA() const noexcept {
    RGBA rgba;
    ui_function_hsla_to_rgba_float32(&this->h, &rgba.r);
    return rgba;
}