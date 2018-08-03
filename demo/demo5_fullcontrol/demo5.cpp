#include <core/ui_manager.h>
#include <control/ui_viewport.h>

void InitViewportCallback(LongUI::UIViewport& v) noexcept;

int main() {
    if (UIManager.Initialize()) {
        {
            LongUI::UIViewport viewport;
            if (viewport.SetXulFromFile([]()noexcept {
                using namespace LongUI;
                return u8"xul/main.xul"_sv;
            }())) ::InitViewportCallback(viewport);

            viewport.RefWindow().ShowWindow();
            viewport.RefWindow().Exec();
        }
        UIManager.Uninitialize();
    }
    return 0;
}

