#include <core/ui_manager.h>
#include <control/ui_label.h>
#include <control/ui_viewport.h>

#include <interface/ui_default_config.h>
#include <interface/ui_ctrlinfolist.h>
#include "mytoggle.h"
#include "mytoggleex.h"

const auto xul = u8R"xml(
<?xml version="1.0"?>
<window lui:clearcolor="white" title="lui demo">
  <spacer flex="1"/>
  <hbox>
    <spacer flex="1"/>
    <label value="C++ Toggle Switch A:"/>
    <mytoggle base="red" time="0.233"/>
    <spacer flex="1"/>
  </hbox>
  <hbox>
    <spacer flex="1"/>
    <label value="C++ Toggle Switch B:"/>
    <mytoggleex base="blue" time="0.233"/>
    <spacer flex="1"/>
  </hbox>
  <spacer flex="1"/>
</window>
)xml";

namespace Demo { struct Config final : LongUI::CUIDefaultConfigure {
    // register control
    void RegisterControl(LongUI::ControlInfoList& list) noexcept override {
        static_assert(2 < MAX_CONTROL_TYPE_COUNT, "too small");
        // MyToggle
        list.Push(&Demo::MyToggle::s_meta);
        // MyToggleEx
        list.Push(&Demo::MyToggleEx::s_meta);
    }
};}

int main() {
    int code = -1;
    Demo::Config config;
    if (UIManager.Initialize(&config)) {
        LongUI::UIViewport viewport;
        viewport.SetXul(xul);

        viewport.GetWindow()->ShowWindow();
        code = LongUI::IntCode(viewport.GetWindow()->Exec());
    }
    UIManager.Uninitialize();
    return code;
}