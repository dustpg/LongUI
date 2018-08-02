#pragma once
#include "../container/pod_vector.h"
#include "ui_string_view.h"
#include "ui_core_type.h"
#include "ui_string_helper.h"

namespace LongUI {
    // basic string
    template<typename T, unsigned B>
    class CUIBasicString {
        // self type
        using Self = CUIBasicString;
        // Vector type
        using V = POD::Vector<T>;
        // View
        using View = PodStringView<T>;
        // B must > 0
        static_assert(B > 0, "B must > 0");
        // type helper
        static inline auto tr(T* ptr) noexcept -> char* { return reinterpret_cast<char*>(ptr); }
        // type helper
        static inline auto tr(const T* ptr) noexcept ->const char* { return reinterpret_cast<const char*>(ptr); }
    public:
        // iterator
        using iterator = typename V::iterator;
        // const iterator
        using const_iterator = typename V::const_iterator;
        // size_type
        using size_type = typename V::size_type;
        // npos
        enum : size_type { npos = size_t(-1) };
        // extra type
        using ex_t = POD::detail::vector_base::extra_t<sizeof(T), +1, B>;
        // ctor +1s
        PCN_NOINLINE CUIBasicString() noexcept : m_vector(sizeof(T), ex_t{}) { m_buffer[0] = 0; }
        // ctor with pair
        CUIBasicString(const T* strb, const T* stre) noexcept : Self() { this->assign(strb, stre); }
        // copy ctor
        CUIBasicString(const Self& str) noexcept : Self() { this->assign(str); }
        // ctor with c-style string
        CUIBasicString(const T* str) noexcept : Self(str, str + detail::strlen(str)) { }
        // ctor with init list
        CUIBasicString(std::initializer_list<T> list) noexcept : Self(list.begin(), list.end()) { }
        // ctor with view
        CUIBasicString(View view) noexcept : Self(view.begin(), view.end()) { }
        // move ctor
        CUIBasicString(Self&& str) noexcept : m_vector(std::move(str.m_vector)) {  }
    public:
        // from utf-8
        static auto FromUtf8(const char* a, const char* b) noexcept->Self;
        // from utf-8
        static auto FromUtf8(PodStringView<char> view) noexcept->Self { return FromUtf8(view.begin(), view.end());  }
        // from utf-8
        static auto FromUtf8(const char* str) noexcept ->Self { return FromUtf8(str, str + detail::strlen(str)); }
        // from latin1
        // from number
    public:
        // is empty?
        bool empty() const noexcept { return m_vector.empty(); }
        // begin iterator
        auto begin() noexcept -> iterator { return m_vector.begin(); }
        // end iterator
        auto end() noexcept ->iterator { return m_vector.end(); }
        // front
        auto front() noexcept ->T& { return m_vector.front(); }
        // back
        auto back() noexcept ->T& { return m_vector.back(); }
        // begin iterator
        auto begin() const noexcept -> const_iterator { return m_vector.cbegin(); }
        // end iterator
        auto end() const noexcept ->const_iterator { return m_vector.cend(); }
        // const begin iterator
        auto cbegin() const noexcept -> const_iterator { return m_vector.cbegin(); }
        // const end iterator
        auto cend() const noexcept ->const_iterator { return m_vector.cend(); }
        // const front
        auto front() const noexcept ->const T& { return m_vector.front(); }
        // const back
        auto back() const noexcept ->const T& { return m_vector.back(); }
    public:
        // as buffer, null-terminated string
        template<typename Lam> void as_buffer_nul(uint32_t len, Lam call) noexcept {
            this->reserve(len); if (is_ok()) {
                call(const_cast<T*>(c_str())); 
                detail::string_helper::relength(m_vector, len);
            }
        }
        // as buffer, not null-terminated string
        template<typename Lam> void as_buffer(uint32_t len, Lam call) noexcept {
            this->reserve(len); if (is_ok()) { 
                void* ntbs = m_vector.detail::string_helper::base_str::begin();
                call(reinterpret_cast<T*>(ntbs));
                detail::string_helper::relength(m_vector, len);
                reinterpret_cast<T*>(ntbs)[len] = 0;
            }
        }
    public:
        // assign with string
        Self&assign(const T* str) noexcept { return assign(str, str + detail::strlen(str)); };
        // assign with string
        Self&assign(const Self& str) noexcept { return assign(str.begin(), str.end()); };
        // assign width RAI
        template<typename RAI> typename std::enable_if<type_helper::is_iterator<RAI>::value, Self&>::type
            assign(RAI first, RAI last) noexcept {
            detail::string_helper::string_assign(m_vector, tr(&first[0]), tr(&last[0]));
            return *this;
        }
    public:
        // append with char
        Self&append(const T& ch) noexcept { return append(&ch, &ch + 1); };
        // append with string
        Self&append(const T* str) noexcept { return append(str, str + detail::strlen(str)); };
        // append with string
        Self&append(const Self& str) noexcept { return append(str.begin(), str.end()); };
        // append with char
        Self&append(const View v) noexcept { return append(v.begin(), v.end()); };
        // append width RAI
        template<typename RAI> typename std::enable_if<type_helper::is_iterator<RAI>::value, Self&>::type
            append(RAI first, RAI last) noexcept {
            detail::string_helper::string_insert(m_vector, npos, tr(&first[0]), tr(&last[0]));
            return *this;
        }
    public:
        // insert
        Self&insert(size_type pos, const T* str) noexcept { return insert(pos, str, detail::strlen(str)); }
        // insert
        Self&insert(size_type pos, const T* str, size_type n) noexcept { return insert(pos, str, str + n); }
        // insert width RAI
        template<typename RAI> typename std::enable_if<type_helper::is_iterator<RAI>::value, Self&>::type
            insert(size_type pos, RAI first, RAI last) noexcept {
            detail::string_helper::string_insert(m_vector, pos, tr(&first[0]), tr(&last[0]));
            return *this;
        }
    public:
        // erase
        Self&erase(size_type pos = 0, size_type len = npos) noexcept { 
            detail::string_helper::string_erase(m_vector, pos, len);
            return *this;
        }
    public:
        // replace
        Self&replace(size_type pos, size_type len, const T* str) noexcept {
            return replace(pos, len, str, detail::strlen(str));
        }
        // replace
        Self&replace(size_type pos, size_type len, const T* str, size_type n) noexcept {
            return replace(pos, len, str, str + n);
        }
        // insert width RAI
        template<typename RAI> typename std::enable_if<type_helper::is_iterator<RAI>::value, Self&>::type
            replace(size_type pos, size_type len, RAI first, RAI last) noexcept {
            detail::string_helper::string_erase(m_vector, pos, len);
            detail::string_helper::string_insert(m_vector, pos, tr(&first[0]), tr(&last[0]));
            return *this;
        }
    public:
        // format
        template<typename... Args> bool format(const T fmt[], Args... args) noexcept {
            return detail::string_helper::string_format(
                m_vector, reinterpret_cast<const char*>(fmt), std::forward<Args>(args)...
            );
        }
        // splited type
        using splited_t = POD::Vector<View>;
#ifdef OPTIMIZATION_STRING_SPLIT_WITHIN_SINGLE_CHAR
        // split within single char
        inline auto split(T ch) const noexcept ->splited_t {
            return detail::string_helper::string_split(m_vector, ch);
        }
#else
        // split within single char
        inline auto split(const T& ch) const noexcept ->splited_t {
            splited_t v;
            detail::string_helper::string_split(m_vector, v, tr(&ch), tr(&ch + 1));
            return v;
        }
#endif
        // split within string
        auto split(const T* strbegin, const T* strend) const noexcept ->splited_t {
            splited_t v;
            detail::string_helper::string_split(m_vector, v, tr(strbegin), tr(strend));
            return v;
        }
        // split within view
        inline auto split(const View view) const noexcept ->splited_t {
            return this->split(view.begin(), view.end());
        }
        // split within string
        inline auto split(const T* str) const noexcept ->splited_t {
            return this->split(str, str + detail::strlen(str));
        }
    public:
        // operator==
        bool operator==(const Self&x) const noexcept {
            return size() == x.size() && !std::memcmp(c_str(), x.c_str(), size() * sizeof(T));
        }
        // operator!=
        bool operator!=(const Self&x) const noexcept {
            return size() != x.size() || std::memcmp(c_str(), x.c_str(), size() * sizeof(T));
        }
        // operator==
        bool operator==(const View x) const noexcept {
            return size() == x.size() && !std::memcmp(c_str(), x.begin(), size() * sizeof(T));
        }
        // operator!=
        bool operator!=(const View x) const noexcept {
            return size() != x.size() || std::memcmp(c_str(), x.begin(), size() * sizeof(T));
        }
    public:
        // operator+=
        Self&operator+=(const T x) noexcept { append(x); return *this; }
        // operator+=
        Self&operator+=(const T* x) noexcept { append(x); return *this; }
        // operator+=
        Self&operator+=(const View x) noexcept { append(x); return *this; }
        // operator+=
        Self&operator+=(const Self&x) noexcept { append(x); return *this; }
        // operator=
        Self&operator=(const Self&x) noexcept { m_vector = x.m_vector; return *this; }
        // operator=
        Self&operator=(View v) noexcept { return this->assign(v.begin(), v.end()); }
        // operator=
        Self&operator=(Self&&x) noexcept { m_vector = std::move(x.m_vector); return *this; }
        // operator[]
        auto operator[](size_t pos) noexcept ->T& { return m_vector[pos]; }
        // operator[]
        auto operator[](size_t pos) const noexcept ->const T&{ return m_vector[pos]; }
    public:
        // is ok?
        bool is_ok() const noexcept { return m_vector.is_ok(); }
        // view
        auto view() const noexcept ->View { return { c_str(), c_str() + length() }; }
        // length
        auto length() const noexcept ->size_type { return m_vector.size(); }
        // length
        auto size() const noexcept ->size_type { return m_vector.size(); }
        // clear data
        void clear() noexcept { m_vector.clear(); *m_vector.data() = 0; };
        // data string
        auto data() const noexcept -> const T*{ return m_vector.data(); };
        // c-style string
        auto c_str() const noexcept -> const T*{ return m_vector.data(); };
        // get capacity
        auto capacity() const noexcept -> size_type { return m_vector.capacity() - 1; }
        // get vector capacity
        auto vcapacity() const noexcept -> size_type { return m_vector.capacity(); }
        // fit memory
        void shrink_to_fit() noexcept { m_vector.shrink_to_fit(); }
        // reserve cap
        void reserve(size_type new_cap) noexcept { m_vector.reserve(new_cap); };
    protected:
        // pod vector
        V                   m_vector;
        // fixed buffer
        T                   m_buffer[B];
    };
    // basic string
    template<typename T, unsigned B, typename U> inline CUIBasicString<T, B>
    operator+(const CUIBasicString<T, B>& a, const U& b) noexcept {
        CUIBasicString<T, B> c{ a }; return c += b;
    }
    // string from utf-8
    template<typename T, unsigned B> inline
    auto CUIBasicString<T, B>::FromUtf8(const char* a,
        const char* b) noexcept -> CUIBasicString<T, B> {
        CUIBasicString<T, B> str;
        detail::string_helper::string_u8(T{}, str.m_vector, a, b);
        return str;
    }
}
