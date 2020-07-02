#include <control/ui_box.h>
#include <control/ui_splitter.h>
#include <core/ui_ctrlmeta.h>
#include <event/ui_splitter_event.h>
#include <util/ui_little_math.h>

#include <algorithm>
#include <cassert>

#include <debugger/ui_debug.h>

#include "../private/ui_private_control.h"



// ui namespace
namespace LongUI {
    // UIVBoxLayout类 元信息
    LUI_CONTROL_META_INFO(UIVBoxLayout, "vbox");
    // UIBoxLayout类 元信息
    LUI_CONTROL_META_INFO(UIBoxLayout, "box");
}

// ui::impl namespace
namespace LongUI { namespace impl {
    /// <summary>
    /// Gets the align factor.
    /// </summary>
    /// <param name="align">The align.</param>
    /// <returns></returns>
    auto get_align_factor(AttributeAlign align) noexcept {
        switch (align)
        {
        default: assert(!"error");
        case LongUI::Align_Baseline:
            // 不打算实现基线对齐
            [[fallthrough]];
        case LongUI::Align_Stretcht:
            [[fallthrough]];
        case LongUI::Align_Start:
            return 0.0f;
        case LongUI::Align_Center:
            return 0.5f;
        case LongUI::Align_End:
            return 1.0f;
        }
    }
}}


/// <summary>
/// Initializes a new instance of the <see cref="UIBoxLayout" /> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UIBoxLayout::UIBoxLayout(const MetaControl& meta) noexcept : Super(meta) {
    m_state.orient = Orient_Horizontal;
    m_oBox.size = { 0 };
}

/// <summary>
/// Initializes a new instance of the <see cref="UIVBoxLayout" /> class.
/// </summary>
/// <param name="meta">The meta.</param>
/// <returns></returns>
LongUI::UIVBoxLayout::UIVBoxLayout(const MetaControl& meta) noexcept : Super(meta) {
    m_state.orient = Orient_Vertical;
}

PCN_NOINLINE
/// <summary>
/// Relayouts the vbox
/// </summary>
/// <returns></returns>
void LongUI::UIBoxLayout::relayout_v() noexcept {
#ifndef NDEBUG
    if (!std::strcmp(name_dbg, "scrollarea::vscrollbar")) {
        int bk = 9;
    }
    if (!std::strcmp(m_id.id, "a")) {
        int bk = 9;
    }
    if (this->GetLevel() == 2) {
        int bk = 9;
    }
#endif
    // - 获取剩余长度
    const auto get_remain_length = [this](Size2F sb) noexcept {
        return sb.height - m_szRealFitting.height;
    };
    // - 获取侧轴剩余长度
    const auto get_remain_axis = [this](Size2F sb) noexcept {
        return std::max(sb.width, m_minScrollSize.width);
    };
    // - 位置下移
    const auto pos_move_next = [](Point2F& pos, Size2F size) noexcept {
        pos.y += size.height;
    };
    // - 调整对齐
    const auto adjust_align = [](Size2F size, float maxs, float f) noexcept {
        // 调整位置
        return Point2F{ (maxs - size.width) * f, 0.f };
    };
    // - 获取标准尺寸
    const auto get_stdsize = [](UIControl& child, float maxs, AttributeAlign a) noexcept {
        auto& style = child.RefStyle();
        Size2F size;
        size.width = style.fitting.width;
        size.height = UIControlPrivate::GetLayoutValueF(child);
        const auto exsize = child.GetBoxExSize();
        if (a == AttributeAlign::Align_Stretcht) {
            size.width = maxs - exsize.width;
            size.width = detail::clamp(size.width, style.limited.width, style.maxsize.width);
        }
        size.width += exsize.width;
        size.height += exsize.height;
        return size;
    };
    // - 获取建议尺寸
    const auto get_fiting_size = [](UIControl& child) noexcept {
        return child.RefStyle().fitting.height;
    };
    // - 钳制尺寸
    const auto clamp_size = [](UIControl& child, float v) noexcept {
        return detail::clamp(v, 
            child.RefStyle().limited.height, 
            child.RefStyle().maxsize.height
        );
    };
    // - 设置PACK位置
    const auto setup_packing = [](Point2F pt, float v) noexcept {
        pt.y += v; return pt;
    };
    // ------------------------------------------------------------------------
    //                                分界线
    // ------------------------------------------------------------------------
    // 加入SB布局
    const auto remaining = this->layout_scroll_bar();
    // 需要重新布局 本次没有必要再计算了
    if (this->is_need_relayout()) return;
    // 初始化数据
    float flex_sum = 0.f;
    auto remain_len = get_remain_length(remaining);
    const auto remian_axis = get_remain_axis(remaining);
    for (auto & child : (*this)) if (child.IsVaildInLayout()) {
        flex_sum += child.RefStyle().flex;
        UIControlPrivate::SetLayoutValueF(child, get_fiting_size(child));
    }
    // 由于可能浮点误差这里硬编码为: 动画30Hz中1变化可以接受
    constexpr float deviation = 1.f / 30.f;
    while (flex_sum > deviation) {
        bool break_this = true;
        const auto unit = remain_len / flex_sum;
        for (auto & child : (*this)) if (child.IsVaildInLayout()) {
            const auto flex = child.RefStyle().flex;
            const auto v = UIControlPrivate::GetLayoutValueF(child);
            const auto p = flex * unit;
            const auto n = clamp_size(child, v + p);
            if (n != v) {
                break_this = false;
                remain_len -= n - v;
                flex_sum -= flex;
                UIControlPrivate::SetLayoutValueF(child, n);
            }
        }
        if (break_this) break;
    }
    // 计算pack位置
    const float fa = [this]() noexcept {
        const auto base = static_cast<float>(m_oStyle.pack) * 0.5f;
        return m_state.direction ? 1.f - base : base;
    }();
    Point2F pos = this->get_layout_position();
    pos = setup_packing(pos, remain_len * fa);
    // 计算align位置
    const auto style_align = m_oStyle.align;
    const float safe_factor = impl::get_align_factor(style_align);
    // 最终遍历
    const auto sp = this->make_sp_traversal();
    for (auto itr = sp.begin; itr != sp.end; itr = LongUI::SpNext(itr, sp.next)) {
        auto& child = *itr;
        // 有效才处理
        if (!child.IsVaildInLayout()) continue;
        // 获取标准大小
        const auto size = get_stdsize(child, remian_axis, style_align);
        // 调整对齐
        const auto opos = adjust_align(size, remian_axis, safe_factor);
        // 调整位置大小
        child.SetPos(pos + opos); this->resize_child(child, size);
        // 移动到下一个位置
        pos_move_next(pos, size);
    }
}


PCN_NOINLINE
/// <summary>
/// Relayouts the h.
/// </summary>
/// <returns></returns>
void LongUI::UIBoxLayout::relayout_h() noexcept {
#ifndef NDEBUG
    if (!std::strcmp(m_id.id, "editable_combobox")) {
        int bk = 9;
    }
    if (this->IsTopLevel()) {
        int bk = 9;
    }
#endif
    // - 获取剩余长度
    const auto get_remain_length = [this](Size2F sb) noexcept {
        return sb.width - m_szRealFitting.width;
    };
    // - 位置下移
    const auto pos_move_next = [](Point2F& pos, Size2F size) noexcept {
        pos.x += size.width;
    };
    // - 调整对齐
    const auto adjust_align = [](Size2F size, Size2F maxs, float f) noexcept {
        // 调整位置
        return Point2F{ 0.f, (maxs.height - size.height) * f };
    };
    // 获取标准大小
    const auto get_stdsize = [](UIControl& child, Size2F maxs, AttributeAlign a) noexcept {
        auto& style = child.RefStyle();
        Size2F size;
        size.width = UIControlPrivate::GetLayoutValueF(child);
        size.height = style.fitting.height;
        const auto exsize = child.GetBoxExSize();
        if (a == AttributeAlign::Align_Stretcht) {
            size.height = maxs.height - exsize.height;
            size.height = detail::clamp(size.height, style.limited.height, style.maxsize.height);
        }
        size.width += exsize.width;
        size.height += exsize.height;
        return size;
    };
    // 获取建议尺寸
    const auto get_fiting_size = [](UIControl& child) noexcept {
        return child.RefStyle().fitting.width;
    };
    // 钳制尺寸
    const auto clamp_size = [](UIControl& child, float v) noexcept {
        return detail::clamp(v, 
            child.RefStyle().limited.width, 
            child.RefStyle().maxsize.width
        );
    };
    // 设置PACK位置
    const auto setup_packing = [](Point2F pt, float v) noexcept {
        pt.x += v; return pt;
    };
    // ------------------------------------------------------------------------
    //                                分界线　
    // ------------------------------------------------------------------------
    // 加入SB布局
    const auto remaining = this->layout_scroll_bar();
    // 需要重新布局 本次没有必要再计算了
    if (this->is_need_relayout()) return;
    // 初始化数据
    float flex_sum = 0.f;
    auto remain_len = get_remain_length(remaining);
    for (auto & child : (*this)) if (child.IsVaildInLayout()) {
        flex_sum += child.RefStyle().flex;
        UIControlPrivate::SetLayoutValueF(child, get_fiting_size(child));
    }
    // 由于可能浮点误差这里硬编码为: 动画30Hz中1变化可以接受
    constexpr float deviation = 1.f / 30.f;
    while (flex_sum > deviation) {
        bool break_this = true;
        const auto unit = remain_len / flex_sum;
        for (auto & child : (*this)) if (child.IsVaildInLayout()) {
            const auto flex = child.RefStyle().flex;
            const auto v = UIControlPrivate::GetLayoutValueF(child);
            const auto p = flex * unit;
            const auto n = clamp_size(child, v + p);
            if (n != v) {
                break_this = false;
                remain_len -= n - v;
                flex_sum -= flex;
                UIControlPrivate::SetLayoutValueF(child, n);
            }
        }
        if (break_this) break;
    }
    // 计算pack位置
    const float fa = [this]() noexcept {
        const auto base = static_cast<float>(m_oStyle.pack) * 0.5f;
        return m_state.direction ? 1.f - base : base;
    }();
    Point2F pos = this->get_layout_position();
    pos = setup_packing(pos, remain_len * fa);
    // 计算align位置
    const auto style_align = m_oStyle.align;
    const float safe_factor = impl::get_align_factor(style_align);
    // 最终遍历
    const auto sp = this->make_sp_traversal();
    for (auto itr = sp.begin; itr != sp.end; itr = LongUI::SpNext(itr, sp.next)) {
        auto& child = *itr;
        // 有效才处理
        if (!child.IsVaildInLayout()) continue;
        // 获取标准大小
        const auto size = get_stdsize(child, remaining, style_align);
        // 调整对齐
        const auto opos = adjust_align(size, remaining, safe_factor);
        // 调整位置大小
        child.SetPos(pos + opos); this->resize_child(child, size);
        // 移动到下一个位置
        pos_move_next(pos, size);
    }
}

/// <summary>
/// updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIBoxLayout::Update(UpdateReason reason) noexcept {
    /*
        偏向于小的大小进行布局

        A(0)__B(0)__C(0)    布局A时, 只有B, B权重0, 
                |___D(0)    没有设置最小, 则B设为0
                |___E(1)     

        B(0)__C(0)          布局B时, 有B,C,D, E权重1,
          |___D(0)          没有设置最小, 则E设为B一
          |___E(1)          样大小, 其余设为0

    */
    if (reason & Reason_BasicUpdateFitting) this->refresh_fitting();
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
    // 存在子控件才计算
    if (!this->GetChildrenCount()) return;
    // 面积不够
    const auto& consize = m_oBox.size;
    if (consize.width <= 0 || consize.height <= 0)  return;
    // 更新布局
    m_state.orient == Orient_Horizontal ? this->relayout_h() : this->relayout_v();
}

// ui::impl
namespace LongUI { namespace impl {
    /// <summary>
    /// get children boxfitting : hor mode
    /// </summary>
    /// <param name="ctrl"></param>
    /// <param name="min"></param>
    /// <param name="fit"></param>
    /// <returns></returns>
    void children_boxfitting_hor(UIControl& ctrl, Size2F& min, Size2F& fit) noexcept {
        Size2F m = { }, f = { };
        // 遍历控件
        for (auto& child : ctrl) {
            if (!child.IsVaildInLayout()) continue;
            const auto exsize = child.GetBoxExSize();
            // 最小值
            // 允许缩放 -> 最小宽度 否则 -> 建议宽度
            // 不管     -> 最小高度
            m.width += exsize.width;
            m.width += child.RefStyle().flex
                ? child.RefStyle().limited.width
                : child.RefStyle().fitting.width
                ;
            m.height = std::max(m.height, child.RefStyle().limited.height + exsize.height);
            // 建议值
            f.width += exsize.width + child.RefStyle().fitting.width;
            f.height = std::max(f.height, child.RefStyle().fitting.height + exsize.height);
        }
        min = m; fit = f;
    }
    /// <summary>
    /// get children boxfitting : ver mode
    /// </summary>
    /// <param name="ctrl"></param>
    /// <param name="min"></param>
    /// <param name="fit"></param>
    /// <returns></returns>
    void children_boxfitting_ver(UIControl& ctrl, Size2F& min, Size2F& fit) noexcept {
        Size2F m = {}, f = {};
        // 遍历控件
        for (auto& child : ctrl) {
            if (!child.IsVaildInLayout()) continue;
            const auto exsize = child.GetBoxExSize();
            // 最小值
            // 不管     -> 最小宽度
            // 允许缩放 -> 最小高度 否则 -> 建议高度
            m.width = std::max(m.width, child.RefStyle().limited.width + exsize.width);
            m.height += exsize.height;
            m.height += child.RefStyle().flex
                ? child.RefStyle().limited.height
                : child.RefStyle().fitting.height
                ;
            // 建议值
            f.width = std::max(f.width, child.RefStyle().fitting.width + exsize.width);
            f.height += exsize.height + child.RefStyle().fitting.height;
        }
        min = m; fit = f;
    }
}}

/// <summary>
/// Refreshes the minimum.
/// </summary>
/// <returns></returns>
void LongUI::UIBoxLayout::refresh_fitting() noexcept {
#ifndef NDEBUG
    if (!std::strcmp(m_id.id, "b")) {
        int bk = 9;
    }
    if (!std::strcmp(name_dbg, "scrollarea::vscrollbar")) {
        int bk = 9;
    }
#endif
    if (m_state.orient == Orient_Horizontal) 
        impl::children_boxfitting_hor(*this, m_minScrollSize, m_szRealFitting);
    else 
        impl::children_boxfitting_ver(*this, m_minScrollSize, m_szRealFitting);
    Size2F limited = m_minScrollSize;
    Size2F fitting = m_szRealFitting;
#ifdef LUI_BOXSIZING_BORDER_BOX
    const auto bpw = m_oBox.border.left + m_oBox.border.right
        + m_oBox.padding.left + m_oBox.padding.right;
    const auto bph = m_oBox.border.top + m_oBox.border.bottom
        + m_oBox.padding.top + m_oBox.padding.bottom;
    limited.width += bpw; limited.height += bph;
    fitting.width += bpw; fitting.height += bph;
#endif
    // 更新值
    if (m_oStyle.overflow_xex & 1)
        this->set_limited_width_lp(limited.width);
    if (m_oStyle.overflow_y & 1)
        this->set_limited_height_lp(limited.height);
    this->update_fitting_size(fitting);
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIBoxLayout::DoEvent(UIControl* sender, const EventArg& e) noexcept -> EventAccept {
    switch (e.nevent)
    {
    case NoticeEvent::Event_Splitter:
        assert(sender && "BAD ACTION");
        this->move_splitter(*sender, e);
        return Event_Accept;
    }
    return Super::DoEvent(sender, e);
}



/// <summary>
/// Moves the splitter.
/// </summary>
/// <param name="splitter">The splitter object.</param>
/// <param name="e">The event.</param>
/// <returns></returns>
void LongUI::UIBoxLayout::move_splitter(UIControl& splitter, const EventArg& e) noexcept {
    //const auto& ev = static_cast<const EventSplitterArg&>(e);
    //const auto attribute = ev.attribute;
    //const Point2F offset{ ev.offset_x, ev.offset_y };
    //assert(splitter.GetParent() == this);
    //// 不能是第一个
    //if (splitter.IsFirstChild()) return;
    //// 最后
    //auto& p = UIControlPrivate::Prev(splitter);
    //const auto index = this->GetOrient() == Orient_Horizontal ? 0 : 1;
    //const auto o = index[&offset.x];
    //// 计算大小
    //auto szp = p.GetSize(); index[&szp.width] += o;
    //if (index[&szp.width] <= index[&p.RefBox().minsize.width]) return;
    //// 不是最后一个?
    //if (!splitter.IsLastChild()) {
    //    // 修改后面的
    //    auto& n = UIControlPrivate::Next(splitter);
    //    auto szn = n.GetSize(); index[&szn.width] -= o;
    //    // 不够?
    //    if (index[&szn.width] <= index[&n.RefBox().minsize.width]) return;
    //    n.SetStyleSize(szn);
    //}
    //// 重置
    //p.SetStyleSize(szp);
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