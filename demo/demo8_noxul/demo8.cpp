#include <core/ui_manager.h>
#include <control/ui_label.h>
#include <control/ui_spacer.h>
#include <control/ui_button.h>
#include <control/ui_textbox.h>
#include <control/ui_viewport.h>
#include <control/ui_box.h>
#include <interface/ui_default_config.h>

// menu
#include <control/ui_popup.h>
#include <control/ui_menuitem.h>


// win32api
extern "C" int __stdcall MessageBoxW(HWND, const char16_t*, const char16_t*, uint32_t);

using namespace LongUI;

class CDemo8NoXul : public LongUI::UIViewport {
protected:
    UISpacer        m_SpacerA;
    UIHBoxLayout    m_oBox;
    // {
        UISpacer        m_oBoxB_SpacerA;
        UILabel         m_oBoxB_Label;
        UITextBox       m_oBoxB_TextBox;
        UIButton        m_oBoxB_Button;
        UISpacer        m_oBoxB_SpacerB;
    // }
    UISpacer        m_SpacerB;
public:
    CDemo8NoXul() noexcept;
    ~CDemo8NoXul() noexcept override { 
        // !!important!!
        m_state.destructing = true; 
    };
};


class CDemo8ContextMenu : public UIPopup {
    UIMenuItem          m_oItemPearl;
    UIMenuItem          m_oItemAramis;
    UIMenuItem          m_oItemYakima;
public:
    CDemo8ContextMenu() noexcept;
    ~CDemo8ContextMenu() noexcept override {
        // !!important!!
        m_state.destructing = true;
    };
};


int main() {
    const auto create_mainwindow = []() noexcept {
        // !!important!!
        UIControl::ControlMakingBegin();
        const auto viewport = new(std::nothrow) CDemo8NoXul;
        UIControl::ControlMakingEnd();
        return viewport;
    };
    const auto create_context = []() noexcept {
        // !!important!!
        UIControl::ControlMakingBegin();
        const auto viewport = new(std::nothrow) CDemo8ContextMenu;
        UIControl::ControlMakingEnd();
        return viewport;
    };
    if (UIManager.Initialize()) {
        LUIDebug(Hint) 
            << "sizeof CDemo8NoXul = " << sizeof(CDemo8NoXul) 
            << " bytes, not on stack"
            << LongUI::endl;
        if (const auto menu = create_context())
            UIManager.MoveToGlobalSubView(*menu);
        if (const auto viewport = create_mainwindow()) {
            viewport->GetWindow()->ShowWindow();
            viewport->GetWindow()->Exec();
            delete viewport;
        }
    }
    UIManager.Uninitialize();
    return 0;
}


// Unsafe!
#include <core/ui_unsafe.h>
#include <constexpr/const_bkdr.h>


CDemo8NoXul::CDemo8NoXul() noexcept :
    UIViewport(),
    m_SpacerA(this),
    m_oBox(this),
    m_oBoxB_SpacerA(&m_oBox),
    m_oBoxB_Label(&m_oBox),
    m_oBoxB_TextBox(&m_oBox),
    m_oBoxB_Button(&m_oBox),
    m_oBoxB_SpacerB(&m_oBox),
    m_SpacerB(this) {
    const auto flex = "flex"_bkdr;
    const auto value = "value"_bkdr;
    const auto label = "label"_bkdr;
    const auto align = "align"_bkdr;
    Unsafe::AddAttrUninited(m_SpacerA, flex, "1.0"_sv);
    Unsafe::AddAttrUninited(m_SpacerB, flex, "1.0"_sv);
    Unsafe::AddAttrUninited(m_oBoxB_SpacerA, flex, "1.0"_sv);
    Unsafe::AddAttrUninited(m_oBoxB_SpacerB, flex, "1.0"_sv);
    Unsafe::AddAttrUninited(m_oBoxB_Label, value, "Input:"_sv);
    Unsafe::AddAttrUninited(m_oBoxB_Button, label, "Check it now"_sv);
    Unsafe::AddAttrUninited(m_oBox, align, "center"_sv);
    this->RefWindow().SetClearColor({ 1,1,1,1 });
    m_oBoxB_Button.AddGuiEventListener(m_oBoxB_Button._onCommand(),
        [this](UIControl& c) noexcept {
        auto& text = m_oBoxB_TextBox.RequestText();
        CUIBlockingGuiOpAutoUnlocker unlocker;
        const auto hwnd = this->RefWindow().GetHwnd();
        const auto cap = u"Check it now";
        ::MessageBoxW(hwnd, text.c_str(), cap, 0);
        return Event_Accept;
    });
    const auto context = "context"_bkdr;
    Unsafe::AddAttrUninited(m_oBoxB_TextBox, context, "demo8context"_sv);
    m_oStyle.overflow_x = m_oStyle.overflow_x = Overflow_Auto;
}


CDemo8ContextMenu::CDemo8ContextMenu() noexcept : UIPopup(CUIWindow::Config_FixedSize | CUIWindow::Config_Popup),
m_oItemPearl(this), m_oItemAramis(this), m_oItemYakima(this) {
    this->init_clear_color_for_default_ctxmenu();
    const auto id = "id"_bkdr;
    const auto label = "label"_bkdr;
    Unsafe::AddAttrUninited(*this, id, "demo8context"_sv);
    Unsafe::AddAttrUninited(m_oItemPearl, label, "Pearl"_sv);
    Unsafe::AddAttrUninited(m_oItemAramis, label, "Aramis"_sv);
    Unsafe::AddAttrUninited(m_oItemYakima, label, "Yakima"_sv);
}
