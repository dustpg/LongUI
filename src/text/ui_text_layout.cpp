// UI HREADER
#include <luiconf.h>
#include <text/ui_text_layout.h>
#include <core/ui_color_list.h>
#include <core/ui_manager.h>
// OS IMPL
#include <graphics/ui_graphics_impl.h>
#include <style/ui_text.h>
#include <text/ui_ctl_arg.h>
#include <text/ui_ctl_impl.h>

/// <summary>
/// Finalizes an instance of the <see cref="CUITextLayout"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUITextLayout::~CUITextLayout() noexcept {
    if (m_text) m_text->Release();
}

/// <summary>
/// Renders the specified renderer.
/// </summary>
/// <param name="renderer">The renderer.</param>
/// <param name="point">The point.</param>
/// <returns></returns>
void LongUI::CUITextLayout::Render(
    I::Renderer2D& renderer, 
    const ColorF& color,
    Point2F point) const noexcept {
    // 布局有效
    if (m_text) {
        //point.y -= m_halfls;
        renderer.DrawTextLayout(
            auto_cast(point),
            m_text,
            &UIManager.RefCCBrush(color)
        );
    }
    // 布局无效
    else {
        /*LUIDebug(Error) LUI_FRAMEID
            << "I::Text pointer null, won't draw any"
            << endl;*/
    }
}

/// <summary>
/// Sets the font.
/// </summary>
/// <param name="arg">The argument.</param>
/// <param name="str">The string.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
auto LongUI::CUITextLayout::SetFont(const TextFont& arg,
    const wchar_t* str, size_t len) noexcept -> Result {
    I::Font* font = nullptr;
    // 创建字体文件
    auto hr = UIManager.CreateCtlFont(arg.font, luiref font, &arg.text);
    // 设置字体
    if (hr) {
        TextArg arg;
        arg.string = str;
        arg.length = len;
        arg.font = font;
        arg.mwidth = DEFAULT_CONTROL_MAX_SIZE;
        arg.mheight = DEFAULT_CONTROL_MAX_SIZE;
        hr = this->set_text(luiref arg);
    }
    // 释放数据
    if (font) font->Release();
    // 返回错误
    return hr;
}


/// <summary>
/// Sets the text.
/// </summary>
/// <param name="str">The string.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
auto LongUI::CUITextLayout::SetText(const wchar_t* str, size_t len) noexcept -> Result {
    const auto font = I::FontFromText(m_text);
    // 参数调整
    TextArg arg;
    arg.string = str;
    arg.length = len;
    arg.font = font;
    arg.mwidth = DEFAULT_CONTROL_MAX_SIZE;
    arg.mheight = DEFAULT_CONTROL_MAX_SIZE;
    const auto hr = this->set_text(luiref arg);
    // 释放字体
    if (font) font->Release();
    return hr;
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="arg">The argument.</param>
/// <returns></returns>
auto LongUI::CUITextLayout::set_text(TextArg & arg) noexcept -> Result {
#ifndef NDEBUG
    m_dbgTexted = true;
#endif
    // 获取之前的大小
    if (m_text) {
        DWRITE_TEXT_METRICS metrice;
        m_text->GetMetrics(&metrice);
        arg.mwidth = metrice.layoutWidth;
        arg.mheight = metrice.layoutHeight;
        m_text = nullptr;
    }
    // 创建布局
    const auto hr = UIManager.CreateCtlText(arg, m_text);
    // 获取首行测量值计算半行距
    if (hr) {
        DWRITE_LINE_METRICS lm; uint32_t c;
        m_text->GetLineMetrics(&lm, 1, &c);
        //m_halfls = (lm.height - lm.baseline) * 0.5f;
    }
    // 返货结果
    return hr;
}

/// <summary>
/// Resizes the specified size.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::CUITextLayout::Resize(Size2F size) noexcept {
    if (m_text) {
        m_text->SetMaxWidth(size.width);
        m_text->SetMaxHeight(size.height);
    }
#ifndef NDEBUG
    else if (m_dbgTexted) {
        LUIDebug(Error) LUI_FRAMEID
            << L"resize null"
            << endl;
    }
#endif
}

/// <summary>
/// Sets the underline.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="len">The length.</param>
/// <param name="has">if set to <c>true</c> [has].</param>
/// <returns></returns>
void LongUI::CUITextLayout::SetUnderline(uint32_t pos, uint32_t len, bool has) noexcept {
    if (m_text) m_text->SetUnderline(has, { pos, len });
}

/// <summary>
/// Gets the size.
/// </summary>
/// <returns></returns>
auto LongUI::CUITextLayout::GetSize() const noexcept -> Size2F {
    if (m_text) {
#ifndef NDEBUG
        float min_width = 0.f;
        m_text->DetermineMinWidth(&min_width);
        DWRITE_LINE_SPACING_METHOD sm;
        float linesp, lineht;
        m_text->GetLineSpacing(&sm, &linesp, &lineht);
        DWRITE_OVERHANG_METRICS om; 
        m_text->GetOverhangMetrics(&om);
        DWRITE_LINE_METRICS lm; uint32_t c = 0;
        m_text->GetLineMetrics(&lm, 1, &c);
#endif
        // 获取整体测量值
        DWRITE_TEXT_METRICS   metrics;
        m_text->GetMetrics(&metrics);
        return{ metrics.widthIncludingTrailingWhitespace, metrics.height };
    }
    else {
        return{ 0, 12.f };
    }
}