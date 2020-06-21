#include <control/ui_grid.h>
#include <control/ui_column.h>
#include <control/ui_columns.h>
#include <control/ui_row.h>
#include <control/ui_rows.h>
#ifndef LUI_NO_UIGRID
#include <core/ui_ctrlmeta.h>
#include <debugger/ui_debug.h>
// Private
#include "../private/ui_private_control.h"

#include <algorithm>

// ui namespace
namespace LongUI {
    // UIRow类 元信息
    LUI_CONTROL_META_INFO(UIRow, "row");
    // UIRows类 元信息
    LUI_CONTROL_META_INFO(UIRows, "rows");
    // UIGrid类 元信息
    LUI_CONTROL_META_INFO(UIGrid, "grid");
    // UIColumn类 元信息
    LUI_CONTROL_META_INFO(UIColumn, "column");
    // UIColumns类 元信息
    LUI_CONTROL_META_INFO(UIColumns, "columns");
    // max count of gridview
    enum : uint32_t { MAX_COUNT_GRIDVIEW = 256 };
}


// ----------------------------------------------------------------------------
//                               UIGrid
// ----------------------------------------------------------------------------

/// <summary>
/// is col mode?
/// </summary>
/// <returns></returns>
inline bool LongUI::UIGrid::is_col_mode() const noexcept {
    const auto first = this->begin();
    return static_cast<const UIControl*>(first) == m_pRows;
}

/// <summary>
/// Finalizes an instance of the <see cref="UIGrid"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIGrid::~UIGrid() noexcept {
}


/// <summary>
/// Initializes a new instance of the <see cref="UIGrid" /> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UIGrid::UIGrid(const MetaControl& meta) noexcept : Super(meta) {

}

/// <summary>
/// initialize UIGrid
/// </summary>
/// <returns></returns>
void LongUI::UIGrid::initialize() noexcept {
    // 初始化
    if (m_pRows && m_pColumns) {
        if (static_cast<UIControl*>(this->begin()) == m_pRows) {
            m_pFirst = m_pRows; m_pSecond = m_pColumns;
        }
        else {
            m_pSecond = m_pRows; m_pFirst = m_pColumns;
        }
        // 直接隐藏第一个
        m_pFirst->SetVisible(false);
    }
    return Super::initialize();
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIGrid::DoEvent(UIControl* sender, 
    const EventArg& e) noexcept -> EventAccept {
    float buf[MAX_COUNT_GRIDVIEW];
    switch (e.nevent)
    {
        Size2F minsize;
    case NoticeEvent::Event_RefreshBoxMinSize:
        minsize = {};
        if (m_pSecond) {
            const auto rv = this->check_minsize(buf);
            float sum = 0;
            for (uint32_t i = 0; i != rv.first; ++i) sum += buf[i];
            if (this->is_col_mode()) minsize = { rv.second, sum };
            else minsize = { sum, rv.second };
        }
        this->set_contect_minsize(minsize);
        return Event_Accept;
    }
    return Super::DoEvent(sender, e);
}


// ui::impl
namespace LongUI { namespace impl {
    /// <summary>
    /// 计算行数据
    /// </summary>
    /// <param name="line"></param>
    /// <param name="buf"></param>
    /// <param name="real_len"></param>
    /// <returns></returns>
    template<typename T, typename U>
    const auto grid_row(U call, UIControl& line, float buf[], uint32_t real_len) noexcept {
        float max_height = 0;
        // 有效行
        if (uisafe_cast<T>(&line)) {
            uint32_t i = 0;
            for (auto& child : line) {
                // 可见即可
                if (child.IsVaildInLayout()) {
                    if (i == real_len) {
#ifndef NDEBUG
                        LUIDebug(Warning) << "children over col" << endl;
#endif // !NDEBUG
                        break;
                    }
                    const auto ms = call(child.GetMinSize());
                    buf[i] = std::max(buf[i], ms.width); ++i;
                    max_height = std::max(max_height, ms.height);
                }
            }
            UIControlPrivate::SetConMinsize(line, call({ 0, max_height }));
        }
        // 无效行
        else {
            const auto ms = call(line.GetMinSize());
            buf[0] = std::max(buf[0], ms.width);
            max_height = std::max(max_height, ms.height);
        }
        return max_height;
    };
}}


/// <summary>
/// check minsize
/// </summary>
/// <returns></returns>
auto LongUI::UIGrid::check_minsize(float buf[]) noexcept ->std::pair<uint32_t, float> {
    assert(m_pFirst && m_pSecond && "check before call");
    const uint32_t view_len = m_pFirst->GetChildrenCount();
    const uint32_t real_len = std::min(view_len, uint32_t(MAX_COUNT_GRIDVIEW));
    if (!real_len) return {};
    std::memset(buf, 0, real_len * sizeof(*buf));
    float length = 0;

    if (this->is_col_mode())
        for (auto& line : (*m_pSecond)) {
            const auto func = [](Size2F s) { std::swap(s.width, s.height); return s; };
            length += impl::grid_row<UIColumn>(func, line, buf, real_len);
        }
    else
        for (auto& line : (*m_pSecond)) {
            const auto func = [](Size2F s) { return s; };
            length += impl::grid_row<UIRow>(func, line, buf, real_len);
        }
    return { real_len, length };
}


/// <summary>
/// update this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIGrid::Update(UpdateReason reason) noexcept {
    // 排版
    if ((reason & Reason_BasicRelayout) && m_pSecond)
        this->relayout_this();
    return Super::Update(reason);
}


/// <summary>
/// add child for this
/// </summary>
/// <param name="child"></param>
/// <returns></returns>
void LongUI::UIGrid::relayout_this() noexcept {
    const auto minsize = m_oBox.minsize;
    const auto consize = m_oBox.GetContentSize();
    const auto remaining_w = consize.width - minsize.width;
    const auto remaining_h = consize.height - minsize.height;
    if (remaining_w < 0 || remaining_h < 0) return;
    assert(m_pFirst && m_pSecond && "check before call");
    const auto size = m_oBox.GetContentSize();
    this->resize_child(*m_pSecond, size);
    const auto pos = m_oBox.GetContentPos();
    m_pSecond->SetPos(pos);
    // 计算剩余数据
    float buf[MAX_COUNT_GRIDVIEW];
    const auto rv = this->check_minsize(buf);
    const auto length = rv.first;
    if (!length) return;
    // 权重
    const auto flex_1st = m_pFirst->SumChildrenFlex();
    const auto flex_2nd = m_pSecond->SumChildrenFlex();
    // 计算单位权重
    const auto col_mode = this->is_col_mode();
    const auto remaining_1st = col_mode ? remaining_h : remaining_w;
    const auto remaining_2nd = col_mode ? remaining_w : remaining_h;
    const Point2F matrix = col_mode ? Point2F{ 1, 0 } : Point2F{ 0, 1 };
    const auto& mode_meta = col_mode ? UIColumn::s_meta : UIRow::s_meta;
    const auto len_unit_1st = flex_1st > 0 ? remaining_1st / flex_1st : 0;
    const auto len_unit_2nd = flex_2nd > 0 ? remaining_2nd / flex_2nd : 0;
    Point2F move_pos = m_oBox.GetContentPos();
    // 计算pack位置
    if (len_unit_2nd == 0) {
        const auto fpack = static_cast<float>(m_pSecond->RefStyle().pack);
        const auto base = fpack * 0.5f * remaining_2nd;
        move_pos.x += base * matrix.x;
        move_pos.y += base * matrix.y;
    }
    // 遍历主体
    for (auto& line : (*m_pSecond)) {
        if (!line.IsVaildInLayout()) continue;
        const auto line_min = line.GetMinSize();
        const auto line_width = (line_min.width + line.RefStyle().flex * len_unit_2nd) * matrix.x;
        const auto line_height = (line_min.height + line.RefStyle().flex * len_unit_2nd) * matrix.y;
        line.SetPos(move_pos);
        move_pos.x += line_width;
        move_pos.y += line_height;
        Size2F line_size;
        line_size.width = consize.width * matrix.y + line_width;
        line_size.height = consize.height * matrix.x + line_height;
        // XXX: max clamp?
        m_pSecond->resize_child(line, line_size);
        // 有效行
        if (line.SafeCastTo(mode_meta)) {
            Point2F item_pos = {  };
            auto itr_1st = m_pFirst->begin();
            const auto end_1st = m_pFirst->end();
            uint32_t i = 0;
            for (auto& item : line) {
                if (!item.IsVaildInLayout()) continue;
                // 越界
                if (i == length) break;
                // 从1st获取flex
                float item_flex = 0.f;
                if (itr_1st != end_1st) {
                    item_flex = itr_1st->RefStyle().flex;
                    ++itr_1st;
                }
                // 计算宽高度
                const auto item_wh = buf[i] + len_unit_1st * item_flex; ++i;
                item.SetPos(item_pos);
                Size2F item_size = { 
                    line_width + item_wh * matrix.y, 
                    line_height + item_wh * matrix.x 
                };
                item_pos.x += item_size.width * matrix.y;
                item_pos.y += item_size.height * matrix.x;
                // 但是不能超过本身限制
                item_size.width = std::min(item.GetMaxSize().width, item_size.width);
                item_size.height = std::min(item.GetMaxSize().height, item_size.height);
                line.resize_child(item, item_size);
            }
        }
    }
}

/// <summary>
/// add child for this
/// </summary>
/// <param name="child"></param>
/// <returns></returns>
void LongUI::UIGrid::add_child(UIControl& child) noexcept {
    if (const auto rows = uisafe_cast<UIRows>(&child)) {
        m_pRows = rows;
    }
    else if (const auto columns = uisafe_cast<UIColumns>(&child)) {
        m_pColumns = columns;
    }
#ifndef NDEBUG
    else {
        LUIDebug(Error) << "<grid> accept <columns> / <rows> only" << endl;
    }
#endif // !NDEBUG
    return Super::add_child(child);
}

#endif


// ----------------------------------------------------------------------------
//                               UIRow
// ----------------------------------------------------------------------------


/// <summary>
/// Finalizes an instance of the <see cref="UIRow"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIRow::~UIRow() noexcept {
}


/// <summary>
/// Initializes a new instance of the <see cref="UIRow" /> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UIRow::UIRow(const MetaControl& meta) noexcept : Super(meta) {

}


/// <summary>
/// update this instance.
/// </summary>
/// <returns></returns>
//void LongUI::UIRow::Update(UpdateReason reason) noexcept {
//    return Super::Update(reason);
//}


/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
//auto LongUI::UIRow::DoEvent(UIControl* sender,
//    const EventArg& e) noexcept -> EventAccept {
//    switch (e.nevent)
//    {
//    case NoticeEvent::Event_RefreshBoxMinSize:
//        this->set_contect_minsize(impl::sum_children_minsize_h(*this));
//        return Event_Accept;
//    }
//    return Super::DoEvent(sender, e);
//}

// ----------------------------------------------------------------------------
//                               UIRows
// ----------------------------------------------------------------------------




/// <summary>
/// Finalizes an instance of the <see cref="UIRows"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIRows::~UIRows() noexcept {
}


/// <summary>
/// Initializes a new instance of the <see cref="UIRows" /> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UIRows::UIRows(const MetaControl& meta) noexcept : Super(meta) {
}


/// <summary>
/// update this instance.
/// </summary>
/// <returns></returns>
//void LongUI::UIRows::Update(UpdateReason reason) noexcept {
//    return Super::Update(reason);
//}


/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
//auto LongUI::UIRows::DoEvent(UIControl* sender,
//    const EventArg& e) noexcept -> EventAccept {
//    switch (e.nevent)
//    {
//    case NoticeEvent::Event_RefreshBoxMinSize:
//        this->set_contect_minsize(impl::sum_children_minsize_v(*this));
//        return Event_Accept;
//    }
//    return Super::DoEvent(sender, e);
//}

// ----------------------------------------------------------------------------
//                              UIColumn
// ----------------------------------------------------------------------------


/// <summary>
/// Finalizes an instance of the <see cref="UIColumn"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIColumn::~UIColumn() noexcept {
}


/// <summary>
/// Initializes a new instance of the <see cref="UIColumn" /> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UIColumn::UIColumn(const MetaControl& meta) noexcept : Super(meta) {

}


/// <summary>
/// update this instance.
/// </summary>
/// <returns></returns>
//void LongUI::UIColumn::Update(UpdateReason reason) noexcept {
//    return Super::Update(reason);
//}


/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
//auto LongUI::UIColumn::DoEvent(UIControl* sender,
//    const EventArg& e) noexcept -> EventAccept {
//    switch (e.nevent)
//    {
//    case NoticeEvent::Event_RefreshBoxMinSize:
//        this->set_contect_minsize(impl::sum_children_minsize_v(*this));
//        return Event_Accept;
//    }
//    return Super::DoEvent(sender, e);
//}

// ----------------------------------------------------------------------------
//                              UIColumns
// ----------------------------------------------------------------------------



/// <summary>
/// Finalizes an instance of the <see cref="UIColumns"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIColumns::~UIColumns() noexcept {
}


/// <summary>
/// Initializes a new instance of the <see cref="UIColumns" /> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UIColumns::UIColumns(const MetaControl& meta) noexcept : Super(meta) {
    //m_state.orient = Orient_Vertical;
}


/// <summary>
/// update this instance.
/// </summary>
/// <returns></returns>
//void LongUI::UIColumns::Update(UpdateReason reason) noexcept {
//    return Super::Update(reason);
//}


/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
//auto LongUI::UIColumns::DoEvent(UIControl* sender,
//    const EventArg& e) noexcept -> EventAccept {
//    switch (e.nevent)
//    {
//    case NoticeEvent::Event_RefreshBoxMinSize:
//        this->set_contect_minsize(impl::sum_children_minsize_h(*this));
//        return Event_Accept;
//    }
//    return Super::DoEvent(sender, e);
//}
