#define LONGUI_WITH_DEFAULT_HEADER
#define _CRT_SECURE_NO_WARNINGS
#include "../../LongUI/LongUI.h"

constexpr char* DEMO_XML = 
u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="512, 512">
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

// MainWindow class
class MainWindow : public LongUI::UIWindow {
    // super class
    using Super = LongUI::UIWindow;
public:
    // ctor
    MainWindow(pugi::xml_node node, LongUI::UIWindow* parent) : Super(node, parent){}
    // do some event
    virtual bool DoEvent(const LongUI::EventArgument&) noexcept override;
    // clean up
    virtual void Cleanup() noexcept override { delete this; }
private:
    // init
    void init();
    // on number button clicked
    void number_button_clicked(LongUI::UIControl* btn);
    // on plus
    bool on_plus();
    // on minus
    bool on_minus();
    // on equal
    bool on_equal();
private:
    // display
    LongUI::UIText*         m_display = nullptr;
    // cached number
    long long               m_number = 0ll;
    // string to display
    std::wstring            m_string;
};

// Entry for App
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow) {
    // every windows desktop app should do this
    ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    // use OleInitialize to init ole and com
    if (SUCCEEDED(::OleInitialize(nullptr))) {
        // init longui manager
        if (SUCCEEDED(UIManager.Initialize())) {
            // my style
            UIManager << DL_Hint << L"Battle Control Online!" << LongUI::endl;
            // create main window, return nullptr for some error
            UIManager.CreateUIWindow<MainWindow>(DEMO_XML);
            // run this app
            UIManager.Run();
            // my style
            UIManager << DL_Hint << L"Battle Control Terminated!" << LongUI::endl;
            // cleanup longui
            UIManager.UnInitialize();
        }
        // cleanup ole and com
        ::OleUninitialize();
    }
    // exit
    return EXIT_SUCCESS;
}


// ---------------------- IMPL -----------------------------------
// do event for ui
bool MainWindow::DoEvent(const LongUI::EventArgument& arg) noexcept {
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

// longui_cast
using LongUI::longui_cast;

// init window
void MainWindow::init() {
    // get display control, check error?
    m_display = longui_cast<LongUI::UIText*>(this->FindControl(L"display"));
    // +
    this->SetEventCallBack(
        L"btn_plus", 
        LongUI::Event::Event_ButtoClicked,
        [](LongUI::UIControl* uithis, LongUI::UIControl* btn) noexcept { 
            return static_cast<MainWindow*>(uithis)->on_plus(); 
        }
    );
    // -
    this->SetEventCallBack(
        L"btn_minu", 
        LongUI::Event::Event_ButtoClicked,
        [](LongUI::UIControl* uithis, LongUI::UIControl* btn) noexcept { 
            return static_cast<MainWindow*>(uithis)->on_minus(); 
        }
    );
    // =
    this->SetEventCallBack(
        L"btn_equl", 
        LongUI::Event::Event_ButtoClicked,
        [](LongUI::UIControl* uithis, LongUI::UIControl* btn) noexcept { 
            return static_cast<MainWindow*>(uithis)->on_equal(); 
        }
    );
    // 0-9
    for (auto i = 0ui32; i < 10; ++i) {
        constexpr int name_buffer_length = 16;
        wchar_t buffer[name_buffer_length];
        std::swprintf(buffer, name_buffer_length, L"btn_num%d", i);
        auto btn = this->FindControl(buffer);
        // use user_data
        if (btn) {
            btn->user_data = i + 1;
        }
    }
}



// on number button clicked
void MainWindow::number_button_clicked(LongUI::UIControl* btn) {
    if (btn->user_data) {
        try {
            register wchar_t ch = L'0' + wchar_t(btn->user_data) - 1;
            m_string += ch;
        }
        catch (std::bad_alloc&) {
            // do some thing
        }
        catch (...) {
            // do some thing
        }
        m_display->SetText(m_string.c_str());
    }
}

// on plus
bool MainWindow::on_plus() {
    m_number = _wtoll(m_string.c_str());
    m_string = L'+';
    m_display->SetText(L"+");
    return true;
}

// on minus
bool MainWindow::on_minus() {
    m_number = _wtoll(m_string.c_str());
    m_string = L'-';
    m_display->SetText(L"-");
    return true;
}

// on equal
bool MainWindow::on_equal() {
    m_number += _wtoll(m_string.c_str());
    constexpr int buffer_length = 256;
    wchar_t buffer[buffer_length];
    std::swprintf(buffer, buffer_length, L"%lld", m_number);
    m_string = buffer;
    m_display->SetText(buffer);
    return true;
}
