#define WIN32_LEAN_AND_MEAN
#include <core/ui_manager.h>
#include <core/ui_color_list.h>
#include <control/ui_viewport.h>
#include <control/ui_label.h>
#include <Windows.h>

extern "C" int CALLBACK WinMain(HINSTANCE, HINSTANCE, char*, int) {
    if (UIManager.Initialize()) {
        {
            UI::UIViewport viewport1;
            const auto l = new(std::nothrow) UI::UILabel{ &viewport1 };
            l->SetText(L"hello, world!");
            viewport1.RefWindow().ShowWindow();
            viewport1.RefWindow().SetTitleName(L"LongUI Demo");
            viewport1.SetBgColor({ UI::RGBA_White });
            UIManager.MainLoop();
        }
        UIManager.Uninitialize();
    }
    return 0;
}

#pragma comment(lib, "CodeUI")
