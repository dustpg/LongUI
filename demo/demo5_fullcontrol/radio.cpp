#include <control/ui_label.h>
#include <control/ui_radio.h>
#include <control/ui_viewport.h>


/// <summary>
/// Initializes the viewport radio.
/// </summary>
/// <param name="viewport">The viewport.</param>
/// <returns></returns>
void InitViewport_Radio(LongUI::UIViewport& viewport) noexcept {
    using namespace LongUI;
    auto& window = viewport.RefWindow();
    struct set_label_text {
        UILabel*    display = nullptr;
        auto operator()(UIControl& ctrl) noexcept {
            const auto radio = longui_cast<UIRadio*>(&ctrl);
            display->SetText(radio->RefText());
            return Event_Accept;
        }
    } calllback;
    // ZONE#1
    const auto display1 = window.FindControl("state-text");
    calllback.display = longui_cast<UILabel*>(display1);
    constexpr int count1 = 3;
    UIControl* zone1[count1];
    for (int i = 0; i != count1; ++i) {
        char buf[] = "zone1_ ";
        buf[6] = i + '1';
        const auto zone1_n = window.FindControl(buf);
        zone1[i] = zone1_n;
        zone1_n->AddGuiEventListener(UIRadio::_onCommand(), calllback);
    }
    // ZONE#2
    const auto display2 = window.FindControl("accesskey-text");
    calllback.display = longui_cast<UILabel*>(display2);
    constexpr int count2 = 6;
    UIControl* zone2[count2];
    for (int i = 0; i != count2; ++i) {
        char buf[] = "zone2_ ";
        buf[6] = i + '1';
        const auto zone2_n = window.FindControl(buf);
        zone2[i] = zone2_n;
        zone2_n->AddGuiEventListener(UIRadio::_onCommand(), calllback);
    }
    // ZONE#3
    const auto display3 = window.FindControl("or-text");
    calllback.display = longui_cast<UILabel*>(display3);
    constexpr int count3 = 6;
    UIControl* zone3[count3];
    for (int i = 0; i != count3; ++i) {
        char buf[] = "zone3_ ";
        buf[6] = i + '1';
        const auto zone3_n = window.FindControl(buf);
        zone3[i] = zone3_n;
        zone3_n->AddGuiEventListener(UIRadio::_onCommand(), calllback);
    }
    // ZONE#4
    const auto display4 = window.FindControl("image-text");
    calllback.display = longui_cast<UILabel*>(display4);
    constexpr int count4 = 6;
    UIControl* zone4[count4];
    for (int i = 0; i != count4; ++i) {
        char buf[] = "zone4_ ";
        buf[6] = i + '1';
        const auto zone4_n = window.FindControl(buf);
        zone4[i] = zone4_n;
        zone4_n->AddGuiEventListener(UIRadio::_onCommand(), calllback);
    }
}