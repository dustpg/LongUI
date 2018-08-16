#include "ui_unimacro.h"
#include <utility>

namespace LongUI { namespace detail {
    // struct: unicall_funcptr_t
    template<typename return_t, typename... args_t>
    struct unicall_funcptr_t {
        return_t (UNICALL* ptr)(args_t...) noexcept;
    };
    // struct unicall_funcptr_helper
    template<typename T, typename return_t, typename... args_t>
    struct unicall_funcptr_helper {
        static return_t UNICALL call(args_t... args) noexcept {
            // C++20     : T lambda;
            // C++14(UB?): T lambda{ *static_cast<T*>(nullptr) };
            T lambda{ *static_cast<T*>(nullptr) };
            return lambda(std::forward<args_t>(args)...);
        }
    };
    // function get_unicall_funcptr
    template<typename return_t, typename... args_t, typename T>
    inline auto get_unicall_funcptr(T) noexcept ->unicall_funcptr_t<return_t, args_t...> {
        unicall_funcptr_t<return_t, args_t...> rv;
        rv.ptr = unicall_funcptr_helper<T, return_t, args_t...>::call;
        return rv;
    }
}}