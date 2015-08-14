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
    // Component namespace
    namespace Component {
        // Component: ShortText
        class ShortText {
            using InfoUIBasicTextRenderer = Helper::InfomationPointer<CUIBasicTextRenderer>;
            // 重建
            void __fastcall recreate(const char* = nullptr) noexcept;
        public:
            // set is xml
            auto SetIsXML(bool b) noexcept { m_pTextRenderer.SetBool1(b); }
            // set is rich
            auto SetIsRich(bool b) noexcept { m_pTextRenderer.SetBool2(b); }
            // get is xml
            auto GetIsXML() const noexcept { return m_pTextRenderer.Bool1(); }
            // set is rich
            auto GetIsRich() const noexcept { return m_pTextRenderer.Bool2(); }
        public:
            // constructor
            ShortText(pugi::xml_node, const char* prefix = "text") noexcept;
            // set new size
            auto SetNewSize(float w, float h) noexcept {
                m_config.width = w; m_config.height = h;
                m_pLayout->SetMaxWidth(w); m_pLayout->SetMaxHeight(h);
            }
            // set new progress
            auto SetNewProgress(float p) {
                m_config.progress = p;
                this->recreate();
            }
            // render it
            auto Render(float x, float y) const noexcept {
                auto ptr = m_pTextRenderer.Ptr();
                ptr->basic_color.color = m_basicColor;
                m_pLayout->Draw(m_buffer.data, ptr, x, y);
            }
            // destructor
            ~ShortText() noexcept;
            // operator = for wide-char(utf16 on windows), must be in core-mode
            auto operator=(const wchar_t*) noexcept ->ShortText&;
            // operator = for utf-8, can be xml-mode or core-mode
            auto operator=(const char*) noexcept ->ShortText&;
            // c_str for wide-char(utf16 on windows)
            LongUIInline auto text_totallegnth() const noexcept { return m_config.text_length; }
            // c_str for wide-char(utf16 on windows)
            LongUIInline auto c_str() const noexcept { return m_text.c_str(); }
            // c_str for utf-8
            void c_str(char* b) const noexcept { b[LongUI::WideChartoUTF8(m_text.c_str(), b)] = 0; }
        private:
            // layout of it
            IDWriteTextLayout*          m_pLayout = nullptr;
            // Text Renderer
            InfoUIBasicTextRenderer     m_pTextRenderer;
            // the text config
            FormatTextConfig            m_config;
            // basic color
            D2D1_COLOR_F                m_basicColor;
            // the string of text
            CUIString                   m_text;
            // context buffer for text renderer
            EzContainer::ContextBuffer  m_buffer;
        };
    }
}