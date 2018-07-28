#pragma once
/**
* Copyright (c) 2014-2018 dustpg   mailto:dustpg@gmail.com
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

// accessible
#include "ui_accessible.h"
#include "../core/ui_core_type.h"
// int
#include "../typecheck/int_by_size.h"

// c++
#include <cstdint>

// ui namespace
namespace LongUI {
#ifdef LUI_ACCESSIBLE
    // | operator
    inline auto operator|(AccessiblePattern a, AccessiblePattern b) noexcept {
        using pattrten_t = typename type_helper::int_type<sizeof(a)>::unsigned_t;
        return static_cast<AccessiblePattern>(static_cast<pattrten_t>(a) | b);
    }
    // & operator
    inline auto operator&(AccessiblePattern a, AccessiblePattern b) noexcept {
        using pattrten_t = typename type_helper::int_type<sizeof(a)>::unsigned_t;
        return static_cast<AccessiblePattern>(static_cast<pattrten_t>(a) & b);
    }
    // |= operator
    inline auto&operator|=(AccessiblePattern& a, AccessiblePattern b) noexcept {
        a = a | b; return a;
    }
    // &= operator
    inline auto&operator&=(AccessiblePattern& a, AccessiblePattern b) noexcept {
        a = a & b; return a;
    }
    // accessible type;
    enum class AccessibleControlType : uint32_t;
    // accessible event: for patterns
    enum AccessibleEvent : uint32_t {
        // get patterns
        Event_GetPatterns = 0,
        // all-get control type
        Event_All_GetControlType,
        // all-get accessible name
        Event_All_GetAccessibleName,
        // all-get description
        Event_All_GetDescription,
        // invoke-invoke
        Event_Invoke_Invoke,
        // toggle-toggle
        Event_Toggle_Toggle,
        // value-set value(string)
        Event_Value_SetValue,
        // value-get value(string)
        Event_Value_GetValue,
        // range&value-read only? return Event_Accept if true
        Event_RangeValue_IsReadOnly,
        // range-get value(double)
        Event_Range_GetValue,
        // range-set value(double)
        Event_Range_SetValue,
        // range-get max value
        Event_Range_GetMax,
        // range-set max value
        Event_Range_GetMin,
        // range-get large step
        Event_Range_GetLargeStep,
        // range-set small step
        Event_Range_GetSmallStep,
    };
    // accessible event args
    struct AccessibleEventArg {
        // event id
        AccessibleEvent     event;
    };
    // accessible event: get patterns
    struct AccessibleGetPatternsArg : AccessibleEventArg {
        // <out> patterns
        mutable AccessiblePattern   patterns;
        // ctor
        AccessibleGetPatternsArg() noexcept {
            this->event = AccessibleEvent::Event_GetPatterns;
            this->patterns = AccessiblePattern::Pattern_None;
        }
    };
    // accessible event: get control type
    struct AccessibleGetCtrlTypeArg : AccessibleEventArg {
        // <out> name
        mutable AccessibleControlType   type;
        // ctor
        AccessibleGetCtrlTypeArg() noexcept {
            this->event = AccessibleEvent::Event_All_GetControlType;
            type = AccessibleControlType(-1);
        }
    };
    // accessible event: get accessible name
    struct AccessibleGetAccNameArg : AccessibleEventArg {
        // <out> name
        CUIString*          name;
        // ctor
        AccessibleGetAccNameArg(CUIString& str) noexcept {
            this->event = AccessibleEvent::Event_All_GetAccessibleName;
            name = &str;
        }
    };
    // accessible event: get description
    struct AccessibleGetDescriptionArg : AccessibleEventArg {
        // <out> name
        CUIString*          description;
        // ctor
        AccessibleGetDescriptionArg(CUIString& str) noexcept {
            this->event = AccessibleEvent::Event_All_GetDescription;
            description = &str;
        }
    };

    // accessible event: set value
    struct AccessibleVSetValueArg : AccessibleEventArg {
        // string pointer
        const char16_t* string;
        // string length
        uint32_t        length;
        // ctor
        AccessibleVSetValueArg(const char16_t* str, uint32_t len) noexcept {
            this->event = AccessibleEvent::Event_Value_SetValue;
            string = str;
            length = len;
        }
    };
    // accessible event: get value
    struct AccessibleVGetValueArg : AccessibleEventArg {
        // <out> name
        CUIString*          value;
        // ctor
        AccessibleVGetValueArg(CUIString& str) noexcept {
            this->event = AccessibleEvent::Event_Value_GetValue;
            value = &str;
        }
    };


    // accessible event: set range value
    struct AccessibleRSetValueArg : AccessibleEventArg {
        // value
        double          value;
        // ctor
        AccessibleRSetValueArg(double v) noexcept {
            this->event = AccessibleEvent::Event_Range_SetValue;
            value = v;
        }
    };
    // accessible event: get range value
    struct AccessibleRGetValueArg : AccessibleEventArg {
        // <out> name
        mutable double  value;
        // ctor
        AccessibleRGetValueArg() noexcept {
            this->event = AccessibleEvent::Event_Range_GetValue;
            value = 0.;
        }
    };
    // accessible event: get range max value
    struct AccessibleRGetMaxArg : AccessibleEventArg {
        // <out> name
        mutable double  value;
        // ctor
        AccessibleRGetMaxArg() noexcept {
            this->event = AccessibleEvent::Event_Range_GetMax;
            value = 0.;
        }
    };
    // accessible event: get range min value
    struct AccessibleRGetMinArg : AccessibleEventArg {
        // <out> name
        mutable double  value;
        // ctor
        AccessibleRGetMinArg() noexcept {
            this->event = AccessibleEvent::Event_Range_GetMin;
            value = 0.;
        }
    };
    // accessible event: get range large-step value
    struct AccessibleRGetLargeStepArg : AccessibleEventArg {
        // <out> name
        mutable double  value;
        // ctor
        AccessibleRGetLargeStepArg() noexcept {
            this->event = AccessibleEvent::Event_Range_GetLargeStep;
            value = 0.;
        }
    };
    // accessible event: get range small-step value
    struct AccessibleRGetSmallStepArg : AccessibleEventArg {
        // <out> name
        mutable double  value;
        // ctor
        AccessibleRGetSmallStepArg() noexcept {
            this->event = AccessibleEvent::Event_Range_GetSmallStep;
            value = 0.;
        }
    };
#endif
}