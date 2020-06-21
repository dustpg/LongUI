#pragma once
#include <control/ui_control.h>

// solidcolor bursh from d2d
struct ID2D1SolidColorBrush;

namespace Demo { 
    // ez to use
    using namespace LongUI;
    // my toggle switch control
    class MyToggleEx final : public UIControl {
        // super class
        using Super = UIControl;
    protected:
        // ctor
        MyToggleEx(const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~MyToggleEx() noexcept override;
        // ctor
        explicit MyToggleEx(UIControl* parent = nullptr) noexcept : MyToggleEx(MyToggleEx::s_meta) { this->final_ctor(parent); }
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
        //auto DoEvent(UIControl* sender, const EventArg& e) noexcept->EventAccept override;
        // do mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
        // update
        //void Update(UpdateReason) noexcept override;
        // render this control only, [Global rendering and Incremental rendering]
        void Render() const noexcept override;
    protected:
        // initialize
        void initialize() noexcept override;
        // add attribute
        void add_attribute(uint32_t attr, U8View view) noexcept override;
        // set checked
        void set_checked(bool checked) noexcept;
    private:
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
        // point logged
        Point2F                 m_point = {};
        // changing timecapsule
        CUITimeCapsule*         m_changing = nullptr;
    };
}

// LUI_DECLARE_METAINFO must be under LongUI namespace
namespace LongUI {
    // get meta info for MyToggleEx
    LUI_DECLARE_METAINFO(Demo::MyToggleEx);
}
