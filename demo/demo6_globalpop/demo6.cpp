#include <core/ui_manager.h>
#include <control/ui_label.h>
#include <control/ui_textbox.h>
#include <control/ui_viewport.h>
#include <control/ui_menuitem.h>
#include <cassert>

const auto xul = u8R"xml(
<?xml version="1.0"?>
<window lui:clearcolor="white" title="lui demo">
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

const auto text_context = u8R"xml(
<?xml version="1.0"?>
<window>
    <popup id="global_context">
        <menuitem label="撤销" id="undo" acceltext="Ctrl+Z"/>
        <menuseparator/>
        <menuitem label="Copy" id="copy" acceltext="Ctrl+C"/>
        <menuitem label="切り取り" id="cut" acceltext="Ctrl+X"/>
        <menuitem label="Paste" id="paste" acceltext="Ctrl+V"/>
        <menuseparator/>
        <menuitem label="全选" id="selall" acceltext="Ctrl+A"/>
        <menu label="More" >
            <menupopup>
            <menuitem label="A" />
            <menuitem label="B" />
            <menuitem label="C" />
            <menuseparator />
            <menu label="More" >
                <menupopup>
                    <menuitem label="1" />
                    <menuitem label="2" />
                    <menuitem label="3" />
                </menupopup>
            </menu>
            </menupopup>
        </menu>
    </popup>
</window>
)xml";

class CTextboxContextMenu {
    LongUI::UIMenuItem*     m_pUndo;
    LongUI::UIMenuItem*     m_pCopy;
    LongUI::UIMenuItem*     m_pCut;
    LongUI::UIMenuItem*     m_pPaste;
    LongUI::UIMenuItem*     m_pSelAll;
public:
    LongUI::UIViewport&     viewport;
    CTextboxContextMenu(LongUI::UIViewport&) noexcept;
    ~CTextboxContextMenu() noexcept = default;
    void DoBeforePopup(LongUI::UITextBox&) noexcept;
    template<typename T>
    static auto EHelper(T call, LongUI::UIViewport& view) noexcept {
        using LongUI::uisafe_cast;
        if (const auto tbox = uisafe_cast<LongUI::UITextBox>(view.GetHoster())) {
            return static_cast<LongUI::EventAccept>(call(*tbox));
        }
        return LongUI::Event_Ignore;
    }
};

class CDemo6Viewport : public LongUI::UIViewport {
    using Super = LongUI::UIViewport;
    CTextboxContextMenu&        m_refTCMenu;
public:
    void SubViewportPopupBegin(UIViewport&, LongUI::PopupType) noexcept override;
public:
    CDemo6Viewport(CTextboxContextMenu& m) noexcept : m_refTCMenu(m) {}
    ~CDemo6Viewport() noexcept override {};
};

LongUI::UIViewport* init_global_pupop() noexcept;


int main() noexcept {
    if (UIManager.Initialize()) {
        const auto gpop = ::init_global_pupop();
        assert(gpop && "bad init");
        CTextboxContextMenu menu(*gpop);
        CDemo6Viewport viewport(menu);
        viewport.SetXul(xul);

        viewport.GetWindow()->ShowWindow();
        viewport.GetWindow()->Exec();

    }
    UIManager.Uninitialize();
    return 0;
}

#include <control/ui_popup.h>


LongUI::UIViewport* init_global_pupop() noexcept {
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



CTextboxContextMenu::CTextboxContextMenu(LongUI::UIViewport& v) noexcept :
    viewport(v)
{
    const auto find_item = [&v](const char* id) noexcept {
        auto& wnd = v.RefWindow();
        const auto ctrl = wnd.FindControl(id);
        using LongUI::longui_cast;
        return longui_cast<LongUI::UIMenuItem*>(ctrl);
    };
    m_pUndo = find_item("undo");
    m_pCopy = find_item("copy");
    m_pCut = find_item("cut");
    m_pPaste = find_item("paste");
    m_pSelAll = find_item("selall");
    assert(m_pUndo && m_pCopy && m_pCut && m_pPaste && m_pSelAll);
    // on - undo
    m_pUndo->AddGuiEventListener(m_pUndo->_onCommand(), [&v](auto& c) noexcept {
        return EHelper([](LongUI::UITextBox& tbox) noexcept {
            return tbox.GuiUndo();
        }, v);
    });
    // on - copy
    m_pCopy->AddGuiEventListener(m_pUndo->_onCommand(), [&v](auto& c) noexcept {
        return EHelper([](LongUI::UITextBox& tbox) noexcept {
            return tbox.GuiCopyCut(false);
        }, v);
    });
    // on - cut
    m_pCut->AddGuiEventListener(m_pUndo->_onCommand(), [&v](auto& c) noexcept {
        return EHelper([](LongUI::UITextBox& tbox) noexcept {
            return tbox.GuiCopyCut(true);
        }, v);
    });
    // on - paste
    m_pPaste->AddGuiEventListener(m_pUndo->_onCommand(), [&v](auto& c) noexcept {
        return EHelper([](LongUI::UITextBox& tbox) noexcept {
            return tbox.GuiPaste();
        }, v);
    });
    // on - select all
    m_pSelAll->AddGuiEventListener(m_pUndo->_onCommand(), [&v](auto& c) noexcept {
        return EHelper([](LongUI::UITextBox& tbox) noexcept {
            return tbox.GuiSelectAll();
        }, v);
    });
}

void CDemo6Viewport::SubViewportPopupBegin(UIViewport& view, LongUI::PopupType pop) noexcept {
    // if popup is global textbox context menu
    if (view == m_refTCMenu.viewport) {
        // if hoster is Textbox
        using LongUI::uisafe_cast;
        if (const auto tbox = uisafe_cast<LongUI::UITextBox>(view.GetHoster()))
            // before popup, set some item to enable/disabled depend on textbox
            m_refTCMenu.DoBeforePopup(*tbox);
    }
    Super::SubViewportPopupBegin(view, pop);
}


void CTextboxContextMenu::DoBeforePopup(LongUI::UITextBox& tbox) noexcept {
    m_pCopy->SetEnabled(tbox.CanCopy());
    m_pCut->SetEnabled(tbox.CanCut());
}