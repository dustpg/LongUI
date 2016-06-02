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

#include <luibase.h>
#include <luiconf.h>
#include <cassert>
#include <d2d1effecthelpers.h>
#include <atomic>

// longui::clsid_hsveffect
namespace LongUI { extern const GUID CLSID_HsvEffect; }

// longUI::effect namespace
namespace LongUI { namespace Effect {
    // HSV color display
    class Hsv final : public ID2D1EffectImpl, public ID2D1DrawTransform {
    public:
        // Register
        static auto Register(ID2D1Factory1*) noexcept->HRESULT;
        // vertex
        struct Vertex {
            // color
            D2D1_COLOR_F    color;
            // position
            D2D1_POINT_2F   pos;
        };
        // const expr
        enum : size_t { 
            UNIT = 128,
            HSV_COLOR_COUNT = 6,
            VERTEX_PER_UNIT = 6,
            VERTEX_COUNT = HSV_COLOR_COUNT * VERTEX_PER_UNIT + 3,
        };
        // 属性列表
        enum : UINT32 {
            // D2D1_COLOR_F
            Properties_PickedColor = 0,
        };
    public:
        // ID2D1EffectImpl
        IFACEMETHODIMP Initialize(ID2D1EffectContext* pContextInternal, ID2D1TransformGraph* pTransformGraph) noexcept override;
        IFACEMETHODIMP PrepareForRender(D2D1_CHANGE_TYPE changeType) noexcept override;
        IFACEMETHODIMP SetGraph(ID2D1TransformGraph* pGraph) noexcept override { assert("unsupported!"); return E_NOTIMPL; }
        // IUnknown
        IFACEMETHODIMP_(ULONG) AddRef() noexcept override { return ++m_cRef; }
        IFACEMETHODIMP_(ULONG) Release() noexcept override;
        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppOutput) noexcept override;
        // ID2D1Transform
        IFACEMETHODIMP MapInputRectsToOutputRect(const D2D1_RECT_L* pInputRects,
            const D2D1_RECT_L* pInputOpaqueSubRects,
            UINT32 inputRectCount,
            D2D1_RECT_L* pOutputRect,
            D2D1_RECT_L* pOutputOpaqueSubRect) noexcept override; 
        IFACEMETHODIMP MapOutputRectToInputRects(const D2D1_RECT_L* pOutputRect,
            D2D1_RECT_L* pInputRects,
            UINT32 inputRectCount) const noexcept override;
        IFACEMETHODIMP MapInvalidRect(UINT32 inputIndex,
            D2D1_RECT_L invalidInputRect,
            D2D1_RECT_L* pInvalidOutputRect) const noexcept override;
        // ID2D1TransformNode
        IFACEMETHODIMP_(UINT32) GetInputCount() const noexcept override { return 0; }
        // ID2D1DrawTransform
        IFACEMETHODIMP SetDrawInfo(ID2D1DrawInfo *pDrawInfo) noexcept override;
    protected:
        // ctor
        Hsv() noexcept;
        // dtor
        ~Hsv() noexcept;
        // set color
        auto SetColor(const D2D1_COLOR_F& c) noexcept { m_cbuffer.color = c; }
        // get color
        auto GetColor() const noexcept { return m_cbuffer.color; }
    protected:
        // ref-count
        std::atomic<uint32_t>       m_cRef = 1;
        // unused u32
        uint32_t                    m_unused_u32 = 233;
        // draw info
        ID2D1DrawInfo*              m_pDrawInfo = nullptr;
        // vertex buffer
        ID2D1VertexBuffer*          m_pVertexBuffer = nullptr;
        // cbuffer
        struct {
            // color
            D2D1_COLOR_F            color;
            // c-buffer
        }m_cbuffer;
        // input
        //D2D1_RECT_L                 m_inputRect = { 0 };
    };

}}