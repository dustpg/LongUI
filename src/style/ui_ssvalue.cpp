// lui
#include <style/ui_ssvalue.h>
#include <core/ui_color_list.h>
#include <core/ui_manager.h>
#include <core/ui_malloc.h>
// css
#include <xul/SimpAC.h>

// longui namespace
namespace LongUI {
    // stylesheets block
    struct SSBlock : CUISmallObject {
        // ctor
        SSBlock(uint32_t len) noexcept;
        // dtor
        ~SSBlock() noexcept;
        // next block
        SSBlock*        next = nullptr;
        // seletor number
        uint32_t        main_len;
        // value list
        SSValues        list;
        // main seletors, at least one
        SSSelector      main[1];
    };
    // free blocks
    void FreeBlocks(SSBlock* head) noexcept;
    // free selector
    void FreeSelector(SSSelector sss[], uint32_t len) noexcept;
    // simpac
    using namespace SimpAC;
    /// <summary>
    /// css-parser
    /// </summary>
    class CUICssStream final : public SimpAC::CACStream {
    public:
        // ctor
        CUICssStream() noexcept;
        // dtor
        ~CUICssStream() noexcept;
    public:
        // add comment
        virtual void add_comment(StrPair) noexcept override;
        // add selector
        virtual void add_selector(BasicSelectors, StrPair) noexcept override;
        // add selector combinator
        virtual void add_selector_combinator(Combinators) noexcept override;
        // add selector comma
        virtual void add_selector_comma() noexcept override;
        // begin properties
        virtual void begin_properties() noexcept override;
        // end properties
        virtual void end_properties() noexcept override;
        // begin property
        virtual void begin_property(StrPair) noexcept override;
        // add value
        virtual void add_value(StrPair) noexcept override;
    private:
        // main seletor list
        POD::Vector<SSSelector>     m_mainList;
        // value list
        SSValues                    m_values;
        // state: new_selector
        bool                        m_bNewSelector = true;
        // state: combinator
        Combinator                  m_combinator = Combinator_None;
    };
    /// <summary>
    /// Initializes a new instance of the <see cref="CUICssStream"/> class.
    /// </summary>
    CUICssStream::CUICssStream() noexcept {
        m_mainList.reserve(16);
        m_values.reserve(32);
    }
    /// <summary>
    /// Finalizes an instance of the <see cref="CUICssStream"/> class.
    /// </summary>
    /// <returns></returns>
    CUICssStream::~CUICssStream() noexcept {
        // 释放意外没有释放的数据
        if (!m_mainList.empty()) {
            const auto ptr = &m_mainList.front();
            const auto len = static_cast<uint32_t>(m_mainList.size());
            LongUI::FreeSelector(ptr, len);
            m_mainList.clear();
        }
    }
    /// <summary>
    /// Adds the comment.
    /// </summary>
    /// <param name="pair">The pair.</param>
    /// <returns></returns>
    void CUICssStream::add_comment(StrPair pair) noexcept {
        int bk = 9;
    }
    /// <summary>
    /// Adds the selector.
    /// </summary>
    /// <param name="bs">The bs.</param>
    /// <param name="pair">The pair.</param>
    /// <returns></returns>
    void CUICssStream::add_selector(BasicSelectors bs, StrPair pair) noexcept {
        if (!m_mainList.is_ok()) return;
        const auto value = U8View{ pair.begin(), pair.end() };
        SSSelector* selector = nullptr;
        // 添加新的选择器
        if (m_bNewSelector) {
            SSSelector empty{};
            m_mainList.push_back(empty);
            m_bNewSelector = false;
            if (!m_mainList.is_ok()) return;
        }
        // 查找最后一个选择器
        assert(m_mainList.empty() == false);
        // 主键最后
        selector = &m_mainList.back();
        // 键表末尾
        while (selector->next) selector = selector->next;
        // 新的组合器
        if (m_combinator != Combinator_None) {
            const auto ptr = LongUI::SmallAllocT<SSSelector>(1);
            if (!ptr) return;
            std::memset(ptr, 0, sizeof(SSSelector));
            selector->next = ptr;
            selector = ptr;
            selector->combinator = m_combinator;
            m_combinator = Combinator_None;
        }
        assert(selector && "cannot be null");
        // 分类讨论
        switch (bs)
        {
        case Selectors_Type:
            assert(!selector->stype && "not null?");
            selector->stype = UIManager.GetUniqueText(value);
            break;
        case Selectors_Class:
            assert(!selector->sclass && "not null?");
            selector->sclass = UIManager.GetUniqueText(value);
            break;
        case Selectors_Id:
            assert(!selector->sid && "not null?");
            selector->sid = UIManager.GetUniqueText(value);
            break;
        case Selectors_PseudoClass:
            assert(!"TODO");
            break;
        }
    }
    /// <summary>
    /// Adds the selector combinator.
    /// </summary>
    /// <param name="cbt">The CBT.</param>
    /// <returns></returns>
    void CUICssStream::add_selector_combinator(Combinators cbt) noexcept {
        assert(m_combinator == Combinator_None);
        m_combinator = static_cast<Combinator>(cbt + 1);
    }
    /// <summary>
    /// Adds the selector comma.
    /// </summary>
    /// <returns></returns>
    void CUICssStream::add_selector_comma() noexcept {
        assert(m_bNewSelector == false);
        m_bNewSelector = true;
    }
    /// <summary>
    /// Begins the properties.
    /// </summary>
    /// <returns></returns>
    void CUICssStream::begin_properties() noexcept {
        int bk = 9;
    }
    /// <summary>
    /// Ends the properties.
    /// </summary>
    /// <returns></returns>
    void CUICssStream::end_properties() noexcept {
        int bk = 9;
    }
    /// <summary>
    /// Begins the property.
    /// </summary>
    /// <param name="pair">The pair.</param>
    /// <returns></returns>
    void CUICssStream::begin_property(StrPair pair) noexcept {
        int bk = 9;
    }
    /// <summary>
    /// Adds the value.
    /// </summary>
    /// <param name="pair">The pair.</param>
    /// <returns></returns>
    void CUICssStream::add_value(StrPair pair) noexcept {
        int bk = 9;
    }
}


/// <summary>
/// Initializes a new instance of the <see cref="SSBlock" /> struct.
/// </summary>
/// <param name="len">The length.</param>
LongUI::SSBlock::SSBlock(uint32_t len) noexcept: main_len(len) {
    std::memset(&this->main, 0, sizeof(this->main) * len);
}

/// <summary>
/// Finalizes an instance of the <see cref="SSBlock"/> class.
/// </summary>
/// <returns></returns>
LongUI::SSBlock::~SSBlock() noexcept {
    LongUI::FreeSelector(this->main, this->main_len);
}

/// <summary>
/// Frees the blocks.
/// </summary>
/// <param name="head">The head.</param>
/// <returns></returns>
void LongUI::FreeBlocks(SSBlock* head) noexcept {
    // 遍历链表
    while (head) {
        const auto old_ptr = head;
        head = head->next;
        delete old_ptr;
    }
}

/// <summary>
/// Frees the selector.
/// </summary>
/// <param name="sss">The SSS.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
void LongUI::FreeSelector(SSSelector sss[], uint32_t len) noexcept {
    const auto this_len = len;
    auto this_main = sss;
    // 遍历主键
    for (uint32_t i = 0; i != this_len; ++i) {
        auto selector = this_main->next;
        ++this_main;
        // 释放链表
        while (selector) {
            const auto old_ptr = selector;
            selector = selector->next;
            LongUI::SmallFree(old_ptr);
        }
    }
}



namespace LongUI {
    /// <summary>
    /// Makes the style sheet.
    /// </summary>
    /// <param name="view">The view.</param>
    /// <param name="ptr">The PTR.</param>
    /// <returns></returns>
    auto MakeStyleSheet(U8View view, SSPtr ptr) noexcept->SSPtr {
#ifndef NDEBUG
        const char* dbg = view.begin();
        dbg = nullptr;
#endif
        auto& ptr2write = ptr;
        // css解析
        CUICssStream stream;
        stream.Load({ view.begin(), view.end() });
        
        return ptr;
    }
}