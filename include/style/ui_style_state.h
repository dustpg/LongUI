#pragma once

// c++
#include <cstdint>

namespace LongUI {
#if 0
    // typeof StyleState
    enum class StyleStateType : uint8_t {
        Type_None = 0, // for fire event
        Type_Selected,
        Type_Default,
        Type_Disabled,
        Type_Hover,
        Type_Active,
        Type_Focus,
        Type_Checked,
        Type_Indeterminate,
        Type_Closed,

        //Type_OddIndex,
        //Type_Current,
        Type_NA_TabAfterSelectedTab,

        //Type_Opening,
        //Type_Idle1,
        //Type_Idle2,
        //Type_Ending,

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
        // reserved flag, index 0 : used for fire event
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
        // closed   [typical: twisty]
        bool        closed      : 1;

        // odd index
        //bool        odd_index   : 1;
        // current 
        //bool        current     : 1;
        // after selected tab
        bool        after_seltab : 1;

        // opening 
        //bool        opening     : 1;
        //// idle1
        //bool        idle1       : 1;
        //// idle2
        //bool        idle2       : 1;
        // ending
        //bool        ending      : 1;

        // first child
        //bool        first_child  : 1;
        // last child
        //bool        last_child   : 1;
    };
#endif
    // typeof StyleState
    enum StyleStateIndex : uint32_t {
        Index_Selected,
        Index_Default,
        Index_Disabled,
        Index_Hover,
        Index_Active,
        Index_Focus,
        Index_Checked,
        Index_Indeterminate,
        Index_Closed,



        Index_NA_TabAfterSelectedTab,
    };
    // style state
    enum StyleState : uint32_t {
        // [non]
        State_Non           = 0,
        // [all]
        State_All           = uint32_t(-1),
        // [selected]
        State_Selected      = 1 << Index_Selected,
        // [default]
        State_Default       = 1 << Index_Default,
        // [disabled]       [inherited]
        State_Disabled      = 1 << Index_Disabled,
        // [hover]
        State_Hover         = 1 << Index_Hover,
        // [active]
        State_Active        = 1 << Index_Active,
        // [focus]
        State_Focus         = 1 << Index_Focus,
        // [checked]
        State_Checked       = 1 << Index_Checked,
        // [indeterminate]
        State_Indeterminate = 1 << Index_Indeterminate,
        // [closed]
        State_Closed        = 1 << Index_Closed,


        // [N.A.]
        State_TAST          = 1 << Index_NA_TabAfterSelectedTab,



        // [mouse cut inherited]
        State_MouseCutInher = State_All,
    };
    // operator |
    inline constexpr StyleState operator|(StyleState a, StyleState b) noexcept {
        using state_t = uint32_t;
        static_assert(sizeof(state_t) == sizeof(a), "bad type");
        return StyleState(state_t(a) | state_t(b));
    }
    // operator &
    inline constexpr StyleState operator&(StyleState a, StyleState b) noexcept {
        using state_t = uint32_t;
        static_assert(sizeof(state_t) == sizeof(a), "bad type");
        return StyleState(state_t(a) & state_t(b));
    }
    // operator ^
    inline constexpr StyleState operator^(StyleState a, StyleState b) noexcept {
        using state_t = uint32_t;
        static_assert(sizeof(state_t) == sizeof(a), "bad type");
        return StyleState(state_t(a) ^ state_t(b));
    }
    // operator ~
    inline constexpr StyleState operator~(StyleState a) noexcept {
        using state_t = uint32_t;
        static_assert(sizeof(state_t) == sizeof(a), "bad type");
        return StyleState(~state_t(a));
    }
    // state change
    struct StyleStateChange { 
        // state mask
        StyleState      state_mask;
        // state to change
        StyleState      state_change;
    };
    // detail namespace
    namespace impl {
        // is_same state
        template<StyleState Mask> inline auto is_same(StyleState a, StyleState b) noexcept {
            return !((a ^ b) & Mask);
        }
        // is_different state
        template<StyleState Mask> inline auto is_different(StyleState a, StyleState b) noexcept {
            return (a ^ b) & Mask;
        }
        // is_different state
        template<StyleState Mask> inline auto is_any(StyleState a, StyleState b) noexcept {
            return (a | b) & Mask;
        }
    }
}
