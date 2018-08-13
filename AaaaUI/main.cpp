/*
    THIS SOURCE FILE FOR TESTING
*/



#define _CRT_SECURE_NO_WARNINGS
#define CASE_NUM (16)

#include <core/ui_string.h>
#include <core/ui_manager.h>
#include <core/ui_malloc.h>
#include <core/ui_ctrlmeta.h>
#include <debugger/ui_debug.h>
#include <control/ui_viewport.h>
#include <graphics/ui_matrix3x2.h>

#include <core/ui_color_list.h>
#include <container/pod_hash.h>
#include <container/nonpod_vector.h>

#include <vector>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void main_inited(LongUI::UIViewport&, int) noexcept;
void object_test() noexcept;

namespace LongUI {
    // BKDR Hash Function
    auto BKDRHash(const char* ) noexcept->uint32_t;
}

extern const char* xml_text_box;
extern const char* xul_layout;
extern const char* xul_popup;
extern const char* xul_listbox;
extern const char* xul_tree;
extern const char* xul_tree2;
extern const char* xul_test;
extern const char* xul_tabs;


#include <interface/ui_default_config.h>
#include <text/ui_ctl_arg.h>

struct HelloConfig : public LongUI::CUIDefaultConfigure {
    using LongUI::IUIConfigure::ConfigureFlag;
    // get def-font
    void DefaultFontArg(LongUI::FontArg& arg) noexcept override {
        //CUIDefaultConfigure::DefaultFontArg(arg);
        arg.size = 20.f;
        //arg.family = "SimSun";
    }
    // get cfg-flag
    ConfigureFlag GetConfigureFlag() noexcept override {
        return ConfigureFlag::Flag_None
            | ConfigureFlag::Flag_OutputDebugString
            | ConfigureFlag::Flag_QuitOnLastWindowClosed
            //| ConfigureFlag::Flag_DbgOutputTimeTook 
//#ifndef NDEBUG
 
//#endif
            //| ConfigureFlag::Flag_DbgDrawDirtyRect
            | ConfigureFlag::Flag_DbgDrawTextCell
            | ConfigureFlag::Flag_DbgDebugWindow
            | ConfigureFlag::Flag_NoAutoScaleOnHighDpi
            ;
    }
};

#pragma comment(lib, "CodeUI")
#include <control/ui_radiogroup.h>
#include <control/ui_menupopup.h>
#include <control/ui_menuitem.h>
#include <control/ui_menulist.h>
#include <control/ui_progress.h>
#include <control/ui_checkbox.h>
#include <control/ui_textbox.h>
#include <control/ui_button.h>
#include <control/ui_spacer.h>
#include <control/ui_scale.h>
#include <control/ui_radio.h>
#include <control/ui_image.h>
#include <control/ui_label.h>
#include <control/ui_test.h>


__declspec(noinline)
float DoIT(LongUI::UIViewport& a) noexcept {
    float f = 0.f;
    for (auto&x : a)
        f += x.GetSize().width;
    return f;
}

struct MemoryLeakDetector {
#ifndef NDEBUG
    // mem state
    _CrtMemState memstate[3];
    // ctor
    MemoryLeakDetector() noexcept { ::_CrtMemCheckpoint(memstate + 0); }
    // dtor
    ~MemoryLeakDetector() noexcept {
        ::_CrtMemCheckpoint(memstate + 1);
        if (::_CrtMemDifference(memstate + 2, memstate + 0, memstate + 1)) {
            ::_CrtDumpMemoryLeaks();
            assert(!"OOps! Memory leak detected");
        }
    }
#else
    ~MemoryLeakDetector() noexcept {}
#endif
};

#include <shlwapi.h>
#include <CommCtrl.h>

#define PACKVERSION(major,minor) MAKELONG(minor,major)

DWORD GetVersion(const wchar_t* lpszDllName)
{
    HINSTANCE hinstDll;
    DWORD dwVersion = 0;

    // For security purposes, LoadLibrary should be provided with a fully qualified 
    // path to the DLL. The lpszDllName variable should be tested to ensure that it 
    // is a fully qualified path before it is used. 
    hinstDll = LoadLibraryW(lpszDllName);

    if (hinstDll)
    {
        DLLGETVERSIONPROC pDllGetVersion;
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");

        // Because some DLLs might not implement this function, you must test for 
        // it explicitly. Depending on the particular DLL, the lack of a DllGetVersion 
        // function can be a useful indicator of the version. 

        if (pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            HRESULT hr;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);

            hr = (*pDllGetVersion)(&dvi);

            if (SUCCEEDED(hr))
            {
                dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
            }
        }
        FreeLibrary(hinstDll);
    }
    return dwVersion;
}

//#pragma comment(lib, "Comctl32")


extern "C" int CALLBACK WinMain(HINSTANCE, HINSTANCE, char*, int) {
    //LongUI::wcharxx_t a;
    const auto code = LongUI::BKDRHash("tooltiptext");
    ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    MemoryLeakDetector dtr;
    HelloConfig cfg;
    if (UIManager.Initialize(&cfg)) {
        LUIDebug(Hint) << "Battle Control Online..." << LongUI::endl;
        ::object_test();
        //const auto ptr1 = LongUI::NormalAlloc(1024);
        //const auto ptr2 = LongUI::NormalRealloc(ptr1, 2048);
        //LongUI::NormalFree(ptr2);
        {
            LongUI::UIViewport viewport1{
                nullptr,
                LongUI::CUIWindow::Config_Default
                //| LongUI::CUIWindow::Config_LayeredWindow
            };
            //viewport1.RefWindow().SetClearColor({ 1,1,1,0.0 });
            viewport1.RefWindow().SetClearColor({ 1,1,1,1 });
#if 1
            main_inited(viewport1, CASE_NUM);
            /*viewport1.AddSpacer({ 100, 100 }, 1);
            (new(std::nothrow) LongUI::UILabel{ &viewport1 })->SetText(u"hello");
            LongUI::UILabel world{ &viewport1 };
            world.SetText(u"world");
            LongUI::UILabel{ &viewport1 }.SetText(u"?!");*/
#else
            viewport1.RefWindow().ShowWindow();
            viewport1.RefWindow().SetTitleName(u"Window");
            viewport1.SetBgColor({ LongUI::RGBA_White });
            viewport1.SetDebugName("root");
            LongUI::Result::GetSystemLastError();
#endif
            std::printf("%f\n", DoIT(viewport1));
            
            viewport1.RefWindow().Exec();
        }
        LUIDebug(Hint) << "Battle Control Terminated." << LongUI::endl;
        UIManager.Uninitialize();
    }
    else assert(!"error");
    return 0;
}


LongUI::UIControl* vv;

#include <util/ui_lastsort.h>
#include <core/ui_time_capsule.h>


void main_inited(LongUI::UIViewport& viewport, int switch_on) noexcept {
    UIManager.SetXulDir(LongUI::U8View::FromCStyle("../doc/test-xul"));
    vv = &viewport;
    // **测试** 用
    viewport.SetAutoOverflow();
    //auto& style = const_cast<LongUI::Style&>(viewport.GetStyle());
    //style.overflow_x = LongUI::Overflow_Auto;
    //style.overflow_y = LongUI::Overflow_Auto;
    {
        using namespace LongUI;
        const auto a = UIManager.CreateTimeCapsule([](float p) noexcept {
        }, 1.f, &viewport);
        const auto b = UIManager.CreateTimeCapsule([](float p) noexcept {
        }, 100.f, &viewport);
        const auto c = UIManager.CreateTimeCapsule([](float p) noexcept {
        }, 10.f, &viewport);
        const auto d = UIManager.CreateTimeCapsule([](float p) noexcept {
        }, 5.f, &viewport);
        const auto e = UIManager.CreateTimeCapsule([](float p) noexcept {
        }, 50.f);
        a->Restart();
        b->Restart();
        b->Terminate();
        c->Terminate();
        e->Terminate();
    }
    {
        LongUI::POD::Vector<int> v;
        v.resize(1024);
        const auto a = v.capacity();
        v.resize(1000);
        const auto b = v.capacity();
        v.resize(1025);
        const auto c = v.capacity();
        v[0];
    }
    //{
    //    LongUI::POD::HashMap<int> as;
    //    const auto view1 = LongUI::U8View::FromCStyle(u8R"(../doc/test-xul/images/25.png)");
    //    const auto view2 = LongUI::U8View::FromCStyle(u8R"(../doc/test-xul/)");
    //    as.insert(view1.begin(), view1.end(), 123);
    //    as.insert(view2.begin(), view2.end(), 321);
    //    as.insert(view2.begin(), view2.end(), 321);
    //}
    {
        auto a = LongUI::Matrix::Matrix3x2F::Scale({ 2, 2 });
        auto b = LongUI::Matrix::Matrix3x2F::Translation({ 100, 100 });
        auto c = a * b;
        auto d = b * a;
        int bk = 9;
    }
    viewport.RefWindow().ShowWindow();
    viewport.RefWindow().SetTitleName(u"Window");
    //auto z = new(std::nothrow) LongUI::UIVBoxLayout;
    //set_root(z); z->user_data = reinterpret_cast<uintptr_t>(ptr);
    //viewport.SetBgColor({ LongUI::RGBA_White });
    viewport.SetDebugName("root");

    const auto loadfile = [&viewport](const char* filename) noexcept {
        if (const auto file = std::fopen(filename, "r")) {
            std::fseek(file, 0L, SEEK_END);
            const auto len = std::ftell(file);
            if (const auto ptr = std::malloc(len + 1)) {
                std::fseek(file, 0L, SEEK_SET);
                std::fread(ptr, len, 1, file);
                const auto str = reinterpret_cast<char*>(ptr);
                str[len] = 0;
                viewport.SetXul(str);
                std::free(ptr);
            }
            std::fclose(file);
        }
    };

    LongUI::UISlider* slider = nullptr;
    auto add_spacer = [](LongUI::UIControl* p) noexcept {
        new(std::nothrow) LongUI::UISpacer{ p };
    };
    auto add_vbox = [](LongUI::UIControl* p, uint32_t color, const char* name) noexcept {
        const auto c = new(std::nothrow) LongUI::UIVBoxLayout{ p };
        //c->SetBgColor({ color }); c->SetDebugName(name);
        return c;
    };
    auto add_btn = [](LongUI::UIControl* p, const char* name) noexcept {
        const auto c = new(std::nothrow) LongUI::UIButton{ p };
        c->SetDebugName(name);
        return c;
    };
    auto add_ctrl = [](LongUI::UIControl* p, uint32_t color, const char* name) noexcept {
        const auto c = new(std::nothrow) LongUI::UIControl{ p };
        //c->SetBgColor({ color }); c->SetDebugName(name);
        return c;
    };
    auto add_img= [](LongUI::UIControl* p, const char* name) noexcept {
        const auto c = new(std::nothrow) LongUI::UIImage{ p };
        c->SetDebugName(name);
        return c;
    };
    auto add_slider = [](LongUI::UIControl* p, const char* name) noexcept {
        const auto c = new(std::nothrow) LongUI::UIScale{ p };
        c->SetDebugName(name);
        return c;
    };
    auto add_label = [](LongUI::UIControl* p, uint32_t color, const char* name) noexcept {
        const auto c = new(std::nothrow) LongUI::UILabel{ p };
        //c->SetBgColor({ color }); 
        c->SetDebugName(name);
        return c;
    };
    auto add_txtbox = [](LongUI::UIControl* p, uint32_t color, const char* name) noexcept {
        const auto c = new(std::nothrow) LongUI::UITextBox{ p };
        //c->SetBgColor({ color });
        c->SetDebugName(name);
        return c;
    };
    auto add_checkbox = [](LongUI::UIControl* p, const char* name) noexcept {
        const auto c = new(std::nothrow) LongUI::UICheckBox{ p };
        c->SetDebugName(name);
        return c;
    };
    auto add_radio = [](LongUI::UIControl* p, const char* name) noexcept {
        const auto c = new(std::nothrow) LongUI::UIRadio{ p };
        c->SetDebugName(name);
        return c;
    };
    auto add_radiogroup = [](LongUI::UIControl* p, const char* name) noexcept {
        const auto c = new(std::nothrow) LongUI::UIRadioGroup{ p };
        c->SetDebugName(name);
        return c;
    };
    auto add_combo = [](LongUI::UIControl* p, const char* name) noexcept {
        const auto c = new(std::nothrow) LongUI::UIMenuList{ p };
        c->SetDebugName(name);
        return c;
    };
    auto add_menuitem = [](LongUI::UIControl* p, const char* name) noexcept {
        const auto c = new(std::nothrow) LongUI::UIMenuItem{ p };
        c->SetDebugName(name);
        return c;
    };
    auto add_test = [](LongUI::UIControl* p, uint32_t color, const char* name) noexcept {
#ifdef NDEBUG
        const auto c = new(std::nothrow) LongUI::UIControl{ p };
#else
        const auto c = new(std::nothrow) LongUI::UITest{ p };
#endif
        //c->SetBgColor({ color }); c->SetDebugName(name);
        return c;
    };
    auto do_button = [](LongUI::CUIWindow*window, const char* name) noexcept {
        const auto ctrl = window->FindControl(name);
        assert(ctrl && "404");
        const auto btn = longui_cast<LongUI::UIButton*>(ctrl);
        return btn;
    };
    auto init_flex = [](LongUI::UIControl* c, float flex) noexcept {
        const_cast<float&>(c->GetStyle().flex) = flex;
    };
    switch (switch_on)
    {
    case 0:
    {
        const auto vbox = new(std::nothrow) LongUI::UIVBoxLayout{ &viewport };
        vbox->SetDebugName("root-vbox");
        //vbox->SetBgColor({ LongUI::RGBA_Cyan });
        //vbox->SetFlex_test(1.f);

        const auto lbl1 = add_label(vbox, LongUI::RGBA_Red, "vbox-label1");
        lbl1->SetText(u"hello world!-x-");


        const auto vbox2 = add_vbox(vbox, LongUI::RGBA_Cyan, "vbox-vbox2");

        const auto vbox3 = add_vbox(vbox2, LongUI::RGBA_Cyan, "vbox2-vbox3");

        const auto ctrl = add_ctrl(vbox3, LongUI::RGBA_Blue, "vbox3-ctrl");
        ctrl->SetStyleMinSize({ 0, 30 });
        break;
    }
    case 1:
        
    {
        const auto vbox = new(std::nothrow) LongUI::UIVBoxLayout{ &viewport };
        vbox->SetDebugName("root-vbox");
        init_flex(vbox, 1.f);

        const auto hbox1 = new(std::nothrow) LongUI::UIHBoxLayout{ vbox };
        hbox1->SetDebugName("vbox-hbox1");
        init_flex(vbox, 1.f);
        const_cast<float&>(hbox1->GetStyle().flex) = 1.f;
        // hbox1->SetStyleMinSize({ 640, 0 });
        {
            //hbox1->SetDebugName("hbox1")
            const auto a = add_test(hbox1, LongUI::RGBA_Green, "hbox1-a");
            const auto b = add_btn (hbox1, "hbox1-b");
            const auto c = add_ctrl(hbox1, LongUI::RGBA_Blue,  "hbox1-c");

            a->SetStyleMinSize({ 50, 30 });
            b->SetStyleMinSize({ 100, 30 });
            c->SetStyleMinSize({ 200, 1000 });
            init_flex(c, 1.f);
        }
        //const auto hbox2 = new(std::nothrow) LongUI::UIHBoxLayout{ vbox };
        //const auto hbox3 = new(std::nothrow) LongUI::UIHBoxLayout{ vbox };

        
        //hbox2->Init(); hbox2->color = 0x00ffff;
        //hbox3->Init();
        break;
    }
    case 2:
    {
        /*const auto box = new(std::nothrow) LongUI::UIHBoxLayout{ &viewport };
        box->SetDebugName("root-box");
        box->Set_test(LongUI::Align_Center);

        const auto a = add_ctrl(box, LongUI::RGBA_Red, "box-a");
        a->SetStyleMinSize({ 20, 20 });

        const auto b = add_btn(box, "box-b{btn}");
        b->AddGuiEventListener(b->_clicked(), [](LongUI::UIControl& ctrl) {
            auto& s = static_cast<LongUI::UIButton&>(ctrl);
            volatile auto value = s.GetText();
            return LongUI::Event_Accept;
        });
        b->SetStyleMinSize({ 0, 30 });
        init_flex(c, 1.f);

        const auto c = add_ctrl(box, LongUI::RGBA_Red, "box-c");
        c->Set_test(LongUI::Appearance_ScrollBarButtonLeft);

        const auto d = add_ctrl(box, LongUI::RGBA_Red, "box-d");
        d->Set_test(LongUI::Appearance_ScrollBarButtonDown);

        const auto e = add_ctrl(box, LongUI::RGBA_Red, "box-e");
        e->Set_test(LongUI::Appearance_ScrollBarButtonRight);

        const auto f = add_ctrl(box, LongUI::RGBA_Red, "box-f");
        f->Set_test(LongUI::Appearance_ScrollBarButtonUp);

        const auto g = add_img(box, "box-g");
        g->Set_test(LongUI::Appearance_Radio);


        const auto box2 = new(std::nothrow) LongUI::UIHBoxLayout{ &viewport };
        box2->SetDebugName("root-box2");
        //box2->Set_test(LongUI::Align_Center);

        const auto h = add_slider(box2, "box2-h");
        h->AddGuiEventListener(h->_changed(), [](LongUI::UIControl& ctrl) {
            auto& s = static_cast<LongUI::UIScale&>(ctrl);
            volatile auto value = s.GetValue();
            return LongUI::Event_Accept;
        });
        slider = h;
        slider->SetValue(1.f);
        h->SetFlex_test(1.f);


        const auto j = add_ctrl(box2, LongUI::RGBA_Red, "box-j");
        j->Set_test(LongUI::Appearance_ScrollBarButtonDown);

        const auto box3 = new(std::nothrow) LongUI::UIHBoxLayout{ &viewport };
        box3->SetDebugName("root-box3");
        box3->SetFlex_test(1.f);

        const auto tb = add_txtbox(box3, LongUI::RGBA_Cyan, "box-j");
        tb->SetFlex_test(1.f);
        //tb->Set_test(LongUI::Appearance_ScrollBarButtonDown);
        */
        break;
    }
    case 3:
    {
#define nonst
        const auto checkbox = add_checkbox(&viewport, "root-checkbox");

        const auto radiog = add_radiogroup(&viewport, "root-radiogroup");

        nonst auto radio = add_radio(radiog, "radiogroup-radio");
        nonst auto radio2 = add_radio(radiog, "radiogroup-radio2");
        nonst auto radio3 = add_radio(radiog, "radiogroup-radio3");

        const auto button = add_btn(&viewport, "root-button");
        const auto button2 = add_btn(&viewport, "root-button2");

        const auto combo = add_combo(&viewport, "root-combo");
        const auto popup = new(std::nothrow) LongUI::UIMenuPopup{ combo };
        popup->SetDebugName("menu");
        //popup->SetBgColor({ LongUI::RGBA_White });

        add_menuitem(popup, "m0");
        add_menuitem(popup, "m1");
        add_menuitem(popup, "m2");


        button2->SetText(u"取消");
        radio2->SetText(u"单选按钮2");
        button->AddGuiEventListener(button->_onCommand(), [=](LongUI::UIControl& ctrl) mutable {
            if (radio) {
                radio->SetParent(nullptr);
                radio->DeleteLater();
                radio = nullptr;
            }
            return LongUI::Event_Accept;
        });

        const auto viewptr = &viewport;
        button2->AddGuiEventListener(button2->_onCommand(), [=](LongUI::UIControl& ctrl) mutable {
            const auto parent = &viewptr->RefWindow();
            //const auto child = new(std::nothrow) LongUI::UIViewport{
            //    parent, LongUI::CUIWindow::Config_Popup
            //};
            ////child->RefWindow().InitAsPopup();
            //child->RefWindow().ShowNoActivate();

            //child->RefWindow().SetTitleName(u"Test");

            //child->DeleteLater();
            return LongUI::Event_Accept;
        });

        break;
    }
    case 4:
        viewport.SetXul(xul_layout);
        viewport.GetWindow()->ResizeAbsolute({ 640, 480 });
        break;
    case 5:
        viewport.SetXul(xul_test);
        {
            const auto window = viewport.GetWindow();
            const auto button1 = window->FindControl("button+");
            const auto button2 = window->FindControl("button-");
            const auto pmeter = window->FindControl("progressmeter");
            if (button1 && pmeter) {
                using LongUI::longui_cast;
                using LongUI::uisafe_cast;
                const auto ptr1 = longui_cast<LongUI::UIButton*>(button1);
                const auto ptr2 = longui_cast<LongUI::UIButton*>(button2);
                const auto meter = longui_cast<LongUI::UIProgress*>(pmeter);
                ptr1->AddGuiEventListener(ptr1->_onCommand(), [=](LongUI::UIControl& ctrl) noexcept {
                    meter->SetValue(meter->GetValue() + 10.f);
                    return LongUI::Event_Accept;
                });
                ptr2->AddGuiEventListener(ptr2->_onCommand(), [=](LongUI::UIControl& ctrl) noexcept {
                    ptr2->SetText(u"++++");
                    meter->SetValue(meter->GetValue() - 10.f);
                    return LongUI::Event_Accept;
                });
            }
        }
        break;
    case 6:
        viewport.SetXul(xul_popup);
        viewport.GetWindow()->ResizeAbsolute({ 640, 480 });
        break;
    case 7:
        viewport.SetXul(xul_listbox);
        break;
    case 8:
        viewport.SetXul(xul_tree);
        viewport.GetWindow()->ResizeAbsolute({ 640, 480 });
        break;
    case 9:
        viewport.SetXul(xul_tree2);
        break;
    case 10:
        viewport.SetXul(xul_tabs);
        break;
    case 11:
        viewport.SetXul(xml_text_box);
        break;
    case 12:
        loadfile("../doc/test-xul/list.xul");
        viewport.GetWindow()->ResizeAbsolute({ 800, 600 });
        break;
    case 13:
        loadfile("../doc/test-xul/tree-simple.xul");
        viewport.GetWindow()->ResizeAbsolute({ 800, 600 });
        break;
    case 14:
        loadfile("../doc/test-xul/tree-nested.xul");
        viewport.GetWindow()->ResizeAbsolute({ 800, 600 });
        break;
    case 15:
        loadfile("../doc/test-xul/layout.xul");
        viewport.GetWindow()->ResizeAbsolute({ 800, 600 });
        break;
    case 16:
        loadfile("../doc/test-xul/css-test.xul");
        viewport.GetWindow()->ResizeAbsolute({ 800, 600 });
        do_button(viewport.GetWindow(), "btn1")->AddGuiEventListener(
            LongUI::UIButton::_onCommand(), [](LongUI::UIControl& control) noexcept {
            const auto window = control.GetWindow();
            //if (const auto img1 = window->FindControl("img1")) {
            //    img1->DeleteLater();
            //}
            //else if (const auto img2 = window->FindControl("img2")) {
            //    const auto img = longui_cast<LongUI::UIImage*>(img2);
            //    static int a = 0;
            //    const auto view = LongUI::U8View::FromCStyle(
            //        (a++) & 1 ? "" : "images/25.png");
            //    img->SetSource(view);
            //}
            int rv_msgbox = 0;
            {
                LongUI::CUIBlockingGuiOpAutoUnlocker unlocker;
                const auto text = L"ASDDSA";
                const auto capt = L"QWEEWQ";
                rv_msgbox = ::MessageBoxW(window->GetHwnd(), text, capt, MB_YESNO);
            }
            if (rv_msgbox == IDYES) {
                int bk = 9;
            }
            return LongUI::Event_Accept;
        });
        //do_button(viewport.GetWindow(), "bgimage")->AddGuiEventListener(
        //    LongUI::UIButton::_clicked(), [](LongUI::UIControl& control) noexcept {
        //    UIManager.CreateTimeCapsule([](float p) noexcept {
        //        p *= 2.f;
        //    }, 2.f, &control);
        //    UIManager.CreateTimeCapsule([](float p) noexcept {
        //        p *= 2.f;
        //    }, 1.f, &control);
        //    UIManager.CreateTimeCapsule([](float p) noexcept {
        //        p *= 2.f;
        //    }, 3.f, &control);
        //    control.DeleteLater();
        //    return LongUI::Event_Accept;
        //});
        break;
    case 17:
        loadfile("../doc/test-xul/ctxmenu.xul");
        //viewport.GetWindow()->ResizeAbsolute({ 800, 600 });
        break;
    case 18:
        loadfile("../doc/test-xul/hidpi.xul");
        viewport.GetWindow()->ResizeAbsolute({ 800, 600 });
        break;
    case 19:
        loadfile("../doc/test-xul/menubar.xul");
        viewport.GetWindow()->ResizeAbsolute({ 800, 600 });
        break;
    case 20:
        loadfile("../doc/test-xul/division.xul");
        viewport.GetWindow()->ResizeAbsolute({ 800, 600 });
        break;
    }




    //LUIDebug(Log) << LongUI::endl;

    LongUI::StyleState state;
    state.Init();
    int bk = 9;
    state.Change({ LongUI::StyleStateType::Type_Checked, true });
    bk = 9;;
    state.Change({ LongUI::StyleStateType::Type_Checked, false });
    bk = 9;

    LongUI::CUIString str;
    str.format(u"%f%ls", 1.f, u"𪚥");
    LongUI::POD::Vector<double> a = { 32.f, 16.f, 8.f, 7.f };
    auto longx4x4 = LongUI::CUIString::FromUtf8(u8"𪚥𪚥𪚥𪚥");
    longx4x4 += longx4x4;
    //auto c = LongUI::CUIStringEx::FromUtf8(u8"𪚥𪚥𪚥𪚥");
    //LongUI::CUIStringU8 str;
    //auto len2 = U::GetBufferLength<U::UTF16>(str.view());
    //auto len3 = U::GetBufferLength<U::UTF32>(str.view());
    LongUI::Result::GetSystemLastError();
}

#include <chrono>

void object_test() noexcept {
    //LongUI::CUIStringEx c = L"壕";
    LongUI::CUIString aaaa;
    LongUI::POD::Vector<const char*> aasaa;
    aasaa.push_back("asd");
    aasaa.push_back("asd");
    aasaa.push_back("asd");
#ifdef LUI_NONPOD_VECTOR
    LongUI::NonPOD::Vector<LongUI::CUIStringEx> a, b;
    a.reserve(10);
    a.emplace_back_ex()->assign(u"SAD");
    a.emplace_back_ex()->assign(u"POI");
    a.emplace_back_ex()->assign(u"OK!");
    a.push_back(c);
    a.pop_back();
    auto& str = a[1];
    for (int i = 0; i != 40; ++i)
        a[2].append(u"喔喔窝");
    a.erase(1);
    str.append(u" DESU!");
    str.shrink_to_fit();
    //a.emplace_ex(2, 2)->assign(u"OK!");
    b = a;
    b = std::move(a);
    b.resize(9);
#endif
    {
        LongUI::CUIString b1, b2;
        LongUI::CUIString a = u"++++";
        b1 = a;
        b2 = std::move(a);
    }

    std::free(std::realloc(std::malloc(100), 128));
    volatile int bk = 9;
}



const char* xml_text_box = u8R"(<?xml version="1.0"?>
<?xml-stylesheet href="chrome://global/skin" type="text/css"?>

<window title="XUL Textboxes"
        xmlns:html="http://www.w3.org/1999/xhtml"
        xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul">

<script>
<![CDATA[

function setText(textBoxID)
{
  var result = document.getElementById(textBoxID).value;
  document.getElementById("result").value = result;
}

]]>
</script>

<!--description><html:h1>XUL Textboxes</html:h1></description-->

<vbox flex="1" style="overflow: auto" >

   <hbox align="center">
      <label style="width:10em" value="Default:"/>
      <textbox id="default" oninput="setText('default');" />
   </hbox>
   <hbox align="center">
      <label style="width:10em" value="Disabled:" />
      <textbox id="disabled" disabled="true" value="disabled" size="10"
               oninput="setText('disabled');" />
   </hbox>
   <hbox align="center">
      <label style="width:10em"  value="Readonly:" />
      <textbox id="readonly" readonly="true" value="readonly"  size="30"
               oninput="setText('readonly');" />
   </hbox>
   <hbox align="center">
      <label style="width:10em" value="Max length of 20:"  />
      <textbox id="max-length" maxlength="20"  size="20"
               oninput="setText('max-length');" />
   </hbox>
   <hbox align="center">
      <label style="width:10em" value="Password:"  />
      <textbox id="password" type="password" password="F"
               oninput="setText('password');" />
   </hbox>
   <hbox align="center">
      <label style="width:10em" value="Multiline:"  />
      <textbox id="multiline" multiline="true" rows="7" cols="20"
               oninput="setText('multiline');" />
   </hbox>

   <hbox  align="center">
      <label style="width:10em" value="Input:"  />
   </hbox>

</vbox>
</window>
)";

const char* xul_tabs = u8R"(<?xml version="1.0"?>
<?xml-stylesheet href="chrome://global/skin" type="text/css"?>
<window title="XUL Layout"
        xmlns:html="http://www.w3.org/1999/xhtml"
        xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul">
<groupbox flex="1">
    <caption label="tab" />
    <tabbox flex="1">
        <tabs>
            <tab label="Default"/>
            <tab label="Tab"/>
            <tab label="Orientation"/>
        </tabs>
        <tabpanels flex="1" >
            <label value="Default" />    
            <label value="Tab " />        
            <label value="Orientation" />
        </tabpanels>
    </tabbox>
</groupbox>
</window>
)";


const char* xul_test = u8R"(<?xml version="1.0"?>
<?xml-stylesheet href="chrome://global/skin" type="text/css"?>
<window title="XUL Layout"
        xmlns:html="http://www.w3.org/1999/xhtml"
        xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul">
<groupbox flex="1">
    <caption label="flexiness" />
    <hbox>
        <button label="+" id="button+"/>
        <splitter/>
        <button label="-" id="button-"/>
        <splitter/>
    </hbox>
    <progressmeter id="progressmeter" mode="determined" value="10" />
</groupbox>
</window>
)";


const char* xul_layout = u8R"(<?xml version="1.0"?>
<?xml-stylesheet href="chrome://global/skin" type="text/css"?>

<window title="XUL Layout"
        xmlns:html="http://www.w3.org/1999/xhtml"
        xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul">
   
    <script type="application/x-javascript"
        src="chrome://global/content/dialogOverlay.js" />
  
<!--description><html:h1>XUL Box Layout</html:h1></description-->

<vbox flex="1" style="overflow: auto">

 <groupbox>
  <caption label="flexiness" />
  <hbox>
    <button label="No Flex" />
    <button label="No Flex" />
    <button label="No Flex" />
    <button label="No Flex" />
    <button label="No Flex" />
  </hbox>

  <hbox>
    <button label="There's a" />
    <button label="spacer" />
    <spacer flex="1" />
    <button label="in the" />
    <button label="middle." />
  </hbox>

  <hbox>
    <button label="This spacer ->" />
    <spacer flex="2" />
    <button label="is bigger than this spacer, ->" />
    <spacer flex="1" />
    <button label="so there!" />
  </hbox>

  <hbox>
    <button label="No Flex" />
    <button label="No Flex" />
    <button flex="1" label="flex=1" />
    <button label="No Flex" />
    <button label="No Flex" />
  </hbox>

  <hbox>
    <button flex="1" label="flex=1" />
    <button flex="2" label="flex=2" />
    <button flex="3" label="flex=3" />
    <button flex="4" label="flex=4" />
    <button flex="5" label="flex=5" />
  </hbox>

  <hbox>
    <button flex="1" label="flex=1" />
    <button flex="1" label="flex=1" />
    <button flex="1" label="flex=1" />
    <button flex="1" label="flex=1" />
    <button flex="1" label="flex=1" />
  </hbox>

 </groupbox>


 <groupbox>
  <caption label="direction" />
  <hbox dir="forward">
    <button label="Here" />
    <button label="the" />
    <button label="direction" />
    <button label="is" />
    <button label="forward" />
  </hbox>
  <hbox dir="reverse">
    <button label="Here" />
    <button label="the" />
    <button label="direction" />
    <button label="is" />
    <button label="reverse" />
  </hbox>
  <hbox>
    <button label="Here the"        ordinal="4" />
    <button label="ordinal"         ordinal="1" />
    <button label="attribute"       ordinal="3" />
    <button label="sets the order." ordinal="2"  />
  </hbox>
 </groupbox>

 <groupbox>
  <caption label="packing" />
  <hbox pack="start">
    <button label="Here" />
    <button label="the" />
    <button label="packing" />
    <button label="is" />
    <button label="start" />
  </hbox>
  <hbox pack="center">
    <button label="Here" />
    <button label="the" />
    <button label="packing" />
    <button label="is" />
    <button label="center" />
  </hbox>
  <hbox pack="end">
    <button label="Here" />
    <button label="the" />
    <button label="packing" />
    <button label="is" />
    <button label="end" />
  </hbox>
  <hbox pack="start">
    <button label="Here" />
    <button label="packing" />
    <button label="yields" />
    <button label="to" />
    <button label="flex" flex="1" />
  </hbox>
 </groupbox>

 <groupbox>
  <caption label="alignment" />
  <hbox align="start">
    <button label="Here" image="images/betty_boop.xbm" orient="vertical" />
    <button orient="vertical">
       <label value="the" />
       <label value="alignment" />
    </button>
    <button label="is" image="images/betty_boop.xbm" />
    <button label="start" />
  </hbox>
  <hbox align="center">
    <button label="Here" image="images/betty_boop.xbm" orient="vertical" />
    <button orient="vertical">
       <label value="the" />
       <label value="alignment" />
    </button>
    <button label="is"  image="images/betty_boop.xbm" />
    <button label="center" />
  </hbox>
  <hbox align="end">
    <button label="Here" image="images/betty_boop.xbm" orient="vertical" />
    <button orient="vertical">
       <label value="the" />
       <label value="alignment" />
    </button>
    <button label="is" image="images/betty_boop.xbm" />
    <button label="end" />
  </hbox>
  <hbox align="baseline">
    <button label="Here" image="images/betty_boop.xbm" orient="vertical" />
    <button orient="vertical">
       <label value="the" />
       <label value="alignment" />
    </button>
    <button label="is" image="images/betty_boop.xbm" />
    <button label="baseline" />
  </hbox>
  <hbox align="stretch">
    <button label="Here" image="images/betty_boop.xbm" orient="vertical" />
    <button orient="vertical">
       <label value="the" />
       <label value="alignment" />
    </button>
    <button label="is" image="images/betty_boop.xbm" />
    <button label="stretch" />
  </hbox>
 </groupbox>

 <groupbox orient="horizontal">
  <caption label="equality" />
  <vbox equalsize="always">
    <button label="Here" image="images/betty_boop.xbm" orient="vertical" />
    <button orient="vertical">
       <label value="the" />
       <label value="equalsize" />
       <label value="attribute" />
    </button>
    <button label="is" image="images/betty_boop.xbm" />
    <button label="always" />
  </vbox>
  <vbox equalsize="never">
    <button label="Here" image="images/betty_boop.xbm" orient="vertical" />
    <button orient="vertical">
       <label value="the" />
       <label value="equalsize" />
       <label value="attribute" />
    </button>
    <button label="is" image="images/betty_boop.xbm" />
    <button label="never" />
  </vbox>
 </groupbox>


 <groupbox>
  <caption label="hiddeness" />
  <hbox>
    <label value="Every other button in the line below is hidden" />
  </hbox>
  <hbox>
    <button label="Every" />
    <button label="other" hidden="true" />
    <button label="button" />
    <button label="in" hidden="true" />
    <button label="the" />
    <button label="line" hidden="true" />
    <button label="below" />
    <button label="is" hidden="true" />
    <button label="hidden" />
  </hbox>
 </groupbox>

</vbox>

</window>



)";






const char* xul_popup = u8R"(<?xml version="1.0"?>
<?xml-stylesheet href="chrome://global/skin" type="text/css"?>

<window title="XUL Popups"
        xmlns:html="http://www.w3.org/1999/xhtml"
        xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul">

<script>
<![CDATA[

]]>
</script>

<!--description><html:h1>XUL Popups</html:h1></description!-->

  <vbox flex="1" style="overflow: auto">

   <hbox>
    <groupbox flex="1" orient="horizontal">
    <caption label="menulists" />
    <groupbox flex="1">
    <caption label="states" />
    <menulist label="Selected">
     <menupopup>
      <menuitem label="Normal"/>
      <menuitem label="Selected" selected="true" />
      <menuitem label="Disabled" disabled="true" />
      <menuitem label="Checkbox" type="checkbox" />
      <menuitem label="Checked"  type="checkbox" checked="true" />
     </menupopup>
    </menulist>
    </groupbox>

    <groupbox flex="1">
    <caption label="non-editable" />
    <menulist label="Pearl">
     <menupopup>
      <menuitem label="Pearl" selected="true"  />
      <menuitem label="Aramis"  />
      <menuitem label="Yakima"  />
      <menuitem label="Tribble" />
      <menuitem label="Cosmo"   />
     </menupopup>
    </menulist>
    </groupbox>

    <groupbox flex="1">
    <caption label="editable" />
    <menulist label="Gray" editable="true">
     <menupopup>
      <menuitem label="Gray" selected="true"  />
      <menuitem label="Black"  />
      <menuitem label="Holstein"  />
      <menuitem label="Orange"  />
      <menuitem label="White"   />
     </menupopup>
    </menulist>
    </groupbox>
    </groupbox>
   </hbox>

   <groupbox orient="horizontal">
   <caption label="Regular Popups, Context Popups, and Tooltips" />
     <button flex="1" label="Press Normal Button"  popup="popupmenu" />
     <button flex="1" label="Press Context Button" context="contextpopup" />
     <button flex="1" label="Hover Mouse Here" 
                      tooltiptext="This is a tooltip" />
     <button flex="1" label="Three-in-one"  popup="popupmenu" 
                                            context="contextpopup" 
                                            tooltiptext="This is a tooltip" />
   </groupbox>

<groupbox>
<caption label="positions for popups" />
<!--grid>
  <columns>
    <column flex="1" />
    <column flex="1" />
    <column flex="1" />
    <column flex="1" />
  </columns>
  <rows>
    <row>
       <button label="Start Before" popup="start_before" />
       <button label="Before Start" popup="before_start" />
       <button label="Before End" popup="before_end" />
       <button label="End Before" popup="end_before" />
    </row>
    <row>
       <button label="Start After"  popup="start_after" />
       <button label="After Start" popup="after_start" />
       <button label="After End" popup="after_end" />
       <button label="End After" popup="end_after" />
    </row>
    <row>
       <button label="Overlap"  popup="overlap" />
       <button label="At Pointer" popup="at_pointer" />
       <button label="After Pointer" popup="after_pointer" />
    </row>
  </rows>
</grid-->
</groupbox>

</vbox>

</window>
)";


const char* xul_listbox = u8R"(<?xml version="1.0"?>
<?xml-stylesheet href="chrome://global/skin" type="text/css"?>
<window title="XUL List Box"
        xmlns:html="http://www.w3.org/1999/xhtml"
        xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul">
<groupbox flex="1">
    <caption label="with single selection" />
    <listbox flex="1" seltype="single">
        <listcols>
           <listcol flex="1"/>
           <listcol flex="2"/>
           <listcol flex="1"/>
        </listcols>
        <listhead>
            <listheader label="A"/>
            <listheader label="A"/>
        </listhead>
        <listitem>
            <label value="Aramis"/>
            <label value="Aramis - Beta"/>
        </listitem>
        <listitem label="Yakima"  />
        <listitem label="Tribble" />
        <listitem label="Cosmo"   />
    </listbox>
</groupbox>
</window>
)";

const char* xul_tree = u8R"(<?xml version="1.0"?>
<?xml-stylesheet href="chrome://global/skin" type="text/css"?>
<window title="XUL Tree Box"
        xmlns:html="http://www.w3.org/1999/xhtml"
        xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul">
<groupbox flex="1">
    <caption label="tree" />
    <tree id="tree3" flex="1" hidecolumnpicker="true" enableColumnDrag="true" >
        <treecols>
            <treecol id="name3"  flex="1" label="Name" />
            <!--splitter class="tree-splitter" /-->
            <treecol id="sex3"   flex="1" label="Sex"  />
            <!--splitter class="tree-splitter" /-->
            <treecol id="color3" flex="1" label="Color" />
            <!--splitter class="tree-splitter" /-->
            <treecol id="attr3"  flex="1" label="Attribute" />
        </treecols>

        <treechildren>
            <treeitem>
                <treerow>
                    <treecell label="Feathers" />
                    <treecell label="Male" />
                    <treecell label="Tabby" />
                    <treecell label="Patiarchal" />
                </treerow>
            </treeitem>
            <treeitem>
                <treerow>
                    <treecell label="Butter" />
                    <treecell label="Male" />
                    <treecell label="Orange" />
                    <treecell label="Old" />
                </treerow>
            </treeitem>
        </treechildren>
    </tree>
</groupbox>
</window>
)";

const char* xul_tree2 = u8R"(<?xml version="1.0"?>
<?xml-stylesheet href="chrome://global/skin" type="text/css"?>
<window title="XUL Tree Box"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul">
    <groupbox flex="1">
        <caption label="tree" />
        <tree id="tree2" flex="1" hidecolumnpicker="true" >
            <treecols>
                <treecol id="sex2"   flex="1" label="Cats" primary="true" />
            </treecols>
            <treechildren>
                <treeitem container="true" open="true">
                    <treerow>
                        <treecell label="Female" />
                    </treerow>
                    <treechildren>
                        <treeitem container="true" open="true">
                            <treerow>
                                <treecell label="Gray" />
                            </treerow>
                            <treechildren>
                                <treeitem>
                                    <treerow>
                                        <treecell label="Pearl" />
                                    </treerow>
                                </treeitem>
                            </treechildren>
                        </treeitem>
                        <treeitem container="true" open="true">
                            <treerow>
                                <treecell label="White" />
                            </treerow>
                            <treechildren>
                                <treeitem>
                                    <treerow>
                                        <treecell label="Cosmo" />
                                    </treerow>
                                </treeitem>
                            </treechildren>
                        </treeitem>
                        <treeitem container="true" open="true">
                            <treerow>
                                <treecell label="Orange" />
                            </treerow>
                            <treechildren>
                                <treeitem>
                                    <treerow>
                                        <treecell label="Tribble" />
                                    </treerow>
                                </treeitem>
                            </treechildren>
                        </treeitem>
                    </treechildren>
                </treeitem>
                <treeitem  container="true" open="true">
                    <treerow>
                        <treecell label="Male" />
                    </treerow>
                    <treechildren>
                        <treeitem container="true" open="true">
                            <treerow>
                                <treecell label="Black" />
                            </treerow>
                            <treechildren>
                                <treeitem>
                                    <treerow>
                                        <treecell label="Aramis" />
                                    </treerow>
                                </treeitem>
                                <treeitem>
                                    <treerow>
                                        <treecell label="Fergus" />
                                    </treerow>
                                </treeitem>
                                <treeitem>
                                    <treerow>
                                        <treecell label="Clint" />
                                    </treerow>
                                </treeitem>
                            </treechildren>
                        </treeitem>
                        <treeitem container="true" open="true">
                            <treerow>
                                <treecell label="Orange" />
                            </treerow>
                            <treechildren>
                                <treeitem>
                                    <treerow>
                                        <treecell label="Butter" />
                                    </treerow>
                                </treeitem>
                                <treeitem>
                                    <treerow>
                                        <treecell label="Zippy" />
                                    </treerow>
                                </treeitem>
                            </treechildren>
                        </treeitem>
                        <treeitem container="true" open="true">
                            <treerow>
                                <treecell label="Tabby" />
                            </treerow>
                            <treechildren>
                                <treeitem>
                                    <treerow>
                                        <treecell label="Feathers" />
                                    </treerow>
                                </treeitem>
                            </treechildren>
                        </treeitem>
                        <treeitem container="true" open="true">
                            <treerow>
                                <treecell label="Holstein" />
                            </treerow>
                            <treechildren>
                                <treeitem>
                                    <treerow>
                                        <treecell label="Yakima" />
                                    </treerow>
                                </treeitem>
                            </treechildren>
                        </treeitem>
                    </treechildren>
                </treeitem>
            </treechildren>
        </tree>
    </groupbox>
</window>
)";