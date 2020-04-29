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
#ifndef LUI_DISABLE_STYLE_SUPPORT
    if (m_pBgRender) delete m_pBgRender;
    if (m_pBdRender) delete m_pBdRender;
#endif
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
    if (this->exist_basic_animation()) {
        // XXX: [优化]
        const auto basic = UIManager.FindBasicAnimation(*this);
        assert(basic && "BAD ACTION");
        if (basic) {
            arg.from = basic->origin;
            arg.progress = basic->GetRate();
        }
    }
    // 渲染本地风格
    LongUI::NativeStyleDraw(arg);
    // 表示已经渲染
    return false;
}


/// <summary>
/// Customs the style update.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::custom_style_update() noexcept {
#ifndef LUI_DISABLE_STYLE_SUPPORT
    if (this->is_size_changed() && m_pBdRender) {
        m_pBdRender->MarkSizeChanged();
    }
#endif
}

/// <summary>
/// Customs the style render.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::custom_style_render() const noexcept {
#ifndef LUI_DISABLE_STYLE_SUPPORT
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
        // UPDATE#0
        Size2F radius = { 0.f };
        if (m_pBdRender) {
            radius.width = m_pBdRender->radius_x;
            radius.height = m_pBdRender->radius_y;
        }
        // UPDATE#1
        // A.
        m_pBgRender->RenderColor(this->GetBox(), radius);
        // B.
        m_pBgRender->RenderImage(this->GetBox(), radius);
    }
    // 边框渲染
    if (m_pBdRender) {
        // UPDATE#2
        m_pBdRender->BeforeRender(this->GetBox());
        // C.
        m_pBdRender->RenderBorder(this->GetBox());
    }
#endif
}

/// <summary>
/// Begins the render.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::apply_world_transform() const noexcept {
    auto& painter = UIManager.Ref2DRenderer();
    painter.SetTransform(auto_cast(m_mtWorld));
}

// LongUI::detail
namespace LongUI { namespace detail {
    // both visible?
    inline bool both_visible(AttributeOverflow x, AttributeOverflow y) noexcept {
        return x == Overflow_Visible && y == Overflow_Visible;
        //return ((x << 4) | y) == ((Overflow_Visible << 4) | Overflow_Visible);
    }
}}


/// <summary>
/// Applies the clip rect.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::apply_clip_rect() const noexcept {
    // 检查overflow属性
    const auto overflow_x = this->GetStyle().overflow_x;
    const auto overflow_y = this->GetStyle().overflow_y;
    if (detail::both_visible(overflow_x, overflow_y)) return;
    // 压入
    auto& painter = UIManager.Ref2DRenderer();
    const float max_size = DEFAULT_CONTROL_MAX_SIZE;
    RectF rect = { 0, 0, max_size, max_size };
    const auto size = this->GetSize();
    if (overflow_x != Overflow_Visible)
        rect.right = size.width;
    if (overflow_y != Overflow_Visible)
        rect.bottom = size.height;

    const auto mode = D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
    painter.PushAxisAlignedClip(auto_cast(&rect), mode);
}

void LongUI::UIControl::cancel_clip_rect() const noexcept {
    // 检查overflow属性
    const auto overflow_x = this->GetStyle().overflow_x;
    const auto overflow_y = this->GetStyle().overflow_y;
    if (detail::both_visible(overflow_x, overflow_y)) return;
    // 弹出
    auto& painter = UIManager.Ref2DRenderer();
    painter.PopAxisAlignedClip();
}


//PCN_NOINLINE
///// <summary>
///// Gets the color of the background.
///// </summary>
///// <param name="">The .</param>
///// <returns></returns>
//void LongUI::UIControl::GetBackgroundColor(ColorF& color) const noexcept {
//    if (m_pBgRender) color = m_pBgRender->GetColor();
//    else color = ColorF::FromRGBA_CT<RGBA_Transparent>();
//}
//
//PCN_NOINLINE
///// <summary>
///// Gets the color of the foreground.
///// </summary>
///// <param name="">The .</param>
///// <returns></returns>
//void LongUI::UIControl::GetForegroundColor(ColorF& color) const noexcept {
//    color = ColorF::FromRGBA_CT<RGBA_Black>();
//}