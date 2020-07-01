#include <control/ui_label.h>
#include <control/ui_button.h>
#include <control/ui_deck.h>
#include <control/ui_radio.h>
#include <control/ui_radiogroup.h>
#include <control/ui_viewport.h>


/// <summary>
/// Initializes the viewport stack.
/// </summary>
/// <param name="viewport">The viewport.</param>
/// <returns></returns>
void InitViewport_Stack(LongUI::UIViewport& viewport) noexcept {
    using namespace LongUI;
    auto& window = viewport.RefWindow();
    const auto group = longui_cast<UIRadioGroup*>(window.FindControl("radiogroup"));
    const auto deck = longui_cast<UIDeck*>(window.FindControl("my-deck"));
    
    uint32_t index = 0;
    for (auto& child : (*group)) {
        auto& radio = *longui_cast<UIRadio*>(&child);
        radio.AddGuiEventListener(radio._onCommand(), [=](const LongUI::GuiEventArg&) noexcept {
            deck->SetSelectedIndex(index);
            return Event_Accept;
        });
        index++;
    }
}
