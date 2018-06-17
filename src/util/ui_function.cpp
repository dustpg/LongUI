#include <util/ui_function.h>
#include <cassert>


/// <summary>
/// Adds the chain helper.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
void LongUI::detail::uifunc_helper::add_chain_helper(
    GuiEventListener& a,
    GuiEventListener& b) noexcept {
    if (b.IsOK()) {
        b.m_pFunction->chain = a.m_pFunction;
        a.m_pFunction = b.m_pFunction;
        b.m_pFunction = nullptr;
    }
    else {
        assert(!"error, check arg#IsOK ");
    }
}

/// <summary>
/// Removes the chain helper.
/// </summary>
/// <param name="gel">The gel.</param>
/// <param name="id">The identifier.</param>
/// <returns></returns>
void LongUI::detail::uifunc_helper::remove_chain_helper(
    GuiEventListener& gel, uintptr_t id) noexcept {
    auto func = gel.m_pFunction;
    // 无头学姐变成有头学姐
    std::aligned_storage<
        sizeof(std::remove_pointer<decltype(func)>::type),
        alignof(uintptr_t)
    >::type buf;
    //reinterpret_cast<decltype(func)>(&buf)->ownid = ~id;
    reinterpret_cast<decltype(func)>(&buf)->chain = func;
    auto last = reinterpret_cast<decltype(func)>(&buf);
    // 带头单链表节点删除
    while (func) {
        // 找到目标节点
        if (func->ownid == id) {
            const auto temp = last->chain = func->chain;
            func->chain = nullptr;
            delete func;
            func = temp;
        }
        // 推进节点
        else {
            last = func;
            func = func->chain;
        }
    }
    // 结算节点, 砍去头
    gel.m_pFunction = reinterpret_cast<decltype(func)>(&buf)->chain;
}

