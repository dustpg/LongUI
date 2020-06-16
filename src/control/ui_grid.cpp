#include <control/ui_grid.h>
#include <control/ui_column.h>
#include <control/ui_columns.h>
#include <control/ui_row.h>
#include <control/ui_rows.h>
#ifndef LUI_NO_UIGRID
#include <core/ui_ctrlmeta.h>
#include <debugger/ui_debug.h>

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
    case NoticeEvent::Event_RefreshBoxMinSize:
        if (m_pSecond) this->refresh_minsize(buf);
        return Event_Accept;
    case NoticeEvent::Event_Initialize:
        // 初始化
        if (m_pRows && m_pColumns) {
            if (static_cast<UIControl*>(this->begin()) == m_pRows) {
                m_pFirst = m_pRows; m_pSecond = m_pColumns;
            }
            else {
                m_pSecond = m_pRows; m_pFirst = m_pColumns;
            }
        }
    }
    return Super::DoEvent(sender, e);
}

/// <summary>
/// refresh minsize
/// </summary>
/// <returns></returns>
auto LongUI::UIGrid::refresh_minsize(float buf[]) noexcept -> uint32_t {
    assert(m_pFirst && m_pSecond && "check before call");
    const uint32_t view_len = m_pFirst->GetChildrenCount();
    const uint32_t real_len = std::min(view_len, uint32_t(0));
    std::memset(buf, 0, real_len * sizeof(*buf));
    // 检查模式
    const auto is_rows_mode = [this]() noexcept { 
        return static_cast<UIControl*>(this->begin()) == m_pColumns;
    };
    // 行模式
    if (is_rows_mode()) {

    }
    // 列模式
    else {

    }
    return real_len;
}

/// <summary>
/// update this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIGrid::Update(UpdateReason reason) noexcept {
    // 检查模式
    //const auto is_rows_mode = [this]() noexcept { 
    //    return static_cast<UIControl*>(this->begin()) == m_pColumns;
    //};
    // 排版
    if (reason & Reason_BasicRelayout)
        this->relayout_this();
    return Super::Update(reason);
}


/// <summary>
/// add child for this
/// </summary>
/// <param name="child"></param>
/// <returns></returns>
void LongUI::UIGrid::relayout_this() noexcept {

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
