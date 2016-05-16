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
#include "../LongUI/luiUiHlper.h"
#include <d2d1.h>
#include <cstdint>

// longui namespace
namespace LongUI {
    // inline obj
    using XUIInlineObject = Helper::ComBase<Helper::QiList<IDWriteInlineObject>, uint32_t>;
    // ruby notation object
    class CUIRubyNotation final : public XUIInlineObject {
        // super class
        using Super = XUIInlineObject;
    public:
        // ctor
        CUIRubyNotation() noexcept : Super(1) {}
        // dtor
        virtual ~CUIRubyNotation() noexcept;
        // operator @delete
        void operator delete(void* p, size_t) noexcept { LongUI::SmallFree(p); };
    public:
        // draw this
        virtual auto STDMETHODCALLTYPE Draw(
            void* clientDrawingContext,
            IDWriteTextRenderer* renderer,
            FLOAT originX,
            FLOAT originY,
            BOOL isSideways,
            BOOL isRightToLeft,
            IUnknown* clientDrawingEffect
        ) noexcept->HRESULT override;
        // get the metrics of this
        virtual auto STDMETHODCALLTYPE GetMetrics(
            DWRITE_INLINE_OBJECT_METRICS* metrics
        ) noexcept->HRESULT override;
        // get the overhang metrics of this
        virtual auto STDMETHODCALLTYPE GetOverhangMetrics(
            DWRITE_OVERHANG_METRICS* overhangs
        ) noexcept->HRESULT override;
        // get break condition
        virtual auto STDMETHODCALLTYPE GetBreakConditions(
            DWRITE_BREAK_CONDITION* breakConditionBefore,
            DWRITE_BREAK_CONDITION* breakConditionAfter
        ) noexcept->HRESULT override;
    private:
        // base line
        float                       m_fBaseline = 0.f;
        // ruby layout
        IDWriteTextLayout*          m_pRubyLayout = nullptr;
        // text layout
        IDWriteTextLayout*          m_pTextLayout = nullptr;
        // width
        float                       m_fWidth = 0.f;
        // height
        float                       m_fHeight = 0.f;
        // offset x for ruby
        float                       m_fOffsetX = 0.f;
        // offset y for ruby
        float                       m_fOffsetY = 0.f;
    public:
        // create a object
        static auto Create(IDWriteTextLayout*, IDWriteTextLayout*) noexcept->CUIRubyNotation*;
    };
    // inline image
    class CUIInlineImage final : public XUIInlineObject {
        // super class
        using Super = XUIInlineObject;
    public:
        // ctor
        CUIInlineImage() noexcept : Super(1) {}
        // dtor
        virtual ~CUIInlineImage() noexcept {}
        // operator @delete
        void operator delete(void* p, size_t) noexcept { LongUI::SmallFree(p); };
    public:
        // draw this
        virtual auto STDMETHODCALLTYPE Draw(
            void* clientDrawingContext,
            IDWriteTextRenderer* renderer,
            FLOAT originX,
            FLOAT originY,
            BOOL isSideways,
            BOOL isRightToLeft,
            IUnknown* clientDrawingEffect
        ) noexcept->HRESULT override;
        // get the metrics of this
        virtual auto STDMETHODCALLTYPE GetMetrics(
            DWRITE_INLINE_OBJECT_METRICS* metrics
        ) noexcept->HRESULT override;
        // get the overhang metrics of this
        virtual auto STDMETHODCALLTYPE GetOverhangMetrics(
            DWRITE_OVERHANG_METRICS* overhangs
        ) noexcept->HRESULT override;
        // get break condition
        virtual auto STDMETHODCALLTYPE GetBreakConditions(
            DWRITE_BREAK_CONDITION* breakConditionBefore,
            DWRITE_BREAK_CONDITION* breakConditionAfter
        ) noexcept->HRESULT override;
    private:
        // bitmap id
        uint32_t                    m_uBitmapId = 0;
        // rect
        D2D1_RECT_F                 m_rcSrc{ 0.f };
        // size
        D2D1_SIZE_F                 m_szDisplay{ 0.f };
    public:
        // create a object
        static auto Create(IDWriteTextLayout*, IDWriteTextLayout*) noexcept->CUIInlineImage*;
    };
}