#pragma once

#include <luiconf.h>
#ifndef LUI_DISABLE_STYLE_SUPPORT
#include <core/ui_object.h>
#include <graphics/ui_graphics_impl.h>
// d2d effect
#include <d2d1effectauthor.h>
#include <atomic>

#define LUI_D2D1_VALUE_TYPE_BINDING(CLASS, TYPE, NAME)\
    {\
        L#NAME, [](IUnknown* obj, const BYTE* data, UINT32 len) noexcept {\
            assert(obj && data && len == sizeof(TYPE));\
            const auto impl = static_cast<ID2D1EffectImpl*>(obj);\
            const auto ths = static_cast<CLASS*>(impl);\
            ths->Set##NAME(*reinterpret_cast<const TYPE*>(data));\
            return S_OK;\
        },  [](const IUnknown* obj, BYTE* data, UINT32 len, UINT32* outeln) noexcept {\
            assert(obj);\
            if (data) {\
                const auto impl = static_cast<const ID2D1EffectImpl*>(obj);\
                const auto ths = static_cast<const CLASS*>(impl);\
                ths->Get##NAME(*reinterpret_cast<TYPE*>(data));\
            }\
            if (outeln) *outeln = sizeof(TYPE);\
            return S_OK;\
        }\
    }

// ui namespace
namespace LongUI {
    // base effect impl in d2d
    class CUIBaseEffectD2D :
        public CUISmallObject,
        public ID2D1EffectImpl,
        public ID2D1DrawTransform {
    public:
        // IUnknown
        IFACEMETHODIMP_(ULONG) AddRef() noexcept override final;
        //IFACEMETHODIMP_(ULONG) Release() noexcept override final;
        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppOutput) noexcept override final;
        // ID2D1EffectImpl
        //IFACEMETHODIMP Initialize(ID2D1EffectContext* pContextInternal, ID2D1TransformGraph* pTransformGraph) noexcept override final;
        //IFACEMETHODIMP PrepareForRender(D2D1_CHANGE_TYPE changeType) noexcept override final;
        IFACEMETHODIMP SetGraph(ID2D1TransformGraph* pGraph) noexcept override final;
        // ID2D1Transform
        IFACEMETHODIMP MapInputRectsToOutputRect(const D2D1_RECT_L* pInputRects,
            const D2D1_RECT_L* pInputOpaqueSubRects,
            UINT32 inputRectCount,
            D2D1_RECT_L* pOutputRect,
            D2D1_RECT_L* pOutputOpaqueSubRect) noexcept override final;
        IFACEMETHODIMP MapOutputRectToInputRects(const D2D1_RECT_L* pOutputRect,
            D2D1_RECT_L* pInputRects,
            UINT32 inputRectCount) const noexcept override final;
        IFACEMETHODIMP MapInvalidRect(UINT32 inputIndex,
            D2D1_RECT_L invalidInputRect,
            D2D1_RECT_L* pInvalidOutputRect) const noexcept override final;
        // ID2D1TransformNode
        IFACEMETHODIMP_(UINT32) GetInputCount() const noexcept override final;
        // ID2D1DrawTransform
        IFACEMETHODIMP SetDrawInfo(ID2D1DrawInfo *pDrawInfo) noexcept override final;
    protected:
        // ctor
        CUIBaseEffectD2D() noexcept;
        // dtor 
        ~CUIBaseEffectD2D() noexcept;
    protected:
        // draw info
        ID2D1DrawInfo*                      m_pDrawInfo = nullptr;
        // recount
        std::atomic_uint32_t                m_cRefCount = 1;
        // cbuffer changed?
        uint32_t                            m_cUnused = 0;
        // size of draw
        Size2L                              m_szDraw = { 0, 0 };
        // size of input
        Size2L                              m_szInput = { 0, 0 };
    };
}
#endif