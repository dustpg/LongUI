#include <Windows.h>
#include <input/ui_kminput.h>
#include <util/ui_ostype.h>
#include <cassert>
#include <cstring>

/// <summary>
/// Gets the state of the key.
/// </summary>
/// <param name="key">The key.</param>
/// <returns></returns>
bool LongUI::CUIInputKM::GetKeyState(KB key) noexcept {
#ifdef LUI_RAWINPUT

#endif
    return (::GetKeyState(key) & 0x80) != 0;
}
#ifdef LUI_RAWINPUT

/// <summary>
/// Initializes a new instance of the <see cref="CUIInputKM"/> class.
/// </summary>
LongUI::CUIInputKM::CUIInputKM() noexcept { 

};

/// <summary>
/// Initializes the specified .
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
auto LongUI::CUIInputKM::Init(HWND hwnd) noexcept ->Result {
    assert(hwnd && "bad argument");
    // 注册鼠标原始输入
    RAWINPUTDEVICE rid[2];
    rid[0].usUsagePage = 0x01; 
    rid[0].usUsage = 0x02; 
    rid[0].dwFlags = RIDEV_INPUTSINK;
    rid[0].hwndTarget = (hwnd);
    rid[1].usUsagePage = 0x01; 
    rid[1].usUsage = 0x06; 
    rid[1].dwFlags = RIDEV_INPUTSINK;
    rid[1].hwndTarget = (hwnd);
    const auto rc = ::RegisterRawInputDevices(rid, 2, sizeof(rid[0]));
    // TODO: WinCode2HRESULT
    return{ rc ? Result::RS_OK : Result::RE_FAIL };
    //return rc ? S_OK : LongUI::WinCode2HRESULT(::GetLastError());
}


// LongUI::impl namespace
namespace LongUI { namespace impl {
    // log2 for 2powed number
    template<size_t n>struct log2 { enum : size_t { value = log2<n / 2>::value + 1 }; };
    // for 0
    template<> struct log2<1> { enum : size_t { value = 0 }; };
}}

#if 0
CUIString str;
str.Format(
    L"Mouse: usFlags=%04x ulButtons=%04x usButtonFlags=%04x "
    L"usButtonData=%04x ulRawButtons=%04x lLastX=%04x lLastY"
    L"=%04x ulExtraInformation=%04x\r\n", 
    raw->data.mouse.usFlags, 
    raw->data.mouse.ulButtons, 
    raw->data.mouse.usButtonFlags, 
    raw->data.mouse.usButtonData, 
    raw->data.mouse.ulRawButtons, 
    raw->data.mouse.lLastX, 
    raw->data.mouse.lLastY, 
    raw->data.mouse.ulExtraInformation
);
::OutputDebugStringW(str.c_str());
#endif

/// <summary>
/// Updates the specified .
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::CUIInputKM::Update(HRAWINPUT hRawInput) noexcept {
    alignas(RAWINPUT) char buffer[sizeof(RAWINPUT) * 5];
    UINT size = sizeof(buffer);
    auto code = ::GetRawInputData(
        (hRawInput), RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER)
    );
    // 发生错误
    if (code == UINT(-1)) { assert(!"ERROR"); return; }
    auto* raw = reinterpret_cast<RAWINPUT*>(buffer);
    // 鼠标消息
    if (raw->header.dwType == RIM_TYPEMOUSE) {
        const auto flags = raw->data.mouse.usButtonFlags;
        enum : DWORD {
            // left button
            F_LtDn = RI_MOUSE_LEFT_BUTTON_DOWN,
            F_LtUp = RI_MOUSE_LEFT_BUTTON_UP,
            F_LtDnShiftR = impl::log2<F_LtDn>::value,
            F_LtUpShiftR = impl::log2<F_LtUp>::value,
            F_LtShiftL = impl::log2<MB::MB_L>::value,
            // right button
            F_RtDn = RI_MOUSE_RIGHT_BUTTON_DOWN,
            F_RtUp = RI_MOUSE_RIGHT_BUTTON_UP,
            F_RtDnShiftR = impl::log2<F_RtDn>::value,
            F_RtUpShiftR = impl::log2<F_RtUp>::value,
            F_RtShiftL = impl::log2<MB::MB_R>::value,
            // middle button
            F_MdDn = RI_MOUSE_MIDDLE_BUTTON_DOWN,
            F_MdUp = RI_MOUSE_MIDDLE_BUTTON_UP,
            F_MdDnShiftR = impl::log2<F_MdDn>::value,
            F_MdUpShiftR = impl::log2<F_MdUp>::value,
            F_MdShiftL = impl::log2<MB::MB_M>::value,
            // x button 1
            F_X1Dn = RI_MOUSE_BUTTON_4_DOWN,
            F_X1Up = RI_MOUSE_BUTTON_4_UP,
            F_X1DnShiftR = impl::log2<F_X1Dn>::value,
            F_X1UpShiftR = impl::log2<F_X1Up>::value,
            F_X1ShiftL = impl::log2<MB::MB_X1>::value,
            // x button 2
            F_X2Dn = RI_MOUSE_BUTTON_5_DOWN,
            F_X2Up = RI_MOUSE_BUTTON_5_UP,
            F_X2DnShiftR = impl::log2<F_X2Dn>::value,
            F_X2UpShiftR = impl::log2<F_X2Up>::value,
            F_X2ShiftL = impl::log2<MB::MB_X2>::value,
        };
        if (flags) {
            // 鼠标左键状态
            const uint8_t left1 = (flags & F_LtDn) << F_LtShiftL >> F_LtDnShiftR;
            const uint8_t left2 = (flags & F_LtUp) << F_LtShiftL >> F_LtUpShiftR;
            // 鼠标右键状态
            const uint8_t rght1 = (flags & F_RtDn) << F_RtShiftL >> F_RtDnShiftR;
            const uint8_t rght2 = (flags & F_RtUp) << F_RtShiftL >> F_RtUpShiftR;
            // 鼠标中键状态
            const uint8_t midd1 = (flags & F_MdDn) << F_MdShiftL >> F_MdDnShiftR;
            const uint8_t midd2 = (flags & F_MdUp) << F_MdShiftL >> F_MdUpShiftR;
            // 鼠标X1键状态
            const uint8_t x1__1 = (flags & F_X1Dn) << F_X1ShiftL >> F_X1DnShiftR;
            const uint8_t x1__2 = (flags & F_X1Up) << F_X1ShiftL >> F_X1UpShiftR;
            // 鼠标X2键状态
            const uint8_t x2__1 = (flags & F_X2Dn) << F_X2ShiftL >> F_X2DnShiftR;
            const uint8_t x2__2 = (flags & F_X2Up) << F_X2ShiftL >> F_X2UpShiftR;
            // 带入计算按中状态
            m_bufMButton[MBI_ThisFrame] |= (left1 | rght1 | midd1 | x1__1 | x2__1);
            m_bufMButton[MBI_ThisFrame] &= ~(left2 | rght2 | midd2 | x1__2 | x2__2);
        }
    }
    // 键盘消息
    else if (raw->header.dwType == RIM_TYPEKEYBOARD)  {
        KEYBOARD_OVERRUN_MAKE_CODE;

        raw->data.keyboard.MakeCode, 
            raw->data.keyboard.Flags, 
            raw->data.keyboard.Reserved, 
            raw->data.keyboard.ExtraInformation, 
            raw->data.keyboard.Message, 
            raw->data.keyboard.VKey;

        const auto key = raw->data.keyboard.VKey;
        m_pKeyState[key] = !(raw->data.keyboard.Flags & RI_KEY_BREAK);
    }
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUIInputKM::AfterUpdate() noexcept {
    // 获取鼠标位置
    ::GetCursorPos(reinterpret_cast<POINT*>(&m_ptMouseL));
    m_ptMouse.x = static_cast<float>(m_ptMouseL.x);
    m_ptMouse.y = static_cast<float>(m_ptMouseL.y);
    // 更新手柄按键
    VK_LSHIFT;
    // 保存上帧输入
    m_bufMButton[MBI_LastFrame] = m_bufMButton[MBI_ThisFrame];
    // 保存上帧输入
    std::memcpy(m_pKeyStateOld, m_pKeyState, KEYBOARD_BUFFER_SIZE);
    m_bufMButton[MBI_LastFrame] = m_bufMButton[MBI_ThisFrame];
}

#endif
