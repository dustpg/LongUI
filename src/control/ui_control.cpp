#include <core/ui_window.h>
#include <core/ui_manager.h>
#include <control/ui_control.h>
#include <container/pod_hash.h>
#include <control/ui_ctrlmeta.h>
#include <event/ui_event_host.h>
#include <style/ui_native_style.h>
#include <event/ui_initialize_event.h>
#include "../private/ui_private_control.h"


#include <luiconf.h>
#include <algorithm>

// ui namespace
namespace LongUI {
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
    const auto layout = this->IsTopLevel() ? this : m_pParent;
    layout->NeedRelayout();
    // XXX: 需要修改世界矩阵
    m_state.world_changed = true;
    // 修改了数据
    m_oBox.size = size;
    return true;
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
    //UIManager.MarkWindowMinsizeChanged(m_pWindow);
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
        hr = this->Recreate();
        // 初始化大小
        if (m_oBox.size.width == static_cast<float>(INVALID_CONTROL_SIZE)) {
            this->Resize({ DEFAULT_CONTROL_WIDTH, DEFAULT_CONTROL_HEIGHT });
        }
        // 重新连接样式表
        this->link_style_sheet();
    }
    return hr;
}

/// <summary>
/// Updates the self.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::Update() noexcept {
    // 本次已经刷新了?
    //assert(!m_state.updated && "been updated this time");
    assert(m_state.inited && "must init control first");
    // 状态修改
    m_state.style_state_changed = false;
    // 最基的不处理子控件索引更改
    m_state.child_i_changed = false;
    m_state.parent_changed = false;


    //UIManager.MarkWindowMinsizeChanged(m_pWindow);

    /*if (!std::strcmp(name_dbg, "hbox1-a")) {
        static int counter = 0;
        if (counter) {
            int bk = 9;
        }
        ++counter;
    }*/

    /*LUIDebug(Hint) LUI_FRAMEID
        << this 
        << LongUI::endl;*/
}


/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::UIControl::Recreate() noexcept -> Result {
    return{ Result::RS_OK };
}

/// <summary>
/// Gets the subelement.
/// </summary>
/// <param name="name">The name.</param>
/// <returns></returns>
auto LongUI::UIControl::get_subelement(U8View name) noexcept -> UIControl * {
    // ::before ::after?
    return nullptr;
}

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
    constexpr auto BKDR_DIR         = 0x001a65b1_ui32;
    constexpr auto BKDR_PACK        = 0x0f1b8d4d_ui32;
    constexpr auto BKDR_FLEX        = 0x0dc767b5_ui32;
    constexpr auto BKDR_LEFT        = 0x0e936497_ui32;
    constexpr auto BKDR_RIGHT       = 0xdf4832f0_ui32;
    constexpr auto BKDR_STYLE       = 0xf253f789_ui32;
    constexpr auto BKDR_ALIGN       = 0xb54685e9_ui32;
    constexpr auto BKDR_CLASS       = 0xd85fe06c_ui32;
    constexpr auto BKDR_WIDTH       = 0x370bff82_ui32;
    constexpr auto BKDR_HEIGHT      = 0x28d4978b_ui32;
    constexpr auto BKDR_ORIENT      = 0xeda466cd_ui32;
    constexpr auto BKDR_VISIBLE     = 0x646b6442_ui32;
    constexpr auto BKDR_TABINDEX    = 0x1c6477b9_ui32;
    constexpr auto BKDR_DISABLED    = 0x715f1adc_ui32;
    constexpr auto BKDR_ACCESSKEY   = 0xba56ab7b_ui32;
    constexpr auto BKDR_DRAGGABLE   = 0xbd13c3b5_ui32;
    constexpr auto BKDR_CONTEXTMENU = 0xb133f7f6_ui32;
    
    constexpr auto BKDR_DATAUSER    = 0x5c110136_ui32;
    constexpr auto BKDR_DATAU16     = 0xc036b6f7_ui32;
    constexpr auto BKDR_DATAU8      = 0xe4278072_ui32;
   
    // HASH 一致就认为一致即可
    switch (key)
    {
    case BKDR_ID:
        // id         : 窗口唯一id
        m_id = UIManager.GetUniqueText(value);
        // 尝试添加命名控件
        if (m_pWindow) m_pWindow->AddNamedControl(*this);
        break;
    case BKDR_LEFT:
        // left
        m_oBox.pos.x = value.ToFloat();
        break;
    case BKDR_RIGHT:
        // right
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
    case BKDR_STYLE:
        // style      : 内联样式表
        break;
    case BKDR_DISABLED:
        // disabled   : 禁用状态
        m_oStyle.state.disabled = static_cast<bool>(value);
        break;
    case BKDR_VISIBLE:
        // visible    : 是否可见
        m_state.visible = static_cast<bool>(value);
        break;
    case BKDR_TABINDEX:
        // tabindex   : tab键索引
        break;
    case BKDR_CLASS:
        // class      : 样式表用类名
        while (value.begin() < value.end()) {
            const auto splited = value.Split(' ');
            if (splited.end() > splited.begin()) {
                const auto unitext = UIManager.GetUniqueText(splited);
                m_classesStyle.push_back(unitext);
            }
        }
        break;
    case BKDR_CONTEXTMENU:
        // contextmenu: 上下文菜单
        break;
    case BKDR_ACCESSKEY:
        // accesskey  : 快捷访问键
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
        m_state.dir = *value.begin() == 'r';
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
        return LongUI::Event_Accept;
    }
    return LongUI::Event_Ignore;
}


/// <summary>
/// Does the input event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIControl::DoInputEvent(InputEventArg e) noexcept -> EventAccept {
    return LongUI::Event_Ignore;
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
        // 截断ENTER LEAVE消息
        return Event_Accept;
    case LongUI::MouseEvent::Event_LButtonDown:
        this->StartAnimation({ StyleStateType::Type_Active, true });
        this->start_animation_children({ StyleStateType::Type_Active, true });
        m_pWindow->SetDefualt(*this);
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
    }
    return Event_Ignore;
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
    case LongUI::MouseEvent::Event_LButtonDown:
        s = Event_Accept;
        this->StartAnimation({ StyleStateType::Type_Active, true });
        m_pWindow->SetDefualt(*this);
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
auto LongUI::UIControl::FindChild(const Point2F & pos) noexcept -> UIControl* {
    UIControl* found = nullptr;
    // TODO: 优化 Attachment_Fixed
    for (auto& ctrl : *this) {
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
        // 布局相关
        if (m_state.parent_notneed_relayout) {
            m_pParent->Invalidate();
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
    const auto app = this->GetStyle().appearance;
    // 非默认控件
    if (app == AttributeAppearance::Appearance_None) {
        UIManager.StartExtraAnimation(*this, change);
    }
    // 默认控件
    else {
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
    // 同步
    if (child.GetCount()) UIControlPrivate::SyncInitData(child);
    m_state.child_i_changed = true;
    this->mark_window_minsize_changed();
    //this->NeedUpdate();
    this->NeedRelayout();
    // 提示管理器新的控件被添加到控件树中
    //UIManager.ControlAttached(child);
}

/// <summary>
/// Resets the style sheet.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::link_style_sheet() noexcept {
#ifndef NDEBUG
    if (UIManager.flag & IUIConfigure::Flag_DbgNoLinkStyle)
        return;
#endif // !NDEBUG
    m_oStyle.matched.clear();

    // 先连接全局的
    LongUI::MatchStyleSheet(*this, UIManager.GetStyleSheet());
    // 再连接窗口的
    if (const auto window = m_pWindow) {
        const auto ss = window->GetStyleSheet();
        LongUI::MatchStyleSheet(*this, ss);
    }
    // 连接内联样式

    // 第一次设置
    this->setup_style_values();
}


/// <summary>
/// Setups the style values.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::setup_style_values() noexcept {
    // 基础继承
    const auto& list = m_oStyle.matched;
    for (auto itr = list.begin(); itr != list.end();) {
        // 必须是START
        assert((*itr).type == ValueType::Type_NewOne);
        // 检测长度 
        const auto len = itr[0].data.u32; assert(len > 2 && "bad size");
        // 检测伪类
        const auto pc = reinterpret_cast<const SSValuePC&>(itr[1]);
        // 匹配状态
        static_assert(sizeof(StyleState) == sizeof(uint32_t), "must be same");
        const auto now = reinterpret_cast<const uint32_t&>(m_oStyle.state);
        const auto yes = reinterpret_cast<const uint32_t&>(pc.yes);
        const auto noo = reinterpret_cast<const uint32_t&>(pc.noo);
        /*
            YES:
                0001  &  0010  -> 0000   X
                0011  &  0010  -> 0010   O
                0001  &  0011  -> 0001   X
            NOO:
                0001  &  0010  -> 0000   O
                0011  &  0010  -> 0010   X
                0001  &  0011  -> 0001   X

        */
        if ((now & yes) == yes && (now & noo) == 0) {
            // 遍历状态
            const auto end_itr = itr + len;
            auto being_itr = itr + 2;
            for (; being_itr != end_itr; ++being_itr) {
                this->ApplyValue(*being_itr);
            }
        }
        // 递进
        itr += len;
    }
    // 内联样式
}

// longui::detail namespace
namespace LongUI { namespace detail {
    // attribute write
    template<typename T, typename U>
    static inline void write_value(T& a, U b) noexcept {
        static_assert(sizeof(T) == sizeof(U));
        a = static_cast<T>(b);
    }
}}

PCN_NOINLINE
/// <summary>
/// Applies the value.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIControl::ApplyValue(SSValue value) noexcept {
    switch (value.type)
    {
    default:
        assert(!"unsupported value type");
        break;
    case ValueType::Type_PositionOverflowX:
        detail::write_value(m_oStyle.overflow_x, value.data.byte);
        break;
    case ValueType::Type_PositionOverflowY:
        detail::write_value(m_oStyle.overflow_y, value.data.byte);
        break;
    case ValueType::Type_BackgroundColor:
        this->SetBgColor({ value.data.u32 });
        break;
    case ValueType::Type_BackgroundImage:
        this->SetBgImage({ value.data.u32 });
        break;
    case ValueType::Type_BackgroundRepeat:
        this->SetBgRepeat(static_cast<AttributeRepeat>(value.data.byte));
        break;
    case ValueType::Type_TransitionDuration:
        using dur_t = decltype(m_oStyle.tduration);
        assert(value.data.single < 65.5f && "out of range");
        m_oStyle.tduration = static_cast<dur_t>(value.data.single * 1000.f);
        break;
    case ValueType::Type_TextColor:
        this->SetFgColor({ value.data.u32 });
        break;
    case ValueType::Type_UIAppearance:
        detail::write_value(m_oStyle.appearance, value.data.byte);
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
    default:
        assert(!"unsupported value type");
        break;
    case ValueType::Type_PositionOverflowX:
        detail::write_value(value.data.byte, m_oStyle.overflow_x);
        break;
    case ValueType::Type_PositionOverflowY:
        detail::write_value(value.data.byte, m_oStyle.overflow_y);
        break;
    case ValueType::Type_BackgroundColor:
        detail::write_value(value.data.u32, this->GetBgColor().primitive);
        break;
    case ValueType::Type_BackgroundImage:
        detail::write_value(value.data.u32, this->GetBgImage());
        break;
    case ValueType::Type_BackgroundRepeat:
        detail::write_value(value.data.byte, this->GetBgRepeat());
        break;
    case ValueType::Type_TransitionDuration:
        value.data.single = static_cast<float>(m_oStyle.tduration) * 1000.f;
        break;
    case ValueType::Type_TextColor:
        detail::write_value(value.data.u32, this->GetFgColor().primitive);
        break;
    case ValueType::Type_UIAppearance:
        detail::write_value(value.data.byte, m_oStyle.appearance);
        break;
    }
}

/// <summary>
/// Sets the xul.
/// </summary>
/// <param name="xul">The xul string.</param>
/// <returns></returns>
void LongUI::UIControl::SetXUL(const char* xul) noexcept {
    CUIControlControl::MakeXUL(*this, xul);
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
            CUIString::FromUtf8(!m_id ? m_refMetaInfo.element_name : m_id);
#else
            CUIString::FromUtf8(!m_id ? this->name_dbg : m_id);
#endif
        return Event_Accept;
    case AccessibleEvent::Event_All_GetDescription:
        // 获取描述字符串
    {
        const auto name = CUIString::FromUtf8(this->GetID());
        static_cast<const getd_t&>(args).description->format(
            L"element<%hs> id<%ls>",
            m_refMetaInfo.element_name,
            name.c_str()
        );
    }
        return Event_Accept;
    }
    return Event_Ignore;
}

#endif
