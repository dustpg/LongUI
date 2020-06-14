#pragma once
//#pragma interface
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
        // add gui event listener
        Conn add_gui_event_listener(GuiEvent, GuiEventListener&&) noexcept;
    public:
        // event string view to event id
        static auto StrToId(U8View view) noexcept ->GuiEvent;
        // add gui event listener
        template<typename Callable>
        inline Conn AddGuiEventListener(GuiEvent e, Callable&& call) noexcept {
            //static_assert(
            //    std::is_same<E, const char*>::value ||
            //    std::is_same<E, U8View>::value ||
            //    std::is_same<E, GuiEvent>::value,
            //    "e must be 'const char*' or 'GuiEvent'"
            //    );
            GuiEventListener listener{ std::move(call) };
            return this->add_gui_event_listener(e, std::move(listener));
        }
        // add gui event listener const ref overload
        template<typename Callable>
        inline Conn AddGuiEventListener(GuiEvent e, const Callable& call) noexcept {
            //static_assert(
            //    std::is_same<E, const char*>::value ||
            //    std::is_same<E, U8View>::value ||
            //    std::is_same<E, GuiEvent>::value,
            //    "e must be 'const char*' or 'GuiEvent'"
            //    );
            Callable mcall = call;
            GuiEventListener listener{ std::move(mcall) };
            return this->add_gui_event_listener(e, std::move(listener));
        }
        // fire event
        virtual auto FireEvent(GuiEvent event) noexcept ->EventAccept;
        // set script
        void SetScript(GuiEvent, U8View) noexcept;
    private:
        // find event
        auto find_event(GuiEvent) const noexcept->FunctionNode*;
        // require event
        auto require_event(GuiEvent) noexcept->FunctionNode*;
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