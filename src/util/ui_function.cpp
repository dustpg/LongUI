#include <util/ui_sort.h>
#include <util/ui_function.h>
#include <cassert>


/// <summary>
/// Disconnects this instance.
/// </summary>
/// <returns></returns>
void LongUI::Conn::Disconnect() noexcept {
    if (!this->handle) return;
    // 清空以免二次 Disconnect
    using func_t = GuiEventListener::FuncT;
    const auto func = reinterpret_cast<func_t*>(this->handle);
    const_cast<uintptr_t&>(this->handle) = 0;
    // 连接前后节点
    *func->prev_funcpp = func->chain;
    func->chain = nullptr;
    //释放数据
    delete func;
}

/// <summary>
/// Adds the chain helper.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
auto LongUI::detail::uifunc_helper::add_chain_helper(
    GuiEventListener& a,
    GuiEventListener&& b) noexcept -> uintptr_t {
    // 将B的调用链添加至A中
    // 1. 寻找A的末尾节点
    // 2. 指向B开始节点
    // 3. 修正prev_funcpp
    // 4. 记录B的句柄
    // 5. 消灭B
    if (b.IsOK()) {
        using write_t = decltype(a.m_pFunction);
        // STEP#1 
        write_t* to_write = &a.m_pFunction;
        while (*to_write) to_write = &(*to_write)->chain;
        // STEP#2
        *to_write = b.m_pFunction;
        // STEP#3
        b.m_pFunction->prev_funcpp = to_write;
        // STEP#4
        const auto handle = reinterpret_cast<uintptr_t>(b.m_pFunction);
        // STEP#5
        b.m_pFunction = nullptr;
        // 返回
        return handle;
    }
    else {
        assert(!"error, check arg#IsOK ");
        return 0;
    }
}

/// <summary>
/// Empties the dtor.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::detail::empty_dtor(void*) noexcept {

}

namespace LongUI {
    // bubble sort for vector or list
    template<typename Itr, typename Lamda>
    inline void BubbleSort(Itr begin, Itr end, Lamda lam) noexcept {
        --end;
        bool flag = true;
        while (flag) {
            flag = false;
            for (auto i = begin; i != end; ++i) {
                auto j = i; ++j;
                if (lam(*j, *i)) {
                    std::swap(*j, *i);
                    flag = true;
                }
            }
            --end;
        }
    }
}

PCN_NOINLINE
/// <summary>
/// sort the pointers
/// </summary>
/// <param name="ptr"></param>
/// <param name="end"></param>
/// <param name="offset"></param>
/// <param name="mask"></param>
/// <returns></returns>
void LongUI::SortPointers(const void**ptr, const void**end, uint32_t offset, uint32_t mask) noexcept {
    assert(end > ptr && "bad count");
    assert((offset & 3) == 0 && "bad offset");
    const auto comp = [offset, mask](const char* a, const char *b) noexcept {
        const auto x = *reinterpret_cast<const uint32_t*>(a + offset) & mask;
        const auto y = *reinterpret_cast<const uint32_t*>(b + offset) & mask;
        return x < y;
    };
    const auto begin_x = reinterpret_cast<const char**>(ptr);
    const auto end_x = reinterpret_cast<const char**>(end);
#if 0
    // 针对大于1000个使用 高级排序算法
    if (count > 1000) {
        std::sort(begin, end, comp);
        return;
    }
#endif
    LongUI::BubbleSort(begin_x, end_x, comp);
}