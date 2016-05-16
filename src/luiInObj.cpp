#include <Graphics/luiGrInObj.h>
#include <algorithm>


// longui namespace
namespace LongUI {
    /// <summary>
    /// Creates the ruby notation.
    /// </summary>
    /// <param name="layout">The layout.</param>
    /// <returns></returns>
    auto CreateRubyNotation(IDWriteTextLayout* r, IDWriteTextLayout* t) noexcept -> IDWriteInlineObject* {
        return CUIRubyNotation::Create(r, t);
    }
    // get bitmap
    auto UIManager_GetBitmap(size_t index) noexcept ->ID2D1Bitmap1*;
}


/// <summary>
/// Finalizes an instance of the <see cref="CUIRubyNotation"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIRubyNotation::~CUIRubyNotation() noexcept {
    LongUI::SafeRelease(m_pRubyLayout);
    LongUI::SafeRelease(m_pTextLayout);
}

#include <LongUI/luiUiTxtRdr.h>

/// <summary>
/// Draws the specified client drawing context.
/// </summary>
/// <param name="clientDrawingContext">The client drawing context.</param>
/// <param name="renderer">The renderer.</param>
/// <param name="originX">The origin x.</param>
/// <param name="originY">The origin y.</param>
/// <param name="isSideways">The is sideways.</param>
/// <param name="isRightToLeft">The is right to left.</param>
/// <param name="clientDrawingEffect">The client drawing effect.</param>
/// <returns></returns>
auto LongUI::CUIRubyNotation::Draw(
    void * clientDrawingContext, 
    IDWriteTextRenderer* renderer, 
    FLOAT originX,  FLOAT originY, 
    BOOL isSideways,  BOOL isRightToLeft, 
    IUnknown* clientDrawingEffect) noexcept -> HRESULT {
    // 注音
    auto hr = m_pRubyLayout->Draw(
        clientDrawingContext,
        renderer,
        originX + m_fOffsetX, originY - m_fOffsetY * 0.2f
    );
    // 文本
    hr = m_pTextLayout->Draw(
        clientDrawingContext,
        renderer,
        originX, originY + m_fOffsetY
    );
    return hr;
}

/// <summary>
/// Gets the metrics.
/// </summary>
/// <param name="metrics">The metrics.</param>
/// <returns></returns>
auto LongUI::CUIRubyNotation::GetMetrics(
    DWRITE_INLINE_OBJECT_METRICS * metrics) noexcept -> HRESULT {
    metrics->supportsSideways = true;
    metrics->width = m_fWidth;
    metrics->height = m_fHeight;
    metrics->baseline = m_fBaseline ;
    return S_OK;
}



/// <summary>
/// Gets the overhang metrics.
/// </summary>
/// <param name="overhangs">The overhangs.</param>
/// <returns></returns>
auto LongUI::CUIRubyNotation::GetOverhangMetrics(
    DWRITE_OVERHANG_METRICS* overhangs ) noexcept->HRESULT {
    overhangs->left      = 0.f;
    overhangs->top       = 0.f;
    overhangs->right     = 0.f;
    overhangs->bottom    = 0.f;
    return S_OK;
}

/// <summary>
/// Gets the break conditions.
/// </summary>
/// <param name="breakConditionBefore">The break condition before.</param>
/// <param name="breakConditionAfter">The break condition after.</param>
/// <returns></returns>
auto LongUI::CUIRubyNotation::GetBreakConditions(
    DWRITE_BREAK_CONDITION* breakConditionBefore, 
    DWRITE_BREAK_CONDITION* breakConditionAfter) noexcept -> HRESULT {
    *breakConditionBefore = DWRITE_BREAK_CONDITION_NEUTRAL;
    *breakConditionAfter  = DWRITE_BREAK_CONDITION_NEUTRAL;
    return S_OK;
}

/// <summary>
/// Creates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIRubyNotation::Create(IDWriteTextLayout* ruby, 
    IDWriteTextLayout* text) noexcept -> CUIRubyNotation * {
    // 参数错误
    if (!ruby || !text) return nullptr;
    auto ptr = LongUI::SmallAllocT<CUIRubyNotation>(1);
    // 内存不足
    if (!ptr) return nullptr;
    // 构造对象
    ptr->CUIRubyNotation::CUIRubyNotation();
    ptr->m_pRubyLayout = ruby;
    ptr->m_pTextLayout = text;
    // 居中对齐
    text->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    ruby->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    text->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    ruby->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    // 调整大小
    {
        float rw = 0.f, tw = 0.f;
        DWRITE_TEXT_METRICS tm;
        text->GetMetrics(&tm);
        text->SetMaxHeight(tm.height);
        tw = tm.widthIncludingTrailingWhitespace;
        ruby->GetMetrics(&tm);
        ruby->SetMaxHeight(tm.height);
        rw = tm.widthIncludingTrailingWhitespace;

        ptr->m_fOffsetX = 0.f;
        ptr->m_fWidth = std::max(rw, tw);
        ruby->SetMaxWidth(ptr->m_fWidth);
        text->SetMaxWidth(ptr->m_fWidth);
    }
    // 添加引用
    ruby->AddRef(); text->AddRef();
    // 行大小
    DWRITE_LINE_METRICS lm; uint32_t c;
    text->GetLineMetrics(&lm, 1, &c);
    // 调整大小
    float bl = lm.baseline;
    float ht = lm.height;
    ruby->GetLineMetrics(&lm, 1, &c);
    float rh  = lm.height * 0.5f;
    ptr->m_fOffsetY = rh;
    ptr->m_fBaseline = bl  + rh;
    ptr->m_fHeight = ht;

    return ptr;
}


/// <summary>
/// Draws the specified client drawing context.
/// </summary>
/// <param name="clientDrawingContext">The client drawing context.</param>
/// <param name="renderer">The renderer.</param>
/// <param name="originX">The origin x.</param>
/// <param name="originY">The origin y.</param>
/// <param name="isSideways">The is sideways.</param>
/// <param name="isRightToLeft">The is right to left.</param>
/// <param name="clientDrawingEffect">The client drawing effect.</param>
/// <returns></returns>
auto LongUI::CUIInlineImage::Draw(
    void * clientDrawingContext, 
    IDWriteTextRenderer* renderer, 
    FLOAT originX, FLOAT originY, 
    BOOL isSideways, BOOL isRightToLeft, 
    IUnknown* clientDrawingEffect) noexcept -> HRESULT {
    // 获取渲染接口
    XUIBasicTextRenderer* rrr = nullptr;
    auto hr = renderer->QueryInterface(LongUI_IID_PV_ARGS(rrr));
    // 本对象需要特定接口
    if (SUCCEEDED(hr)) {
        // 获取指定位图
        if (const auto bitmap = LongUI::UIManager_GetBitmap(m_uBitmapId)) {
            // 计算目标位置
            D2D1_RECT_F des{
                originX, originY,
                originX + m_szDisplay.width,
                originY + m_szDisplay.height
            };
            // 可渲染对象
            if (!(bitmap->GetOptions() & D2D1_BITMAP_OPTIONS_CANNOT_DRAW)) {
                constexpr auto im = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
                rrr->target->DrawBitmap(bitmap, &des, 1.f, im, &m_rcSrc);
            }
            // 不可渲染 -> 填充矩形
            else {
                rrr->FillRect(des);
                hr = S_FALSE;
                assert(!"TARGET BITMAP CANNOT DRAW!");
            }
            // 释放位图
            bitmap->Release();
        }
        else {
            hr = E_NOT_SET;
        }
    }
    else {
        assert(!"NEED LongUI::XUIBasicTextRenderer to render this");
    }
    // 扫尾处理
    LongUI::SafeRelease(rrr);
    return hr;
}

