#include <Windows.h>
#include <cassert>
#include <thread/ui_fiber.h>

/// <summary>
/// Switches to fiber.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <returns></returns>
void LongUI::CUIFiber::switch_to(void* ptr) noexcept {
    ::SwitchToFiber(ptr);
}

/*
#include <cstddef>
/// <summary>
/// Switches to.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <param name="arg">The argument.</param>
/// <returns></returns>
auto CUIFiber::switch_to(void * ptr, intptr_t arg) noexcept -> intptr_t {
    const auto data = ::GetFiberData();
    constexpr auto a = offsetof(CUIMainFiber, m_arg);
    constexpr auto b = offsetof(CUIFiber, m_arg);
    static_assert(a == b, "must be same");
    reinterpret_cast<data>
    return intptr_t();
}
*/

/// <summary>
/// Initializes a new instance of the <see cref="CUIFiber"/> class.
/// </summary>
/// <param name="main">The main.</param>
LongUI::CUIFiber::CUIFiber(size_t stack, void* main) noexcept:
m_main(main), m_fiber(::CreateFiber(stack, [](void* ptr) {
    const auto _this = reinterpret_cast<Self*>(ptr);
#ifndef NDEBUG
    _this->m_released = false;
#endif
    _this->run();
#ifndef NDEBUG
    _this->m_released = true;
#endif
    _this->yield();
}, this)) { assert((stack % (1024*4)==0) && "stack size must alignas 4k"); }



/// <summary>
/// Finalizes an instance of the <see cref="CUIFiber"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIFiber::~CUIFiber() noexcept {
#ifndef NDEBUG
    assert(m_released && "this fiber not exit, care about resource leak.");
#endif
    if (m_fiber) ::DeleteFiber(m_fiber);
    /*
    1. If the target thread owns a critical section,
    the critical section will not be released.

    2. If the target thread is allocating memory from
    the heap, the heap lock will not be released.
    */
}

/// <summary>
/// Gets the fibder data.
/// </summary>
/// <returns></returns>
void*LongUI::CUIFiber::get_fibder_data() noexcept { return ::GetFiberData(); }

/// <summary>
/// Initializes a new instance of the <see cref="CUIMainFiber"/> class.
/// </summary>
LongUI::CUIMainFiber::CUIMainFiber() noexcept 
    : m_fiber(::ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH)) {}

/// <summary>
/// Finalizes an instance of the <see cref="CUIMainFiber"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIMainFiber::~CUIMainFiber() noexcept {
    ::ConvertFiberToThread();
}

/// <summary>
/// Fibers the yield.
/// </summary>
/// <returns></returns>
void LongUI::CUIFiber::FiberYield() noexcept {
    const auto ptr = ::GetFiberData();
    assert(ptr && "cannot yield for main fiber");
    reinterpret_cast<CUIFiber*>(ptr)->yield();
}