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
    const auto change = longui_cast<UILabel*>(window.FindControl("change"));
    const auto input = longui_cast<UILabel*>(window.FindControl("input"));
    UITextBox* const list[] = {
        longui_cast<UITextBox*>(window.FindControl("default")),
        longui_cast<UITextBox*>(window.FindControl("disabled")),
        longui_cast<UITextBox*>(window.FindControl("readonly")),
        longui_cast<UITextBox*>(window.FindControl("max-length")),
        longui_cast<UITextBox*>(window.FindControl("password")),
        longui_cast<UITextBox*>(window.FindControl("multiline")),
    };
    const auto callback_i = [input](UIControl& ctrl) noexcept {
        const auto textbox = longui_cast<UITextBox*>(&ctrl);
        CUIString str_input = u"Input: "_sv;
        str_input += textbox->RequestText();
        input->SetText(std::move(str_input));
        return Event_Accept;
    };
    const auto callback_c = [change](UIControl& ctrl) noexcept {
        const auto textbox = longui_cast<UITextBox*>(&ctrl);
        CUIString str_change = u"Change: "_sv;
        str_change += textbox->RequestText();
        change->SetText(std::move(str_change));
        return Event_Accept;
    };
    for (const auto textbox : list)
        textbox->AddGuiEventListener(UITextBox::_onChange(), callback_c);

    for (const auto textbox : list)
        textbox->AddGuiEventListener(UITextBox::_onInput(), callback_i);
}