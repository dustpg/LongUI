#pragma once

// window xml layout
static const char* const DEMO_XML =
u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="512, 512" debug="true" name="LongUI Demo2 Window">
    <Text name="display" text="0"/>
    <HorizontalLayout>
        <Button name="btn_num7" margin="4,4,4,4" borderwidth="1" text="7"/>
        <Button name="btn_num8" margin="4,4,4,4" borderwidth="1" text="8"/>
        <Button name="btn_num9" margin="4,4,4,4" borderwidth="1" text="9"/>
    </HorizontalLayout>
    <HorizontalLayout>
        <Button name="btn_num4" margin="4,4,4,4" borderwidth="1" text="4"/>
        <Button name="btn_num5" margin="4,4,4,4" borderwidth="1" text="5"/>
        <Button name="btn_num6" margin="4,4,4,4" borderwidth="1" text="6"/>
    </HorizontalLayout>
    <HorizontalLayout>
        <Button name="btn_num1" margin="4,4,4,4" borderwidth="1" text="1"/>
        <Button name="btn_num2" margin="4,4,4,4" borderwidth="1" text="2"/>
        <Button name="btn_num3" margin="4,4,4,4" borderwidth="1" text="3"/>
    </HorizontalLayout>
    <HorizontalLayout>
        <Button name="btn_plus" margin="4,4,4,4" borderwidth="1" text="+"/>
        <Button name="btn_num0" margin="4,4,4,4" borderwidth="1" text="0"/>
        <Button name="btn_minu" margin="4,4,4,4" borderwidth="1" text="-"/>
    </HorizontalLayout>
    <Button name="btn_equl" margin="4,4,4,4" borderwidth="1" text="="/>
</Window>
)xml";

// longui::demo namespace
namespace LongUI { namespace Demo {
    // MainWindow class
    class MainWindow : public UIWindow {
        // super class
        using Super = UIWindow;
    public:
        // ctor
        MainWindow(pugi::xml_node node, UIWindow* parent) : Super(node, parent) {}
        // do some event
        virtual bool DoEvent(const EventArgument&) noexcept override;
        // clean up
        virtual void Cleanup() noexcept override { delete this; }
    private:
        // init
        void init();
        // on number button clicked
        void number_button_clicked(UIControl* btn);
        // on plus
        bool on_plus();
        // on minus
        bool on_minus();
        // on equal
        bool on_equal();
    private:
        // display
        UIText*                 m_display = nullptr;
        // cached number
        long long               m_number = 0ll;
        // string to display
        std::wstring            m_string;
    };
}}


// -------------------------- IMPLEMENT ---------------------------
// do event for ui
bool LongUI::Demo::MainWindow::DoEvent(const EventArgument& arg) noexcept {
    // longui event
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_ButtoClicked:
            // number button clicked event
            this->number_button_clicked(arg.sender);
            return true;
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
    // get display control, check error?
    m_display = longui_cast<UIText*>(this->FindControl(L"display"));
    // +
    this->SetEventCallBack(
        L"btn_plus",
        LongUI::Event::Event_ButtoClicked,
        [](UIControl* uithis, UIControl* btn) noexcept {
            return static_cast<MainWindow*>(uithis)->on_plus();
        }
    );
    // -
    this->SetEventCallBack(
        L"btn_minu",
        LongUI::Event::Event_ButtoClicked,
        [](UIControl* uithis, UIControl* btn) noexcept {
            return static_cast<MainWindow*>(uithis)->on_minus();
        }
    );
    // =
    this->SetEventCallBack(
        L"btn_equl",
        LongUI::Event::Event_ButtoClicked,
        [](UIControl* uithis, UIControl* btn) noexcept {
            return static_cast<MainWindow*>(uithis)->on_equal();
        }
    );
    // 0-9
    for (auto i = 0; i < 10; ++i) {
        constexpr int name_buffer_length = 16;
        wchar_t buffer[name_buffer_length];
        std::swprintf(buffer, name_buffer_length, L"btn_num%d", i);
        auto btn = this->FindControl(buffer);
        // use user_data
        if (btn) {
            btn->user_data = size_t(i + 1);
        }
    }
}

// on number button clicked
void LongUI::Demo::MainWindow::number_button_clicked(UIControl* btn) {
    if (btn->user_data) {
        try {
            register wchar_t ch = L'0' + wchar_t(btn->user_data) - 1;
            m_string += ch;
        }
        catch (std::bad_alloc& exp) {
            UNREFERENCED_PARAMETER(exp);
            // do some thing
        }
        catch (...) {
            // do some thing
        }
        m_display->SetText(m_string.c_str());
    }
}

// on plus
bool LongUI::Demo::MainWindow::on_plus() {
    m_number = _wtoll(m_string.c_str());
    m_string = L'+';
    m_display->SetText(L"+");
    return true;
}

// on minus
bool LongUI::Demo::MainWindow::on_minus() {
    m_number = _wtoll(m_string.c_str());
    m_string = L'-';
    m_display->SetText(L"-");
    return true;
}

// on equal
bool LongUI::Demo::MainWindow::on_equal() {
    m_number += _wtoll(m_string.c_str());
    constexpr int buffer_length = 256;
    wchar_t buffer[buffer_length];
    std::swprintf(buffer, buffer_length, L"%lld", m_number);
    m_string = buffer;
    m_display->SetText(buffer);
    return true;
}
