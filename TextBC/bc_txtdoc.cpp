#include "bc_txtdoc.h"
#include <cassert>
#include <algorithm>
#include <cstring>
#ifndef NDEBUG
#include <cstdio>
#endif

/// <summary>
/// Initializes a new instance of the <see cref="CBCTextDocument" /> class.
/// </summary>
/// <param name="plat">The plat.</param>
/// <param name="args">The arguments.</param>
TextBC::CBCTextDocument::CBCTextDocument(IBCTextPlatform& plat, InitArgs args) noexcept
    : platform(plat), m_cMaxLen(args.max_length),
    m_flag(args.flag), m_chPassword(args.password) {
    assert(m_cMaxLen > 2 && "too small");
#ifndef NDEBUG
    // 密码不能和多行共存
    if ((m_flag & Flag_UsePassword) && (m_flag & Flag_MultiLine)) {
        assert(!"cannot password & multi-line");
    }
    // 密码不能和富文本共存
    if ((m_flag & Flag_UsePassword) && (m_flag & Flag_RichText)) {
        assert(!"cannot password & rich-text");
    }
#endif
    // CRLF
    m_bCRLF = !(m_flag & Flag_NoneCRLR);
    // 点中选择区
    m_bClickInSelection = false;
    // 文本修改
    m_textChanged = false;
    // 选则修改
    m_selectionChanged = false;
#ifdef TBC_UNDOREDO
    // 记录UNDO-REDO
    m_recordUndoRedo = false;
#endif

    // 首行空数据
    m_lines.Resize(INIT_RESERVE_LINE);
    m_lines.Resize(1);
    m_lines[0] = {};
    // 选择区
    m_selections.Resize(INIT_RESERVE_LINE);
    m_selections.Clear();
    // TODO: 休眠
    // TODO: 行高
    // TODO: 不依赖行数据计算内容区域大小
    // TODO: 视口渲染
    // TODO: 脏矩形渲染

    // 初始化一个节点?
#if 0
    // 载入测试数据
    {
        char16_t buf[6];
        memcpy(buf, L"A機x\r\n", sizeof buf);
        // 测试数据
        
        // 首行数据
        const auto a = new CBCTextCell{ *this };
        a->prev = &m_head;
        a->next = m_head.next;
        m_head.next = a;

        a->InsertText(0, { buf , buf + 3 });
        a->MarkAsBOL();
        a->MarkAsEOL();

        memcpy(buf, L"F機x\r\n", sizeof buf);
        auto b = a->NewAfterThis();
        b->InsertText(0, { buf , buf + 3 });
        b->MarkAsBOL();
        b->MarkAsEOL();

        memcpy(buf, L"KLxNO", sizeof buf);
        auto c = b->NewAfterThis();
        c->InsertText(0, { buf , buf + 5 });
        c->MarkAsBOL();

        memcpy(buf, L"P😄ST", sizeof buf);
        auto d = c->NewAfterThis();
        d->InsertText(0, { buf , buf + 5 });

        m_cTotalCount = m_cTotalLen = 20;
        m_lines.Resize(4);
        if (false) {
            //this->sync_cache_to_length(6);
            //this->remove_text({ 0, 0 });
            //this->remove_text({ 5, 6 });
            this->remove_text({ 1, 10 });
            //this->remove_text({ 3, 17 });
            auto text = (char16_t*)std::malloc(1024 * sizeof(char16_t));
            text[0] = 0;
            {
                int i = 0;
                for (i = 0; i != 50; ++i) {
                    text[i] = 'a' + i % 26;
                }
                text[i] = 0;
                text[21] = '\r';
                text[22] = '\n';
                const auto len = std::char_traits<char16_t>::length(text);
                this->insert_text(2, { text , text + len });
            }
            std::free(text);
            //reinterpret_cast<TextCell&>(head).insert_new();
        }
        this->sync_cache_to_length(m_cTotalLen);
        this->recalculate_content_size();
        this->text_changed();
    }
#else
    // 首行数据
    const auto a = new(std::nothrow) CBCTextCell{ *this };
    a->prev = &m_head;
    a->next = m_head.next;
    a->MarkAsBOL();
    //a->MarkAsEOL();
    m_head.next = a;

    this->sync_cache_to_length(0);
    this->recalculate_content_size();
    //this->text_changed();
    m_lines.Resize(2);
#endif
    m_uCaretPos = uint32_t(-1);
}

/// <summary>
/// Finalizes an instance of the <see cref="CBCTextDocument"/> class.
/// </summary>
TextBC::CBCTextDocument::~CBCTextDocument() noexcept {
#ifdef TBC_UNDOREDO
    // 撤销栈
    this->free_undo_stack();
#endif
    // 释放缓存表
    this->free_mem_list();
    // 一次删除各个节点
    auto node = m_head.next;
    while (node != &m_tail) {
        const auto tmp = static_cast<CBCTextCell*>(node);
        node = tmp->next;
        tmp->DeleteNode();
    }
}

/// <summary>
/// Renders the specified context.
/// </summary>
/// <param name="context">The context.</param>
/// <param name="rects">The rects.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::Render(void* context, DirtyRects* rects) noexcept {
    // TODO: 增量渲染

    // 检查BUFFER有效性
    if (!m_lines.IsOK()) return;
    // 渲染偏移
    const auto offset = Point2F{ -m_ptViewport.x, -m_ptViewport.y };

    // 暂时同步所有数据, 增量渲染后就可以不用了
    //this->sync_cache_to_length(m_cTotalLen);

    // 渲染选择区
    this->platform.DrawSelection(
        context,
        offset,
        m_selections.GetData(),
        m_selections.GetSize()
    );
    // 渲染插入符
    if (m_bDrawCaret) this->platform.DrawCaret(context, offset, m_rcCaret);

    // 从开始位置进行渲染
    auto this_line = m_lines.begin() + m_beginLineVisible;
    // 同步一行
    if (!m_cValidLine) this->sync_cache_to_length(0);
    // 获取首节点
    auto node = this_line->first;
    // 临时偏移
    float dx = 0, dy = static_cast<float>(this_line->offset);
#ifndef NDEBUG
    int index_cell_dbg = 0;
#endif
    // 遍历所有节点
    while (node != &m_tail) {
        // 尝试创建
        this->relayout_cell(*node);
        const auto ct = node->GetContent();
        // TODO: 基线对齐
        const auto baseoffset = this_line->max_height1 - node->GetBaseLine();

        // 计算显示位置
        const auto x = dx + offset.x;
        const auto y = offset.y + dy + baseoffset;
        const auto h = this_line->max_height1 + this_line->max_height2;
#ifndef NDEBUG
        // 单元块背景调试
        const auto w = node->GetSize().width;
        this->platform.DrawCell(context, { x, y, w, h }, index_cell_dbg);
        ++index_cell_dbg;
#endif
        this->platform.DrawContent(*ct, context,{ x, y });
        // 行末尾
        if (node->IsEOL()) {
            // 计算换行数据
            dy += this_line->max_height1 + this_line->max_height2;
            dx = 0;
            // 超过显示范围
            // XXX: 行间距?
            if (y + h > m_szViewport.height) break;
            // 下一行数
            const uint32_t line_no = static_cast<uint32_t>(this_line - m_lines.begin());
            ++this_line;
            // 下一行数超过上限了?
            if (m_cValidLine == line_no) {
                // 不可能是超过最后一行
                assert(m_cValidLine != m_lines.GetSize() - 1);
                const auto next = static_cast<CBCTextCell*>(node->next);
                this->add_line(this_line[0], this_line[-1], next);
                ++m_cValidLine;
            }
            else {
                assert(m_cValidLine > line_no && "cannot be less");
            }
        }
        // 行中间
        else {
            dx += node->GetSize().width;
        }
        node = static_cast<CBCTextCell*>(node->next);
    }

}

/// <summary>
/// Clears the specified lastline.
/// </summary>
/// <param name="lastline">The lastline.</param>
/// <returns></returns>
void TextBC::TextLineData::Clear(const TextLineData& lastline) noexcept {
    this->offset = lastline.offset 
        + lastline.max_height1
        + lastline.max_height2
        ;
    this->char_count = lastline.char_count;
    this->string_len = lastline.string_len;
    this->first = this->last = nullptr;
    this->max_height1 = 0.f;
    this->max_height2 = 0.f;
}

PCN_NOINLINE
/// <summary>
/// Calculates the width.
/// </summary>
/// <param name="start">The start.</param>
/// <param name="end">The end.</param>
/// <returns></returns>
auto TextBC::TextLineData::CalculateWidth(
    const CBCTextCell* first,
    const CBCTextCell* last) noexcept -> float {
    // 将宽度加起来
    float width = 0.f;
    auto node = first;
    while (true) {
        width += node->GetSize().width;
        if (node == last) break;
        node = static_cast<decltype(node)>(node->next);
    }
    return width;
}


PCN_NOINLINE
/// <summary>
/// Calculates the height.
/// </summary>
/// <param name="first">The first.</param>
/// <param name="last">The last.</param>
/// <returns></returns>
auto TextBC::TextLineData::CalculateHeight(
    const CBCTextCell* first, const CBCTextCell* last) noexcept -> float {
    // 将高度取大
    float height = 0.f;
    auto node = first;
    while (true) {
        height = std::max(height, node->GetSize().height);
        if (node == last) break;
        node = static_cast<decltype(node)>(node->next);
    }
    return height;
}

/// <summary>
/// Operator+=s the specified cell.
/// </summary>
/// <param name="cell">The cell.</param>
/// <returns></returns>
void TextBC::TextLineData::operator+=(CBCTextCell& cell) noexcept {
    assert(!cell.IsDirty() && "relayout first");
    this->char_count += cell.GetCharCount();
    this->string_len += cell.GetStringLen();
    // 更新高度
    const float height1 = cell.GetBaseLine();
    const float height2 = cell.GetSize().height - cell.GetBaseLine();
    this->max_height1 = std::max(this->max_height1, height1);
    this->max_height2 = std::max(this->max_height2, height2);

    // 行开头
    if (cell.IsBOL()) this->first = &cell;
    // 行结尾或者文本结尾
    if (cell.IsEOL() || cell.IsLastCell()) this->last = &cell;
}

/// <summary>
/// Caches the length of the valid.
/// </summary>
/// <returns></returns>
inline auto TextBC::CBCTextDocument::cache_valid_length() const noexcept -> uint32_t {
    return m_lines[m_cValidLine].string_len;
}

/// <summary>
/// Caches the valid offset.
/// </summary>
/// <returns></returns>
inline auto TextBC::CBCTextDocument::cache_valid_offset() const noexcept -> double {
    return m_lines[m_cValidLine].offset;
}



PCN_NOINLINE
/// <summary>
/// Synchronizes the length of the cache to.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::sync_cache_to_length(uint32_t pos) noexcept {
    // 检查BUFFER有效性
    if (!m_lines.IsOK()) return;
    // 检查必要性
    const auto cvlen = this->cache_valid_length();
    if (pos < cvlen) return;
    if (pos == cvlen && pos == m_cTotalLen && m_cValidLine == m_lines.GetSize() - 1) 
        return;
    // 回调函数
    struct func_t {
        static bool func(void* this_ptr, uint64_t len) noexcept {
            const auto this_ = static_cast<CBCTextDocument*>(this_ptr);
            auto& this_line = this_->m_lines[this_->m_cValidLine];
            return this_line.char_count > uint32_t(len);
        }
        static bool maxp(void* , uint64_t ) noexcept { return false; }
    };
    // 范围内
    if (pos < m_cTotalLen) {
        this->sync_cache_until({ func_t::func }, pos);
    }
    // 全满
    else {
        this->sync_cache_until({ func_t::maxp }, pos);
    }
}

PCN_NOINLINE
/// <summary>
/// Synchronizes the cache to offset.
/// </summary>
/// <param name="offset">The offset.</param>
void TextBC::CBCTextDocument::sync_cache_to_offset(double offset) noexcept {
    // 检查必要性
    if (offset <= cache_valid_offset()) return;
    // union大法好
    union offset_t { uint64_t  len; double offset; };
    offset_t data; data.offset = offset;
    // 回调函数
    struct func_t {
        static bool func(void* this_ptr, uint64_t len) noexcept {
            const auto this_ = static_cast<CBCTextDocument*>(this_ptr);
            offset_t data; data.len = len;
            auto& this_line = this_->m_lines[this_->m_cValidLine];
            const double max_end = this_line.offset 
                + this_line.max_height1
                + this_line.max_height2
                ;
            return max_end >= data.offset;
        }
    };
    // 同步数据 
    this->sync_cache_until({ func_t::func }, data.len);
}



/// <summary>
/// Clears the last hittest.
/// </summary>
/// <returns></returns>
inline void TextBC::CBCTextDocument::clear_last_hittest() noexcept {
    m_lastHitTest = nullptr; 
#ifndef NDEBUG
    m_lastHitStart = 0xffff;
    m_lastHitCellPos = { -1.f, -1.f };
#endif
}

/// <summary>
/// Caches the last hittest.
/// </summary>
/// <param name="c">The c.</param>
/// <param name="l">The l.</param>
/// <param name="p">The p.</param>
/// <returns></returns>
inline void TextBC::CBCTextDocument::cache_last_hittest(CBCTextCell* c, uint32_t l, Point2F p) noexcept {
    m_lastHitTest = c; 
    m_lastHitStart = l; 
    m_lastHitCellPos = p;
}


/// <summary>
/// Adds the line.
/// </summary>
/// <param name="this_line">The this line.</param>
/// <param name="last_line">The last line.</param>
/// <param name="node">The node.</param>
/// <returns></returns>
auto TextBC::CBCTextDocument::add_line(
    TextLineData& this_line, 
    const TextLineData& last_line, 
    CBCTextCell * node) noexcept -> CBCTextCell * {
    // 继承上一行数据
    this_line.Clear(last_line);
    do {
        // 尝试重新布局文本节点
        CBCTextDocument::relayout_cell(*node);
        // 添加节点到改行
        this_line += *node;
        // 推进节点
        node = static_cast<CBCTextCell*>(node->next);
        // 遍历到行尾就退出
    } while (!this_line.last);
    // 返回下一行的首节点
    return node;
}

/// <summary>
/// Relayouts the cell.
/// </summary>
/// <param name="cell">The cell.</param>
void TextBC::CBCTextDocument::relayout_cell(CBCTextCell& cell) noexcept {
    // 只有脏节点需要重新布局
    if (!cell.IsDirty()) return;
    // 准备布局
    cell.BeginLayout();

    // TODO: 完成格式化
    // format(cell);

    // 结束
    cell.EndLayout();
}

PCN_NOINLINE
/// <summary>
/// Synchronizes the cache until X ??.
/// </summary>
/// <param name="this_ptr">The this PTR.</param>
/// <param name="arg">The argument.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::sync_cache_until(until_call call, uint64_t arg) noexcept {
    // 检查BUFFER有效性
    if (!m_lines.IsOK()) return;
    // 空数据...
    if (m_lines.GetSize() <= 1) return;
    assert(m_cValidLine <= m_lines.GetSize() && "bug");
    const auto itr = m_lines.GetData() + m_cValidLine;
    // 获取有效的首节点
    auto node = static_cast<CBCTextCell*>(m_head.next);
    if (m_cValidLine) node = static_cast<CBCTextCell*>(itr->last->next);
    // 遍历上限
    const uint32_t len = static_cast<uint32_t>(m_lines.GetSize()) - m_cValidLine - 1;
    //float offset_height = 0;
    // 遍历次数
    for (uint32_t i = 0; i != len; ++i) {
        auto& last_line = itr[i];
        auto& this_line = itr[i + 1];
        node = this->add_line(this_line, last_line, node);
        ++m_cValidLine;
        if (call.func(this, arg)) break;
    }
}

/// <summary>
/// Inserts the text.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="view">The view.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::InsertText(uint32_t pos, U16View view) noexcept {
    const auto total_string_len = m_cTotalLen;
    // 范围断言
    assert(pos <= total_string_len && "out of range");
    // 插入位置超过范围
    if (pos <= total_string_len) {
        this->insert_text(pos, view);
    }
}

// TODO: 移除
namespace TextBC {
    // is_surrogate
    inline bool IsSurrogate(uint16_t ch) noexcept { return ((ch) & 0xF800) == 0xD800; }
    // is_low_surrogate
    inline bool IsLowSurrogate(uint16_t ch) noexcept { return ((ch) & 0xFC00) == 0xDC00; }
    // is_high_surrogate
    inline bool IsHighSurrogate(uint16_t ch) noexcept { return ((ch) & 0xFC00) == 0xD800; }
}

// LongUI::detail
namespace TextBC { namespace detail {
    // get a nicepair
    U16View nice_view(uint32_t max, U16View view) noexcept {
        assert(max != 0 && "look bad");
        U16View rv = { view.first, view.first };
        max = std::min(max, uint32_t(view.second - view.first));
        while (max--) if (*rv.second == '\n') break; else ++rv.second;
        if (rv.second < view.second && IsLowSurrogate(*rv.second)) ++rv.second;
        assert(IsLowSurrogate(*rv.second) == false);
        return rv;
    }
}}


/// <summary>
/// Called when [l button down].
/// </summary>
/// <param name="pt">The pt.</param>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
void TextBC::CBCTextDocument::OnLButtonDown(Point2F pt, bool shift) noexcept {
    m_ptMoseDown = pt;
    // 没有选中
    if (!m_bClickInSelection) {
        this->SetSelection(pt, shift);
    }
    const auto range = this->GetSelectionRange();
    this->refresh_selection_metrics(range);
    this->platform.NeedRedraw();
}

/// <summary>
/// Called when [l button up].
/// </summary>
/// <param name="pt">The pt.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::OnLButtonUp(Point2F pt) noexcept {
    // 鼠标没有移动即表示移动插入符号
    if (m_bClickInSelection && m_ptMoseDown == pt) {
        this->SetSelection(pt, false);
    }
}


/// <summary>
/// Called when [l button hold].
/// </summary>
/// <param name="pt">The pt.</param>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
void TextBC::CBCTextDocument::OnLButtonHold(Point2F pt) noexcept {
    // XXX: 暂时这样
    this->SetSelection(pt, true);
    const auto range = this->GetSelectionRange();
    this->refresh_selection_metrics(range);
    this->platform.NeedRedraw();
}


/// <summary>
/// Called when [character].
/// </summary>
/// <param name="ch">The ch.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::OnChar(char32_t ch) noexcept {
    // TODO: 连续On Char优化
    char16_t buffer[2]; 
    buffer[0] = static_cast<char16_t>(ch);
    U16View view = { buffer, buffer + 1 };
    // utf32 -> utf32x2
    if (ch > 0xFFFF) {
        // From http://unicode.org/faq/utf_bom.html#35
        buffer[0] = static_cast<char16_t>(0xD800 + (ch >> 10) - (0x10000 >> 10));
        buffer[1] = static_cast<char16_t>(0xDC00 + (ch & 0x3FF));
        view.second = buffer + 2;
    }
    // 文本
    this->OnText(view);
}


/// <summary>
/// Finds the insert node.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="view">The view.</param>
/// <param name="insert_line_height">Height of the insert line.</param>
/// <param name="insert_offset">The insert offset.</param>
/// <returns></returns>
auto TextBC::CBCTextDocument::find_insert_node(
    uint32_t pos, 
    U16View view,
    float& insert_line_height,
    uint32_t& insert_offset) noexcept -> CBCTextCell * {
    // 预备数据
    TextLineData* line_data = m_lines.GetData() + 1;
    const auto insert_len = uint32_t(view.second - view.first);
    CBCTextCell* node = nullptr;
    // 插入之后的位置
    if (pos) {
        // 查找范围首节点
        const auto rv = this->find_cell_or_la_by_pos(pos);
        // 更新有效行
        const auto lineno = static_cast<uint32_t>(rv.line - m_lines.GetData());
        // 更新有效缓存行
        m_cValidLine = lineno - 1;
        // 插入偏移
        insert_offset = rv.cell->GetStringLen() + pos - rv.string_len;
        // 对应数据
        node = rv.cell;
        // 可以通过rv.line获取index(lineno)再访问该行
        // 以避免const_cast...不过无所谓了
        line_data = const_cast<TextLineData*>(rv.line);
        // 优先插入前方
        if (rv.line->first != node && insert_offset == 0) {
            const auto prev_node = static_cast<CBCTextCell*>(node->prev);
            if (prev_node->GetStringLen() < (TEXT_CELL_NICE_LENGTH - 1)) {
                node = prev_node;
                insert_offset = node->GetStringLen();
            }
        }
        // 获取行高
        insert_line_height = rv.line->max_height1 + rv.line->max_height2;
    }
    // 插入第一个位置
    else {
        m_cValidLine = 0;
        insert_offset = 0;
        node = static_cast<CBCTextCell*>(m_head.next);
        // 获取行高
        auto& line = m_lines[1];
        insert_line_height = line.max_height1 + line.max_height2;
    }
    // 分裂首个节点
    {
        const auto has_new_line = [view]() noexcept {
            return std::any_of(view.first, view.second, [](char16_t ch) noexcept {
                return ch == '\n';
            });
        };
        // 插入过长或者有换行则进行有丝分裂
        if (const auto oldlength = node->GetStringLen()) {
            if (insert_len + oldlength > TEXT_CELL_NICE_LENGTH || has_new_line()) {
                this->split_cell(*line_data, *node, insert_offset);
                if (oldlength == insert_offset) {
                    node = static_cast<decltype(node)>(node->next);
                    insert_offset = 0;
                }
            }
        }
    }
    return node;
}




/// <summary>
/// Adjusts the content size insert.
/// </summary>
/// <param name="insert_line_height">Height of the insert line.</param>
/// <param name="insert_begin_node">The insert begin node.</param>
/// <param name="insert_end_node">The insert end node.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::adjust_content_size_insert(
    float insert_line_height, 
    CBCTextCell* insert_begin_node, 
    CBCTextCell* insert_end_node) noexcept {
    // 范围扩散到行始终点
    const auto begin_node = insert_begin_node->FindBeginSameLine();
    const auto end_node = insert_end_node->FindEndSameLine();
    float max_line_width = 0.f;
    // 对范围内节点进行遍历
    const auto added = [=, &max_line_width]() noexcept {
        auto node = begin_node;
        float hight_added = 0.f;
        float height_this_line = 0.f;
        float line_width = 0.f;
        while (true) {
            // 对范围内节点进行重新布局
            CBCTextDocument::relayout_cell(*node);
            // 获取大小
            const auto size = node->GetSize();
            height_this_line = std::max(size.height, height_this_line);
            line_width += size.width;
            // 结束
            if (node == end_node) break;
            // 检查行结束
            if (node->IsEOL()) {
                hight_added += height_this_line;
                height_this_line = 0.f;
                max_line_width = std::max(max_line_width, line_width);
            }
            // 步进
            node = static_cast<decltype(node)>(node->next);
        }
        // 最后一行
        hight_added += height_this_line;
        max_line_width = std::max(max_line_width, line_width);
        // 返回
        return hight_added;
    }();
    // TODO: 浮点误差累计

    // 调整高度
    m_szContent.height += added - insert_line_height;
    // 调整宽度上限
    m_szContent.width = std::max(m_szContent.width, max_line_width);
    // 调整宽度下限
}




/// <summary>
/// Inserts the text.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="view">The view.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::insert_text(uint32_t pos, U16View view) noexcept {
    // 检查BUFFER有效性
    if (!m_lines.IsOK()) return;
    // TODO: 针对单个字符插入的优化
    // TODO: 针对末尾追加字符的优化

    // 范围断言
    assert(pos <= m_cTotalLen && "out of range");
    assert(view.second >= view.first && "bad string");
    // 检查是否有效
    if (view.second == view.first) return;
#ifdef TBC_UNDOREDO
    // 检查是否记录UNDO-REDO
    if (m_recordUndoRedo) {
        this->add_insert_text_unre(pos, view);
    }
#endif
    // 上次点击测试缓存失效
    this->clear_last_hittest();
    // 同步有效长度
    this->sync_cache_to_length(pos);
    // 插入偏移量
    uint32_t insert_offset = 0;
    float insert_line_height = 0.f;
    // 获取插入点以及相关数据
    const auto insert_begin_node = this->find_insert_node(
        pos, view, insert_line_height, insert_offset
    );
    auto node = insert_begin_node;
    // 循环待使用数据
    bool newline = false;
    uint32_t line_inserted = 0;
    uint32_t char_inserted = 0;
#ifndef NDEBUG
    int loop_count_dbg = 0;
#endif
    // 获取合适的串范围
    while (true) {
#ifndef NDEBUG
        ++loop_count_dbg;
#endif
        const auto nicelen = TEXT_CELL_NICE_LENGTH - node->GetStringLen();
        const U16View now = detail::nice_view(nicelen, view);

        // 检查是否起行
        if (newline) {
            node->MarkAsBOL();
            newline = false;
        }
        // 有效情况下
        assert(now.second != now.first);
        //if (now.second != now.first) {
        view.first = now.second;
        // 为节点插入字符串
        char_inserted += node->InsertText(insert_offset, now);
        // 之后的字符串插入0
        insert_offset = 0;
        // 检查是否换行
        if (*now.second == '\n' && m_flag & this->Flag_MultiLine) {
            newline = true;
            node->MarkAsEOL();
            ++line_inserted;
            view.first++;
            char_inserted += m_bCRLF + 1;
        }
        // 字符串用完
        if (view.first == view.second) break;
        //}
        // TODO: 错误处理
        node = node->NewAfterThis();
    }
    // 检查是否起行
    // XXX: 将检查放到循环内
    if (newline) {
        const auto next_node = node->IsLastCell() ? 
            node->NewAfterThis() :
            static_cast<decltype(node)>(node->next);
        //assert(next_node != &m_tail);
        assert(next_node->IsBOL() == false);
        next_node->MarkAsBOL();
        node = next_node;
    }
    // 修改中止点
    const auto insert_end_node = node;
    // 调整内容区大小
    this->adjust_content_size_insert(
        insert_line_height,
        insert_begin_node,
        insert_end_node
    );
    // 修改行号数量
    m_lines.Resize(m_lines.GetSize() + line_inserted);
    // 字符数量插入时候可能插入[AB\r\n\n\nAB]
    m_cTotalLen += char_inserted;
    // TODO: 字符数量
    m_cTotalCount += char_inserted;

#ifndef NDEBUG
    // 插入字符的差异
    const auto insert_len = uint32_t(view.second - view.first);
    if (char_inserted != insert_len) {
        char buffer[128];
        const auto fmt = "char_inserted, insert_len = %d, %d";
        std::sprintf(buffer, fmt, int(char_inserted), int(insert_len));
        this->platform.DebugOutput(buffer);
    }
#endif
    // XXX: 尝试合并插入末的两端节点

    // 标记修改
    this->text_changed();
}


/// <summary>
/// Refreshes the selection metrics.
/// </summary>
/// <param name="range">The range.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::refresh_selection_metrics(Range range) noexcept {
    assert(range.pos + range.len <= m_cTotalLen && "out of range");
    // 重复判断
    if (m_lastSelection.pos == range.pos && m_lastSelection.len == range.len) {
        return;
    }
    // 修改写入
    this->selection_changed();
    m_lastSelection = range;
    // 清空选择
    m_selections.Clear();
    if (!range.len) return;
    // 保存上一次点击测试结果
    const auto lastHitTest = m_lastHitTest;
    const auto lastHitStart = m_lastHitStart;
    const auto lastHitCellPos = m_lastHitCellPos;
    // XXX: 应该有一端的结果与上一次一样?
    // TODO: 多行的话选择的时候, 每行末尾应该有一段长度来标记换行

    // 进行起始位置的点击测试
    const auto startLine = this->hittest_at(range.pos);
    const auto startNode = m_lastHitTest;
    // 计算起始坐标
    const auto startX = this->char_metrics(
        *m_lastHitTest, range.pos - m_lastHitStart).x;

    // 就行结束为止的点击测试
    const auto endLine = this->hittest_at(range.pos + range.len);
    const auto endNode = m_lastHitTest;
    // 计算结束坐标
    const auto endX = this->char_metrics(
        *m_lastHitTest, range.pos + range.len - m_lastHitStart).x;

    // 内存不足
    const auto selsize = static_cast<uint32_t>(endLine - startLine + 1);
    if (!m_selections.Resize(selsize)) return;
    // 添加数据
    auto selection_push = m_selections.begin();
    for (auto itr = startLine; itr != endLine; ++itr) {
        const auto x = 0.f;
        const auto y = static_cast<float>(itr->offset);
        const auto w = itr->CalculateWidth();
        const auto h = itr->max_height1 + itr->max_height2;
        // 0.4的大概x-高度即可(准确一点应该是直接获取x-height)
        const auto xheight = h * 0.4f;
        *selection_push = { x, y, w + xheight, h };
        ++selection_push;
    }
    // 保存结束坐标
    {
        const auto x = 0.f;
        const auto y = static_cast<float>(endLine->offset);
        const auto w = endLine->CalculateWidth(endLine->first, endNode)
            - endNode->GetSize().width + endX
            ;
        const auto h = endLine->max_height1 + endLine->max_height2;
        *selection_push = { x, y, w, h };
    }
    // 修正开始坐标
    {
        const auto w = startLine->CalculateWidth(startLine->first, startNode)
            - startNode->GetSize().width + startX
            ;
        auto& start = m_selections[0];
        start.x = w;
        start.width -= w;
    }
    // 保存回去
    this->cache_last_hittest(lastHitTest, lastHitStart, lastHitCellPos);
#ifndef NDEBUG
#if 0
    char buffer[1024];
    const auto range = this->GetSelectionRange();
    std::sprintf(buffer, "range = { %d, %d }", int(range.pos), int(range.len));
    this->platform.DebugOutput(buffer);
#endif
#endif
}

/// <summary>
/// Removes the text.
/// </summary>
/// <param name="range">The range.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::RemoveText(Range range) noexcept {
    // 只读文本
    if (m_flag & this->Flag_ReadOnly)
        return this->play_beep();
    // 基本检查
    const uint32_t endpos = range.pos + range.len;
    assert(range.pos < m_cTotalLen && "out of range");
    assert(endpos <= m_cTotalLen && "out of range");
    // 根本没有删除
    if (!range.len) return;
    // 删除位置超过范围
    if (range.pos > m_cTotalLen) return;
    // 修正长度
    range.len = std::min(endpos, m_cTotalLen) - range.pos;
    // 正式执行
    this->remove_text(range);
}


/// <summary>
/// Removes all.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextDocument::remove_all() noexcept {
    // 特殊优化, 针对全部删完的处理


    // 上次点击测试缓存失效
    this->clear_last_hittest();


    // 删除文本
    auto node = static_cast<CBCTextCell*>(m_head.next);
    while (node != &m_tail) {
        const auto next = static_cast<CBCTextCell*>(node->next);
#ifdef TBC_UNDOREDO
        // 检查是否记录UNDO-REDO
        if (m_recordUndoRedo) {
            const auto ptr = node->GetStringPtr();
            const auto end = ptr + node->GetStringLen();
            this->add_remove_text_unre(0, { ptr, end });
        }
#endif
        node->RemoveText({ 0, node->GetStringLen() });
        node = next;
    }

    // 清空数据
    m_lines.Resize(2);
    m_cValidLine = 0;
    m_cTotalLen = 0;
    m_cTotalCount = 0;
    // 标记修改
    this->text_changed();
}



/// <summary>
/// Adjusts the content size remove.
/// </summary>
/// <param name="height_removed">The height removed.</param>
/// <param name="first">The first.</param>
/// <param name="last">The last.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::adjust_content_size_remove(
    float height_removed,
    CBCTextCell* first, 
    CBCTextCell* last) noexcept {
    // 必须是同一行的
    assert(first->FindEndSameLine() == last);
    float line_width = 0.f;
    float height_this_line = 0.f;
    // 遍历范围
    auto node = first;
    while (true) {
        // 对范围内节点进行重新布局
        CBCTextDocument::relayout_cell(*node);
        // 获取大小
        const auto size = node->GetSize();
        height_this_line = std::max(size.height, height_this_line);
        line_width += size.width;
        // 结束
        if (node == last) break;
        // 步进
        node = static_cast<decltype(node)>(node->next);
    }
    // 多行
    if (m_flag & Flag_MultiLine) {
        // 宽度上限
        m_szContent.width = std::max(m_szContent.width, line_width);
        // 宽度下限
    }
    // 单行
    else m_szContent.width = line_width;

    // 高度计算
    m_szContent.height += height_this_line;
    m_szContent.height -= height_removed;
}

/// <summary>
/// Removes the text.
/// </summary>
/// <param name="range">The range.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::remove_text(Range range) noexcept {
    // 检查BUFFER有效性
    if (!m_lines.IsOK()) return;
    if (!range.len) return;
    // 全部删完?
    if (range.len == m_cTotalLen) return this->remove_all();
    assert(range.len && "cannot remove 0 char");
    const uint32_t endpos = range.pos + range.len;
    assert(range.pos < m_cTotalLen && "out of range");
    assert(endpos <= m_cTotalLen && "out of range");
    // 上次点击测试缓存失效
    this->clear_last_hittest();
    // 同步有效长度
    this->sync_cache_to_length(endpos);
    // 查找范围首节点
    const auto rv = this->find_cell_or_la_by_pos(range.pos);
    // 记录当前行数
    const auto lineno = static_cast<uint32_t>(rv.line - m_lines.GetData());
    // 更新有效缓存行
    m_cValidLine = lineno - 1;
    // 删除首节点文本
    const auto start_length = rv.string_len - static_cast<uint32_t>(rv.cell->GetStringLen());
    uint32_t max2remove = rv.string_len - range.pos;
    auto remain_len = range.len;
    auto offset_pos = range.pos - start_length;
    auto node = rv.cell;
    uint32_t line_removed = 0;
    // 内容区调整用
    float this_line_height = TextLineData::CalculateHeight(node->FindBeginSameLine(), node);
    float height_removed = 0.f;
    // 直到删完
    while (true) {
        const auto this_node = node;
        node = static_cast<CBCTextCell*>(node->next);
        // 记录当前节点大小
        const auto node_size = this_node->GetSize();
        this_line_height = std::max(this_line_height, node_size.height);
        // 检查是否为行开始
        const auto linestart = this_node->IsBOL();
        // 准备删除的数控
        const auto removed = std::min(max2remove, remain_len);
#ifdef TBC_UNDOREDO
        // 检查是否记录UNDO-REDO
        if (m_recordUndoRedo) {
            const auto ptr = this_node->GetStringPtr() + offset_pos;
            const auto end = ptr + removed;
            this->add_remove_text_unre(endpos - remain_len, { ptr, end });
        }
#endif
        // 删除并检查是否删除行
        if (this_node->RemoveText({ offset_pos, removed })) {
            height_removed += this_line_height;
            this_line_height = 0.f;
            ++line_removed;
        }
        // 调整下次使用数据
        remain_len -= removed; offset_pos = 0;
        // 没得删了?
        if (!remain_len) break;
        // 节点删除 + 删除行?
        if (linestart && node->prev != this_node) {
            //node->beginofline = true;
            node->MarkAsBOL();
        }
        // 获取下次最大删除数量
        max2remove = node->GetStringLen();
    }
    assert(line_removed <= m_lines.GetSize());
    // 最后一行
    const auto prev = node->IsFirstCell() ? node : static_cast<decltype(node)>(node->prev);
    const auto first = prev->FindBeginSameLine();
    const auto last = prev->FindEndSameLine();
    const auto back = TextLineData::CalculateHeight(prev, last);
    height_removed += std::max(this_line_height, back);
    this->adjust_content_size_remove(height_removed, first, last);
    // 不用处理OOM了
    m_lines.Resize(m_lines.GetSize() - line_removed);
    // TODO: 删除一半双字字符
    m_cTotalLen -= range.len;
    // TODO: 字符数量
    m_cTotalCount -= range.len;
    // TODO: 尝试合并删除点两侧节点

    // 标记修改
    this->text_changed();
}


/// <summary>
/// Finds the cell or la by position.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
auto TextBC::CBCTextDocument::find_cell_or_la_by_pos(uint32_t pos) const noexcept ->find_rv {
    // 检查BUFFER有效性
    if (!m_lines.IsOK()) return {};
    // 最后一个有效字符
    if (pos == this->cache_valid_length())
        return this->find_last_valid_cell();
    // 范围断言
    assert(pos <= cache_valid_length() && "out of range");
    // 二分查找区间
    const auto b = m_lines.begin() + 1;
    const auto e = m_lines.begin() + m_cValidLine + 1;
    // 二分查找到行
    auto itr = std::lower_bound(b, e, pos, [](const TextLineData& data, uint32_t pos) {
        return data.string_len <= pos;
    });
    // 结果断言
    assert(itr != e && "not found");
    // 数据处理
    auto node = itr->first;
    auto length = itr[-1].string_len;
    auto count = itr[-1].char_count;
    // 行内定位到节点
    while (true) {
        length += node->GetStringLen();
        count += node->GetCharCount();
        if (length > pos) break;
        node = static_cast<CBCTextCell*>(node->next);
    }
    return{ &*itr, node, count, length };
}

/// <summary>
/// Finds the last valid cell.
/// </summary>
/// <returns></returns>
auto TextBC::CBCTextDocument::find_last_valid_cell() const noexcept -> find_rv {
    auto& line = m_lines[m_cValidLine];
    const auto cell = line.last;
    return{ &line, cell, line.char_count, line.string_len };
}

/// <summary>
/// Splits the cell.
/// </summary>
/// <param name="line">The line.</param>
/// <param name="node">The node.</param>
/// <param name="pos">The position.</param>
void TextBC::CBCTextDocument::split_cell(TextLineData& line, 
    CBCTextCell& node, uint32_t pos) noexcept  {
    // 长度为0不考虑
    //assert(pos != node.GetStringLen() && "UNTESTED");
    // 在尾分裂则不考虑
    /*if (pos != node.GetStringLen())*/ {
#ifndef NDEBUG
        if (pos != node.GetStringLen())
            this->platform.DebugOutput(u8"split cell");
        else 
            this->platform.DebugOutput(u8"split cell ZERO");
#endif
        const auto created = node.NewAfterThis();
        const auto raw_string = node.GetStringPtr();
        const auto endlen = static_cast<uint32_t>(node.GetStringLen());
        // 为创建的节点插入(有效)文本
        if (pos != node.GetStringLen())
            created->InsertText(0, { raw_string + pos, raw_string + endlen });
        //else
        //    created->MarkDirty();
        node.RemoveTextOnly({ pos, endlen - pos });
        // 插入行尾节点检查
        if (line.last == &node) {
            line.last = created;
            node.MoveEOL2Next();
        }
    }
}


/// <summary>
/// Hittests at.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
const TextBC::TextLineData* TextBC::CBCTextDocument::hittest_at(uint32_t pos) noexcept {
    // 检查BUFFER有效性
    if (!m_lines.IsOK()) return nullptr;
    // 同步有效长度
    this->sync_cache_to_length(pos);
    // 查找范围首节点
    const auto rv = this->find_cell_or_la_by_pos(pos);
    // 0值处理
    if (!rv.cell) {
        const auto node = static_cast<CBCTextCell*>(m_head.next);
        assert(node != &m_tail && "NO NODE");
        m_caretLineTemp = 0;
        this->cache_last_hittest(node, 0, { 0.f, 0.f });
        return rv.line;
    }
    // 保留行号
    m_caretLineTemp = static_cast<uint32_t>(rv.line - m_lines.GetData());
    // 计算XY偏移
    const auto xoffset = [&rv]() noexcept {
        float x = 0.f;
        for (auto node = rv.line->first;;) {
            if (node == rv.cell) break;
            x += node->GetSize().width;
            node = static_cast<CBCTextCell*>(node->next);
        }
        return x;
    }();
    const auto yoffset = static_cast<float>(rv.line->offset);
    // 计算起始值
    const auto strlen_start = rv.string_len - rv.cell->GetStringLen();
    // 保存缓存数据
    this->cache_last_hittest(rv.cell, strlen_start, { xoffset, yoffset });
    // 返回行数据
    return rv.line;
}


/// <summary>
/// Hits the test.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
auto TextBC::CBCTextDocument::HitTest(Point2F pos) noexcept -> TextBC::HitTest {
    // 检查BUFFER有效性
    if (!m_lines.IsOK()) return{};
    // XXX: 固定行距优化
    // 同步到指定行
    const double scrollx = m_ptViewport.x, scrolly = m_ptViewport.y;
    const double offy = scrolly + double(pos.y);
    this->sync_cache_to_offset(offy);
    // 获取对应迭代器, 超过范围的话取得最后一个有效迭代器aaa
    const auto itr = [this, offy]() noexcept {
        // 超过范围?
        if (m_lines.GetSize() == (m_cValidLine + 1) && offy >= this->cache_valid_offset()) {
            return m_lines.begin() + m_cValidLine;
        }
        // 二分查找到行
        const auto b = m_lines.begin() + 1;
        const auto e = m_lines.begin() + m_cValidLine + 1;
        return std::lower_bound(b, e, offy, [](const TextLineData& data, double y) {
            return data.offset + data.max_height1 + data.max_height2 <= y;
        });
    }();
    assert(itr < (m_lines.begin() + m_cValidLine + 1) && "not found");
    m_caretLineTemp = static_cast<uint32_t>(itr - m_lines.begin());
    assert(m_caretLineTemp);
    // 顺序遍历使用数据
    auto string_len = itr[-1].string_len;
    auto node = itr->first;
    const auto last = itr->last;
    float offx = float(scrollx) + pos.x;
    float xadded = 0.0;
    const auto yadded = itr->offset;
    // 顺序遍历到指定节点
    while (true) {
        const auto node_size_width = node->GetSize().width;
        if (node == last || offx < node_size_width) break;
        offx -= node_size_width;
        xadded += node_size_width;
        string_len += node->GetStringLen();
        node = static_cast<CBCTextCell*>(node->next);
    }
    // 节点
    union { TextBC::HitTest ht; float pos; } hittest_v;
    hittest_v.pos = offx;
    const auto ct = node->GetContent();
    this->platform.ContentEvent(*ct, platform.Event_HitTest, &hittest_v);
    // 保存缓存节点
    this->cache_last_hittest(node, string_len, { 
        static_cast<float>(xadded), 
        static_cast<float>(yadded)
    });
    // 结果
    const auto rva = hittest_v.ht.pos;
    const auto rvb = hittest_v.ht.u16_trailing;

    // 字符位置
    return { string_len + rva, rvb };
}


// richtb::impl namespace
namespace TextBC { namespace impl {
    // selection mode
    enum selection_mode : uint32_t {
        mode_all = 0,       // 全选文本
        mode_leading,       // 鼠标点击
        mode_trailing,      // 选择后缘
        //mode_up,            // 一行上升
        //mode_down,          // 一行下降
        mode_left,          // 左移一下
        mode_right,         // 右移一下
        //mode_home,          // 一行起始
        //mode_end,           // 一行结束
        //mode_first,         // 文本开头
        //mode_last,          // 文本结尾
        //mode_leftchar,      // 左移字符
        //mode_rightchar,     // 右移字符
        //mode_leftword,      // 左移字段
        //mode_rightword,     // 右移字段
    };
    // selection flag
    enum selection_flag : uint32_t {
        flag_keepanchor_i = 16,   // 简单说就是按住SHIFT
        flag_hittestpos_i = 17,   // 简单说就是请求位置缓存

        flag_keepanchor_v = 1 << flag_keepanchor_i,
        flag_hittestpos_v = 1 << flag_hittestpos_i
    };
}}


PCN_NOINLINE
/// <summary>
/// Truncations the text.
/// </summary>
/// <param name="view">The view.</param>
/// <param name="remain">The remain.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::truncation_text(U16View& view, ptrdiff_t remain) noexcept {
    auto proc_view = view;
    // 检查长度
    if (proc_view.second - proc_view.first > remain) {
        proc_view.second = proc_view.first + remain;
        const auto ch = proc_view.second[-1];
        // 输入一半还是不行
        if (TextBC::IsHighSurrogate(ch)) {
            --proc_view.second;
        }
    }
    // 检查多行换行
    if (!(m_flag & this->Flag_MultiLine))
        this->truncation_singleline(proc_view);
    // 检查有效密码
    if (m_flag & this->Flag_UsePassword)
        this->truncation_password(this->platform, proc_view);

    // 字符串截断检查
    if (proc_view.first != view.first ||
        proc_view.second != view.second) {
        view = proc_view;
        this->play_beep();
    }
}


/// <summary>
/// Truncations the singleline.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::truncation_singleline(U16View& view) noexcept {
    // 进行\n截断
    const auto begin_itr = view.first;
    const auto end_itr = view.second;
    // 遍历文本
    for (auto itr = begin_itr; itr != end_itr; ++itr) {
        // 截断换行符号
        if (*itr == '\n') {
            view.second = itr;
            break;
        }
    }
}

/// <summary>
/// Truncations the password.
/// </summary>
/// <param name="platform">The platform.</param>
/// <param name="view">The view.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::truncation_password(
    IBCTextPlatform& platform, U16View& view) noexcept {
    // 进行密码截断
    const auto begin_itr = view.first;
    const auto end_itr = view.second;
    // 遍历文本
    for (auto itr = begin_itr; itr != end_itr; ++itr) {
        // 截断双字
        if (TextBC::IsSurrogate(*itr)) {
            view.second = itr;
            break;
        }
        // 截断非法
        if (!platform.IsValidPassword(*itr)) {
            view.second = itr;
            break;
        }
    }
}

//PCN_NOINLINE
/// <summary>
/// Plays the beep.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextDocument::play_beep() noexcept {
    this->platform.ErrorBeep();
}

/// <summary>
/// Called when [text].
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::OnText(U16View view) noexcept {
    // 只读文本
    if (m_flag & this->Flag_ReadOnly)
        return this->play_beep();
    // 获取截断长度
    const auto l = m_cMaxLen + GetSelectionRange().len - m_cTotalLen;
    // 文本截断处理
    this->truncation_text(view, l);
#ifdef TBC_UNDOREDO
    // 开始记录
    m_recordUndoRedo = true;
#endif
    // 删除选中文本
    this->delete_selection();
    // FIXME: 不能视为新的节点
    this->MarkEndOfOperation();
    // 保存当前长度
    const auto old_len = m_cTotalLen;
    // 插入输入文本
    this->insert_text(get_abs_pos(), view);
    // 计算文本长度
    const uint32_t len = m_cTotalLen - old_len;
    // 移动插入符号
    this->set_selection(impl::mode_right | impl::flag_hittestpos_v, len);
#ifdef TBC_UNDOREDO
    // 停止记录
    m_recordUndoRedo = false;
#endif
    // 更新选择区域
    this->refresh_selection_metrics(GetSelectionRange());
    // 要求平台刷新
    this->platform.NeedRedraw();
}


/// <summary>
/// Sets the text.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::SetText(U16View view) noexcept {
    // 文本截断处理
    this->truncation_text(view, m_cMaxLen);
    // 移除选择文本
    this->remove_text({ 0, m_cTotalCount });
    // 插入输入文本
    this->insert_text(0, view);
    // 移动插入符号
    this->set_selection(impl::mode_leading | impl::flag_hittestpos_v, m_cTotalLen);
    // 更新选择区域
    this->refresh_selection_metrics(GetSelectionRange());
    // 要求平台刷新
    this->platform.NeedRedraw();
}

/// <summary>
/// Called when [new line].
/// </summary>
/// <param name="ctrl">if set to <c>true</c> [control].</param>
/// <returns></returns>
bool TextBC::CBCTextDocument::OnNewLine() noexcept {
    // 多行检查
    if (m_flag & this->Flag_MultiLine) {
        const char16_t buffer[] = { '\r', '\n' };
        U16View view = { buffer, buffer + 2 };
        if (!this->IsCRLF()) view.first = buffer + 1;
        this->OnText(view);
        return false;
    }
    // 单行模式
    return true;
}


/// <summary>
/// Called when [backspace].
/// </summary>
/// <param name="ctrl">if set to <c>true</c> [control].</param>
/// <returns></returns>
void TextBC::CBCTextDocument::OnBackspace(bool ctrl) noexcept {
    // 只读文本
    if (m_flag & this->Flag_ReadOnly)
        return this->play_beep();

    const auto abspos = this->get_abs_pos();
    // 有选择的话就删掉选择的
    if (abspos != m_uAnchor) {
        this->DeleteSelection();
    }
    else {
        // 根本没有头发
        if (!abspos) return;
#ifdef TBC_UNDOREDO
        // 标记撤销记录
        m_recordUndoRedo = true;
#endif
        // TODO: (跨单元格)CTRL删除字符集群
        const auto count = this->get_strlen_before();
        // 移除选择前方
        this->remove_text({ abspos - count, count });
        // 设置当前选择
        this->set_selection(impl::mode_left | impl::flag_hittestpos_v, count);
#ifdef TBC_UNDOREDO
        // 停止记录
        m_recordUndoRedo = false;
#endif
    }
}



/// <summary>
/// Gets the strlen before.
/// </summary>
/// <returns></returns>
auto TextBC::CBCTextDocument::get_strlen_before() const noexcept -> uint32_t {
    const auto abspos = m_uAnchor;
    // TODO: 是断言还是返回？
    assert(abspos);
    //if (!abspos) return 0;
    // 预先返回1
    uint32_t count = 1;
    assert(abspos <= m_cTotalLen);
    // 在可能是双字的情况下
    if (abspos > 1) {
        // 检查缓存失效
        assert(m_lastHitTest);
        assert(m_lastHitCellPos.x >= 0.f);
        assert(m_lastHitCellPos.y >= 0.f);
        assert(get_abs_pos() >= m_lastHitStart);
        // 因为是删除插入符号位置, 直接用上次的即可
        const uint32_t index = abspos - m_lastHitStart;
        const auto node = m_lastHitTest;

        // 本单元格处理
        if (index) {
            const auto ch2 = node->GetStringPtr()[index - 1];
            // 单元格限制: 本单元格前方不会有换行
            assert(ch2 != '\n');
            // 判断是否为双字节(使用IsSurrogate不考虑高低位)
            if (TextBC::IsSurrogate(ch2)) {
                // 单元格限制, 不能在不同的单元格删除一个字符
                assert(index != 1);
                // 获取前一个字符
                const auto ch1 = node->GetStringPtr()[index - 2];
                // 编码限制, 非法UTF-16字符串
                assert(TextBC::IsSurrogate(ch1) && "bad utf-16");
                // 调整为双字符
                count = 2; ch1;
            }
        }
        // 前单元格处理
        else {
            const auto this_node = static_cast<CBCTextCell*>(node->prev);
            const auto this_leng = this_node->GetStringLen();
            const auto this_sptr = this_node->GetStringPtr() + this_leng;
            const auto ch2 = this_sptr[-1];
            // 换行?
            if (ch2 == '\n') {
                // 单元格限制, 不能在不同的单元格删除一个字符
                assert(this_leng > 1);
                // CR字符
                if (this_sptr[-2] == '\r') count = 2;
                // HACK: 特殊处理, 本节点就是一个单独的\r\n就删除后面

            }
            // 判断是否为双字节(使用IsSurrogate不考虑高低位)
            else if (TextBC::IsSurrogate(ch2)) {
                // 单元格限制, 不能在不同的单元格删除一个字符
                assert(this_leng > 1);
                // 编码限制, 非法UTF-16字符串
                assert(TextBC::IsSurrogate(this_sptr[-2]) && "bad utf-16");
                // 调整为双字符
                count = 2;
            }
        }
    }
    return count;
}


/// <summary>
/// Gets the strlen behind.
/// </summary>
/// <returns></returns>
auto TextBC::CBCTextDocument::get_strlen_behind() const noexcept -> uint32_t {
    const auto abspos = m_uAnchor;
    assert(abspos <= m_cTotalLen);
    // TODO: 是断言还是返回？
    assert(abspos != m_cTotalLen);
    if (abspos == m_cTotalLen) return 0;
    // 预先返回
    uint32_t count = 1;
    // 在可能是双字的情况下
    if (abspos < m_cTotalLen - 1) {
        // 检查缓存失效
        assert(m_lastHitTest);
        assert(m_lastHitCellPos.x >= 0.f);
        assert(m_lastHitCellPos.y >= 0.f);
        assert(get_abs_pos() >= m_lastHitStart);
        // 因为是删除插入符号位置, 直接用上次的即可
        const uint32_t index = abspos - m_lastHitStart;
        const auto node = m_lastHitTest;

        // 本单元格处理
        if (index != node->GetStringLen()) {
            const auto nollen = node->GetStrLenNoEOL();
            // 判断是否为换行
            if (index == nollen) {
                count = node->GetStringLen() - nollen;
            }
            // 判断是否为双字节(使用IsSurrogate不考虑高低位)
            else if (TextBC::IsSurrogate(node->GetStringPtr()[index])) {
                // 单元格限制, 不能在不同的单元格删除一个字符
                assert(index != node->GetStringLen() - 1);
                // 获取后一个字符
                const auto ch1 = node->GetStringPtr()[index + 1];
                // 编码限制, 非法UTF-16字符串
                assert(TextBC::IsSurrogate(ch1) && "bad utf-16");
                // 调整为双字符
                count = 2; ch1;
            }
        }
        // 后单元格处理
        else {
            // 后面自然不能为空
            assert(!node->IsLastCell());
            const auto this_node = static_cast<decltype(node)>(node->next);
            assert(this_node->GetStringLen());
            // 后单元格限制: 后单元格开头没有换行符号
            const auto ch1 = this_node->GetStringPtr()[0];
            assert(ch1 != '\n' || ch1 != '\r');
            // 判断是否为双字字符
            if (TextBC::IsSurrogate(ch1)) {
                // 不能只有一个
                assert(this_node->GetStringLen() > 1);
                // 不能为非法UTF-16字符
                const auto ch2 = this_node->GetStringPtr()[1]; ch2;
                assert(TextBC::IsSurrogate(ch2) && "bad utf-16"); 
                // 调整为双字
                count = 2;
            }
        }
    }
    return count;
}

/// <summary>
/// Called when [delete].
/// </summary>
/// <param name="ctrl">if set to <c>true</c> [control].</param>
/// <returns></returns>
void TextBC::CBCTextDocument::OnDelete(bool ctrl) noexcept {
    // 只读文本
    if (m_flag & this->Flag_ReadOnly)
        return this->play_beep();

    const auto abspos = this->get_abs_pos();
    // 有选择的话就删掉选择的
    if (abspos != m_uAnchor) {
        this->DeleteSelection();
    }
    else {
        // 根本没有头发
        if (abspos >= m_cTotalLen) return;
#ifdef TBC_UNDOREDO
        // 标记撤销记录
        m_recordUndoRedo = true;
#endif
        // TODO: (跨单元格)CTRL删除字符集群
        const auto count = this->get_strlen_behind();
        // 移除选择后方
        this->remove_text({ abspos, count });
        // 设置当前选择
        this->set_selection(impl::mode_leading, abspos);
#ifdef TBC_UNDOREDO
        // 停止记录
        m_recordUndoRedo = false;
#endif
        // XXX: 大多数情况下, DELETE可以保留上次的HITTEST结果

        // 设置点击测试位置
        this->hittest_at(abspos);
        // 请求重绘
        this->platform.NeedRedraw();
    }
}

/// <summary>
/// Called when [left].
/// </summary>
/// <param name="ctrl">if set to <c>true</c> [control].</param>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
void TextBC::CBCTextDocument::OnLeft(bool ctrl, bool shift) noexcept {
    if (m_uAnchor == 0) return;
    // XXX: 本单元格移动可以不用重新HIT TEST
    // TODO: (跨单元格)CTRL移动插入符(经过字符集群)
    assert((shift & 1) == shift && "bad bool");
    const auto mode = impl::mode_left;
    const auto flag = uint32_t(shift) << impl::flag_keepanchor_i
        | impl::flag_hittestpos_v;
    this->set_selection(mode | flag, this->get_strlen_before());
    this->debug_anchor();

    // 更新选择区域
    this->refresh_selection_metrics(GetSelectionRange());
}


/// <summary>
/// Called when [right].
/// </summary>
/// <param name="ctrl">if set to <c>true</c> [control].</param>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
void TextBC::CBCTextDocument::OnRight(bool ctrl, bool shift) noexcept {
    if (m_uAnchor == m_cTotalCount) return;
    // XXX: 本单元格移动可以不用重新HIT TEST
    // TODO: (跨单元格)CTRL移动插入符(经过字符集群)
    assert((shift & 1) == shift && "bad bool");
    const auto mode = impl::mode_right;
    const auto flag = uint32_t(shift) << impl::flag_keepanchor_i
        | impl::flag_hittestpos_v;
    this->set_selection(mode | flag, this->get_strlen_behind());
    this->debug_anchor();

    // 更新选择区域
    this->refresh_selection_metrics(GetSelectionRange());
}

/// <summary>
/// Called when [up].
/// </summary>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
void TextBC::CBCTextDocument::OnUp(bool shift) noexcept {
    // 第一行就算了
    assert(m_caretLineReal);
    if (m_caretLineReal < 2) return;
    // 检查有效行数
    assert(m_cValidLine >= m_caretLineReal);
    // 获取行高
    const auto& line = m_lines[m_caretLineReal - 1];
    const auto line_height = line.max_height1 + line.max_height2;
    const auto xx = m_rcCaret.x/* + 1.f*/;
    const auto yy = m_rcCaret.y - line_height;
    this->SetSelection({ xx, yy }, shift);
    // 更新选择区域
    this->refresh_selection_metrics(GetSelectionRange());
}

/// <summary>
/// Called when [down].
/// </summary>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
void TextBC::CBCTextDocument::OnDown(bool shift) noexcept {
    // 检查BUFFER有效性
    if (!m_lines.IsOK()) return;
    // 最后一行就算了
    assert(m_caretLineReal);
    if (m_caretLineReal >= m_lines.GetSize()-1) return;
    // 检查有效行数
    assert(m_cValidLine >= m_caretLineReal);
    // 获取行高
    const auto& line = m_lines[m_caretLineReal];
    const auto line_height = line.max_height1 + line.max_height2;
    const auto xx = m_rcCaret.x/* + 1.f*/;
    const auto yy = m_rcCaret.y + line_height;
    this->SetSelection({ xx, yy }, shift);
    // 更新选择区域
    this->refresh_selection_metrics(GetSelectionRange());
}


/// <summary>
/// Called when [select all].
/// </summary>
/// <returns></returns>
void TextBC::CBCTextDocument::OnSelectAll() noexcept {
    // 设置选择区域
    this->set_selection(
        impl::mode_all 
        | impl::flag_keepanchor_v 
        | impl::flag_hittestpos_v, 0
    );
    // 更新选择区域
    this->refresh_selection_metrics(GetSelectionRange());
}


/// <summary>
/// Called when [home].
/// </summary>
/// <param name="ctrl">if set to <c>true</c> [control].</param>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
void TextBC::CBCTextDocument::OnHome(bool ctrl, bool shift) noexcept {

}

/// <summary>
/// Called when [end].
/// </summary>
/// <param name="ctrl">if set to <c>true</c> [control].</param>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
void TextBC::CBCTextDocument::OnEnd(bool ctrl, bool shift) noexcept {

}

/// <summary>
/// Requests the text.
/// </summary>
/// <param name="string">The string.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::request_text(void* string) noexcept {
    auto node = m_head.next;
    while (node != &m_tail) {
        const auto cell = static_cast<CBCTextCell*>(node);
        const auto ptr = cell->GetStringPtr();
        const auto len = cell->GetStringLen();
        this->platform.GenerateText(string, { ptr, ptr + len });
        node = node->next;
    }
}


/// <summary>
/// Requests the range.
/// </summary>
/// <param name="string">The string.</param>
/// <param name="range">The range.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::request_range(void* string, Range range) noexcept {
    assert(range.pos < m_cTotalLen && "out of range");
    assert(range.pos + range.len <= m_cTotalLen && "out of range");
    // 超过范围
    if (range.pos + range.len > m_cTotalLen) return;
    // 根本没有头发
    if (!range.len) return;
    // 检查范围

    // XXX: 保存两端的测试结果?

    // 保存上一次点击测试结果
    const auto lastHitTest = m_lastHitTest;
    const auto lastHitStart = m_lastHitStart;
    const auto lastHitCellPos = m_lastHitCellPos;

    // 进行起始位置的点击测试
    this->hittest_at(range.pos);
    const auto start_node = m_lastHitTest;
    const auto start_offset = range.pos - m_lastHitStart;

    // 进行结束位置的点击测试
    this->hittest_at(range.pos + range.len);
    const auto end_node = m_lastHitTest;
    const auto end_offset = range.pos + range.len - m_lastHitStart;

    // 保存回去
    this->cache_last_hittest(lastHitTest, lastHitStart, lastHitCellPos);

    // 进行计算
    auto node = start_node;
    uint32_t offset = start_offset;
    while (node != end_node) {
        const auto ptr = node->GetStringPtr();
        const auto a = ptr + offset;
        const auto b = ptr + node->GetStringLen();
        this->platform.GenerateText(string, { a, b });
        node = static_cast<decltype(node)>(node->next);
        offset = 0;
    }
    // 最后一个节点
    const auto ptr = node->GetStringPtr();
    const auto a = ptr + offset;
    const auto b = ptr + end_offset;
    this->platform.GenerateText(string, { a, b });
}

/// <summary>
/// Deletes the selection.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextDocument::delete_selection() noexcept {
    // 获取选择区域
    const auto range = this->GetSelectionRange();
    // 根本没有头发
    if (!range.len) return;
#ifdef TBC_UNDOREDO
    // 动态长度
    m_undoDynamic = range.len;
#endif
    // 移除选择文本
    this->remove_text(range);
    // 取消选择区域
    this->set_selection(impl::mode_leading | impl::flag_hittestpos_v, range.pos);
#ifdef TBC_UNDOREDO
    // 动态长度
    m_undoDynamic = 0;
#endif
}

/// <summary>
/// Deletes the selection.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextDocument::DeleteSelection() noexcept {
#ifdef TBC_UNDOREDO
    // 标记撤销记录
    m_recordUndoRedo = true;
#endif
    // 直接删除选择
    this->delete_selection();
    // 更新选择区域
    this->refresh_selection_metrics(GetSelectionRange());
    // 要求平台刷新
    this->platform.NeedRedraw();
#ifdef TBC_UNDOREDO
    // 取消标记记录
    m_recordUndoRedo = false;
#endif
}

/// <summary>
/// Gets the selection range.
/// </summary>
/// <returns></returns>
auto TextBC::CBCTextDocument::GetSelectionRange() const noexcept -> Range {
    // 返回当前选择返回
    auto caretBegin = m_uAnchor;
    auto caretEnd = m_uCaretPos;
    // 相反则交换
    if (caretBegin > caretEnd) {
        std::swap(caretBegin, caretEnd);
    }
    // 限制长度
    caretBegin = std::min(caretBegin, m_cTotalLen);
    caretEnd = std::min(caretEnd, m_cTotalLen);
    // 返回范围
    return { caretBegin, caretEnd - caretBegin };
}

/// <summary>
/// Sets the selection.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="keep_anchor">if set to <c>true</c> [keep anchor].</param>
/// <returns></returns>
void TextBC::CBCTextDocument::SetSelection(uint32_t pos, bool keep_anchor) noexcept {
    assert((keep_anchor & 1) == keep_anchor && "bad bool");
    const auto mode = impl::mode_leading;
    const auto flag = uint32_t(keep_anchor) << impl::flag_keepanchor_i;
    this->set_selection(mode | flag, pos);
}

/// <summary>
/// Sets the selection from point.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="keep_anchor">if set to <c>true</c> [keep anchor].</param>
/// <returns></returns>
void TextBC::CBCTextDocument::SetSelection(Point2F pos, bool keep_anchor) noexcept {
    assert((keep_anchor & 1) == keep_anchor && "bad bool");
    const auto ht = this->HitTest(pos);
    const auto mode = impl::mode_leading;
    const auto flag = uint32_t(keep_anchor) << impl::flag_keepanchor_i;
    this->set_selection(mode | flag, ht.pos);
}


/// <summary>
/// Sets the size of the viewport.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::SetViewportSize(SizeF size) noexcept {
    if (m_szViewport.width != size.width || m_szViewport.height != size.height) {
        m_szViewport = size;
    }
}


/// <summary>
/// Sleeps this instance.
/// </summary>
/// <param name="free_undo_stack">if set to <c>true</c> [free undo stack].</param>
/// <returns></returns>
void TextBC::CBCTextDocument::Sleep() noexcept {
    // 释放缓存表
    this->free_mem_list();
    // 一次删除各个节点
    auto node = static_cast<CBCTextCell*>(m_head.next);
    while (node != &m_tail) {
        node->Sleep();
        node = static_cast<CBCTextCell*>(node->next);
    }
}


/// <summary>
/// Frees the list.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextDocument::free_mem_list() noexcept {
    // 释放节点缓存表
    auto node = m_pFreeList;
    m_pFreeList = nullptr;
    while (node) {
        const auto next = static_cast<decltype(node)>(node->next);
        node->DeleteNode();
        node = next;
    }
}

/// <summary>
/// Sets the viewport position.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::SetViewportPos(Point2F pos) noexcept {
    if (m_ptViewport == pos) return;
    // 修改显示位置
    m_ptViewport = pos;
    // 计算起始行数
    {
        // 保存上一次点击测试结果
        const auto lastHitTest = m_lastHitTest;
        const auto lastHitStart = m_lastHitStart;
        const auto lastHitCellPos = m_lastHitCellPos;
        // 点击测试
        this->HitTest({});
        // 记录地点
        const auto start_line = m_caretLineTemp;
        m_beginLineVisible = start_line;
        //char buffer[64];
        //std::sprintf(buffer, "start_line: %d\n", int(start_line));
        //this->platform.DebugOutput(buffer);
        // 保存回去
        this->cache_last_hittest(lastHitTest, lastHitStart, lastHitCellPos);
    }
    // 要求刷新
    this->platform.NeedRedraw();
}

/// <summary>
/// Sets the selection.
/// </summary>
/// <param name="mode_flag">The mode flag.</param>
/// <param name="pos">The position.</param>
/// <remarks>
/// mode_flag: 低16位是selection_mode, 高位是selection_flag
/// </remarks>
/// <returns></returns>
void TextBC::CBCTextDocument::set_selection(
    uint32_t mode_flag,
    uint32_t pos) noexcept {
    // 记录旧数据
    const auto anchor_old = m_uAnchor;
    const auto abspos_old = this->get_abs_pos();
    // 查看低16位的数据
    switch (static_cast<impl::selection_mode>(mode_flag & 0xffffu))
    {
    default: assert(!"BAD MODE"); break;
    case impl::mode_all:
        m_uAnchor = 0;
    //    [[fallthrough]];
    //case impl::mode_last:
        m_uCaretPos = m_cTotalLen;
        break;
    case impl::mode_leading:
        m_uCaretPos = pos;
        break;
    //case impl::mode_trailing:
    //    break;
    //case impl::mode_up:
    //    break;
    //case impl::mode_down:
    //    break;
    case impl::mode_left:
        if (m_uCaretPos >= pos) {
            m_uCaretPos -= pos;
        }
        else m_uCaretPos = 0;
        break;
    case impl::mode_right:
        m_uCaretPos = abspos_old + pos;
        m_uCaretPos = std::min(m_cTotalCount, m_uCaretPos);
        break;
    //case impl::mode_leftchar:
    //    m_uCaretPos = abspos_old;
    //    m_uCaretPos -= std::min(pos, abspos_old);
    //    break;
    //case impl::mode_rightchar:
    //    break;
    //case impl::mode_leftword:
    //    break;
    //case impl::mode_rightword:
    //    break;
    }
    // 当前位置
    const auto abspos_now = this->get_abs_pos();
    // 不保留上次锚点
    if (!(mode_flag & impl::flag_keepanchor_v)) m_uAnchor = abspos_now;
    // 移动了?
    if (abspos_now != abspos_old || m_uAnchor != abspos_old) {
        // 标记新的操作
        this->MarkEndOfOperation();
        // 请求缓存更新
        if (mode_flag & impl::flag_hittestpos_v)
            this->hittest_at(abspos_now);
        this->update_caret_rect();
        this->platform.NeedRedraw();
    }
}

/// <summary>
/// Updates the caret rect.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextDocument::update_caret_rect() noexcept {
    // 检查缓存失效
    assert(m_lastHitTest);
    assert(m_lastHitCellPos.x >= 0.f);
    assert(m_lastHitCellPos.y >= 0.f);
    assert(get_abs_pos() >= m_lastHitStart);
    m_caretLineReal = m_caretLineTemp;
    // 上一次点击测试过的节点
    const auto node = m_lastHitTest;
    // 利用文本相对位置
    const uint32_t offset = this->get_abs_pos() - m_lastHitStart;
    const auto cm = this->char_metrics(*node, offset);
    // 记录位置
    m_rcCaret.x = cm.x + m_lastHitCellPos.x;
    //m_rcCaret.width = cm_data.cm.width;
    m_rcCaret.width = 1;
    m_rcCaret.y = m_lastHitCellPos.y;
    m_rcCaret.height = node->GetSize().height;

    m_bDrawCaret = true;
}


PCN_NOINLINE
/// <summary>
/// Characters the metrics.
/// </summary>
/// <param name="cell">The cell.</param>
/// <param name="offset">The offset.</param>
/// <returns></returns>
auto TextBC::CBCTextDocument::char_metrics(
    CBCTextCell& cell, uint32_t offset) noexcept -> CharMetrics {
    // TODO: 优化: 貌似点击字符后会进行两次hittest, 一次是坐标, 一次是偏移
    // 基础数据
    union { uint32_t pos; CharMetrics cm; } cm_data;
    // 越界检查
    if (offset >= cell.GetStrLenNoEOL()) {
        cm_data.cm.x = cell.GetSize().width;
        cm_data.cm.width = 0.f;
    }
    else {
        cm_data.pos = offset;
        const auto ct = cell.GetContent();
        this->platform.ContentEvent(*ct, platform.Event_CharMetrics, &cm_data);
    }
    return cm_data.cm;
}

/// <summary>
/// Recalculates the size of the content.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextDocument::recalculate_content_size() noexcept {
    // 缓存数据必须全部存在
    assert(m_cValidLine == m_lines.GetSize() - 1);

    // 清空数据
    float& width = m_szContent.width;
    float& height = m_szContent.height;
    width = height = 0.f;
    const auto dofor = [&height, &width](const TextLineData& x) noexcept {
        height += x.max_height1 + x.max_height2;
        width = std::max(width, x.CalculateWidth());
    };
    std::for_each(m_lines.begin() + 1, m_lines.end(), dofor);
}


#ifndef NDEBUG
/// <summary>
/// Debugs the anchor.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextDocument::debug_anchor() noexcept {
    char buffer[32];
    std::sprintf(buffer, "anchor = %d", int(m_uAnchor));
    this->platform.DebugOutput(buffer);
}

#endif




// ============================================================================
// ---------------------------     UnDo-ReDo    -------------------------------
// ============================================================================

#ifdef TBC_UNDOREDO

// textbc::impl
namespace TextBC { namespace impl {
    // fixed length
    enum : uint32_t { fixed_length = TEXT_CELL_NICE_MAX + 2 };
    // invalid pos
    enum : uint32_t { invalid_pos = static_cast<uint32_t>(-1) };
    // do type
    enum do_type : uint8_t {
        // insert text buffer -> static-length buffer
        type_insert_sb = 0,
        // insert text buffer -> dynamic-length buffer
        type_insert_db,
        // remove text buffer -> static-length buffer
        type_remove_sb,
        // remove text buffer -> dynamic-length buffer
        type_remove_db,
    };
    // undo redo 
    struct undo_redo_t {
        // prev node
        undo_redo_t*    prev;
        // next node
        undo_redo_t*    next;
        // pos
        uint32_t        pos;
        // length
        uint32_t        length;
        // end of operation
        bool            eoo;
        // type of this
        do_type         type;
        // char buffer - for \0
        char16_t        buffer[1];
    };
    // push view
    void push_view(undo_redo_t& node, U16View view) noexcept {
        const uint32_t len = static_cast<uint32_t>(view.second - view.first);
#ifndef NDEBUG
        switch (node.type)
        {
        case type_insert_sb:
        case type_remove_sb:
            assert(node.length + len <= fixed_length);
        }
#endif
        const size_t bylen = reinterpret_cast<const char*>(view.second)
            - reinterpret_cast<const char*>(view.first);
        std::memcpy(node.buffer + node.length, view.first, bylen);
        node.length += len;
        node.buffer[node.length] = 0;
    }
    // create new node: fixed-length
    PCN_NOINLINE auto create_node(uint32_t len) noexcept {
        const size_t ex = sizeof(char16_t) * len;
        const auto ptr = std::malloc(sizeof(undo_redo_t) + ex);
        const auto node = reinterpret_cast<undo_redo_t*>(ptr);
        if (node) {
            std::memset(node, 0, sizeof(undo_redo_t) + ex);
            //node->range.pos = impl::invalid_pos;
        }
        return node;
    }
}}


/// <summary>
/// Clears the undo stack.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextDocument::ClearUndoStack() noexcept {
    this->free_undo_stack();
}



/// <summary>
/// Inserts the text [un-re].
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::insert_text_unre(const void* ptr) noexcept {
    auto& node = *reinterpret_cast<const impl::undo_redo_t*>(ptr);
    U16View view;
    // 计算文本区间
    view.first = node.buffer;
    view.second = node.buffer + node.length;
    // 插入输入文本
    this->insert_text(node.pos, view);
    // 移动插入符号
    this->set_selection(
        impl::mode_leading | impl::flag_hittestpos_v,
        node.pos + node.length
    );
    // 更新选择区域
    this->refresh_selection_metrics({ m_uAnchor, 0 });
    // 重绘
    this->platform.NeedRedraw();
}

/// <summary>
/// Removes the text [un-re].
/// </summary>
/// <param name="ptr">The pointer.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::remove_text_unre(const void* ptr) noexcept {
    auto& node = *reinterpret_cast<const impl::undo_redo_t*>(ptr);
    // 获取区间
    const Range range = { node.pos, node.length };
    // 正式执行
    this->remove_text(range);
    // 移动插入符号
    this->set_selection(
        impl::mode_leading | impl::flag_hittestpos_v,
        node.pos
    );
    // 更新选择区域
    this->refresh_selection_metrics({ m_uAnchor, 0});
    // 重绘
    this->platform.NeedRedraw();
}

/// <summary>
/// Undoes this instance.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextDocument::Undo() noexcept {
    // 检查是否能重做
    if (this->CanUndo()) {
        const auto node = reinterpret_cast<impl::undo_redo_t*>(m_pUndoNow);
        m_pUndoNow = node->next;
        assert(m_pUndoNow && "bad node");
        switch (node->type)
        {
        case impl::type_insert_sb:
        case impl::type_insert_db:
            this->remove_text_unre(node);
            break;
        default:
            this->insert_text_unre(node);
            break;
        }
    }
    // 错误音效
    else {
        this->play_beep();
    }
}


/// <summary>
/// Adds the insert text unre.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="view">The view.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::add_insert_text_unre(
    uint32_t pos, U16View view) noexcept {
    // 可以连续?
    const uint32_t len = static_cast<uint32_t>(view.second - view.first);
    const auto ptr = this->operation_length(len);
    // 节点有效
    if (const auto node = reinterpret_cast<impl::undo_redo_t*>(ptr)) {
        node->pos = pos;
        node->type = m_undoDynamic
            ? impl::type_insert_db
            : impl::type_insert_sb
            ;
        impl::push_view(*node, view);
    }
}

/// <summary>
/// Adds the remove text unre.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="view">The view.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::add_remove_text_unre(
    uint32_t pos, U16View view) noexcept {
    // 可以连续?
    const uint32_t len = static_cast<uint32_t>(view.second - view.first);
    const auto ptr = this->operation_length(len);
    // 节点有效
    if (const auto node = reinterpret_cast<impl::undo_redo_t*>(ptr)) {
        node->pos = pos;
        node->type = m_undoDynamic 
            ? impl::type_remove_db 
            : impl::type_remove_sb
            ;
        impl::push_view(*node, view);
    }
}


/// <summary>
/// Operations the length.
/// </summary>
/// <param name="len">The length.</param>
/// <returns></returns>
void* TextBC::CBCTextDocument::operation_length(uint32_t len) noexcept {
    const uint32_t node_len = m_undoDynamic ? m_undoDynamic : impl::fixed_length;
    // 没有先创建
    //if (!m_pUndoNow) m_pUndoNow = impl::create_node(node_len);
    const auto old = reinterpret_cast<impl::undo_redo_t*>(m_pUndoNow);
    // 再没有就是内存不足
    //if (!old) return nullptr;
    // 为空或者不允许添加或者BUF已满则重新创建
    if (m_pUndoNow == &m_undoStackBottom || old->eoo 
        || old->length + len > node_len) {
        // 不允许则创建一个新的节点
        const auto nwe = impl::create_node(node_len);
        // 内存不足
        if (!nwe) return nullptr;
        // 链接链表节点
        nwe->next = old;
        // 释放可能的重做节点
        if (old->prev) this->free_prev(old);
        old->prev = nwe;
        // 记录新的
        m_pUndoNow = nwe;
    }
    // 返回指针
    m_pUndoStackTop = m_pUndoNow;
    return m_pUndoNow;
}

/// <summary>
/// Redoes this instance.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextDocument::Redo() noexcept {
    //// 测试数据
    //struct {
    //    impl::undo_redo_t   node;
    //    char16_t            buffer[80];
    //}test = { 0 };
    //test.node.pos = 0;
    //std::memcpy(test.node.buffer, u"A機", 3 * sizeof(char16_t));
    //test.node.length = 2;
    //test.node.type = impl::type_insert_sb;

    // 检查是否能重做
    if (this->CanRedo()) {
        const auto prev = reinterpret_cast<impl::undo_redo_t*>(m_pUndoNow);
        m_pUndoNow = prev->prev;
        const auto node = reinterpret_cast<impl::undo_redo_t*>(m_pUndoNow);
        switch (node->type)
        {
        case impl::type_insert_sb:
        case impl::type_insert_db:
            this->insert_text_unre(node);
            break;
        default:
            this->remove_text_unre(node);
            break;
        }
    }
    // 错误音效
    else {
        this->play_beep();
    }
}

/// <summary>
/// Frees the undo stack.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextDocument::free_undo_stack() noexcept {
    // 遍历释放掉节点
    this->free_prev(&m_undoStackBottom);
    // 长度置〇
    m_undoStackLen = 0;
    m_pUndoNow = nullptr;
    m_pUndoStackTop = nullptr;
    m_undoStackBottom.prev = nullptr;
}


/// <summary>
/// Frees the previous.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <returns></returns>
void TextBC::CBCTextDocument::free_prev(void* ptr) noexcept {
    const auto end = reinterpret_cast<impl::undo_redo_t*>(ptr);
    auto node = end->prev;
    // 遍历释放掉节点
    while (node) {
        const auto tmp = node;
        node = node->prev;
        std::free(tmp);
    }
}

/// <summary>
/// Marks the end of operation.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextDocument::MarkEndOfOperation() noexcept {
    const auto node = reinterpret_cast<impl::undo_redo_t*>(m_pUndoNow);
    // 之前没有操作或者OOM
    if (m_pUndoNow != &m_undoStackBottom) node->eoo = true;
}



#ifndef NDEBUG
/// <summary>
/// Debugs the out undo stack.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextDocument::DebugOutUndoStack() noexcept {
    char buffer[1024];
    char utf8_t[1024];
    auto node = reinterpret_cast<impl::undo_redo_t*>(m_pUndoStackTop);
    const auto end = reinterpret_cast<impl::undo_redo_t*>(&m_undoStackBottom);
    this->platform.DebugOutput("==UndoStack==VVV");
    while (node != end) {
        const auto eoo = node->eoo ? 'E' : '-';
        const auto type = [node]() noexcept {
            switch (node->type)
            {
            case impl::type_insert_sb: return "SI";
            case impl::type_insert_db: return "DI";
            case impl::type_remove_sb: return "SR";
            case impl::type_remove_db: return "DR";
            default:
                return "??";
            }
        }();
        const auto arrow = node == m_pUndoNow ? "->" : "  ";
        
        {
            // 强行转换 不考虑非ASCII码
            auto rptr = node->buffer;
            auto wptr = utf8_t;
            while (const auto ch = *rptr) {
                if (ch < 128) {
                    if (ch == '\r') {
                        wptr[0] = '\\';
                        wptr[1] = 'r';
                        ++wptr;
                    }
                    else if (ch == '\n') {
                        wptr[0] = '\\';
                        wptr[1] = 'n';
                        ++wptr;
                    }
                    else *wptr = static_cast<char>(ch);
                }
                else *wptr = '?';
                ++wptr;
                ++rptr;
            }
            *wptr = 0;
        }

        const auto len = std::sprintf(
            buffer, 
            "%s[%c][%s][%3d]: %s", arrow, eoo, type, 
            int(node->pos),
            utf8_t
        );
        assert(len > 0 && len < 1024);
        buffer[len] = 0;
        this->platform.DebugOutput(buffer);
        node = node->next;
    }
    this->platform.DebugOutput("==UndoStack==AAA");
}

#endif

#else
#ifndef NDEBUG

/// <summary>
/// Debugs the out undo stack.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextDocument::DebugOutUndoStack() noexcept {

}
#endif

#endif