#include <control/ui_label.h>
#include <control/ui_checkbox.h>
#include <control/ui_viewport.h>


/// <summary>
/// Initializes the viewport CheckBox.
/// </summary>
/// <param name="viewport">The viewport.</param>
/// <returns></returns>
void InitViewport_CheckBox(LongUI::UIViewport& viewport) noexcept {
    using namespace LongUI;
    auto& window = viewport.RefWindow();
    struct set_label_text {
        UILabel*    display = nullptr;
        auto operator()(UIControl& ctrl) noexcept {
            const auto checkbox = longui_cast<UICheckBox*>(&ctrl);
            display->SetText(checkbox->GetTextString());
            return Event_Accept;
        }
    } calllback;
    // ZONE#1
    const auto display1 = window.FindControl("tab-text");
    calllback.display = longui_cast<UILabel*>(display1);
    constexpr int count1 = 6;
    UIControl* zone1[count1];
    for (int i = 0; i != count1; ++i) {
        char buf[] = "zone1_ ";
        buf[6] = i + '1';
        const auto zone1_n = window.FindControl(buf);
        zone1[i] = zone1_n;
        zone1_n->AddGuiEventListener(UICheckBox::_onCommand(), calllback);
    }
    // ZONE#2
    const auto display2 = window.FindControl("accesskey-text");
    calllback.display = longui_cast<UILabel*>(display2);
    constexpr int count2 = 6;
    UIControl* zone2[count2];
    for (int i = 0; i != count1; ++i) {
        char buf[] = "zone2_ ";
        buf[6] = i + '1';
        const auto zone1_n = window.FindControl(buf);
        zone1[i] = zone1_n;
        zone1_n->AddGuiEventListener(UICheckBox::_onCommand(), calllback);
    }
}