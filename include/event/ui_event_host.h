#pragma once

#include "ui_gui_event.h"
#include "../core/ui_core_type.h"
#include "../util/ui_function.h"
#include "../core/ui_string_view.h"
#include <utility>

namespace LongUI{
    /// <summary>
    /// gui event host
    /// </summary>
    class CUIEventHost {
        // function node
        struct FunctionNode;
    public:
        // add gui event listener with ownid
        template<typename E, typename Callable>
        inline void AddGuiEventListener(E e, uintptr_t ownid, Callable call) {
            static_assert(
                std::is_same<E, const char*>::value ||
                std::is_same<E, U8View>::value ||
                std::is_same<E, GuiEvent>::value,
                "e must be 'const char*' or 'GuiEvent'"
                );
            GuiEventListener listener{ call, ownid };
            this->add_gui_event_listener(e, std::move(listener));
        }
        // add gui event listener without ownid(ownid = 0)
        template<typename Callable>
        inline void AddGuiEventListener(GuiEvent e, Callable call) {
            return AddGuiEventListener(e, 0, call);
        }
        // add gui event listener without ownid(ownid = 0)
        template<typename Callable>
        inline void AddGuiEventListener(const char* e, Callable call) {
            return AddGuiEventListener(e, 0, call);
        }
        // remove gui event listener with enum-name-event
        void RemoveGuiEventListener(uintptr_t ownid, GuiEvent) noexcept;
        // remove gui event listener with string-name-event
        void RemoveGuiEventListener(uintptr_t ownid, U8View view) noexcept {
            RemoveGuiEventListener(ownid, strtoe(view));
        }
        // remove gui event listener with string-name-event
        void RemoveGuiEventListener(uintptr_t ownid, const char* str) noexcept {
            RemoveGuiEventListener(ownid, strtoe(U8View::FromCStyle(str)));
        }
        // trigger event
        auto TriggrtEvent(GuiEvent event) noexcept ->EventAccept;
    private:
        // string to event
        static auto strtoe(U8View) noexcept->GuiEvent;
        // find event
        auto find_event(GuiEvent) const noexcept->FunctionNode*;
        // require event
        auto require_event(GuiEvent) noexcept->FunctionNode*;
        // add gui event listener
        void add_gui_event_listener(GuiEvent, GuiEventListener&&) noexcept;
        // add gui event listener
        void add_gui_event_listener(const char* str, GuiEventListener&& l) noexcept {
            add_gui_event_listener(U8View::FromCStyle(str), std::move(l));
        }
        // add gui event listener
        void add_gui_event_listener(U8View view, GuiEventListener&& l) noexcept {
            add_gui_event_listener(view, std::move(l));
        }
    protected:
        // ctor
        CUIEventHost() noexcept {}
        // dtor
        virtual ~CUIEventHost() noexcept;
    private:
        // head function node
        FunctionNode*           m_pFuncNode = nullptr;
    };
}