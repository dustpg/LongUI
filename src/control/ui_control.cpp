#include <core/ui_window.h>
#include <core/ui_manager.h>
#include <core/ui_ctrlmeta.h>
#include <control/ui_control.h>
#include <container/pod_hash.h>
#include <event/ui_event_host.h>
#include <core/ui_popup_window.h>
#include <core/ui_string.h>
#include <style/ui_native_style.h>
#include <event/ui_initialize_event.h>
#include <input/ui_kminput.h>
//#include <style/ui_attribute.h>

#include "../private/ui_private_control.h"

#include <util/ui_little_math.h>

#include <graphics/ui_bd_renderer.h>
#include <graphics/ui_bg_renderer.h>


#include <algorithm>

// ui namespace
namespace LongUI {
    // remove
    void RemoveTriggered(uintptr_t id, UIControl& ctrl) noexcept;
    // 视口?
    bool IsViewport(const UIControl&) noexcept;
    // 添加视口
    void AddSubViewport(CUIWindow&, UIControl&) noexcept;
    // 删除控件
    void DeleteControl(UIControl* ctrl) noexcept { 
        delete ctrl; 
    }
    // 删除控件
    bool CheckControlDeleteLater(const UIControl& ctrl) noexcept {
        return ctrl.IsDeleteLater();
    }
    // 标记控件被删除
    void MarkControlDeleteLater(UIControl& ctrl) noexcept {
        ctrl.MarkDeleteLater();
    }
    // UIControll类 元信息
    LUI_CONTROL_META_INFO_TOP(UIControl, "ctrl")
}


PCN_NOINLINE
/// <summary>
/// Safes the cast to.
/// </summary>
/// <param name="meta">The meta.</param>
/// <returns></returns>
auto LongUI::UIControl::SafeCastTo(
    const LongUI::MetaControl & meta
) const noexcept -> const UIControl *{
    // 空this指针安全
    if (this) {
        auto info = &m_refMetaInfo;
        while (info) {
            if (info == &meta) return this;
            info = info->super_class;
        }
    }
    return nullptr;
}

#ifndef NDEBUG
/// <summary>
/// Asserts the cast.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::UIControl::AssertCast(const LongUI::MetaControl & meta) const noexcept {
    // 空this指针安全
    if (this) {
        auto info = &m_refMetaInfo;
        while (info) {
            if (info == &meta) return;
            info = info->super_class;
        }
        assert(!"cast failed");
    }
    else {
        LUIDebug(Warning)
            << "try cast null this pointer"
            << LongUI::endl;
    }
}
#endif


/// <summary>
/// need update this
/// </summary>
/// <param name="reason"></param>
/// <returns></returns>
void LongUI::UIControl::NeedUpdate(UpdateReason reason) noexcept {
#ifndef NDEBUG
    //if (LongUI::IsViewport(*this)) {
    //    int bk = 9;
    //}
#endif // NDEBUG

    m_state.reason = m_state.reason | reason;
    UIManager.AddUpdateList(*this);
}

/// <summary>
/// Determines whether [is first child].
/// </summary>
/// <returns></returns>
bool LongUI::UIControl::IsFirstChild() const noexcept {
#ifdef LUI_CONTROL_USE_SINLENODE
    assert(m_pParent && "orphan");
    return this->prev == &m_pParent->m_oHead;
#else
    assert(this->prev && "orphan");
    return this->prev->prev == nullptr;
#endif
}


/// <summary>
/// Determines whether [is last child].
/// </summary>
/// <returns></returns>
bool LongUI::UIControl::IsLastChild() const noexcept {
#ifdef LUI_CONTROL_USE_SINLENODE
    assert(m_pParent && "orphan");
    return this->next == &m_pParent->m_oHead;
#else
    assert(this->next && "orphan");
    return this->next->next == nullptr;
#endif
}

/// <summary>
/// Determines whether [is descendant or sibling for] [the specified control].
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
bool LongUI::UIControl::IsDescendantOrSiblingFor(const UIControl& ctrl) const noexcept {
    if (ctrl.GetParent() == this->GetParent()) return true;
    auto ptr = this;
    while (ptr->GetLevel() > ctrl.GetLevel())
        ptr = ptr->GetParent();
    return ptr == &ctrl;
}

/// <summary>
/// Resizes the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UIControl::resize_child(UIControl& child, Size2F size) noexcept {
    // 无需修改
    if (IsSameInGuiLevel(child.m_oBox.size, size)) return;
    child.mark_world_changed();
    child.NeedUpdate(Reason_SizeChanged);
    child.m_oBox.size = size;
}

/// <summary>
/// Resizes the specified size.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
bool LongUI::UIControl::Resize(Size2F size) noexcept {
    // 无需修改
    if (IsSameInGuiLevel(m_oBox.size, size)) return false;
    // XXX: 如果与父节点布局不相关可以略过?
    if (m_pParent) m_pParent->NeedUpdate(Reason_ChildLayoutChanged);
    // 修改了数据
    this->NeedUpdate(Reason_SizeChanged);
    this->mark_world_changed();
    m_oBox.size = size;
    return true;
}


/// <summary>
/// mark world changed
/// </summary>
/// <returns></returns>
void LongUI::UIControl::mark_world_changed() noexcept {
#ifndef NDEBUG
    //if (!std::strcmp(m_id.id, "btn1")) {
    //    int bk = 9;
    //}
#endif // !NDEBUG
    m_state.world_changed = true;
    this->NeedUpdate(Reason_NonChanged);
}

PCN_NOINLINE
/// <summary>
/// Gets the minimum size.
/// </summary>
/// <returns></returns>
auto LongUI::UIControl::GetMinSize() const noexcept -> Size2F {
    // XXX: 优化为直接返回
    constexpr float IMS = INVALID_MINSIZE;
    const auto s = m_oStyle.minsize;
    const auto b = m_oBox.minsize;
    return Size2F {
        s.width == IMS ? b.width : s.width,
        s.height == IMS ? b.height : s.height,
    };
}

/// <summary>
/// Sets the size of the fixed.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::UIControl::SetFixedSize(Size2F size) noexcept {
    this->SetStyleMaxSize(size);
    this->SetStyleMinSize(size);
    this->Resize(size);
}

/// <summary>
/// Sets the maximum size of the style.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::UIControl::SetStyleMaxSize(Size2F size) noexcept {
    m_oStyle.maxsize = size;
    //UIManager.MarkWindowMinsizeChanged(m_pWindow);
}


/// <summary>
/// Sets the minimum size of the style.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::UIControl::SetStyleMinSize(Size2F size) noexcept {
    m_oStyle.minsize = size;
    this->mark_window_minsize_changed();
}

/// <summary>
/// Sets the minimum size of the box.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::UIControl::set_box_minsize(Size2F size) noexcept {
    m_oBox.minsize = size;
}

/// <summary>
/// Sets the contect minsize.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::UIControl::set_contect_minsize(Size2F size) noexcept {
    const auto nc = m_oBox.GetNonContect();
    this->set_box_minsize({
        size.width + nc.left + nc.right,
        size.height + nc.top + nc.bottom
    });
}

#if 0
/// <summary>
/// Sets the minimum size.
/// 指定最小尺寸
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::UIControl::SpecifyMinSize(Size2F size) noexcept {
    m_oStyle.minsize = size;
    assert(m_oStyle.minsize.width <= m_oStyle.maxsize.width);
    assert(m_oStyle.minsize.height <= m_oStyle.maxsize.height);
    UIManager.MarkWindowMinsizeChanged(m_pWindow);
    auto& s = m_oStyle;
    auto& b = m_oBox;
    b.minsize.width = std::max(s.minsize.width, b.minsize.width);
    b.minsize.height = std::max(s.minsize.height, b.minsize.height);
}

/// <summary>
/// Specifies the minimum size of the contect.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::UIControl::SpecifyMinContectSize(Size2F size) noexcept {
    const auto nc = m_oBox.GetNonContect();
    this->SpecifyMinSize({
        size.width + nc.left + nc.right,
        size.height + nc.top + nc.bottom
    });
}

/// <summary>
/// Specifies the maximum size.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::UIControl::SpecifyMaxSize(Size2F size) noexcept {
    m_oStyle.maxsize = size;
    assert(m_oStyle.minsize.width <= m_oStyle.maxsize.width);
    assert(m_oStyle.minsize.height <= m_oStyle.maxsize.height);
}
#endif





/// <summary>
/// Afters the created.
/// </summary>
/// <returns></returns>
auto LongUI::UIControl::init() noexcept -> Result {
    assert(this && "bad action");
    assert(m_state.inited == false && "this control has been inited");
#ifndef NDEBUG
    //if (m_oStyle.accesskey) {
    //    if (!m_pWindow) 
    //        LUIDebug(Error) 
    //            << "accesskey("
    //            << m_oStyle.accesskey
    //            << ") but no window" 
    //            << endl;
    //}
#endif // !NDEBUG
    // 初始化对象
    EventInitializeArg arg; 
    this->DoEvent(this, arg);
    Result hr = arg.GetResult();
    // 初始化其他
    if (hr) {
        // 检查特殊外貌
        //assert(m_oStyle.appearance != Appearance_ViaParent);
        // 依赖类型初始化控件
        UIManager.RefNativeStyle().InitStyle(*this, m_oStyle.appearance);
        // 重建对象资源
        UIManager.RenderLock();
        hr = this->Recreate(false);
        UIManager.RenderUnlock();
        // 初始化大小
        if (m_oBox.size.width == static_cast<float>(INVALID_CONTROL_SIZE)) {
            this->Resize({ DEFAULT_CONTROL_WIDTH, DEFAULT_CONTROL_HEIGHT });
        }
    }
#ifndef LUI_DISABLE_STYLE_SUPPORT
    // 重新连接样式表
    this->link_style_sheet();
#endif
    // 设置初始化状态
    this->setup_init_state();
    // 初始化完毕
    m_state.inited = true;
    // 链接窗口
    m_pWindow->ControlAttached(*this);
    // TODO: 应该截断错误
    assert(hr);
    return hr;
}

/// <summary>
/// Setups the state of the initialize.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::setup_init_state() noexcept {
    // FIXME: 貌似(?)是错误的实现
    if (m_oStyle.appearance != Appearance_None) {
        // 静止
        if (this->IsDisabled()) {
            auto& naive_style = UIManager.RefNativeStyle();
            const auto color = naive_style.GetFgColor(m_oStyle.state);
            this->SetFgColor({ color });
        }
    }
}

/// <summary>
/// Removes the triggered.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::remove_triggered() noexcept {
#ifndef LUI_DISABLE_STYLE_SUPPORT
    for (const auto& x : m_oStyle.trigger) {
        if (x.tid & 1) LongUI::RemoveTriggered(x.tid - 1, *this);
    }
#endif
}

/// <summary>
/// state changed, update this
/// </summary>
/// <returns></returns>
void LongUI::UIControl::Update(UpdateReason reason) noexcept {
    assert(m_state.inited && "must init control first");
    //if (reason & Reason_WindowChanged)
    //    m_pWindow->ControlAttached(*this);
    // 大小修改
    if (reason & (Reason_SizeChanged | Reason_BoxChanged)) {
        // 需要重绘
        this->Invalidate();
        // 提示样式渲染器大小修改
        this->custom_style_size_changed();
    }
}

/// <summary>
/// Recreates this instance.
/// </summary>
/// <param name="release_only">if set to <c>true</c> [release only].</param>
/// <returns></returns>
auto LongUI::UIControl::Recreate(bool release_only) noexcept -> Result {
    Result hr = { Result::RS_OK };
    // --------------------- 释放数据

#ifndef LUI_DISABLE_STYLE_SUPPORT
    // 背景
    if (m_pBgRender) m_pBgRender->ReleaseDeviceData();
    // 边框
    if (m_pBdRender) m_pBdRender->ReleaseDeviceData();
#endif

    // 仅仅释放
    if (release_only) return hr;


    // --------------------- 创建数据

#ifndef LUI_DISABLE_STYLE_SUPPORT
    // 背景
    if (m_pBgRender) hr = m_pBgRender->CreateDeviceData();
    // 边框
    if (m_pBdRender && hr) hr = m_pBdRender->CreateDeviceData();
#endif

    return{ Result::RS_OK };
}

/// <summary>
/// Gets the subelement.
/// </summary>
/// <param name="name">The name.</param>
/// <returns></returns>
//auto LongUI::UIControl::get_subelement(U8View name) noexcept -> UIControl * {
//    // ::before ::after?
//    return nullptr;
//}

#ifndef NDEBUG
#include <constexpr/const_bkdr.h>
#endif

/// <summary>
/// Parses the specified .
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIControl::add_attribute(uint32_t key, U8View value) noexcept {
// 这会在初始化之前添加, 所以一般直接修改就能达到效果而不需标记更新
#ifndef NDEBUG
    assert(!m_state.inited && "cannot call this after init-ed");
#endif
    constexpr auto BKDR_ID          = 0x0000361f_ui32;
    constexpr auto BKDR_TOP         = 0x001e9951_ui32;
    constexpr auto BKDR_DIR         = 0x001a65b1_ui32;
    constexpr auto BKDR_PACK        = 0x0f1b8d4d_ui32;
    constexpr auto BKDR_FLEX        = 0x0dc767b5_ui32;
    constexpr auto BKDR_LEFT        = 0x0e936497_ui32;
    constexpr auto BKDR_STYLE       = 0xf253f789_ui32;
    constexpr auto BKDR_ALIGN       = 0xb54685e9_ui32;
    constexpr auto BKDR_CLASS       = 0xd85fe06c_ui32;
    constexpr auto BKDR_WIDTH       = 0x370bff82_ui32;
    constexpr auto BKDR_HEIGHT      = 0x28d4978b_ui32;
    constexpr auto BKDR_ORIENT      = 0xeda466cd_ui32;
    constexpr auto BKDR_CHECKED     = 0x091a155f_ui32;
    constexpr auto BKDR_VISIBLE     = 0x646b6442_ui32;
    constexpr auto BKDR_CONTEXT     = 0x89f92d3b_ui32;
    constexpr auto BKDR_TOOLTIP     = 0x9a54b5f3_ui32;
    constexpr auto BKDR_DEFAULT     = 0xdc8b8bf9_ui32;
    //constexpr auto BKDR_TABINDEX    = 0x1c6477b9_ui32;
    constexpr auto BKDR_DISABLED    = 0x715f1adc_ui32;
    constexpr auto BKDR_ACCESSKEY   = 0xba56ab7b_ui32;
    constexpr auto BKDR_DRAGGABLE   = 0xbd13c3b5_ui32;
    constexpr auto BKDR_TOOLTIPTEXT = 0x77a52e88_ui32;

    constexpr auto BKDR_DATAUSER    = 0x5c110136_ui32;
    constexpr auto BKDR_DATAU16     = 0xc036b6f7_ui32;
    constexpr auto BKDR_DATAU8      = 0xe4278072_ui32;
   

    // HASH 一致就认为一致即可
    switch (key)
    {
#ifndef NDEBUG
    case "debug"_bkdr:
        m_state.dbg_output = value.ToBool();
        break;
#endif
    case BKDR_ID:
        // id         : 窗口唯一id
        m_id = UIManager.GetUniqueText(value);
        // 尝试添加命名控件
        m_pWindow->AddNamedControl(*this);
        break;
    case BKDR_LEFT:
        // left
        // TODO: 单位?
        //const auto unit = LongUI::SplitUnit(luiref value);
        m_oBox.pos.x = value.ToFloat();
        break;
    case BKDR_TOP:
        // top
        // TODO: 单位?
        //const auto unit = LongUI::SplitUnit(luiref value);
        m_oBox.pos.y = value.ToFloat();
        break;
    case BKDR_WIDTH:
        // width
        m_oStyle.maxsize.width =
            m_oStyle.minsize.width =
            m_oBox.size.width = value.ToFloat();
        break;
    case BKDR_HEIGHT:
        // height
        m_oStyle.maxsize.height =
            m_oStyle.minsize.height =
            m_oBox.size.width = value.ToFloat();
        break;
#ifndef LUI_DISABLE_STYLE_SUPPORT
    case BKDR_STYLE:
        // style      : 内联样式表
        if (LongUI::ParseInlineStyle(m_oStyle.matched, value))
            m_state.has_inline_style = true;

        break;
#endif
    case BKDR_DISABLED:
        // disabled   : 禁用状态
        //if (value) m_oStyle.state.disabled = true;
        if (value.ToBool())
            m_oStyle.state = m_oStyle.state | State_Disabled;
        break;
    case BKDR_CHECKED:
        // checked
        if (value.ToBool())
            m_oStyle.state = m_oStyle.state | State_Checked;
        break;
    case BKDR_DEFAULT:
        // default    : 窗口初始默认控件
        if (m_pWindow && value.ToBool()) m_pWindow->SetDefault(*this);
        break;
    case BKDR_VISIBLE:
        // visible    : 是否可见
        m_state.visible = value.ToBool();
        break;
    //case BKDR_TABINDEX:
    //    // tabindex   : tab键索引
    //    break;
    case BKDR_CLASS:
        // class      : 样式表用类名
        while (value.begin() < value.end()) {
            const auto splited = value.Split(' ');
            if (splited.end() > splited.begin()) {
                const auto unitext = UIManager.GetUniqueText(splited);
                m_classesStyle.push_back(unitext.id);
            }
        }
        break;
    case BKDR_CONTEXT:
        // context    : 上下文菜单 
        m_pCtxCtrl = UIManager.GetUniqueText(value).id;
        break;
    case BKDR_TOOLTIP:
        // tooltip    : 提示窗口
        m_pTooltipCtrl = UIManager.GetUniqueText(value).id;
        break;
    case BKDR_TOOLTIPTEXT:
        // tooltiptext: 提示文本
        m_strTooltipText = value;
        break;
    case BKDR_ACCESSKEY:
        // accesskey  : 快捷访问键
        if (value.end() > value.begin()) {
            auto ch = *value.begin();
            if (ch >= 'a' && ch <= 'z') ch -= 'a' - 'A';
            m_oStyle.accesskey = ch;
        }
#ifndef NDEBUG
        else LUIDebug(Error) << "bad accesskey" << endl;
#endif // !NDEBUG
        break;
    case BKDR_DRAGGABLE:
        // draggable  : 允许拖拽
        break;
    case BKDR_FLEX:
        // flex       : 布局弹性系数
        m_oStyle.flex = value.ToFloat();
        break;
    case BKDR_ORIENT:
        // orient     : 布局方向
        m_state.orient = *value.begin() == 'v';
        break;
    case BKDR_DIR:
        // dir        : 排列方向
        m_state.direction = *value.begin() == 'r';
        break;
    case BKDR_ALIGN:
        // align      : 布局方向垂直对齐方法
        m_oStyle.align = AttrParser::Align(value);
        break;
    case BKDR_PACK:
        // pack       : 布局方向平行对齐方法
        m_oStyle.pack = AttrParser::Pack(value);
        break;
#ifdef LUI_USER_INIPTR_DATA
    case BKDR_DATAUSER:
        // data-user
        this->user_data = value.ToInt32();
        break;
#endif
#ifdef LUI_USER_U16STR_DATA
    case BKDR_DATAU16:
        // data-u16
        this->user_u16str = CUIString::FromUtf8(value);
        break;
#endif
#ifdef LUI_USER_U8STR_DATA
    case BKDR_DATAU8:
        // data-u8
        this->user_u8str = value;
        break;
#endif
    }
}


PCN_NOINLINE
/// <summary>
/// Determines whether [is point inside border] [the specified position].
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
bool LongUI::UIControl::IsPointInsideBorder(Point2F pos) const noexcept {
    this->MapFromWindow(luiref pos);
    const auto rect = this->RefBox().GetBorderEdge();
    return LongUI::IsInclude(rect, pos);
}

/// <summary>
/// Maps to window.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::UIControl::MapToWindow(RectF& rect) const noexcept {
#ifndef NDEBUG
    if (m_state.dbg_in_update) {
        LUIDebug(Warning) << "cannot call this in update" << endl;
    }
#endif // !NDEBUG
    auto ptr = reinterpret_cast<Point2F*>(&rect);
    ptr[0] = LongUI::TransformPoint(m_mtWorld, ptr[0]);
    ptr[1] = LongUI::TransformPoint(m_mtWorld, ptr[1]);
}

/// <summary>
/// Maps to parent.
/// </summary>
/// <param name="point">The point.</param>
/// <returns></returns>
void LongUI::UIControl::MapToParent(RectF& rect) const noexcept {
#ifndef NDEBUG
    if (m_state.dbg_in_update) {
        LUIDebug(Warning) << "cannot call this in update" << endl;
    }
#endif // !NDEBUG
    rect;
    if (this->IsTopLevel()) {
        assert(!"NOT IMPL");
    }
    else {
        assert(!"NOT IMPL");
    }
}

/// <summary>
/// Maps to window.
/// </summary>
/// <param name="pt">The pt.</param>
/// <returns></returns>
void LongUI::UIControl::MapToWindow(Point2F& pt) const noexcept {
#ifndef NDEBUG
    if (m_state.dbg_in_update) {
        LUIDebug(Warning) << "cannot call this in update" << endl;
    }
#endif // !NDEBUG
    pt = LongUI::TransformPoint(m_mtWorld, pt);
}


/// <summary>
/// Maps from window.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::UIControl::MapFromWindow(RectF& rect) const noexcept {
#ifndef NDEBUG
    if (m_state.dbg_in_update) {
        LUIDebug(Warning) << "cannot call this in update" << endl;
    }
#endif // !NDEBUG
    assert(!"NOT IMPL");
}

/// <summary>
/// Maps from parent.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::UIControl::MapFromParent(RectF & rect) const noexcept {
#ifndef NDEBUG
    if (m_state.dbg_in_update) {
        LUIDebug(Warning) << "cannot call this in update" << endl;
    }
#endif // !NDEBUG
    assert(!"NOT IMPL");
}

/// <summary>
/// Maps from window.
/// </summary>
/// <param name="point">The point.</param>
/// <returns></returns>
void LongUI::UIControl::MapFromWindow(Point2F& point) const noexcept {
#ifndef NDEBUG
    if (m_state.dbg_in_update) {
        LUIDebug(Warning) << "cannot call this in update" << endl;
    }
#endif // !NDEBUG
    point = TransformPointInverse(m_mtWorld, point);
}

/// <summary>
/// Maps from parent.
/// </summary>
/// <param name="point">The point.</param>
/// <returns></returns>
void LongUI::UIControl::MapFromParent(Point2F & point) const noexcept {
#ifndef NDEBUG
    if (m_state.dbg_in_update) {
        LUIDebug(Warning) << "cannot call this in update" << endl;
    }
#endif // !NDEBUG
    point; assert(!"NOT IMPL");
}

/// <summary>
/// Maps to parent.
/// </summary>
/// <param name="point">The point.</param>
/// <returns></returns>
void LongUI::UIControl::MapToParent(Point2F& point) const noexcept {
#ifndef NDEBUG
    if (m_state.dbg_in_update) {
        LUIDebug(Warning) << "cannot call this in update" << endl;
    }
#endif // !NDEBUG
    point; assert(!"NOT IMPL");
}


/// <summary>
/// Controls the making begin.
/// 在大量创建控件前调用此函数
/// </summary>
/// <returns></returns>
void LongUI::UIControl::ControlMakingBegin() noexcept {
#ifdef LUI_USING_CTOR_LOCKER
    UIManager.RefLaterLocker().Lock();
#else
    UIManager.DataLock();
#endif
}

/// <summary>
/// Controls the making end.
/// 在大量创建控件后调用此函数
/// </summary>
/// <returns></returns>
void LongUI::UIControl::ControlMakingEnd() noexcept {
#ifdef LUI_USING_CTOR_LOCKER
    UIManager.RefLaterLocker().Unlock();
#else
    UIManager.DataUnlock();
#endif
}

/// <summary>
/// Initializes a new instance of the <see cref="UIControl"/> class.
/// </summary>
LongUI::UIControl::UIControl(const MetaControl& meta) noexcept : 
m_pParent(nullptr), m_refMetaInfo(meta) {
    Node::next = nullptr;
    Node::prev = nullptr;
    m_ptChildOffset = { 0, 0 };
    m_mtWorld = { 1, 0, 0, 1, 0, 0 };
    // 存在
    m_oBox.Init();
    m_state.Init();
    m_oBox.size = { INVALID_CONTROL_SIZE, INVALID_CONTROL_SIZE };
    // 初始化一般数据
#ifdef LUI_CONTROL_USE_SINLENODE
    const auto pointer = static_cast<UIControl*>(&m_oHead);
    m_oHead = { pointer, pointer };
#else
    m_oHead = { nullptr, static_cast<UIControl*>(&m_oTail) };
    m_oTail = { static_cast<UIControl*>(&m_oHead), nullptr };
#endif
#ifndef NDEBUG
    this->name_dbg = meta.element_name;
    m_state.dbg_output = false;
#endif
}

PCN_NOINLINE
/// <summary>
/// final call for ctor
/// </summary>
/// <param name="parent">The parent</param>
/// <returns></returns>
void LongUI::UIControl::final_ctor(UIControl* parent) noexcept {
    UIControl::ControlMakingBegin();
    UIManager.AddInitList(*this);
#ifdef LUI_USING_CTOR_LOCKER
    static_assert(false, "NOT IMPL");
#endif
    if (parent) parent->add_child(*this);
    UIControl::ControlMakingEnd();
}

/// <summary>
/// make SpTraversal
/// </summary>
/// <param name="sp"></param>
/// <returns></returns>
auto LongUI::UIControl::make_sp_traversal() noexcept -> SpTraversal {
    SpTraversal sp;
    if (m_state.direction) {
        sp.begin = this->rbegin();
        sp.end = this->rend();
        sp.next = offsetof(UIControl, prev);
    }
    else {
        sp.begin = this->begin();
        sp.end = this->end();
        sp.next = offsetof(UIControl, next);
    }
    return sp;
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIControl::DoEvent(UIControl* sender, const EventArg& e) noexcept -> EventAccept {
    // ---------------- 事件处理分支
    switch (e.nevent)
    {
    case LongUI::NoticeEvent::Event_RefreshBoxMinSize:
        this->set_contect_minsize({});
        return Event_Accept;
    }
    return Event_Ignore;
}


/// <summary>
/// Does the input event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIControl::DoInputEvent(InputEventArg e) noexcept -> EventAccept {
    switch (e.event)
    {
    case InputEvent::Event_KeyDown:
        switch (static_cast<CUIInputKM::KB>(e.character))
        {
        case CUIInputKM::KB_LEFT:
        case CUIInputKM::KB_UP:
            // 左/上箭头键
            assert(m_pWindow);
            return EventAccept(m_pWindow->FocusPrev());
        case CUIInputKM::KB_RIGHT:
        case CUIInputKM::KB_DOWN:
            // 右下箭头键
            assert(m_pWindow);
            return EventAccept(m_pWindow->FocusNext());
        case CUIInputKM::KB_SPACE:
            // 持续按下不算
            if (this->IsActive()) return Event_Ignore;
            //LUIDebug(Hint) << this << endl;
            // 相当于按下鼠标左键
            this->StartAnimation({ State_Active, State_Active });
            // 必须可以设为焦点 设为捕捉控件
            m_pWindow->SetCapture(*this);
            return Event_Accept;
        }
        break;
    case InputEvent::Event_KeyUp:
        // XXX: 弹出位置
        switch (static_cast<CUIInputKM::KB>(e.character))
        {
        case CUIInputKM::KB_APPS:
            return LongUI::PopupWindowFromName(
                *this, m_pCtxCtrl, { 0 },
                PopupType::Type_Context,
                AttributePopupPosition::Position_Default
            );
        case CUIInputKM::KB_SPACE:
            //LUIDebug(Hint) << this << endl;
            // 相当于弹起鼠标左键
            this->StartAnimation({ State_Active, State_Non });
            assert(m_pWindow);
#ifndef NDEBUG
            if (!m_pWindow->ReleaseCapture(*this)) {
                assert(!"BUG??");
            }
#endif
            m_pWindow->ForceReleaseCapture();
            // XXX: 调用_onClick?
            return Event_Accept;
        }
    }
    return Event_Ignore;
}

PCN_NOINLINE
/// <summary>
/// Calculates the index of the child.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
auto LongUI::UIControl::cal_child_index(
    const UIControl& ctrl, const MetaControl& meta) const noexcept->uint32_t {
    assert(ctrl.GetParent() == this);
    uint32_t index = 0;
    for (auto& x : *this) {
        if (x == ctrl) break;
        // XXX: 优化
        if (x.SafeCastTo(meta)) ++index;
    }
    return index;
}

PCN_NOINLINE
/// <summary>
/// Calculates child via index. 
/// </summary>
/// <param name="index"></param>
/// <param name="meta"></param>
/// <returns></returns>
auto LongUI::UIControl::cal_index_child(uint32_t index, const MetaControl& meta) noexcept -> UIControl* {
    if (index >= this->GetChildrenCount()) return nullptr;
    if (&meta != &UIControl::s_meta) {
        for (auto& child : (*this)) {
            if (child.SafeCastTo(meta)) {
                if (!index) return &child;
                --index;
            }
        }
        return nullptr;
    }
    auto child = this->begin();
    while (index) { ++child; --index; }
    return child;
    return nullptr;
}

#if 0
/// <summary>
/// Mouses the under atomicity.
/// </summary>
/// <returns></returns>
auto LongUI::UIControl::mouse_under_atomicity(const MouseEventArg& e) noexcept -> EventAccept {
    using Pc = UIControlPrivate;
    // 自己处理消息
    switch (e.type)
    {
    case LongUI::MouseEvent::Event_MouseEnter:
        // 检查是否存在动画
        this->StartAnimation({ StyleStateType::Type_Hover, true });
        this->start_animation_children({ StyleStateType::Type_Hover, true });
        // 截断ENTER ENTER消息
        return Event_Accept;
    case LongUI::MouseEvent::Event_MouseLeave:
        this->StartAnimation({ StyleStateType::Type_Hover, false });
        this->start_animation_children({ StyleStateType::Type_Hover, false });
        this->release_tooltip();
        // 截断ENTER LEAVE消息
        return Event_Accept;
    case LongUI::MouseEvent::Event_MouseIdleHover:
        // 处理Tooltip
        return this->do_tooltip({ e.px, e.py });
    case LongUI::MouseEvent::Event_LButtonDown:
        this->StartAnimation({ StyleStateType::Type_Active, true });
        this->start_animation_children({ StyleStateType::Type_Active, true });
        m_pWindow->SetDefault(*this);
        // 可以设为焦点-设为捕捉控件
        if (this->SetFocus()) m_pWindow->SetCapture(*this);
        return Event_Accept;
    case LongUI::MouseEvent::Event_LButtonUp:
        // 释放捕捉成功: 截断消息, 自下而上释放ACTIVE状态
        if (m_pWindow->ReleaseCapture(*this)) {
            const auto c = this->take_clicked();
            c->start_animation_b2u({ StyleStateType::Type_Active, false });
        }
        // 释放失败: 仅仅释放自己, 继续传递消息
        else {
            this->StartAnimation({ StyleStateType::Type_Active, false });
        }
        this->start_animation_children({ StyleStateType::Type_Active, false });
        return Event_Accept;
    case LongUI::MouseEvent::Event_RButtonUp:
        // 直接调用
        return LongUI::PopupWindowFromName(
            *this, m_pCtxCtrl, { e.px, e.py }, 
            PopupType::Type_Context,
            AttributePopupPosition::Position_Default
        );
    }
    return Event_Ignore;
}
#endif

/// <summary>
/// Does the tooltip.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
auto LongUI::UIControl::do_tooltip(Point2F pos) noexcept -> EventAccept {
    if (m_state.tooltip_shown) return Event_Ignore;
    m_state.tooltip_shown = true;
    const auto ppos = AttributePopupPosition::Position_Default;
    // 显示TOOLTIP
    if (const auto tooltip = m_pTooltipCtrl) {
        constexpr auto type = PopupType::Type_Tooltip;
        return LongUI::PopupWindowFromName(*this, tooltip, pos, type, ppos);
    }
    // 显示TOOLTIP TEXT
    if (!m_strTooltipText.empty()) {
        const auto text = m_strTooltipText.c_str();
        LongUI::PopupWindowFromTooltipText(*this, text, pos, ppos);
        return Event_Accept;
    }
    m_state.tooltip_shown = false;
    return Event_Ignore;
}

/// <summary>
/// Releases the tooltip.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::release_tooltip() noexcept {
    if (m_state.tooltip_shown) {
        m_state.tooltip_shown = false;
        LongUI::PopupWindowCloseTooltip(*this);
    }
}

/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIControl::DoMouseEvent(const MouseEventArg& e) noexcept -> EventAccept {
    using Pc = UIControlPrivate;
    EventAccept s = EventAccept::Event_Ignore;

    // 自己处理消息
    switch (e.type)
    {
    case LongUI::MouseEvent::Event_MouseWheelV:
    case LongUI::MouseEvent::Event_MouseWheelH:
        // 检查指向控件是否处理
        return (m_state.mouse_continue && m_pHovered && m_pHovered->IsEnabled()
            && m_pHovered->DoMouseEvent(e)) ?
            Event_Accept : Event_Ignore;
    case LongUI::MouseEvent::Event_MouseEnter:
        // 检查是否存在动画
        this->StartAnimation({ State_Hover, State_Hover });
        // 截断ENTER ENTER消息
        return Event_Accept;
    case LongUI::MouseEvent::Event_MouseLeave:
        this->StartAnimation({ State_Hover, State_Non });
        // 即便Disabled也可以收到LEAVE/ENTER消息
        if (m_pHovered) {
            UIControlPrivate::DoMouseLeave(*m_pHovered, { e.px, e.py });
            m_pHovered = nullptr;
            this->NeedUpdate(Reason_HoveredChanged);
        }
        // 截断ENTER LEAVE消息
        this->release_tooltip();
        return Event_Accept;
    case LongUI::MouseEvent::Event_MouseMove:
    {
        const auto child = this->FindChild({ e.px, e.py });
        const auto hover = m_pHovered;
        // 即便Disabled也可以收到LEAVE/ENTER消息
        if (hover != child) {
            if (hover) UIControlPrivate::DoMouseLeave(*hover, { e.px, e.py });
            if (child) UIControlPrivate::DoMouseEnter(*child, { e.px, e.py });
            m_pHovered = child;
            this->NeedUpdate(Reason_HoveredChanged);
        }
        if (m_state.mouse_continue && child && child->IsEnabled()) 
            return child->DoMouseEvent(e);
        // 处理MOUSE消息
        return Event_Accept;
    }
    case LongUI::MouseEvent::Event_MouseIdleHover:
        // 子控件优先处理事件
        if (m_state.mouse_continue && m_pHovered && m_pHovered->IsEnabled()) {
            if (m_pHovered->DoMouseEvent(e)) return Event_Accept;
        }
        return this->do_tooltip({ e.px, e.py });
    case LongUI::MouseEvent::Event_LButtonDown:
        s = Event_Accept;
        this->StartAnimation({ State_Active, State_Active });
        m_pWindow->SetDefault(*this);
        m_pWindow->SetFocus(*this);
        // 可以捕捉-设为捕捉控件
        if (m_state.capturable) m_pWindow->SetCapture(*this);
        m_pClicked = m_pHovered;
        break;
    case LongUI::MouseEvent::Event_LButtonUp:
        // 不可用则弃用
        if (m_pClicked && !m_pClicked->IsEnabled()) m_pClicked = nullptr;
        this->StartAnimation({ State_Active, State_Non });
        // 释放捕捉成功: 截断消息
        if (m_pWindow->ReleaseCapture(*this)) {
            //assert(!"NOT IMPL");
            if (m_state.mouse_continue && m_pClicked) 
                m_pClicked->StartAnimation({ State_Active, State_Non });
        }
        // 释放失败:  继续传递消息
        else {
            //this->StartAnimation({ State_Active, State_Non });
            // 存在点击的
            if (m_state.mouse_continue && m_pClicked) 
                m_pClicked->DoMouseEvent(e);
        }
        // 触发[onclick]事件
        this->FireEvent(_onClick());
        return Event_Accept;
    case LongUI::MouseEvent::Event_RButtonUp:
        // 子控件优先处理事件
        if (m_pHovered && m_pHovered->IsEnabled()) {
            if (m_pHovered->DoMouseEvent(e)) return Event_Accept;
        }
        return LongUI::PopupWindowFromName(
            *this, m_pCtxCtrl, { e.px, e.py },
            PopupType::Type_Context,
            AttributePopupPosition::Position_Default
        );
    }
    // 子控件有效则处理消息
    if (auto child = m_pHovered) {
        if (child->IsEnabled()) s = child->DoMouseEvent(e);
    }
    return s;
}

/// <summary>
/// Finalizes an instance of the <see cref="UIControl"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIControl::~UIControl() noexcept {
    m_state.destructing = true;
#ifdef LUI_ACCESSIBLE
    // 释放Accessible接口
    if (m_pAccessible) {
        LongUI::FinalizeAccessible(*m_pAccessible);
        m_pAccessible = nullptr;
    }
#endif
    // 检测计时器
    if (m_state.timer) {
        uint8_t mask = 1;
        for (uint32_t i = 0; i != CONTROL_ALIGNAS; ++i) {
            if (m_state.timer & mask) UIManager.KillTimer(*this, i);
            mask <<= 1;
        }
        m_state.timer = 0;
    }
    // 移除被触发列表
    this->remove_triggered();
    // 清理渲染器
    this->delete_renderer();
    // 移除高层引用
    UIManager.ControlDisattached(*this);
    // 移除窗口引用
    m_pWindow->ControlDisattached(*this);
    // #DTOR# 同样适用
    UIControl::ControlMakingBegin();
    // 清理子节点
    while (begin() != end()) delete begin();
    // 清除父节点中的自己
    if (m_pParent) m_pParent->remove_child(*this);
    // #DTOR# 同样适用
    UIControl::ControlMakingEnd();
#ifndef NDEBUG
    m_pParent = reinterpret_cast<UIControl*>(1);
#endif
}

/// <summary>
/// Deletes the later.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::DeleteLater() noexcept {
    UIManager.DeleteLater(*this);
}

/// <summary>
/// Determines whether [is ancestor from this] [the specified node].
/// </summary>
/// <param name="node">The node.</param>
/// <returns></returns>
bool LongUI::UIControl::IsAncestorForThis(const UIControl& node) const noexcept {
    // 先检查深度做为优化
    if (m_state.level <= node.m_state.level) return false;
    auto ctrl = this;
    // TODO: 没有必要遍历到IsTopLevel
    while (ctrl->m_state.level != node.m_state.level) {
        if (ctrl->m_pParent == &node) return true;
        ctrl = ctrl->m_pParent;
    }
    return false;
}

/// <summary>
/// Sets the parent.
/// </summary>
/// <param name="parent">The parent.</param>
/// <returns></returns>
void LongUI::UIControl::SetParent(UIControl& parent) noexcept {
    assert(this && "bad this ptr");
    parent.add_child(*this);
}

/// <summary>
/// Sets the focus.
/// </summary>
/// <returns></returns>
bool LongUI::UIControl::SetFocus() noexcept {
    return m_pWindow ? m_pWindow->SetFocus(*this) : false;
}


/// <summary>
/// Sets as default and focus.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::SetAsDefaultAndFocus() noexcept {
    if (!m_pWindow) return;
    m_pWindow->SetFocus(*this);
    m_pWindow->SetDefault(*this);
}

/// <summary>
/// Kills the focus.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::KillFocus() noexcept {
    if (m_pWindow) m_pWindow->KillFocus(*this);
}


/// <summary>
/// Swaps the children.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
void LongUI::UIControl::SwapChildren(UIControl& a, UIControl& b) noexcept {
    assert(this && "bad this pointer");
    assert(a.GetParent() == this && "not child for this");
    assert(b.GetParent() == this && "not child for this");
    if (a == b) return;
    this->NeedUpdate(Reason_ChildIndexChanged);
    this->mark_window_minsize_changed();
    UIControl::SwapNode(a, b);
}

/// <summary>
/// Finds the child.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
auto LongUI::UIControl::FindChild(const Point2F pos) noexcept -> UIControl* {
    UIControl* found = nullptr;
    // TODO: 优化 Attachment_Fixed
    for (auto& ctrl : *this) {
#ifndef NDEBUG
        //if (!std::strcmp(ctrl.name_dbg, "button")) {
        //    LUIDebug(Hint)
        //        << "M["
        //        << pos
        //        << "]"
        //        << ctrl.RefBox().visible
        //        << endl;
        //}
#endif
        if (ctrl.IsVisible() && IsInclude(ctrl.RefBox().visible, pos)) {
            if (ctrl.m_state.attachment == Attachment_Fixed)
                return &ctrl;
            else
                found = &ctrl;
        }
    }
    return found;
}

/// <summary>
/// Sets the position.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
void LongUI::UIControl::SetPos(Point2F pos) noexcept {
    if (IsSameInGuiLevel(m_oBox.pos, pos)) return;
    m_oBox.pos = pos;
    this->mark_world_changed();
    this->Invalidate();
}

/// <summary>
/// Sets the tooltip text.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
void LongUI::UIControl::SetTooltipText(U8View view) noexcept {
    m_strTooltipText = view;
    //this->Invalidate();
}

/// <summary>
/// Sets the visible.
/// </summary>
/// <param name="visible">if set to <c>true</c> [visible].</param>
/// <returns></returns>
void LongUI::UIControl::SetVisible(bool visible) noexcept {
    if (this->IsVisible() != visible) {
        m_state.visible = visible;
        if (!m_pParent) return;
        // XXX: 优化其他情况

        // 布局相关
        this->Invalidate();
        if (m_state.attachment == Attachment_Fixed) {

        }
        else {
            m_pParent->NeedUpdate(Reason_ChildLayoutChanged);
            this->mark_window_minsize_changed();
        }
    }
}

/// <summary>
/// Invalidates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::Invalidate() noexcept {
    UIManager.InvalidateControl(*this);
}


/// <summary>
/// Removes the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UIControl::remove_child(UIControl& child) noexcept {
    assert(child.m_pParent == this && "no child for this");
    // 检查数量是否一致
#ifndef NDEBUG
    assert(m_cChildrenCount && "bad child");
    const auto c = std::distance(begin(), end());
    assert(m_cChildrenCount == static_cast<uint32_t>(c));
#endif
    // 修改动态样式

    // 移除直接引用
    if (m_pHovered == &child) m_pHovered = nullptr;
    --m_cChildrenCount;
    // 连接前后节点
    child.prev->next = child.next;
    child.next->prev = child.prev;
#ifndef NDEBUG
    if (!m_cChildrenCount) {
#ifdef LUI_CONTROL_USE_SINLENODE
        assert(m_oHead.prev == &m_oHead);
        assert(m_oHead.prev == m_oHead.next);
#else
        assert(m_oHead.next == &m_oTail);
        assert(m_oTail.prev == &m_oHead);
#endif
    }
#endif
    // 在析构中?
    if (m_state.destructing) return;
    // 要求刷新
    this->mark_window_minsize_changed();
    this->NeedUpdate(Reason_ChildIndexChanged);
}


/// <summary>
/// Determines whether [is excluded from the layout].
/// </summary>
/// <returns></returns>
bool LongUI::UIControl::IsVaildInLayout() const noexcept {
    // 排除: 不可直视
    const bool a = this->IsVisible();
    // 排除: 固定定位
    const bool b = m_state.attachment == Attachment_Scroll;
    // 排除: 相对定位

    // 排除: 绝对定位

    // AND
    return a & b;
}

/// <summary>
/// Adds the style class.
/// </summary>
/// <param name="pair">The pair.</param>
/// <returns></returns>
void LongUI::UIControl::AddStyleClass(U8View pair) noexcept {
    // 可能就几条甚至不超过一条, 直接插入即可
    const auto style_class = UIManager.GetUniqueText(pair).id;
    const auto b = m_classesStyle.begin();
    const auto e = m_classesStyle.end();
    if (std::find(b, e, style_class) == e) 
        m_classesStyle.push_back(style_class);
}

/// <summary>
/// Removes the style class.
/// </summary>
/// <param name="pair">The pair.</param>
/// <returns></returns>
void LongUI::UIControl::RemoveStyleClass(U8View pair) noexcept {
    // 可能就几条甚至不超过一条, 直接删除即可
    const auto style_class = UIManager.GetUniqueText(pair).id;
    const auto b = m_classesStyle.begin();
    const auto e = m_classesStyle.end();
    // NOTE: <UB说明> STL中, 这句是UB(out of range)
    //       不过这是是自己实现的, 所以不是UB
    m_classesStyle.erase(std::find(b, e, style_class));
}

/// <summary>
/// Sets the enabled.
/// </summary>
/// <param name="disabled">if set to <c>true</c> [disabled].</param>
/// <returns></returns>
void LongUI::UIControl::SetDisabled(bool disabled) noexcept {
    // 一样就算了
    if (!!this->IsDisabled() == disabled) return;
    const auto target = disabled ? State_Disabled : State_Non;
    // 禁止的话清除焦点状态
    if (disabled) this->KillFocus();
    // 标记自己和所有后代处于[enable]状态
    this->StartAnimation({ State_Disabled, target });
#if 0
    // 原子控件除外, 因为对外是一个控件
    if (m_state.atomicity) return;
    // 递归调用
    for (auto& child : (*this)) child.SetDisabled(disabled);
#endif
}

/// <summary>
/// Determines whether [is visible to root].
/// </summary>
/// <returns></returns>
bool LongUI::UIControl::IsVisibleEx() const noexcept {
    auto ctrl = this;
    while (!ctrl->IsTopLevel()) {
        if (!ctrl->IsVisible()) return false;
        ctrl = ctrl->GetParent();
    }
    return true;
}

/// <summary>
/// Sets the timer.
/// </summary>
/// <param name="elapse">The elapse.</param>
/// <param name="id0_7">The id0 7.</param>
/// <returns></returns>
void LongUI::UIControl::SetTimer(uint32_t elapse, uint32_t id0_7) noexcept {
    assert(id0_7 < CONTROL_ALIGNAS);
    const uint8_t flag = 1 << id0_7;
    m_state.timer |= flag;
    UIManager.SetTimer(*this, elapse, id0_7);
}

/// <summary>
/// Kills the timer.
/// </summary>
/// <param name="id0_7">The id0 7.</param>
/// <returns></returns>
void LongUI::UIControl::KillTimer(uint32_t id0_7) noexcept {
    assert(id0_7 < CONTROL_ALIGNAS);
    const uint8_t flag = 1 << id0_7;
    assert(m_state.timer & flag);
    m_state.timer &= ~flag;
    UIManager.KillTimer(*this, id0_7);
}

namespace LongUI {
    /// <summary>
    /// Called when [timer].
    /// </summary>
    /// <param name="data">The data.</param>
    /// <returns></returns>
    void OnTimer(uintptr_t data) noexcept {
        constexpr uintptr_t MASK_DTA = CONTROL_ALIGNAS - 1;
        constexpr uintptr_t MASK_PTR = ~MASK_DTA;
        const auto ctrl = reinterpret_cast<UIControl*>(data & MASK_PTR);
        const auto id = static_cast<uint32_t>(data & MASK_DTA);
        const auto eid = static_cast<uint32_t>(NoticeEvent::Event_Timer0) + id;
        EventArg arg = { static_cast<NoticeEvent>(eid) };
        assert(ctrl);
        ctrl->DoEvent(nullptr, arg);
    }
}


/// <summary>
/// Starts the animation.
/// </summary>
/// <param name="change">The change.</param>
/// <returns></returns>
void LongUI::UIControl::StartAnimation(StyleStateChange change) noexcept {
    // 一样就截断
    if (!this->will_change_state(change)) return;
    // 未初始化
    if (!this->is_inited()) {
        this->change_state(change);
    }
    // 非默认控件
    else if (this->RefStyle().appearance == AttributeAppearance::Appearance_None) {
#ifndef LUI_DISABLE_STYLE_SUPPORT
        UIManager.StartExtraAnimation(*this, change);
#else
        m_oStyle.state.Change(change);
#endif
    }
    // 默认控件
    else {
#ifndef LUI_DISABLE_STYLE_SUPPORT
        // 拥有额外的动画?
        if (!m_oStyle.matched.empty()) {
            const auto state = m_oStyle.state;
            UIManager.StartExtraAnimation(*this, change);
            m_oStyle.state = state;
        }
#endif
        UIManager.StartBasicAnimation(*this, change);
    }
    // 继承状态
    for (auto& child : (*this)) {
        if (child.m_oStyle.inherited & change.state_mask)
            child.StartAnimation(change);
    }
}

/// <summary>
/// Clears the parent.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::clear_parent() noexcept {
    // 这条函数(clear_parent)比较重要
    assert(this && "bad this ptr");
    // 清除父节点
    if (m_pParent) {
        m_pParent->remove_child(*this);
        this->NeedUpdate(Reason_ParentChanged);
        m_pParent = nullptr;
    }
}

/// <summary>
/// mark offset_tf
/// </summary>
/// <param name="child"></param>
/// <returns></returns>
void LongUI::UIControl::make_offset_tf_direct(UIControl & child) noexcept {
    assert(child.RefStyle().offset_tf);
    auto& style = child.RefStyle();
    const auto ptr0 = reinterpret_cast<const char*>(&m_oStyle);
    const auto ptr1 = reinterpret_cast<const char*>(&style);
    const auto offset = static_cast<uintptr_t>(ptr1 - ptr0);
    assert(offset < 0xffff && "out of range");
    m_oStyle.offset_tf = offset + style.offset_tf ;
}

/// <summary>
/// Marks the window minsize changed.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::mark_window_minsize_changed() noexcept {
    UIManager.MarkWindowMinsizeChanged(m_pWindow);
}



/// <summary>
/// Adds the child.
/// </summary>
/// <remarks>这条函数非常重要</remarks>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UIControl::add_child(UIControl& child) noexcept {
    // 视口?
    if (LongUI::IsViewport(child)) {
        assert(m_pWindow && "add subwindow must be vaild window");
        LongUI::AddSubViewport(*m_pWindow, child);
        return;
    }
    // 无需再次添加
    if (child.m_pParent == this) return;
    // 鼠标信息隔断默认会继承所有状态
    if (!m_state.mouse_continue) child.RefInheritedMask() = State_MouseCutInher;
    // 标记
    child.NeedUpdate(Reason_ParentChanged);
    this->NeedUpdate(Reason_ChildIndexChanged);
    this->mark_window_minsize_changed();
    // 在之前的父控件移除该控件
    if (child.m_pParent) child.m_pParent->remove_child(child);
    child.m_pParent = this;
    ++m_cChildrenCount;
    // 连接前后节点
#ifdef LUI_CONTROL_USE_SINLENODE
    m_oHead.prev->next = &child;
    child.prev = m_oHead.prev;
    child.next = static_cast<UIControl*>(&m_oHead);
    m_oHead.prev = &child;
#else
    m_oTail.prev->next = &child;
    child.prev = m_oTail.prev;
    child.next = static_cast<UIControl*>(&m_oTail);
    m_oTail.prev = &child;
#endif
    // 同步
    UIControl::sync_data(child, *this);
    // 提示管理器新的控件被添加到控件树中
    //UIManager.ControlAttached(child);
}

inline
/// <summary>
/// set new window - force
/// </summary>
/// <returns></returns>
void LongUI::UIControl::set_new_window(CUIWindow* window) noexcept {
    if (m_pWindow == window) return;
    const auto prev = m_pWindow;
    // 设置新的窗口
    m_pWindow = window;
    if (this->is_inited()) {
        // 移除之前的窗口引用
        prev->ControlDisattached(*this);
        // 添加新的窗口引用
        window->ControlAttached(*this);
        // 标记窗口修改
        this->NeedUpdate(Reason_WindowChanged);
    }
}


/// <summary>
/// Updates the level.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::UIControl::sync_data(UIControl& ctrl, UIControl& parent) noexcept {
    // 窗口
    ctrl.set_new_window(parent.m_pWindow);
    ctrl.m_state.level = parent.m_state.level + 1;
#ifndef NDEBUG
    // 树节点深度深度过大
    const uint32_t lv = ctrl.GetLevel();
    if (lv >= (MAX_CONTROL_TREE_DEPTH / 2)) {
        if (lv == (MAX_CONTROL_TREE_DEPTH - 2))
            LUIDebug(Error) << "Tree to deep" << lv << endl;
        else
            LUIDebug(Warning) << "Tree to deep" << lv << endl;
    }
#endif
    assert(ctrl.m_pCtxCtrl == nullptr && "TODO: move to global");
    for (auto& child : ctrl) UIControl::sync_data(child, ctrl);
}

#ifndef LUI_DISABLE_STYLE_SUPPORT
/// <summary>
/// Resets the style sheet.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::link_style_sheet() noexcept {
#ifndef NDEBUG
    if (UIManager.flag & ConfigureFlag::Flag_DbgNoLinkStyle)
        return;
#endif // !NDEBUG
    auto& style_matched = m_oStyle.matched;
    // 移除被触发列表
    this->remove_triggered();
    // 最高支持32(默认)枚内联样式
    SSValue vbuf[SMALL_BUFFER_LENGTH]; uint32_t inline_size = 0;
    // 处理之前的内联样式
    if (m_state.has_inline_style) {
        auto last = &style_matched.back();
        while (true) {
            assert(last >= &style_matched.front());
            if (last->type == ValueType::Type_NewOne) break;
            --last;
        }
        constexpr uint32_t len = sizeof(vbuf) / sizeof(vbuf[0]);
        auto& pcl = *reinterpret_cast<SSValuePCL*>(last);
        inline_size = std::min(pcl.length, len);
        std::memcpy(vbuf, last, inline_size * sizeof(vbuf[0]));
#ifndef NDEBUG
        if (pcl.length != inline_size) {
            LUIDebug(Warning)
                << "inline style num. greater than "
                << inline_size
                << "  : "
                << pcl.length
                << endl;
        }
#endif // !NDEBUG
        pcl.length = inline_size;
    }
    // DOWN-RESIZE
    style_matched.clear();
    // 先连接全局的
    LongUI::MatchStyleSheet(*this, UIManager.GetStyleSheet());
    // 最高支持32(默认)层窗口
    CUIWindow* buf[SMALL_BUFFER_LENGTH];
    auto itr = buf;
    // 生成列表
    {
        auto window = m_pWindow;
        const auto end_buf = buf + sizeof(buf) / sizeof(buf[0]);
        while (window) {
            *itr = window;
            window = window->GetParent();
            // 越界保护
            if (++itr == end_buf) break;
        }
    }
    // 再逆序连接窗口的样式表
    while (itr != buf) {
        --itr; const auto window = *itr;
        LongUI::MatchStyleSheet(*this, window->GetStyleSheet());
    }
    // 处理内联样式
    if (inline_size) {
        // 适应内联数据
        for (uint32_t i = 1; i != inline_size; ++i) 
            this->ApplyValue(vbuf[i]);
        style_matched.insert(style_matched.end(), vbuf, vbuf + inline_size);
    }
}


#endif

// longui::detail namespace
namespace LongUI { namespace detail {
    // attribute write
    template<typename T, typename U>
    static inline void write_value(T& a, U b) noexcept {
        static_assert(sizeof(T) == sizeof(U), "must be same");
        a = static_cast<T>(b);
    }
    // same cast
    template<typename T, typename U>
    static inline T same_cast(U a) noexcept {
        static_assert(sizeof(T) == sizeof(U), "must be same");
        return static_cast<T>(a);
    }
}}

#ifndef LUI_DISABLE_STYLE_SUPPORT
PCN_NOINLINE
/// <summary>
/// Applies the value.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIControl::ApplyValue(const SSValue& value) noexcept {
    switch (value.type)
    {
        RectF tmp_rect;
    default:
        assert(!"unsupported value type");
        break;
    case ValueType::Type_PositionOverflowX:
        detail::write_value(m_oStyle.overflow_x, value.data4.byte);
        break;
    case ValueType::Type_PositionOverflowY:
        detail::write_value(m_oStyle.overflow_y, value.data4.byte);
        break;
    case ValueType::Type_PositionLeft:
        detail::write_value(m_oBox.pos.x, value.data4.single);
        this->mark_world_changed();
        break;
    case ValueType::Type_PositionTop:
        detail::write_value(m_oBox.pos.y, value.data4.single);
        this->mark_world_changed();
        break;
    case ValueType::Type_DimensionWidth:
        detail::write_value(m_oStyle.minsize.width, value.data4.single);
        detail::write_value(m_oStyle.maxsize.width, value.data4.single);
        detail::write_value(m_oBox.size.width, value.data4.single);
        this->mark_window_minsize_changed();
        this->NeedUpdate(Reason_SizeChanged);
        break;
    case ValueType::Type_DimensionHeight:
        detail::write_value(m_oStyle.minsize.height, value.data4.single);
        detail::write_value(m_oStyle.maxsize.height, value.data4.single);
        detail::write_value(m_oBox.size.height, value.data4.single);
        this->mark_window_minsize_changed();
        this->NeedUpdate(Reason_SizeChanged);
        break;
    case ValueType::Type_DimensionMinWidth:
        detail::write_value(m_oStyle.minsize.width, value.data4.single);
        this->mark_window_minsize_changed();
        break;
    case ValueType::Type_DimensionMinHeight:
        detail::write_value(m_oStyle.minsize.height, value.data4.single);
        this->mark_window_minsize_changed();
        break;
    case ValueType::Type_DimensionMaxWidth:
        //if (detail::is_percent_value(value.data4.single)) {
        //    auto p = detail::get_percent_value(value.data4.single);
        //    if (m_pParent) p *= m_pParent->GetSize().width;
        //    detail::write_value(m_oStyle.maxsize.width, p);
        //}
        detail::write_value(m_oStyle.maxsize.width, value.data4.single);
        if (m_pParent) m_pParent->NeedUpdate(Reason_ChildLayoutChanged);
        break;
    case ValueType::Type_DimensionMaxHeight:
        detail::write_value(m_oStyle.maxsize.height, value.data4.single);
        if (m_pParent) m_pParent->NeedUpdate(Reason_ChildLayoutChanged);
        break;
    case ValueType::Type_BoxFlex:
        detail::write_value(m_oStyle.flex, value.data4.single);
        if (m_pParent) m_pParent->NeedUpdate(Reason_ChildLayoutChanged);
        break;
    case ValueType::Type_BorderStyle:
        this->SetBdStyle(detail::same_cast<AttributeBStyle>(value.data4.byte));
        break;
    case ValueType::Type_BorderColor:
        this->SetBdColor({ value.data4.u32 });
        break;
    case ValueType::Type_BorderRadius:
        this->SetBdRadius({ value.data8.single[0], value.data8.single[1] });
        break;
    case ValueType::Type_BorderImageSource:
        this->SetBdImageSourceID(value.data8.handle);
        break;
    case ValueType::Type_BorderImageSlice:
        LongUI::UniByte8ToSliceRect(value.data8, &tmp_rect.left);
        this->SetBdImageSlice(tmp_rect, value.data4.boolean);
        break;
    case ValueType::Type_BorderImageRepeat:
        this->SetBdImageRepeat(detail::same_cast<AttributeRepeat>(value.data4.byte));
        break;
    case ValueType::Type_BackgroundColor:
        this->SetBgColor({ value.data4.u32 });
        break;
    case ValueType::Type_BackgroundImage:
        this->SetBgImageID({ value.data8.handle });
        break;
    case ValueType::Type_BackgroundRepeat:
        this->SetBgRepeat(detail::same_cast<AttributeRepeat>(value.data4.byte));
        break;
    case ValueType::Type_BackgroundClip:
        this->SetBgClip(detail::same_cast<AttributeBox>(value.data4.byte));
        break;
    case ValueType::Type_BackgroundOrigin:
        this->SetBgOrigin(detail::same_cast<AttributeBox>(value.data4.byte));
        break;
    case ValueType::Type_TransitionDuration:
        using dur_t = decltype(m_oStyle.tduration);
        assert(value.data4.single < 65.5f && "out of range");
        m_oStyle.tduration = static_cast<dur_t>(value.data4.single * 1000.f);
        break;
    case ValueType::Type_TransitionTimingFunc:
        detail::write_value(m_oStyle.tfunction, value.data4.byte);
        break;
    case ValueType::Type_TextColor:
        this->SetFgColor({ value.data4.u32 });
        break;
    case ValueType::Type_TextAlign:
        this->SetTextAlign(detail::same_cast<AttributeTextAlign>(value.data4.byte));
        break;
    case ValueType::Type_WKTextStrokeWidth:
        this->SetTextStrokeWidth({ value.data4.single });
        break;
    case ValueType::Type_WKTextStrokeColor:
        this->SetTextStrokeColor({ value.data4.u32 });
        break;
    case ValueType::Type_FontSize:
        this->SetFontSize({ value.data4.single });
        break;
    case ValueType::Type_FontStyle:
        this->SetFontStyle(detail::same_cast<AttributeFontStyle>(value.data4.byte));
        break;
    case ValueType::Type_FontStretch:
        this->SetFontStretch(detail::same_cast<AttributeFontStretch>(value.data4.byte));
        break;
    case ValueType::Type_FontWeight:
        this->SetFontWeight(detail::same_cast<AttributeFontWeight>(value.data4.word));
        break;
    case ValueType::Type_FontFamily:
        this->SetFontFamily(value.data8.strptr);
        break;
    case ValueType::Type_MarginTop:
        this->SetMarginTop(value.data4.single);
        break;
    case ValueType::Type_MarginRight:
        this->SetMarginRight(value.data4.single);
        break;
    case ValueType::Type_MarginBottom:
        this->SetMarginBottom(value.data4.single);
        break;
    case ValueType::Type_MarginLeft:
        this->SetMarginLeft(value.data4.single);
        break;
    case ValueType::Type_PaddingTop:
        this->SetPaddingTop(value.data4.single);
        break;
    case ValueType::Type_PaddingRight:
        this->SetPaddingRight(value.data4.single);
        break;
    case ValueType::Type_PaddingBottom:
        this->SetPaddingBottom(value.data4.single);
        break;
    case ValueType::Type_PaddingLeft:
        this->SetPaddingLeft(value.data4.single);
        break;
    case ValueType::Type_BorderTopWidth:
        this->SetBorderTop(value.data4.single);
        break;
    case ValueType::Type_BorderRightWidth:
        this->SetBorderRight(value.data4.single);
        break;
    case ValueType::Type_BorderBottomWidth:
        this->SetBorderBottom(value.data4.single);
        break;
    case ValueType::Type_BorderLeftWidth:
        this->SetBorderLeft(value.data4.single);
        break;
    case ValueType::Type_LUIAppearance:
        detail::write_value(m_oStyle.appearance, value.data4.byte);
#if 0
        if (m_oStyle.appearance == Appearance_None)
            this->ClearAppearance();
        else
            LongUI::NativeStyleInit(*this, m_oStyle.appearance);
#endif
        break;
    }
}


PCN_NOINLINE
/// <summary>
/// Gets the value.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIControl::GetValue(SSValue& value) const noexcept {
    switch (value.type)
    {
        RectF tmp_rect; Size2F tmp_size;
    default:
        assert(!"unsupported value type");
        break;
    case ValueType::Type_PositionOverflowX:
        detail::write_value(value.data4.byte, m_oStyle.overflow_x);
        break;
    case ValueType::Type_PositionOverflowY:
        detail::write_value(value.data4.byte, m_oStyle.overflow_y);
        break;
    case ValueType::Type_PositionLeft:
        detail::write_value(value.data4.single, m_oBox.pos.x);
        break;
    case ValueType::Type_PositionTop:
        detail::write_value(value.data4.single, m_oBox.pos.y);
        break;
    case ValueType::Type_DimensionWidth:
        detail::write_value(value.data4.single, m_oBox.size.width);
        break;
    case ValueType::Type_DimensionHeight:
        detail::write_value(value.data4.single, m_oBox.size.width);
        break;
    case ValueType::Type_DimensionMinWidth:
        detail::write_value(value.data4.single, m_oStyle.minsize.width);
        break;
    case ValueType::Type_DimensionMinHeight:
        detail::write_value(value.data4.single, m_oStyle.minsize.height);
        break;
    case ValueType::Type_DimensionMaxWidth:
        //detail::write_value(value.data4.single, detail::mark_percent_from1(
            //m_oStyle.maxsize.width / m_pParent->GetSize().width));
        detail::write_value(value.data4.single, m_oStyle.maxsize.width);
        break;
    case ValueType::Type_DimensionMaxHeight:
        detail::write_value(value.data4.single, m_oStyle.maxsize.height);
        break;
    case ValueType::Type_BoxFlex:
        detail::write_value(value.data4.single, m_oStyle.flex);
        break;
    case ValueType::Type_BorderStyle:
        detail::write_value(value.data4.byte, this->GetBdStyle());
        break;
    case ValueType::Type_BorderColor:
        detail::write_value(value.data4.u32, this->GetBdColor().primitive);
        break;
    case ValueType::Type_BorderRadius:
        tmp_size = this->GetBdRadius();
        detail::write_value(value.data8.single[0], tmp_size.width);
        detail::write_value(value.data8.single[1], tmp_size.height);
        break;
    case ValueType::Type_BorderImageSource:
        detail::write_value(value.data8.handle, this->GetBdImageSourceID());
        break;
    case ValueType::Type_BorderImageSlice:
        detail::write_value(value.data4.boolean, this->GetBdImageSlice(tmp_rect));
        LongUI::SliceRectToUniByte8(&tmp_rect.left, value.data8);
        break;
    case ValueType::Type_BorderImageRepeat:
        detail::write_value(value.data4.byte, this->GetBdImageRepeat());
        break;
    case ValueType::Type_BackgroundColor:
        detail::write_value(value.data4.u32, this->GetBgColor().primitive);
        break;
    case ValueType::Type_BackgroundImage:
        detail::write_value(value.data8.handle, this->GetBgImageID());
        break;
    case ValueType::Type_BackgroundRepeat:
        detail::write_value(value.data4.byte, this->GetBgRepeat());
        break;
    case ValueType::Type_BackgroundClip:
        detail::write_value(value.data4.byte, this->GetBgClip());
        break;
    case ValueType::Type_BackgroundOrigin:
        detail::write_value(value.data4.byte, this->GetBgOrigin());
        break;
    case ValueType::Type_TransitionDuration:
        value.data4.single = static_cast<float>(m_oStyle.tduration) * 1000.f;
        break;
    case ValueType::Type_TransitionTimingFunc:
        detail::write_value(value.data4.byte, m_oStyle.tfunction);
        break;
    case ValueType::Type_TextColor:
        detail::write_value(value.data4.u32, this->GetFgColor().primitive);
        break;
    case ValueType::Type_TextAlign:
        detail::write_value(value.data4.byte, this->GetTextAlign());
        break;
    case ValueType::Type_WKTextStrokeWidth:
        detail::write_value(value.data4.single, this->GetTextStrokeWidth());
        break;
    case ValueType::Type_WKTextStrokeColor:
        detail::write_value(value.data4.u32, this->GetTextStrokeColor().primitive);
        break;
    case ValueType::Type_FontSize:
        detail::write_value(value.data4.single, this->GetFontSize());
        break;
    case ValueType::Type_FontStyle:
        detail::write_value(value.data4.byte, this->GetFontStyle());
        break;
    case ValueType::Type_FontStretch:
        detail::write_value(value.data4.byte, this->GetFontStretch());
        break;
    case ValueType::Type_FontWeight:
        detail::write_value(value.data4.word, this->GetFontWeight());
        break;
    case ValueType::Type_FontFamily:
        detail::write_value(value.data8.strptr, this->GetFontFamily());
        break;
    case ValueType::Type_LUIAppearance:
        detail::write_value(value.data4.byte, m_oStyle.appearance);
        break;
    case ValueType::Type_MarginTop:
        detail::write_value(value.data4.single, this->RefBox().margin.top);
        break;
    case ValueType::Type_MarginRight:
        detail::write_value(value.data4.single, this->RefBox().margin.right);
        break;
    case ValueType::Type_MarginBottom:
        detail::write_value(value.data4.single, this->RefBox().margin.bottom);
        break;
    case ValueType::Type_MarginLeft:
        detail::write_value(value.data4.single, this->RefBox().margin.left);
        break;
    case ValueType::Type_PaddingTop:
        detail::write_value(value.data4.single, this->RefBox().padding.top);
        break;
    case ValueType::Type_PaddingRight:
        detail::write_value(value.data4.single, this->RefBox().padding.right);
        break;
    case ValueType::Type_PaddingBottom:
        detail::write_value(value.data4.single, this->RefBox().padding.bottom);
        break;
    case ValueType::Type_PaddingLeft:
        detail::write_value(value.data4.single, this->RefBox().padding.left);
        break;
    case ValueType::Type_BorderTopWidth:
        detail::write_value(value.data4.single, this->RefBox().border.top);
        break;
    case ValueType::Type_BorderRightWidth:
        detail::write_value(value.data4.single, this->RefBox().border.right);
        break;
    case ValueType::Type_BorderBottomWidth:
        detail::write_value(value.data4.single, this->RefBox().border.bottom);
        break;
    case ValueType::Type_BorderLeftWidth:
        detail::write_value(value.data4.single, this->RefBox().border.left);
        break;
    }
}
#endif

/// <summary>
/// Sets the xul.
/// </summary>
/// <param name="xul">The xul string.</param>
/// <remarks>
/// SetXul accept null-terminated-string only
/// SetXul目前只接受 NUL 结尾字符串
/// </remarks>
/// <returns></returns>
void LongUI::UIControl::SetXul(const char* xul) noexcept {
    UIControl::ControlMakingBegin();
    CUIControlControl::MakeXul(*this, xul);
    UIControl::ControlMakingEnd();
}

/// <summary>
/// Sets the xul from file.
/// </summary>
/// <param name="url">The URL.</param>
/// <returns></returns>
bool LongUI::UIControl::SetXulFromFile(U8View url) noexcept {
#ifndef NDEBUG
    if (UIManager.GetXulDir().size()) {
        LUIDebug(Error)
            << "you cannot set xul dir out of SetXulFromFile"
            << endl;
    }
#endif
    POD::Vector<uint8_t> buffer;
    UIManager.LoadDataFromUrl(url, buffer);
    if (const auto len = buffer.size()) {
        UIManager.SetXulDir(LongUI::FindLastDir(url));
        {
            // TODO: NUL-结尾字符串
            buffer.resize(len + 1);
            const auto ptr = &buffer.front();
            const auto str = reinterpret_cast<const char*>(ptr);
            this->SetXul(str);
        }
        UIManager.SetXulDir({});
        return true;
    }
    return false;
}


PCN_NOINLINE
/// <summary>
/// Clears the application.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::ClearAppearance() noexcept {
    m_oStyle.appearance = Appearance_None;
    m_oBox.margin = {};
    m_oBox.border = {};
    m_oBox.padding = {};
}

#ifdef LUI_ACCESSIBLE
#include <accessible/ui_accessible_event.h>
#include <core/ui_string.h>

/// <summary>
/// Accessibles the specified .
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
auto LongUI::UIControl::accessible(const AccessibleEventArg& args) noexcept -> EventAccept {
    switch (args.event)
    {
        using get0_t = AccessibleGetPatternsArg;
        using get1_t = AccessibleGetAccNameArg;
        using getd_t = AccessibleGetDescriptionArg;
    case AccessibleEvent::Event_GetPatterns:
        // 获取行为模式
        static_cast<const get0_t&>(args).patterns |= Pattern_None;
        return Event_Accept;
    case AccessibleEvent::Event_All_GetAccessibleName:
        // 获取Acc名称
        *static_cast<const get1_t&>(args).name =
#ifdef NDEBUG
            CUIString::FromUtf8(!m_id.id[0] ? m_refMetaInfo.element_name : m_id.id);
#else
            CUIString::FromUtf8(!m_id.id[0] ? this->name_dbg : m_id.id);
#endif
        return Event_Accept;
    case AccessibleEvent::Event_All_GetDescription:
        // 获取描述字符串
    {
        const auto name = CUIString::FromUtf8(this->GetID().id);
        static_cast<const getd_t&>(args).description->format(
            u"element<%hs> id<%ls>",
            m_refMetaInfo.element_name,
            name.c_str()
        );
        return Event_Accept;
    }
    }
    return Event_Ignore;
}

#endif
