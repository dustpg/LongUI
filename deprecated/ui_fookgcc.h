template<typename Result, typename... Args>
struct stdcall_funcptr_t {
    Result(LUISTDCALL* ptr)(Args...) noexcept;
};
template<typename T, typename Result, typename... Args>
struct stdcall_funcptr_helper {
    static Result LUISTDCALL call(Args... args) noexcept {
        T lambda{ *static_cast<T*>(nullptr) };
        return lambda(std::forward<Args>(args)...);
    }
};

template<typename Result, typename... Args, typename T>
auto get_stdcall_funcptr(T) noexcept ->stdcall_funcptr_t<Result, Args...> {
    stdcall_funcptr_t<Result, Args...> rv;
    rv.ptr = stdcall_funcptr_helper<T, Result, Args...>::call;
    return rv;
}