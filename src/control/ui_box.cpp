#include <control/ui_boxlayout.h>
#include <control/ui_splitter.h>
#include <core/ui_ctrlmeta.h>
#include <event/ui_splitter_event.h>

#include <algorithm>
#include <cassert>

#include <debugger/ui_debug.h>

#include "../private/ui_private_control.h"



// ui namespace
namespace LongUI {
    // UIVBoxLayout类 元信息
    LUI_CONTROL_META_INFO(UIVBoxLayout, "vbox");
    // UIHBoxLayout类 元信息
    LUI_CONTROL_META_INFO(UIHBoxLayout, "hbox");
    // UIBoxLayout类 元信息
    LUI_CONTROL_META_INFO(UIBoxLayout, "box");
    // impl
    namespace impl {
        PCN_NOINLINE
        /// <summary>
        /// Gets the align factor.
        /// </summary>
        /// <param name="align">The align.</param>
        /// <returns></returns>
        auto get_align_factor(AttributeAlign align) noexcept {
            switch (align)
            {
            case LongUI::Align_Stretcht:
                return -1.0f;
            default: assert(!"error"); 
                [[fallthrough]];
            case LongUI::Align_Baseline:
                [[fallthrough]];
            case LongUI::Align_Start:
                return 0.0f;
            case LongUI::Align_Center:
                return 0.5f;
            case LongUI::Align_End:
                return 1.0f;
            }
        }
    }
}


/// <summary>
/// Initializes a new instance of the <see cref="UIBoxLayout" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIBoxLayout::UIBoxLayout(UIControl* parent, const MetaControl& meta) noexcept 
    : Super(parent, meta) {
    m_state.orient = Orient_Vertical;
}

/// <summary>
/// Initializes a new instance of the <see cref="UIVBoxLayout" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
/// <returns></returns>
LongUI::UIVBoxLayout::UIVBoxLayout(UIControl* parent, const MetaControl& meta) noexcept 
    : Super(parent, meta) {
    m_state.orient = Orient_Vertical;
}

/// <summary>
/// Initializes a new instance of the <see cref="UIHBoxLayout" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
/// <returns></returns>
LongUI::UIHBoxLayout::UIHBoxLayout(UIControl* parent, const MetaControl& meta) noexcept 
    : Super(parent, meta) {
    m_state.orient = Orient_Horizontal;
}

/*/// <summary>
/// Refreshes the minimum size of the content.
/// </summary>
/// <returns></returns>
void LongUI::UIBoxLayout::refresh_min_content_size() noexcept {

}*/



/// <summary>
/// need relayout ?
/// </summary>
/// <returns></returns>
auto LongUI::UIBoxLayout::is_need_relayout() noexcept -> UpdateReason {
    return m_state.reason & Reason_BasicRelayout;
};

PCN_NOINLINE
/// <summary>
/// Relayouts the vbox
/// </summary>
/// <returns></returns>
void LongUI::UIBoxLayout::relayout_v() noexcept {
#ifndef NDEBUG
    if (!std::strcmp(name_dbg, "scrollarea::vscrollbar")) {
        int bbk = 9;
    }
#endif
    // - 获取剩余长度
    const auto get_remain_length = [this](Size2F sb) noexcept {
        return sb.height - m_minScrollSize.height;
    };
    // - 位置下移
    const auto pos_move_next = [](Point2F& pos, Size2F size) noexcept {
        pos.y += size.height;
    };
    // - 添加弹性
    const auto add_flex = [](Size2F& size, float length) noexcept {
        size.height += length;
    };
    // - 调整对齐
    const auto adjust_align = [](Size2F& size, Size2F maxs, float f) noexcept {
        // 仅仅调整位置
        if (f >= 0.0f) {
            return Point2F{ (maxs.width - size.width) * f, 0.f };
        }
        // 仅仅调整大小
        else {
            if (maxs.width > size.width) size.width = maxs.width;
            return Point2F{ 0.f, 0.f };
        }
    };
    // 0. 计算预备数据
    const float align_factor = impl::get_align_factor(m_oStyle.align);
    // 1. 遍历子控件, 将有效可变权重加起来
    const float flex_sum = this->sum_children_flex();
    // 2. 加入SB布局
    const auto remaining = this->layout_scroll_bar();
    // 需要重新布局
    if (this->is_need_relayout())  return;
    // 3. 计算每权重长度
    const float len_in_unit = flex_sum > 0.f ?
        std::max(get_remain_length(remaining), 0.f) / flex_sum : 0.f;
    Point2F pos = this->get_layout_position();
    // pi.计算pack位置
    if (len_in_unit == 0.f) {
        const float fa = static_cast<float>(m_oStyle.pack) * 0.5f;
        pos.y += get_remain_length(remaining) * fa;
    }
    // 4. 遍历控件
    for (auto& child : *this) {
#ifndef NDEBUG
        //if (!std::strcmp(child.name_dbg, "scrollbar::slider")) {
        //    int bbk = 9;
        //}
#endif
        // 有效才处理
        if (child.IsVaildInLayout()) {
            // 先考虑使用最小尺寸
            auto size = child.GetMinSize();
            // 对应方向分别使用弹性布局
            add_flex(size, child.GetStyle().flex * len_in_unit);
            // 调整对齐
            const auto opos = adjust_align(size, remaining, align_factor);
            // 但是不能超过本身限制
            size.width = std::min(child.GetMaxSize().width, size.width);
            size.height = std::min(child.GetMaxSize().height, size.height);
            // 调整位置大小
            child.SetPos(pos + opos); this->resize_child(child, size);
            // 移动到下一个位置
            pos_move_next(pos, size);
        }
    }
}


PCN_NOINLINE
/// <summary>
/// Relayouts the h.
/// </summary>
/// <returns></returns>
void LongUI::UIBoxLayout::relayout_h() noexcept {
#ifndef NDEBUG
    if (!std::strcmp(name_dbg, "listcols")) {
        int bbk = 9;
    }
#endif
    // - 获取剩余长度
    const auto get_remain_length = [this](Size2F sb) noexcept {
        return sb.width - m_minScrollSize.width;
    };
    // - 位置下移
    const auto pos_move_next = [](Point2F& pos, Size2F size) noexcept {
        pos.x += size.width;
    };
    // - 添加弹性
    const auto add_flex = [](Size2F& size, float length) noexcept {
        size.width += length;
    };
    // - 调整对齐
    const auto adjust_align = [](Size2F& size, Size2F maxs, float f) noexcept {
        // 仅仅调整位置
        if (f >= 0.0f) {
            return Point2F{ 0.f, (maxs.height - size.height) * f };
        }
        // 仅仅调整大小
        else {
            if (maxs.height > size.height) size.height = maxs.height;
            return Point2F{ 0.f, 0.f };
        }
    };
    // 0. 计算预备数据
    const float align_factor = impl::get_align_factor(m_oStyle.align);
    // 1. 遍历子控件, 将有效可变权重加起来
    const float flex_sum = this->sum_children_flex();
    // 2. 加入SB布局
    const auto remaining = this->layout_scroll_bar();
    // 需要重新布局的话就没有必要继续算了
    if (this->is_need_relayout()) return;
    // 3. 计算每权重长度
    const float len_in_unit = flex_sum > 0.f ?
        std::max(get_remain_length(remaining), 0.f) / flex_sum : 0.f;
    Point2F pos = this->get_layout_position();
    // pi.计算pack位置
    if (len_in_unit == 0.f) {
        const float fa = static_cast<float>(m_oStyle.pack) * 0.5f;
        pos.x += get_remain_length(remaining) * fa;
    }
    // 4. 遍历控件
    for (auto& child : *this) {
        // 有效才处理
        if (child.IsVaildInLayout()) {
            // 先考虑使用最小尺寸
            auto size = child.GetMinSize();
            // 对应方向分别使用弹性布局
            add_flex(size, child.GetStyle().flex * len_in_unit);
            // 调整对齐
            const auto opos = adjust_align(size, remaining, align_factor);
            // 但是不能超过本身限制
            size.width = std::min(child.GetMaxSize().width, size.width);
            size.height = std::min(child.GetMaxSize().height, size.height);
            // 调整位置大小
            child.SetPos(pos + opos); this->resize_child(child, size);
            // 移动到下一个位置
            pos_move_next(pos, size);
        }
    }
}

/// <summary>
/// updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIBoxLayout::Update(UpdateReason reason) noexcept {
    // TODO: 修改
    constexpr UpdateReason relayout_reason
        = Reason_ParentChanged
        | Reason_ChildIndexChanged
        | Reason_SizeChanged
        | Reason_BoxChanged
        | Reason_ChildLayoutChanged
        ;
    /*
        偏向于小的大小进行布局

        A(0)__B(0)__C(0)    布局A时, 只有B, B权重0, 
                |___D(0)    没有设置最小, 则B设为0
                |___E(1)     

        B(0)__C(0)          布局B时, 有B,C,D, E权重1,
          |___D(0)          没有设置最小, 则E设为B一
          |___E(1)          样大小, 其余设为0

    */
    if (reason & Reason_BasicRelayout) this->relayout_this();
    return Super::Update(reason);
}

/// <summary>
/// relayout this
/// </summary>
/// <returns></returns>
void LongUI::UIBoxLayout::relayout_this() noexcept {
    /*
        偏向于小的大小进行布局

        A(0)__B(0)__C(0)    布局A时, 只有B, B权重0,
                |___D(0)    没有设置最小, 则B设为0
                |___E(1)

        B(0)__C(0)          布局B时, 有B,C,D, E权重1,
          |___D(0)          没有设置最小, 则E设为B一
          |___E(1)          样大小, 其余设为0

    */
    const auto s = this->GetSize();
    if (s.width * s.height <= 0.f) return;
    // 存在子控件才计算
    if (this->GetChildrenCount()) {
        // 更新布局
        const auto ishor = m_state.orient == Orient_Horizontal;
        return ishor ? this->relayout_h() : this->relayout_v();
        // 更新子控件
        for (auto& child : *this)
            child.NeedUpdate(Reason_NonChanged);
    }
}


/// <summary>
/// Refreshes the minimum.
/// </summary>
/// <returns></returns>
void LongUI::UIBoxLayout::refresh_min() noexcept {
#ifndef NDEBUG
    if (!std::strcmp(name_dbg, "scrollarea::vscrollbar")) {
        int bbk = 9;
    }
#endif
    const bool ishor = m_state.orient == Orient_Horizontal;
    Size2F minsize = {};
    // 遍历控件
    for (auto& child : *this) {
#ifndef NDEBUG
        if (!std::strcmp(child.name_dbg, "scrollbar::slider")) {
            int bbk = 9;
        }
#endif
        // 可见即可
        if (child.IsVaildInLayout()) {
            const auto ms = child.GetMinSize();
            // 水平布局
            if (ishor) {
                minsize.width += ms.width;
                minsize.height = std::max(minsize.height, ms.height);
            }
            // 垂直布局
            else {
                minsize.height += ms.height;
                minsize.width = std::max(minsize.width, ms.width);
            }
        }
    }
    // 更新值
    m_minScrollSize = minsize;
    this->set_contect_minsize(minsize);
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIBoxLayout::DoEvent(UIControl* sender, const EventArg& e) noexcept -> EventAccept {
    // ---------------- 事件处理分支
    switch (e.nevent)
    {
    case NoticeEvent::Event_RefreshBoxMinSize:
        this->refresh_min();
        return Event_Accept;
    case NoticeEvent::Event_Splitter:
        this->move_splitter(*longui_cast<UISplitter*>(sender), [&e]() noexcept {
            const auto& ev = static_cast<const EventSplitterArg&>(e);
            return Point2F{ ev.offset_x, ev.offset_y };
        }());
        return Event_Accept;
    }
    return Super::DoEvent(sender, e);
}



/// <summary>
/// Moves the splitter.
/// </summary>
/// <param name="splitter">The splitter.</param>
/// <param name="offset">The offset.</param>
/// <returns></returns>
void LongUI::UIBoxLayout::move_splitter(UISplitter& splitter, Point2F offset) noexcept {
    assert(splitter.GetParent() == this);
    // 不能是第一个
    if (splitter.IsFirstChild()) return;
    // 最后
    auto& p = UIControlPrivate::Prev(splitter);
    const auto index = this->GetOrient() == Orient_Horizontal ? 0 : 1;
    const auto o = index[&offset.x];
    // 计算大小
    auto szp = p.GetSize(); index[&szp.width] += o;
    if (index[&szp.width] <= index[&p.GetBox().minsize.width]) return;
    // 不是最后一个?
    if (!splitter.IsLastChild()) {
        // 修改后面的
        auto& n = UIControlPrivate::Next(splitter);
        auto szn = n.GetSize(); index[&szn.width] -= o;
        // 不够?
        if (index[&szn.width] <= index[&n.GetBox().minsize.width]) return;
        n.SetStyleSize(szn);
    }
    // 重置
    p.SetStyleSize(szp);
}


/// <summary>
/// Finalizes an instance of the <see cref="UIBoxLayout"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIBoxLayout::~UIBoxLayout() noexcept {
}


/// <summary>
/// Sets the orient.
/// </summary>
/// <param name="o">The o.</param>
/// <returns></returns>
void LongUI::UIBoxLayout::SetOrient(AttributeOrient o) noexcept {
    const bool orient = o & 1;
    if (orient != m_state.orient) {
        m_state.orient = orient;
        this->NeedUpdate(Reason_ChildLayoutChanged);
    }
}



/// <summary>
/// Finalizes an instance of the <see cref="UIVBoxLayout"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIVBoxLayout::~UIVBoxLayout() noexcept {
}


/// <summary>
/// Finalizes an instance of the <see cref="UIHBoxLayout"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIHBoxLayout::~UIHBoxLayout() noexcept {
}


#ifdef LUI_ACCESSIBLE
#include <accessible/ui_accessible_event.h>
#include <core/ui_string.h>
/// <summary>
/// Accessibles the specified .
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
auto LongUI::UIBoxLayout::accessible(const AccessibleEventArg& args) noexcept->EventAccept {
    switch (args.event)
    {
        using get0_t = AccessibleGetPatternsArg;
        using get1_t = AccessibleGetAccNameArg;
        using getd_t = AccessibleGetDescriptionArg;
        CUIString* output;
    case AccessibleEvent::Event_All_GetAccessibleName:
        // 获取Acc名称  []
        output = static_cast<const get1_t&>(args).name;
        {
            CUIStringU8 string = m_state.orient == Orient_Horizontal ?
                "hor-"_sv : "ver-"_sv;
            string.append(m_refMetaInfo.element_name);
            if (m_id.id[0]) {
                string.append(": "_sv);
                string.append(m_id.id);
            }
            *output = CUIString::FromUtf8(string.view());
        }
        return Event_Accept;
    }
    return Super::accessible(args);
}
#endif