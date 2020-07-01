#include <core/ui_manager.h>
#include <control/ui_label.h>
#include <control/ui_button.h>
#include <event/ui_gui_event.h>
#include <control/ui_viewport.h>

const auto xul = u8R"xml(
<?xml version="1.0"?>
<window lui:clearcolor="white" title="lui demo">
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
        // #EVENT1
        find_button("exit")->AddGuiEventListener(
            UIButton::_onCommand(), [display](const GuiEventArg&) noexcept {
            // Time Capsule
            UIManager.CreateTimeCapsule([display](float p) noexcept {
                CUIString text;
                text.format(u"now exit in 0.49sec! @ %.2f%%", p*100.f);
                display->SetText(std::move(text));
                if (p == 1.f) UIManager.Exit();
            }, 0.49f, display);
            return Event_Accept;
        });
        // #EVENT2
        find_button("exit")->AddGuiEventListener(
            UIButton::_onCommand(), [display](const GuiEventArg&) noexcept {
            display->SetText(u"now exit in 0.49sec!"_sv);
            // Time Capsule# 2
            UIManager.CreateTimeCapsule([display](float p) noexcept {
                if (p == 1.f) 
                    display->SetText(u"here never arrived! because of 1.99sec!"_sv);
            }, 1.99f, display);
            return Event_Accept;
        });
        // #EVENT3
        find_button("exit")->AddGuiEventListener(
            UIButton::_onCommand(), [display](const GuiEventArg&) noexcept {
            display->SetText(u"here never arrived! because of Disconnect!"_sv);
            return Event_Accept;
        }).Disconnect();
    }
    // normal event
    auto DoEvent(UIControl* sender, const LongUI::EventArg& e) noexcept ->LongUI::EventAccept override {
        using namespace LongUI;
        switch (e.nevent)
        {
        case NoticeEvent::Event_UIEvent:
        {
            auto& guie = static_cast<const GuiEventArg&>(e);
            switch (guie.GetType())
            {
            case UIButton::_onCommand():
                if (const auto btn = uisafe_cast<UIButton>(sender)) {
                    const auto id = btn->GetID().id;
                    if (!std::strncmp(id, "btn", 3)) {
                        const auto index = id[3];
                        auto& text = m_pDisplay->RefText();
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
    int code = -1;
    if (UIManager.Initialize()) {
        LUIDebug(Hint) << "Battle Control Online..." << LongUI::endl;
        {
            MyViewport viewport;
            viewport.SetXul(xul);

            viewport.SetupControl();
            viewport.GetWindow()->ShowWindow();
            const auto rc = viewport.GetWindow()->Exec();
            code = LongUI::IntCode(rc);
        }
        LUIDebug(Hint) << "Battle Control Terminated." << LongUI::endl;
    }
    UIManager.Uninitialize();
    return code;
}