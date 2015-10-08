#define LONGUI_WITH_DEFAULT_HEADER
#define _CRT_SECURE_NO_WARNINGS
#include "LongUI.h"
#include "../Step3_handleeventex/demo.h"

#include <vector>

const wchar_t* const CPU_ADAPTER = L"Microsoft Basic Render Driver";

const char* const VIDEO_CARD_XML = 
u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window autoshow="false" size="1024, 512" clearcolor="1,1,1,0.9" name="Choose Video Card">
    <HorizontalLayout>
        <List weight="3" borderwidth="1" name="lst_vc" linetemplate="Text, Text" rightcontrol="ScrollBarA"
            margin="4,4,4,4" topcontrol="ListHeader, 1" bottomcontrol="ScrollBarA">
        </List>
        <VerticalLayout weight="1">
            <Null weight="4"/>
            <Button borderwidth="1" margin="4,4,4,4" name="btn_ok" text="OK! Choose It!"/>
            <Null weight="4"/>
        </VerticalLayout>
    </HorizontalLayout>
</Window>
)xml";

const char* const TEMPLATE_XML = 
u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<!-- You can use other name not limited in 'Template' -->
<Template>
    <Control desc="list header test" sepwidth="-8" >
        <Button weight="0.2" borderwidth="1" margin="1,1,1,1" text="type" name="lst_header0"/>
        <Button borderwidth="1" margin="1,1,1,1" text="name" name="lst_header1"/>
    </Control>
</Template>
)xml";


// longui::demo namespace
LONGUI_NAMESPACE_BEGIN namespace Demo {
    // config
    class MyConfig final : public CUIDefaultConfigure {
        // super class
        using Super = CUIDefaultConfigure;
    public:
        // ctor
        MyConfig() : Super(UIManager) { }
        // 获取控件模板
        auto GetTemplateString() noexcept->const char* override { return TEMPLATE_XML; }
        // return true, if use cpu rendering
        virtual auto GetConfigureFlag() noexcept->ConfigureFlag override { return Flag_OutputDebugString; }
        // choose it
        virtual auto ChooseAdapter(DXGI_ADAPTER_DESC1 adapters[], const size_t length) noexcept->size_t override;
        // adapters
        std::vector<DXGI_ADAPTER_DESC1>     adapter_vector;
    };
    // window to choose adapter
    class WindowChooseAdapter final : public UIWindow {
        // super class
        using Super = UIWindow;
        // clean up
        virtual void cleanup() noexcept override { delete this; }
    public:
        // on close event, do not exit app directly
        virtual auto OnClose() noexcept -> bool { 
            auto old = m_bExit;
            this->delay_cleanup(); 
            if (old) UIManager.Exit(); 
            return true; 
        };
        // ctor
        WindowChooseAdapter(pugi::xml_node node, UIWindow* parent) : Super(node, parent) {}
    public:
        // set config
        void SetMyConfig(MyConfig& config) noexcept;
        //
        size_t              m_bExit = true;
    };
}
LONGUI_NAMESPACE_END

template<typename... Args>
void ssss(char buf[], Args... arg) {
    std::sprintf(buf, "%d%d", arg...);
}

// Entry for App
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow) {
    {
        char buf[1024];
        ssss(buf, 0, 5);
        int bk = 9;
    }
    // every windows desktop app should do this
    ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    // use OleInitialize to init ole and com
    if (SUCCEEDED(::OleInitialize(nullptr))) {
        // my config
        LongUI::Demo::MyConfig config;
        // init longui manager
        if (SUCCEEDED(UIManager.Initialize(&config))) {
            // my style
            UIManager << DL_Hint << L"Battle Control Online!" << LongUI::endl;
            // create main window, return nullptr for some error
            auto svc_window = UIManager.CreateUIWindow<LongUI::Demo::WindowChooseAdapter>(VIDEO_CARD_XML);
            // successed
            if (svc_window) {
                svc_window->SetMyConfig(config);
            }
            // some error
            else {
                // TODO: error handle
            }
            // run this app
            UIManager.Run();
            // my style
            UIManager << DL_Hint << L"Battle Control Terminated!" << LongUI::endl;
            // cleanup longui
            UIManager.Uninitialize();
        }
        // cleanup ole and com
        ::OleUninitialize();
    }
    // exit
    return EXIT_SUCCESS;
}


// ChooseAdapter
auto LongUI::Demo::MyConfig::ChooseAdapter(DXGI_ADAPTER_DESC1 adapters[], const size_t length) noexcept -> size_t {
    // first call
    if (adapter_vector.empty()) {
        try {
            adapter_vector.assign(adapters, adapters + length);
        }
        catch (const std::exception& exp) {
            auto info = exp.what();
            assert(!info);
        }
        // CPU renderer first
        for (size_t i = 0; i < length; ++i) {
            if (!::wcscmp(CPU_ADAPTER, adapters[i].Description))
                return i;
        }
    }
    // stored infomation
    else {
        for (size_t i = 0; i < length; ++i) {
            if (!::wcscmp(adapter_vector.back().Description, adapters[i].Description))
                return i;
        }
    }
    return length;
}

// set MyConfig
void LongUI::Demo::WindowChooseAdapter::SetMyConfig(MyConfig& config) noexcept {
    // find list that named "lst_vc"
    auto list = longui_cast<UIList*>(this->FindControl("lst_vc"));
    // if found
    if (list) {
        // add infomation to list via 'adapter_vector'
        for (auto i = 0u; i < config.adapter_vector.size(); ++i) {
            // insert line-template to the list
            auto line = list->InsertLineTemplateToList(i);
            // succeeded
            if (line) {
                // assert if line children count doesn't equal
                // because we just keep "TYPE" and "NAME" in the list
                assert(line->GetCount() == 2);
                // set "TYPE" in 1st line slot
                auto text = config.adapter_vector[i].Description;
                if (std::wcscmp(text, CPU_ADAPTER)) {
                    line->GetAt(0)->SetText(L"GPU");
                }
                else {
                    line->GetAt(0)->SetText(L"CPU");
                }
                // set "NAME" in 2nd line slot
                line->GetAt(1)->SetText(text);
            }
        }
        // find button that named "btn_ok"
        auto btn = this->FindControl("btn_ok");
        // if found
        if (btn) {
            // I wanna to capture 'adapter_vector'
            auto& vec = config.adapter_vector;
            // add event callback
            btn->AddEventCall([&vec, list, this](UIControl*) noexcept ->bool {
                // if selectED
                auto& sel = list->GetSelectedIndices();
                if (sel.size()) {
                    vec.push_back(vec[sel.front()]);
                }
                else {
                    ::MessageBoxW(m_hwnd, L"You may choose one adapter", L"HINT", MB_OK);
                }
                // close this window later
                this->CloseWindowLater();
                // do not exit
                m_bExit = false;
                // recreate
                UIManager.RecreateResources();
                // create main window
                UIManager.CreateUIWindow<LongUI::Demo::MainWindow>(DEMO_XML);
                return true;
            }, SubEvent::Event_ItemClicked);
        }
    }
    // show the window
    this->ShowWindow(SW_SHOW);
}

/*

            <ListLine name="lin1">
                <Text text="1" name="listline1-1"/>
                <Text text="伍湖"/>
            </ListLine>
            <ListLine name="lin2">
                <Text text="2" name="listline2-1"/>
                <Text text="亖死"/>
            </ListLine>
            <ListLine name="lin3">
                <Text text="3" name="listline3-1"/>
                <Text text="三妻"/>
            </ListLine>
            <ListLine name="lin4">
                <Text text="5" name="listline4-1"/>
                <Text text="二连"/>
            </ListLine>
            <ListLine name="lin5">
                <Text text="4" name="listline5-1"/>
                <Text text="一瞬"/>
            </ListLine>
            <ListLine>
                <Text text="4"/>
                <Text text="一瞬"/>
            </ListLine>
            <ListLine>
                <Text text="4"/>
                <Text text="一瞬"/>
            </ListLine>
            <ListLine>
                <Text text="4"/>
                <Text text="一瞬"/>
            </ListLine>
            <ListLine>
                <Text text="4"/>
                <Text text="一瞬"/>
            </ListLine>
*/