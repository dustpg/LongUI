#include "ed_txtdoc.h"
#include "ed_txtplat.h"
#include "ed_txtcell.h"
#include <cstring>
#include <new>


// namespace Riched::detail
namespace RichED { namespace detail {
    // is_2nd_surrogate
    static inline bool is_2nd_surrogate(uint16_t ch) noexcept { return ((ch) & 0xFC00) == 0xDC00; }
    // is_1st_surrogate
    static inline bool is_1st_surrogate(uint16_t ch) noexcept { return ((ch) & 0xFC00) == 0xD800; }
    // clear all
    inline auto&clear_all(CEDTextDocument& doc, CEDTextCell* cell) noexcept {
        std::memset(cell, 0, sizeof(*cell));
        return doc;
    }
    /// <summary>
    /// Estimates the specified cell.
    /// </summary>
    /// <param name="cell">The cell.</param>
    /// <param name="metrice">The metrice.</param>
    /// <returns></returns>
    void estimate(CEDTextCell& cell) noexcept {
        // 估计高度 = 字体大小
        // 估计宽度 = 字体大小 * 字符个数
        //const auto len = cell.RefString().length;
        //const auto fontsize = cell.RefRichED().size;
        //cell.metrics.ar_height = 0;
        //cell.metrics.dr_height = fontsize;
        //cell.metrics.width = times(fontsize, len);
    }
    /// <summary>
    /// Inserts the specified position.
    /// </summary>
    /// <param name="pos">The position.</param>
    /// <param name="str">The string.</param>
    /// <param name="len">The length.</param>
    /// <returns></returns>
    void insert(FixedStringA& obj, uint32_t pos, const char16_t* str, uint32_t len) noexcept {
        // TODO: utf-16 双字断言
        assert(obj.capacity == TEXT_CELL_STR_MAXLEN);
        assert(len <= uint32_t(obj.capacity + 1) && "out of range");
        assert(obj.length + len <= TEXT_CELL_STR_MAXLEN + 1 && "out of range");
        assert(pos <= obj.length);
        const size_t moved = (obj.length - pos) * sizeof(obj.data[0]);
        std::memmove(obj.data + pos + len, obj.data + pos, moved);
        std::memcpy(obj.data + pos, str, len * sizeof(obj.data[0]));
        obj.length += len;
    }
    /// <summary>
    /// Erases the specified object.
    /// </summary>
    /// <param name="obj">The object.</param>
    /// <param name="pos">The position.</param>
    /// <param name="len">The length.</param>
    /// <returns></returns>
    void erase(FixedStringA& obj, uint32_t pos, uint32_t len) noexcept {
        // TODO: utf-16 双字断言
        assert(obj.capacity == TEXT_CELL_STR_MAXLEN);
        assert(len <= obj.length && "out of range");
        assert(pos + len <= obj.length && "out of range");
        assert(obj.length <= TEXT_CELL_STR_MAXLEN + 1 && "out of length");
        const size_t moved = (obj.length - pos) * sizeof(obj.data[0]);
        std::memmove(obj.data + pos, obj.data + pos + len, moved);
        obj.length -= len;
    }
}}


// riched namespace
namespace RichED {
    // RichData ==
    inline bool operator==(const RichData& a, const RichData& b) noexcept {
        return !std::memcmp(&a, &b, sizeof(a));
    }
}


/// <summary>
/// Releases unmanaged and - optionally - managed resources.
/// </summary>
/// <returns></returns>
void RichED::CEDTextCell::Dispose() noexcept {
    //delete this;
    this->~CEDTextCell();
    std::free(this);
}

/// <summary>
/// Sleeps this instance.
/// </summary>
/// <returns></returns>
void RichED::CEDTextCell::Sleep() noexcept {
    this->doc.platform.DeleteContext(*this);
}

/// <summary>
/// Initializes a new instance of the <see cref="CEDTextCell" /> class.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="red">The red.</param>
RichED::CEDTextCell::CEDTextCell(CEDTextDocument& doc, const RichData& red) noexcept
    : doc(detail::clear_all(doc, this)) {
    m_riched = red;
    this->metrics.ar_height = this->metrics.bounding.bottom = red.size;
}


/// <summary>
/// Finalizes an instance of the <see cref="CEDTextCell"/> class.
/// </summary>
/// <returns></returns>
RichED::CEDTextCell::~CEDTextCell() noexcept {
    //RichED::RemoveFromListOnly(*this);
    this->Sleep();
}


/// <summary>
/// Sets the rich ed.
/// </summary>
/// <param name="data">The data.</param>
/// <returns></returns>
void RichED::CEDTextCell::SetRichED(const RichData& data) noexcept {
    m_riched = data;
    detail::estimate(*this);
    m_meta.dirty = true;
}

/// <summary>
/// Splits the specified position.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
auto RichED::CEDTextCell::Split(uint32_t pos) noexcept -> CEDTextCell * {
    assert(pos <= m_string.length);
    if (pos >= m_string.length) return static_cast<CEDTextCell*>(next);
    return this->SplitEx(pos);
}


/// <summary>
/// Moves the eol.
/// </summary>
/// <param name="cell">The cell.</param>
/// <returns></returns>
void RichED::CEDTextCell::MoveEOL(CEDTextCell & cell) noexcept {
    cell.m_meta.eol = m_meta.eol;
    m_meta.eol = false;
}

/// <summary>
/// Splits the specified position.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
auto RichED::CEDTextCell::SplitEx(uint32_t pos) noexcept -> CEDTextCell* {
    if (pos == 0) return this;
    if (const auto cell = RichED::CreateNormalCell(this->doc, m_riched)) {
        // 复制信息
        cell->m_riched = m_riched;
        cell->m_meta.eol = m_meta.eol;
        m_meta.eol = false;
        // 复制文本
        if (pos < m_string.length) {
            assert(m_meta.metatype < Type_UnderRuby && "cannot split this");
            const auto len_after = m_string.length - pos;
            cell->InsertText(0, {
                m_string.data + pos,
                m_string.data + m_string.length
                });
            this->RemoveText({ pos, len_after });
        }
        RichED::InsertAfterFirst(*this, *cell);
        return cell;
    }
    return nullptr;
}

/// <summary>
/// Merges the with next.
/// </summary>
/// <returns></returns>
bool RichED::CEDTextCell::MergeWithNext() noexcept {
    assert(this->next && this->prev && "bad action");
    // 不能融合EOL
    if (m_meta.eol) return false;
    assert(this->next->next && "bad action");
    const auto next_cell = static_cast<CEDTextCell*>(this->next);
    // 各种检查
    if (next_cell->m_string.length <= TEXT_MERGE_LEN) {
        if (next_cell->m_meta.metatype | m_meta.metatype) return false;
        const auto all_len = m_string.length + next_cell->m_string.length;
        assert(m_string.capacity == TEXT_CELL_STR_MAXLEN);
        if (all_len > TEXT_CELL_STR_MAXLEN) return false;
        if (!(m_riched == next_cell->m_riched)) return false;
        U16View view;
        view.first = next_cell->m_string.data;
        view.second = next_cell->m_string.data + next_cell->m_string.length;
        this->InsertText(m_string.length, view);
        // 传递EOL
        m_meta.eol = next_cell->m_meta.eol;
        // 删除节点
        next_cell->DisposeEx();
#ifndef NDEBUG
        //this->doc.platform.DebugOutput("Merga!");
#endif
        return true;
    }
    return false;
}

/// <summary>
/// Removes the text.
/// </summary>
/// <param name="range">The range.</param>
/// <returns></returns>
void RichED::CEDTextCell::RemoveText(Range range) noexcept {
    // 移除空字符
    if (!range.len) return;
    // 直接删除
    detail::erase(m_string, range.pos, range.len);
    // 进行估计
    detail::estimate(*this);
    // 标记为脏
    m_meta.dirty = true;
}

/// <summary>
/// Removes the text ex.
/// </summary>
/// <param name="range">The range.</param>
/// <returns></returns>
void RichED::CEDTextCell::RemoveTextEx(Range range) noexcept {
    if (range.pos == 0 && range.len == m_string.length)
        this->DisposeEx();
    else
        this->RemoveText(range);
}

/// <summary>
/// Inserts the text.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="view">The view.</param>
/// <returns></returns>
void RichED::CEDTextCell::InsertText(uint32_t pos, U16View view) noexcept {
    // 插入空字符
    if (view.second == view.first) return;
    // 必须是NormalCell
    assert(m_meta.metatype < Type_UnknownInline);
    // 计算插入长度
    const uint32_t len = static_cast<uint32_t>(view.second - view.first);
    detail::insert(m_string, pos, view.first, len);
    // 进行估计
    detail::estimate(*this);
    // 标记为脏
    m_meta.dirty = true;
}

/// <summary>
/// Gets the length of the logic.
/// </summary>
/// <returns></returns>
//auto RichED::CEDTextCell::GetLogicLength() const noexcept -> uint32_t {
//    const uint32_t mask = uint32_t(m_meta.eol_mask);
//    const uint32_t linefeed = this->doc.linefeed.length;
//    return m_string.length + linefeed & mask;
//}

// ----------------------------------------------------------------------------
//                            Normal Text Cell
// ----------------------------------------------------------------------------

namespace RichED {
    // shrinked cell
    struct CEDTextCellPublic : CEDTextCell {
        // ctor
        CEDTextCellPublic(CEDTextDocument&d, const RichData& red, uint32_t capacity) noexcept : CEDTextCell(d, red) {
            m_string.capacity = TEXT_CELL_STR_MAXLEN;
        }
    };
    PCN_NOINLINE
    /// <summary>
    /// Creates the cell.
    /// </summary>
    /// <param name="doc">The document.</param>
    /// <param name="red">The red.</param>
    /// <param name="capacity">The capacity.</param>
    /// <param name="extra">The extra.</param>
    /// <returns></returns>
    auto CreatePublicCell(CEDTextDocument& doc, const RichData& red, 
        uint32_t exlen, uint32_t capacity) noexcept -> CEDTextCell* {
        auto& plat = doc.platform;
        for (uint32_t i = 0; ; ++i) {
            const size_t len = sizeof(CEDTextCellPublic) + exlen;
            if (const auto ptr = std::malloc(len)) {
                assert((uintptr_t(ptr) & (alignof(CEDTextCellPublic) - 1)) == 0);
                return new(ptr) CEDTextCellPublic{ doc, red, capacity };
            }
            if (plat.OnOOM(i, len) == OOM_Ignore) return nullptr;
        }
    }
    /// <summary>
    /// Creates the normal cell.
    /// </summary>
    /// <param name="doc">The document.</param>
    /// <returns></returns>
    auto CreateNormalCell(CEDTextDocument& doc, const RichData& red) -> CEDTextCell* {
        return CreatePublicCell(
            doc, red, sizeof(FixedStringB), TEXT_CELL_STR_MAXLEN
        );
    }
    /// <summary>
    /// Creates the shrinked cell.
    /// </summary>
    /// <param name="doc">The document.</param>
    /// <param name="red">The red.</param>
    /// <returns></returns>
    auto RichED::CreateShrinkedCell(CEDTextDocument& doc, const RichData& red) -> CEDTextCell * {
        return CreatePublicCell(doc, red, sizeof(FixedStringB), 1);
    }
}
