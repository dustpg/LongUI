// LUI
#include <core/ui_string_view.h>
#include <control/ui_viewport.h>
#include <core/ui_color_list.h>
#include <input/ui_kminput.h>
#include <core/ui_platform.h>
#include <core/ui_manager.h>
#include <core/ui_window.h>
// private
#include "../private/ui_private_control.h"
// C/C++
#include <cassert>
#include <algorithm>

// LongUI::impl
namespace LongUI { namespace impl {
#if 0
    // get dpi scale
    auto get_dpi_scale_from_hwnd(HWND hwnd) noexcept -> Size2F;
    // get subpixcel rendering level
    void get_subpixel_text_rendering(uint32_t&) noexcept;
    // eval script for window
    void eval_script_for_window(U8View view, CUIWindow* window) noexcept {
        assert(window && "eval script but no window");
        UIManager.Evaluation(view, *window);
    }
    
#endif
    // mark two rect
    inline auto mark_two_rect_dirty(
        const RectF& a,
        const RectF& b,
        const Size2F size,
        RectF* output
    ) noexcept {
        const auto is_valid = [size](const RectF& rect) noexcept {
            return rect.bottom > rect.top
                && rect.right > rect.left
                && rect.right > 0.f
                && rect.bottom > 0.f
                && rect.left < size.width
                && rect.top < size.height
                ;
        };
        // 判断有效性
        const auto av = is_valid(a);
        const auto bv = is_valid(b);
        // 都无效
        if (!av && !bv) return output;
        // 默认处理包含仅有A有效的情况
        RectF merged_rect = a;
        // 都有效
        if (av && bv) {
            // 存在交集
            if (LongUI::IsOverlap(a, b)) {
                merged_rect.top = std::min(a.top, b.top);
                merged_rect.left = std::min(a.left, b.left);
                merged_rect.right = std::max(a.right, b.right);
                merged_rect.bottom = std::max(a.bottom, b.bottom);
            }
            // 没有交集
            else { *output = b; ++output; }
        }
        // 只有B有效
        else if (bv) merged_rect = b;
        // 标记
        *output = merged_rect; ++output;
        return output;
    }
}}

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
LongUI::CUIPlatform::CUIPlatform() noexcept : titlename(u"LUI"_sv) {
    clear_color = ColorF::FromRGBA_CT<RGBA_TianyiBlue>();
    caret_color = ColorF::FromRGBA_CT<RGBA_Black>();
    std::memset(access_key_map, 0, sizeof(access_key_map));
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
/// Marks the dirty rect.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::CUIPlatform::MarkDirtyRect(const DirtyRect& rect) noexcept {
    auto& counter = this->dirty_count_recording;
    assert(counter <= LongUI::DIRTY_RECT_COUNT);
    // 满了/全渲染 就算了
    if (counter == LongUI::DIRTY_RECT_COUNT)
        return this->mark_fr_for_update();
    // 如果脏矩形列表存在祖先节点就算了
    const auto ctrl = rect.control;
    for (uint32_t i = 0; i != counter; ++i) {
        if (ctrl->IsAncestorForThis(*this->dirty_rect_recording[i].control))
            return;
    }
    // 标记在表
    UIControlPrivate::MarkInDirty(*rect.control);
    // 写入数据
    this->dirty_rect_recording[counter] = rect;
    ++counter;
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

/// <summary>
/// close the window
/// </summary>
/// <returns></returns>
void LongUI::CUIPlatform::close_window() noexcept {
    window()->close_window();
}

/// <summary>
/// resize the viewport
/// </summary>
/// <param name="size"></param>
/// <returns></returns>
void LongUI::CUIPlatform::resize_viewport(Size2F size) noexcept {
    viewport()->resize_window(size);
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
/// Befores the render.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::CUIPlatform::PrepareRender(const Size2F size) noexcept {
    // 先清除信息
    this->clear_fr_for_render();
    const uint32_t count = this->dirty_count_recording;
    this->dirty_count_recording = 0;
    std::for_each(
        this->dirty_rect_recording,
        this->dirty_rect_recording + count,
        [](const DirtyRect& x) noexcept {
        UIControlPrivate::ClearInDirty(*x.control);
    });
    // 全渲染
    if (this->is_fr_for_update()) {
        this->clear_fr_for_update();
        this->mark_fr_for_render();
        return;
    }
    // 初始化信息
    auto itr = this->dirty_rect_presenting;
    const auto endi = itr + LongUI::DIRTY_RECT_COUNT;
    // 遍历脏矩形
    for (uint32_t i = 0; i != count; ++i) {
        const auto& data = this->dirty_rect_recording[i];
        assert(data.control && data.control->GetWindow());
        itr = impl::mark_two_rect_dirty(
            data.rectangle,
            data.control->RefBox().visible,
            size,
            itr
        );
        // 溢出->全渲染
        if (itr > endi) return this->mark_fr_for_render();
    }
    // 写入数据
    const auto pcount = itr - this->dirty_rect_presenting;
    this->dirty_count_presenting = uint32_t(pcount);
#if 0
    // 先清除
    this->clear_full_rendering_for_render();
    // 复制全渲染信息
    if (this->is_full_render_for_update()) {
        this->mark_full_rendering_for_render();
        this->clear_full_rendering_for_update();
        this->dirty_count_for_render = 0;
        this->dirty_count_for_update = 0;
        return;
    }
    // 复制脏矩形信息
    this->dirty_count_for_render = this->dirty_count_for_update;
    this->dirty_count_for_update = 0;
    std::memcpy(
        this->dirty_rect_for_render,
        this->dirty_rect_for_update,
        sizeof(this->dirty_rect_for_update[0]) * this->dirty_count_for_render
    );
#endif
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
            this->popup->platform().OnAccessKey(i);
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


#include <core/ui_platform_win.h>

// longui namespace
namespace LongUI {
    // now platform
    using CUIPlatformNow = CUIPlatformWin;
    /// <summary>
    /// create platform
    /// </summary>
    /// <param name="buf"></param>
    /// <param name="len"></param>
    /// <returns></returns>
    void CreatePlatform(void* buf, size_t len) noexcept {
        using platform = impl::platform<sizeof(void*)>;
        enum { platform_size = sizeof(CUIPlatformNow), platform_align = alignof(CUIPlatformNow) };
        static_assert(platform_size <= platform::size, "buffer not safe");
        static_assert(platform_align <= platform::align, "buffer not safe");
        assert(uintptr_t(buf) % platform_align == 0 && "bad aligned");
        assert(len >= platform_size && "bad length");
        impl::ctor_dtor<CUIPlatformNow>::create(buf);
        const auto obj = reinterpret_cast<CUIPlatformNow*>(buf);
        assert(static_cast<void*>(obj) == static_cast<CUIPlatform*>(obj));
    }
    // down cast the platform
    inline auto platdown_cast(CUIPlatform* p) noexcept { return static_cast<CUIPlatformNow*>(p); }
    // down cast the platform
    inline auto platdown_cast(const CUIPlatform* p) noexcept { return static_cast<const CUIPlatformNow*>(p); }
    // longui static polymorphism
#define LUI_PLATSP(func, ...) return platdown_cast(this)->func(__VA_ARGS__)
    void CUIPlatform::Init(CUIWindow* p, uint16_t f) noexcept { LUI_PLATSP(Init, p, f); }
    auto CUIPlatform::Recreate() noexcept -> Result { LUI_PLATSP(Recreate); }
    auto CUIPlatform::Render() noexcept -> Result { LUI_PLATSP(Render); }
    void CUIPlatform::Dispose() noexcept { LUI_PLATSP(Dispose); }
    void CUIPlatform::ReleaseDeviceData() noexcept { LUI_PLATSP(ReleaseDeviceData); }
    void CUIPlatform::AfterTitleName() noexcept { LUI_PLATSP(AfterTitleName); }
    void CUIPlatform::AfterPosition() noexcept { LUI_PLATSP(AfterPosition); }
    void CUIPlatform::AfterAbsRect() noexcept { LUI_PLATSP(AfterAbsRect); }
    void CUIPlatform::CloseWindow() noexcept { LUI_PLATSP(CloseWindow); }
    void CUIPlatform::ShowWindow(int s) noexcept { LUI_PLATSP(ShowWindow, s); }
    void CUIPlatform::ResizeAbsolute(Size2L s) noexcept { LUI_PLATSP(ResizeAbsolute, s); }
    auto CUIPlatform::GetWorkArea() const noexcept ->RectL { LUI_PLATSP(GetWorkArea); }
    auto CUIPlatform::GetRawHandle() const noexcept ->uintptr_t { LUI_PLATSP(GetRawHandle); }
    void CUIPlatform::EnableWindow(bool e) noexcept { LUI_PLATSP(EnableWindow, e); }
    void CUIPlatform::ActiveWindow() noexcept { LUI_PLATSP(ActiveWindow); }
}