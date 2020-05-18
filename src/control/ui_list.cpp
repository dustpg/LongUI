#include <control/ui_listbox.h>
#include <control/ui_listitem.h>
#include <control/ui_listcol.h>
#include <control/ui_listcols.h>
#include <control/ui_listhead.h>
#include <control/ui_listheader.h>

#include <core/ui_ctrlmeta.h>
#include <control/ui_image.h>
#include <control/ui_label.h>
#include <control/ui_boxlayout.h>
#include <input/ui_kminput.h>
#include <constexpr/const_bkdr.h>
#include <debugger/ui_debug.h>

#include <core/ui_unsafe.h>
#include "../private/ui_private_control.h"

#include <algorithm>

// ui namespace
namespace LongUI {
    // UIListBox类 元信息
    LUI_CONTROL_META_INFO(UIListBox, "listbox");
    /*
    1. 每行高度(名词)一致, 选取最大的一个作为基准点
    2. rows属性影响minsize


    <listbox>

      <listcols>
        <listcol/>
        <listcol/>
        <!--splitter/-->
        <listcol/>
      </listcols>

      <listhead>
        <listheader label="Name" />
        <listheader label="Sex" />
        <listheader label="Color" />
      </listhead>

      <listitem>
        <label value="Pearl" />
        <label value="Female" />
        <label value="Gray" />
      </listitem>
      <listitem>
        <label value="Aramis" />
        <label value="Male" />
        <label value="Black" />
      </listitem>
      <listitem>
        <label value="Yakima" />
        <label value="Male" />
        <label value="Holstein" />
      </listitem>
    </listbox>

    */
}


/// <summary>
/// Finalizes an instance of the <see cref="UIListBox"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIListBox::~UIListBox() noexcept {
    // 需要提前释放
    m_state.destructing = true;
}


/// <summary>
/// Initializes a new instance of the <see cref="UIListBox" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIListBox::UIListBox(UIControl* parent, const MetaControl& meta) noexcept
    : Super(impl::ctor_lock(parent), meta),
    m_oListboxBody(this) {
    // 焦点允许
    //m_state.focusable = true;
    // 默认为列表框
    m_oStyle.appearance = Appearance_ListBox;
    // 默认样式
    m_oBox.border = { 1.f, 1.f, 1.f, 1.f };
    m_oBox.margin = { 4.f, 2.f, 4.f, 2.f };
    // 至少一个
    m_selected.reserve(1);
    // TODO: OOM处理

    // 创建Body控件
    m_oListboxBody.SetAutoOverflow();
#ifndef NDEBUG
    m_oListboxBody.SetDebugName("listbox::listboxbody");
#endif // !NDEBUG

    // 构造锁
    impl::ctor_unlock();
}


/// <summary>
/// Sets the size of the line.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::UIListBox::SetLineSize(Size2F size) noexcept {
    if (m_oListboxBody.line_size.height != size.height) {
        m_oListboxBody.line_size = size;
    }
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIListBox::Update() noexcept {
    // 其他的交给父类处理
    Super::Update();
    // 要求重新布局
    if (this->is_need_relayout()) {
        // 不脏了
        m_state.dirty = false;
        // 重新布局
        this->relayout();
    }
}


/// <summary>
/// Relayouts this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIListBox::relayout() noexcept {
    // 获取左上角位置
    auto lt = this->GetBox().GetContentPos();
    auto ctsize = this->GetBox().GetContentSize();
    const auto xofffset = lt.x;

    // 先布局列对象(有面积!)
    if (m_pCols) {
        m_pCols->SetPos(lt);
        this->resize_child(*m_pCols, ctsize);
        // 下次再来
        if (m_pCols->WillRelayout())
            return this->NeedRelayout();
    }
    // 拥有头对象?
    if (m_pHead) {
        m_pHead->SetPos(lt);
        const auto height = m_pHead->GetMinSize().height;
        lt.y += height;
        ctsize.height -= height;
        this->resize_child(*m_pHead, { ctsize.width, height });
    }
    // Body 位置大小
    m_oListboxBody.SetPos(lt);
    this->resize_child(m_oListboxBody, ctsize);

    // == Item ==
    float item_offset = 0.f;
    // 先获取获取行高度
    const auto line_height = [this]() noexcept {
        float height = 0.f;
        for (auto* child : m_list) 
            height = std::max(height, child->GetMinSize().height);
        return height;
    }();
    // 遍历有效子节点
    for (auto* child : m_list) {
        child->SetPos({xofffset, item_offset });
        this->resize_child(*child, { ctsize.width,  line_height });
        item_offset += line_height;
    }
    // TODO: 宽度
    m_oListboxBody.ForceUpdateScrollSize({ ctsize.width, item_offset });
}


/// <summary>
/// Refreshes the index of the item.
/// </summary>
/// <returns></returns>
void LongUI::UIListBox::refresh_item_index() noexcept {
    uint32_t index = 0;
    for (auto child : m_list) {
        UIControlPrivate::SetParentData(*child, index);
        ++index;
    }
    this->clear_need_refresh_index();
}

/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIListBox::add_attribute(uint32_t key, U8View value) noexcept {
    constexpr auto BKDR_SELTYPE     = 0xdee1c438_ui32;
    constexpr auto BKDR_ROWS        = 0x0f63dd45_ui32;
    switch (key)
    {
    case BKDR_SELTYPE:
        // seltype      : 选择类型
        m_seltype = AttrParser::Seltype(value);
        break;
    case BKDR_ROWS:
        // rows         : 显示列数
        m_displayRow = value.ToInt32();
        break;
    default:
        // 其他交给父类处理
        return Super::add_attribute(key, value);
    }
}


/// <summary>
/// Adds the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UIListBox::add_child(UIControl& child) noexcept {
    // 是ListItem
    if (uisafe_cast<UIListItem>(&child)) {
        const auto ptr = static_cast<UIListItem*>(&child);
        m_list.push_back(ptr);
        this->mark_need_refresh_index();
        return UIControlPrivate::CallAddChild(m_oListboxBody, child);
    }
    // 是ListCols
    else if (uisafe_cast<UIListCols>(&child)) {
        m_pCols = static_cast<UIListCols*>(&child);
        Super::add_child(child);
        // 交换body cols以提高优先级处理鼠标消息
        this->SwapNode(child, m_oListboxBody);
        // 正在添加节点, 没必要用SwapChildren?
        //this->SwapChildren(child, *m_pListboxBody);
        return;
    }
    // 是ListHead
    else if (uisafe_cast<UIListHead>(&child)) {
        m_pHead = static_cast<UIListHead*>(&child);
    }
    // 调用父类接口
    Super::add_child(child);
}


/// <summary>
/// Inserts the item.
/// </summary>
/// <param name="index">The index.</param>
/// <param name="">The .</param>
/// <returns></returns>
auto LongUI::UIListBox::InsertItem(uint32_t index, const CUIString &) noexcept -> UIListItem* {
    assert(!"NOT IMPL");
    return nullptr;
}


/// <summary>
/// Clears the select.
/// </summary>
/// <returns></returns>
void LongUI::UIListBox::ClearAllSelected() noexcept {
    for (auto* item : m_selected) {
        item->StartAnimation({ StyleStateType::Type_Selected, false });
    }
    m_selected.clear();
}

/// <summary>
/// Clears the selected.
/// </summary>
/// <param name="item">The item.</param>
/// <returns></returns>
void LongUI::UIListBox::ClearSelected(UIListItem& item) noexcept {
    const auto enditr = m_selected.end();
    const auto itr = std::find(m_selected.begin(), enditr, &item);
    m_selected.erase(itr);
    item.StartAnimation({ StyleStateType::Type_Selected, false });
}

/// <summary>
/// Selects the item.
/// </summary>
/// <param name="item">The item.</param>
/// <param name="add">if set to <c>true</c> [add].</param>
/// <returns></returns>
void LongUI::UIListBox::SelectItem(UIListItem& item, bool exadd) noexcept {
    // 在表内额外添加就算了
    if (item.IsSelected() && exadd) return;
    // 记录上次操作对象
    m_pLastOp = &item;
    // 清空?
    if (!exadd || !this->IsMultiple()) this->ClearAllSelected();
    // 写入表内
    this->select_item(item);
}

PCN_NOINLINE
/// <summary>
/// Selects the item.
/// </summary>
/// <param name="item">The item.</param>
/// <returns></returns>
void LongUI::UIListBox::select_item(UIListItem& item) noexcept {
    // 写入表内
    // TODO: 链表实现m_selected
    m_selected.push_back(&item);
    item.StartAnimation({ StyleStateType::Type_Selected, true });
}


/// <summary>
/// Selects to.
/// </summary>
/// <param name="item">The item.</param>
/// <returns></returns>
void LongUI::UIListBox::SelectTo(UIListItem& item) noexcept {
    assert(!m_list.empty() && "WTF?!");
    // 单选?
    if (!this->IsMultiple()) return this->SelectItem(item, false);
    // 起点
    const auto start = m_pLastOp ? m_pLastOp : m_list[0];
    const auto beginitr = m_list.begin();
    const auto enditr = m_list.end();
    // XXX: 直接暴力搜索(考虑使用m_uData4Parent?)
    const auto startitr = std::find(beginitr, enditr, start);
    const auto itemitr = std::find(beginitr, enditr, &item);
    assert(startitr != enditr);
    assert(itemitr != enditr);
    // 清楚
    this->ClearAllSelected();
    // 开始遍历
    const auto selbegin = startitr < itemitr ? startitr : itemitr;
    const auto selend = (startitr < itemitr ? itemitr : startitr) + 1;
    for (auto itr = selbegin; itr != selend; ++itr) {
        const auto ctrl = *itr;
        this->select_item(*ctrl);
    }
}

PCN_NOINLINE
/// <summary>
/// Gets the index of the item.
/// </summary>
/// <param name="item">The item.</param>
/// <returns></returns>
auto LongUI::UIListBox::GetItemIndex(const UIListItem& item) noexcept -> uint32_t {
    if (this->need_refresh_index()) this->refresh_item_index();
    return UIControlPrivate::GetParentData(item);
}

/// <summary>
/// Items the removed.
/// </summary>
/// <param name="item">The item.</param>
/// <returns></returns>
void LongUI::UIListBox::ItemRemoved(UIListItem& item) noexcept {
    // 删除中直接返回?
    if (m_state.destructing) return;
    // 0. 删除上次操作对象
    if (m_pLastOp == &item) m_pLastOp = nullptr;
    // 1. 删除在表数据
    {
        const auto enditr = m_list.end();
        const auto itr = std::find(m_list.begin(), enditr, &item);
        // !!必须在
        assert(itr != enditr);
        m_list.erase(itr);
    }
    // 2. 删除在选数据
    {
        const auto enditr = m_selected.end();
        const auto itr = std::find(m_selected.begin(), enditr, &item);
#ifndef NDEBUG
        // 自己写的允许擦除end迭代器
        m_selected.erase(m_selected.end());
#endif
        //if (itr != enditr)
        m_selected.erase(itr);
    }
    // 3. 标记需要修改索引
    this->mark_need_refresh_index();
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIListBox::DoEvent(UIControl* sender, 
    const EventArg& e) noexcept -> EventAccept {
    switch (e.nevent)
    {
    case NoticeEvent::Event_RefreshBoxMinSize:
        // 先更新cols的最小大小以确定宽度
        this->refresh_cols_minsize();
        // 更新最小大小
        //Super::DoEvent(sender, e);
        this->refresh_minsize();
        return Event_Accept;
    default:
        // 其他事件
        return Super::DoEvent(sender, e);
    }
}

/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
//auto LongUI::UIListBox::DoMouseEvent(const MouseEventArg & e) noexcept -> EventAccept {
//    switch (e.type)
//    {
//    case LongUI::MouseEvent::Event_MouseWheelV:
//        //LUIDebug(Hint) LUI_FRAMEID << e.type << endl;
//        //return Event_Accept;
//        break;
//    case LongUI::MouseEvent::Event_MouseWheelH:
//        LUIDebug(Hint) LUI_FRAMEID << e.type << endl;
//        break;
//    case LongUI::MouseEvent::Event_MouseEnter:
//        break;
//    case LongUI::MouseEvent::Event_MouseLeave:
//        break;
//    case LongUI::MouseEvent::Event_MouseMove:
//        break;
//    case LongUI::MouseEvent::Event_LListItemDown:
//    {
//        //int* ptr = nullptr;
//        //ptr[12] = 0;
//    }
//        break;
//    case LongUI::MouseEvent::Event_LListItemUp:
//        break;
//    case LongUI::MouseEvent::Event_RListItemDown:
//        break;
//    case LongUI::MouseEvent::Event_RListItemUp:
//        break;
//    case LongUI::MouseEvent::Event_MListItemDown:
//        break;
//    case LongUI::MouseEvent::Event_MListItemUp:
//        break;
//    }
//    return Super::DoMouseEvent(e);
//}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
//void LongUI::UIListBox::Render() const noexcept {
//    return Super::Render();
//}

/// <summary>
/// update this instance.
/// </summary>
/// <returns></returns>
//void LongUI::UIListBox::Update() noexcept {
//    return Super::Update();
//}

/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
//auto LongUI::UIListBox::Recreate() noexcept -> Result {
//    return Super::Recreate();
//}


/// <summary>
/// Refreshes the minsize.
/// </summary>
/// <returns></returns>
void LongUI::UIListBox::refresh_cols_minsize() noexcept {
    // 根本没有列信息
    if (!m_pCols) return;
    // 内存不足?
    m_minwidth.resize(m_pCols->GetCount());
    if (!m_minwidth.is_ok()) return;
    // 清空
    std::memset(m_minwidth.data(), 0, m_pCols->GetCount() * sizeof(float));
    const auto end = m_minwidth.end();
    // 遍历数据
    for (auto ctrl : m_list) {
        auto itr = m_minwidth.begin();
        for (auto& child : (*ctrl)) {
            if (itr == end) break;
            const auto ms = child.GetMinSize();
            float& minwidth = *itr;
            minwidth = std::max(minwidth, ms.width);
            ++itr;
        }
    }
    // 设置
    auto itr = m_minwidth.begin();
    for (auto& child : (*m_pCols)) {
        UIControlPrivate::SetBoxMinWidth(child, *itr);
        ++itr;
    }
    m_pCols->DoEvent(this, { NoticeEvent::Event_RefreshBoxMinSize });
}


/// <summary>
/// Refreshes the minsize.
/// </summary>
/// <returns></returns>
void LongUI::UIListBox::refresh_minsize() noexcept {
    Size2F msize = { 0.f };
    // 先确定Head
    if (m_pHead) msize = m_pHead->GetBox().minsize;
    // 再确定Body
    const auto line_height = m_oListboxBody.line_size.height;
    msize.height += m_displayRow * line_height;
    // 设置大小
    this->set_contect_minsize(msize);
}

// ----------------------------------------------------------------------------
// --------------------           List Item           -------------------------
// ----------------------------------------------------------------------------

// ui namespace
namespace LongUI {
    // UIListItem类 元信息
    LUI_CONTROL_META_INFO(UIListItem, "listitem");
}

/// <summary>
/// Gets the text.
/// </summary>
/// <returns></returns>
auto LongUI::UIListItem::GetText() const noexcept -> const char16_t* {
    return m_oLabel.GetText();
}

/// <summary>
/// Gets the index.
/// </summary>
/// <returns></returns>
auto LongUI::UIListItem::GetIndex() const noexcept -> uint32_t {
    if (const auto list = m_pListBox) {
        return list->GetItemIndex(*this);
    }
    return 0;
}

/// <summary>
/// Gets the text string.
/// </summary>
/// <returns></returns>
auto LongUI::UIListItem::GetTextString() const noexcept -> const CUIString&{
    return m_oLabel.GetTextString();
}


/// <summary>
/// Initializes a new instance of the <see cref="UIListItem" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIListItem::UIListItem(UIControl* parent, const MetaControl& meta) noexcept
    : Super(impl::ctor_lock(parent), meta),
    m_oImage(nullptr), m_oLabel(nullptr) {
    //m_state.focusable = true;
    // 原子性, 子控件为本控件的组成部分
    //m_state.atomicity = true;
    // 水平布局
    m_state.orient = Orient_Horizontal;
    // 列表项目
    m_oStyle.appearance = Appearance_ListItem;
    // 私有实现
    //UIControlPrivate::SetFocusable(image, false);
    //UIControlPrivate::SetFocusable(label, false);
#ifndef NDEBUG
    m_oImage.name_dbg = "listitem::image";
    m_oLabel.name_dbg = "listitem::label";
    assert(m_oImage.IsFocusable() == false);
    assert(m_oLabel.IsFocusable() == false);
#endif
    // 构造锁
    impl::ctor_unlock();
}


/// <summary>
/// Finalizes an instance of the <see cref="UIListItem"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIListItem::~UIListItem() noexcept {
    // 存在提前释放子控件, 需要标记"在析构中"
    m_state.destructing = true;
    // 这里不用输出调试信息
#ifndef NDEBUG
    if (m_pParent)
#endif
    // 操作上级标记自己被删除
    if (const auto list = m_pListBox) {
        list->ItemRemoved(*this);
    }
}

/// <summary>
/// Adds the private child.
/// </summary>
/// <returns></returns>
void LongUI::UIListItem::add_private_child() noexcept {
    if (!m_oImage.GetParent()) {
        m_oImage.SetParent(*this);
        m_oLabel.SetParent(*this);
    }
}


/// <summary>
/// Relayouts this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIListItem::relayout() noexcept {
    // 检查是不是使用的默认控件
    auto& maybe = m_oImage;
    // 根据列表的布局
    if (maybe.GetParent() != this) {
        // 获取UIListBox
        if (const auto list = m_pListBox) {
            if (const auto cols = list->GetCols()) {
                return cols->MatchLayout(*this);
            }
            // TODO: 没有cols的场合?
        }
    }
    // 默认的水平布局
    this->relayout_h();
}


/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UIListItem::SetText(const CUIString& text) noexcept {
    this->SetText(CUIString{ text });
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UIListItem::SetText(U16View text) noexcept {
    this->SetText(CUIString(text));
}


/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIListItem::DoMouseEvent(const MouseEventArg& e) noexcept -> EventAccept {
    switch (e.type)
    {
    case LongUI::MouseEvent::Event_LButtonDown:
        // 存在ListBox
        if (const auto list = m_pListBox) {
            // 判断SHIFT
            if (CUIInputKM::GetKeyState(CUIInputKM::KB_SHIFT)) {
                list->SelectTo(*this);
            }
            else {
                // 判断CTRL键
                const auto ctrl = CUIInputKM::GetKeyState(CUIInputKM::KB_CONTROL);
                if (ctrl && list->IsMultiple() && this->IsSelected()) {
                    // 取消选择
                    list->ClearSelected(*this);
                }
                // 添加选择
                else list->SelectItem(*this, ctrl);
            }
        }
        [[fallthrough]];
    default:
        return Super::DoMouseEvent(e);
    }
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UIListItem::SetText(CUIString&& text) noexcept {
    this->add_private_child();
    if (m_oLabel.SetText(std::move(text))) {
#ifdef LUI_ACCESSIBLE
        //LongUI::Accessible(m_pAccessible, Callback_PropertyChanged);
#endif
    }
}


/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIListItem::Update() noexcept {
    // 获取listbox
    if (m_state.parent_changed) {
        UIListBox* listbox = nullptr;
        if (m_pParent) {
            const auto ppp = m_pParent->GetParent();
            listbox = longui_cast<UIListBox*>(ppp);
        }
        m_pListBox = listbox;
    }
    // 父类处理
    Super::Update();
}


/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIListItem::DoEvent(UIControl * sender,
    const EventArg & e) noexcept -> EventAccept {
    switch (e.nevent)
    {
    case NoticeEvent::Event_RefreshBoxMinSize:
        Super::DoEvent(sender, e);
        if (const auto list = m_pListBox)
            list->SetLineSize(this->GetBox().minsize);
        return Event_Accept;
    case NoticeEvent::Event_Initialize:
        // 没子控件
        if (!this->GetCount()) {
            // TODO: 没有文本时候的处理
            m_oLabel.SetAsDefaultMinsize();
            this->add_private_child();
        }
        break;
    }
    // 基类处理
    return Super::DoEvent(sender, e);
}


/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIListItem::add_attribute(uint32_t key, U8View value) noexcept {
    // 新增属性列表
    constexpr auto BKDR_VALUE = 0x246df521_ui32;
    // 分类讨论
    switch (key)
    {
    case "label"_bkdr:
        // 传递给子控件
        Unsafe::AddAttrUninited(m_oLabel, BKDR_VALUE, value);
        break;
    default:
        // 其他情况, 交给基类处理
        return Super::add_attribute(key, value);
    }
}

// ----------------------------------------------------------------------------
// --------------------           List Cols           -------------------------
// ----------------------------------------------------------------------------

// ui namespace
namespace LongUI {
    // UIListCol类 元信息
    LUI_CONTROL_META_INFO(UIListCols, "listcols");
}

/// <summary>
/// Initializes a new instance of the <see cref="UIListItem" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIListCols::UIListCols(UIControl* parent, const MetaControl& meta) noexcept
    : Super(parent, meta) {
    // 水平布局
    m_state.orient = Orient_Horizontal;
    // 构造锁
    //impl::ctor_unlock();
}

/// <summary>
/// Finalizes an instance of the <see cref="UIListCols"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIListCols::~UIListCols() noexcept {
    // TODO: 移除box节点cols对象
}

/// <summary>
/// Adds the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
//void LongUI::UIListCols::add_child(UIControl& child) noexcept {
//    // 是列对象?
//    if (uisafe_cast<UIListCol>(&child)) {
//        m_list.push_back(static_cast<UIListCol*>(&child));
//    }
//    Super::add_child(child);
//}

/// <summary>
/// Matches the layout.
/// </summary>
/// <param name="container">The container.</param>
/// <returns></returns>
void LongUI::UIListCols::MatchLayout(UIControl& container) noexcept {
    const auto height = container.GetBox().GetContentSize().height;
    auto itr = container.begin();
    const auto end = container.end();
    // 遍历cols
    for (auto& col : (*this)) {
        // 必须是col
        if (!uisafe_cast<UIListCol>(&col)) continue;
        // 下面没了
        if (itr == end) break;
        // 设置
        auto& cchild = *itr;
        const auto width = col.GetSize().width;
        UIControl::resize_child(cchild, { width, height });
        cchild.SetPos(col.GetPos());
        // 递进
        ++itr;
    }
}


// ----------------------------------------------------------------------------
// --------------------           List Col            -------------------------
// ----------------------------------------------------------------------------


// ui namespace
namespace LongUI {
    // UIListCol类 元信息
    LUI_CONTROL_META_INFO(UIListCol, "listcol");
}


/// <summary>
/// Finalizes an instance of the <see cref="UIListCol"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIListCol::~UIListCol() noexcept {
}


/// <summary>
/// Initializes a new instance of the <see cref="UIListCol" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIListCol::UIListCol(UIControl* parent, const MetaControl& meta) noexcept
    : Super(parent, meta) {
    // 构造锁
    //impl::ctor_unlock();
}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIListCol::Render() const noexcept {

}



// ----------------------------------------------------------------------------
// --------------------          List Head            -------------------------
// ----------------------------------------------------------------------------

// ui namespace
namespace LongUI {
    // UIListHead类 元信息
    LUI_CONTROL_META_INFO(UIListHead, "listhead");
}

/// <summary>
/// Initializes a new instance of the <see cref="UIListItem" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIListHead::UIListHead(UIControl* parent, const MetaControl& meta) noexcept
    : Super(parent, meta) {
    // 水平布局
    m_state.orient = Orient_Horizontal;
    // 构造锁
    //impl::ctor_unlock();
}


/// <summary>
/// Finalizes an instance of the <see cref="UIListHead"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIListHead::~UIListHead() noexcept {
    // TODO: 移除box节点head对象

}


/// <summary>
/// Relayouts this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIListHead::relayout() noexcept {
    // 获取UIListBox
    if (const auto list = longui_cast<UIListBox*>(m_pParent)) {
        if (const auto cols = list->GetCols()) {
            return cols->MatchLayout(*this);
        }
    }
    // 默认的水平布局
    this->relayout_h();
}


// ----------------------------------------------------------------------------
// --------------------         List Header           -------------------------
// ----------------------------------------------------------------------------

// ui namespace
namespace LongUI {
    // UIListHeader类 元信息
    LUI_CONTROL_META_INFO(UIListHeader, "listheader");
}



/// <summary>
/// Initializes a new instance of the <see cref="UIListItem" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIListHeader::UIListHeader(UIControl* parent, const MetaControl& meta) noexcept
    : Super(impl::ctor_lock(parent), meta),
    m_oImage(this), m_oLabel(this), m_oSortDir(this) {
    //m_state.focusable = true;
    // 内间距
    m_oBox.padding = { 4, 0, 4, 0 };
    // 水平布局
    m_state.orient = Orient_Horizontal;
    // 属于HEADER CELL
    m_oStyle.appearance = Appearance_TreeHeaderCell;
    // 私有实现
    //UIControlPrivate::SetFocusable(image, false);
    //UIControlPrivate::SetFocusable(label, false);
#ifndef NDEBUG
    m_oImage.name_dbg = "listheader::image";
    m_oLabel.name_dbg = "listheader::label";
    m_oSortDir.name_dbg = "listheader::sortdir";
    assert(m_oImage.IsFocusable() == false);
    assert(m_oLabel.IsFocusable() == false);
    assert(m_oSortDir.IsFocusable() == false);
#endif
    // 构造锁
    impl::ctor_unlock();
}


/// <summary>
/// Finalizes an instance of the <see cref="UIListItem"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIListHeader::~UIListHeader() noexcept {
    // 存在提前释放子控件, 需要标记"在析构中"
    m_state.destructing = true;
}

/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIListHeader::add_attribute(uint32_t key, U8View value) noexcept {
    // 新增属性列表
    constexpr auto BKDR_VALUE = 0x246df521_ui32;
    // 分类讨论
    switch (key)
    {
    case "label"_bkdr:
        // 传递给子控件
        Unsafe::AddAttrUninited(m_oLabel, BKDR_VALUE, value);
        break;
    default:
        // 其他情况, 交给基类处理
        return Super::add_attribute(key, value);
    }
}


/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UIListHeader::SetText(CUIString&& text) noexcept {
    // 设置
    if (m_oLabel.SetText(std::move(text))) {
#ifdef LUI_ACCESSIBLE
        //LongUI::Accessible(m_pAccessible, Callback_PropertyChanged);
#endif
    }
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UIListHeader::SetText(const CUIString & text) noexcept {
    return this->SetText(CUIString{ text });
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UIListHeader::SetText(U16View text) noexcept {
    return this->SetText(CUIString(text));
}