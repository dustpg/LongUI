#include <control/ui_viewport.h>

#ifndef NDEBUG
#include <control/ui_button.h>
#include <control/ui_checkbox.h>
#include <core/ui_manager.h>
#include <core/ui_window.h>
#include <container/pod_vector.h>


#ifdef LUI_DEBUG_WINDOW_MEMLEAK
#include <crtdbg.h>
#endif


namespace LongUI {
    // debug window view xul
    extern const char* debug_view_xul;
    // debug view
    class CUIDebugView final : public UIViewport {
        // super class
        using Super = UIViewport;
#ifdef LUI_DEBUG_WINDOW_MEMLEAK
        // mem state
        _CrtMemState    m_memstates[3];
        // fa
        bool            m_memtype = false;
#endif
    private:
        // do all render
        template<typename T>
        static void do_all_render(T a, const T b) noexcept {
            while (a != b) {
                a->MarkFullRendering();
                do_all_render(a->begin(), a->end());
                a++;
            }
        }
        // do button
        auto do_button(const char* name) noexcept {
            const auto ctrl = this->RefWindow().FindControl(name);
#ifndef NDEBUG
            if (!ctrl) LUIDebug(Error) << "control not found: " << name << endl;
#endif
            const auto btn = longui_cast<UIButton*>(ctrl);
            return btn;
        }
        // do checkbox
        auto do_checkbox(const char* name) noexcept {
            const auto ctrl = this->RefWindow().FindControl(name);
#ifndef NDEBUG
            if (!ctrl) LUIDebug(Error) << "control not found: " << name << endl;
#endif
            const auto btn = longui_cast<UICheckBox*>(ctrl);
            return btn;
        }
        // do label
        auto do_label(const char* name) noexcept {
            const auto ctrl = this->RefWindow().FindControl(name);
#ifndef NDEBUG
            if (!ctrl) LUIDebug(Error) << "control not found: " << name << endl;
#endif
            const auto btn = longui_cast<UILabel*>(ctrl);
            return btn;
        }
        // force render
        void force_render() noexcept {
            const auto btn = do_button("btn-force");
            if (!btn) return;
            btn->AddGuiEventListener(
                UIButton::_onCommand(), [](const GuiEventArg&) noexcept {
                //const auto& o = UIManager.GetWindowList();
                //using List = POD::Vector<CUIWindow*>;
                //auto& list = reinterpret_cast<const List&>(o);
                //for (const auto x : list) x->MarkFullRendering();
                do_all_render(UIManager.begin(), UIManager.end());
                return Event_Accept;
            });
        }
        // exit
        void exit() noexcept {
            const auto btn = do_button("btn-exit");
            if (!btn) return;
            btn->AddGuiEventListener(
                UIButton::_onCommand(), [](const GuiEventArg&) noexcept {
                UIManager.Exit();
                return Event_Accept;
            });
        }
        // recreate_device
        void recreate() noexcept {
            const auto btn = do_button("btn-recreate");
            if (!btn) return;
            btn->AddGuiEventListener(
                UIButton::_onCommand(), [](const GuiEventArg&) noexcept {
                UIManager.NeedRecreate();
                return Event_Accept;
            });
        }
        // draw dirty rect
        void draw_dirty_rect() noexcept {
            const auto checkbox = do_checkbox("cbx-dirty");
            if (!checkbox) return;
            const auto& flag = UIManager.flag;
            checkbox->SetChecked(!!(flag & ConfigureFlag::Flag_DbgDrawDirtyRect));
            checkbox->AddGuiEventListener(
                checkbox->_onCommand(), [&flag](const GuiEventArg& arg) noexcept {
                const auto box = longui_cast<UICheckBox*>(arg.current);
                auto& mflag = const_cast<ConfigureFlag&>(flag);
                if (box->IsChecked()) mflag = mflag | ConfigureFlag::Flag_DbgDrawDirtyRect;
                else mflag = mflag & ConfigureFlag(~ConfigureFlag::Flag_DbgDrawDirtyRect);
                return Event_Accept;
            });
        }
        // draw text cell
        void draw_text_cell() noexcept {
            const auto checkbox = do_checkbox("cbx-cell");
            if (!checkbox) return;
            const auto& flag = UIManager.flag;
            checkbox->SetChecked(!!(flag & ConfigureFlag::Flag_DbgDrawTextCell));
            checkbox->AddGuiEventListener(
                checkbox->_onCommand(), [&flag](const GuiEventArg& arg) noexcept {
                const auto box = longui_cast<UICheckBox*>(arg.current);
                auto& mflag = const_cast<ConfigureFlag&>(flag);
                if (box->IsChecked()) mflag = mflag | ConfigureFlag::Flag_DbgDrawTextCell;
                else mflag = mflag & ConfigureFlag(~ConfigureFlag::Flag_DbgDrawTextCell);
                return Event_Accept;
            });
        }
        // link style sheet
        void link_style_sheet() noexcept {
            const auto checkbox = do_checkbox("cbx-style");
            if (!checkbox) return;
            const auto& flag = UIManager.flag;
            checkbox->SetChecked(!(flag & ConfigureFlag::Flag_DbgNoLinkStyle));
#ifdef NDEBUG
            checkbox->AddGuiEventListener(
                checkbox->_stateChanged(), [&flag](const GuiEventArg& arg) noexcept {
                const auto box = longui_cast<UICheckBox*>(arg.current);
                auto& mflag = const_cast<CUIManager::ConfigFlag&>(flag);
                if (!box->GetChecked()) mflag = mflag | ConfigureFlag::Flag_DbgNoLinkStyle;
                else mflag = mflag & CUIManager::ConfigFlag(~ConfigureFlag::Flag_DbgNoLinkStyle);
                return Event_Accept;
            });
#endif
        }

    public:
        // ctor
        CUIDebugView() noexcept :
            Super(nullptr, CUIWindow::Config_ToolWindow) {
            this->SetXul(debug_view_xul);
            this->SetAutoOverflow();
            auto& window = this->RefWindow();
            window.SetClearColor({ 1, 1, 1, 1 });
            window.ShowWindow();
            window.SetPos({ 0, 0 });
            this->exit();
            this->recreate();
            this->force_render();
            this->draw_text_cell();
            this->draw_dirty_rect();
            this->link_style_sheet();
#ifdef LUI_DEBUG_WINDOW_MEMLEAK
            {
                const auto btn = do_button("btn-mem");
                const auto cbx = do_checkbox("cbx-mem");
                const auto lbl = do_label("label-mem");
                if (btn && cbx && lbl) {
                    std::memset(m_memstates, 0, sizeof(m_memstates));
                    btn->AddGuiEventListener(
                        UIButton::_onCommand(), [=](const GuiEventArg& arg) noexcept {
                        m_memtype = !m_memtype;
                        if (m_memtype) {
                            ::_CrtMemCheckpoint(m_memstates + 0);
                            lbl->SetText(u"press again!");
                        }
                        else {
                            ::_CrtMemCheckpoint(m_memstates + 1);
                            if (::_CrtMemDifference(m_memstates + 2, m_memstates + 0, m_memstates + 1)) {
                                ::_CrtMemDumpStatistics(m_memstates + 2);
                                LUIDebug(Warning) << "Memory leak detected" << endl;
                                if (cbx->IsChecked()) {
                                    assert(!"Check Memory leak detected");
                                }
                            }
                            std::memset(m_memstates, 0, sizeof(m_memstates));
                            lbl->SetText(u"--finished--");
                        }
                        return Event_Accept;
                    });
                }
            }

#else
            if (const auto box = this->RefWindow().FindControl("hbox-mem")) {
                box->SetVisible(false);
            }
#endif
            //UIManager.CreateTimeCapsule([this](float t) noexcept {
            //    if (t < 1.f) return;
            //    const auto window = this->GetWindow();
            //    const auto ctrl = window->FindControl("btn-exit");
            //    const auto btn = longui_cast<UIButton*>(ctrl);
            //    btn->SetText(L"EXIT"_sv);
            //}, 5.f, this);
        }
        // dtor
        ~CUIDebugView() noexcept { }
    };
}

// LongUI::impl namespace
namespace LongUI { namespace impl {
    // create the debug window
    auto create_debug_window() noexcept -> CUIWindow * {
        const auto ptr = new(std::nothrow) CUIDebugView;
        return ptr ? ptr->GetWindow() : nullptr;
    }
}}


// xul stirng
const char* LongUI::debug_view_xul = u8R"(
<?xml version="1.0"?>
<window title="XUL Layout">
<groupbox flex="1">
    <caption label="Core Debug"/>
    <label href="https://github.com/dustpg/LongUI" value="LongUI on github"/>
    <hbox>
        <button id="btn-force" label="force render" accesskey="f"/>
        <button id="btn-recreate" label="recreate res" accesskey="r"/>
        <button id="btn-exit" label="exit" default="true" accesskey="e"/>
    </hbox>
    <hbox id="hbox-mem" align="center">
        <checkbox id="cbx-mem" label="assert"/>
        <button id="btn-mem" label="check" accesskey="c"/>
        <label id="label-mem"/>
    </hbox>
    <checkbox id="cbx-dirty" label="draw dirty rect"/>
    <checkbox id="cbx-cell" label="draw text cell"/>
    <checkbox id="cbx-style" label="link style sheet"/>
</groupbox>
</window>)";


#endif