#include <core/ui_object.h>
#include <core/ui_manager.h>
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
    ~FunctionNode() noexcept { this->free_script(); }
    // free script
    void free_script() noexcept;
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

/// <summary>
/// Frees the script.
/// </summary>
/// <returns></returns>
void LongUI::CUIEventHost::FunctionNode::free_script() noexcept {
    if (this->scriptptr) {
        UIManager.FreeScript(this->script());
        scriptlen = 0;
        this->scriptptr = nullptr;
    }
}

namespace LongUI { 
    // find index
    auto Uint32FindIndex(const uint32_t list[], uint32_t len, uint32_t code) noexcept->uint32_t;
    // event hash code list
    static const uint32_t GUIEVENT_NAME_HASH[] = {
        0x0d8f8046_ui32,    // unknown
        0x0d3e3973_ui32,    // blur
        0x0d707348_ui32,    // focus
        0xd861f07c_ui32,    // click
        0xc450725e_ui32,    // dblclick
        0x72d7a914_ui32,    // change
        0xf7ea35ac_ui32,    // select
        0x777a465b_ui32,    // command
        0x41fab016_ui32,    // input
    };
}



/// <summary>
/// string to gui event id
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
auto LongUI::CUIEventHost::StrToId(U8View view) noexcept -> GuiEvent {
#ifndef DEBUG
    const auto ori_view = view;
#endif
    // 以on开头?
    if (view.end() > view.begin() + 2) {
        const auto ch0 = view.begin()[0];
        const auto ch1 = view.begin()[1];
        if (ch0 == 'o' && ch1 == 'n') view.first += 2;
    }
    // XXX: 有现成的查找函数
    constexpr uint32_t len = sizeof(GUIEVENT_NAME_HASH)
        / sizeof(GUIEVENT_NAME_HASH[0]);
    // 计算hash
    const auto hash = LongUI::BKDRHash(view.begin(), view.end());
    auto index = LongUI::Uint32FindIndex(GUIEVENT_NAME_HASH, len, hash);
    // 范围检查
    if (index == len) {
        index = 0;
#ifndef DEBUG
        LUIDebug(Error) 
            << "event not found: "
            << ori_view
            << endl;
#endif
    }
    return static_cast<GuiEvent>(index);
}


/// <summary>
/// Sets the script.
/// </summary>
/// <param name="event">The event.</param>
/// <param name="script">The script.</param>
/// <returns></returns>
void LongUI::CUIEventHost::SetScript(GuiEvent event, U8View script) noexcept {
    assert(event != GuiEvent::Event_Unknown);
    if (const auto node = this->require_event(event)) {
        // 释放之前的
        node->free_script();
        // 申请新的
        const auto data = UIManager.AllocScript(script);
        node->scriptlen = static_cast<uint32_t>(data.size);
        node->scriptptr = data.script;
    }
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

/// <summary>
/// Triggers the event.
/// </summary>
/// <param name="event">The event.</param>
/// <returns></returns>
auto LongUI::CUIEventHost::TriggerEvent(GuiEvent event) noexcept ->EventAccept {
    const auto ctrl = static_cast<UIControl*>(this);
    auto func = Event_Ignore;
    bool script_result = false;
    // 查找相应事件
    if (const auto node = this->find_event(event)) {
        // 脚本
        if (node->scriptptr) {
            script_result = UIManager.Evaluation(node->script(), *ctrl);
        }
        // 监听
        if (node->func.IsOK()) func = node->func(*ctrl);
    }
    // 直接管理则为超类
    assert(ctrl->GetWindow() && "cannot trigger event not under window");
    UIControl* const handler = ctrl->IsGuiEvent2Parent() ?
        ctrl->GetParent() : &ctrl->GetWindow()->RefViewport();
    // 广播事件
    const auto upper = handler->DoEvent(ctrl, EventGuiArg{ event });
    // 检查准确性
    assert((func & 0) == 0); assert((upper & 0) == 0);
    // 返回结果
    return static_cast<EventAccept>(0 | script_result | func | upper);
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
