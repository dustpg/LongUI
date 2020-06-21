#include "mytoggle.h"
#include <core/ui_manager.h>
#include <core/ui_ctrlmeta.h>
#include <util/ui_aniamtion.h>
#include <core/ui_color_list.h>
#include <constexpr/const_bkdr.h>
#include <util/ui_color_system.h>
#include <core/ui_control_state.h>


// LUI_CONTROL_META_INFO should under class-own-namespace
namespace Demo {
    // define
    LUI_CONTROL_META_INFO(MyToggle, "mytoggle");
    // const for this
    enum { MyToggleMM = 51, MyToggleMH = 17 };
    /// <summary>
    /// Initializes a new instance of the <see cref="MyToggle"/> class.
    /// </summary>
    /// <param name="meta">The meta.</param>
    MyToggle::MyToggle(const MetaControl& meta) noexcept : Super(meta) {
        m_base = ColorF::FromRGBA_CT<RGBA_TianyiBlue>();
        m_top0 = ColorF::FromRGBA_CT<RGBA_Black>();
        m_top1 = ColorF::FromRGBA_CT<RGBA_White>();
        m_oBox.border = { 1,1,1,1 };
        this->SetStyleMinSize({ MyToggleMM , MyToggleMH });
    }
    /// <summary>
    /// Does the event.
    /// </summary>
    /// <returns></returns>
    void MyToggle::initialize() noexcept {
        // called on init

        // init m_value
        m_value = this->IsChecked() ? 1.f : 0.f;
        // force set alpha to 1.0
        m_base.a = 1.f;
        // initialize super class
        Super::initialize();
    }
    /// <summary>
    /// Does the mouse event.
    /// </summary>
    /// <param name="e">The e.</param>
    /// <returns></returns>
    auto MyToggle::DoMouseEvent(const MouseEventArg& e) noexcept -> EventAccept {
        switch (e.type)
        {
        case MouseEvent::Event_LButtonUp:
            // called on lbutton up
            this->Toggle();
            break;
        }
        return Super::DoMouseEvent(e);
    }
    /// <summary>
    /// Sets the base color.
    /// </summary>
    /// <param name="color">The color.</param>
    /// <returns></returns>
    void MyToggle::SetBaseColor(const ColorF& color) noexcept {
        m_base = color;
        // m_pBaseBrush is the rendering-data
        // before call m_pBaseBrush->SetColor, must be locked
        // but we call delay it into Update to get lock-free
#if 0
        CUIRenderAutoLocker locker;
        m_pBaseBrush->SetColor(auto_cast(m_base));
#endif
        this->NeedUpdate(Reason_ValueTextChanged);
    }
    /// <summary>
    /// Adds the attribute.
    /// </summary>
    /// <param name="attr">The attribute.</param>
    /// <param name="view">The view.</param>
    /// <returns></returns>
    void MyToggle::add_attribute(uint32_t attr, U8View view) noexcept {
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
    /// set checked state - force
    /// </summary>
    /// <returns></returns>
    void MyToggle::SetChecked(bool checked) noexcept {
        if (this->IsDisabled()) return;
        if (this->m_changing) return;
        if (!!this->IsChecked() == checked) return;
        // animation impl via TimeCapsule
        //this->StartAnimation({ State_Checked , checked ? State_Checked : State_Non });
        // just set it directly
        m_oStyle.state = m_oStyle.state ^ State_Checked;

        this->m_changing = true;
        const auto value = m_value;
        UIManager.CreateTimeCapsule([=](float v) noexcept {
            const auto target = 1.f - value;
            if (v == 1.f) {
                m_value = target;
                this->m_changing = false;
            }
            else {
                const float p = LongUI::EasingFunction(Type_BackEaseOut, v);
                m_value = value + (target - value) * p;
            }
            this->Invalidate();
        }, m_time, this);
        // trigger 'oncommand'
        this->FireEvent(this->_onCommand());
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
void Demo::MyToggle::Update(LongUI::UpdateReason reason) noexcept {
    // color changed
    if (reason & Reason_ValueTextChanged) {
        // update always called on rendering thread
        // rendering-lock free
        m_pBaseBrush->SetColor(auto_cast(m_base));
        this->Invalidate();
    }
    // super
    return Super::Update(reason);
}

/// <summary>
/// Recreates the specified release only.
/// </summary>
/// <param name="release_only">if set to <c>true</c> [release only].</param>
/// <returns></returns>
auto Demo::MyToggle::Recreate(bool release_only) noexcept -> LongUI::Result {
    // Recreate always lock the rendering-locker
    Result hr = { Result::RS_OK };
    // release gpu-data here


    LongUI::SafeRelease(m_pBaseBrush);



    // recreate super class
    hr = Super::Recreate(release_only);
    // release only
    if (release_only) return hr;
    // create gpu-data here


    // create brush
    if (hr) {
        hr.code = UIManager.Ref2DRenderer().CreateSolidColorBrush(
            auto_cast(m_base), &m_pBaseBrush
        );
    }

    return hr;
}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void Demo::MyToggle::Render() const noexcept {
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
        auto& ccbrush = UIManager.RefCCBrush(m_top0);
        // D2D DrawXxxx at center, line at 1 width, need 0.5 offset
        rounded.rect.left += 0.5f;
        rounded.rect.top += 0.5f;
        rounded.rect.right -= 0.5f;
        rounded.rect.bottom -= 0.5f;
        render.DrawRoundedRectangle(&rounded, m_pBaseBrush);
        render.FillEllipse(&ellipse, &ccbrush);
    }
    // draw checked
    else {
        auto& ccbrush = UIManager.RefCCBrush(m_top1);
        render.FillRoundedRectangle(&rounded, m_pBaseBrush);
        render.FillEllipse(&ellipse, &ccbrush);
    }
}



/// <summary>
/// Finalizes an instance of the <see cref="MyToggle"/> class.
/// </summary>
/// <returns></returns>
Demo::MyToggle::~MyToggle() noexcept {
    // dtor may be called on main-thread
    // m_pBrush is rendering-data, a rendering-locker required
    CUIRenderAutoLocker locker;
    // RELEASE DATA
    LongUI::SafeRelease(m_pBaseBrush);
}
