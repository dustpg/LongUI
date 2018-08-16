#include "ui_unimacro.h"
#include <utility>

namespace LongUI { namespace detail {
    // struct: unicall_funcptr_t
    template<typename Result, typename... Args>
    struct unicall_funcptr_t {
        Result(UNICALL* ptr)(Args...) noexcept;
    };
    // struct unicall_funcptr_helper
    template<typename T, typename Result, typename... Args>
    struct unicall_funcptr_helper {
        static Result UNICALL call(Args... args) noexcept {
            T lambda{ *static_cast<T*>(nullptr) };
            return lambda(std::forward<Args>(args)...);
        }
    };
    // function get_unicall_funcptr
    template<typename Result, typename... Args, typename T>
    inline auto get_unicall_funcptr(T) noexcept ->unicall_funcptr_t<Result, Args...> {
        unicall_funcptr_t<Result, Args...> rv;
        rv.ptr = unicall_funcptr_helper<T, Result, Args...>::call;
        return rv;
    }
}}