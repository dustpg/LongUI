#pragma once

// c++
#include <cstdint>

namespace LongUI {
    // typeof StyleState
    enum class StyleStateType : uint8_t {
        Type_None = 0,
        Type_Selected,
        Type_Default,
        Type_Disabled,
        Type_Hover,
        Type_Active,
        Type_Focus,
        Type_Checked,
        Type_Indeterminate,
        Type_Closed,

        Type_OddIndex,
        //Type_Current,
        Type_NA_TabAfterSelectedTab,

        Type_Opening,
        //Type_Idle1,
        //Type_Idle2,
        Type_Ending,

        //Type_NA_FirstChild,
        //Type_NA_LastChild,
    };
    // state change
    struct StyleStateTypeChange { StyleStateType type; bool change; };
    // style state
    struct alignas(uint32_t) StyleState {
        // ctor
        void Init() noexcept;
        // change, return true if changed
        bool Change(StyleStateTypeChange) noexcept;
        // reserved flag, index 0
        bool        none        : 1;
        // ui-selected
        bool        selected    : 1;
        // default command control
        bool        default5    : 1;
        // disable
        bool        disabled    : 1;
        // hover
        bool        hover       : 1;
        // active, higher than hover
        bool        active      : 1;
        // [space to trigger] focus
        bool        focus       : 1;
        // checked
        bool        checked     : 1;
        // indeterminate, higher than checked
        bool        indeterminate : 1;
        // closed   [twisty]
        bool        closed      : 1;

        // odd index
        bool        odd_index   : 1;
        // current 
        //bool        current     : 1;
        // after selected tab
        bool        after_seltab : 1;

        // opening 
        bool        opening     : 1;
        //// idle1
        //bool        idle1       : 1;
        //// idle2
        //bool        idle2       : 1;
        // ending
        bool        ending      : 1;

        // first child
        //bool        first_child  : 1;
        // last child
        //bool        last_child   : 1;
    };
}