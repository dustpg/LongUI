#pragma once
#include <cstdint>
#include "../util/ui_unimacro.h"

// virtual dtor
#define FIBER_DTOR_TYPE virtual


namespace LongUI {
    // fiber
    class CUIFiber;
    // lambda fiber
    template<typename T> class CUILambdaFiber;
    /// <summary>
    /// main fiber class, one main fiber in one thread
    /// </summary>
    class CUIMainFiber {
        // self type
        using Self = CUIMainFiber;
        // friend class
        friend CUIFiber;
    public:
        // ctor, create one for one thread
        CUIMainFiber() noexcept;
        // dtor
        ~CUIMainFiber() noexcept;
        // create fiber with class, default stack-size is 1M, you can set as small as possible
        template<class T> auto CreateFiber(size_t stack = 0/*alignas 4k*/) noexcept ->T {
            return{ stack, m_fiber };
        }
        // create fiber with lambda
        template<class T> auto CreateFiber(T, size_t stack = 0) noexcept->CUILambdaFiber<T>;
    public:
        // no copy ctor
        CUIMainFiber(const Self&) = delete;
        // no copy assgin
        Self&operator=(const Self&) = delete;
        // no move ctor?
        CUIMainFiber(Self&&) = delete;
        // no move assgin?
        Self&operator=(Self&&) = delete;
    private:
        // fiber pointer
        void*           m_fiber;
    };
    /// <summary>
    /// fiber class
    /// </summary>
    class CUIFiber {
        // self type
        using Self = CUIFiber;
        // friend class
        friend CUIMainFiber;
        // run fiber
        virtual void run() noexcept = 0;
    protected:
        // ctor
        CUIFiber(size_t stack, void* main) noexcept;
        // yield, switch to main
        void yield() noexcept { return switch_to(m_main); }
        // get fiber data
        static void* get_fibder_data() noexcept;
    public:
        // dtor
        FIBER_DTOR_TYPE ~CUIFiber() noexcept;
        // is ok?
        bool IsOK() const noexcept { return !!m_fiber; }
        // switch to this
        void Resume() noexcept { return switch_to(m_fiber); }
        // yield
        static void FiberYield() noexcept;
    public:
        // move ctor
        CUIFiber(Self&& x) noexcept : m_main(x.m_main), m_fiber(x.m_fiber) { x.m_fiber = nullptr; }
        // no copy ctor
        CUIFiber(const Self&) = delete;
        // no copy assgin
        Self&operator=(const Self&) = delete;
        // no move assgin?
        Self&operator=(Self&&) = delete;
    private:
        // switch to fiber
        static void switch_to(void* ptr) noexcept;
        // switch to fiber with arg
        static auto switch_to(void* ptr, intptr_t arg) noexcept->intptr_t;
        // main fiber
        void*           m_main;
        // fiber pointer
        void*           m_fiber;
#ifndef NDEBUG
        // debug flag
        bool            m_released = true;
#endif
    };
    template<typename T>
    class CUILambdaFiber : public CUIFiber {
        // friend class
        friend CUIMainFiber;
        // ctor
        CUILambdaFiber(size_t stack, void* main, T call) noexcept
            : CUIFiber(stack, main), m_lambda(call) {}
        // run fiber
        void run() noexcept override { m_lambda(); }
        // lambda
        T           m_lambda;
    };
    /// <summary>
    /// Creates the fiber.
    /// </summary>
    /// <param name="">The .</param>
    /// <returns></returns>
    template<class T>
    auto CUIMainFiber::CreateFiber(T call, size_t stack) noexcept -> CUILambdaFiber<T> {
        const auto ptr = CUIFiber::get_fibder_data();
        return CUILambdaFiber<T>{stack, m_fiber, call};
    }
    // helper func
    inline void fiber_yield() noexcept { CUIFiber::FiberYield(); }
}

// into top namespace
using LongUI::fiber_yield;
