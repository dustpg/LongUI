#include <core/ui_object.h>
#include <core/ui_window.h>
#include <container/pod_hash.h>
#include <control/ui_viewport.h>
#include <event/ui_event_host.h>
#include "../private/ui_private_control.h"

// function node
struct LongUI::CUIEventHost::FunctionNode : LongUI::CUISmallObject {
    // ctor
    FunctionNode(FunctionNode*n, GuiEvent ge) noexcept 
        :next(n), type(ge)   {}
    // dtor
    ~FunctionNode() noexcept { assert(!scriptptr && "NOT IMPL"); }
    // Script
    auto script() const noexcept { return ScriptUI{ scriptptr, scriptlen }; }
    // next
    FunctionNode*           next;
    // function
    GuiEventListener        func;
    // type
    GuiEvent                type;
    // script data length
    uint32_t                scriptlen = 0;
    // script data address
    const uint8_t*          scriptptr = nullptr;
};

namespace LongUI { 
    // find index
    auto Uint32FindIndex(const uint32_t list[], uint32_t len, uint32_t code) noexcept->uint32_t;
    // event hash code list
    static const uint32_t GUIEVENT_NAME_HASH[] = {
        0xa3affbe4_ui32,    // oncommand
        0x44abeefd_ui32,    // onclick
        0xdcb4e517_ui32,    // onchange
        0xa67f471e_ui32,    // onblur
        0x79ba71c9_ui32,    // onfocus
        0x61c771af_ui32,    // onselect
    };
}



/// <summary>
/// string to gui event id
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
auto LongUI::CUIEventHost::strtoe(U8View view) noexcept -> GuiEvent {
    // XXX: 有现成的查找函数
    constexpr uint32_t len = sizeof(GUIEVENT_NAME_HASH)
        / sizeof(GUIEVENT_NAME_HASH[0]);
    // 计算hash
    const auto hash = BKDRHash(view.begin(), view.end());
    const auto index = LongUI::Uint32FindIndex(GUIEVENT_NAME_HASH, len, hash);
    // 范围检查
    assert(index < len && "type not found");
    return static_cast<GuiEvent>(index);
}


PCN_NOINLINE
/// <summary>
/// Finds the event.
/// </summary>
/// <param name="type">The type.</param>
/// <returns></returns>
auto LongUI::CUIEventHost::find_event(GuiEvent type)const noexcept->FunctionNode*{
    auto node = m_pFuncNode;
    // 查找已存在的节点
    while (node) { if (node->type == type) return node; node = node->next; }
    return nullptr;
}

/// <summary>
/// Requires the event.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
auto LongUI::CUIEventHost::require_event(GuiEvent type) noexcept -> FunctionNode*{
    if (const auto node = this->find_event(type)) return node;
    // 创建新的节点
    if (const auto node = new(std::nothrow) FunctionNode{m_pFuncNode, type}) {
        return m_pFuncNode = node;
    }
    return nullptr;
}


/// <summary>
/// Adds the GUI event listener.
/// </summary>
/// <param name="ownid">The ownid.</param>
/// <param name="e">The e.</param>
/// <param name="listener">The listener.</param>
/// <returns></returns>
auto LongUI::CUIEventHost::add_gui_event_listener(
    GuiEvent e,
    GuiEventListener&& listener) noexcept ->Conn {
    const auto ctrl = static_cast<UIControl*>(this);
    assert(ctrl && "null this pointer!");
    const auto a = reinterpret_cast<void*>(this);
    const auto b = reinterpret_cast<void*>(ctrl);
    assert(a == b && "must be same");
    if (const auto node = this->require_event(e)) {
        return node->func.AddCallChain(std::move(listener));
    }
    return { 0 };
}

PCN_NOINLINE
/// <summary>
/// Triggers the event.
/// </summary>
/// <param name="event">The event.</param>
/// <returns></returns>
auto LongUI::CUIEventHost::TriggerEvent(GuiEvent event) noexcept ->EventAccept {
    const auto ctrl = static_cast<UIControl*>(this);
    auto func = Event_Ignore;
    // 查找相应事件
    if (const auto node = this->find_event(event)) {
        // 脚本
        assert(!node->scriptptr && "NOT IMPL");
        // 事件
        func = node->func(*ctrl);
    }
    // 直接管理则为父类
    assert(ctrl->GetWindow() && "cannot trigger event not under window");
    UIControl* const handler = ctrl->IsGuiEvent2Parent() ?
        ctrl->GetParent() : &ctrl->GetWindow()->RefViewport();
    // 广播事件
    const auto upper = handler->DoEvent(ctrl, EventGuiArg{ event });
    // 检查准确性
    assert((func & 0) == 0); assert((upper & 0) == 0);
    // 返回结果
    return static_cast<EventAccept>(0 | func | upper);
}

/// <summary>
/// Removes the GUI event listener.
/// </summary>
/// <param name="ownid">The ownid.</param>
/// <param name="">The .</param>
/// <returns></returns>
//void LongUI::CUIEventHost::RemoveGuiEventListener(GuiEvent e) noexcept {
//    const auto ctrl = static_cast<UIControl*>(this);
//    if (const auto node = this->find_event(e)) {
//        node->func.RemoveCallChain(ownid);
//    }
//}


/// <summary>
/// Finalizes an instance of the <see cref="CUIEventHost"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIEventHost::~CUIEventHost() noexcept {
    auto node = m_pFuncNode;
    while (node) {
        const auto tmp = node->next;
        delete node;
        node = tmp;
    }
}
