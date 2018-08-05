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
    UITextBox* const list[] = {
        longui_cast<UITextBox*>(window.FindControl("default")),
        longui_cast<UITextBox*>(window.FindControl("disabled")),
        longui_cast<UITextBox*>(window.FindControl("readonly")),
        longui_cast<UITextBox*>(window.FindControl("max-length")),
        longui_cast<UITextBox*>(window.FindControl("password")),
        longui_cast<UITextBox*>(window.FindControl("multiline")),
    };
    const auto callback = [input](UIControl& ctrl) noexcept {
        const auto textbox = longui_cast<UITextBox*>(&ctrl);
        input->SetText(textbox->RequestText());
        return Event_Accept;
    };
    for (const auto textbox : list)
        textbox->AddGuiEventListener(UITextBox::_textChanged(), callback);
}