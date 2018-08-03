#include <control/ui_viewport.h>
#include <control/ui_checkbox.h>
#include <control/ui_button.h>
#include <core/ui_manager.h>
#include <cassert>

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
    CUIWindow* const parent = nullptr;
    // NORMAL
    const auto create_viewport = [&window, modal](U8View view) noexcept {


        const CUIWindow::WindowConfig cfg =
            CUIWindow::Config_Default
            //| CUIWindow::Config_QuitOnClose
            | CUIWindow::Config_DeleteOnClose
            ;
        const auto p = new(std::nothrow) UIViewport{ &window, cfg };
        if (!p) return;
        p->SetXulFromFile(view);
        //LongUI::UIControl::ControlMakingEnd();

        p->SetAutoOverflow();
        p->RefWindow().SetClearColor({ 1,1,1,1 });
        p->RefWindow().ShowWindow();

        if (modal->GetChecked()) {
            p->RefWindow().Exec();
            //delete p;
        }
    };
    // LAYOUT
    btn_layout->AddGuiEventListener(
        UIButton::_clicked(), [create_viewport](UIControl&) noexcept {
        create_viewport(u8"xul/layout.xul"_sv);
        return Event_Accept;
    });
    // TEXTBOX
    btn_textbox->AddGuiEventListener(
        UIButton::_clicked(), [create_viewport](UIControl&) noexcept {
        create_viewport(u8"xul/textbox.xul"_sv);
        return Event_Accept;
    });
}