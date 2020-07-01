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
    // MAX subline count
    enum : uint32_t { MAX_SUBLINE_COUNT = 256 };
    // l x f
    struct GridLF { float limited, fitting; };
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
#ifndef NDEBUG
        m_pFirst->SetVisible(m_state.dbg_output);
#endif
    }
    return Super::initialize();
}

// ui::impl
namespace LongUI { namespace impl {
    /// <summary>
    /// each line
    /// </summary>
    template<typename T, typename U, typename V> 
    auto each_line(T& line, GridLF buf[], U ucall, V vcall) noexcept {
        uint32_t index = 0;
        float max_fitting = 0;
        for (auto& child : line) {
            if (!child.IsVaildInLayout()) continue;
            auto& value = buf[index];
            const auto lsize = child.GetBoxLimitedSize();
            value.limited = std::max(value.limited, ucall(lsize));
            const auto fsize = child.GetBoxFittingSize();
            value.fitting = std::max(value.fitting, ucall(fsize));
            max_fitting = std::max(max_fitting, vcall(fsize));
            ++index; if (index == MAX_SUBLINE_COUNT) break;
        }
        line.UpdateValue(max_fitting, max_fitting);
        return max_fitting;
    };
    /// <summary>
    /// each line
    /// </summary>
    template<typename T> 
    auto update_line(UIControl& first, GridLF buf[]) noexcept {
        float limited = 0, fitting = 0;
        uint32_t index = 0;
        for (auto& child : first)
            if (const auto column = uisafe_cast<T>(&child)) {
                const auto& value = buf[index];
                column->UpdateValue(value.limited, value.fitting);
                limited += value.limited;
                fitting += value.fitting;
                ++index;
            }
        GridLF rv;
        rv.limited = limited;
        rv.fitting = fitting;
        return rv;
    };
}}


/// <summary>
/// check minsize
/// </summary>
/// <returns></returns>
void LongUI::UIGrid::refresh_fitting() noexcept {
    assert(m_pFirst && m_pSecond && "check before call");
    // 初始化建议值
    const auto real_count = m_pFirst->GetChildrenCount();
#ifndef NDEBUG
    if (real_count > MAX_SUBLINE_COUNT) {
        LUIDebug(Error) << "LongUI donot support subline > MAX_SUBLINE_COUNT" << endl;
    }
#endif
    const auto current = std::min(uint32_t(MAX_SUBLINE_COUNT), real_count);
    GridLF buf[MAX_SUBLINE_COUNT];
    std::memset(buf, 0, sizeof(buf[0]) * current);
    const auto get_width = [](Size2F s) noexcept {return s.width; };
    const auto get_height = [](Size2F s) noexcept {return s.height; };
    // 初始化
    if (this->is_col_mode()) {
        float sum = 0;
        // 计算建议值
        for (auto& line : (*m_pSecond)) {
            if (const auto row = uisafe_cast<UIColumn>(&line)) 
                sum += impl::each_line(*row, buf, get_height, get_width);
        }
        // 利用BOX设置
        const auto rv = impl::update_line<UIRow>(*m_pFirst, buf);
        // 更新值
        this->set_limited_width_lp(sum );
        this->set_limited_height_lp(rv.limited);
        this->update_fitting_size({ sum, rv.fitting });
    }
    else {
        float sum = 0;
        // 计算建议值
        for (auto& line : (*m_pSecond)) {
            if (const auto row = uisafe_cast<UIRow>(&line)) 
                sum += impl::each_line(*row, buf, get_width, get_height);
        }
        // 利用BOX设置
        const auto rv = impl::update_line<UIColumn>(*m_pFirst, buf);
        // 更新值
        this->set_limited_width_lp(rv.limited);
        this->set_limited_height_lp(sum);
        this->update_fitting_size({ rv.fitting , sum });
    }
}


/// <summary>
/// update this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIGrid::Update(UpdateReason reason) noexcept {
    // 更新尺寸
    if (reason & Reason_BasicUpdateFitting && m_pSecond)
        this->refresh_fitting();
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
    const auto consize = m_oBox.GetContentSize();
    assert(m_pFirst && m_pSecond && "check before call");
    const auto size = m_oBox.GetContentSize();
    this->resize_child(*m_pFirst, size);
    this->resize_child(*m_pSecond, size);
    const auto pos = m_oBox.GetContentPos();
    m_pFirst->SetPos(pos);
    m_pSecond->SetPos(pos);
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
void LongUI::UIRow::Update(UpdateReason reason) noexcept {
    // 重新布局
    if (reason & Reason_BasicUpdateFitting) {
        if (m_pParent) if (const auto grid = uisafe_cast<UIGrid>(m_pParent->GetParent())) {
            assert(grid->GetChildrenCount());
            UIControl* first = grid->begin();
            if (m_pParent != first && uisafe_cast<UIColumns>(first)) {
                this->MatchLayout(UIColumn::s_meta, *first, 1);
            }
        }
    }
    // 超类处理
    return Super::Update(reason);
}


/// <summary>
/// update value
/// </summary>
/// <param name="value"></param>
/// <returns></returns>
void LongUI::UIRow::UpdateValue(float limited, float fitting) noexcept {
    this->set_limited_height_lp(limited);
    this->update_fitting_size({ 0, fitting });
}



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
    m_state.orient = Orient_Vertical;
}


/// <summary>
/// update this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIRows::Update(UpdateReason reason) noexcept {
    // 交给超类处理
    Super::Update(reason);
    // 父节点状态
    if (reason & Reason_BasicRelayout)
        if (const auto grid = uisafe_cast<UIGrid>(m_pParent)) {
            const auto first = grid->GetFirst();
            const auto second = grid->GetSecond();
            if (first == this) for (auto& child : (*second))
                child.NeedUpdate(Reason_ChildLayoutChanged);
        }
}


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
void LongUI::UIColumn::Update(UpdateReason reason) noexcept {
    // 重新布局
    if (reason & Reason_BasicUpdateFitting) {
        if (m_pParent) if (const auto grid = uisafe_cast<UIGrid>(m_pParent->GetParent())) {
            assert(grid->GetChildrenCount());
            UIControl* first = grid->begin();
            if (m_pParent != first && uisafe_cast<UIRows>(first)) {
                this->MatchLayout(UIRow::s_meta, *first, 0);
            }
        }
    }
    // 超类处理
    return Super::Update(reason);
}


/// <summary>
/// update value
/// </summary>
void LongUI::UIColumn::UpdateValue(float limited, float fitting) noexcept {
    this->set_limited_width_lp(limited);
    this->update_fitting_size({ fitting, 0 });
}

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
void LongUI::UIColumns::Update(UpdateReason reason) noexcept {
    // 交给超类处理
    Super::Update(reason);
    // 父节点状态
    if (reason & Reason_BasicRelayout) 
        if (const auto grid = uisafe_cast<UIGrid>(m_pParent)) {
            const auto first = grid->GetFirst();
            const auto second = grid->GetSecond();
            if (first == this) for (auto& child : (*second)) 
                child.NeedUpdate(Reason_ChildLayoutChanged);
        }
}
