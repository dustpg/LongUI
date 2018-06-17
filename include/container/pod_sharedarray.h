#pragma once
#include <cstdint>
#include <type_traits>
#include "../util/ui_unimacro.h"


// ui namespace
namespace LongUI { namespace POD {
    // forward
    template<typename T> class SharedArray;
    // detail namespace
    namespace detail {
        // basic array
        class basic_shared_array {
            // friend class
            template<typename T> friend class POD::SharedArray;
        protected:
            // assert range
#ifdef NDEBUG
            inline void assert_range(uint32_t) noexcept {};
#else
            void assert_range(uint32_t i) noexcept;
#endif
            // self type
            using self = basic_shared_array;
            // add ref
            auto add_ref() noexcept -> uint32_t { return ++m_cRefCount; }
            // release
            auto release() noexcept ->uint32_t;
            // get data
            auto data() noexcept ->void* { return m_data; }
            // length
            auto length() const noexcept ->uint32_t { return m_cLength; }
            // copy from
            static void copy_from(self*&, self*) noexcept;
            // move from
            static void move_from(self*&, self*&) noexcept;
            // safe ref
            static auto safe_ref(self* x) noexcept -> self * { if (x) x->add_ref(); return x; }
            // safe release
            static void safe_release(self* x) noexcept { if (x) x->release(); }
            // create one
            static auto create(
                const void* data,
                uint32_t len,
                uint32_t byte
            ) noexcept->self*;
        protected:
            // ref-count
            uint32_t        m_cRefCount;
            // length in count
            uint32_t        m_cLength;
            // data
#pragma warning(suppress: 4200)
            char            m_data[0];
        };
    }
    // pod shared array
    template<typename T>
    class SharedArray {
        // self type
        using Self = SharedArray;
    public:
        // check for pod
        static_assert(std::is_pod<T>::value, "type T must be POD type");
        // release
        //void release() noexcept { detail::basic_shared_array::safe_release(m_pointer); m_pointer = nullptr; }
        // ctor
        ~SharedArray() noexcept { detail::basic_shared_array::safe_release(m_pointer); }
        // ctor with data
        SharedArray(const T* begin, const T* end) noexcept : m_pointer(
            detail::basic_shared_array::create(begin, uint32_t(end - begin), sizeof(T))) {}
        // ctor with data
        SharedArray(const T* begin, uint32_t length) noexcept : m_pointer(
            detail::basic_shared_array::create(begin, length, sizeof(T))) {}
        // copy ctor
        SharedArray(const SharedArray& x) noexcept : m_pointer(
            detail::basic_shared_array::safe_ref(x.m_pointer)){  }
        // move ctor
        SharedArray(SharedArray&& x) noexcept : m_pointer(x.m_pointer)
            { x.m_pointer = nullptr; }
        // operator copy =
        Self& operator=(const SharedArray&x) noexcept { detail::basic_shared_array::copy_from(m_pointer, x.m_pointer); return *this; }
        // operator move =
        Self& operator=(SharedArray&&x) noexcept { detail::basic_shared_array::move_from(m_pointer, x.m_pointer); return *this; }
        //SharedArray(SharedArray&& x) noexcept { release(); m_pointer = x.m_pointer; x.m_pointer = nullptr; }
        // is ok?
        bool is_ok() const noexcept { return !!m_pointer; }
        // bool
        operator bool() const noexcept { return !!m_pointer; }
        // not
        bool operator !() const noexcept { return !m_pointer; }
        // size
        auto size() const noexcept ->uint32_t { return m_pointer->length(); }
        // length
        auto length() const noexcept ->uint32_t { return m_pointer->length(); }
        // end
        auto end() noexcept -> T* { return begin() + length(); }
        // begin
        auto begin() noexcept -> T* { return reinterpret_cast<T*>(m_pointer->data()); }
        // cend
        auto cend() noexcept -> const T*{ return cbegin() + length(); ; }
        // cbegin
        auto cbegin() noexcept -> const T* { return reinterpret_cast<const T*>(m_pointer->data()); }
        // operator []
        T& operator [](uint32_t i) noexcept { 
            m_pointer->assert_range(i); 
            return reinterpret_cast<T*>(m_pointer->data())[i]; 
        }
        // operator [] const
        const T& operator [](uint32_t i) const noexcept {
            m_pointer->assert_range(i);
            return reinterpret_cast<const T*>(m_pointer->data())[i];
        }
    private:
        // array pointer
        detail::basic_shared_array*     m_pointer;
    };
    // make shared array
    template<typename T, typename U>
    inline auto MakeSharedArray(const T* ptr, U b) noexcept ->SharedArray<T> {
        return { ptr, b };
    }
}}
