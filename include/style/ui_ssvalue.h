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
    // style sheet pointer
    using SSPtr = CUIStyleSheet * ;
    /// <summary>
    /// value of style sheet
    /// </summary>
    struct SSValue {
        // type of value
        ValueType       type;
        // unit of value
        //ValueUnit       unit;
        // union
        union {
            // u32 data
            uint32_t    u32;
            // i32 data
            int32_t     i32;
            // single float data
            float       single;
            // byte data
            uint8_t     byte;
            // boolean data
            bool        boolean;
        };
    };
#if 0
    // stylesheets: block
    struct SSBlock {
        // window hosted value buffer, unknown-terminated values
        SSValue*        values;
        // yes pseudo-classes
        StyleState      yes;
        // not pseudo-classes
        StyleState      noo;
    };
#endif
    // Combinator
    enum Combinator : uint8_t {
        // Combinator None
        Combinator_None         = 0,
        // Adjacent sibling selectors   A + B
        Combinator_AdjacentSibling,
        // General sibling selectors    A ~ B
        Combinator_General,
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
        StyleStateType  pseudocl;
        // combinator
        Combinator      combinator;
    };
    // stylesheets: values
    using SSValues = POD::Vector<SSValue>;
}