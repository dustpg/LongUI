#include "ui_private_effect.h"
#include <cassert>
#ifndef LUI_DISABLE_STYLE_SUPPORT

/// <summary>
/// Adds the reference.
/// </summary>
/// <returns></returns>
ULONG LongUI::CUIBaseEffectD2D::AddRef() noexcept {
    return ++m_cRefCount;
}

/// <summary>
/// Sets the draw information.
/// </summary>
/// <param name="drawInfo">The draw information.</param>
/// <returns></returns>
HRESULT LongUI::CUIBaseEffectD2D::SetDrawInfo(ID2D1DrawInfo *drawInfo) noexcept {
    LongUI::SafeRelease(m_pDrawInfo);
    m_pDrawInfo = LongUI::SafeAcquire(drawInfo);
    return S_OK;
}

/// <summary>
/// Queries the interface.
/// </summary>
/// <param name="riid">The riid.</param>
/// <param name="ppOutput">The pp output.</param>
/// <returns></returns>
HRESULT LongUI::CUIBaseEffectD2D::QueryInterface(REFIID riid, void** ppOutput) noexcept {
    HRESULT hr = S_OK;
    void* ptr = nullptr;
    // 获取 ID2D1EffectImpl
    if (riid == IID_ID2D1EffectImpl) 
        ptr = static_cast<ID2D1EffectImpl*>(this);
    // 获取 ID2D1DrawTransform
    else if (riid == IID_ID2D1DrawTransform) 
        ptr = static_cast<ID2D1DrawTransform*>(this);
    // 获取 ID2D1Transform
    else if (riid == IID_ID2D1Transform) 
        ptr = static_cast<ID2D1Transform*>(this);
    // 获取 ID2D1TransformNode
    else if (riid == IID_ID2D1TransformNode) 
        ptr = static_cast<ID2D1TransformNode*>(this);
    // 获取 IUnknown
    else if (riid == IID_IUnknown) 
        ptr = this;
    // 没有接口
    else hr = E_NOINTERFACE;
    // 数据有效 a(=)b
    if ((*ppOutput = ptr)) this->AddRef();
    return hr;
}


/// <summary>
/// Maps the invalid rect.
/// </summary>
/// <param name="inputIndex">Index of the input.</param>
/// <param name="invalidInputRect">The invalid input rect.</param>
/// <param name="pInvalidOutputRect">The p invalid output rect.</param>
/// <returns></returns>
HRESULT LongUI::CUIBaseEffectD2D::MapInvalidRect(
    UINT32 inputIndex,
    D2D1_RECT_L invalidInputRect,
    D2D1_RECT_L* pInvalidOutputRect
) const noexcept {
    *pInvalidOutputRect = { 0, 0, m_szDraw.width, m_szDraw.height };
    return S_OK;
}

/// <summary>
/// Maps the output rect to input rects.
/// 映射输出矩形到输入矩形数组
/// </summary>
/// <param name="pOutputRect">The p output rect.</param>
/// <param name="pInputRects">The p input rects.</param>
/// <param name="inputRectCount">The input rect count.</param>
/// <returns></returns>
HRESULT LongUI::CUIBaseEffectD2D::MapOutputRectToInputRects(
    _In_ const D2D1_RECT_L* pOutputRect,
    _Out_writes_(inputRectCount) D2D1_RECT_L* pInputRects,
    UINT32 inputRectCount
) const noexcept {
    // 虽说是数组 这里就一个
    if (inputRectCount != 1) return E_INVALIDARG;
    // 映射
    *pInputRects = { 0, 0, m_szInput.width, m_szInput.height };
    return S_OK;
}

/// <summary>
/// Maps the input rects to output rect.
/// 映射输入矩形数组到输出输出矩形
/// </summary>
/// <param name="pInputRects">The p input rects.</param>
/// <param name="pInputOpaqueSubRects">The p input opaque sub rects.</param>
/// <param name="inputRectCount">The input rect count.</param>
/// <param name="pOutputRect">The p output rect.</param>
/// <param name="pOutputOpaqueSubRect">The p output opaque sub rect.</param>
/// <returns></returns>
HRESULT LongUI::CUIBaseEffectD2D::MapInputRectsToOutputRect(
    _In_reads_(inputRectCount) const D2D1_RECT_L* pInputRects,
    _In_reads_(inputRectCount) const D2D1_RECT_L* pInputOpaqueSubRects,
    UINT32 inputRectCount,
    _Out_ D2D1_RECT_L* pOutputRect,
    _Out_ D2D1_RECT_L* pOutputOpaqueSubRect
) noexcept {
    // 虽说是数组 这里就一个
    if (inputRectCount != 1) return E_INVALIDARG;
    m_szInput.width = pInputRects->right - pInputRects->left;
    m_szInput.height = pInputRects->bottom - pInputRects->top;
    *pOutputRect = { 0, 0, m_szDraw.width, m_szDraw.height };
    *pOutputOpaqueSubRect = {};
    //m_inputRect = pInputRects[0];
    //*pOutputOpaqueSubRect = *pOutputRect;
    return S_OK;
}

/// <summary>
/// Sets the graph.
/// </summary>
/// <param name="pGraph">The p graph.</param>
/// <returns></returns>
HRESULT LongUI::CUIBaseEffectD2D::SetGraph(ID2D1TransformGraph* pGraph) noexcept {
    assert("unsupported!"); 
    return E_NOTIMPL; 
}

/// <summary>
/// Gets the input count.
/// </summary>
/// <returns></returns>
UINT32 LongUI::CUIBaseEffectD2D::GetInputCount() const noexcept {
    return 1;
}

/// <summary>
/// Initializes a new instance of the <see cref="CUIBaseEffectD2D"/> class.
/// </summary>
LongUI::CUIBaseEffectD2D::CUIBaseEffectD2D() noexcept : m_cRefCount(1) {

}



/// <summary>
/// Finalizes an instance of the <see cref="CUIBaseEffectD2D"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIBaseEffectD2D::~CUIBaseEffectD2D() noexcept {
    LongUI::SafeRelease(m_pDrawInfo);
}
#endif