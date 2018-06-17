#pragma once

// c++
#include <cstdint>
// ui
#include "ui_style_state.h"
#include "ui_ssvalue_list.h"
#include "../container/pod_vector.h"

namespace LongUI {
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
    // stylesheets: block
    struct SSBlock {
        // window hosted value buffer, unknown-terminated values
        SSValue*        values;
        // yes pseudo-classes
        StyleState      yes;
        // not pseudo-classes
        StyleState      noo;
    };
    // stylesheets: selector
    struct SSSelector {
        // id - unique pointer in window
        const char*     idn;
        // class - unique pointer in program
        const char*     cln;
        // element - unique pointer in program
        const char*     ele;
        // index of value
        uint16_t        idx;
        // length of value
        uint16_t        len;
    };
    // stylesheets: blocks
    using SSBlocks = POD::Vector<SSBlock>;
    // stylesheets: values
    using SSValues = POD::Vector<SSValue>;
    // stylesheets: selectors
    using SSSelectors = POD::Vector<SSSelector>;
    // parse stylesheet, return false if failed to parse
    bool ParseStylesheet(const char*, SSValues&, SSSelectors&) noexcept;
}