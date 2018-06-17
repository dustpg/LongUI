#include "bc_txtdoc.h"
#include <cassert>
#include <new>
#ifndef NDEBUG
#include <cstdio>
#endif


/// <summary>
/// Initializes a new instance of the <see cref="CBCTextCell"/> class.
/// </summary>
/// <param name="doc">The document.</param>
TextBC::CBCTextCell::CBCTextCell(CBCTextDocument& doc) noexcept:
m_document(doc), m_bCRLF(doc.IsCRLF()){
    sizeof(*this);
    m_bDirty = true;
    m_bBeginOfLine = false;
    m_bEndOfLine = false;
#ifndef NDEBUG
    m_bInLayout = false;
#endif
}


/// <summary>
/// Finalizes an instance of the <see cref="CBCTextCell"/> class.
/// </summary>
/// <returns></returns>
TextBC::CBCTextCell::~CBCTextCell() noexcept {
}


/// <summary>
/// Deletes the this.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextCell::DeleteNode() noexcept {
    assert(this);
    delete this;
}

/// <summary>
/// Finds the begin same line.
/// </summary>
/// <returns></returns>
auto TextBC::CBCTextCell::FindBeginSameLine() noexcept -> CBCTextCell* {
    // 标记了BOL的
    auto node = this;
    while (true) {
        if (node->IsBOL()) break;
        node = static_cast<decltype(node)>(node->prev);
    }
    return node;
}

/// <summary>
/// Finds the end same line.
/// </summary>
/// <returns></returns>
auto TextBC::CBCTextCell::FindEndSameLine() noexcept -> CBCTextCell* {
    // 标记了EOL的或者最后一个
    auto node = this;
    while (true) {
        if (node->IsEOL() || node->IsLastCell()) break;
        node = static_cast<decltype(node)>(node->next);
    }
    return node;
}

#ifndef NDEBUG
/// <summary>
/// Bcs the assert move eol.
/// </summary>
/// <param name="cell">The cell.</param>
/// <remarks>
/// EOL = end of line
/// </remarks>
/// <returns></returns>
void bc_assert_move_eol(TextBC::CBCTextCell& cell) noexcept {
    //assert(!"UNTESTED");
    assert(!cell.IsLastCell());
    const auto next = static_cast<TextBC::CBCTextCell*>(cell.next);
    assert(cell.IsEOL() || next->IsLastCell());
}
#endif


/// <summary>
/// News the after this.
/// </summary>
/// <returns></returns>
auto TextBC::CBCTextCell::NewAfterThis() noexcept -> CBCTextCell* {
    // 创建新的对象
    if (auto node = new(std::nothrow) CBCTextCell{ m_document }) {
        // 添加到后面
        this->next->prev = node;
        node->prev = this;
        node->next = this->next;
        this->next = node;
        return node;
    }
    // 内存不足
    return nullptr;
}

/// <summary>
/// Sleeps this instance.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextCell::Sleep() noexcept {
    assert(m_pContent);
    auto& platform = m_document.platform;
    platform.DeleteContent(*m_pContent);
    m_pContent = nullptr;
}


/// <summary>
/// Awakes this instance.
/// </summary>
/// <param name="str">The string.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
void TextBC::CBCTextCell::Awake(const char16_t* str, uint32_t len) noexcept {
    assert(m_pContent == nullptr);
    auto& platform = m_document.platform;
    auto& last = *m_pContent;
    m_pContent = platform.CreateContent(str, len, std::move(last));
    /* 
        TODO: 两种策略
        m_bDirty:
          是否需要重新布局, 获取新的测量数据
        m_pContent:
          是否需要创建内容, 重新布局, 但是不需要重新获取数据
    */
}

/// <summary>
/// Removes the text.
/// </summary>
/// <param name="range">The range.</param>
/// <returns>return <c>true</c> if CR/LF deleted</returns>
bool TextBC::CBCTextCell::RemoveText(Range range) noexcept {
    // 不允许删除0长度
    assert(range.len && "bad arg");
    assert(this->GetStringLen() > range.pos);
    assert(this->GetStringLen() >= range.pos + range.len);
    // 返回码
    bool rv = false;
    // 回车换行做为整体看待
    if (range.pos + range.len == this->GetStringLen()) {
        rv = this->IsEOL();
        // 只要把最后一个字符删了, 不管任何情况都不是EOL
        this->clear_eol();
        const auto bol = this->IsBOL();
        // 下一节点会连接到本行后面就清除BOL标记
        // -- 算法:
        //    1. 完全删除则继承本节点的BOL
        //    2. 否则清除BOL
        const auto node = this->next;
        // 前提是, 不是结尾的话
        if (node->next) {
            if (range.pos == 0) {
                static_cast<CBCTextCell*>(node)->FollowBOL(*this);
            }
            else {
                static_cast<CBCTextCell*>(node)->clear_bol();
            }
        }
    }
    // 直接删完
    if (range.pos == 0 && range.len == this->GetStringLen()) {
        // 不删最后一行的唯一节点
        if (this->IsBOL() && (/*this->IsEOL() || */this->IsLastCell())) {
            //int bk = 9;
        }
        // 直接删除节点
        else {
#ifndef NDEBUG
            char buf[128];
            std::sprintf(buf, "rv = %s", rv ? "true" : "false");
            m_document.platform.DebugOutput(buf);
#endif
            this->RemoveFromListOnly();
            this->DeleteNode();
            return rv;
        }
    }
    // 仅删除文本
    this->RemoveTextOnly(range);
    return rv;
}


/// <summary>
/// Inserts the text.
/// </summary>
/// <param name="">The .</param>
/// <param name="view">The view.</param>
/// <returns></returns>
uint32_t TextBC::CBCTextCell::InsertText(uint32_t pos, U16View view) noexcept {
    // 不允许插入空字符
    assert(view.second != view.first);
#if 1
    // 最后一个是 \r?
    if (view.second[-1] == '\r') {
        --view.second;
        // 只有一个\r就什么也不做
        if (view.second == view.first) return 0;
    }
#endif
    // 插入长度
    const uint32_t len = static_cast<uint32_t>(view.second - view.first);
    // 直接插入
    m_text.insert(pos, view.first, len);
    // 标记为脏
    m_bDirty = true;
    // TODO: 字符个数 != 字符串长
    m_cCharCount += static_cast<decltype(m_cCharCount)>(len);
    // 返回插入字符串
    return len;
}

/// <summary>
/// Removes the text only.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void TextBC::CBCTextCell::RemoveTextOnly(Range range) noexcept {
    assert(range.len + range.pos <= this->GetStringLen());
    // 直接删除
    m_text.erase(range.pos, range.len);
    // 标记为脏
    m_bDirty = true;
    // TODO: 字符个数 != 字符串长
    m_cCharCount -= range.len;
}

/// <summary>
/// Begins the layout.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextCell::BeginLayout() noexcept {
#ifndef NDEBUG
    // 递归检测
    assert(m_bInLayout == false);
    m_bInLayout = true;
    // 不是最后一块不允许为空
    if (!this->IsLastCell() && this->GetStringLen() == 0) {
        assert(!"BAD LEN");
    }
    // 居然不脏?!
    assert(m_bDirty && "not ditry?");
#endif
    // 设置文本
    //auto& platform = m_document.platform;
    const uint32_t len = this->GetStrLenNoEOL();
    // 移除有效的文本
    if (m_pContent) this->Sleep();
    // 密码用缓存
    constexpr uint32_t BUFLEN = (TEXT_CELL_NICE_MAX + 2);
    assert(GetStringLen() < BUFLEN);
    char16_t buffer[BUFLEN];
    
    // 只用
    const auto* strptr = m_text.c_str();
    // 检查密码
    if (m_document.GetFlag() & CBCTextDocument::Flag_UsePassword) {
        // 重定向指针
        strptr = buffer;
        // 填充密码
        const auto ch = m_document.GetPasswordChar();
#if 0
        // 双字 16x2
        if (ch > 0xFFFF) {
            // From http://unicode.org/faq/utf_bom.html#35
            const auto ch1 = static_cast<char16_t>(0xD800 + (ch >> 10) - (0x10000 >> 10));
            const auto ch2 = static_cast<char16_t>(0xDC00 + (ch & 0x3FF));
            // 遍历即可
            for (uint32_t i = 0; i != len * 2; i += 2) {
                buffer[i + 0] = ch1;
                buffer[i + 1] = ch2;
            }
        }
        // 单字 16x1
        else {
            const auto ch16 = static_cast<char16_t>(ch);
            for (uint32_t i = 0; i != len; ++i) buffer[i] = ch16;
        }
#else
        // 直接复制
        for (uint32_t i = 0; i != len; ++i) buffer[i] = ch;
#endif
    }
    // 重新苏醒
    this->Awake(strptr, len);
}

/// <summary>
/// Ends the layout.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextCell::EndLayout() noexcept {
#ifndef NDEBUG
    // 递归检测
    assert(m_bInLayout == true);
    m_bInLayout = false;
    // 有消息检查
    assert(m_bDirty);
    assert(m_pContent);
#endif
    auto& platform = m_document.platform;
    const auto get_size = IBCTextPlatform::Event_GetSize;
    const auto get_base = IBCTextPlatform::Event_GetBaseline;
    // 获取大小
    platform.ContentEvent(*m_pContent, get_size, &m_size);
    // 行结尾
    //if (this->IsEOL()) m_size.width += m_size.height * 0.5f;
    // 获取基线偏移
    platform.ContentEvent(*m_pContent, get_base, &m_fBaseline);
    // 标记已经干净
    m_bDirty = false;
}


/// <summary>
/// Marks as eol.
/// </summary>
/// <returns></returns>
void TextBC::CBCTextCell::MarkAsEOL() noexcept {
    assert(IsEOL() == false && "already EOL");
    m_bEndOfLine = true;
    const uint32_t crlf = m_bCRLF;
    // 获取长度
    const auto index = this->GetStringLen();
    // 增加字符数量
    m_cCharCount += crlf + 1;
    // 直接resize
    m_text.resize(index + 2);
    // 添加换行符号
    if (crlf) m_text[index] = '\r';
    m_text[index + crlf] = '\n';
}
