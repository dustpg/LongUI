#include <control/ui_viewport.h>
#include <control/ui_checkbox.h>
#include <control/ui_button.h>
#include <core/ui_manager.h>
#include <cassert>

void InitViewport_CheckBox(LongUI::UIViewport&) noexcept;
void InitViewport_TextBox(LongUI::UIViewport&) noexcept;
void InitViewport_Radio(LongUI::UIViewport&) noexcept;
void InitViewport_Scale(LongUI::UIViewport&) noexcept;
struct init_func_t { void (*func)(LongUI::UIViewport&) noexcept; };

/// <summary>
/// Initializes the viewport callback.
/// </summary>
/// <param name="v">The v.</param>
/// <returns></returns>
void InitViewportCallback(LongUI::UIViewport& v) noexcept {
    using namespace LongUI;
    auto& window = v.RefWindow();
    const auto modal = longui_cast<UICheckBox*>(window.FindControl("modal"));
    assert(modal);
    const auto btn_layout = window.FindControl("layout");
    assert(btn_layout);
    const auto btn_textbox = window.FindControl("textbox");
    assert(btn_textbox);
    const auto btn_textbox2 = window.FindControl("textbox2");
    assert(btn_textbox2);
    const auto btn_scale = window.FindControl("scale");
    assert(btn_scale);
    const auto btn_tab = window.FindControl("tab");
    assert(btn_tab);
    const auto btn_checkbox = window.FindControl("checkbox");
    assert(btn_checkbox);
    const auto btn_radio = window.FindControl("radio");
    assert(btn_radio);
    const auto btn_scrolling = window.FindControl("scrolling");
    assert(btn_scrolling);
    const auto btn_popup = window.FindControl("popup");
    assert(btn_popup);
    const auto btn_button = window.FindControl("button");
    assert(btn_button);
    const auto btn_stack = window.FindControl("stack");
    assert(btn_stack);
    const auto btn_menubar = window.FindControl("menubar");
    assert(btn_menubar);
    const auto btn_image = window.FindControl("image");
    assert(btn_image);
    
    
    CUIWindow* const parent = nullptr;
    // NORMAL
    const auto create_viewport = [&window, modal](U8View view, init_func_t call) noexcept {


        const CUIWindow::WindowConfig cfg =
            CUIWindow::Config_Default
            //| CUIWindow::Config_QuitOnClose
            | CUIWindow::Config_DeleteOnClose
            ;
        const auto parent = modal->IsChecked() ? &window : nullptr;
        const auto p = new(std::nothrow) UIViewport{ parent, cfg };
        if (!p) return;
        p->RefWindow().ResizeRelative({ 600, 400 });
        p->SetXulFromFile(view);
        if (call.func) call.func(*p);

        p->SetAutoOverflow();
        p->RefWindow().SetClearColor({ 1,1,1,1 });
        p->RefWindow().ShowWindow();

        if (modal->IsChecked())
            p->RefWindow().Exec();
    };
    // LAYOUT
    btn_layout->AddGuiEventListener(
        UIButton::_onCommand(), [create_viewport](UIControl&) noexcept {
        create_viewport(u8"xul/layout.xul"_sv, {});
        return Event_Accept;
    });
    // TEXTBOX
    btn_textbox->AddGuiEventListener(
        UIButton::_onCommand(), [create_viewport](UIControl&) noexcept {
        create_viewport(u8"xul/textbox.xul"_sv, { InitViewport_TextBox });
        return Event_Accept;
    });
    // TEXTBOX
    btn_textbox2->AddGuiEventListener(
        UIButton::_onCommand(), [create_viewport](UIControl&) noexcept {
        create_viewport(u8"xul/textbox2.xul"_sv, {  });
        return Event_Accept;
    });
    // SCALE
    btn_scale->AddGuiEventListener(
        UIButton::_onCommand(), [create_viewport](UIControl&) noexcept {
        create_viewport(u8"xul/scale.xul"_sv, { InitViewport_Scale });
        return Event_Accept;
    });
    // TAB
    btn_tab->AddGuiEventListener(
        UIButton::_onCommand(), [create_viewport](UIControl&) noexcept {
        create_viewport(u8"xul/tab.xul"_sv, { });
        return Event_Accept;
    });
    // CHECKBOX
    btn_checkbox->AddGuiEventListener(
        UIButton::_onCommand(), [create_viewport](UIControl&) noexcept {
        create_viewport(u8"xul/checkbox.xul"_sv, { InitViewport_CheckBox });
        return Event_Accept;
    });
    // RADIO
    btn_radio->AddGuiEventListener(
        UIButton::_onCommand(), [create_viewport](UIControl&) noexcept {
        create_viewport(u8"xul/radio.xul"_sv, { InitViewport_Radio });
        return Event_Accept;
    });
    // SCROLLING
    btn_scrolling->AddGuiEventListener(
        UIButton::_onCommand(), [create_viewport](UIControl&) noexcept {
        create_viewport(u8"xul/scrolling.xul"_sv, {  });
        return Event_Accept;
    });
    // POPUP
    btn_popup->AddGuiEventListener(
        UIButton::_onCommand(), [create_viewport](UIControl&) noexcept {
        create_viewport(u8"xul/popup.xul"_sv, {});
        return Event_Accept;
    });
    // BUTTON
    btn_button->AddGuiEventListener(
        UIButton::_onCommand(), [create_viewport](UIControl&) noexcept {
        create_viewport(u8"xul/button.xul"_sv, {});
        return Event_Accept;
    });
    // STACK
    btn_stack->AddGuiEventListener(
        UIButton::_onCommand(), [create_viewport](UIControl&) noexcept {
        create_viewport(u8"xul/stack.xul"_sv, {});
        return Event_Accept;
    });
    // MENU BAR
    btn_menubar->AddGuiEventListener(
        UIButton::_onCommand(), [create_viewport](UIControl&) noexcept {
        create_viewport(u8"xul/menubar.xul"_sv, {});
        return Event_Accept;
    });
    // IMAGE
    btn_image->AddGuiEventListener(
        UIButton::_onCommand(), [create_viewport](UIControl&) noexcept {
        create_viewport(u8"xul/image.xul"_sv, {});
        return Event_Accept;
    });
}