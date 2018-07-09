#include <core/ui_manager.h>
#include <control/ui_label.h>
#include <control/ui_viewport.h>

const auto xul = u8R"xml(
<?xml version="1.0"?>
<window clearcolor="white" title="lui demo">
  <spacer flex="1"/>
  <hbox>
    <spacer flex="1"/>
    <label value="hellow world!"/>
    <spacer flex="1"/>
  </hbox>
  <spacer flex="1"/>
</window>
)xml";

int main() {
    if (UIManager.Initialize()) {
        {
            LongUI::UIViewport viewport;
            viewport.SetXul(xul);
            viewport.GetWindow()->ShowWindow();
            UIManager.MainLoop();
        }
        UIManager.Uninitialize();
    }
    return 0;
}