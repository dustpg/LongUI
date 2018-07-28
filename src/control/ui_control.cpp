#include <core/ui_window.h>
#include <core/ui_manager.h>
#include <core/ui_ctrlmeta.h>
#include <control/ui_control.h>
#include <container/pod_hash.h>
#include <event/ui_event_host.h>
#include <core/ui_popup_window.h>
#include <style/ui_native_style.h>
#include <event/ui_initialize_event.h>
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
    // 计时器函数
    //void OnTimer(UIControl& ctrl, uint32_t id) noexcept {
    //    assert(id < 4 && "bad id");
    //    constexpr auto eid = static_cast<uint32_t>(NoticeEvent::Event_Timer0);
    //    const auto nid = static_cast<NoticeEvent>(eid + id);
    //    ctrl.DoEvent(nullptr, { nid });
    //}
    // 删除控件
    bool CheckControlDeleteLater(const UIControl& ctrl) noexcept {
        return ctrl.IsDeleteLater();
    }
    // 标记控件被删除
    void MarkControlDeleteLater(UIControl& ctrl) noexcept {
        ctrl.MarkDeleteLater();
    }
    // 创建UIControl
    static UIControl* create_UIControl(UIControl* p) noexcept {
        return new(std::nothrow) UIControl{ p };
    }
    // UIControll类 元信息
    const MetaControl UIControl::s_meta = {
        nullptr,
        "ctrl",
        create_UIControl
    };
    // private 实现
    //struct UIControl::Private { };
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
/// Needs the update.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::NeedUpdate() noexcept {
    assert(this && "null this pointer");
    UIManager.AddUpdateList(*this);
}

/// <summary>
/// Nexts the update.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::NextUpdate() noexcept {
    assert(this && "null this pointer");
    UIManager.AddNextUpdateList(*this);
}

/// <summary>
/// Needs the relayout.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::NeedRelayout() noexcept {
    m_state.dirty = true; 
    this->NeedUpdate();
}

/// <summary>
/// Determines whether [is first child].
/// </summary>
/// <returns></returns>
bool LongUI::UIControl::IsFirstChild() const noexcept {
    return this->prev->prev == nullptr;
}


/// <summary>
/// Determines whether [is last child].
/// </summary>
/// <returns></returns>
bool LongUI::UIControl::IsLastChild() const noexcept {
    return this->next->next == nullptr;
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
/// Sets the timer[0123]
/// </summary>
/// <param name="id0123">The id0123.</param>
/// <param name="elapse">The elapse.</param>
/// <returns></returns>
//void LongUI::UIControl::SetTimer0123(uint32_t id0123, uint32_t elapse) noexcept {
//    m_bHasTimer = true;
//}

/// <summary>
/// Kills the timer[0123]
/// </summary>
/// <param name="id0123">The id0123.</param>
/// <param name="elapse">The elapse.</param>
/// <returns></returns>
//void LongUI::UIControl::KillTimer0123(uint32_t id0123) noexcept {
//
//}

/// <summary>
/// Resizes the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UIControl::resize_child(UIControl& child, Size2F size) noexcept {
    // 无需修改
    if (IsSameInGuiLevel(child.m_oBox.size, size)) return;
    // XXX: 需要修改世界矩阵
    child.m_state.world_changed = true;
    // 需要重新布局
    child.NeedRelayout();
    // 重新渲染
    child.Invalidate();
    // 确定修改
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
    // 修改布局
    //const auto layout = this->IsTopLevel() ? this : m_pParent;
    if (m_pParent) m_pParent->NeedRelayout();
    this->NeedRelayout();
    // XXX: 需要修改世界矩阵
    m_state.world_changed = true;
    // 修改了数据
    m_oBox.size = size;
    return true;
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
/// Starts the animation bottom up.
/// </summary>
/// <param name="c">The c.</param>
/// <returns></returns>
void LongUI::UIControl::start_animation_b2u(StyleStateTypeChange c) noexcept {
    this->StartAnimation(c);
    if (!this->IsTopLevel()) m_pParent->start_animation_b2u(c);
}

/// <summary>
/// Starts the animation u2p.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::UIControl::start_animation_children(StyleStateTypeChange c) noexcept {
    for (auto& child : *this) child.StartAnimation(c);
}

/// <summary>
/// Finds the clicked.
/// </summary>
/// <returns></returns>
auto LongUI::UIControl::take_clicked() noexcept->UIControl* {
    auto ctrl = this;
    while (ctrl->m_pClicked) {
        const auto last = ctrl;
        ctrl = ctrl->m_pClicked;
        last->m_pClicked = nullptr;
    }
    return ctrl;
}



/// <summary>
/// Afters the created.
/// </summary>
/// <returns></returns>
auto LongUI::UIControl::init() noexcept -> Result {
    // 空指针
    if (!this) return{ Result::RE_OUTOFMEMORY };
    assert(m_state.inited == false && "this control has been inited");
    m_state.inited = true;
    // XXX: 注册访问按键
    if (m_chAccessKey >= 'A' && m_chAccessKey <= 'Z' && m_pWindow)
        m_pWindow->RegisterAccessKey(*this);
    // 初始化对象
    EventInitializeArg arg; 
    this->DoEvent(this, arg);
    Result hr = arg.GetResult();
    // 初始化其他
    if (hr) {
        // 设置默认样式
        constexpr auto defapp = Appearance_None;
        UIControlPrivate::SetAppearanceIfNotSet(*this, defapp);
        // 依赖类型初始化控件
        LongUI::NativeStyleInit(*this, this->GetStyle().appearance);
        // 重建对象
        hr = this->Recreate(false);
        // 初始化大小
        if (m_oBox.size.width == static_cast<float>(INVALID_CONTROL_SIZE)) {
            this->Resize({ DEFAULT_CONTROL_WIDTH, DEFAULT_CONTROL_HEIGHT });
        }
#ifndef LUI_DISABLE_STYLE_SUPPORT
        // 重新连接样式表
        this->link_style_sheet();
#endif
    }
    // 设置初始化状态
    this->setup_init_state();

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
        if (m_oStyle.state.disabled) {
            const auto color = LongUI::NativeFgColor(m_oStyle.state);
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
/// Updates the self.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::Update() noexcept {
    // 调用update前必须初始化
    assert(m_state.inited && "must init control first");
    // 状态修改
    m_state.style_state_changed = false;
    m_state.textfont_display_changed = false;
    m_state.textfont_layout_changed = false;
    // 最基的不处理子控件索引更改
    m_state.child_i_changed = false;
    m_state.parent_changed = false;

    /*LUIDebug(Hint) LUI_FRAMEID
        << this 
        << LongUI::endl;*/
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
   
    // 优化用HASH掩码
    constexpr auto MASK_HASH        = 0xffffffff_ui32;
    //constexpr auto MASK_HASH        = 0x00000fff_ui32;

    // HASH 一致就认为一致即可
    switch (key & MASK_HASH)
    {
    case BKDR_ID & MASK_HASH:
        // id         : 窗口唯一id
        m_id = UIManager.GetUniqueText(value);
        // 尝试添加命名控件
        if (m_pWindow) m_pWindow->AddNamedControl(*this);
        break;
    case BKDR_LEFT & MASK_HASH:
        // left
        // TODO: 单位?
        //const auto unit = LongUI::SplitUnit(luiref value);
        m_oBox.pos.x = value.ToFloat();
        break;
    case BKDR_TOP & MASK_HASH:
        // top
        // TODO: 单位?
        //const auto unit = LongUI::SplitUnit(luiref value);
        m_oBox.pos.y = value.ToFloat();
        break;
    case BKDR_WIDTH & MASK_HASH:
        // width
        m_oStyle.maxsize.width =
            m_oStyle.minsize.width =
            m_oBox.size.width = value.ToFloat();
        break;
    case BKDR_HEIGHT & MASK_HASH:
        // height
        m_oStyle.maxsize.height =
            m_oStyle.minsize.height =
            m_oBox.size.width = value.ToFloat();
        break;
#ifndef LUI_DISABLE_STYLE_SUPPORT
    case BKDR_STYLE & MASK_HASH:
        // style      : 内联样式表
        if (LongUI::ParseInlineStlye(m_oStyle.matched, value))
            m_bHasInlineStyle = true;

        break;
#endif
    case BKDR_DISABLED & MASK_HASH:
        // disabled   : 禁用状态
        //if (value) m_oStyle.state.disabled = true;
        m_oStyle.state.disabled = value.ToBool();
        break;
    case BKDR_CHECKED & MASK_HASH:
        // checked
        m_oStyle.state.checked = value.ToBool();
        break;
    case BKDR_DEFAULT & MASK_HASH:
        // default    : 窗口初始默认控件
        if (m_pWindow && value.ToBool()) m_pWindow->SetDefault(*this);
        break;
    case BKDR_VISIBLE & MASK_HASH:
        // visible    : 是否可见
        m_state.visible = value.ToBool();
        break;
    //case BKDR_TABINDEX:
    //    // tabindex   : tab键索引
    //    break;
    case BKDR_CLASS & MASK_HASH:
        // class      : 样式表用类名
        while (value.begin() < value.end()) {
            const auto splited = value.Split(' ');
            if (splited.end() > splited.begin()) {
                const auto unitext = UIManager.GetUniqueText(splited);
                m_classesStyle.push_back(unitext);
            }
        }
        break;
    case BKDR_CONTEXT & MASK_HASH:
        // context    : 上下文菜单 
        m_pCtxCtrl = UIManager.GetUniqueText(value);
        break;
    case BKDR_TOOLTIP & MASK_HASH:
        // tooltip    : 提示窗口
        m_pTooltipCtrl = UIManager.GetUniqueText(value);
        break;
    case BKDR_TOOLTIPTEXT & MASK_HASH:
        // tooltiptext: 提示文本
        m_strTooltipText = value;
        break;
    case BKDR_ACCESSKEY & MASK_HASH:
        // accesskey  : 快捷访问键
        assert(value.end() > value.begin() && "bad accesskey");
        //if (value.end() > value.begin()) {
        if (true) {
            auto ch = *value.begin();
            if (ch >= 'a' && ch <= 'z') ch -= 'a' - 'A';
            m_chAccessKey = ch;
        }
        break;
    case BKDR_DRAGGABLE & MASK_HASH:
        // draggable  : 允许拖拽
        break;
    case BKDR_FLEX & MASK_HASH:
        // flex       : 布局弹性系数
        m_oStyle.flex = value.ToFloat();
        break;
    case BKDR_ORIENT & MASK_HASH:
        // orient     : 布局方向
        m_state.orient = *value.begin() == 'v';
        break;
    case BKDR_DIR & MASK_HASH:
        // dir        : 排列方向
        m_state.dir = *value.begin() == 'r';
        break;
    case BKDR_ALIGN & MASK_HASH:
        // align      : 布局方向垂直对齐方法
        m_oStyle.align = AttrParser::Align(value);
        break;
    case BKDR_PACK & MASK_HASH:
        // pack       : 布局方向平行对齐方法
        m_oStyle.pack = AttrParser::Pack(value);
        break;
#ifdef LUI_USER_INIPTR_DATA
    case BKDR_DATAUSER & MASK_HASH:
        // data-user
        this->user_data = value.ToInt32();
        break;
#endif
#ifdef LUI_USER_U16STR_DATA
    case BKDR_DATAU16 & MASK_HASH:
        // data-u16
        this->user_u16str = CUIString::FromUtf8(value);
        break;
#endif
#ifdef LUI_USER_U8STR_DATA
    case BKDR_DATAU8 & MASK_HASH:
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
    const auto rect = this->GetBox().GetBorderEdge();
    return LongUI::IsInclude(rect, pos);
}

/// <summary>
/// Maps to window.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::UIControl::MapToWindow(RectF& rect) const noexcept {
    assert(m_state.world_changed == false && "world changed!");
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
    assert(m_state.world_changed == false && "world changed!");
    pt = LongUI::TransformPoint(m_mtWorld, pt);
}


/// <summary>
/// Maps from window.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::UIControl::MapFromWindow(RectF& rect) const noexcept {
    assert(!"NOT IMPL");
}

/// <summary>
/// Maps from parent.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::UIControl::MapFromParent(RectF & rect) const noexcept {
    assert(!"NOT IMPL");
}

/// <summary>
/// Maps from window.
/// </summary>
/// <param name="point">The point.</param>
/// <returns></returns>
void LongUI::UIControl::MapFromWindow(Point2F& point) const noexcept {
    assert(m_state.world_changed == false && "world changed!");
    point = TransformPointInverse(m_mtWorld, point);
}

/// <summary>
/// Maps from parent.
/// </summary>
/// <param name="point">The point.</param>
/// <returns></returns>
void LongUI::UIControl::MapFromParent(Point2F & point) const noexcept {
    point; assert(!"NOT IMPL");
}

/// <summary>
/// Maps to parent.
/// </summary>
/// <param name="point">The point.</param>
/// <returns></returns>
void LongUI::UIControl::MapToParent(Point2F& point) const noexcept {
    point; assert(!"NOT IMPL");
}

/// <summary>
/// Initializes a new instance of the <see cref="UIControl"/> class.
/// </summary>
LongUI::UIControl::UIControl(UIControl* parent, const MetaControl& meta) noexcept : 
m_pParent(nullptr), m_refMetaInfo(meta) {
    m_bHasTimer = false;
    m_bHasInlineStyle = false;
    m_bTextChanged = false;
    Node::next = nullptr;
    Node::prev = nullptr;
    m_ptChildOffset = { 0, 0 };
    m_mtWorld = { 1, 0, 0, 1, 0, 0 };
    m_oBox.Init();
    m_state.Init();
    m_oBox.size = { INVALID_CONTROL_SIZE, INVALID_CONTROL_SIZE };
    // 初始化一般数据
    m_oHead = { nullptr, &m_oTail };
    m_oTail = { &m_oHead, nullptr };
    // 添加到父节点的子节点链表中
    if (parent) parent->add_child(*this);
    // 更新世界
    m_state.world_changed = true;
    // 延迟初始化
    UIManager.AddInitList(*this);
#ifndef NDEBUG
    this->name_dbg = meta.element_name;
#endif
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
    return Event_Ignore;
}


/// <summary>
/// Calculates the index of the child.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
auto LongUI::UIControl::calculate_child_index(const UIControl& ctrl) const noexcept->uint32_t {
    assert(ctrl.GetParent() == this);
    uint32_t index = 0;
    for (auto& x : *this) {
        if (x == ctrl) break;
        ++index;
    }
    return index;
}

/// <summary>
/// Calculates the child at.
/// </summary>
/// <param name="index">The index.</param>
/// <returns></returns>
auto LongUI::UIControl::calculate_child_at(uint32_t index) noexcept -> UIControl* {
    if (index >= this->GetCount()) return nullptr;
    auto child = this->begin();
    while (index) { ++child; --index; }
    return child;
}


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
            *this, m_pCtxCtrl, { e.px, e.py }, PopupType::Type_Context);
    }
    return Event_Ignore;
}

/// <summary>
/// Does the tooltip.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
auto LongUI::UIControl::do_tooltip(Point2F pos) noexcept -> EventAccept {
    if (m_state.tooltip_shown) return Event_Ignore;
    m_state.tooltip_shown = true;
    // 显示TOOLTIP
    if (const auto tooltip = m_pTooltipCtrl) {
        constexpr auto type = PopupType::Type_Tooltip;
        return LongUI::PopupWindowFromName(*this, tooltip, pos, type);
    }
    // 显示TOOLTIP TEXT
    if (!m_strTooltipText.empty()) {
        const auto text = m_strTooltipText.c_str();
        LongUI::PopupWindowFromTooltipText(*this, text, pos);
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
    // 检查原子性
    if (m_state.atomicity) return mouse_under_atomicity(e);
    using Pc = UIControlPrivate;
    EventAccept s = EventAccept::Event_Ignore;
    /*
        XXX: 将switch处理分成两部分

        switch [1]
        if (m_pHovered && m_pHovered->IsEnabled()) {
            if (m_pHovered->DoMouseEvent(e)) return Event_Accept;
        }
        switch [2]
    */

    // 自己处理消息
    switch (e.type)
    {
    case LongUI::MouseEvent::Event_MouseWheelV:
    case LongUI::MouseEvent::Event_MouseWheelH:
        // 检查指向控件是否处理
        return (m_pHovered && m_pHovered->IsEnabled() 
            && m_pHovered->DoMouseEvent(e)) ?
            Event_Accept : Event_Ignore;
    case LongUI::MouseEvent::Event_MouseEnter:
        // 检查是否存在动画
        this->StartAnimation({ StyleStateType::Type_Hover, true });
        // 截断ENTER ENTER消息
        return Event_Accept;
    case LongUI::MouseEvent::Event_MouseLeave:
        this->StartAnimation({ StyleStateType::Type_Hover, false });
        // 即便Disabled也可以收到LEAVE/ENTER消息
        if (m_pHovered) {
            UIControlPrivate::DoMouseLeave(*m_pHovered, { e.px, e.py });
            m_pHovered = nullptr;
            this->NeedUpdate();
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
            this->NeedUpdate();
        }
        if (child && child->IsEnabled()) return child->DoMouseEvent(e);
        // 处理MOUSE消息
        return Event_Accept;
    }
    case LongUI::MouseEvent::Event_MouseIdleHover:
        // 子控件优先处理事件
        if (m_pHovered && m_pHovered->IsEnabled()) {
            if (m_pHovered->DoMouseEvent(e)) return Event_Accept;
        }
        return this->do_tooltip({ e.px, e.py });
    case LongUI::MouseEvent::Event_LButtonDown:
        s = Event_Accept;
        this->StartAnimation({ StyleStateType::Type_Active, true });
        m_pWindow->SetDefault(*this);
        // 可以设为焦点-设为捕捉控件
        if (this->SetFocus()) m_pWindow->SetCapture(*this);
        m_pClicked = m_pHovered;
        break;
    case LongUI::MouseEvent::Event_LButtonUp:
        // 不可用则弃用
        if (m_pClicked && !m_pClicked->IsEnabled()) m_pClicked = nullptr;
        // 释放捕捉成功: 截断消息, 自下而上释放ACTIVE状态
        if (m_pWindow->ReleaseCapture(*this)) {
            const auto c = this->take_clicked();
            c->start_animation_b2u({ StyleStateType::Type_Active, false });
        }
        // 释放失败: 仅仅释放自己, 继续传递消息
        else {
            this->StartAnimation({ StyleStateType::Type_Active, false });
            // 存在点击的
            if (m_pClicked) m_pClicked->DoMouseEvent(e);
        }
        return Event_Accept;
    case LongUI::MouseEvent::Event_RButtonUp:
        // 子控件优先处理事件
        if (m_pHovered && m_pHovered->IsEnabled()) {
            if (m_pHovered->DoMouseEvent(e)) return Event_Accept;
        }
        return LongUI::PopupWindowFromName(
            *this, m_pCtxCtrl, { e.px, e.py },  PopupType::Type_Context);

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
    //if (m_bHasTimer) {
    //    this->KillTimer0123(0);
    //    this->KillTimer0123(1);
    //    this->KillTimer0123(2);
    //    this->KillTimer0123(3);
    //}
    // 移除被触发列表
    this->remove_triggered();
    // 移除高层引用
    UIManager.ControlDisattached(*this);
    // 移除窗口引用
    m_pWindow->ControlDisattached(*this);
    // 清理子节点
    while (begin() != end()) delete begin();
    // 清理渲染器
    this->delete_renderer();
    // 清除父节点中的自己
    if (m_pParent) {
        m_pParent->remove_child(*this);
#ifndef NDEBUG
        m_pParent = nullptr;
        m_pParent++;
#endif
    }
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
    while (!ctrl->IsTopLevel()) {
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
    m_state.child_i_changed = true;
    this->mark_window_minsize_changed();
    Node::SwapNode(a, b);
    this->NeedRelayout();
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
        //        << ctrl.GetBox().visible
        //        << endl;
        //}
#endif
        if (ctrl.IsVisible() && IsInclude(ctrl.GetBox().visible, pos)) {
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
    m_state.world_changed = true;
    m_oBox.pos = pos;
    this->NeedUpdate();
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
        // TODO: 顶级控件?
        //if (this->IsTopLevel()) {
        //}
        // XXX: 优化其他情况
        // 布局相关
        this->Invalidate();
        if (m_state.attachment == Attachment_Fixed) {

        }
        else {
            m_pParent->NeedRelayout();
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
        assert(m_oHead.next == &m_oTail);
        assert(m_oTail.prev == &m_oHead);
    }
#endif
    // 要求刷新
    m_state.child_i_changed = true;
    // 在析构中?
    if (m_state.destructing) return;
    this->mark_window_minsize_changed();
    this->NeedRelayout();
    //this->Invalidate();
    //this->NeedUpdate();
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
    const auto style_class = UIManager.GetUniqueText(pair);
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
    const auto style_class = UIManager.GetUniqueText(pair);
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
    // 标记自己和所有后代处于[enable]状态
    this->StartAnimation({ StyleStateType::Type_Disabled, disabled });
    // 禁止的话清除焦点状态
    if (disabled) this->KillFocus();
    // 原子控件除外, 因为对外是一个控件
    if (m_state.atomicity) return;
    // 递归调用
    for (auto& child : (*this)) child.SetDisabled(disabled);
}

/// <summary>
/// Determines whether [is visible to root].
/// </summary>
/// <returns></returns>
bool LongUI::UIControl::IsVisibleToRoot() const noexcept {
    auto ctrl = this;
    while (!ctrl->IsTopLevel()) {
        if (!ctrl->IsVisible()) return false;
        ctrl = ctrl->GetParent();
    }
    return true;
}

/// <summary>
/// Starts the animation.
/// </summary>
/// <param name="change">The change.</param>
/// <returns></returns>
void LongUI::UIControl::StartAnimation(StyleStateTypeChange change) noexcept {
#if 0
    if (this->IsTopLevel())
        LUIDebug(Hint) << "window-" << type << endl;
    if (change.type == StyleStateType::Type_Active) {
        LUIDebug(Hint) << this << change << endl;
    }
#endif
    // 未初始化
    if (!this->is_inited()) { m_oStyle.state.Change(change); return; }

    // 正式处理
    const auto app = this->GetStyle().appearance;
    // 非默认控件
    if (app == AttributeAppearance::Appearance_None) {
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
        m_state.parent_changed = true;
        m_pParent = nullptr;
    }
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
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UIControl::add_child(UIControl& child) noexcept {
    // 这条函数(add_child)非常重要
    assert(this && "bad this ptr");
    // 视口?
    if (LongUI::IsViewport(child)) {
        assert(m_pWindow && "add subwindow must be vaild window");
        LongUI::AddSubViewport(*m_pWindow, child);
        return;
    }
    // 无需再次添加
    if (child.m_pParent == this) return;
    child.m_state.parent_changed = true;
    // 在之前的父控件移除该控件
    if (child.m_pParent) child.m_pParent->remove_child(child);
    child.m_pParent = this;
    ++m_cChildrenCount;
    // 连接前后节点
    m_oTail.prev->next = &child;
    child.prev = m_oTail.prev;
    child.next = &m_oTail;
    m_oTail.prev = &child;
    // 要求刷新
    child.m_state.level = m_state.level + 1;
    assert(child.GetLevel() < 100ui8 && "tree too deep");
    // 移除之前的窗口引用
    child.m_pWindow->ControlDisattached(child);
    // 设置新的窗口
    child.m_pWindow = m_pWindow;
    // XXX: 添加新的窗口引用
    //child.m_pWindow->ControlAttached(child);
    // 同步
    if (child.GetCount()) UIControlPrivate::SyncInitData(child);
    m_state.child_i_changed = true;
    this->mark_window_minsize_changed();
    //this->NeedUpdate();
    this->NeedRelayout();
    // 提示管理器新的控件被添加到控件树中
    //UIManager.ControlAttached(child);
}

#ifndef LUI_DISABLE_STYLE_SUPPORT
/// <summary>
/// Resets the style sheet.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::link_style_sheet() noexcept {
#ifndef NDEBUG
    if (UIManager.flag & IUIConfigure::Flag_DbgNoLinkStyle)
        return;
#endif // !NDEBUG
    auto& style_matched = m_oStyle.matched;
    // 移除被触发列表
    this->remove_triggered();
    // 最高支持32(默认)枚内联样式
    LongUI::DEFAULT_CONTROL_MAX_SIZE;
    SSValue vbuf[SMALL_BUFFER_LENGTH]; uint32_t inline_size = 0;
    // 处理之前的内联样式
    if (m_bHasInlineStyle) {
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
        m_state.world_changed = true;
        this->NeedUpdate();
        break;
    case ValueType::Type_PositionTop:
        detail::write_value(m_oBox.pos.y, value.data4.single);
        m_state.world_changed = true;
        this->NeedUpdate();
        break;
    case ValueType::Type_DimensionWidth:
        detail::write_value(m_oStyle.minsize.width, value.data4.single);
        detail::write_value(m_oStyle.maxsize.width, value.data4.single);
        detail::write_value(m_oBox.size.width, value.data4.single);
        this->mark_window_minsize_changed();
        this->NeedRelayout();
        break;
    case ValueType::Type_DimensionHeight:
        detail::write_value(m_oStyle.minsize.height, value.data4.single);
        detail::write_value(m_oStyle.maxsize.height, value.data4.single);
        detail::write_value(m_oBox.size.height, value.data4.single);
        this->mark_window_minsize_changed();
        this->NeedRelayout();
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
        if (m_pParent) m_pParent->NeedRelayout();
        break;
    case ValueType::Type_DimensionMaxHeight:
        detail::write_value(m_oStyle.maxsize.height, value.data4.single);
        if (m_pParent) m_pParent->NeedRelayout();
        break;
    case ValueType::Type_BoxFlex:
        detail::write_value(m_oStyle.flex, value.data4.single);
        if (m_pParent) m_pParent->NeedRelayout();
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
        this->SetBdImageSource_NCRC(value.data4.u32);
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
        this->SetBgImage_NCRC({ value.data4.u32 });
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
        detail::write_value(value.data4.u32, this->GetBdImageSource_NCRC());
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
        detail::write_value(value.data4.u32, this->GetBgImage_NCRC());
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
        detail::write_value(value.data4.single, this->GetBox().margin.top);
        break;
    case ValueType::Type_MarginRight:
        detail::write_value(value.data4.single, this->GetBox().margin.right);
        break;
    case ValueType::Type_MarginBottom:
        detail::write_value(value.data4.single, this->GetBox().margin.bottom);
        break;
    case ValueType::Type_MarginLeft:
        detail::write_value(value.data4.single, this->GetBox().margin.left);
        break;
    case ValueType::Type_PaddingTop:
        detail::write_value(value.data4.single, this->GetBox().padding.top);
        break;
    case ValueType::Type_PaddingRight:
        detail::write_value(value.data4.single, this->GetBox().padding.right);
        break;
    case ValueType::Type_PaddingBottom:
        detail::write_value(value.data4.single, this->GetBox().padding.bottom);
        break;
    case ValueType::Type_PaddingLeft:
        detail::write_value(value.data4.single, this->GetBox().padding.left);
        break;
    case ValueType::Type_BorderTopWidth:
        detail::write_value(value.data4.single, this->GetBox().border.top);
        break;
    case ValueType::Type_BorderRightWidth:
        detail::write_value(value.data4.single, this->GetBox().border.right);
        break;
    case ValueType::Type_BorderBottomWidth:
        detail::write_value(value.data4.single, this->GetBox().border.bottom);
        break;
    case ValueType::Type_BorderLeftWidth:
        detail::write_value(value.data4.single, this->GetBox().border.left);
        break;
    }
}
#endif

/// <summary>
/// Sets the xul.
/// </summary>
/// <param name="xul">The xul string.</param>
/// <returns></returns>
void LongUI::UIControl::SetXul(const char* xul) noexcept {
    CUIControlControl::MakeXul(*this, xul);
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
            CUIString::FromUtf8(!m_id[0] ? m_refMetaInfo.element_name : m_id);
#else
            CUIString::FromUtf8(!m_id[0] ? this->name_dbg : m_id);
#endif
        return Event_Accept;
    case AccessibleEvent::Event_All_GetDescription:
        // 获取描述字符串
    {
        const auto name = CUIString::FromUtf8(this->GetID());
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
