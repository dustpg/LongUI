#include "mycontrol.h"
#include <core/ui_manager.h>
#include <core/ui_ctrlmeta.h>
#include <constexpr/const_bkdr.h>
#include <core/ui_color_list.h>
#include <util/ui_aniamtion.h>
#include <util/ui_color_system.h>


// LUI_CONTROL_META_INFO should under class-own-namespace
namespace Demo {
    // defi
    LUI_CONTROL_META_INFO(MyControl, "myctrl");

    /// <summary>
    /// Initializes a new instance of the <see cref="MyControl"/> class.
    /// </summary>
    /// <param name="parent">The parent.</param>
    /// <param name="meta">The meta.</param>
    MyControl::MyControl(UIControl* parent, 
        const MetaControl& meta) noexcept : Super(parent, meta) {
        m_color = ColorF::FromRGBA_CT<RGBA_TianyiBlue>();
    }
    /// <summary>
    /// Does the event.
    /// </summary>
    /// <param name="sender">The sender.</param>
    /// <param name="e">The e.</param>
    /// <returns></returns>
    auto MyControl::DoEvent(UIControl* sender, const EventArg& e) noexcept -> EventAccept {
        switch (e.nevent)
        {
        case NoticeEvent::Event_Initialize:
            // called on init

            // force set alpha to 1.0
            m_color.a = 1.f;
            break;
        }
        return Super::DoEvent(sender, e);
    }
    /// <summary>
    /// Does the mouse event.
    /// </summary>
    /// <param name="e">The e.</param>
    /// <returns></returns>
    auto MyControl::DoMouseEvent(const MouseEventArg& e) noexcept -> EventAccept {
        switch (e.type)
        {
        case MouseEvent::Event_LButtonUp:
            // called on lbutton up
            UIManager.CreateTimeCapsule([this](float p) noexcept {
                p = LongUI::EasingFunction(Type_BackEaseOut, p);
                ColorF color;
                color = m_color;
                color.a = p;
                this->SetColor(color);
            }, 0.3f, this);
            break;
        case MouseEvent::Event_LButtonDown:
            // called on lbutton up
            UIManager.CreateTimeCapsule([this](float p) noexcept {
                p = LongUI::EasingFunction(Type_BackEaseOut, p);
                ColorF color;
                color = m_color;
                color.a = 1.f - p;
                this->SetColor(color);
            }, 0.3f, this);
            break;
        }
        return Super::DoMouseEvent(e);
    }
    /// <summary>
    /// Sets the color.
    /// </summary>
    /// <param name="color">The color.</param>
    /// <returns></returns>
    void MyControl::SetColor(const ColorF& color) noexcept {
        m_color = color;
        m_bColorChanged = true;
        this->NeedUpdate();
    }
    /// <summary>
    /// Adds the attribute.
    /// </summary>
    /// <param name="attr">The attribute.</param>
    /// <param name="view">The view.</param>
    /// <returns></returns>
    void MyControl::add_attribute(uint32_t attr, U8View view) noexcept {
        // attr: bkdr-hashed code
        switch (attr)
        {
        case "value"_bkdr:
            ColorF::FromRGBA_RT(luiref m_color, { view.ColorRGBA32() });
            break;
        default:
            // others: 
            return Super::add_attribute(attr, view);
        }
    }
}

// ----------------------------------------------

#include <graphics/ui_graphics_impl.h>


/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void Demo::MyControl::Update() noexcept {
    // color changed
    if (m_bColorChanged) {
        m_bColorChanged = false;
        m_width = m_color.a * 0.5f;
        m_pBrush->SetColor(auto_cast(m_color));
        this->Invalidate();
    }
    // super
    return Super::Update();
}

/// <summary>
/// Recreates the specified release only.
/// </summary>
/// <param name="release_only">if set to <c>true</c> [release only].</param>
/// <returns></returns>
auto Demo::MyControl::Recreate(bool release_only) noexcept -> LongUI::Result {
    Result hr = { Result::RS_OK };
    // release gpu-data here


    LongUI::SafeRelease(m_pBrush);



    // recreate super class
    hr = Super::Recreate(release_only);
    // release only
    if (release_only) return hr;
    // create gpu-data here


    // create brush
    if (hr) {
        hr.code = UIManager.Ref2DRenderer().CreateSolidColorBrush(
            auto_cast(m_color), &m_pBrush
        );
    }

    return hr;
}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void Demo::MyControl::Render() const noexcept {
    // call super render if want to css-background-look
    //Super::Render();
    
    // do custom render
    const auto rect = m_oBox.GetBorderEdge();
    auto& render = UIManager.Ref2DRenderer();

    auto& ccbrush = UIManager.RefCCBrush({ 1.f, 0.f, 1.f, 1.f });
    render.FillRectangle(&auto_cast(rect), &ccbrush);

    D2D1_ELLIPSE ell;
    ell.point.x = (rect.left + rect.right) * 0.5f;
    ell.point.y = (rect.bottom + rect.top) * 0.5f;
    ell.radiusX = (rect.left - rect.right) * m_width;
    ell.radiusY = (rect.bottom - rect.top) * m_width;
    render.FillEllipse(&ell, m_pBrush);
}



/// <summary>
/// Finalizes an instance of the <see cref="MyControl"/> class.
/// </summary>
/// <returns></returns>
Demo::MyControl::~MyControl() noexcept {
    // RELEASE DATA

    LongUI::SafeRelease(m_pBrush);
}