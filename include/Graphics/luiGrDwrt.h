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
#include "../../3rdParty/pugixml/pugixml.hpp"
#include <dwrite.h>
#include <d2d1_3.h>
#include <cstdint>

// longui::dx namespace, helper for directx
namespace LongUI { namespace DX {
    /// <summary>
    /// config for formating text
    /// </summary>
    /// <remarks> if your string more than 1K, do not use this</remarks>
    struct FormatTextConfig {
        // [in] basic text format
        IDWriteTextFormat*      format;
        // [in] text layout width
        float                   width;
        // [in] text layout hright
        float                   height;
        // [in] make the text showing progress, maybe you want
        // a "typing-effect", set 1.0f to show all, 0.0f to hide
        float                   progress;
        // [in] format for this
        RichType                rich_type;
        // [out] the text real(without format) length
        mutable uint16_t        text_length;
    };
    // format the text in C++ within core string
    auto FormatTextCoreC(const FormatTextConfig&, const wchar_t*, ...) noexcept ->IDWriteTextLayout*;
    // format the text within core string
    auto FormatTextCore(const FormatTextConfig&, const wchar_t*, va_list=nullptr) noexcept ->IDWriteTextLayout*;
    // format the text within xml string
    auto FormatTextXML(const FormatTextConfig&, const wchar_t*) noexcept ->IDWriteTextLayout*;
    // get default LongUI imp  IDWriteFontCollection
    auto CreateFontCollection(
        const wchar_t* filename = L"*.*tf",
        const wchar_t* folder = L"Fonts",
        bool include_system_font = true
    ) noexcept ->IDWriteFontCollection*;
    // create path-geometry from utf-32 char array using text format
    // fontface: (you can see <LongUI::UIScrollBar::UIScrollBar>)
    //          fontface == nullptr, ok but a bit slow
    //          *fontface == nullptr, ok, a bit slow, and out a IDWriteFontFace*, you can use it in next time(same format)
    //          *fontface != nullptr, ok
    auto CreateTextPathGeometry(
        IN const char32_t* utf32_string,
        IN uint32_t string_length,
        IN IDWriteTextFormat* format,
        IN ID2D1Factory* factory,
        IN OUT OPTIONAL IDWriteFontFace** fontface,
        OUT ID2D1PathGeometry** geometry
    ) noexcept ->HRESULT;
    // properties for creating text format, file data friendly
    struct TextFormatProperties {
        // text size
        float           size;
        // tab width, 0.f for default(size x 4)
        float           tab;
        // weight 0~1000 in uint16_t
        uint16_t        weight;
        // style 0~x in uint8_t
        uint8_t         style;
        // stretch 0~x in uint8_t
        uint8_t         stretch;
        // valign 0~x in uint8_t
        uint8_t         valign;
        // halign 0~x in uint8_t
        uint8_t         halign;
        // flow direction 0~x in uint8_t
        uint8_t         flow;
        // reading direction 0~x in uint8_t
        uint8_t         reading;
        // word wrapping
        uint32_t        wrapping;
        // font name, maybe use 0 instead of 4
        wchar_t         name[4];
    };
    // init TextFormatProperties
    void InitTextFormatProperties(TextFormatProperties& prop, size_t name_buf_len) noexcept;
    // create text format with properties
    auto CreateTextFormat(const TextFormatProperties&, IDWriteTextFormat** OUT fmt) noexcept ->HRESULT;
    // make text format with xml node, out "*fmt" maybe SAME as template_fmt if no changes
    auto MakeTextFormat(
        IN pugi::xml_node node, 
        OUT IDWriteTextFormat** fmt, 
        IN OPTIONAL IDWriteTextFormat* template_fmt = nullptr, 
        IN OPTIONAL const char* prefix=nullptr
    ) noexcept ->HRESULT;
}}