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

#include "../luibase.h"
#include "../luiconf.h"
#include "luiGrHlper.h"
#include <d2d1.h>
#include <cstdint>

// longui namespace
namespace LongUI {
    // Color Effect
    class CUIColorEffect final : public Helper::ComBase<Helper::QiList<IUnknown>> {
        // super class
        using Super = Helper::ComBase<Helper::QiList<IUnknown>>;
    public:
        // ctor
        CUIColorEffect(ULONG count) noexcept : Super(count) {}
        // dtor
        virtual ~CUIColorEffect() noexcept {}
        // operator @delete
        void operator delete(void* p, size_t) noexcept { LongUI::SmallFree(p); };
        // color of effect
        D2D1_COLOR_F        color;
    public:
        // create a object
        static inline auto Create(const D2D1_COLOR_F& color, bool addref = true) {
            auto ptr = LongUI::SmallAlloc(sizeof(CUIColorEffect));
            auto*obj = reinterpret_cast<CUIColorEffect*>(ptr);
            if (obj) {
                obj->CUIColorEffect::CUIColorEffect(addref ? 1 : 0);
                obj->color = color;
            }
            return obj;
        }
    };
}