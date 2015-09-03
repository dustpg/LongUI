#include "LongUI.h"


// CUIRubyCharacter 构造函数
LongUI::CUIRubyCharacter::
CUIRubyCharacter(const CtorContext& ctx) noexcept : Super(CUIInlineObject::Type_Ruby) {
    UNREFERENCED_PARAMETER(ctx);
}

// CUIRubyCharacter 析构函数
LongUI::CUIRubyCharacter::~CUIRubyCharacter() noexcept {
    ::SafeRelease(m_pBaseLayout);
    ::SafeRelease(m_pRubyLayout);
}

// CUIRubyCharacter 刻画
auto LongUI::CUIRubyCharacter::Draw(
    void* clientDrawingContext,
    IDWriteTextRenderer* renderer,
    FLOAT originX,
    FLOAT originY,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect
    ) noexcept ->HRESULT {
    UNREFERENCED_PARAMETER(clientDrawingContext);
    UNREFERENCED_PARAMETER(renderer);
    UNREFERENCED_PARAMETER(originX);
    UNREFERENCED_PARAMETER(originY);
    UNREFERENCED_PARAMETER(isSideways);
    UNREFERENCED_PARAMETER(isRightToLeft);
    UNREFERENCED_PARAMETER(clientDrawingEffect);
    return E_NOTIMPL;
}

// 获取 Metrics
auto LongUI::CUIRubyCharacter::GetMetrics(
    DWRITE_INLINE_OBJECT_METRICS* metrics) noexcept ->HRESULT {
    DWRITE_INLINE_OBJECT_METRICS inlineMetrics = { 0 };
    inlineMetrics;

    *metrics = inlineMetrics;
    return S_OK;
}


// 获取 Overhang Metrics
auto LongUI::CUIRubyCharacter::GetOverhangMetrics(
     DWRITE_OVERHANG_METRICS* overhangs) noexcept ->HRESULT {
    overhangs->left = 0;
    overhangs->top = 0;
    overhangs->right = 0;
    overhangs->bottom = 0;
    return S_OK;
}


auto LongUI::CUIRubyCharacter::GetBreakConditions(
    DWRITE_BREAK_CONDITION* breakConditionBefore,
    DWRITE_BREAK_CONDITION* breakConditionAfter
    ) noexcept ->HRESULT {
    *breakConditionBefore = DWRITE_BREAK_CONDITION_NEUTRAL;
    *breakConditionAfter = DWRITE_BREAK_CONDITION_NEUTRAL;
    return S_OK;
}
