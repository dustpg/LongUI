#include <type_traits>
#include <cstdint>

#if _MSC_VER < 1900
#define noexcept
#endif

namespace LongUI { namespace POD {
    /// <summary>
    /// detail namespace
    /// </summary>
    namespace detail {
        /// <summary>
        /// Vector base class
        /// </summary>
        class fixed_queue_base {
        protected:
            // dtor
            ~fixed_queue_base() noexcept = default;
            // ctor
            fixed_queue_base(uint32_t bytesize, uint32_t capacity) noexcept;
            // no copy ctor
            fixed_queue_base(const fixed_queue_base&) = delete;
            // no move ctor
            fixed_queue_base(fixed_queue_base&&) = delete;
            // full ?
            bool full() const noexcept;
            // empty ?
            bool empty() const noexcept { return m_uHeadBytePos == m_uTailBytePos; }
            // get data
            auto front() noexcept ->char* { return data_ptr() + m_uHeadBytePos; }
            // get data
            auto front() const noexcept ->const char* { return data_ptr() + m_uHeadBytePos; }
            // push back
            void push_back(const char* ptr) noexcept;
            // pop front
            void pop_front() noexcept;
            // size 
            auto size() const->size_t;
        protected:
            // get data ptr
            auto data_ptr() noexcept ->char* { return reinterpret_cast<char*>(this + 1); }
            // get const data ptr
            auto data_ptr() const noexcept ->const char* { return reinterpret_cast<const char*>(this + 1); }
        private:
            // data byte size
            const uint32_t  m_uByteLen;
            // capacity byte
            const uint32_t  m_uByteCapacity;
            // head data byte pos
            uint32_t        m_uHeadBytePos = 0;
            // tail data pos
            uint32_t        m_uTailBytePos = 0;
#ifndef NDEBUG
        public:
            int             m_ddd[4];
            // debug data count
            uintptr_t       data_count_dbg = 0;
#endif
        };
        // fixed_queue_helper
        struct fixed_queue_helper {
            /// <summary>
            /// Offsets the check.
            /// </summary>
            /// <returns></returns>
            template<typename T, uintptr_t LEN> 
            static void offset_check() noexcept;
        };
    }
    /// <summary>
    /// fixed queue
    /// </summary>
    /// <seealso cref="detail::fixed_queue_base" />
    template<typename T, uintptr_t LEN>
    class fixed_queue : protected detail::fixed_queue_base {
        // helper
        using helper = detail::fixed_queue_helper;
        // offset check
        friend struct helper;
        // type helper
        static auto tr(T*p) noexcept ->char* { return reinterpret_cast<char*>(p); }
        // type helper
        static auto tr(const T*p) noexcept ->const char* { return reinterpret_cast<const char*>(p); }
    public:
        // check for pod
        static_assert(std::is_pod<T>::value, "type T must be POD type");
        // ctor
        fixed_queue() noexcept : fixed_queue_base(sizeof(T), LEN) { helper::offset_check<T, LEN>(); }
        // dtor
        ~fixed_queue() noexcept = default;
        // push back
        void push_back(const T& x) noexcept { fixed_queue_base::push_back(tr(&x)); }
        // pop front
        void pop_front() noexcept { fixed_queue_base::pop_front(); }
    public:
        // size 
        auto size() const->size_t { return fixed_queue_base::size(); }
        // full ?
        bool full() const noexcept { return fixed_queue_base::full(); }
        // empty ?
        bool empty() const noexcept { return fixed_queue_base::empty(); }
        // get data
        auto front() noexcept ->T& { return *reinterpret_cast<T*>(fixed_queue_base::front()); }
        // get data
        auto front() const noexcept ->const T&{ return *reinterpret_cast<const T*>(fixed_queue_base::front()); }
    private:
        // fixed data[+1 for FULL check ]
        T               m_data[LEN+1];
    };
    /// <summary>
    /// detail namespace
    /// </summary>
    namespace detail {
        template<typename T, uintptr_t LEN>
        inline void fixed_queue_helper::offset_check() noexcept {
            using queue_t = fixed_queue<T, LEN>;
            constexpr auto offset = offsetof(queue_t, m_data);
            constexpr auto fixeds = sizeof(fixed_queue_base);
            static_assert(
                offset == fixeds,
                "bad member layout, check type T's alignment"
                );
        }
    }
}}