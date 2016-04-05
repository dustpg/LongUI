#pragma once

// window xml layout
static const char* const DEMO_XML =
u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="512, 512" debug="true" titlename="LongUI Demo Window">
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
    <HorizontalLayout>
        <Button name="btn_equl" weight="1" margin="4,4,4,4" borderwidth="1" text="="/>
        <Button name="btn_clear" weight="0.5" margin="4,4,4,4" borderwidth="1" text="C"/>
    </HorizontalLayout>
</Window>
)xml";

#include <string>

// longui::demo namespace
namespace LongUI { namespace Demo {
    // MainWindow class
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
    private:
        // tree built
        void tree_built();
        // on number button clicked
        void number_button_clicked(UIControl* btn);
        // on plus
        bool on_plus(UIControl* btn);
        // on minus
        bool on_minus(UIControl* btn);
        // on equal
        bool on_equal(UIControl* btn);
    private:
        // display
        UIControl*              m_display = nullptr;
        // cached number
        long long               m_number = 0ll;
        // string to display
        std::wstring            m_string;
    };
}}


// -------------------------- IMPLEMENT ---------------------------
// do event for ui
bool LongUI::Demo::MainViewport::DoEvent(const EventArgument& arg) noexcept {
    // longui event
    switch (arg.event)
    {
    case LongUI::Event::Event_SubEvent:
        // number button clicked event
        if (arg.ui.subevent == LongUI::SubEvent::Event_ItemClicked) {
            this->number_button_clicked(arg.sender);
        }
        return true;
    case LongUI::Event::Event_TreeBuildingFinished:
        // Event_TreeBulidingFinished could as "init" event
        this->tree_built();
        // super will send this event to children
        __fallthrough;
    default:
        return Super::DoEvent(arg);
    }
}

// init window
void LongUI::Demo::MainViewport::tree_built() {
    UIControl* control = nullptr;
    // get display control, check error?
    m_display = m_pWindow->FindControl("display");
    // +
    if ((control = m_pWindow->FindControl("btn_plus"))) {
        control->AddEventCall([this, control](UIControl* btn)->bool {
            auto test = control == btn;
            try {
                return this->on_plus(btn);
            }
            catch (...) {
                assert(!"oh no");
                return true;
            }
        }, SubEvent::Event_ItemClicked);
    }
    // -
    if ((control = m_pWindow->FindControl("btn_minu"))) {
        control->AddEventCall([this](UIControl* btn) noexcept ->bool {
            try {
                return this->on_minus(btn);
            }
            catch (...) {
                assert(!"oh no");
                return true;
            }
        }, SubEvent::Event_ItemClicked);
    }
    // =
    if ((control = m_pWindow->FindControl("btn_equl"))) {
        control->AddEventCall([this](UIControl* btn) noexcept ->bool {
            try {
                return this->on_equal(btn);
            }
            catch (...) {
                assert(!"oh no");
                return true;
            }
        }, SubEvent::Event_ItemClicked);
    }
    // C
    if ((control = m_pWindow->FindControl("btn_clear"))) {
        control->AddEventCall([this](UIControl* btn) noexcept ->bool {
            m_number = 0; m_string.clear(); 
            m_display->SetText(L"0"); 
            return true;
        }, SubEvent::Event_ItemClicked);
    }
    // 0-9
    for (auto i = 0; i < 10; ++i) {
        constexpr int name_buffer_length = 16;
        char buffer[name_buffer_length];
        std::snprintf(buffer, name_buffer_length, "btn_num%d", i);
        auto btn = m_pWindow->FindControl(buffer);
        // use user_data
        if (btn) {
            btn->user_data = size_t(i + 1);
        }
    }
}

// on number button clicked
void LongUI::Demo::MainViewport::number_button_clicked(UIControl* btn) {
    if (btn->user_data) {
        try {
            wchar_t ch = L'0' + wchar_t(btn->user_data) - 1;
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
bool LongUI::Demo::MainViewport::on_plus(UIControl* btn) {
    UNREFERENCED_PARAMETER(btn);
    m_number = _wtoll(m_string.c_str());
    m_string = L'+';
    m_display->SetText(L"+");
    return true;
}

// on minus
bool LongUI::Demo::MainViewport::on_minus(UIControl* btn) {
    UNREFERENCED_PARAMETER(btn);
    m_number = _wtoll(m_string.c_str());
    m_string = L'-';
    m_display->SetText(L"-");
    return true;
}

// on equal
bool LongUI::Demo::MainViewport::on_equal(UIControl* btn) {
    UNREFERENCED_PARAMETER(btn);
    m_number += _wtoll(m_string.c_str());
    constexpr int buffer_length = 256;
    wchar_t buffer[buffer_length];
    std::swprintf(buffer, buffer_length, L"%lld", m_number);
    m_string = buffer;
    m_display->SetText(buffer);
    return true;
}
