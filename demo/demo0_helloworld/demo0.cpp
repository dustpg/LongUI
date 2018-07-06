#include <core/ui_manager.h>
#include <control/ui_label.h>
#include <control/ui_viewport.h>

int main() {
    if (UIManager.Initialize()) {
        {
            LongUI::UIViewport viewport;
            LongUI::UILabel label{ &viewport };
            label.SetText(L"Hello world!");
            viewport.GetWindow()->ShowWindow();
            UIManager.MainLoop();
        }
        UIManager.Uninitialize();
    }
    return 0;
}
