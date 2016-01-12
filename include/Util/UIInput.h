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
    // input of ui
    class CUIInput final {
    public:
        // the buffer size of kerboard
        enum : DWORD {  KEYBOARD_BUFFER_SIZE = 256 } ;
    public:
        // ctor
        CUIInput() noexcept { m_ptMouseL = { 0, 0 }; std::memset(m_abKeyStateBuffer, 0, sizeof(m_abKeyStateBuffer)); };
        // get mouse position
        auto GetMousePosition() const noexcept { return m_ptMouse; };
        // get mouse position/long
        auto GetMousePositionL() const noexcept { return m_ptMouseL; };
        // is key pressed?
        auto IsKeyPressed(uint32_t index) const noexcept { return m_pKeyState[index] != 0; }
        // is key down?
        auto IsKeyDown(uint32_t index) const noexcept { return m_pKeyState[index] != 0 && m_pKeyStateOld[index] == 0; }
        // is key down?
        auto IsKeyUp(uint32_t index) const noexcept { return m_pKeyState[index] == 0 && m_pKeyStateOld[index] != 0;  }
    public:
        // update, impl @ UIUtil.cpp
        auto Update() noexcept {
            ::GetCursorPos(&m_ptMouseL);
            m_ptMouse.x = static_cast<float>(m_ptMouseL.x);
            m_ptMouse.y = static_cast<float>(m_ptMouseL.y);
            std::swap(m_pKeyState, m_pKeyStateOld);
            (void)::GetKeyboardState(m_pKeyState);
        }
    protected:
        // mouse postion
        D2D1_POINT_2F           m_ptMouse = D2D1::Point2F();
        // mouse postion
        POINT                   m_ptMouseL = POINT();
        // KeyState for now
        uint8_t*                m_pKeyState = m_abKeyStateBuffer;
        // KeyState for last frame
        uint8_t*                m_pKeyStateOld = m_abKeyStateBuffer + KEYBOARD_BUFFER_SIZE;
        // key state buffer
        uint8_t                 m_abKeyStateBuffer[KEYBOARD_BUFFER_SIZE*2];
    };
}
