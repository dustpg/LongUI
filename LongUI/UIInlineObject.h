#pragma once
/**
* Copyright (c) 2014-2015 dustpg   mailto:dustpg@gmail.com
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

// longui namespace
namespace LongUI {
    // LongUI Inline Object
    class UIInlineObject : public  ComBase<QiList<IDWriteInlineObject>> {
    public:
        // inline obj type
        enum InlineType : size_t {
            // CUIInlineMeta
            Type_Meta = 0,
            // CUIRubyCharacter
            Type_Ruby,
            // Unknown
        };
        // type: 
        InlineType  const   type;
        // ctor
        UIInlineObject(InlineType _type) noexcept: type(_type) {}
    };
    // Meta inlineobject ver.
    class  CUIInlineMeta final : public UIInlineObject {
        using Super = UIInlineObject;
    public:
        // UIInlineMeta
        CUIInlineMeta(): Super(UIInlineObject::Type_Meta){}
        // ~UIInlineMeta
        ~CUIInlineMeta() {}
    public:
        // meta to render
        Meta                meta;
    };
    // Ruby Character
    class  CUIRubyCharacter final : public UIInlineObject {
        using Super = UIInlineObject;
    public:
        // ctor's context
        struct CtorContext {
            // basic format
            IDWriteTextFormat*      basic_format;
            // ruby format
            IDWriteTextFormat*      ruby_format;
            // basic text
            const wchar_t*          basic_text;
            // ruby text
            const wchar_t*          ruby_text;
            // basic text length
            size_t                  basic_text_length;
            // ruby text length
            size_t                  ruby_text_length;
        };
        // ctor
        CUIRubyCharacter(const CtorContext&) noexcept;
        // ~dtor
        ~CUIRubyCharacter() noexcept;
    public: // impl for IDWriteInlineObject
        HRESULT STDMETHODCALLTYPE Draw(void*, IDWriteTextRenderer*, FLOAT, FLOAT, BOOL, BOOL, IUnknown*) noexcept override;
        // draw this
        HRESULT STDMETHODCALLTYPE GetMetrics(DWRITE_INLINE_OBJECT_METRICS*) noexcept override;
        // draw this
        HRESULT STDMETHODCALLTYPE GetOverhangMetrics(DWRITE_OVERHANG_METRICS*) noexcept override;
        // draw this
        HRESULT STDMETHODCALLTYPE GetBreakConditions(DWRITE_BREAK_CONDITION*, DWRITE_BREAK_CONDITION*) noexcept override;
    private:
        // base chars layout
        IDWriteTextLayout*              m_pBaseLayout = nullptr;
        // ruby chars
        IDWriteTextLayout*              m_pRubyLayout = nullptr;
        // inline metrics
        DWRITE_INLINE_OBJECT_METRICS    m_inlineMetrics;
        // overhand metrics
        DWRITE_OVERHANG_METRICS         m_overhandMetrics;
    };
}