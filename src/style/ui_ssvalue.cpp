// lui
#include <style/ui_ssvalue.h>
#include <util/ui_ctordtor.h>
#include <control/ui_control.h>
#include <control/ui_ctrlmeta.h>
#include <core/ui_color_list.h>
#include <core/ui_manager.h>
#include <core/ui_malloc.h>
// css
#include <xul/SimpAC.h>
// c/c++
#include <cstring>

// longui namespace
namespace LongUI {
    // stylesheets block
    struct SSBlock : CUINoMo {
        // new 
        static auto Create(uint32_t len) noexcept->SSBlock*;
        // delete
        void Dispose() noexcept;
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
    // block ptr
    using SSBlockPtr = SSBlock * ;
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
        // property finished
        void property_finished() noexcept;
        // property group finished
        void property_group_finished() noexcept;
    public:
        // ctor
        CUICssStream(SSBlockPtr* ptr) noexcept;
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
        // write ptr
        SSBlockPtr *                m_pBlockWrite;
        // main seletor list
        POD::Vector<SSSelector>     m_mainList;
        // pvalues
        POD::Vector<StrPair>        m_propertyValues;
        // value list
        SSValues                    m_values;
        // now property
        ValueType                   m_nowProperty;
        // state: new_selector
        bool                        m_bNewSelector = true;
        // state: combinator
        Combinator                  m_combinator = Combinator_None;
    };
    /// <summary>
    /// Initializes a new instance of the <see cref="CUICssStream" /> class.
    /// </summary>
    /// <param name="ptr">The PTR.</param>
    CUICssStream::CUICssStream(SSBlockPtr* ptr) noexcept:m_pBlockWrite(ptr) {
        m_nowProperty = {};
        m_mainList.reserve(16);
        m_values.reserve(16);
        m_propertyValues.reserve(32);
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
        const auto selector = &m_mainList.back();
        // 新的组合器
        if (m_combinator != Combinator_None) {
            // STEP 2
            const auto ptr = LongUI::SmallAllocT<SSSelector>(1);
            if (!ptr) return;
            /*
                1. A -> B -> C
                2. A -> B -> C     [_]
                3. _ -- B -> C     [A] -> [B]
                4. Z -> A -> B -> C
            */
            // STEP3 : 复制主键数据
            *ptr = *selector;
            // 清空主键数据
            std::memset(selector, 0, sizeof(SSSelector));
            selector->next = ptr;
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
        assert(m_bNewSelector == false);
        m_bNewSelector = true;
    }
    /// <summary>
    /// Ends the properties.
    /// </summary>
    /// <returns></returns>
    void CUICssStream::end_properties() noexcept {
        this->property_finished();
        this->property_group_finished();
    }
    /// <summary>
    /// Begins the property.
    /// </summary>
    /// <param name="pair">The pair.</param>
    /// <returns></returns>
    void CUICssStream::begin_property(StrPair pair) noexcept {
        this->property_finished();
        const U8View view{ pair.begin(), pair.end() };
        m_nowProperty = LongUI::U8View2ValueType(view);
#ifndef NDEBUG
        if (m_nowProperty == ValueType::Type_Unknown) {
            const U8View property {pair.begin(), pair.end()};
            LUIDebug(Error)
                << "unknown property: "
                << endl;
        }
#endif // !NDEBUG
    }
    /// <summary>
    /// Adds the value.
    /// </summary>
    /// <param name="pair">The pair.</param>
    /// <returns></returns>
    void CUICssStream::add_value(StrPair pair) noexcept {
        m_propertyValues.push_back(pair);
    }
    /// <summary>
    /// Properties the finished.
    /// </summary>
    /// <returns></returns>
    void LongUI::CUICssStream::property_finished() noexcept {
        if (m_nowProperty == ValueType::Type_Unknown) return;
        assert(m_propertyValues.size() && "bad size");
        if (m_propertyValues.empty()) return;
        // 根据属性处理值
        const auto& u8_view = reinterpret_cast<
            POD::Vector<U8View>&>(m_propertyValues);
        LongUI::ValueTypeMakeValue(
            m_nowProperty,
            static_cast<uint32_t>(u8_view.size()),
            &u8_view.front(),
            &m_values
        );
        // 收尾处理
        m_propertyValues.clear();
        m_nowProperty = ValueType::Type_Unknown;
    }
    /// <summary>
    /// Properties the group finished.
    /// </summary>
    /// <returns></returns>
    void CUICssStream::property_group_finished() noexcept {

        // 错误处理

        // 添加一组
        const auto main_len = static_cast<uint32_t>(m_mainList.size());
        if (const auto block = SSBlock::Create(main_len)) {
            // 填写block
            *m_pBlockWrite = block;
            m_pBlockWrite = &block->next;
            // 选择器数量
            block->main_len = main_len;
            // 复制数据
            const auto src = &m_mainList.front();
            std::memcpy(block->main, src, sizeof(SSSelector) * main_len);
            // 值数据交换
            m_values.swap(block->list);
        }
        // 收尾处理
        m_values.clear();
        m_values.reserve(16);
        m_mainList.clear();
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
/// Creates the specified length.
/// </summary>
/// <param name="len">The length.</param>
/// <returns></returns>
auto LongUI::SSBlock::Create(uint32_t len) noexcept -> SSBlock* {
    assert(len > 0 && "bad len");
    const auto size = sizeof(SSBlock) - sizeof(SSSelector) 
        + sizeof(SSSelector) * len;
    if (const auto ptr = LongUI::SmallAlloc(size)) {
        const auto obj = detail::ctor_dtor<SSBlock>::create(ptr, len);
        return obj;
    }
    return nullptr;
}

/// <summary>
/// Disposes the specified .
/// </summary>
/// <returns></returns>
void LongUI::SSBlock::Dispose() noexcept {
    assert(this && "bad ptr");
    detail::ctor_dtor<SSBlock>::delete_obj(this);
    LongUI::SmallFree(this);
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
        old_ptr->Dispose();
    }
}

/// <summary>
/// Deletes the style sheet.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <returns></returns>
void LongUI::DeleteStyleSheet(CUIStyleSheet* ptr) noexcept {
    LongUI::FreeBlocks(reinterpret_cast<SSBlock*>(ptr));
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
        // css解析
        CUICssStream stream{ reinterpret_cast<SSBlockPtr*>(&ptr) };
        stream.Load({ view.begin(), view.end() });
        
        return ptr;
    }
    /// <summary>
    /// Matches the selector.
    /// </summary>
    /// <param name="ctrl">The control.</param>
    /// <param name="selector">The selector.</param>
    /// <returns></returns>
    bool MatchSelector(UIControl& ctrl, const SSSelector& selector) noexcept {
        //auto now_comb = selector.combinator;
        auto now_slct = &selector;
        auto now_ctrl = &ctrl;
        // 匹配基本选择器
        auto match_basic = [](UIControl& c, const SSSelector& s) noexcept {
            // 类型选择器  <->  XUL 类型
            if (s.stype) {
                const auto name = c.GetMetaInfo().element_name;
                // XUL 储存在 一开始的静态文本区
                // 类型选择器中类型是动态申请的
                // 使用strcmp比较
                if (std::strcmp(s.stype, name)) return false;
                int bk = 9;
            }
            // 类名选择器  <->  控件样式类
            if (s.sclass) {
                const auto& classes = c.GetStyleClasses();
                // 都是动态申请的Unique Text, 使用 == 比较
                const auto eitr = classes.end();
                const auto nitr = std::find(classes.begin(), eitr, s.sclass);
                if (eitr == nitr) return false;
                int bk = 9;
            }
            // ID选择器  <->  控件ID
            if (s.sid) {
                // 都是动态申请的Unique Text, 使用 == 比较

                // HINT: 最后一个, 直接return
                return c.GetID() == s.sid;
            }
            return true;
        };
        // 遍历所有选择器
        while (now_slct) {
            // 匹配基本选择器
            match_basic(*now_ctrl, *now_slct);
            // 推进下一层
            now_slct = now_slct->next;
        }
        return true;
    }
}


/// <summary>
/// Matches the style sheet.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="ptr">The PTR.</param>
/// <returns></returns>
void LongUI::MatchStyleSheet(UIControl& ctrl, CUIStyleSheet* ptr) noexcept {
    auto block = reinterpret_cast<SSBlock*>(ptr);
    while (block) {
        // 遍历BLOCK中所有主选择器
        const auto len = block->main_len;
        for (uint32_t i = 0; i != len ; ++i){
            const auto& this_main = block->main[i];
            // 匹配成功: 添加属性
            if (LongUI::MatchSelector(ctrl, this_main)) {
                break;
            }
        }
        // 推进
        block = block->next;
    }
}
