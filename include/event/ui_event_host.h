#pragma once
#pragma interface

#include "ui_gui_event.h"
#include "../core/ui_core_type.h"
#include "../util/ui_function.h"
#include "../core/ui_string_view.h"
#include "../style/ui_style_value.h"
#include <utility>

namespace LongUI {
    /// <summary>
    /// gui event host
    /// </summary>
    class PCN_NOVTABLE CUIEventHost : public CUIStyleValue {
        // function node
        struct FunctionNode;
    public:
        // add gui event listener with ownid
        template<typename E, typename Callable>
        inline Conn AddGuiEventListener(E e, Callable call) {
            static_assert(
                std::is_same<E, const char*>::value ||
                std::is_same<E, U8View>::value ||
                std::is_same<E, GuiEvent>::value,
                "e must be 'const char*' or 'GuiEvent'"
                );
            GuiEventListener listener{ call };
            return this->add_gui_event_listener(e, std::move(listener));
        }
        //// remove all same-gui event listener with enum-name-event
        //void RemoveGuiEventListener(GuiEvent) noexcept;
        //// remove all same-gui event listener with string-name-event
        //void RemoveGuiEventListener(U8View view) noexcept {
        //    RemoveGuiEventListener(strtoe(view)); }
        //// remove all same-gui event listener with string-name-event
        //void RemoveGuiEventListener(const char* str) noexcept {
        //    RemoveGuiEventListener(strtoe(U8View::FromCStyle(str))); }
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
        Conn add_gui_event_listener(GuiEvent, GuiEventListener&&) noexcept;
        // add gui event listener
        Conn add_gui_event_listener(const char* str, GuiEventListener&& l) noexcept {
            return add_gui_event_listener(U8View::FromCStyle(str), std::move(l)); }
        // add gui event listener
        Conn add_gui_event_listener(U8View view, GuiEventListener&& l) noexcept {
            return add_gui_event_listener(view, std::move(l)); }
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