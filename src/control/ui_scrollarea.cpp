// Gui
#include <control/ui_scrollarea.h>
#include <control/ui_scrollbar.h>
#include <control/ui_spacer.h>
#include <core/ui_ctrlmeta.h>
#include <core/ui_manager.h>
#include <util/ui_little_math.h>
// C++
#include <algorithm>
// Private
#include "../private/ui_private_control.h"


// ui namespace
namespace LongUI {
    // UIScrollArea类 元信息
    LUI_CONTROL_META_INFO(UIScrollArea, "scrollarea");
}

/// <summary>
/// Initializes a new instance of the <see cref="UIScrollArea" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
/// <returns></returns>
LongUI::UIScrollArea::UIScrollArea(UIControl* parent, const MetaControl& meta) noexcept
    : Super(impl::ctor_lock(parent), meta) {
    this->line_size = { EMPTY_HEIGHT_PER_ROW, EMPTY_HEIGHT_PER_ROW };
    m_minScrollSize = { };
    impl::ctor_unlock();
}


/// <summary>
/// Finalizes an instance of the <see cref="UIScrollArea"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIScrollArea::~UIScrollArea() noexcept {
#ifndef NDEBUG
    m_pVerticalSB = nullptr;
    m_pVerticalSB++;
    m_pHorizontalSB = nullptr;
    m_pHorizontalSB++;
#endif
}


/// <summary>
/// Sets the automatic overflow.
/// </summary>
/// <returns></returns>
void LongUI::UIScrollArea::SetAutoOverflow() noexcept {
    m_oStyle.overflow_x = Overflow_Auto;
    m_oStyle.overflow_y = Overflow_Auto;
}


/// <summary>
/// Forces the size of the update scroll.
/// </summary>
/// <param name="ss">The ss.</param>
/// <returns></returns>
void LongUI::UIScrollArea::ForceUpdateScrollSize(Size2F ss) noexcept {
    m_minScrollSize = ss;
    this->NeedRelayout();
}

/// <summary>
/// Adds the spacer.
/// </summary>
/// <returns></returns>
void LongUI::UIScrollArea::AddSpacer(Size2F size, float flex) noexcept {
    // 生成对象
    if (const auto spacer = new(std::nothrow) UISpacer{ this }) {
        // 最小尺寸
        spacer->SetStyleMinSize(size);
        // 有效弹簧
        if (flex > 0.f) UIControlPrivate::SetFlex(*spacer, flex);
        // 无效弹簧
        else spacer->SetStyleMaxSize(size);
    }
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="arg">The argument.</param>
/// <returns></returns>
auto LongUI::UIScrollArea::DoEvent(
    UIControl* sender, const EventArg& arg) noexcept -> EventAccept {
    // 消息处理
    if (arg.nevent == NoticeEvent::Event_UIEvent) {
        assert(sender && "sender in gui event cannot be null");
        switch (static_cast<const EventGuiArg&>(arg).GetEvent())
        {
        return_changed:
            // SB修改之后调用
            m_state.world_changed = true;
            this->NeedUpdate();
            this->Invalidate();
            return Event_Accept;
        case GuiEvent::Event_OnChange:
            if (sender == m_pHorizontalSB) {
                m_ptChildOffset.x = m_pHorizontalSB->GetValue();
                goto return_changed;
            }
            else if (sender == m_pVerticalSB) {
                m_ptChildOffset.y = m_pVerticalSB->GetValue();
                //LUIDebug(Hint) << m_ptChildOffset.y << endl;
                goto return_changed;
            }
        }
    }
    // 基类处理消息
    return Super::DoEvent(sender, arg);
}

/// <summary>
/// Does the wheel.
/// </summary>
/// <param name="index">The index.</param>
/// <param name="wheel">The wheel.</param>
/// <returns></returns>
auto LongUI::UIScrollArea::do_wheel(int index, float wheel) noexcept ->EventAccept {
    // 检查内容区域是否满足
    const auto content_size = this->GetBox().GetContentSize();
    const auto cremainw = index[&m_minScrollSize.width] - index[&content_size.width];
    if (cremainw <= 0.f) return Event_Ignore;
    // 位置变动检查
    auto& pos_o = index[&m_ptChildOffset.x];
    const auto line_height = UIManager.GetWheelScrollLines();
    float pos = pos_o - index[&this->line_size.width] * wheel * line_height;
    pos = detail::clamp(pos, 0.f, cremainw);

    EventAccept accept = Event_Ignore;
    // 已经修改
    if (pos != pos_o) {
        pos_o = pos;
        accept = Event_Accept;
        m_state.world_changed = true;
        this->NeedUpdate();
        this->Invalidate();
    }
    // 同步SB
    this->sync_scroll_bar();
    return accept;
}

/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIScrollArea::DoMouseEvent(const MouseEventArg & e) noexcept -> EventAccept {
    // 分类判断
    switch (e.type)
    {
    case LongUI::MouseEvent::Event_MouseWheelV:
        // 检查是否有子控件处理(super有, 也可以现写)
        return Super::DoMouseEvent(e) == Event_Ignore ?
            this->do_wheel(1, e.wheel) : Event_Accept;
    case LongUI::MouseEvent::Event_MouseWheelH:
        // 检查是否有子控件处理(super有, 也可以现写)
        return Super::DoMouseEvent(e) == Event_Ignore ?
            this->do_wheel(0, e.wheel) : Event_Accept;
    }
    // 其他未处理事件交给super处理
    return Super::DoMouseEvent(e);
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIScrollArea::Update() noexcept {
#if 0
    // 理论上应该这样写

    // 污了?
    if (this->is_need_relayout()) {
        this->on_state_dirty();
        if (this->is_need_relayout()) return;
        m_state.dirty = true;
        Super::Update();
        m_state.dirty = false;
    }
    // 链式调用
    else return Super::Update();
#else
    // 这样好像没有BUG
    Super::Update();
    if (this->is_need_relayout()) this->on_state_dirty();
#endif
}

/// <summary>
/// Ons the state dirty.
/// </summary>
/// <returns></returns>
void LongUI::UIScrollArea::on_state_dirty() noexcept {
    //if (std::strcmp("listbox::listboxbody", this->name_dbg) == 0) {
    //    int bk = 9;
    //}
    // 有面积才算数
    const auto s = this->GetSize();
    if (s.width * s.height <= 0.f) return;

    // 处理大小修改
    this->size_change_handled();
    // 存在子控件才计算
    if (this->GetCount()) {
        // 更新布局
        this->relayout();
        // 更新子控件
        for (auto& child : *this)
            child.NeedUpdate();
    }
    // 这里, 世界不再改变
    //assert(m_state.world_changed == false);
}

/// <summary>
/// Relayouts this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIScrollArea::relayout() noexcept {
    // TODO: 基本布局

    this->layout_scroll_bar();
}

/// <summary>
/// Sums the children flex.
/// </summary>
/// <returns></returns>
auto LongUI::UIScrollArea::sum_children_flex() const noexcept -> float {
    float sum = 0.f;
    for (auto& child : *this) if (child.IsVaildInLayout())
        sum += child.GetStyle().flex;
    return sum;
}

/// <summary>
/// Synchronizes the scroll bar.
/// </summary>
/// <returns></returns>
void LongUI::UIScrollArea::sync_scroll_bar() noexcept {
    const bool hok = m_pHorizontalSB && m_pHorizontalSB->IsVisible();
    const bool vok = m_pVerticalSB && m_pVerticalSB->IsVisible();
    // 交界区
    Size2F cross_zone;
    cross_zone.width = vok ? m_pVerticalSB->GetMinSize().width : 0.f;
    cross_zone.height = hok ? m_pHorizontalSB->GetMinSize().height : 0.f;
    auto csize = m_oBox.GetContentSize();
    // 水平滚动条
    if (hok) {
        m_pHorizontalSB->SetIncrement(this->line_size.width);
        m_pHorizontalSB->SetPageIncrement(csize.width - cross_zone.width);
        m_pHorizontalSB->SetMax(m_minScrollSize.width - csize.width);
        m_pHorizontalSB->SetValue(m_ptChildOffset.x);
        //m_pHorizontalSB->SetSingleStep(m_szSingleStep.width);
    }
    else m_ptChildOffset.x = 0.f;
    // 垂直滚动条
    if (vok) {
        m_pVerticalSB->SetIncrement(this->line_size.height);
        m_pVerticalSB->SetPageIncrement(csize.height - cross_zone.height);
        m_pVerticalSB->SetMax(m_minScrollSize.height - csize.height);
        m_pVerticalSB->SetValue(m_ptChildOffset.y);
        //m_pVerticalSB->SetSingleStep(m_szSingleStep.height);
    }
    else m_ptChildOffset.y = 0.f;
    
}

/// <summary>
/// Layouts the hscrollbar.
/// </summary>
/// <param name="notenough">if set to <c>true</c> [notenough].</param>
/// <returns></returns>
auto LongUI::UIScrollArea::layout_hscrollbar(bool notenough) noexcept -> float {
    const auto overflow = this->GetStyle().overflow_x;
    // 检查绝不可能的情况
    const bool nosb = overflow & 1;
    if (nosb) return 0.0f;
    // 可能存在滚动条
    if (overflow == Overflow_Auto) {
        // 不够用就显示
        if (!notenough) {
            if (m_pHorizontalSB) m_pHorizontalSB->SetVisible(false);
            return 0.f;
        }
    }
    // 需要滚动条
    if (!m_pHorizontalSB) m_pHorizontalSB = this->create_scrollbar(Orient_Horizontal);
    return m_pHorizontalSB ? m_pHorizontalSB->GetMinSize().height : 0.0f;
}

/// <summary>
/// Layouts the vscrollbar.
/// </summary>
/// <param name="notenough">if set to <c>true</c> [notenough].</param>
/// <returns></returns>
auto LongUI::UIScrollArea::layout_vscrollbar(bool notenough) noexcept -> float {
    const auto overflow = this->GetStyle().overflow_y;
    // 检查绝不可能的情况
    const bool nosb = overflow & 1;
    if (nosb) return 0.0f;
    // 可能存在滚动条
    if (overflow == Overflow_Auto) {
        // 不够用就显示
        if (!notenough) {
            if (m_pVerticalSB) m_pVerticalSB->SetVisible(false);
            return 0.f; 
        }
    }
    // 需要滚动条
    if (!m_pVerticalSB) m_pVerticalSB = this->create_scrollbar(Orient_Vertical);
    return m_pVerticalSB ? m_pVerticalSB->GetMinSize().width : 0.0f;
}

/// <summary>
/// Creates the scrollbar.
/// </summary>
/// <returns></returns>
auto LongUI::UIScrollArea::create_scrollbar(AttributeOrient o) noexcept -> UIScrollBar * {
    // 创建滚动条
    if (auto bar = new(std::nothrow) UIScrollBar{ o, nullptr }) {
#ifndef NDEBUG
        if (o == Orient_Horizontal)
            bar->name_dbg = "scrollarea::hscrollbar";
        else
            bar->name_dbg = "scrollarea::vscrollbar";
#endif
        this->add_child(*bar);
        this->resize_child(*bar, {});
        this->set_child_fixed_attachment(*bar);
        UIControlPrivate::SetGuiEvent2Parent(*bar);
        //bar->Init();
        return bar;
    }
    return nullptr;
}

/// <summary>
/// Layouts the size of the content.
/// </summary>
/// <returns></returns>
auto LongUI::UIScrollArea::layout_scroll_bar() noexcept -> Size2F {
    Size2F rv;
    constexpr float MDW = MIN_SCROLLBAR_DISPLAY_SIZE;
    constexpr float MDH = MIN_SCROLLBAR_DISPLAY_SIZE;
    // 内容大小
    const auto content_size = this->GetBox().GetContentSize();
    // 不存在的
    if (1 & m_oStyle.overflow_x & m_oStyle.overflow_y) 
        return content_size;
    // 内容显示
    const auto scroll = m_minScrollSize;
    // 需要显示VSB
    const bool vsbdisplay = content_size.width > MDW &&
        scroll.height > content_size.height;
    // 需要显示HSB
    const bool hsbdisplay = content_size.height > MDW &&
        scroll.width > content_size.width;
    // 获取VSB长度
    const auto vsbar = this->layout_vscrollbar(vsbdisplay);
    // 获取HSB长度
    const auto hsbar = this->layout_hscrollbar(hsbdisplay);
    // 需要再次布局
    if (this->is_need_relayout()) return content_size;
    // 设置VSB位置: 正向-右侧 反向-左侧
    if (vsbar > 0.f) {
        resize_child(*m_pVerticalSB, { vsbar, content_size.height - hsbar });
        Point2F pos = {};
        const bool normaldir = m_state.dir == Dir_Normal;
        if (normaldir) pos.x = content_size.width - vsbar;
        m_pVerticalSB->SetPos(pos);
        m_pVerticalSB->SetVisible(true);
    }
    // 设置HSB位置: 正向-下侧 反向-上侧
    if (hsbar > 0.f) {
        resize_child(*m_pHorizontalSB, { content_size.width - vsbar, hsbar });
        Point2F pos = {};
        const bool normaldir = m_state.dir == Dir_Normal;
        if (normaldir) pos.y = content_size.height - hsbar;
        m_pHorizontalSB->SetPos(pos);
        m_pHorizontalSB->SetVisible(true);
    }
    // 同步SB显示
    this->sync_scroll_bar();
    // 返回剩余大小
    rv.width = content_size.width - vsbar;
    rv.height = content_size.height - hsbar;
    return rv;
}

/// <summary>
/// Gets the layout position.
/// </summary>
/// <returns></returns>
auto LongUI::UIScrollArea::get_layout_position() const noexcept -> Point2F {
    const auto base = this->GetBox().GetContentPos();
    //const auto base = Point2F{};
    // 正向- 左上角
    if (m_state.dir == Dir_Normal) {

    }
    // 反向- 左上角+[VSB, 0]
    else {
        if (m_pVerticalSB && m_pVerticalSB->IsVisible()) {
            return base + Point2F{ m_pVerticalSB->GetSize().width, 0.f };
        }
    }
    return base;
}