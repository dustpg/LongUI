// Gui
#include <luiconf.h>
#include <core/ui_window.h>
#include <core/ui_manager.h>
#include <control/ui_viewport.h>
// base platform
#include <core/ui_platform.h>
// private
#include "../private/ui_private_control.h"



// C++
#include <cmath>
#include <cassert>
#include <algorithm>



// longui namespace
namespace LongUI {
    // Commons the tooltip create.
    auto CommonTooltipCreate(UIControl& hoster) noexcept->UIViewport*;
    // Commons the tooltip set text.
    void CommonTooltipSetText(UIViewport& viewport, CUIString&& text) noexcept;
    // LongUI::impl
    namespace impl {
        // eval script for window
        void eval_script_for_window(U8View view, CUIWindow* window) noexcept {
            assert(window && "eval script but no window");
            UIManager.Evaluation(view, *window);
        }
    }
}


// LongUI::impl
namespace LongUI { namespace impl {
    /// <summary>
    /// Lowests the common ancestor.
    /// </summary>
    /// <param name="now">The now.</param>
    /// <param name="old">The old.</param>
    /// <returns></returns>
    UIControl* lowest_common_ancestor(UIControl* now, UIControl* old) noexcept {
        /* 
            由于控件有深度信息, 所以可以进行优化
            时间复杂度 O(q) q是目标解与最深条件节点之间深度差
        */
        // now不能为空
        assert(now && "new one cannot be null");
        // old为空则返回now
        if (!old) return now;
        // 连接到相同深度
        UIControl* upper, *lower;
        if (now->GetLevel() < old->GetLevel()) {
            // 越大就是越低
            lower = old; upper = now;
        }
        else {
            // 越小就是越高
            lower = now; upper = old;
        }
        // 将低节点调整至于高节点同一水平
        auto adj = lower->GetLevel() - upper->GetLevel();
        while (adj) {
            lower = lower->GetParent();
            --adj;
        }
        // 共同遍历
        while (upper != lower) {
            assert(upper->IsTopLevel() == false);
            assert(lower->IsTopLevel() == false);
            upper = upper->GetParent();
            lower = lower->GetParent();
        }
        assert(upper && lower && "cannot be null");
        return upper;
    }
    // 获取透明窗口适合的大小
    inline auto get_fit_size_for_trans(uint32_t len) noexcept {
        constexpr uint32_t UNIT = TRANSPARENT_WIN_BUFFER_UNIT;
        return static_cast<uint32_t>(len + UNIT - 1) / UNIT * UNIT;
    }
}}

/// <summary>
/// Sets the control world changed.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::CUIWindow::SetControlWorldChanged(UIControl& ctrl) noexcept {
    assert(ctrl.GetWindow() == this);
    m_pMiniWorldChange = impl::lowest_common_ancestor(&ctrl, m_pMiniWorldChange);
    //LUIDebug(Log) << ctrl << ctrl.GetLevel() << endl;
}

/// <summary>
/// Deletes the later.
/// </summary>
/// <returns></returns>
void LongUI::CUIWindow::DeleteLater() noexcept {
    const auto view = &this->RefViewport();
    view->DeleteLater();
}

/// <summary>
/// Deletes this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUIWindow::Delete() noexcept {
    const auto view = &this->RefViewport();
    delete view;
}

// ui namespace
namespace LongUI {
    // create platform
    void CreatePlatform(void* buf, size_t len) noexcept;
#ifndef LUI_DISABLE_STYLE_SUPPORT
    // make style sheet
    auto MakeStyleSheet(U8View view, SSPtr old) noexcept->SSPtr;
    // make style sheet
    auto MakeStyleSheetFromFile(U8View view, SSPtr old) noexcept->SSPtr;
#endif
}


/// <summary>
/// update focus rect
/// </summary>
/// <param name="cfg">The config.</param>
/// <param name="parent">The parent.</param>
void LongUI::CUIWindow::UpdateFocusRect(const RectF& rect) noexcept {
    platform().foucs = rect;
}

/// <summary>
/// Initializes a new instance of the <see cref="CUIWindow" /> class.
/// </summary>
/// <param name="cfg">The config.</param>
/// <param name="parent">The parent.</param>
LongUI::CUIWindow::CUIWindow(CUIWindow* parent, WindowConfig cfg) noexcept :
    m_pParent(parent), config(cfg) {
    this->prev = this->next = nullptr;
    // 创建平台
    LongUI::CreatePlatform(&m_platform, sizeof(m_platform));
    // 节点
    m_oHead = { nullptr, static_cast<CUIWindow*>(&m_oTail) };
    m_oTail = { static_cast<CUIWindow*>(&m_oHead), nullptr };
    m_oListNode = { nullptr, nullptr };
    // 初始化BF
    m_inDtor = false;
    m_bInExec = false;
    // 子像素渲染
    //if (UIManager.flag & ConfigureFlag::Flag_SubpixelTextRenderingAsDefault)
    //    impl::get_subpixel_text_rendering(platform().text_antialias);
    // XXX: 内联窗口的场合
    // 添加分层窗口支持
    //if ((cfg & Config_LayeredWindow)) 
    //    platform().SetLayeredWindowSupport();
    // 初始化

    // TODO: 初始化默认大小位置
    //platform().InitWindowPos();
    // XXX: 自动睡眠?
    //if (this->IsAutoSleep()) {

    //}
}


/// <summary>
/// Initializes this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUIWindow::init() noexcept {
    // 存在父窗口则加入父窗口
    if (m_pParent) m_pParent->add_child(*this);
    // 否则加入窗口管理器的顶层管理
    else UIManager.add_topwindow(*this);
    // 为了方便遍历, 添加到全窗口列表
    UIManager.add_to_allwindow(*this);
}


#ifndef LUI_DISABLE_STYLE_SUPPORT
/// <summary>
/// Makes the style sheet from file.
/// </summary>
/// <param name="file">The file.</param>
/// <param name="old">The old.</param>
/// <returns></returns>
auto LongUI::MakeStyleSheetFromFile(U8View file, SSPtr old) noexcept -> SSPtr {
    const CUIStringU8 old_dir = UIManager.GetXulDir();
    auto path = old_dir; path += file;
    // 获取CSS目录以便获取正确的文件路径
    const auto view = LongUI::FindLastDir(path.view());
    // 设置CSS目录作为当前目录
    UIManager.SetXulDir(view);
    // 待使用缓存
    POD::Vector<uint8_t> css_buffer;
    // 载入文件
    UIManager.LoadDataFromUrl(path.view(), luiref css_buffer);
    // 字符缓存有效
    if (const auto len = css_buffer.size()) {
        const auto bptr = &*css_buffer.cbegin();
        const auto ptr0 = reinterpret_cast<const char*>(bptr);
        const U8View view{ ptr0, ptr0 + len };
        old = LongUI::MakeStyleSheet(view, old);
    }
    // 设置之前的目录作为当前目录
    UIManager.SetXulDir(old_dir.view());
    return old;
}

/// <summary>
/// Loads the CSS file.
/// </summary>
/// <param name="file">The file.</param>
/// <returns></returns>
void LongUI::CUIWindow::LoadCssFile(U8View file) noexcept {
    m_pStyleSheet = LongUI::MakeStyleSheetFromFile(file, m_pStyleSheet);
}

/// <summary>
/// Loads the CSS string.
/// </summary>
/// <param name="string">The string.</param>
/// <returns></returns>
void LongUI::CUIWindow::LoadCssString(U8View string) noexcept {
    m_pStyleSheet = LongUI::MakeStyleSheet(string, m_pStyleSheet);
}

#endif


/// <summary>
/// Adds the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::CUIWindow::add_child(CUIWindow& child) noexcept {
    // TODO: 在之前的父控件移除该控件
    assert(child.GetParent() == this && this);

    // 连接前后节点
    m_oTail.prev->next = &child;
    child.prev = m_oTail.prev;
    child.next = static_cast<CUIWindow*>(&m_oTail);
    m_oTail.prev = &child;
}

/// <summary>
/// Finalizes an instance of the <see cref="CUIWindow"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIWindow::~CUIWindow() noexcept {
    // 需要即时修改节点信息, 只能上超级锁了
    UIManager.DataLock();
    UIManager.RenderLock();
#ifdef LUI_ACCESSIBLE
    if (m_pAccessible) {
        LongUI::FinalizeAccessible(*m_pAccessible);
        m_pAccessible = nullptr;
    }
#endif
#ifndef LUI_DISABLE_STYLE_SUPPORT
    // 释放样式表
    LongUI::DeleteStyleSheet(m_pStyleSheet);
    m_pStyleSheet = nullptr;
#endif
    // 有脚本
    if (this->custom_script) UIManager.FinalizeScript(*this);
    // 析构中
    m_inDtor = true;
    // 未初始化就被删除的话节点为空
    if (this->prev) {
        // 连接前后节点
        this->prev->next = this->next;
        this->next->prev = this->prev;
        this->prev = this->next = nullptr;
    }
    UIManager.remove_from_allwindow(*this);
    // 存在父窗口
    //m_pParent->remove_child(*this);
    // 有效私有数据
    {
        // 弹出窗口会在下一步删除
        platform().popup = nullptr;
        // XXX: 删除自窗口?
        while (m_oHead.next != &m_oTail) 
            m_oTail.prev->Delete();
        // 管理器层移除引用
        //UIManager.remove_window(*this);
        platform().Dispose();
        // 摧毁窗口
        //if (m_hwnd) Private::Destroy(m_hwnd, platform().accessibility);
        //m_hwnd = nullptr;
        // 删除数据
        //platform().~Private();
    }
    UIManager.RenderUnlock();
    UIManager.DataUnlock();
}

/// <summary>
/// Prepare the render.
/// </summary>
/// <returns></returns>
void LongUI::CUIWindow::PrepareRender() noexcept {
    const auto size = this->RefViewport().GetRealSize();
    platform().PrepareRender(size);
}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIWindow::Render() noexcept -> Result {
    return platform().Render();
#if 0
    // 可见可渲染?
    if (this->IsVisible()) return platform().Render();
    // 不可见增加计数
    static_assert(WINDOW_AUTOSLEEP_TIME > 10, "must large than 10");
    if (auto& count = platform().auto_sleep_count) {
        count += UIManager.GetDeltaTimeMs();
        if (count > WINDOW_AUTOSLEEP_TIME) {
            this->IntoSleepImmediately();
            count = 0;
        }
    }
    return{ Result::RS_OK };
#endif
}

namespace LongUI {
    /// <summary>
    /// Recursives the recreate_device.
    /// </summary>
    /// <param name="ctrl">The control.</param>
    /// <param name="release">if set to <c>true</c> [release].</param>
    /// <returns></returns>
    auto RecursiveRecreate(UIControl& ctrl, bool release) noexcept ->Result {
        Result hr = ctrl.Recreate(release);
        for (auto& x : ctrl) {
#if 0
            const auto code = LongUI::RecursiveRecreate(x, release);
            if (!code) hr = code;
#else
            hr = LongUI::RecursiveRecreate(x, release);
            if (!hr) break;
#endif
        }
        return hr;
    }
}

/// <summary>
/// Recreates the device data.
/// </summary>
/// <returns></returns>
auto LongUI::CUIWindow::RecreateDeviceData() noexcept -> Result {
    // 重建窗口设备资源
    const auto hr = platform().Recreate();
    if (!hr) return hr;
    // 重建控件设备资源
    return LongUI::RecursiveRecreate(this->RefViewport(), false);
}

/// <summary>
/// Releases the device data.
/// </summary>
/// <returns></returns>
void LongUI::CUIWindow::ReleaseDeviceData() noexcept {
    // 释放窗口设备资源
    platform().ReleaseDeviceData();
    // 释放控件设备资源
    LongUI::RecursiveRecreate(this->RefViewport(), true);
}

/// <summary>
/// Gets the position.
/// </summary>
/// <returns></returns>
auto LongUI::CUIWindow::GetPos() const noexcept -> Point2L {
    const auto& rect = RefPlatform().rect;
    return { rect.left, rect.top };
}

/// <summary>
/// Gets the position.
/// </summary>
/// <returns></returns>
auto LongUI::CUIWindow::GetAbsoluteSize() const noexcept -> Size2L {
    const auto& rect = RefPlatform().rect;
    return { rect.width, rect.height };
}


/// <summary>
/// Finds the control.
/// </summary>
/// <param name="id">The identifier.</param>
/// <returns></returns>
auto LongUI::CUIWindow::FindControl(const char* id) noexcept -> UIControl* {
    assert(id && "bad id");
    U8View view = U8View::FromCStyle(id);
    return this->FindControl(UIManager.GetUniqueText(view));
}


/// <summary>
/// Finds the control.
/// </summary>
/// <param name="id">The identifier.</param>
/// <returns></returns>
auto LongUI::CUIWindow::FindControl(U8View id) noexcept -> UIControl* {
    assert(id.size() && "bad id");
    return this->FindControl(UIManager.GetUniqueText(id));
}

PCN_NOINLINE
/// <summary>
/// Finds the control.
/// </summary>
/// <param name="id">The identifier.</param>
/// <returns></returns>
auto LongUI::CUIWindow::FindControl(ULID id) noexcept -> UIControl * {
    if (id.id == CUIConstShortString::EMPTY) return nullptr;
    assert(id.id && *id.id && "bad string");
    auto node = platform().named_list.first;
    // 遍历命名列表
    while (node) {
        if (node->GetID().id == id.id) return node;
        node = node->m_oManager.next_named;
    }
    return nullptr;
}

/// <summary>
/// Controls the attached.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::CUIWindow::ControlAttached(UIControl& ctrl) noexcept {
    if (!this) return;
    // 本函数调用地点
    // A. UIControl::init
    // B. UIControl::set_window_force
    assert(ctrl.is_inited() && "call after init");
    // 注册焦点链
    if (ctrl.IsTabstop()) {
        assert(ctrl.IsFocusable());
        auto& list = platform().focus_list;
#ifndef NDEBUG
        // 必须不在里面
        auto node = list.first;
        while (node) {
            if (*node == ctrl) {
                LUIDebug(Error) LUI_FRAMEID
                    << "add focus control but control exist: "
                    << ctrl
                    << endl;
            }
            node = node->m_oManager.next_tabstop;
        }
#endif
        const size_t offset = offsetof(UIControl, m_oManager.next_tabstop);
        CUIControlControl::AddControlToList(ctrl, list, offset);
    }
    // 注册快捷键
    if (const auto ch = ctrl.GetAccessKey()) {
        if (ch >= 'A' && ch <= 'Z') {
            const auto index = ch - 'A';
            const auto map = platform().access_key_map;
#ifndef NDEBUG
            if (map[index]) LUIDebug(Warning) << "Access-key(" << ch << ") existed" << endl;
#endif
            map[index] = &ctrl;
        }
#ifndef NDEBUG
        else if (ch)  LUIDebug(Warning) << "unsupported Access-key" << endl;
#endif
    }
}

/// <summary>
/// Adds the named control.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::CUIWindow::AddNamedControl(UIControl& ctrl) noexcept {
#ifndef NDEBUG
    if (!this) LUIDebug(Warning) << "null this pointer" << endl;
#endif
    if (!this) return;
    // XXX: 自己就是窗口的场合
    if (this->RefViewport() == ctrl) return;
    // 注册命名控件
    if (ctrl.GetID().id != CUIConstShortString::EMPTY) {
        assert(ctrl.GetID().id && ctrl.GetID().id[0]);
#ifndef NDEBUG
        // 必须没有被注册过
        if (this->FindControl(ctrl.GetID())) {
            LUIDebug(Error) LUI_FRAMEID
                << "add named control but id exist: "
                << ctrl.GetID()
                << endl;
        }
#endif
        auto& list = platform().named_list;
        const size_t offset = offsetof(UIControl, m_oManager.next_named);
        CUIControlControl::AddControlToList(ctrl, list, offset);
    }
}

/// <summary>
/// Controls the disattached.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <remarks>
/// null this ptr acceptable
/// </remarks>
/// <returns></returns>
void LongUI::CUIWindow::ControlDisattached(UIControl& ctrl) noexcept {
    // 没有承载窗口就算了
    if (!this) return;
    // 清除
    const auto cleanup = [this, &ctrl]() noexcept {
        // 强制重置
        ctrl.m_oStyle.state = ctrl.m_oStyle.state & ~State_Focus;
        m_pMiniWorldChange = nullptr;
        platform().dirty_count_recording = 0;
    };
    // 析构中
    if (m_inDtor) return cleanup();
    // 移除相关弱引用

    CUIDataAutoLocker locker;
    // 2. 移除最小世界修改
    if (m_pMiniWorldChange == &ctrl) {
        // TEST THIS
        assert(m_pMiniWorldChange != &RefViewport());
        m_pMiniWorldChange = m_pMiniWorldChange->GetParent();
    }
    // 3. 移除在脏矩形列表
    if (UIControlPrivate::IsInDirty(ctrl)) {
        UIControlPrivate::ClearInDirty(ctrl);
        // 查找
        if (!platform().is_fr_for_update()) {
            const auto b = platform().dirty_rect_recording;
            const auto e = b + platform().dirty_count_recording;
            const auto itr = std::find_if(b, e, [&ctrl](const auto& x) noexcept {
                return x.control == &ctrl;
            });
            // 安全起见
            if (itr != e) {
                // 最后一个和itr调换
                // 特殊情况: itr == e[-1], 调换并不会出现问题
                std::swap(*itr, e[-1]);
                assert(platform().dirty_count_recording);
                platform().dirty_count_recording--;
            }
            else assert(!"NOT FOUND");
        }
    }
    // 4. 移除普通弱引用
    std::for_each(
        platform().GetFirst(), 
        platform().GetLast() + 1,
        [&](auto& p) noexcept { if (p == &ctrl) p = nullptr; }
    );
    // 5. 移除快捷弱引用
    const auto ch = ctrl.GetAccessKey();
    if (ch >= 'A' && ch <= 'Z') {
        const auto index = ch - 'A';
        const auto map = platform().access_key_map;
        // 移除引用
        if (map[index] == &ctrl) map[index] = nullptr;
        // 提出警告
#ifndef NDEBUG
        else {
            LUIDebug(Warning)
                << "map[index] != &ctrl:(map[index]:"
                << map[index]
                << ")"
                << endl;
        }
#endif

    }
    // 5. 移除查找表中的弱引用
    if (ctrl.GetID().id[0]) {
        auto& list = platform().named_list;
        const size_t offset = offsetof(UIControl, m_oManager.next_named);
        CUIControlControl::RemoveControlInList(ctrl, list, offset);
    }
    // 6. 移除焦点表中的弱引用
    if (ctrl.IsFocusable()) {
        auto& list = platform().focus_list;
        const size_t offset = offsetof(UIControl, m_oManager.next_tabstop);
        CUIControlControl::RemoveControlInList(ctrl, list, offset);
    }
    ctrl.m_oManager.next_tabstop = nullptr;
    ctrl.m_oManager.next_named = nullptr;
}

/// <summary>
/// Sets the focus.
/// 为目标控件设置键盘焦点
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
bool LongUI::CUIWindow::SetFocus(UIControl& ctrl) noexcept {
    // 不可聚焦
    if (!ctrl.IsFocusable()) return false;
    // 焦点控件
    auto& focused = platform().focused;
    // 当前焦点不能是待聚焦控件的祖先控件
#ifndef DEBUG
    if (focused) {
        assert(
            !ctrl.IsAncestorForThis(*focused) && 
            "cannot set focus to control that ancestor was focused"
        );
    }
    const auto old_focused = focused;
#endif
    // 已为焦点
    if (focused == &ctrl) return true;
    // 释放之前焦点
    if (focused) this->KillFocus(*focused);
    // 设为焦点
    focused = &ctrl;
    ctrl.StartAnimation({ State_Focus, State_Focus });
    // Focus 事件
    ctrl.FireSimpleEvent(UIControl::_onFocus());
    return true;
}


/// <summary>
/// set focus to prev control
/// 设置键盘焦点至上一个焦点
/// </summary>
/// <returns></returns>
bool LongUI::CUIWindow::FocusPrev() noexcept {
    // 搜索上一个
    const auto find_prev = [this](UIControl* focused) noexcept {
        auto node = platform().focus_list.first;
        UIControl* rcode = nullptr;
        while (node) {
            if (node == focused) break;
            if (node->IsEnabled()) rcode = node;
            node = node->m_oManager.next_tabstop;
        }
        return rcode;
    };
    UIControl* target = nullptr;
    // 当前焦点的下一个
    if (platform().focused) target = find_prev(platform().focused);
    // 没有就最后一个
    // XXX: 最后一个无效时候怎么处理
    if (!target) target = platform().focus_list.last;
    // 还没有就算了
    if (!target) return false;
    this->SetDefault(*target);
    return this->SetFocus(*target);
}

/// <summary>
/// set focus to next control
/// 设置键盘焦点至下一个焦点
/// </summary>
/// <returns></returns>
bool LongUI::CUIWindow::FocusNext() noexcept {
    // TODO: 下一个可以成为焦点控件但是不能成为默认控件时候
    // 默认控件应该回退至窗口初始的默认控件


    // 搜索下一个
    const auto find_next = [](UIControl* node) noexcept {
        while (true) {
            node = node->m_oManager.next_tabstop;
            if (!node || (node->IsEnabled() && node->IsVisibleEx())) break;
        }
        return node;
    };
    UIControl* target = nullptr;
    // 当前焦点的下一个
    if (platform().focused) target = find_next(platform().focused);
    // 没有就第一个
    // XXX: 第一个无效时候怎么处理
    if (!target) target = platform().focus_list.first;
    // 还没有就算了
    if (!target) return false;
    this->SetDefault(*target);
    return this->SetFocus(*target);
}

/// <summary>
/// do something after control got invisible
/// </summary>
/// <param name="ctrl"></param>
/// <returns></returns>
void LongUI::CUIWindow::DoControlInvisible(UIControl& ctrl) noexcept {
    // TODO: 是检查VisibleEx 还是 直接取消?
    if (const auto f = platform().focused)
        if (f->IsAncestorForThis(ctrl)) this->KillFocus(*f);
    if (const auto d = platform().now_default)
        if (d->IsAncestorForThis(ctrl)) this->NullDefault();
}

/// <summary>
/// Shows the caret.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::CUIWindow::ShowCaret(UIControl& ctrl, const RectF& rect) noexcept {
    assert(this);
    platform().careted = &ctrl;
    platform().draw_caret = true;
    platform().caret = rect;
    //LUIDebug(Warning) << rect << endl;
    //ctrl.MapToWindow(platform().caret);
}

/// <summary>
/// Hides the caret.
/// </summary>
/// <returns></returns>
void LongUI::CUIWindow::HideCaret() noexcept {
    assert(this);
    platform().careted = nullptr;
    platform().draw_caret = false;
}

/// <summary>
/// Sets the color of the caret.
/// </summary>
/// <param name="color">The color.</param>
/// <returns></returns>
void LongUI::CUIWindow::SetCaretColor(const ColorF& color) noexcept {
    assert(this);
    platform().caret_color = color;
}

/// <summary>
/// Sets the capture.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::CUIWindow::SetCapture(UIControl& ctrl) noexcept {
    platform().captured = &ctrl;
    //LUIDebug(Hint) << ctrl << endl;
}

/// <summary>
/// Releases the capture.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
bool LongUI::CUIWindow::ReleaseCapture(UIControl& ctrl) noexcept {
    if (&ctrl == platform().captured) {
        //LUIDebug(Hint) << ctrl << endl;
        assert(platform().captured);
        platform().captured = nullptr;
        return true;
    }
    return false;
}

/// <summary>
/// force release
/// </summary>
/// <returns></returns>
void LongUI::CUIWindow::ForceReleaseCapture() noexcept {
    platform().captured = nullptr;
}

/// <summary>
/// Kills the focus.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::CUIWindow::KillFocus(UIControl& ctrl) noexcept {
    if (platform().focused == &ctrl) {
        platform().focused = nullptr;
        //m_private->saved_focused = nullptr;
        ctrl.StartAnimation({ State_Focus, State_Non });
        // Blur 事件
        ctrl.FireSimpleEvent(UIControl::_onBlur());
    }
}

/// <summary>
/// Resets the default.
/// </summary>
/// <returns></returns>
void LongUI::CUIWindow::ResetDefault() noexcept {
    if (platform().wnd_default) {
        this->SetDefault(*platform().wnd_default);
    }
}

/// <summary>
/// Invalidates the control.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::CUIWindow::InvalidateControl(UIControl& ctrl, const RectF* rect) noexcept {
    // 已经在里面就算了
    if (UIControlPrivate::IsInDirty(ctrl)) return;
    assert(ctrl.GetWindow() == this);
    // 全渲染
    if (platform().is_fr_for_update()) return;
    // TODO: 相对矩形
    assert(rect == nullptr && "unsupported yet");
    platform().MarkDirtyRect({ &ctrl, ctrl.RefBox().visible });
}


/// <summary>
/// Marks the full rendering.
/// </summary>
/// <returns></returns>
void LongUI::CUIWindow::MarkFullRendering() noexcept {
    platform().mark_fr_for_update();
}

/// <summary>
/// Determines whether [is full render this frame].
/// </summary>
/// <returns></returns>
bool LongUI::CUIWindow::IsFullRenderThisFrame() const noexcept {
    return RefPlatform().is_fr_for_update();
}

/// <summary>
/// Sets the default.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::CUIWindow::SetDefault(UIControl& ctrl) noexcept {
    assert(this && "null this ptr");
    if (!ctrl.IsDefaultable()) return;
    auto& nowc = platform().now_default;
    if (nowc) nowc->StartAnimation({ State_Default, State_Non });
    (nowc = &ctrl)->StartAnimation({ State_Default, State_Default });
}

/// <summary>
/// Sets the default.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::CUIWindow::NullDefault() noexcept {
    auto& nowc = platform().now_default;
    if (!nowc) return;
    nowc->StartAnimation({ State_Default, State_Non });
    nowc = nullptr;
}

/// <summary>
/// Sets the color of the clear.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::CUIWindow::SetClearColor(const ColorF& color) noexcept {
    platform().clear_color = color;
}

/// <summary>
/// Sets the now cursor.
/// </summary>
/// <param name="cursor">The cursor.</param>
/// <returns></returns>
void LongUI::CUIWindow::SetNowCursor(const CUICursor& cursor) noexcept {
    platform().cursor = cursor;
}

/// <summary>
/// Sets the now cursor.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::CUIWindow::SetNowCursor(std::nullptr_t) noexcept {
    platform().cursor = { CUICursor::Cursor_Arrow };
}


/// <summary>
/// get word area
/// </summary>
/// <returns></returns>
auto LongUI::CUIWindow::GetWorkArea() const noexcept -> RectL {
    return RefPlatform().GetWorkArea();
}

/// <summary>
/// draw raw handle
/// </summary>
/// <returns></returns>
auto LongUI::CUIWindow::GetRawHandle() const noexcept -> uintptr_t {
    return RefPlatform().GetRawHandle();
}

/// <summary>
/// enable the window
/// </summary>
/// <param name="enable"></param>
/// <returns></returns>
void LongUI::CUIWindow::EnableWindow(bool enable) noexcept {
    platform().EnableWindow(enable);
}

/// <summary>
/// active window(the focus to window self)
/// </summary>
/// <returns></returns>
void LongUI::CUIWindow::ActiveWindow() noexcept {
    platform().ActiveWindow();
}

#if 0
/// <summary>
/// Called when [resize].
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::CUIWindow::Private::OnResizeTs(Size2U size) noexcept {
    assert(size.width && size.height && "bad size");
    // 不一样才处理
    const auto samew = this->rect.width == size.width;
    const auto sameh = this->rect.height == size.height;
    if (samew && sameh) return;
    // 数据锁
    CUIDataAutoLocker locker;
    //LUIDebug(Hint) << size.width << ", " << size.height << endl;
    this->mark_fr_for_update();
    // 修改
    this->rect.width = size.width;
    this->rect.height = size.height;
    const auto fw = static_cast<float>(size.width);
    const auto fh = static_cast<float>(size.height);
    // 重置大小
    this->viewport()->resize_window({ fw, fh });
    // 修改窗口缓冲帧大小
    this->flag_sized = true;
}
#endif

/// <summary>
/// Resizes the absolute.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::CUIWindow::ResizeAbsolute(Size2L size) noexcept {
    assert(size.width > 0 && size.height > 0);
    platform().ResizeAbsolute(size);
}

/// <summary>
/// Resizes the relative.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::CUIWindow::ResizeRelative(Size2F size) noexcept {
    // 先要清醒才行
    this->WakeUp();
    // TODO: 可以不要求清醒状态
    return this->ResizeAbsolute(RefViewport().AdjustSize(size));
}

/// <summary>
/// get screen left-top point
/// </summary>
template<typename T> inline auto LongUI::CUIWindow::screen_lt() const noexcept -> Point<T> {
    const auto& platpt = RefPlatform();
    const auto left = static_cast<T>(platpt.rect.left);
    const auto top = static_cast<T>(platpt.rect.top);
    const auto ax = static_cast<T>(platpt.adjust.left);
    const auto ay = static_cast<T>(platpt.adjust.top);
    return { left + ax, top + ay };
}

/// <summary>
/// Maps to screen.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::CUIWindow::MapToScreen(RectL& rect) const noexcept {
    const auto platpt = screen_lt<int32_t>();
    rect.left += platpt.x;
    rect.top += platpt.y;
    rect.right += platpt.x;
    rect.bottom += platpt.y;
}

/// <summary>
/// Maps to screen.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::CUIWindow::MapToScreen(RectF& rect) const noexcept {
    const auto platpt = screen_lt<float>();
    rect.left += platpt.x;
    rect.top += platpt.y;
    rect.right += platpt.x;
    rect.bottom += platpt.y;
}

/// <summary>
/// Maps to screen.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
auto LongUI::CUIWindow::MapToScreenEx(Point2F pos) const noexcept ->Point2F {
    const auto platpt = screen_lt<float>();
    return pos + platpt;
}

/// <summary>
/// Maps from screen.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::CUIWindow::MapFromScreen(Point2F& pos) const noexcept {
    const auto platpt = screen_lt<float>();
    pos = pos - platpt;
}

/// <summary>
/// get window object
/// </summary>
/// <returns></returns>
auto LongUI::CUIPlatform::window() noexcept -> CUIWindow * {
    const auto ptr = reinterpret_cast<char*>(this);
    const auto offset = offsetof(CUIWindow, m_platform);
    const auto window = reinterpret_cast<CUIWindow*>(ptr - offset);
    assert(this && &window->RefPlatform() == (void*)this);
    return window;
}

/// <summary>
/// get viewport object
/// </summary>
/// <returns></returns>
auto LongUI::CUIPlatform::viewport() noexcept -> UIViewport * {
    const auto ptr = reinterpret_cast<char*>(this);
    const auto offset = offsetof(CUIWindow, m_platform);
    const auto window = reinterpret_cast<CUIWindow*>(ptr - offset);
    assert(this && &window->RefPlatform() == (void*)this);
    return &window->RefViewport();
}


/// <summary>
/// Closes the window.
/// </summary>
/// <returns></returns>
void LongUI::CUIWindow::close_window() noexcept {
    // 取消
    this->SetResult(0);
    // 通用处理
    UIManager.close_helper(*this);
    // 提醒VP: 窗口关了
    this->RefViewport().WindowClosed();
}


/// <summary>
/// Shows the window.
/// </summary>
/// <param name="sw">The sw type.</param>
/// <returns></returns>
void LongUI::CUIWindow::show_window(TypeShow sw) noexcept {
    // 隐藏
    if (sw == Show_Hide) {
        this->TrySleep();
        platform().visible = false;
    }
    // 显示
    else {
        this->WakeUp();
        platform().visible = true;
    }
    platform().ShowWindow(sw);
}

/// <summary>
/// Wakeks up.
/// </summary>
void LongUI::CUIWindow::WakeUp() noexcept {
#if 0
    // 不在睡眠状态强行渲染一帧
    if (!this->IsInSleepMode()) {
        if (!platform().window_visible) platform().EmptyRender();
        return;
    }
    // XXX: 0. 尝试唤醒父窗口
    if (m_pParent) m_pParent->WakeUp();
    // 1. 创建窗口
    const auto pwnd = m_pParent ? m_pParent->GetHwnd() : nullptr;
    m_hwnd = platform().Init(pwnd, this->config);
    // 1.5 检测DPI支持
    this->HiDpiSupport();
    // 2. 创建资源
    const auto hr = this->recreate_window();
    assert(hr && "TODO: error handle");
#endif
    // 1. 创建窗口
    platform().Init(m_pParent, this->config);
}

/// <summary>
/// Intoes the sleep.
/// </summary>
void LongUI::CUIWindow::SleepImmediately() noexcept {
#if 0
    if (this->IsInSleepMode()) return;
#ifndef NDEBUG
    LUIDebug(Hint) 
        << this << this->RefViewport()
        << "into sleep mode" << endl;
#endif // !NDEBUG
    //assert(this->begin() == this->end());

    // 释放资源
    platform().ReleaseDeviceData();
    Private::Destroy(m_hwnd, platform().accessibility);
    m_hwnd = nullptr;
#endif
}

/// <summary>
/// Tries the sleep.
/// </summary>
/// <returns></returns>
void LongUI::CUIWindow::TrySleep() noexcept {
#if 0
    // 已经进入就算了
    if (this->IsInSleepMode()) return;
    // 必须是自动休眠
    if (!this->IsAutoSleep()) return;
    // XXX: 存在子窗口就算了?
    //if (this->begin() != this->end()) return;
    // 增加1毫秒(+1ms)长度表示进入睡眠计时
    platform().auto_sleep_count = 1;
#endif
}

/// <summary>
/// Executes this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIWindow::Exec() noexcept->uintptr_t {
    uintptr_t rv = 0;
    const auto parent = this->GetParent();
    m_bInExec = true;
    CUIBlockingGuiOpAutoUnlocker unlocker;
    if (parent) {
        // 禁止父窗口调用
        parent->EnableWindow(false);
        // 增加一层消息循环
        UIManager.RecursionMsgLoop();
        // 恢复父窗口调用
        parent->EnableWindow(true);
        // 激活父窗口
        parent->ActiveWindow();
    }
    else UIManager.RecursionMsgLoop();
    m_bInExec = false;
    return rv;
}


/// <summary>
/// Recursives the set result.
/// </summary>
/// <param name="result">The result.</param>
/// <returns></returns>
void LongUI::CUIWindow::recursive_set_result(uintptr_t result) noexcept {
    // TEST
    assert(this->IsInExec());
    auto node = m_oHead.next;
    const auto tail = &m_oTail;
    while (node != tail) {
        if (node->IsInExec()) node->recursive_set_result(0);
        node = node->next;
    }
    UIManager.BreakMsgLoop(result);
}

/// <summary>
/// Sets the result.
/// </summary>
/// <param name="result">The result.</param>
/// <returns></returns>
void LongUI::CUIWindow::SetResult(uintptr_t result) noexcept {
    if (m_bInExec) this->recursive_set_result(result);
}

/// <summary>
/// Closes the window.
/// </summary>
/// <returns></returns>
void LongUI::CUIWindow::CloseWindow() noexcept {
    platform().CloseWindow();
}

/// <summary>
/// Determines whether this instance is visible.
/// </summary>
/// <returns></returns>
bool LongUI::CUIWindow::IsVisible() const noexcept {
    return RefPlatform().visible;
}

/// <summary>
/// Sets the name of the title.
/// </summary>
/// <param name="view">The tile name.</param>
/// <returns></returns>
void LongUI::CUIWindow::SetTitleName(U16View view) noexcept {
    this->SetTitleName(CUIString{ view });
}

/// <summary>
/// Gets the name of the title.
/// </summary>
/// <returns></returns>
auto LongUI::CUIWindow::GetTitleName() const noexcept -> U16View {
    return RefPlatform().titlename.view();
}

/// <summary>
/// Popups the window.
/// </summary>
/// <param name="wnd">The WND.</param>
/// <param name="pos">The position.</param>
/// <param name="type">The type.</param>
/// <returns></returns>
void LongUI::CUIWindow::PopupWindow(CUIWindow& wnd, Point2L pos, PopupType type) noexcept {
    auto& this_popup = platform().popup;
    // 再次显示就是关闭
    if (this_popup == &wnd) {
        // 直接关闭
        this->ClosePopup();
    }
    else {
        this->ClosePopup();
        this_popup = &wnd;
        // 记录类型备用
        platform().popup_type = type;
        // 提示窗口
        auto& view = wnd.RefViewport();
        view.HosterPopupBegin();
        this->RefViewport().SubViewportPopupBegin(view, type);
        // 计算位置
        wnd.SetPos(pos);
        wnd.ShowNoActivate();
    }
}

/// <summary>
/// Tooltips the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
auto LongUI::CUIWindow::TooltipText(CUIString&& text)noexcept->UIViewport* {
    auto& ptr = platform().common_tooltip;
    if (!ptr) ptr = LongUI::CommonTooltipCreate(this->RefViewport());
    if (ptr) LongUI::CommonTooltipSetText(*ptr, std::move(text));
    return platform().common_tooltip;
}

/// <summary>
/// Closes the popup.
/// 关闭当前的弹出窗口
/// </summary>
/// <returns></returns>
void LongUI::CUIWindow::ClosePopup() noexcept {
    platform().ClosePopup();
}

/// <summary>
/// Gets the now popup.
/// 获取当前的弹出窗口
/// </summary>
/// <returns>
/// <see cref="CUIWindow"/>指针, 当前没有弹出窗口则返回空指针
///</returns>
auto LongUI::CUIWindow::GetNowPopup() const noexcept-> CUIWindow* {
    return this->RefPlatform().popup;
}

/// <summary>
/// Gets the now popup with specify type.
/// 获取当前指定类型的弹出窗口
/// </summary>
/// <param name="type">The type.</param>
/// <returns>
/// <see cref="CUIWindow" />指针, 当前没有指定类型的弹出窗口则返回空指针
/// </returns>
auto LongUI::CUIWindow::GetNowPopup(PopupType type) const noexcept-> CUIWindow* {
    auto& platform = this->RefPlatform();
    return platform.popup_type == type ? platform.popup : nullptr;
}

/// <summary>
/// Closes the popup high level.
/// 尽可能向上级关闭弹出窗口
/// </summary>
/// <remarks>
/// 该函数允许传递空this指针
/// </remarks>
void LongUI::CUIWindow::ClosePopupHighLevel() noexcept {
#if 0
    // 根据Parent信息
    auto winp = this;
    // 正式处理
    while (winp && winp->config & Config_Popup) winp = winp->GetParent();
#else
    // 根据Hoster信息
    auto& view = this->RefViewport();
    auto hoster = view.GetHoster();
    CUIWindow* winp = nullptr;
    while (hoster) {
        winp = hoster->GetWindow();
        if (winp) hoster = winp->RefViewport().GetHoster();
    }
    assert(!winp || !(winp->config & Config_Popup));
#endif
    if (winp) winp->ClosePopup();
    else LUIDebug(Error) << "winp -> null" << endl;
}

/// <summary>
/// Closes the tooltip.
/// </summary>
/// <returns></returns>
void LongUI::CUIWindow::CloseTooltip() noexcept {
    if (platform().popup_type == PopupType::Type_Tooltip) {
        platform().ClosePopup();
    }
}

/// <summary>
/// Sets the absolute rect.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::CUIWindow::SetAbsoluteRect(const RectL& rect) noexcept {
    // 懒得判断了
    auto& write = platform().rect;
    write.left = rect.left;
    write.top = rect.top;
    write.width = rect.right - rect.left;
    write.height = rect.bottom - rect.top;
    platform().AfterAbsRect();
}


/// <summary>
/// Sets the position.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
void LongUI::CUIWindow::SetPos(Point2L pos) noexcept {
    auto& this_pos = reinterpret_cast<Point2L&>(platform().rect.left);
    // 无需移动窗口
    if (this_pos.x == pos.x && this_pos.y == pos.y) return; 
    this_pos = pos;
    platform().AfterPosition();
}


/// <summary>
/// Sets the name of the title.
/// </summary>
/// <param name="name">The name.</param>
/// <returns></returns>
void LongUI::CUIWindow::SetTitleName(CUIString&& name) noexcept {
    platform().titlename = std::move(name);
    platform().AfterTitleName();
}
