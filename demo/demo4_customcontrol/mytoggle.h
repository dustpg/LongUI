#pragma once
#include <control/ui_control.h>

// solidcolor bursh from d2d
struct ID2D1SolidColorBrush;

namespace Demo { 
    // ez to use
    using namespace LongUI;
    // my toggle switch control
    class MyToggle final : public UIControl {
        // super class
        using Super = UIControl;
    protected:
        // ctor
        MyToggle(const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~MyToggle() noexcept override;
        // ctor
        // LUI_CONTROL_OUTER_CTOR(MyToggle);
        explicit MyToggle(UIControl* parent = nullptr) noexcept : MyToggle(MyToggle::s_meta) { this->final_ctor(parent); }
    public:
        // on commnad event
        static constexpr auto _onCommand() noexcept { return GuiEvent::Event_OnCommand; }
    public:
        // set base color
        void SetBaseColor(const ColorF&) noexcept;
        // checked?
        auto IsChecked() const noexcept { return m_oStyle.state & State_Checked; }
        // set checked
        void SetChecked(bool checked) noexcept;
        // toggle this
        void Toggle() noexcept { SetChecked(!IsChecked()); }
    public:
        // do normal event
        auto DoEvent(UIControl* sender, const EventArg& e) noexcept->EventAccept override;
        // do mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
        // update
        void Update(UpdateReason) noexcept override;
        // render this control only, [Global rendering and Incremental rendering]
        void Render() const noexcept override;
        // recreate/init device(gpu) resource
        auto Recreate(bool release_only) noexcept->Result override;
    protected:
        // add attribute
        void add_attribute(uint32_t attr, U8View view) noexcept override;
    private:
        // base brush
        ID2D1SolidColorBrush *  m_pBaseBrush = nullptr;
        // base color
        ColorF                  m_base = {};
        // top0 color
        ColorF                  m_top0 = {};
        // top1 color
        ColorF                  m_top1 = {};
        // float value [0, 1]
        float                   m_value = 0.f;
        // animation time
        float                   m_time = 0.2f;
        // in changing
        bool                    m_changing = false;
    };
}

// LUI_DECLARE_METAINFO must be under LongUI namespace
namespace LongUI {
    // get meta info for MyToggle
    LUI_DECLARE_METAINFO(Demo::MyToggle);
}
