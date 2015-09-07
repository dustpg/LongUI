#pragma once
/**
* Copyright (c) 2014-2015 dustpg   mailto:dustpg@gmail.com
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

// longui namespace
namespace LongUI {
    // the type of aniamtion
    enum class AnimationType : uint32_t {
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
    // easing func
    float __fastcall EasingFunction(AnimationType, float) noexcept;
    // UI Animation
    template<typename T>
    class CUIAnimation {
    public:
        // constructor
        CUIAnimation(AnimationType t) noexcept :type(t)  {};
        // destructor
        ~CUIAnimation() noexcept {}
        // update with delta time
        void __fastcall Update(float t) noexcept;
        // update without delta time
        auto Update() noexcept { return this->Update(UIManager.GetDeltaTime()); }
    public:
        // the type
        AnimationType       type;
        // time index
        float               time = 0.0f;
        // duration time
        float               duration = 0.12f;
        // start
        T                   start ;
        // end
        T                   end ;
        // value
        T                   value;
    };
    // spacial
    template<> void __fastcall LongUI::CUIAnimation<float>::Update(float t) noexcept;
    template<> void __fastcall LongUI::CUIAnimation<D2D1_COLOR_F>::Update(float t) noexcept;
    template<> void __fastcall LongUI::CUIAnimation<D2D1_POINT_2F>::Update(float t) noexcept;
    template<> void __fastcall LongUI::CUIAnimation<D2D1_MATRIX_3X2_F>::Update(float t) noexcept;
}