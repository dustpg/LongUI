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
        void SetState(ControlState state) noexcept { m_pColor = this->color + state; };
        // real length
        auto GetRealLength() const noexcept { return m_config.text_length; }
        // operator = for wide-char(utf16 on windows)
        auto&operator=(const wchar_t* str) noexcept { m_text = str; this->RecreateLayout(); return *this; }
        // operator = for utf-8,
        auto&operator=(const char* str) noexcept { m_text = str; this->RecreateLayout(); return *this; }
        // get string
        auto&GetString() noexcept { return m_text; }
        // c_str for stl-like
        auto c_str() const noexcept { return m_text.c_str(); }
    public:
        // get layout
        auto GetLayout() const noexcept { return LongUI::SafeAcquire(m_pLayout); }
        // set layout
        auto SetLayout(IDWriteTextLayout* layout) noexcept {
            assert(layout);
            assert(m_config.rich_type == RichType::Type_None && "set layout must be Type_None mode");
            LongUI::SafeRelease(m_pLayout);
            m_pLayout = LongUI::SafeAcquire(layout);
        }
        // set new size
        auto Resize(float w, float h) noexcept {
            m_config.width = w; m_config.height = h;
            m_pLayout->SetMaxWidth(w); m_pLayout->SetMaxHeight(h);
        }
        // set new progress
        auto SetNewProgress(float p) noexcept { 
            m_config.progress = p; 
            return this->RecreateLayout();
        }
        // render it
        auto Render(float x, float y) const noexcept {
            m_pTextRenderer->basic_color.color = *m_pColor;
            m_pLayout->Draw(m_buffer.GetDataVoid(), m_pTextRenderer, x, y);
        }
    public:
        // state color
        D2D1_COLOR_F                color[STATE_COUNT];
    private:
        // layout of it
        IDWriteTextLayout*          m_pLayout = nullptr;
        // Text Renderer
        XUIBasicTextRenderer*       m_pTextRenderer = nullptr;
        // basic color
        D2D1_COLOR_F*               m_pColor = this->color + State_Normal;
        // the text config
        FormatTextConfig            m_config;
        // the string of text
        CUIString                   m_text;
        // context buffer for text renderer
        ContextBuffer               m_buffer;
    };
}}