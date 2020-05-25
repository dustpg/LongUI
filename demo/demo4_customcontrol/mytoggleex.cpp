#include "mytoggleex.h"
#include <core/ui_manager.h>
#include <core/ui_ctrlmeta.h>
#include <util/ui_aniamtion.h>
#include <core/ui_color_list.h>
#include <constexpr/const_bkdr.h>
#include <util/ui_color_system.h>
#include <core/ui_control_state.h>

#include <util/ui_little_math.h>


// LUI_CONTROL_META_INFO should under class-own-namespace
namespace Demo {
    // define
    LUI_CONTROL_META_INFO(MyToggleEx, "mytoggleex");
    // const for this
    enum { MyToggleMM = 51, MyToggleMH = 17 };
    /// <summary>
    /// Initializes a new instance of the <see cref="MyToggleEx"/> class.
    /// </summary>
    /// <param name="parent">The parent.</param>
    /// <param name="meta">The meta.</param>
    MyToggleEx::MyToggleEx(UIControl* parent, 
        const MetaControl& meta) noexcept : Super(impl::ctor_lock(parent), meta) {
        m_base = ColorF::FromRGBA_CT<RGBA_TianyiBlue>();
        m_top0 = ColorF::FromRGBA_CT<RGBA_Black>();
        m_top1 = ColorF::FromRGBA_CT<RGBA_White>();
        m_oBox.border = { 1,1,1,1 };
        this->SetStyleMinSize({ MyToggleMM , MyToggleMH });
        impl::ctor_unlock();
    }
    /// <summary>
    /// Does the event.
    /// </summary>
    /// <param name="sender">The sender.</param>
    /// <param name="e">The e.</param>
    /// <returns></returns>
    auto MyToggleEx::DoEvent(UIControl* sender, const EventArg& e) noexcept -> EventAccept {
        switch (e.nevent)
        {
        case NoticeEvent::Event_Initialize:
            // called on init
            m_value = this->IsChecked() ? 1.f : 0.f;
            // force set alpha to 1.0
            m_base.a = 1.f;
            break;
        }
        return Super::DoEvent(sender, e);
    }
    /// <summary>
    /// Does the mouse event.
    /// </summary>
    /// <param name="e">The e.</param>
    /// <returns></returns>
    auto MyToggleEx::DoMouseEvent(const MouseEventArg& e) noexcept -> EventAccept {
        switch (e.type)
        {
        case MouseEvent::Event_MouseMove:
            // mouse hold&move
            if (e.modifier & Modifier_LButton) {
                // map point to local
                const auto get_delta_x = [this, &e]() noexcept {
#if 0
                    // way 1: map double point then get delta
                    Point2F points[] = { {e.px, e.py}, m_point };
                    this->MapFromWindow(points[0]);
                    this->MapFromWindow(points[1]);
                    return points[0].x - points[1].x;
#else
                    // way 2: get native delta then div the scale
                    const auto native_delta = e.px - m_point.x;
                    return native_delta / m_mtWorld._11;
#endif
                };
                // dragging
                const auto rect = m_oBox.GetContentEdge();
                const auto height = rect.bottom - rect.top;
                const auto width = rect.right - rect.left - height;
                const auto pos = get_delta_x() / width;
                const auto base = m_oStyle.state.checked ? 1.f : 0.f;
                const auto value = detail::clamp(base + pos, 0.f, 1.f);
                if (m_value != value) {
                    m_value = value;
                    this->Invalidate();
                }
            }
            break;
        case MouseEvent::Event_LButtonDown:
            // called on lbutton down
            m_point = { e.px, e.py };
            break;
        case MouseEvent::Event_LButtonUp:
            // called on lbutton up
            if (LongUI::IsSameInGuiLevel(m_point, { e.px, e.py }))
                this->Toggle();
            else
                this->set_checked(m_value >= 0.5f);
            break;
        }
        return Super::DoMouseEvent(e);
    }
    /// <summary>
    /// Sets the base color.
    /// </summary>
    /// <param name="color">The color.</param>
    /// <returns></returns>
    void MyToggleEx::SetBaseColor(const ColorF& color) noexcept {
        m_base = color;
        this->Invalidate();
    }
    /// <summary>
    /// Adds the attribute.
    /// </summary>
    /// <param name="attr">The attribute.</param>
    /// <param name="view">The view.</param>
    /// <returns></returns>
    void MyToggleEx::add_attribute(uint32_t attr, U8View view) noexcept {
        // attr: bkdr-hashed code
        switch (attr)
        {
        case "base"_bkdr:
            ColorF::FromRGBA_RT(luiref m_base, { view.ColorRGBA32() });
            break;
        case "top0"_bkdr:
            ColorF::FromRGBA_RT(luiref m_top0, { view.ColorRGBA32() });
            break;
        case "top1"_bkdr:
            ColorF::FromRGBA_RT(luiref m_top1, { view.ColorRGBA32() });
            break;
        case "time"_bkdr:
            m_time = view.ToFloat();
            break;
        default:
            // others: 
            return Super::add_attribute(attr, view);
        }
    }
    /// <summary>
    /// set checked state
    /// </summary>
    /// <returns></returns>
    void MyToggleEx::SetChecked(bool checked) noexcept {
        if (this->IsDisabled()) return;
        if (this->IsChecked() == checked) return;
        this->set_checked(checked);
    }

    /// <summary>
    /// set checked state - force
    /// </summary>
    /// <returns></returns>
    void Demo::MyToggleEx::set_checked(bool checked) noexcept {
        // animation impl via TimeCapsule
        //this->StartAnimation({ StyleStateType::Type_Checked , checked });
        // just set it directly
        m_oStyle.state.checked = checked;
        // force terminate anination if vaild
        if (m_changing) m_changing->Terminate();
        const auto target = checked ? 1.f : 0.f;
        const auto value = m_value;
        m_changing = UIManager.CreateTimeCapsule([=](float v) noexcept {
            if (v == 1.f) {
                m_value = target;
                m_changing = nullptr;
            }
            else {
                const float p = LongUI::EasingFunction(Type_CubicEaseOut, v);
                m_value = value + (target - value) * p;
            }
            this->Invalidate();
        }, m_time, this);
        // trigger 'oncommand'
        this->TriggerEvent(this->_onCommand());
        // TODO: ACCESSIBLE
#ifndef LUI_ACCESSIBLE

#endif
    }
}

// ----------------------------------------------------------------------------
//                            Gtaphics API Required
// ----------------------------------------------------------------------------

#include <graphics/ui_graphics_impl.h>


/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
//void Demo::MyToggleEx::Update(LongUI::UpdateReason reason) noexcept {
//    // super
//    return Super::Update(reason);
//}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void Demo::MyToggleEx::Render() const noexcept {
    // call super render if want to css-background-look
    //Super::Render();
    

    // D2D API
    D2D1_ROUNDED_RECT rounded;
    rounded.rect = auto_cast(m_oBox.GetContentEdge());
    const auto center = (rounded.rect.bottom + rounded.rect.top) * 0.5f;
    const auto radius = (rounded.rect.bottom - rounded.rect.top) * 0.5f;
    const auto width = rounded.rect.right - rounded.rect.left - radius * 2.f;
    rounded.radiusX = rounded.radiusY = radius;
    D2D1_ELLIPSE ellipse;
    ellipse.point = { rounded.rect.left + radius + width * m_value, center };
    ellipse.radiusX = ellipse.radiusY = radius * 0.75f;
    // draw case0 or case1 ?
    const bool draw_case0 = this->IsChecked() ? m_value == 0.f : m_value != 1.f;

    // do custom render
    const auto rect = m_oBox.GetBorderEdge();
    auto& render = UIManager.Ref2DRenderer();

    // draw unchecked
    if (draw_case0) {
        // D2D DrawXxxx at center, line at 1 width, need 0.5 offset
        rounded.rect.left += 0.5f;
        rounded.rect.top += 0.5f;
        rounded.rect.right -= 0.5f;
        rounded.rect.bottom -= 0.5f;
        render.DrawRoundedRectangle(&rounded, &UIManager.RefCCBrush(m_base));
        render.FillEllipse(&ellipse, &UIManager.RefCCBrush(m_top0));
    }
    // draw checked
    else {
        render.FillRoundedRectangle(&rounded, &UIManager.RefCCBrush(m_base));
        render.FillEllipse(&ellipse, &UIManager.RefCCBrush(m_top1));
    }
}



/// <summary>
/// Finalizes an instance of the <see cref="MyToggleEx"/> class.
/// </summary>
/// <returns></returns>
Demo::MyToggleEx::~MyToggleEx() noexcept {

}
