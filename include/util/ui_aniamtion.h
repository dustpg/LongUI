#pragma once

// c++
#include <cstdint>
// ui
#include "../util/ui_unimacro.h"
#include "../style/ui_style_state.h"
#include "../style/ui_ssvalue_list.h"

// ui namespace
namespace LongUI {
    // the type of aniamtion
    enum AnimationType : uint8_t {
        Type_LinearInterpolation = 0,   // 线性插值
        Type_QuadraticEaseIn,           // 平方渐入插值
        Type_QuadraticEaseOut,          // 平方渐出插值
        Type_QuadraticEaseInOut,        // 平方渐入渐出插值
        Type_CubicEaseIn,               // 立方渐入插值
        Type_CubicEaseOut,              // 立方渐出插值
        Type_CubicEaseInOut,            // 立方渐入渐出插值
        Type_QuarticEaseIn,             // 四次渐入插值
        Type_QuarticEaseOut,            // 四次渐出插值
        Type_QuarticEaseInOut,          // 四次渐入渐出插值
        Type_QuinticEaseIn,             // 五次渐入插值
        Type_QuinticEaseOut,            // 五次渐出插值
        Type_QuinticEaseInOut,          // 五次渐入渐出插值
        Type_SineEaseIn,                // 正弦渐入插值
        Type_SineEaseOut,               // 正弦渐出插值
        Type_SineEaseInOut,             // 正弦渐入渐出插值
        Type_CircularEaseIn,            // 四象圆弧插值
        Type_CircularEaseOut,           // 二象圆弧插值
        Type_CircularEaseInOut,         // 圆弧渐入渐出插值
        Type_ExponentialEaseIn,         // 指数渐入插值
        Type_ExponentialEaseOut,        // 指数渐出插值
        Type_ExponentialEaseInOut,      // 指数渐入渐出插值
        Type_ElasticEaseIn,             // 弹性渐入插值
        Type_ElasticEaseOut,            // 弹性渐出插值
        Type_ElasticEaseInOut,          // 弹性渐入渐出插值
        Type_BackEaseIn,                // 回退渐入插值
        Type_BackEaseOut,               // 回退渐出插值
        Type_BackEaseInOut,             // 回退渐出渐出插值
        Type_BounceEaseIn,              // 反弹渐入插值
        Type_BounceEaseOut,             // 反弹渐出插值
        Type_BounceEaseInOut,           // 反弹渐入渐出插值
    };
    // easing function
    auto EasingFunction(AnimationType type, float x) noexcept -> float;
    // cubic bezier
    //auto CubicBezierEx(float t, uint32_t arg) noexcept ->float;
    // control class
    class UIControl;
    // ssvalue2
    struct SSFromTo {
        // from
        SSValue     from;
        // to
        SSValue     to;
    };
    // indeterminate value
    auto IndeterminateValue(SSFromTo, float) noexcept->SSValue;
#ifdef NDEBUG
    enum { EXTRA_FROM_TO_LIST_LENGTH = 8 };
#else
    enum { EXTRA_FROM_TO_LIST_LENGTH = 4 };
#endif
    // extra control animation
    struct ControlAnimationExtra {
        // control pointer
        UIControl*          ctrl;
        // target state time done(unit: ms)
        uint16_t            done;
        // target state duration(unit: ms)
        uint16_t            duration;
        // changed list length
        uint32_t            length;
        // value list
        SSFromTo            list[EXTRA_FROM_TO_LIST_LENGTH];
        // get rate
        auto GetRate() const noexcept { return float(done) / float(duration); }
    };
    // basic control animation
    struct ControlAnimationBasic {
        // control
        UIControl*          ctrl;
        // origin state
        StyleState          origin;
        // target state time done(unit: ms)
        uint16_t            done;
        // target state duration(unit: ms)
        uint16_t            duration;
        // get rate
        auto GetRate() const noexcept { return float(done) / float(duration); }
    };
}