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

#include <guiddef.h>
#include "../util/ui_unimacro.h"
#include "../core/ui_basic_type.h"
#include "../core/ui_core_type.h"

// ui namespace
namespace LongUI {
    // UI Ref-Based Interface
    struct PCN_NOVTABLE IUIRefCount {
        // add ref count
        virtual auto UNICALL AddRef() noexcept->uint32_t = 0;
        // release
        virtual auto UNICALL Release() noexcept->uint32_t = 0;
    };
    // UI Command Interface
    /*struct PCN_NOVTABLE IUICommand {
        // 
        virtual ~IUICommand() noexcept = 0;
        // undo
        virtual void Undo() noexcept = 0;
        // redo
        virtual void Redo() noexcept = 0;
    };*/
    // text formatter interface
    /*struct PCN_NOVTABLE IUITextFormatter : IUIInterface {
        /// <summary>
        /// Customs the type of the rich.
        /// </summary>
        /// <param name="config">The configuration.</param>
        /// <param name="format">The format.</param>
        /// <remarks>if in RichType::Type_Custom, will call this</remarks>
        /// <returns></returns>
        //virtual auto CustomRichType(const DX::FormatTextConfig& config, const wchar_t* format) noexcept->IDWriteTextLayout* = 0;
        /// <summary>
        /// create inline img interface.
        /// </summary>
        /// <param name="img">The img.</param>
        /// <returns></returns>
        //virtual auto XmlImgInterface(const DX::InlineImage& img) noexcept->IDWriteInlineObject* = 0;
        /// /// <summary>
        /// eval string for xml formatting.
        /// </summary>
        /// <param name="pair">The pair.</param>
        /// <returns></returns>
        virtual auto XmlEvalString(StringPair pair) noexcept->StringPair = 0;
        /// <summary>
        /// Frees the string(from XmlEvalString).
        /// </summary>
        /// <param name="pair">The pair.</param>
        /// <returns></returns>
        virtual void XmlFreeString(StringPair pair) noexcept = 0;
    };*/
    // ui res loader
    /*class PCN_NOVTABLE IUIResourceLoader : public IUIInterface {
    public:
        // resource type
        enum ResourceType : uint32_t {
            Type_Null = 0,       // none resource
            Type_Bitmap,        // bitmap within ID2D1Bitmap1*
            Type_Brush,         // brush with ID2D1Brush*
            Type_TextFormat,    // text format within IDWriteTextFormat*
            Type_Meta,          // meta within LongUI::Meta
            RESOURCE_TYPE_COUNT,// count of this
        };
    public:
        // get resouce count with type
        virtual auto GetResourceCount(ResourceType type) const noexcept->size_t = 0;
        // get resouce by index, index in range [0, count),  for Type_Bitmap, Type_Brush, Type_TextFormat
        virtual auto GetResourcePointer(ResourceType type, size_t index) noexcept ->void* = 0;
        // get meta by index, index in range [0, count)
        //virtual void GetMeta(size_t index, DeviceIndependentMeta&) noexcept = 0;
    };*/
}

