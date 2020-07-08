// LUI
#include <control/ui_viewport.h>
#include <input/ui_kminput.h>
#include <core/ui_platform.h>
#include <core/ui_manager.h>
#include <core/ui_window.h>
// C/C++
#include <cassert>
// error beep
extern "C" void longui_error_beep() noexcept;

/// <summary>
/// 
/// </summary>
/// <returns></returns>
LongUI::CUIPlatform::~CUIPlatform() noexcept {
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
LongUI::CUIPlatform::CUIPlatform() noexcept {
}

/// <summary>
/// Mouses the event.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUIPlatform::DoMouseEventTs(const MouseEventArg & args) noexcept {
    CUIDataAutoLocker locker;
    UIControl* ctrl = this->captured;
    if (!ctrl) ctrl = this->viewport();
    // TODO: 处理返回值
    ctrl->DoMouseEvent(args);
}


/// <summary>
/// Closes the popup.
/// </summary>
/// <returns></returns>
void LongUI::CUIPlatform::ClosePopup() noexcept {
    if (this->popup) {
        // 递归关闭
        this->popup->ClosePopup();
        auto& sub = this->popup->RefViewport();
        sub.HosterPopupEnd();
        this->viewport()->SubViewportPopupEnd(sub, this->popup_type);
        this->popup->CloseWindow();
        this->popup = nullptr;
    }
}


/// <summary>
/// Toggles the access key display.
/// </summary>
/// <returns></returns>
void LongUI::CUIPlatform::toggle_access_key_display() noexcept {
    auto& key = this->access_key_display; key = !key;
    const EventArg arg{ NoticeEvent::Event_ShowAccessKey, key };
    for (auto ctrl : this->access_key_map) if (ctrl)
        ctrl->DoEvent(nullptr, arg);
}



// c
extern "C" {
    // char16 -> char32
    char32_t impl_char16x2_to_char32(char16_t lead, char16_t trail) noexcept;
}

/// <summary>
/// Called when [character].
/// </summary>
/// <param name="ch">The ch.</param>
/// <returns></returns>
void LongUI::CUIPlatform::OnChar(char16_t ch) noexcept {
    char32_t ch32;
    // UTF16 第一字
    if (Unicode::IsHighSurrogate(ch)) { this->saved_utf16 = ch; return; }
    // UTF16 第二字
    else if (Unicode::IsLowSurrogate(ch)) {
        ch32 = impl_char16x2_to_char32(this->saved_utf16, ch);
        this->saved_utf16 = 0;
    }
    // UTF16 仅一字
    else ch32 = static_cast<char32_t>(ch);
    // 处理输入
    this->OnCharTs(ch32);
}

/// <summary>
/// Called when [character].
/// </summary>
/// <param name="ch">The ch.</param>
/// <returns></returns>
void LongUI::CUIPlatform::OnCharTs(char32_t ch) noexcept {
    // IME输入优化
    if (this->ime_count)--this->ime_count;
    // TODO: 自己检查有效性?
    if ((ch >= 0x20 && ch != 0x7f) || ch == '\t') {
        // 直接将输入引导到焦点控件
        if (const auto focused_ctrl = this->focused) {
            CUIDataAutoLocker locker;
            focused_ctrl->DoInputEvent({
                LongUI::InputEvent::Event_Char, this->ime_count, ch
                });
        }
    }
    else assert(this->ime_count == 0 && "IME cannot input control char");
}


/// <summary>
/// Called when [hot key].
/// </summary>
/// <param name="i">The index.</param>
/// <returns></returns>
void LongUI::CUIPlatform::OnAccessKey(uintptr_t i) noexcept {
    // 存在弹出窗口就指向弹出窗口
    if (this->popup) {
        // TOOLTIP不算
        if (this->popup_type != PopupType::Type_Tooltip) {
            const auto popprivate = this->popup->pimpl();
            popprivate->OnAccessKey(i);
            return;
        }
    }
    // 正式处理
    CUIDataAutoLocker locker;
    const auto ctrl = this->access_key_map[i];
    if (ctrl && ctrl->IsEnabled() && ctrl->IsVisibleEx()) {
        ctrl->DoEvent(nullptr, { NoticeEvent::Event_DoAccessAction, 0 });
    }
    else ::longui_error_beep();
}

/// <summary>
/// Called when [dpi changed].
/// </summary>
/// <param name="zoom">The zoom parameter.</param>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::CUIPlatform::OnDpiChanged(Size2F zoom, const RectL& rect) noexcept {
    // dpi改变了
    if (UIManager.flag & ConfigureFlag::Flag_NoAutoScaleOnHighDpi) return;
    //float xdpi = float(uint16_t(LOWORD(wParam)));
    //float ydpi = float(uint16_t(HIWORD(wParam)));
    //float x = xdpi / float(LongUI::BASIC_DPI);
    //float y = ydpi / float(LongUI::BASIC_DPI);
    const auto x = zoom.width;
    const auto y = zoom.height;
    CUIDataAutoLocker locker;
    auto& vp = *this->viewport();
    auto& window = vp.RefWindow();
    // 固定大小应该需要缩放窗口
    if (window.config & CUIWindow::Config_FixedSize /*|| true*/) {
        vp.JustResetZoom(x, y);
        window.SetAbsoluteRect(rect);
    }
    // 不定大小应该懒得弄了
    else {
        vp.JustResetZoom(x, y);
        const auto fw = static_cast<float>(this->rect.width);
        const auto fh = static_cast<float>(this->rect.height);
        this->resize_viewport({ fw, fh });
    }
}

/// <summary>
/// called when [menu key]
/// </summary>
/// <returns></returns>
void LongUI::CUIPlatform::OnMenuTs() noexcept {
    CUIDataAutoLocker locker;
    // 有popup就关闭
    if (this->popup) this->ClosePopup();
    // 没有就开关ACCESSKEY显示
    else this->toggle_access_key_display();
}

/// <summary>
/// Called when [key down/up].
/// </summary>
/// <param name="vk">The vk.</param>
/// <returns></returns>
void LongUI::CUIPlatform::OnKey(InputEventArg arg) noexcept {
    // 直接将输入引导到焦点控件
    if (const auto focused_ctrl = this->focused) {
        assert(focused_ctrl->IsEnabled());
        // 检查输出
        const auto rv = focused_ctrl->DoInputEvent(arg);
        if (rv != Event_Ignore) return;
    }
    // 如果无视事件则进行特殊处理
    switch (arg.character)
    {
    case CUIInputKM::KB_ESCAPE:
        // 检查释放Esc关闭窗口
        if (this->viewport()->RefWindow().config
            & CUIWindow::Config_EscToCloseWindow)
            return this->viewport()->RefWindow().CloseWindow();
        break;
    case CUIInputKM::KB_RETURN:
        // 回车键: 直接将输入引导到默认控件
        if (const auto defc = this->now_default) {
            if (defc->IsEnabled()) defc->DoInputEvent(arg);
            return;
        }
        break;
#if 0
    case CUIInputKM::KB_SPACE:
        if (const auto nowfocus = this->focused) {
            if (nowfocus->IsEnabled()) nowfocus->DoInputEvent({ ekey, 0, key });
            return;
        }
        break;
#endif
    case CUIInputKM::KB_TAB:
        // Tab键: 聚焦上/下键盘焦点
        this->draw_focus = true;
        if (arg.event == InputEvent::Event_KeyUp) {
            if (CUIInputKM::GetKeyState(CUIInputKM::KB_SHIFT))
                this->viewport()->RefWindow().FocusPrev();
            else
                this->viewport()->RefWindow().FocusNext();
        }
        return;
    }
}
