#include <core/ui_manager.h>
#include <control/ui_viewport.h>

void InitViewportCallback(LongUI::UIViewport& v) noexcept;
void InitStyleSheet() noexcept;

int main() {
    if (UIManager.Initialize()) {
        ::InitStyleSheet();
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

static const char s_test[] = R"css(
textbox {
    font-family: KaiTi
}
)css";

void InitStyleSheet() noexcept {
    //UIManager.AddGlobalCssString(LongUI::U8View::FromCStyle(s_test));
}