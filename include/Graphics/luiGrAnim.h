#pragma once
/**
* Copyright (c) 2014-2016 dustpg   mailto:dustpg@gmail.com
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

#include "../Platless/luiPlUtil.h"
#include <d2d1_3.h>

// longui namespace
namespace LongUI {
    // helper namespace
    namespace Helper {
        // start/end 
        template<typename T> auto GetAnimaStart() noexcept->T;
        template<typename T> auto GetAnimaEnd() noexcept->T;
        // spacial
        template<> inline auto GetAnimaStart<float>() noexcept -> float { 
            return 0.f;
        }
        template<> inline auto GetAnimaEnd<float>() noexcept -> float { 
            return 1.f; 
        }
        template<> inline auto GetAnimaStart<D2D1_COLOR_F>() noexcept -> D2D1_COLOR_F { 
            return D2D1::ColorF(D2D1::ColorF::White); 
        }
        template<> inline auto GetAnimaEnd<D2D1_COLOR_F>() noexcept -> D2D1_COLOR_F { 
            return D2D1::ColorF(D2D1::ColorF::Black); 
        }
        template<> inline auto GetAnimaStart<D2D1_POINT_2F>() noexcept -> D2D1_POINT_2F { 
            return D2D1::Point2F(); 
        }
        template<> inline auto GetAnimaEnd<D2D1_POINT_2F>() noexcept -> D2D1_POINT_2F { 
            return D2D1::Point2F(); 
        }
    }
    // UI Animation
    template<typename T> class CUIAnimation {
    public:
        // constructor
        CUIAnimation(AnimationType t) noexcept :type(t)  {};
        // destructor
        ~CUIAnimation() noexcept {}
        // update with delta time
        void Update(float t) noexcept;
    public:
        // the type
        AnimationType       type;
        // time index
        float               time    = 0.0f;
        // duration time
        float               duration= 0.12f;
        // start
        T                   start   = Helper::GetAnimaStart<T>();
        // end
        T                   end     = Helper::GetAnimaEnd<T>();
        // value
        T                   value   = Helper::GetAnimaStart<T>();
    };
    // spacial
    template<> void LongUI::CUIAnimation<float>::Update(float t) noexcept;
    template<> void LongUI::CUIAnimation<D2D1_COLOR_F>::Update(float t) noexcept;
    template<> void LongUI::CUIAnimation<D2D1_POINT_2F>::Update(float t) noexcept;
}