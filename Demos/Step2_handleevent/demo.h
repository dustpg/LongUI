#pragma once

// window xml layout
static const char* const DEMO_XML =
u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="512, 256" debug="true" name="LongUI Demo Window">
    <HorizontalLayout>
        <Button name="ok" margin="4,4,4,4" borderwidth="1" text="OK"/>
        <Button name="exit" margin="4,4,4,4" borderwidth="1" text="Exit"/>
    </HorizontalLayout>
</Window>
)xml";

// longui::demo namespace
namespace LongUI { namespace Demo {
    // MainViewport class
    class MainViewport final : public UIViewport {
        // super class
        using Super = UIViewport;
        // clean up
        virtual void cleanup() noexcept override { this->before_deleted(); delete this; }
    public:
        // ctor
        MainViewport(XUIBaseWindow* window) : Super(window) {}
        // do some event
        virtual bool DoEvent(const EventArgument& arg) noexcept override;
    public:
        // canbe closed now?
        virtual bool CanbeClosedNow() noexcept override { return true; }
        // onclose
        virtual void OnClose() noexcept override {
            // exit on only 1 window
            if (UIManager.GetSystemWindowCount() == 1) {
                UIManager.Exit();
            }
        }
    private:
        // tree finished
        void tree_bulit();
    };
}}


// -------------------------- IMPLEMENT ---------------------------
// do event for ui
bool LongUI::Demo::MainViewport::DoEvent(const EventArgument& arg) noexcept {
    // longui event
    switch (arg.event)
    {
    case LongUI::Event::Event_TreeBuildingFinished:
        // Event_TreeBuildingFinished could as "init" event
        this->tree_bulit();
        // super will send this event to children
        __fallthrough;
    default:
        return Super::DoEvent(arg);
    }
}

// init window
void LongUI::Demo::MainViewport::tree_bulit() {
    UIControl* control = nullptr;
    // OK
    if ((control = m_pWindow->FindControl("ok"))) {
        control->AddEventCall([](UIControl* btn) noexcept ->bool {
            UIManager.CreateUIWindow<LongUI::Demo::MainViewport>(DEMO_XML)->ShowWindow(SW_SHOW);
            return true;
        }, SubEvent::Event_ItemClicked);
    }
    // Exit
    if ((control = m_pWindow->FindControl("exit"))) {
        control->AddEventCall([](UIControl* btn) noexcept ->bool {
            UIManager.Exit();
            return true;
        }, SubEvent::Event_ItemClicked);
    }
}
