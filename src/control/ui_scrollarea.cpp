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
LongUI::UIScrollArea::UIScrollArea(const MetaControl& meta) noexcept : Super(meta) {
    this->line_size = { EMPTY_HEIGHT_PER_ROW, EMPTY_HEIGHT_PER_ROW };
    m_minScrollSize = { };
    m_maxScrollSize = { };
    m_pFinalEnd = this->end();
}


/// <summary>
/// Finalizes an instance of the <see cref="UIScrollArea"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIScrollArea::~UIScrollArea() noexcept {
#ifndef NDEBUG
    m_pScrollBarVer = nullptr;
    m_pScrollBarVer++;
    m_pScrollBarHor = nullptr;
    m_pScrollBarHor++;
#endif
}


/// <summary>
/// Sets the automatic overflow.
/// </summary>
/// <returns></returns>
void LongUI::UIScrollArea::SetAutoOverflow() noexcept {
    static_assert(Overflow_Auto == 0, "must be 0");
    constexpr uint8_t mask = ~3;
    reinterpret_cast<uint8_t&>(m_oStyle.overflow_xex) &= mask;
    m_oStyle.overflow_y = Overflow_Auto;
}


/// <summary>
/// Adds the spacer.
/// </summary>
/// <returns></returns>
void LongUI::UIScrollArea::AddSpacer(Size2F size, float flex) noexcept {
    // 生成对象
    if (const auto spacer = new(std::nothrow) UISpacer{ this }) {
        // 设置对应数据
        spacer->SetSpacer(size, flex);
    }
}



/// <summary>
/// add child for UIScrollArea
/// </summary>
/// <param name="child"></param>
/// <returns></returns>
void LongUI::UIScrollArea::add_child(UIControl& child) noexcept {
    Super::insert_child(child, *m_pFinalEnd);
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
        switch (static_cast<const GuiEventArg&>(arg).GetType())
        {
        case GuiEvent::Event_OnChange:
            if (sender == m_pScrollBarHor)
                m_ptChildOffset.x = m_pScrollBarHor->GetValue();
            else if (sender == m_pScrollBarVer)
                m_ptChildOffset.y = m_pScrollBarVer->GetValue();
            else break;
            // SB修改之后调用
            this->mark_world_changed();
            this->Invalidate();
            return Event_Accept;
        }
    }
    // 超类处理消息
    return Super::DoEvent(sender, arg);
}

/// <summary>
/// Does the wheel.
/// </summary>
/// <param name="index">The index.</param>
/// <param name="wheel">The wheel.</param>
/// <returns></returns>
auto LongUI::UIScrollArea::do_wheel(int index, float wheel) noexcept ->EventAccept {
    const auto maxv = index[&m_maxScrollSize.width];
    if (maxv > 0) {
        // 位置变动检查
        auto& offset = index[&m_ptChildOffset.x];
        const auto line_height = UIManager.GetWheelScrollLines();
        float pos = offset - index[&this->line_size.width] * wheel * line_height;
        pos = impl::clamp(pos, 0.f, maxv);
        // 已经修改
        if (pos != offset) {
            offset = pos;
            this->mark_world_changed();
            this->Invalidate();
            // 同步SB
            this->sync_scroll_bar(luiref m_ptChildOffset);
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

#if 0
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
        this->layout_scroll_bar(this->RefBox().GetContentSize());

    return Super::Update(reason);
}
#endif

PCN_NOINLINE
/// <summary>
/// Sums the children flex.
/// </summary>
/// <returns></returns>
auto LongUI::UIControl::SumChildrenFlex() const noexcept -> float {
    float sum = 0.f;
    for (auto& child : *this) if (child.IsVaildInLayout())
        sum += child.RefStyle().flex;
    return sum;
}

/// <summary>
/// Synchronizes the scroll bar.
/// </summary>
/// <returns></returns>
void LongUI::UIScrollArea::sync_scroll_bar(Point2F& offset) noexcept {
    m_maxScrollSize = { 0, 0 };
    const auto csize = m_oBox.GetContentSize();
    const bool hok = m_pScrollBarHor && m_pScrollBarHor->IsVisible();
    const bool vok = m_pScrollBarVer && m_pScrollBarVer->IsVisible();
    // 交界区
    const Size2F corner = {
        vok ? m_pScrollBarVer->GetBoxWidth() : 0.f,
        hok ? m_pScrollBarHor->GetBoxHeight() : 0.f
    };
    const auto& min_scroll = m_minScrollSize;
    // 水平滚动条
    if (hok) {
        m_pScrollBarHor->SetIncrement(this->line_size.width);
        m_pScrollBarHor->SetPageIncrement(csize.width - corner.width);
        m_maxScrollSize.width = min_scroll.width - csize.width + corner.width;
        m_pScrollBarHor->SetMax(m_maxScrollSize.width);
        m_pScrollBarHor->SetValue(offset.x);
        //m_pScrollBarHor->SetSingleStep(m_szSingleStep.width);
    }
    else offset.x = 0.f;
    // 垂直滚动条
    if (vok) {
        m_pScrollBarVer->SetIncrement(this->line_size.height);
        m_pScrollBarVer->SetPageIncrement(csize.height - corner.height);
        m_maxScrollSize.height = min_scroll.height - csize.height + corner.height;
        m_pScrollBarVer->SetMax(m_maxScrollSize.height);
        m_pScrollBarVer->SetValue(offset.y);
        //m_pScrollBarVer->SetSingleStep(m_szSingleStep.height);
    }
    else offset.y = 0.f;
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
        assert(m_pScrollBarHor && m_pScrollBarVer);
        // 没有就尝试创建并且初始化
        if (!m_pCorner) {
            // 插入尾部
            const auto context = m_pFinalEnd;
            m_pFinalEnd = this->end();
            m_pCorner = new(std::nothrow) UIControl(this);
            m_pFinalEnd = context;
            if (!m_pCorner) return;
#ifndef NDEBUG
            m_pCorner->name_dbg = "scrollarea::corner";
#endif
            UIControlPrivate::FixedAttachment(*m_pCorner);
            this->set_child_fixed_attachment(*m_pCorner);
            UIControlPrivate::SetAppearance(*m_pCorner, Appearance_Resizer);
        }
        // 有就进行设置
        if (m_pCorner) {
            m_pCorner->SetVisible(true);
            this->resize_child(*m_pCorner, size);
            const auto x = m_pScrollBarVer->GetPos().x;
            const auto y = m_pScrollBarHor->GetPos().y;
            m_pCorner->SetPos({ x, y });
        }
    }
    // 有就有, 没有就算了
    else if(m_pCorner) m_pCorner->SetVisible(false);
}


inline
/// <summary>
/// Creates the scrollbar.
/// </summary>
/// <returns></returns>
auto LongUI::UIScrollArea::create_scrollbar(AttributeOrient o) noexcept -> UIScrollBar * {
    // 插入尾部
    auto context = m_pFinalEnd;
    m_pFinalEnd = this->end();
    UIScrollBar* bar = nullptr;
    // 创建滚动条
    if (bar = new(std::nothrow) UIScrollBar{ this, o }) {
#ifndef NDEBUG
        if (o == Orient_Horizontal)
            bar->name_dbg = "scrollarea::hscrollbar";
        else
            bar->name_dbg = "scrollarea::vscrollbar";
#endif
        this->resize_child(*bar, {});
        this->set_child_fixed_attachment(*bar);
        UIControlPrivate::SetGuiEvent2Parent(*bar);
        // 首次插入 则记录起点
        if (context == m_pFinalEnd) context = bar;
    }
    m_pFinalEnd = context;
    return bar;
}

PCN_NOINLINE
/// <summary>
/// Layouts the scrollbar.
/// </summary>
/// <param name="notenough">if set to <c>true</c> [notenough].</param>
/// <returns></returns>
auto LongUI::UIScrollArea::layout_scrollbar(bool notenough, bool index) noexcept -> float {
    const auto baseflow = index[&m_oStyle.overflow_xex];
    const auto overflow = static_cast<AttributeOverflow>(baseflow & 3);
    // 检查绝不可能的情况
    const bool nosb = overflow & 1;
    if (nosb) return 0.0f;
    auto& scrollbar = index[&m_pScrollBarHor];
    // 可能存在滚动条
    if (overflow == Overflow_Auto) {
        // 不够用就显示
        if (!notenough) {
            if (scrollbar) scrollbar->SetVisible(false);
            return 0.f;
        }
    }
    // 需要滚动条
    const auto orient = static_cast<AttributeOrient>(index);
    if (!scrollbar) scrollbar = this->create_scrollbar(orient);
    if (!scrollbar) return 0.0f;
    return (&scrollbar->RefStyle().limited.width)[index ^ 1];
}


PCN_NOINLINE
/// <summary>
/// Layouts the size of the content.
/// </summary>
/// <returns></returns>
auto LongUI::UIScrollArea::layout_scroll_bar(Point2F& offset) noexcept -> Size2F {
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
    constexpr float MDW = MIN_SCROLLBAR_DISPLAY_SIZE;
    constexpr float MDH = MIN_SCROLLBAR_DISPLAY_SIZE;
    // 内容大小
    const auto content_size = this->RefBox().GetContentSize();
    const auto overflow_xex = this->RefStyle().overflow_xex;
    const auto overflow_y = this->RefStyle().overflow_y;
    // 不存在的
    if (1 & overflow_xex & overflow_y) return content_size;
    const auto scroll = m_minScrollSize; // m_oStyle.fitting;
    auto real_size = content_size;
    float vsbar, hsbar;
    // 遍历两次
    for (int i = 0; i != 2; ++i) {
        // 需要显示VSB
        const bool vsbdisplay = real_size.width > MDW &&
            scroll.height > real_size.height;
        // 需要显示HSB
        const bool hsbdisplay = real_size.height > MDW &&
            scroll.width > real_size.width;
        // 获取VSB长度
        vsbar = this->layout_scrollbar(vsbdisplay, 1);
        // 获取HSB长度
        hsbar = this->layout_scrollbar(hsbdisplay, 0);
        real_size.width = content_size.width - vsbar;
        real_size.height = content_size.height - hsbar;
    }
    // 需要再次布局
    if (is_need_relayout()) return real_size;
    // 设置VSB位置: 正向-右侧 反向-左侧
    if (vsbar > 0.f) {
        resize_child(*m_pScrollBarVer, { vsbar, real_size.height });
        Point2F pos = this->RefBox().GetContentPos();
        const bool normaldir = m_state.direction == Dir_Normal;
        if (normaldir) pos.x += real_size.width;
        m_pScrollBarVer->SetPos(pos);
        m_pScrollBarVer->SetVisible(true);
    }
    // 设置HSB位置: 正向-下侧 反向-上侧
    if (hsbar > 0.f) {
        resize_child(*m_pScrollBarHor, { real_size.width , hsbar });
        Point2F pos = this->RefBox().GetContentPos();
        const bool normaldir = m_state.direction == Dir_Normal;
        if (normaldir) pos.y += real_size.height;
        m_pScrollBarHor->SetPos(pos);
        m_pScrollBarHor->SetVisible(true);
    }
    // 同步SB显示
    this->sync_scroll_bar(luiref offset);
    // 返回剩余大小
    return real_size;
}

/// <summary>
/// Gets the layout position.
/// </summary>
/// <returns></returns>
auto LongUI::UIScrollArea::get_layout_position() const noexcept -> Point2F {
    auto base = this->RefBox().GetContentPos();
    //const auto base = Point2F{};
    // 反向- 左上角+[VSB, HSB]
    if (m_state.direction) {
        if (m_pScrollBarVer && m_pScrollBarVer->IsVisible())
            base.x += m_pScrollBarVer->GetBoxWidth();
        if (m_pScrollBarHor && m_pScrollBarHor->IsVisible())
            base.y += m_pScrollBarHor->GetBoxHeight();
    }
    return base;
}