#pragma once
/**
* Copyright (c) 2014-2016 dustpg   mailto:dustpg@gmail.com
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

#include "../luibase.h"
#include "../luiconf.h"


// longui::helper namespace, xml helper
namespace LongUI { namespace Helper {
    // XMLGetValueEnum Properties
    struct GetEnumProperties {
        // values list of enum
        const char* const*  values_list;
        // length of 'values'
        uint32_t            values_length;
        // length of 'values'
        uint32_t            bad_match;
    };
    // get value string
    auto XMLGetValue(pugi::xml_node node, const char* attribute, const char* prefix =nullptr) noexcept -> const char*;
    // get value enum-int
    auto GetEnumFromString(const char* value, const GetEnumProperties& prop) noexcept ->uint32_t;
    // get longui richtype
    auto GetEnumFromString(const char* value, RichType bad_match) noexcept ->RichType;
    // get animation type
    auto GetEnumFromString(const char* value, AnimationType bad_match) noexcept ->AnimationType;
    // get Brush type
    auto GetEnumFromString(const char* value, BrushType bad_match) noexcept ->BrushType;
    // get longui bitmap render rule
    auto GetEnumFromString(const char* value, BitmapRenderRule bad_match) noexcept ->BitmapRenderRule;
    // get DWRITE_FONT_STYLE
    auto GetEnumFromString(const char* value, DWRITE_FONT_STYLE bad_match) noexcept ->DWRITE_FONT_STYLE;
    // get DWRITE_FONT_STRETCH
    auto GetEnumFromString(const char* value, DWRITE_FONT_STRETCH bad_match) noexcept ->DWRITE_FONT_STRETCH;
    // get DWRITE_FLOW_DIRECTION
    auto GetEnumFromString(const char* value, DWRITE_FLOW_DIRECTION bad_match) noexcept ->DWRITE_FLOW_DIRECTION;
    // get DWRITE_READING_DIRECTION
    auto GetEnumFromString(const char* value, DWRITE_READING_DIRECTION bad_match) noexcept ->DWRITE_READING_DIRECTION;
    // get DWRITE_WORD_WRAPPING
    auto GetEnumFromString(const char* value, DWRITE_WORD_WRAPPING bad_match) noexcept ->DWRITE_WORD_WRAPPING;
    // get DWRITE_PARAGRAPH_ALIGNMENT
    auto GetEnumFromString(const char* value, DWRITE_PARAGRAPH_ALIGNMENT bad_match) noexcept ->DWRITE_PARAGRAPH_ALIGNMENT;
    // get DWRITE_TEXT_ALIGNMENT
    auto GetEnumFromString(const char* value, DWRITE_TEXT_ALIGNMENT bad_match) noexcept ->DWRITE_TEXT_ALIGNMENT;
    // get d2d extend mode
    auto GetEnumFromString(const char* value, D2D1_EXTEND_MODE bad_match) noexcept ->D2D1_EXTEND_MODE;
    // get D2D1_TEXT_ANTIALIAS_MODE
    auto GetEnumFromString(const char* value, D2D1_TEXT_ANTIALIAS_MODE bad_match) noexcept ->D2D1_TEXT_ANTIALIAS_MODE;
    // get d2d interpolation mode
    auto GetEnumFromString(const char* value, D2D1_INTERPOLATION_MODE bad_match) noexcept ->D2D1_INTERPOLATION_MODE;
    // get CheckBoxState
    auto GetEnumFromString(const char* value, CheckBoxState bad_match) noexcept->CheckBoxState;
    //    ------------------------------------------------------
    //    ------------------ INLINE OVERLOAD -------------------
    //    ------------------------------------------------------
    // get animation type
    inline auto GetEnumFromXml(pugi::xml_node node, AnimationType bad_match,
        const char* attribute = "animationtype", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get animation type
    inline auto GetEnumFromXml(pugi::xml_node node, BrushType bad_match,
        const char* attribute = "type", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
        // get d2d interpolation mode
    inline auto GetEnumFromXml(pugi::xml_node node, D2D1_INTERPOLATION_MODE bad_match,
        const char* attribute = "interpolation", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get d2d extend mode
    inline auto GetEnumFromXml(pugi::xml_node node, D2D1_EXTEND_MODE bad_match,
        const char* attribute = "extend", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get longui bitmap render rule
    inline auto GetEnumFromXml(pugi::xml_node node, BitmapRenderRule bad_match,
        const char* attribute = "rule", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get longui richtype
    inline auto GetEnumFromXml(pugi::xml_node node, RichType bad_match,
        const char* attribute = "richtype", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get DWRITE_FONT_STYLE
    inline auto GetEnumFromXml(pugi::xml_node node,  DWRITE_FONT_STYLE bad_match,
        const char* attribute = "style", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get DWRITE_FONT_STRETCH
    inline auto GetEnumFromXml(pugi::xml_node node, DWRITE_FONT_STRETCH bad_match,
        const char* attribute = "stretch", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get DWRITE_FLOW_DIRECTION
    inline auto GetEnumFromXml(pugi::xml_node node, DWRITE_FLOW_DIRECTION bad_match,
        const char* attribute = "flowdirection", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get DWRITE_READING_DIRECTION
    inline auto GetEnumFromXml(pugi::xml_node node, DWRITE_READING_DIRECTION bad_match,
        const char* attribute = "readingdirection", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get DWRITE_READING_DIRECTION
    inline auto GetEnumFromXml(pugi::xml_node node, DWRITE_WORD_WRAPPING bad_match,
        const char* attribute = "wordwrapping", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get DWRITE_PARAGRAPH_ALIGNMENT
    inline auto GetEnumFromXml(pugi::xml_node node, DWRITE_PARAGRAPH_ALIGNMENT bad_match,
        const char* attribute = "valign", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get DWRITE_PARAGRAPH_ALIGNMENT
    inline auto GetEnumFromXml(pugi::xml_node node, DWRITE_TEXT_ALIGNMENT bad_match,
        const char* attribute = "align", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get d2d text anti-mode
    inline auto GetEnumFromXml(pugi::xml_node node, D2D1_TEXT_ANTIALIAS_MODE bad_match,
        const char* attribute = LongUI::XmlAttribute::WindowTextAntiMode, const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
    // get animation type
    inline auto GetEnumFromXml(pugi::xml_node node, CheckBoxState bad_match,
        const char* attribute = "checkstate", const char* prefix = nullptr) noexcept {
        return GetEnumFromString(Helper::XMLGetValue(node, attribute, prefix), bad_match);
    }
}}
