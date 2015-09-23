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
    // MainWindow class
    class MainWindow final : public UIWindow {
        // super class
        using Super = UIWindow;
    public:
        // ctor
        MainWindow(pugi::xml_node node, UIWindow* parent) : Super(node, parent) {}
        // do some event
        virtual bool DoEvent(const EventArgument& arg) noexcept override;
        // clean up
        virtual void Cleanup() noexcept override { delete this; }
        // clean up
        virtual bool OnClose() noexcept override {
            // only one window remain ?
            if (UIManager.GetWindowsCount() == 1) {
                UIManager.Exit();
            }
            // delete this
            else {
                this->Cleanup();
            }
            return true;
        }
    private:
        // init
        void init();
    };
}}


// -------------------------- IMPLEMENT ---------------------------
// do event for ui
bool LongUI::Demo::MainWindow::DoEvent(const EventArgument& arg) noexcept {
     
        // longui event
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_TreeBulidingFinished:
            // Event_TreeBulidingFinished could as "init" event
            this->init();
            // super will send this event to children
            __fallthrough;
        default:
            return Super::DoEvent(arg);
        }
    }
    // system event
    else {
        return Super::DoEvent(arg);
    }
}

// init window
void LongUI::Demo::MainWindow::init() {
    UIControl* control = nullptr;
    // OK
    if ((control = this->FindControl(L"ok"))) {
        control->AddEventCall([](UIControl* btn) noexcept ->bool {
            UIManager.CreateUIWindow<LongUI::Demo::MainWindow>(DEMO_XML);
            return true;
        }, SubEvent::Event_ItemClicked);
    }
    // Exit
    if ((control = this->FindControl(L"exit"))) {
        control->AddEventCall([](UIControl* btn) noexcept ->bool {
            UIManager.Exit();
            return true;
        }, SubEvent::Event_ItemClicked);
    }
}
