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
LongUI::UIScrollArea::UIScrollArea(const MetaControl& meta) noexcept: Super(meta) {
    this->line_size = { EMPTY_HEIGHT_PER_ROW, EMPTY_HEIGHT_PER_ROW };
    m_minScrollSize = { };
    m_maxValue = { };
}


/// <summary>
/// Finalizes an instance of the <see cref="UIScrollArea"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIScrollArea::~UIScrollArea() noexcept {
#ifndef NDEBUG
    m_pSBVertical = nullptr;
    m_pSBVertical++;
    m_pSBHorizontal = nullptr;
    m_pSBHorizontal++;
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
    // XXX: 新的理由
    this->NeedUpdate(Reason_ChildLayoutChanged);
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
        case GuiEvent::Event_OnChange:
            if (sender == m_pSBHorizontal)
                m_ptChildOffset.x = m_pSBHorizontal->GetValue();
            else if (sender == m_pSBVertical)
                m_ptChildOffset.y = m_pSBVertical->GetValue();
            else break;
            // SB修改之后调用
            this->mark_world_changed();
            this->Invalidate();
            return Event_Accept;
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
    const auto maxv = index[&m_maxValue.width];
    if (maxv > 0) {
        // 位置变动检查
        auto& offset = index[&m_ptChildOffset.x];
        const auto line_height = UIManager.GetWheelScrollLines();
        float pos = offset - index[&this->line_size.width] * wheel * line_height;
        pos = detail::clamp(pos, 0.f, maxv);
        // 已经修改
        if (pos != offset) {
            offset = pos;
            this->mark_world_changed();
            this->Invalidate();
            // 同步SB
            this->sync_scroll_bar();
            return Event_Accept;
        }
    }
    return Event_Ignore;
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
/// Relayouts this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIScrollArea::Update(UpdateReason reason) noexcept {
    constexpr UpdateReason relayout_reason
        = Reason_ParentChanged
        | Reason_ChildIndexChanged
        | Reason_SizeChanged
        | Reason_BoxChanged
        | Reason_ChildLayoutChanged
        ;
    // TODO: 基本布局
    if (reason & relayout_reason)
        this->layout_scroll_bar();
    return Super::Update(reason);
}

/// <summary>
/// Sums the children flex.
/// </summary>
/// <returns></returns>
auto LongUI::UIScrollArea::sum_children_flex() const noexcept -> float {
    float sum = 0.f;
    for (auto& child : *this) if (child.IsVaildInLayout())
        sum += child.RefStyle().flex;
    return sum;
}

/// <summary>
/// Synchronizes the scroll bar.
/// </summary>
/// <returns></returns>
void LongUI::UIScrollArea::sync_scroll_bar() noexcept {
    const bool hok = m_pSBHorizontal && m_pSBHorizontal->IsVisible();
    const bool vok = m_pSBVertical && m_pSBVertical->IsVisible();
    // 交界区
    const Size2F corner = {
        vok ? m_pSBVertical->GetMinSize().width : 0.f,
        hok ? m_pSBHorizontal->GetMinSize().height : 0.f
    };
    const auto csize = m_oBox.GetContentSize();
    m_maxValue = { 0, 0 };
    // 水平滚动条
    if (hok) {
        m_pSBHorizontal->SetIncrement(this->line_size.width);
        m_pSBHorizontal->SetPageIncrement(csize.width - corner.width);
        m_maxValue.width = m_minScrollSize.width - csize.width + corner.width;
        m_pSBHorizontal->SetMax(m_maxValue.width);
        m_pSBHorizontal->SetValue(m_ptChildOffset.x);
        //m_pSBHorizontal->SetSingleStep(m_szSingleStep.width);
    }
    else m_ptChildOffset.x = 0.f;
    // 垂直滚动条
    if (vok) {
        m_pSBVertical->SetIncrement(this->line_size.height);
        m_pSBVertical->SetPageIncrement(csize.height - corner.height);
        m_maxValue.height = m_minScrollSize.height - csize.height + corner.height;
        m_pSBVertical->SetMax(m_maxValue.height);
        m_pSBVertical->SetValue(m_ptChildOffset.y);
        //m_pSBVertical->SetSingleStep(m_szSingleStep.height);
    }
    else m_ptChildOffset.y = 0.f;
    this->layout_corner(hok && vok, corner);
}

/// <summary>
/// layout corner
/// </summary>
/// <param name="visible"></param>
/// <returns></returns>
void LongUI::UIScrollArea::layout_corner(bool visible, Size2F size) noexcept{
    if (visible) {
        // 这个是大前提
        assert(m_pSBHorizontal && m_pSBVertical);
        // 没有就尝试创建并且初始化
        if (!m_pCorner) {
            m_pCorner = new(std::nothrow) UIControl(this);
            if (!m_pCorner) return;
#ifndef NDEBUG
            m_pCorner->name_dbg = "scrollarea::corner";
#endif
            this->set_child_fixed_attachment(*m_pCorner);
            UIControlPrivate::SetAppearance(*m_pCorner, Appearance_WeakApp | Appearance_Resizer);
        }
        // 有就进行设置
        if (m_pCorner) {
            m_pCorner->SetVisible(true);
            m_pCorner->Resize(size);
            const auto x = m_pSBVertical->GetPos().x;
            const auto y = m_pSBHorizontal->GetPos().y;
            m_pCorner->SetPos({ x, y });
        }
    }
    // 有就有, 没有就算了
    else if(m_pCorner) m_pCorner->SetVisible(false);
}

/// <summary>
/// Layouts the hscrollbar.
/// </summary>
/// <param name="notenough">if set to <c>true</c> [notenough].</param>
/// <returns></returns>
auto LongUI::UIScrollArea::layout_hscrollbar(bool notenough) noexcept -> float {
    const auto overflow = this->RefStyle().overflow_x;
    // 检查绝不可能的情况
    const bool nosb = overflow & 1;
    if (nosb) return 0.0f;
    // 可能存在滚动条
    if (overflow == Overflow_Auto) {
        // 不够用就显示
        if (!notenough) {
            if (m_pSBHorizontal) m_pSBHorizontal->SetVisible(false);
            return 0.f;
        }
    }
    // 需要滚动条
    if (!m_pSBHorizontal) m_pSBHorizontal = this->create_scrollbar(Orient_Horizontal);
    return m_pSBHorizontal ? m_pSBHorizontal->GetMinSize().height : 0.0f;
}

/// <summary>
/// Layouts the vscrollbar.
/// </summary>
/// <param name="notenough">if set to <c>true</c> [notenough].</param>
/// <returns></returns>
auto LongUI::UIScrollArea::layout_vscrollbar(bool notenough) noexcept -> float {
    const auto overflow = this->RefStyle().overflow_y;
    // 检查绝不可能的情况
    const bool nosb = overflow & 1;
    if (nosb) return 0.0f;
    // 可能存在滚动条
    if (overflow == Overflow_Auto) {
        // 不够用就显示
        if (!notenough) {
            if (m_pSBVertical) m_pSBVertical->SetVisible(false);
            return 0.f; 
        }
    }
    // 需要滚动条
    if (!m_pSBVertical) m_pSBVertical = this->create_scrollbar(Orient_Vertical);
    return m_pSBVertical ? m_pSBVertical->GetMinSize().width : 0.0f;
}

/// <summary>
/// Creates the scrollbar.
/// </summary>
/// <returns></returns>
auto LongUI::UIScrollArea::create_scrollbar(AttributeOrient o) noexcept -> UIScrollBar * {
    // 创建滚动条
    if (auto bar = new(std::nothrow) UIScrollBar{ this, o }) {
#ifndef NDEBUG
        if (o == Orient_Horizontal)
            bar->name_dbg = "scrollarea::hscrollbar";
        else
            bar->name_dbg = "scrollarea::vscrollbar";
#endif
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
    // TODO: 修改
    const auto is_need_relayout = [this]() noexcept {
        constexpr UpdateReason relayout_reason
            = Reason_ParentChanged
            | Reason_ChildIndexChanged
            | Reason_SizeChanged
            | Reason_BoxChanged
            | Reason_ChildLayoutChanged
            ;
        return m_state.reason & relayout_reason;
    };
    Size2F rv;
    constexpr float MDW = MIN_SCROLLBAR_DISPLAY_SIZE;
    constexpr float MDH = MIN_SCROLLBAR_DISPLAY_SIZE;
    // 内容大小
    const auto content_size = this->RefBox().GetContentSize();
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
    if (is_need_relayout()) return content_size;
    // 设置VSB位置: 正向-右侧 反向-左侧
    if (vsbar > 0.f) {
        resize_child(*m_pSBVertical, { vsbar, content_size.height - hsbar });
        Point2F pos = {};
        const bool normaldir = m_state.direction == Dir_Normal;
        if (normaldir) pos.x = content_size.width - vsbar;
        m_pSBVertical->SetPos(pos);
        m_pSBVertical->SetVisible(true);
    }
    // 设置HSB位置: 正向-下侧 反向-上侧
    if (hsbar > 0.f) {
        resize_child(*m_pSBHorizontal, { content_size.width - vsbar, hsbar });
        Point2F pos = {};
        const bool normaldir = m_state.direction == Dir_Normal;
        if (normaldir) pos.y = content_size.height - hsbar;
        m_pSBHorizontal->SetPos(pos);
        m_pSBHorizontal->SetVisible(true);
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
    const auto base = this->RefBox().GetContentPos();
    //const auto base = Point2F{};
    // 正向- 左上角
    if (m_state.direction == Dir_Normal) {

    }
    // 反向- 左上角+[VSB, 0]
    else {
        if (m_pSBVertical && m_pSBVertical->IsVisible()) {
            return base + Point2F{ m_pSBVertical->GetSize().width, 0.f };
        }
    }
    return base;
}