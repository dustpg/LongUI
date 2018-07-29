#include <core/ui_manager.h>
#include <control/ui_label.h>
#include <control/ui_button.h>
#include <event/ui_gui_event.h>
#include <control/ui_viewport.h>

#include <control/ui_button.h>
#include <control/ui_menulist.h>
#include <control/ui_menupopup.h>

#include "customconfig.h"


void InitList(
    const LongUI::POD::Vector<LongUI::GraphicsAdapterDesc>& adapters,
    LongUI::UIControl* list
) noexcept;

void InitButton(
    LongUI::UIControl* list,
    LongUI::UIControl* button, 
    Demo::CustomConfig& cfg
) noexcept;


int main() {
    Demo::CustomConfig config;
    if (UIManager.Initialize(&config)) {
        LUIDebug(Hint) << "Battle Control Online..." << LongUI::endl;
        {
            LongUI::UIViewport viewport;
            using namespace LongUI;
            // LongUI::operator""_sv to create UxxView
            if (viewport.SetXulFromFile(u8"luidata.zip/main.xul"_sv)) {
                // menupopup is popupwindow, cannot be found througn FindControl
                const auto list = viewport.RefWindow().FindControl("list");
                const auto button = viewport.RefWindow().FindControl("button");
                ::InitButton(list, button, config);
                ::InitList(config.RefAdapters(), list);
            }
            viewport.GetWindow()->ShowWindow();
            UIManager.MainLoop();
        }
        LUIDebug(Hint) << "Battle Control Terminated." << LongUI::endl;
        UIManager.Uninitialize();
    }
    return 0;
}

/// <summary>
/// Initializes the list.
/// </summary>
/// <param name="adapters">The adapters.</param>
/// <param name="list">The list.</param>
/// <returns></returns>
void InitList(
    const LongUI::POD::Vector<LongUI::GraphicsAdapterDesc>& adapters, 
    LongUI::UIControl* list) noexcept {
    const auto menu = longui_cast<LongUI::UIMenuList*>(list)->GetPopupObj();
    assert(menu && "BAD ACTION");
    for (const auto& adapter : adapters) {
        menu->AddItem(adapter.friend_name);
    }
}

/// <summary>
/// Initializes the button.
/// </summary>
/// <param name="button">The button.</param>
/// <param name="cfg">The CFG.</param>
/// <returns></returns>
void InitButton(
    LongUI::UIControl* list,
    LongUI::UIControl* button,
    Demo::CustomConfig& cfg) noexcept {
    const auto btn = longui_cast<LongUI::UIButton*>(button);
    const auto menu = longui_cast<LongUI::UIMenuList*>(list);
    assert(btn && menu && "BAD ACTION");
    btn->AddGuiEventListener(LongUI::UIButton::_clicked(), 
        [&cfg, menu](LongUI::UIControl&) noexcept {
        const auto index = menu->GetSelectionIndex();
        // index < 0 -> not selected
        if (index < 0) return LongUI::Event_Ignore;
        // index >=0 -> selected
        cfg.SetAdapterIndex(static_cast<uint32_t>(index));
        UIManager.NeedRecreate();
        return LongUI::Event_Accept;
    });
}