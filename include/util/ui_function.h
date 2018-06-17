#pragma once

#include "../core/ui_object.h"
#include "../core/ui_core_type.h"
#include <cassert>

namespace LongUI {
    // detail namespace
    namespace detail {
        // func base
        template<typename Result, typename ...Args>
        class func : public CUISmallObject {
        public:
            // call
            virtual auto call(Args&&... args) noexcept->Result = 0;
            // dtor
            virtual ~func() noexcept { delete this->chain; };
            // own id
            uintptr_t   ownid;
            // call chain [next node]
            func*       chain = nullptr;
        };
        // func derived 
        template<typename Func, typename Result, typename ...Args>
        class func_ex final : public func<Result, Args...> {
            // cannot be function pointer
            static_assert(
                std::is_pointer<Func>::value == false,
                "cannot be function pointer because of type-safety"
                );
            // func data
            Func                m_func;
        public:
            // ctor
            func_ex(const Func &x, uintptr_t id) noexcept : m_func(x) { this->ownid = id; }
            // call
            auto call(Args&&... args) noexcept ->Result override {
                if (this->chain) this->chain->call(std::forward<Args>(args)...);
                return m_func(std::forward<Args>(args)...);
            }
        };
        // type helper
        template<typename Func> struct type_helper {
            using type = Func;
        };
        // type helper
        template<typename Result, typename ...Args> struct type_helper<Result(Args...)> {
            using type = Result(*)(Args...);
        };
        // uifunc helper
        struct uifunc_helper {
            // add chain, specialization for reducing code size
            static void add_chain_helper(GuiEventListener&, GuiEventListener&) noexcept;
            // add chain
            template<typename T> static inline void add_chain(T& a, T& b) noexcept {
                add_chain_helper(reinterpret_cast<GuiEventListener&>(a), reinterpret_cast<GuiEventListener&>(b));
            }
            // remove chain, specialization for reducing code size
            static void remove_chain_helper(GuiEventListener& gel, uintptr_t id) noexcept;
            // remove chian
            template<typename T> static inline void remove_chain(T& gel, uintptr_t id) noexcept {
                remove_chain_helper(reinterpret_cast<GuiEventListener&>(gel), id);
            }
        };
    }
    // UI Function, lightweight and chain-call-able version std::function
    template<typename Result, typename... Args>
    class CUIFunction<Result(Args...)> {
    public:
        // friend
        friend detail::uifunc_helper;
        // this type
        using Self = CUIFunction<Result(Args...)>;
        // RealFunc pointer
        detail::func<Result, Args...>* m_pFunction = nullptr;
        // dispose
        void dispose() noexcept { delete m_pFunction; }
    public:
        // Ok
        bool IsOK() const noexcept { return !!m_pFunction; }
        // dtor
        ~CUIFunction() noexcept { this->dispose(); }
        // ctor
        CUIFunction() noexcept = default;
        // move ctor
        CUIFunction(Self&& obj) noexcept : m_pFunction(obj.m_pFunction) {
            assert(&obj != this && "bad move"); obj.m_pFunction = nullptr;
        };
        // no copy ctor
        CUIFunction(const Self&) = delete;
        // operator =
        Self&operator=(const Self &x) noexcept = delete;
        // add call chain
        Self&operator += (Self&& chain) { this->AddCallChain(std::move(chain)); return *this; }
        // add call chain
        template<typename Func>
        Self& operator += (const Func &x) { this->AddCallChain(std::move(CUIFunction(x))); return *this; }
        // operator =
        template<typename Func> Self& operator=(const Func &x) noexcept {
            this->dispose(); m_pFunction = new(std::nothrow) detail::
                func_ex<typename detail::type_helper<Func>::type, Result, Args...>(x); return *this;
        }
        // operator =
        Self& operator=(Self&& x) noexcept {
            this->dispose(); std::swap(m_pFunction, x.m_pFunction); return *this;
        }
        // ctor with func
        template<typename Func> CUIFunction(const Func& f, uintptr_t ownid = 0) noexcept : m_pFunction(
            new(std::nothrow) detail::func_ex<typename detail::type_helper<Func>::type, Result, Args...>(f, ownid)) { }
        // () operator
        auto operator()(Args&&... args) const noexcept -> Result {
            assert(m_pFunction && "bad call or oom");
            return m_pFunction ? m_pFunction->call(std::forward<Args>(args)...) : Result();
        }
        // add call chain with ownid
        template<typename Func>
        void AddCallChain(const Func &x, uintptr_t ownid) { this->AddCallChain(std::move(CUIFunction(x, ownid))); }
        // add call chain
        void AddCallChain(Self&& chain) noexcept { detail::uifunc_helper::add_chain(*this, chain); }
        // remove call chain 
        void RemoveCallChain(uintptr_t ownid) noexcept { detail::uifunc_helper::remove_chain(*this, ownid); }
    };
}