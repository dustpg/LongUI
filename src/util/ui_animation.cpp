#include <cassert>
#include <algorithm>
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