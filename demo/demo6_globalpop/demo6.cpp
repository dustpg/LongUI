#include <core/ui_manager.h>
#include <control/ui_label.h>
#include <control/ui_viewport.h>

const auto xul = u8R"xml(
<?xml version="1.0"?>
<window clearcolor="white" title="lui demo">
    <popup id="popupmenu">
        <menuitem label="Pearl"   />
        <menuitem label="Aramis"  />
        <menuitem label="Yakima"  />
    </popup>
    <spacer flex="1"/>
    <hbox align="center">
        <label value="TextBoxA:"/>
        <textbox id="txt1" context="global_context" />
    </hbox>
    <hbox align="center">
        <label value="TextBoxB:"/>
        <textbox id="txt2" context="global_context" />
    </hbox>
    <hbox align="center">
        <label value="TextBoxC:"/>
        <textbox id="txt3" context="popupmenu" />
    </hbox>
  <spacer flex="1"/>
</window>
)xml";

LongUI::UIViewport* init_global_pupop() noexcept;


int main() noexcept {
    if (UIManager.Initialize()) {
        const auto gpop = ::init_global_pupop();
        LongUI::UIViewport viewport;
        viewport.SetXul(xul);

        viewport.GetWindow()->ShowWindow();
        viewport.GetWindow()->Exec();
    }
    UIManager.Uninitialize();
    return 0;
}

#include <control/ui_popup.h>


LongUI::UIViewport* init_global_pupop() noexcept {
    const auto text_context = u8R"xml(
<?xml version="1.0"?>
<window>
    <popup id="global_context">
        <menuitem label="撤销 - Undo"/>
        <menuseparator/>
        <menuitem label="复制 - Copy"/>
        <menuitem label="剪切 - Cut"/>
        <menuitem label="粘贴 - Paste"/>
        <menuseparator/>
        <menuitem label="全选 - Select All"/>
    </popup>
</window>
)xml";
    const auto viewport = [=]() noexcept{
        LongUI::UIViewport view;
        view.SetXul(text_context);
        const auto a = LongUI::U8View::FromCStyle("global_context");
        const auto viewport = view.FindSubViewport(a);
        assert(viewport && "subview not found");
        UIManager.MoveToGlobalSubView(*viewport);
        return viewport;
    }();
    return viewport;
}