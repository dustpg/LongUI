#pragma once

#include <cstdint>

// if you insure xml correct, simpax won't check error EXCEPT OOM
//#define SAX_NO_ERROR_CHECK

//define your own __restrict if not support "__restrict"
//#define __restrict

//#define SAX_API _declspec(dllexport)
#define SAX_API

#define SAX_PURE_METHOD = 0

namespace SimpAX {
    // char type, utf-8 as default
    using Char = char; // wchar_t char16_t char32_t
    // string pair
    struct StrPair { 
        // pair
        const Char* a, *b; 
        // begin
        auto begin() const noexcept { return a; }
        // end
        auto end() const noexcept { return b; }
    };
    // stack element
    struct StackEle {
        // string pair
        StrPair             pair;
        // user pointer
        void*               user_ptr;
        // user data
        size_t              user_data;
    };
    // is same
    SAX_API bool IsSame(const StrPair a, const StrPair b) noexcept;
    // StrPair == StrPair
    inline bool operator ==(const StrPair a, const StrPair b) noexcept { return IsSame(a, b); }
    // StrPair != StrPair
    inline bool operator !=(const StrPair a, const StrPair b) noexcept { return !IsSame(a, b); }
    // Result
    struct Result { 
        // error code enum
        enum Code : uint32_t {
            // ok
            Code_OK = 0,
            // internal error
            Code_InternalError,
            // out of memory
            Code_OOM,
            // bad pi
            Code_BadPI,
            // bad comment
            Code_BadComment,
            // bad element
            Code_BadElement,
            // mismatched element
            Code_Mismatched,
            // syntax error
            Code_SyntaxError,
            // erro code
        }               code;
        // error position
        uint32_t        pos; 
        // is ok
        bool IsOk() const noexcept { return code == Code_OK; }
    };
    /// <summary>
    /// documen
    /// </summary>
    class CAXStream {
    public:
        // fixed stack length
        enum { FIXED_STACK_LENGTH = 12 };
        // Processing Instruction
        struct PIs { StrPair target, instructions; };
        // attributes
        struct ATTRs { StrPair key, value; };
        // ctor
        SAX_API CAXStream() noexcept;
        // dtor
        SAX_API ~CAXStream() noexcept;
        // load string
        SAX_API auto Load(const Char* str) noexcept ->Result;
    private:
        // free
        static void free(void*) noexcept;
        // memory alloc
        static void*malloc(size_t) noexcept;
        // re-alloc
        static void*realloc(void* ptr, size_t) noexcept;
    private:
        // add Processing Instruction
        virtual void add_processing(const PIs& attr) noexcept SAX_PURE_METHOD;
        // begin element
        virtual void begin_element(const StrPair tag) noexcept SAX_PURE_METHOD;
        // end element
        virtual void end_element(const StrPair tag) noexcept SAX_PURE_METHOD;
        // add attribute
        virtual void add_attribute(const ATTRs& attr) noexcept SAX_PURE_METHOD;
        // add comment
        virtual void add_comment(const StrPair ) noexcept SAX_PURE_METHOD;
        // add text, maybe add more once like <a>A<b/>B</a>
        virtual void add_text(const StrPair) noexcept SAX_PURE_METHOD;
    protected:
        // find first namespace
        SAX_API static void find_1st_namespace(StrPair& pair) noexcept;
        // try get value in instructions
        SAX_API static bool try_get_instruction_value(const Char* key, StrPair& ins) noexcept;
        // element stack begin
        auto stack_begin() noexcept { return m_pStackBase; }
        // element stack end
        auto stack_end() noexcept { return m_pStackTop; }
#ifdef NDEBUG
        // top element
        auto stack_top() noexcept->StackEle& { return m_pStackTop[-1]; }
#else
        // top element
        SAX_API auto stack_top() noexcept->StackEle&;
#endif
    private:
        // interpret escape
        void interpret_escape(StrPair&) noexcept;
        // check ok
        bool is_stack_ok() const noexcept { return !!m_pStackBase; }
        // push element
        bool push(StrPair str);
        // pop element
        void pop();
        // grow up
        void grow_up();
    private:
        // escape buffer
        Char*           m_pEscapeBuffer = nullptr;
        // escape buffer end
        Char*           m_pEscapeBufferEnd = nullptr;
        // stack base
        StackEle*       m_pStackBase = m_aStackBuffer;
        // stack top len
        StackEle*       m_pStackTop = m_aStackBuffer;
        // stack top cap
        StackEle*       m_pStackCap = m_aStackBuffer + FIXED_STACK_LENGTH;
        // fixed buffer
        StackEle        m_aStackBuffer[FIXED_STACK_LENGTH];
    };
}