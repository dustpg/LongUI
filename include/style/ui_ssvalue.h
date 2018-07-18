#pragma once

// c++
#include <cstdint>
// ui
#include "ui_style_state.h"
#include "ui_ssvalue_list.h"
#include "../container/pod_vector.h"

namespace LongUI {
    // style sheet
    class CUIStyleSheet;
    // control
    class UIControl;
    // style sheet pointer
    using SSPtr = CUIStyleSheet * ;
    // stylesheets: values
    using SSValues = POD::Vector<SSValue>;
    // delete style sheet
    void DeleteStyleSheet(CUIStyleSheet* ptr) noexcept;
    // match style sheet
    void MatchStyleSheet(UIControl&, CUIStyleSheet* ptr) noexcept;
    // parse inline style
    bool ParseInlineStlye(SSValues&, U8View) noexcept;
    // stylesheets: value pc
    struct SSValuePC {
        // yes pseudo-classes
        StyleState      yes;
        // not pseudo-classes
        StyleState      noo;
    };
    // stylesheets: value pc with length
    struct SSValuePCL {
        // type of value
        ValueType       type;
        // length of data
        uint32_t        length;
        // yes pseudo-classes
        StyleState      yes;
        // not pseudo-classes
        StyleState      noo;
    };
    // SSValuePC = SSValue
    static_assert(sizeof(SSValuePCL) == sizeof(SSValue), "must be same");
    // Combinator
    enum Combinator : uint8_t {
        // Combinator None
        Combinator_None         = 0,
        // Adjacent sibling selectors   A + B
        Combinator_AdjacentSibling,
        // General sibling selectors    A ~ B
        Combinator_GeneralSibling,
        // Child selectors              A > B
        Combinator_Child,
        // Descendant selectors         A   B
        Combinator_Descendant,
    };
    // stylesheets: selector
    struct SSSelector {
        // next level 
        SSSelector*     next;
        // type selector 
        const char*     stype;
        // class selector
        const char*     sclass;
        // id selector
        const char*     sid;
        // pseudo class (index)
        SSValuePC       pc;
        // combinator
        Combinator      combinator;
    };
}