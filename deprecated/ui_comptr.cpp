#include <cassert>
#include <combaseapi.h>
#include <util/ui_comptr.h>

// ui::detail namespace
namespace UI { namespace detail {
    // safe add-ref
    inline void safe_ref(void* ptr) noexcept {
        if (ptr) static_cast<IUnknown*>(ptr)->AddRef();
    }
    // safe release
    inline void safe_release(void* ptr) noexcept {
        if (ptr) static_cast<IUnknown*>(ptr)->Release();
    }
    // safe release
    inline uint32_t safe_release_ex(void* ptr) noexcept {
        if (ptr) return static_cast<IUnknown*>(ptr)->Release();
        return 0;
    }
}}

PCN_NOINLINE
/// <summary>
/// Initializes a new instance of the <see cref="com_ptr_base" /> class.
/// </summary>
/// <param name="x">The x.</param>
UI::detail::com_ptr_base::com_ptr_base(object& x) noexcept 
    : m_pointer(&x) {
    detail::safe_ref(m_pointer);
}


PCN_NOINLINE
/// <summary>
/// Initializes a new instance of the <see cref="com_ptr_base"/> class.
/// </summary>
/// <param name="x">The x.</param>
UI::detail::com_ptr_base::com_ptr_base(const self& x) noexcept
: m_pointer(x.m_pointer){
    detail::safe_ref(m_pointer);
}

PCN_NOINLINE
/// <summary>
/// Finalizes an instance of the <see cref="com_ptr_base"/> class.
/// </summary>
/// <returns></returns>
UI::detail::com_ptr_base::~com_ptr_base() noexcept {
#ifdef NDEBUG
    detail::safe_release(m_pointer);
#else
    constexpr intptr_t invptr = -1;
    assert(reinterpret_cast<intptr_t>(m_pointer) != invptr);
    auto refc = detail::safe_release_ex(m_pointer);
    refc = refc;
    reinterpret_cast<intptr_t&>(m_pointer) = invptr;
#endif
}

PCN_NOINLINE
/// <summary>
/// Operator=s the specified x.
/// </summary>
/// <param name="x">The x.</param>
/// <returns></returns>
auto UI::detail::com_ptr_base::operator=(self&& x) noexcept ->self& {
    detail::safe_release(m_pointer);
    m_pointer = x.m_pointer;
    x.m_pointer = nullptr;
    return *this;
}

PCN_NOINLINE
/// <summary>
/// Operator=s the specified x.
/// </summary>
/// <param name="x">The x.</param>
/// <returns></returns>
auto UI::detail::com_ptr_base::operator=(const self& x) noexcept ->self& {
    detail::safe_release(m_pointer);
    m_pointer = x.m_pointer;
    detail::safe_ref(m_pointer);
    return *this;
}