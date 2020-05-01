#pragma once
/**
* Copyright (c) 2014-2020 dustpg   mailto:dustpg@gmail.com
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

#include "../core/ui_basic_type.h"
#include "../util/ui_ostype.h"

// ui namespace
namespace LongUI {
    // input of ui for keyboard/mouse
    class CUIInputKM final {
    public:
        // mouse button index
        enum MB : uint32_t { 
            MB_L = 1 << 0, // Left-Button 
            MB_R = 1 << 1, // Right-Button
            MB_M = 1 << 2, // Middle-Buttom
            MB_X1= 1 << 3, // X1-Button
            MB_X2= 1 << 4, // X2-Button
        };
        // keyboard button index
        enum KB : uint32_t { 
            KB_BACK           = 0X08, // BACK
            KB_TAB            = 0X09, // TAB
            KB_CLEAR          = 0X0C, // CLEAR
            KB_RETURN         = 0X0D, // RETURN
            KB_SHIFT          = 0X10, // SHIFT
            KB_CONTROL        = 0X11, // CTRL
            KB_MENU           = 0X12, // MENU
            KB_PAUSE          = 0x13, 
            KB_CAPITAL        = 0x14, 
            KB_KANA           = 0x15, 
            KB_HANGUL         = 0x15, 
            KB_JUNJA          = 0x17, 
            KB_FINAL          = 0x18, 
            KB_HANJA          = 0x19, 
            KB_KANJI          = 0x19, 
            KB_ESCAPE         = 0x1B, 
            KB_CONVERT        = 0x1C, 
            KB_NONCONVERT     = 0x1D, 
            KB_ACCEPT         = 0x1E, 
            KB_MODECHANGE     = 0x1F, 
            KB_SPACE          = 0x20, 
            KB_PRIOR          = 0x21, 
            KB_NEXT           = 0x22, 
            KB_END            = 0x23, 
            KB_HOME           = 0x24, 
            KB_LEFT           = 0x25, 
            KB_UP             = 0x26, 
            KB_RIGHT          = 0x27, 
            KB_DOWN           = 0x28, 
            KB_SELECT         = 0x29, 
            KB_PRINT          = 0x2A, 
            KB_EXECUTE        = 0x2B, 
            KB_SNAPSHOT       = 0x2C, 
            KB_INSERT         = 0x2D, 
            KB_DELETE         = 0x2E, 
            KB_HELP           = 0x2F, 
            KB_A              = 'A',
            KB_B              = 'B',
            KB_C              = 'C',
            KB_D              = 'D',
            KB_E              = 'E',
            KB_F              = 'F',
            KB_G              = 'G',
            KB_H              = 'H',
            KB_I              = 'I',
            KB_J              = 'J',
            KB_K              = 'K',
            KB_L              = 'L',
            KB_M              = 'M',
            KB_N              = 'N',
            KB_O              = 'O',
            KB_P              = 'P',
            KB_Q              = 'Q',
            KB_R              = 'R',
            KB_S              = 'S',
            KB_T              = 'T',
            KB_U              = 'U',
            KB_V              = 'V',
            KB_W              = 'W',
            KB_X              = 'X',
            KB_Y              = 'Y',
            KB_Z              = 'Z',
#ifdef LUI_LRKEY_NOT_WORKING
            KB_LWIN           = 0x5B, 
            KB_RWIN           = 0x5C, 
#endif
            KB_APPS           = 0x5D, 
            KB_SLEEP          = 0x5F, 
            KB_NUMPAD0        = 0x60, 
            KB_NUMPAD1        = 0x61, 
            KB_NUMPAD2        = 0x62, 
            KB_NUMPAD3        = 0x63, 
            KB_NUMPAD4        = 0x64, 
            KB_NUMPAD5        = 0x65, 
            KB_NUMPAD6        = 0x66, 
            KB_NUMPAD7        = 0x67, 
            KB_NUMPAD8        = 0x68, 
            KB_NUMPAD9        = 0x69, 
            KB_MULTIPLY       = 0x6A, 
            KB_ADD            = 0x6B, 
            KB_SEPARATOR      = 0x6C, 
            KB_SUBTRACT       = 0x6D, 
            KB_DECIMAL        = 0x6E, 
            KB_DIVIDE         = 0x6F, 
            KB_F1             = 0x70, 
            KB_F2             = 0x71, 
            KB_F3             = 0x72, 
            KB_F4             = 0x73, 
            KB_F5             = 0x74, 
            KB_F6             = 0x75, 
            KB_F7             = 0x76, 
            KB_F8             = 0x77, 
            KB_F9             = 0x78, 
            KB_F10            = 0x79, 
            KB_F11            = 0x7A, 
            KB_F12            = 0x7B, 
            KB_F14            = 0x7D, 
            KB_F15            = 0x7E, 
            KB_F16            = 0x7F, 
            KB_F17            = 0x80, 
            KB_F18            = 0x81, 
            KB_F19            = 0x82, 
            KB_F20            = 0x83, 
            KB_F21            = 0x84, 
            KB_F22            = 0x85, 
            KB_F23            = 0x86, 
            KB_F24            = 0x87, 
            KB_NUMLOCK        = 0x90, 
            KB_SCROLL         = 0x91, 
            KB_OEM_NEC_EQUAL  = 0x92,    // '=' key on numpad
            KB_OEM_FJ_JISHO   = 0x92,    // 'Dictionary' key
            KB_OEM_FJ_MASSHOU = 0x93,    // 'Unregister word' key
            KB_OEM_FJ_TOUROKU = 0x94,    // 'Register word' key
            KB_OEM_FJ_LOYA    = 0x95,    // 'Left OYAYUBI' key
            KB_OEM_FJ_ROYA    = 0x96,    // 'Right OYAYUBI' key
#ifdef LUI_LRKEY_NOT_WORKING
            KB_LSHIFT         = 0xA0, 
            KB_RSHIFT         = 0xA1, 
            KB_LCONTROL       = 0xA2, 
            KB_RCONTROL       = 0xA3, 
            KB_LMENU          = 0xA4, 
            KB_RMENU          = 0xA5, 
#endif
            KB_BROWSER_BACK        = 0xA6, 
            KB_BROWSER_FORWARD     = 0xA7, 
            KB_BROWSER_REFRESH     = 0xA8, 
            KB_BROWSER_STOP        = 0xA9, 
            KB_BROWSER_SEARCH      = 0xAA, 
            KB_BROWSER_FAVORITES   = 0xAB, 
            KB_BROWSER_HOME        = 0xAC, 
            KB_VOLUME_MUTE         = 0xAD, 
            KB_VOLUME_DOWN         = 0xAE, 
            KB_VOLUME_UP           = 0xAF, 
            KB_MEDIA_NEXT_TRACK    = 0xB0, 
            KB_MEDIA_PREV_TRACK    = 0xB1, 
            KB_MEDIA_STOP          = 0xB2, 
            KB_MEDIA_PLAY_PAUSE    = 0xB3, 
            KB_LAUNCH_MAIL         = 0xB4, 
            KB_LAUNCH_MEDIA_SELECT = 0xB5, 
            KB_LAUNCH_APP1         = 0xB6, 
            KB_LAUNCH_APP2         = 0xB7, 
            KB_OEM_1          = 0xBA,    // ';:' for US
            KB_OEM_PLUS       = 0xBB,    // '+' any country
            KB_OEM_COMMA      = 0xBC,    // ',' any country
            KB_OEM_MINUS      = 0xBD,    // '-' any country
            KB_OEM_PERIOD     = 0xBE,    // '.' any country
            KB_OEM_2          = 0xBF,    // '/?' for US
            KB_OEM_3          = 0xC0,    // '`~' for US
            KB_OEM_AX         = 0xE1,   //  'AX' key on Japanese AX kbd
            KB_OEM_102        = 0xE2,   //  "<>" or "\|" on RT 102-key kbd.
            KB_ICO_HELP       = 0xE3,   //  Help key on ICO
            KB_ICO_00         = 0xE4,   //  00 key on ICO
            KB_OEM_RESET      = 0xE9, 
            KB_OEM_JUMP       = 0xEA, 
            KB_OEM_PA1        = 0xEB, 
            KB_OEM_PA2        = 0xEC, 
            KB_OEM_PA3        = 0xED, 
            KB_OEM_WSCTRL     = 0xEE, 
            KB_OEM_CUSEL      = 0xEF, 
            KB_OEM_ATTN       = 0xF0, 
            KB_OEM_FINISH     = 0xF1, 
            KB_OEM_COPY       = 0xF2, 
            KB_OEM_AUTO       = 0xF3, 
            KB_OEM_ENLW       = 0xF4, 
            KB_OEM_BACKTAB    = 0xF5, 
            KB_ATTN           = 0xF6, 
            KB_CRSEL          = 0xF7, 
            KB_EXSEL          = 0xF8, 
            KB_EREOF          = 0xF9, 
            KB_PLAY           = 0xFA, 
            KB_ZOOM           = 0xFB, 
            KB_NONAME         = 0xFC, 
            KB_PA1            = 0xFD, 
            KB_OEM_CLEAR      = 0xFE, 
        };
        // the buffer size of kerboard
        enum : uint32_t {  KEYBOARD_BUFFER_SIZE = 256 } ;
    private:
        // mouse button index
        enum MBI : uint32_t {
            MBI_ThisFrame= 0,   // this frame data
            MBI_LastFrame,      // last frame data
            MBI_SIZE            // buffer count
        };
    public:
        // get instance
        static auto Instance() noexcept->CUIInputKM&;
        // get key state
        static bool GetKeyState(KB) noexcept;
#ifdef LUI_RAWINPUT
        // ctor
        CUIInputKM() noexcept;
        // clear mouse state
        void ClearMouseState() noexcept { 
            m_bufMButton[MBI_ThisFrame] = 0; 
            m_bufMButton[MBI_LastFrame] = 0; 
        }
        // init with tool window
        auto Init(HWND) noexcept ->Result;
        // get mouse position
        auto GetMousePosition() const noexcept { return m_ptMouse; };
        // get mouse position/long
        auto GetMousePositionL() const noexcept { return m_ptMouseL; };
        // is mouse button pressed?
        auto IsMbPressed(MB b) const noexcept { return !!(m_bufMButton[MBI_ThisFrame] & b); }
        // is mouse button down?
        auto IsMbDown(MB b) const noexcept { return (m_bufMButton[MBI_ThisFrame] & b) != 0 && (m_bufMButton[MBI_LastFrame] & b) == 0; }
        // is mouse button up?
        auto IsMbUp(MB b) const noexcept { return (m_bufMButton[MBI_ThisFrame] & b) == 0 && (m_bufMButton[MBI_LastFrame] & b) != 0; }
        // is keyboard pressed?
        auto IsKbPressed(KB b) const noexcept { return m_pKeyState[b]; }
        // is keyboard down?
        auto IsKbDown(KB b) const noexcept { return m_pKeyState[b] && m_pKeyStateOld[b] == 0; }
        // is keyboard down?
        auto IsKbUp(KB b) const noexcept { return m_pKeyState[b] == 0 && m_pKeyStateOld[b] != 0;  }
    public:
        // update, impl @ UIUtil.cpp
        void AfterUpdate() noexcept;
        // update with HRAWINPUT
        void Update(HRAWINPUT) noexcept;
    protected:
        // mouse postion
        Point2F         m_ptMouse{0.f, 0.f};
        // mouse postion
        Point2L         m_ptMouseL{0, 0};
        // mouse button buffer
        char            m_bufMButton[MBI_SIZE];
        // unused
        char            m_unused_u8x6[MBI_SIZE * 3];
        // KeyState for this frame
        bool*           m_pKeyState = m_abKeyStateBuffer;
        // KeyState for last frame
        bool*           m_pKeyStateOld = m_abKeyStateBuffer + KEYBOARD_BUFFER_SIZE;
        // key state buffer
        bool            m_abKeyStateBuffer[KEYBOARD_BUFFER_SIZE * 2] = {0};
#endif
    };
}
