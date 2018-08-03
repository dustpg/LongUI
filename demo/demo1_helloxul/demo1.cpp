#include <core/ui_manager.h>
#include <control/ui_label.h>
#include <control/ui_viewport.h>

const auto xul = u8R"xml(
<?xml version="1.0"?>
<window clearcolor="white" title="lui demo">
  <spacer flex="1"/>
  <hbox>
    <spacer flex="1"/>
    <label class="fookin-comic-sans" value="hellow world!"/>
    <spacer flex="1"/>
  </hbox>
  <spacer flex="1"/>
</window>
)xml";

const auto global_css = u8R"css(
spacer ~ .fookin-comic-sans {
  font-family: "Comic Sans MS"
}
)css";

int main() {
    if (UIManager.Initialize()) {
#ifndef LUI_DISABLE_STYLE_SUPPORT
        using LongUI::U8View;
        UIManager.AddGlobalCssString(U8View::FromCStyle(global_css));
#endif
        {
            LongUI::UIViewport viewport;
            viewport.SetXul(xul);

            viewport.GetWindow()->ShowWindow();
            viewport.GetWindow()->Exec();
        }
        UIManager.Uninitialize();
    }
    return 0;
}