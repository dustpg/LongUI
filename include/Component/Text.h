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
#include "../Platless/luiPlEzC.h"
#include "../LongUI/luiUiTxtRdr.h"
#include "../Graphics/luiGrDwrt.h"

// longui::component namespace
namespace LongUI { namespace Component {
    // component: short text
    class ShortText {
    public:
        // recreate layout
        void RecreateLayout() noexcept;
        // constructor
        ShortText() noexcept;
        // initizlize with xml-node
        void Init(pugi::xml_node, const char* prefix = "text") noexcept;
        // initizlize without xml-node
        void Init() noexcept;
        // destructor
        ~ShortText() noexcept;
        // set color
        void SetState(ControlState state) noexcept { m_pColor = this->color + state; }
        // get color
        const auto*GetColor() const noexcept { return m_pColor; }
        // real length
        auto GetRealLength() const noexcept { return m_config.text_length; }
        // operator = for wide-char(utf16 on windows)
        auto&operator=(const wchar_t* str) noexcept { m_text = str; this->RecreateLayout(); return *this; }
        // get string
        auto&GetString() noexcept { return m_text; }
        // get string
        const auto&GetString() const noexcept { return m_text; }
        // c_str for stl-like
        auto c_str() const noexcept { return m_text.c_str(); }
        // get layout
        auto GetLayout() const noexcept { return LongUI::SafeAcquire(m_pLayout); }
        // set new progress
        void SetNewProgress(float p) noexcept { m_config.progress = p; this->RecreateLayout(); }
    public:
        // set layout
        auto SetLayout(IDWriteTextLayout* layout) noexcept;
        // set new size
        void Resize(float w, float h) noexcept;
        // render it
        void Render(ID2D1RenderTarget* target, D2D1_POINT_2F) const noexcept;
        // get text box
        void GetTextBox(RectLTWH_F& rect) const noexcept;
    public:
        // state color
        D2D1_COLOR_F                color[STATE_COUNT];
        // text render offset
        D2D1_POINT_2F               offset = D2D1_POINT_2F{0.f};
    private:
        // layout of it
        IDWriteTextLayout*          m_pLayout = nullptr;
        // Text Renderer
        XUIBasicTextRenderer*       m_pTextRenderer = nullptr;
        // text context
        void*                       m_pTextContext = nullptr;
        // basic color
        D2D1_COLOR_F*               m_pColor = this->color + State_Normal;
        // the text config
        DX::FormatTextConfig        m_config;
        // the string of text
        CUIString                   m_text;
    };
}}