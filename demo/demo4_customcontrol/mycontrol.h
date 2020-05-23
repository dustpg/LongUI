#pragma once
#include <control/ui_control.h>

// solidcolor bursh from d2d
struct ID2D1SolidColorBrush;

namespace Demo { 
    // ez to use
    using namespace LongUI;
    // my control
    class MyControl final : public UIControl {
        // super class
        using Super = UIControl;
    protected:
        // ctor
        MyControl(UIControl* parent, const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~MyControl() noexcept;
        // ctor
        MyControl(UIControl* parent = nullptr) noexcept : MyControl(parent, MyControl::s_meta) {}
    public:
        // set color
        void SetColor(const ColorF&) noexcept;
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
        // brush
        ID2D1SolidColorBrush *  m_pBrush = nullptr;
        // color
        ColorF                  m_color = {};
        // color
        ColorF                  m_mc = {};
        // width
        float                   m_width = 0.5f;
    };
}

// LUI_DECLARE_METAINFO must be under LongUI namespace
namespace LongUI {
    // get meta info for MyControl
    LUI_DECLARE_METAINFO(Demo::MyControl);
}