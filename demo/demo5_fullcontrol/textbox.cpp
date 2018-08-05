#include <control/ui_label.h>
#include <control/ui_textbox.h>
#include <control/ui_viewport.h>


/// <summary>
/// Initializes the viewport text box.
/// </summary>
/// <param name="viewport">The viewport.</param>
/// <returns></returns>
void InitViewport_TextBox(LongUI::UIViewport& viewport) noexcept {
    using namespace LongUI;
    auto& window = viewport.RefWindow();
    const auto input = longui_cast<UILabel*>(window.FindControl("input"));
    const auto tb_default = longui_cast<UITextBox*>(window.FindControl("default"));
    const auto tb_disabled = longui_cast<UITextBox*>(window.FindControl("disabled"));
    const auto tb_readonly = longui_cast<UITextBox*>(window.FindControl("readonly"));
    const auto tb_max_length = longui_cast<UITextBox*>(window.FindControl("max-length"));
    const auto tb_password = longui_cast<UITextBox*>(window.FindControl("password"));
    const auto tb_multiline = longui_cast<UITextBox*>(window.FindControl("multiline"));
    const auto callback = [input](UIControl& ctrl) noexcept {
        const auto textbox = longui_cast<UITextBox*>(&ctrl);
        input->SetText(textbox->RequestText());
        return Event_Accept;
    };
    tb_default->AddGuiEventListener(UITextBox::_textChanged(), callback);
    tb_disabled->AddGuiEventListener(UITextBox::_textChanged(), callback);
    tb_readonly->AddGuiEventListener(UITextBox::_textChanged(), callback);
    tb_max_length->AddGuiEventListener(UITextBox::_textChanged(), callback);
    tb_password->AddGuiEventListener(UITextBox::_textChanged(), callback);
    tb_multiline->AddGuiEventListener(UITextBox::_textChanged(), callback);
}