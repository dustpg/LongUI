#pragma once

#include <cstdint>

// attribute selector? obj[attr="sb"]
//#define SAC_ATTRIBUTE_SELECTOR
// pure virtual ?
#define SAC_PURE_VIRTUAL


// simpac namespace
namespace SimpAC {
    // char type, utf-8 as default
    using Char = char; // wchar_t char16_t char32_t
    // string pair
    struct StrPair {
        // pair
        const Char* first, *second;
        // begin
        auto begin() const noexcept { return first; }
        // end
        auto end() const noexcept { return second; }
    };
    // func type
    enum FuncType : uint16_t;
    // func value
    struct alignas(void*) FuncValue {
        // string begin pointer
        const Char*     first;
        // string length max in 65,535
        uint16_t        length;
        // type of func
        FuncType        func;
    };
    // split unit
    auto SplitUnit(StrPair& pair) noexcept->StrPair;
    // Combinators
    enum Combinators : uint32_t;
    // Combinators
    enum BasicSelectors : uint32_t;
    // pure virtual ?
#ifdef SAC_PURE_VIRTUAL
#define SAC_PURE_VIRTUAL_SUFFIX = 0
#else
#define SAC_PURE_VIRTUAL_SUFFIX
#endif
    /// <summary>
    /// document
    /// </summary>
    class CACStream {
    public:
        // ctor
        CACStream() noexcept {}
        // dtor
        ~CACStream() noexcept {}
        // load string
        void Load(StrPair) noexcept;
    protected:
        // state for combinator
        enum class combinator_state;
        // state for css-parser
        enum class css_state : unsigned;
        // start to parse coment
        bool parse_comment(StrPair& view) noexcept;
        // start to parse selector at level-1
        auto parse_selector_lv1(char, combinator_state&) noexcept->css_state;
    private:
        // add a comment
        virtual void add_comment(StrPair) noexcept SAC_PURE_VIRTUAL_SUFFIX;
        // add a selector
        virtual void add_selector(BasicSelectors, StrPair) noexcept SAC_PURE_VIRTUAL_SUFFIX;
        // add a selector-combinator
        virtual void add_selector_combinator(Combinators) noexcept SAC_PURE_VIRTUAL_SUFFIX;
        // add a comma under selector
        virtual void add_selector_comma() noexcept SAC_PURE_VIRTUAL_SUFFIX;
        // begin properties {
        virtual void begin_properties() noexcept SAC_PURE_VIRTUAL_SUFFIX;
        // end properties }
        virtual void end_properties() noexcept SAC_PURE_VIRTUAL_SUFFIX;
        // add a property
        virtual void begin_property(StrPair) noexcept SAC_PURE_VIRTUAL_SUFFIX;
        // add a value
        virtual void add_value(StrPair) noexcept SAC_PURE_VIRTUAL_SUFFIX;
        // add a function value
        virtual void add_func_value(FuncValue value, StrPair raw_func) noexcept SAC_PURE_VIRTUAL_SUFFIX;
#ifdef SAC_ATTRIBUTE_SELECTOR
        // add a attribute selector
        virtual void add_attribute_selector(BasicSelectors, StrPair, StrPair) noexcept SAC_PURE_VIRTUAL_SUFFIX;
#endif 
    protected:
    };
    // Combinators
    enum Combinators : uint32_t {
        // Adjacent sibling selectors   A + B
        Combinators_AdjacentSibling = 0,
        // General sibling selectors    A ~ B
        Combinators_GeneralSibling,
        // Child selectors              A > B
        Combinators_Child,
        // Descendant selectors         A   B
        Combinators_Descendant,
    };
    // Basic Selectors
    enum BasicSelectors : uint32_t {
        // Type selectors               elementname
        Selectors_Type = 0,
        // Class selectors              .classname
        Selectors_Class,
        // ID selectors                 #idname
        Selectors_Id,
        // Universal selectors          *
        Selectors_Universal,
        // Pseudo classed selectors     :nth-child(2)
        Selectors_PseudoClass,
        // Pseudo elements selectors    ::maker
        Selectors_PseudoElement,
#ifdef SAC_ATTRIBUTE_SELECTOR
        // Attribute selectors: set     E[foo]
        Selectors_AttributeSet,
        // Attribute selectors: Exact   E[foo="bar"]
        Selectors_AttributeExact,
        // Attribute selectors: List    E[foo~="bar"]
        Selectors_AttributeList,
        // Attribute selectors: Hyphen  E[foo|="bar"]
        Selectors_AttributeHyphen,
        // Attribute selectors: Contain E[foo*="bar"]
        Selectors_AttributeContain,
        // Attribute selectors: Begin   E[foo^="bar"]
        Selectors_AttributeBegin,
        // Attribute selectors: End     E[foo&="bar"]
        Selectors_AttributeEnd,
#endif
    };
    // func type: see also SimpAC::FUNC_HASH_LIST
    enum FuncType : uint16_t {
        Type_None = 0,
        Type_Attr,              // attr()
        Type_Calc,              // calc()
        Type_CubicBezier,       // cubic-bezier()
        Type_Hsl,               // hsl()
        Type_Hsla,              // hsla()
        Type_LinearGradient,    // linear-gradient()
        Type_RadialGradient,    // radial-gradient()
        Type_ReLinearGradient,  // repeating-linear-gradient()
        Type_ReRadialGradient,  // repeating-radial-gradient()
        Type_Rgb,               // rgb()
        Type_Rgba,              // rgba()
        Type_Url,               // url()
        Type_Var,               // var()
        Type_Unknown,           // unknown
    };
}