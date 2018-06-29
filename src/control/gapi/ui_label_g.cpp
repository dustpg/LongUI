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
    const auto& color = m_tfBuffer.text.color;
    // 文本偏移
    const float xoffset = rect.left + DEFUALT_TEXT_X_OFFSET;
    const float yoffset = rect.top + DEFUALT_TEXT_Y_OFFSET;
    // 具体渲染
    m_text.Render(UIManager.Ref2DRenderer(), color, { xoffset, yoffset });
}

