#include <core/ui_manager.h>
#include <control/ui_label.h>
#include <control/ui_button.h>
#include <event/ui_gui_event.h>
#include <control/ui_viewport.h>

const auto xul = u8R"xml(
<?xml version="1.0"?>
<window clearcolor="white" title="lui demo">
  <label id="display" flex="1"/>
  <hbox flex="1">
    <button flex="1" label="7" id="btn7"/>
    <button flex="1" label="8" id="btn8"/>
    <button flex="1" label="9" id="btn9"/>
  </hbox>
  <hbox flex="1">
    <button flex="1" label="4" id="btn4"/>
    <button flex="1" label="5" id="btn5"/>
    <button flex="1" label="6" id="btn6"/>
  </hbox>
  <hbox flex="1">
    <button flex="1" label="1" id="btn1"/>
    <button flex="1" label="2" id="btn2"/>
    <button flex="1" label="3" id="btn3"/>
  </hbox>
  <hbox flex="1">
    <button flex="1" label="0" id="btn0"/>
    <button flex="1" label="X" id="exit" default="true"/>
  </hbox>
</window>
)xml";

class MyViewport final : public LongUI::UIViewport {
    // super class
    using Super = LongUI::UIViewport;
    // find button
    auto find_button(const char* name) noexcept {
        return longui_cast<LongUI::UIButton*>(m_window.FindControl(name));
    }
    // find label
    auto find_label(const char* name) noexcept {
        return longui_cast<LongUI::UILabel*>(m_window.FindControl(name));
    }
    // display
    LongUI::UILabel*            m_pDisplay = nullptr;
public:
    // dtor
    MyViewport() noexcept = default;
    // setup control
    void SetupControl() noexcept {
        using namespace LongUI;
        const auto display = this->find_label("display");
        m_pDisplay = display;
        find_button("exit")->AddGuiEventListener(
            UIButton::_clicked(), [display](UIControl&) noexcept {
            display->SetText(L"now exit!"_sv);
            UIManager.CreateTimeCapsule([](float p) noexcept {
                if (p == 1.f) UIManager.Exit();
            }, 0.49f, display);
            return Event_Accept;
        });
    }
    // normal event
    auto DoEvent(UIControl* sender, const LongUI::EventArg& e) noexcept ->LongUI::EventAccept override {
        using namespace LongUI;
        switch (e.nevent)
        {
        case NoticeEvent::Event_UIEvent:
        {
            auto& guie = static_cast<const EventGuiArg&>(e);
            switch (guie.GetEvent())
            {
            case UIButton::_clicked():
                if (const auto btn = uisafe_cast<UIButton>(sender)) {
                    const auto id = btn->GetID();
                    if (!std::strncmp(id, "btn", 3)) {
                        const auto index = id[3];
                        auto& text = m_pDisplay->GetTextString();
                        m_pDisplay->SetText(text + index);
                    }
                }
                break;
            }
            return Event_Accept;
        }
        default:
            return Super::DoEvent(sender, e);
        }
    }
};

int main() {
    if (UIManager.Initialize()) {
        {
            MyViewport viewport;
            viewport.SetXul(xul);
            viewport.SetupControl();
            viewport.GetWindow()->ShowWindow();
            UIManager.MainLoop();
        }
        UIManager.Uninitialize();
    }
    return 0;
}