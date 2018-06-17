// ui
#include <core/ui_manager.h>
#include <util/ui_aniamtion.h>
#include <core/ui_color_list.h>
#include <control/ui_control.h>
#include <style/ui_native_style.h>
#include <graphics/ui_bg_renderer.h>
#include <graphics/ui_bd_renderer.h>
#include <graphics/ui_graphics_impl.h>

/// <summary>
/// Deletes the renderer.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::delete_renderer() noexcept {
    if (m_pBgRender) delete m_pBgRender;
    if (m_pBdRender) delete m_pBdRender;
}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::Render() const noexcept {
#if 0
    char buffer[256]; std::memset(buffer, ' ', 256);
    const auto level = this->GetLevel();
    buffer[level] = 0;
    LUIDebug(Log) << buffer << this << endl;
#endif
    assert(m_state.inited && "must init control first");
    if (this->native_style_render()) {
        this->custom_style_render();
    }
}

/// <summary>
/// Natives the style render.
/// </summary>
/// <returns></returns>
bool LongUI::UIControl::native_style_render() const noexcept {
    const auto type = this->GetStyle().appearance;
    // 使用自定义风格渲染
    if (type == AttributeAppearance::Appearance_None) return true;
    // 使用本地风格渲染
    NativeDrawArgs arg;
    arg.border = this->GetBox().GetBorderEdge();
    arg.from = arg.to = this->GetStyle().state;
    arg.progress = 0.f;
    arg.appearance = type;
    // HACK: 特殊处理
    switch (type)
    {
    case AttributeAppearance::Appearance_GroupBox:
        arg.border.top += (*begin()).GetSize().height * 0.5f
                        + this->GetBox().padding.top 
                        + this->GetBox().border.top
                        //+ 1.f
                        ;
    break;
    }
    // 存在动画
    if (m_pAnimation) {
        arg.from = m_pAnimation->basic.origin;
        arg.progress = m_pAnimation->basic.GetRate();
    }
    // 渲染本地风格
    LongUI::NativeStyleDraw(arg);
    // 表示已经渲染
    return false;
}

/// <summary>
/// Customs the style render.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::custom_style_render() const noexcept {
    /*
        栈顺序:
        - 这里实现
           A. background color
           B. background image
           C. border
        - 外面实现
           A. children
           B. outline
    */
    // 背景渲染
    if (m_pBgRender) {
        // A.
        m_pBgRender->RenderColor(this->GetBox());
        // B.
        m_pBgRender->RenderImage(this->GetBox());
    }
    // 边框渲染

}

/// <summary>
/// Begins the render.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::apply_world_transform() const noexcept {
    auto& painter = UIManager.Ref2DRenderer();
    painter.SetTransform(auto_cast(m_mtWorld));
}


PCN_NOINLINE
/// <summary>
/// Gets the color of the background.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::UIControl::GetBackgroundColor(ColorF& color) const noexcept {
    if (m_pBgRender) color = m_pBgRender->GetColor();
    else color = ColorF::FromRGBA_CT<RGBA_Transparent>();
}

PCN_NOINLINE
/// <summary>
/// Gets the color of the foreground.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::UIControl::GetForegroundColor(ColorF& color) const noexcept {
    color = ColorF::FromRGBA_CT<RGBA_Black>();
}