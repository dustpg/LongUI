// ui
#include <core/ui_manager.h>
#include <control/ui_label.h>
#include <graphics/ui_graphics_impl.h>

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void LongUI::UILabel::Render() const noexcept {
    // 基本渲染
    Super::Render();

    //if (m_string == L"Male") {
    //    auto& rdr = UIManager.Ref2DRenderer();
    //    Matrix3X2F tr;
    //    rdr.GetTransform(&auto_cast(tr));
    //    LUIDebug(Hint) LUI_FRAMEID 
    //        << this->GetPos() 
    //        << ' ' << m_mtWorld._31
    //        << endl;
    //    const auto rect = this->GetBox().GetContentEdge();
    //    auto& brush = UIManager.RefCCBrush({ 1,0,0,1 });
    //    rdr.FillRectangle(auto_cast(&rect), &brush);
    //}

    // 前景文本
    const auto rect = this->GetBox().GetContentEdge();
    //ColorF color; ColorF::FromRGBA_RT(color, this->GetFgColor());
    auto color = m_tfBuffer.text.color;
    // XXX: 默认控件而且是HREF模式
    if (this->is_def_href()) {
        color.a = 1.f;        color.r = 0.f;
        color.g = 0.4f;       color.b = 0.8f;
    }
    // 文本偏移
    const float xoffset = rect.left + DEFUALT_TEXT_X_OFFSET;
    const float yoffset = rect.top + DEFUALT_TEXT_Y_OFFSET;
    // 具体渲染
    // -- 拥有描边?
    if (m_tfBuffer.text.stroke_width > 0.f) {
        TextOutlineContext context;
        context.renderer = &UIManager.Ref2DRenderer();
        context.color = m_tfBuffer.text.color;
        context.outline_color = m_tfBuffer.text.stroke_color;
        context.outline_width = m_tfBuffer.text.stroke_width;
        m_outline.Render(context, m_text.GetCtlText(), { xoffset, yoffset });
    }
    else {
        m_text.Render(UIManager.Ref2DRenderer(), color, { xoffset, yoffset });
    }
}

