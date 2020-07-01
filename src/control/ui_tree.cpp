// LUI
#include <control/ui_tree.h>
#include <control/ui_treerow.h>
#include <control/ui_treecol.h>
#include <control/ui_treecols.h>
#include <control/ui_treeitem.h>
#include <control/ui_treecell.h>
#include <control/ui_treechildren.h>
#include <core/ui_ctrlmeta.h>
#include <control/ui_scrollbar.h>

#include <core/ui_manager.h>
#include <input/ui_kminput.h>
#include <debugger/ui_debug.h>
#include <constexpr/const_bkdr.h>

// PRIVATE
#include "../private/ui_private_control.h"
// C++
#include <algorithm>

// ACCESSIBLE
#ifdef LUI_ACCESSIBLE
#include <accessible/ui_accessible_callback.h>
#include <accessible/ui_accessible_event.h>
#include <accessible/ui_accessible_type.h>
#endif

// ui namespace
namespace LongUI {
    // UITree类 元信息
    LUI_CONTROL_META_INFO(UITree, "tree");
    // UITreeRow类 元信息
    LUI_CONTROL_META_INFO(UITreeRow, "treerow");
    // UITreeCol类 元信息
    LUI_CONTROL_META_INFO(UITreeCol, "treecol");
    // UITreeCols类 元信息
    LUI_CONTROL_META_INFO(UITreeCols, "treecols");
    // UITreeItem类 元信息
    LUI_CONTROL_META_INFO(UITreeItem, "treeitem");
    // UITreeCell类 元信息
    LUI_CONTROL_META_INFO(UITreeCell, "treecell");
    // UITreeChildren类 元信息
    LUI_CONTROL_META_INFO(UITreeChildren, "treechildren");
    // detail namespace
    namespace detail {
        // calculate lowest common ancestor
        UIControl* lowest_common_ancestor(UIControl* now, UIControl* old) noexcept;
    }
}


// ----------------------------------------------------------------------------
// --------------------             Tree              -------------------------
// ----------------------------------------------------------------------------



/// <summary>
/// Finalizes an instance of the <see cref="UITree"/> class.
/// </summary>
/// <returns></returns>
LongUI::UITree::~UITree() noexcept {

}


/// <summary>
/// Initializes a new instance of the <see cref="UITree"/> class.
/// </summary>
/// UITree(tree) 是树形控件的基础顶层控件, 所有的子控件需要包含在内部
/// <remarks>
/// </remarks>
/// <param name="meta">The meta.</param>
LongUI::UITree::UITree(const MetaControl& meta) noexcept : Super(meta) {
    // 树节点即为自己
    m_pTree = this;
    // 默认为列表框
    m_oStyle.appearance = Appearance_ListBox;
#ifdef LUI_ACCESSIBLE
    // 默认逻辑对象为空
    m_pAccCtrl = this;
#endif
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UITree::Update(UpdateReason reason) noexcept {
#if 0
    // 更新行高
    if (reason & Reason_ChildIndexChanged) {
        // 更新
        this->OnTreeNodeToggled(nullptr);
        // 判断DISPLAY ROWS
        if (m_pChildren) {
            UIControl* ctrl = m_pChildren;
            while (ctrl->GetChildrenCount()) ctrl = &(*ctrl->begin());
            // 正常情况下ctrl现在是treecell, parent就是treerow
            const auto mh = ctrl->GetParent()->GetMinSize().height;
            m_pChildren->RefLineHeight() = mh;
        }
    }
    // XXX: 要求重新布局
    if (reason & Reason_BasicRelayout) {
        // 重新布局
        this->relayout_base(m_pCols);
        // 更新行高?

        // 截断消息
        return UIControl::Update(reason);
    }
    // 重新计算建议值
    if (reason & Reason_BasicUpdateFitting) this->refresh_fitting();
    // 重新布局
    if (reason & Reason_BasicRelayout) this->relayout();
#endif
#ifndef NDEBUG
    if (!std::strcmp(m_id.id, "tree1")) {
        if (reason & Reason_BasicRelayout) {
            int bk = 9;
        }
    }
#endif
    // 重新计算建议值
    if (reason & Reason_BasicUpdateFitting) this->refresh_fitting(m_pCols);
    // 重新布局
    if (reason & Reason_BasicRelayout) this->relayout(m_pCols);
    // 其他的交给超超类处理
    UIControl::Update(reason);
}


/// <summary>
/// Adds the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UITree::add_child(UIControl& child) noexcept {
    // TreeCols对象?
    if (const auto clos = uisafe_cast<UITreeCols>(&child)) {
        m_pCols = clos;
    }
    // 直属TreeChildren
    else if (const auto tchild = uisafe_cast<UITreeChildren>(&child)) {
        tchild->SetAutoOverflow();
    }
    return Super::add_child(child);
}

/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UITree::add_attribute(uint32_t key, U8View value) noexcept {
    constexpr auto BKDR_SELTYPE     = 0xdee1c438_ui32;
    constexpr auto BKDR_ROWS        = 0x0f63dd45_ui32;
    switch (key)
    {
    case BKDR_SELTYPE:
        // seltype      : 选择类型
        m_seltype = AttrParser::Seltype(value);
        //if (this->IsSelCell()) this->write_item_appearance();
        break;
    case BKDR_ROWS:
        // rows         : 显示列数
        m_displayRow = value.ToInt32();
        break;
    default:
        // 其他交给超类处理
        return Super::add_attribute(key, value);
    }
}

/// <summary>
/// initialize UITree
/// </summary>
/// <returns></returns>
//void LongUI::UITree::initialize() noexcept {
//    // 根节点必须为打开状态
//    m_bOpened = true;
//    // 初始化超类
//    Super::initialize();
//}


/// <summary>
/// Items the removed.
/// </summary>
/// <param name="item">The item.</param>
/// <returns></returns>
void LongUI::UITree::ItemRemoved(UITreeItem& item) noexcept {
    // 删除中直接返回?
    if (m_state.destructing) return;
    // 清除引用
    item.ClearTreeNode();
    // 移除Last Op引用
    if (m_pLastOp == &item) m_pLastOp = nullptr;
    // 取消所有显示等待下次刷新
    m_displayed.clear();
    // 1. 删除在选数据
    if (item.IsSelected()) {
        const auto enditr = m_selected.end();
        const auto itr = std::find(m_selected.begin(), enditr, &item);
#ifndef NDEBUG
        // 自己写的允许擦除end迭代器
        m_selected.erase(m_selected.end());
#endif
        //if (itr != enditr)
        m_selected.erase(itr);
    }
}


/// <summary>
/// Selects the item.
/// </summary>
/// <param name="item">The item.</param>
/// <param name="exadd">if set to <c>true</c> [exadd].</param>
/// <param name="cell">cell</param>
/// <returns></returns>
void LongUI::UITree::SelectItem(UITreeItem& item, bool exadd, UITreeCell* cell) noexcept {
    // 在表内额外添加就算了
    if (item.IsSelected() && exadd) return;
    // 记录上次操作对象
    m_pLastOp = &item;
    // 清空?
    if (!exadd || !this->IsMultiple()) this->ClearAllSelected();
    // 写入表内
    this->select_item(item, cell);
}


/// <summary>
/// Selects to.
/// </summary>
/// <param name="index">The index.</param>
/// <param name="cell">The cell.</param>
/// <returns></returns>
void LongUI::UITree::SelectTo(uint32_t index, UITreeCell* cell) noexcept {
    // 范围外
    if (index >= m_displayed.size()) {
#ifndef NDEBUG
        LUIDebug(Warning) << this << "out of range or OOM" << endl;
#endif // !NDEBUG
        return;
    }
    const auto item = m_displayed[index];
    assert(m_pChildren && "cannot be null on call select");
    assert(m_pChildren->GetChildrenCount() && "cannot be 0 on call select");
    // 单选?
    if (!this->IsMultiple()) return this->SelectItem(*item, false, cell);
    // 先清除之前的所有选择
    this->ClearAllSelected();
    // 存在m_pLastOp
    uint32_t sel_from = 0, sel_to = index;
    if (m_pLastOp) {
        const auto last = m_pLastOp->index;
        if (last < m_displayed.size()) {
            sel_from = std::min(index, last);
            sel_to = std::max(index, last);
        }
    }
    // 选择
    std::for_each(
        &m_displayed[sel_from],
        &m_displayed[sel_to] + 1,
        [this](UITreeItem* i) noexcept { this->select_item(*i, nullptr); }
    );
}

/// <summary>
/// Clears the selected.
/// </summary>
/// <param name="item">The item.</param>
/// <returns></returns>
void LongUI::UITree::ClearSelected(UITreeItem& item) noexcept {
    const auto enditr = m_selected.end();
    const auto itr = std::find(m_selected.begin(), enditr, &item);
    m_selected.erase(itr);
    item.StartAnimation({ State_Selected, State_Non });
    item.SelectCell(nullptr);
}

/// <summary>
/// Clears all selected.
/// </summary>
/// <returns></returns>
void LongUI::UITree::ClearAllSelected() noexcept {
    for (auto* item : m_selected) {
        assert(item->GetRow() && "???");
        item->SelectCell(nullptr);
        item->StartAnimation({ State_Selected, State_Non });
    }
    m_selected.clear();
}


// private impl for tree
struct LongUI::UITree::Private {
    // same tree
    static void same_tree(UITreeChildren& tc, UITree* tree) noexcept {
        // TODO: [优化] 非递归
        for (auto& child : tc) {
            auto& item = *longui_cast<UITreeItem*>(&child);
            item.m_pTree = tree;
            if (const auto data = item.GetTreeChildren()) same_tree(*data, tree);
        }
    }
    // add displayed
    static void add_displayed(ItemList& list, UITreeItem& node) noexcept {
        // TODO: [优化] 非递归
        if (!node.IsVisible()) return;
        list.push_back(&node);
        if (node.IsOpened()) {
            const auto children = node.GetTreeChildren();
            if (children) for (auto& child : (*children))
                add_displayed(list, *longui_cast<UITreeItem*>(&child));
        };
    }
};



/// <summary>
/// write item 
/// </summary>
/// <returns></returns>
void LongUI::UITree::write_item_appearance() noexcept {

}



/// <summary>
/// tree node tooggle
/// </summary>
/// <returns></returns>
void LongUI::UITree::OnTreeNodeToggled(UITreeItem* item) noexcept {
    // TODO: [优化] 更新从后面item开始所有节点
    m_displayed.clear();
    // 第一级可能有滚动条
    if (m_pChildren) for (auto& child : (*m_pChildren)) {
        if (const auto node = uisafe_cast<UITreeItem>(&child))
            Private::add_displayed(m_displayed, *node);
    }
    // 有效
    if (m_displayed.empty())  return;
    const auto count = m_displayed.size();
    for (uint32_t i = 0; i != count; ++i) m_displayed[i]->index = i;
}

/// <summary>
/// Selects the item.
/// </summary>
/// <param name="item">The item.</param>
/// <returns></returns>
void LongUI::UITree::select_item(UITreeItem& item, UITreeCell* cell) noexcept {
    // XXX: 链表实现m_selected ?

    // 写入表内
    m_selected.push_back(&item);
    item.StartAnimation({ State_Selected, State_Selected });
    //if (const auto row = item.GetRow())
    //    row->StartAnimation({ StyleStateType::Type_Selected, true });
    // CELL 选择
    if (this->IsSelCell()) item.SelectCell(cell);
}


/// <summary>
/// check item is in displaying
/// </summary>
/// <param name="item"></param>
/// <returns></returns>
bool LongUI::UITree::is_item_in_displaying(const UITreeItem& item) const noexcept {
    const auto i = item.index;
    return i < m_displayed.size() && m_displayed[i] == &item;
}

#ifdef LUI_ACCESSIBLE

/// <summary>
/// Accessibles the specified .
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
auto LongUI::UITree::accessible(const AccessibleEventArg& args) noexcept -> EventAccept {
    // 分类处理
    switch (args.event)
    {
        using get0_t = AccessibleGetPatternsArg;
        using get1_t = AccessibleGetCtrlTypeArg;
        using get2_t = AccessibleGetAccNameArg;
    case AccessibleEvent::Event_All_GetControlType:
        // 获取控件类型
        static_cast<const get1_t&>(args).type =
            AccessibleControlType::Type_Tree;
        return Event_Accept;
    case AccessibleEvent::Event_All_GetAccessibleName:
        // 获取Acc名称
        return UIControl::accessible(args);
    }
    return Super::accessible(args);
}

#endif

// ----------------------------------------------------------------------------
// --------------------           Tree Row            -------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Finalizes an instance of the <see cref="UITreeRow"/> class.
/// </summary>
/// <returns></returns>
LongUI::UITreeRow::~UITreeRow() noexcept {
    // 拥有额外的操作, 先标记析构
    m_state.destructing = true;
}

/// <summary>
/// Sets the has child.
/// </summary>
/// <param name="has">if set to <c>true</c> [has].</param>
/// <returns></returns>
void LongUI::UITreeRow::SetHasChild(bool has) noexcept {
    auto& sstate = UIControlPrivate::RefStyleState(m_oTwisty);
    const uint32_t mask = uint32_t(has) << Index_Indeterminate;
    sstate = (sstate & ~State_Indeterminate) | StyleState(mask);
    m_bHasChild = has;
}

/// <summary>
/// Gets the row string.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UITreeRow::GetRowString(CUIString& text) const noexcept {
    for (auto& child : (*this)) {
        if (const auto cell = uisafe_cast<const UITreeCell>(&child)) {
            if (!text.empty()) text += L' ';
            text += cell->RefText();
        }
    }
}


/// <summary>
/// Opens the node.
/// </summary>
/// <returns></returns>
bool LongUI::UITreeRow::OpenNode() noexcept {
    return m_bOpened ? false : (this->open_close(true), true);
}

/// <summary>
/// Closes the node.
/// </summary>
/// <returns></returns>
bool LongUI::UITreeRow::CloseNode() noexcept {
    return m_bOpened ? (this->open_close(false), true) : false ;
}

/// <summary>
/// Toggles the node.
/// </summary>
/// <returns></returns>
bool LongUI::UITreeRow::ToggleNode() noexcept {
    if (!m_bHasChild) return false;
    return m_bOpened ? CloseNode() : OpenNode();
}

/// <summary>
/// init open data
/// </summary>
/// <param name="open">The open.</param>
/// <returns></returns>
void LongUI::UITreeRow::InitOpen_Parent(bool open) noexcept {
    m_bOpened = open;
    auto& state = UIControlPrivate::RefStyleState(m_oTwisty);
    state = (state & ~State_Closed) | (open ? State_Non : State_Closed);
}



/// <summary>
/// Closes the node.
/// </summary>
/// <returns></returns>
void LongUI::UITreeRow::open_close(bool open) noexcept {
    m_bOpened = open;
    // 修改twisty状态
    auto& twisty = m_oTwisty;
    twisty.StartAnimation({ State_Closed, open ? State_Non : State_Closed });
    // 父节点必须是UITreeItem
    if (const auto item = longui_cast<UITreeItem*>(m_pParent)) {
        item->TreeChildrenOpenClose(open);
        // 父节点就必须有根节点
        const auto tree = item->GetTreeNode();
        assert(tree && "parent exsit but tree?");
        // 提示根节点
        tree->OnTreeNodeToggled(item);
    }
}

/// <summary>
/// Initializes a new instance of the <see cref="UITreeRow"/> class.
/// </summary>
/// <remarks>
/// treerow 表示树形控件的一行, 父控件是treeitem, 子控件是treecell
/// 在XUL中主要作用就是用来展开收拢树形控件的一行
/// </remarks>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UITreeRow::UITreeRow(const MetaControl& meta) noexcept : Super(meta),
    m_oTwisty(this), m_oImage(this) {
    // 暂时用ListItem?
    //m_oStyle.appearance = Appearance_app | Appearance_ListItem;
    // 私有实现
    //UIControlPrivate::SetFocusable(image, false);
    //UIControlPrivate::SetFocusable(label, false);
#ifndef NDEBUG
    m_oTwisty.name_dbg = "treerow::twisty";
    m_oImage.name_dbg = "treerow::image";
#endif
    UIControlPrivate::SetGuiEvent2Parent(m_oTwisty);
    UIControlPrivate::SetAppearance(m_oTwisty, Appearance_TreeTwisty);
    // 继承选择/禁止状态
    m_oStyle.inherited = State_Disabled | State_Selected;
    // 一开始假定没有数据
    this->SetHasChild(false);
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UITreeRow::Update(UpdateReason reason) noexcept {
    // 重新计算建议值
    if (reason & Reason_BasicUpdateFitting) this->refresh_fitting();
    // 重新布局
    if (reason & Reason_BasicRelayout) this->relayout();
    // 其他的交给超类处理
    Super::Update(reason);
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UITreeRow::DoEvent(UIControl* sender, const EventArg& e) noexcept->EventAccept {
    switch (e.nevent)
    {
    case NoticeEvent::Event_UIEvent:
        // 点击了 twisty?
        if (sender == &m_oTwisty) {
            switch (static_cast<const GuiEventArg&>(e).GetType())
            {
            case UIControl::_onClick():
                this->ToggleNode();
                break;
            }
        }
        return Event_Accept;
    default:
        // 其他事件
        return Super::DoEvent(sender, e);
    }
}

/// <summary>
/// initialize UITreeRow
/// </summary>
/// <returns></returns>
void LongUI::UITreeRow::initialize() noexcept {
#ifndef NDEBUG
    if (m_pParent)
#endif
        if (const auto parent = longui_cast<UITreeItem*>(m_pParent)) {
            if (const auto tree = parent->GetTreeNode()) {
                if (tree->IsSelCell())
                    UIControlPrivate::SetAppearance(*this, Appearance_TreeRowModeCell);
                else
                    UIControlPrivate::SetAppearance(*this, Appearance_ListItem);
            }
        }
    return Super::initialize();
}


/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UITreeRow::DoMouseEvent(const MouseEventArg& e) noexcept -> EventAccept {
    switch (e.type)
    {
    case LongUI::MouseEvent::Event_LButtonDown:
        // 父节点必须是UITreeItem
        if (const auto item = longui_cast<UITreeItem*>(m_pParent)) {
            const auto tree = item->GetTreeNode();
            assert(tree && "parent exsit but tree");
            // 判断SHIFT
            if (CUIInputKM::GetKeyState(CUIInputKM::KB_SHIFT)) {
                tree->SelectTo(item->index);
            }
            else {
                // 判断CTRL键
                const auto ctrl = CUIInputKM::GetKeyState(CUIInputKM::KB_CONTROL);
                if (ctrl && tree->IsMultiple() && item->IsSelected()) {
                    // 取消选择
                    tree->ClearSelected(*item);
                }
                // 添加选择
                else {
                    const auto cell = uisafe_cast<UITreeCell>(m_pHovered);
                    tree->SelectItem(*item, ctrl, cell);
                }
            }
        }
        [[fallthrough]];
    default:
        return Super::DoMouseEvent(e);
    }
}

/// <summary>
/// Refreshes the fitting.
/// </summary>
/// <returns></returns>
void LongUI::UITreeRow::refresh_fitting() noexcept {
    //float width = m_fLevelOffset;
    float height = 0.f;
    // 遍历
    for (auto& child : (*this)) {
        const auto ms = child.GetBoxFittingSize();
        //width += ms.width;
        height = std::max(height, ms.height);
    }
    // 加上间距?

    // 更新
    this->update_fitting_size({ 0, height });
}

/// <summary>
/// Relayouts this instance.
/// </summary>
/// <returns></returns>
void LongUI::UITreeRow::relayout() noexcept {
    // TODO: 重新编写? MatchLayout
    const auto item = longui_cast<UITreeItem*>(m_pParent);
    const auto level_offset = item->CalTreeOffset();
    // 没有cols时调用
    const auto when_no_cols = [=]() noexcept {
        const auto ctsize = this->RefBox().GetContentSize();
        float xoffset = level_offset;
        // 遍历有效子节点
        for (auto& child : (*this)) {
            // TODO: maxsize足够小应该居中放置
            child.SetPos({ xoffset, 0.f });
            const auto min = child.GetBoxFittingSize();
            this->resize_child(child, { min.width, ctsize.height });
            xoffset += min.width;
        }
    };
    // 存在cols时调用
    const auto when_cols = [=](UITreeCols* cols) noexcept {
        const auto ctsize = this->RefBox().GetContentSize();
        float xoffset = level_offset;
        const auto set_child = [&xoffset, ctsize](UIControl& child) noexcept {
            child.SetPos({ xoffset, 0.f });
            const auto min = child.GetBoxFittingSize();
            UITreeRow::resize_child(child, { min.width, ctsize.height });
            xoffset += min.width;
        };
        // twisty
        set_child(m_oTwisty);
        // image
        set_child(m_oImage);
        
        // (image, end)
        auto itr = ++Iterator{ &m_oImage };


        for (auto& col : (*cols)) {
            // 必须是col
            if (!uisafe_cast<UITreeCol>(&col)) continue;
            // 提前出局
            if (itr == this->end()) break;
            const auto pos = col.GetPos();
            const auto size = col.GetBoxSize();
            auto& child = *itr;
            child.SetPos({ pos.x + xoffset, pos.y });
            const auto width = size.width - xoffset;
            this->resize_child(child, { width, size.height });
            ++itr;
            // 后续的不再偏移
            xoffset = 0.f;
        }

    };
    // 父节点是treeitem
    if (const auto treeitem = longui_cast<UITreeItem*>(m_pParent)) {
        const auto tree = treeitem->GetTreeNode();
        assert(tree && "bad tree");
        // 获取树列
        if (const auto cols = tree->GetCols()) {
            if (UIControlPrivate::IsNeedRelayout(*cols)) return;
            return when_cols(cols);
        }
    }
    // 没有cols
    when_no_cols();
}


// ----------------------------------------------------------------------------
// --------------------           Tree Item           -------------------------
// ----------------------------------------------------------------------------




/// <summary>
/// Finalizes an instance of the <see cref="UITreeItem"/> class.
/// </summary>
/// <returns></returns>
LongUI::UITreeItem::~UITreeItem() noexcept {
    m_state.destructing = true;
    // 移除引用
    if (m_pTree) m_pTree->ItemRemoved(*this);
}


/// <summary>
/// Initializes a new instance of the <see cref="UITreeItem" /> class.
/// </summary>
/// <remarks>
/// UITreeItem(treeitem) 应该被直接包含在treechildren下面, 以及包含treerow控件
/// treeitem 可以被用户点击以选择树形控件的一行
/// </remarks>
/// <param name="meta">The meta.</param>
LongUI::UITreeItem::UITreeItem(const MetaControl& meta) noexcept : Super(meta) {
#ifdef LUI_ACCESSIBLE
    // 默认逻辑对象为空
    m_pAccCtrl = nullptr;
#endif
}


/// <summary>
/// add attribute
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UITreeItem::add_attribute(uint32_t key, U8View value) noexcept {
    const auto BKDR_OPEN = 0x0efd2e42_ui32;
    const auto BKDR_CONTAINER = 0xc85e44a5_ui32;
    switch (key)
    {
    case BKDR_OPEN:
        m_bOpened = value.ToBool();
        return;
    case BKDR_CONTAINER:
        m_bContainer = value.ToBool();
        return;
    }
    return Super::add_attribute(key, value);
}

/// <summary>
/// Adds the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UITreeItem::add_child(UIControl& child) noexcept {
    // 是TreeChildren?
    if (uisafe_cast<UITreeChildren>(&child)) {
        m_pChildren = static_cast<UITreeChildren*>(&child);
        // 递归设置树节点
        UITree::Private::same_tree(*m_pChildren, m_pTree);
#ifdef LUI_ACCESSIBLE
        // 有children就使用
        if (!m_pAccCtrl) m_pAccCtrl = m_pChildren;
#endif
    }
    // 是TreeRow?
    else if (uisafe_cast<UITreeRow>(&child)) {
        m_pRow = static_cast<UITreeRow*>(&child);
        if (m_pTree) {
            const auto app = m_pTree->IsSelCell()
                ? Appearance_TreeRowModeCell
                : Appearance_ListItem
                ;
            UIControlPrivate::ForceAppearance(*m_pRow, app);
        }
    }
#ifndef NDEBUG
    // 不允许其他类型的子节点
    else {
        if (uisafe_cast<UITreeCols>(&child)) {

        }
        else {
            assert(!"cannot add other type to treeitem");
        }
    }
#endif
    return Super::add_child(child);
}



/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UITreeItem::Update(UpdateReason reason) noexcept {
    // 重新计算建议值
    if (reason & Reason_BasicUpdateFitting) this->refresh_fitting(m_pRow);
    // 重新布局
    if (reason & Reason_BasicRelayout) this->relayout(m_pRow);
    // 其他的交给超类处理
    Super::Update(reason);
}


/// <summary>
/// Trees the children changed.
/// </summary>
/// <param name="has_child">if set to <c>true</c> [has child].</param>
/// <returns></returns>
void LongUI::UITreeItem::TreeChildrenChanged(bool has_child) noexcept {
    if (m_pRow) m_pRow->SetHasChild(has_child);
}


#if 0
/// <summary>
/// Selects to this.
/// </summary>
/// <returns></returns>
void LongUI::UITreeItem::SelectToThis() noexcept {
    assert(m_pTree && "tree cannot be null on call select");
    m_pTree->SelectTo();
}

/// <summary>
/// Selects this instance.
/// </summary>
/// <returns></returns>
void LongUI::UITreeItem::Select(bool exsel) noexcept {
    assert(m_pTree && "tree cannot be null on call select");
    m_pTree->SelectItem(*this, exsel);
}

#endif

/// <summary>
/// calculate: this is last item?
/// </summary>
/// <param name="open">if set to <c>true</c> [open].</param>
/// <returns></returns>
bool LongUI::UITreeItem::cal_is_last_item() const noexcept {
    assert(m_pParent && "bad parent");
    const UIControl* ctrl = this;
    while (!ctrl->IsLastChild()) {
        ctrl = UIControlPrivate::Next(ctrl);
        if (uisafe_cast<const UITreeItem>(ctrl)) return false;
    }
    return true;
}

/// <summary>
/// Trees the children close.
/// </summary>
/// <param name="open">if set to <c>true</c> [open].</param>
/// <returns></returns>
void LongUI::UITreeItem::TreeChildrenOpenClose(bool open) noexcept {
    m_bOpened = open;
    // 节点不是最后一个的话重绘父节点减少脏矩形压力
    if (!this->cal_is_last_item()) m_pParent->Invalidate();
    assert(m_pChildren && "bad apple??");
    if (m_pChildren) m_pChildren->SetVisible(open);
}

/// <summary>
/// cell remove
/// </summary>
/// <param name=""></param>
/// <returns></returns>
void LongUI::UITreeItem::CellRemoved(UITreeCell& cell) noexcept {
    // 移除弱引用
    if (&cell == m_pSelected) m_pSelected = nullptr;
}


/// <summary>
/// select cell
/// </summary>
/// <param name="cell"></param>
/// <returns></returns>
void LongUI::UITreeItem::SelectCell(UITreeCell* cell) noexcept {
    const auto prev_sel = m_pSelected; m_pSelected = cell;
    if (prev_sel) prev_sel->StartAnimation({ State_Selected, State_Non });
    if (cell) cell->StartAnimation({ State_Selected, State_Selected });
}



/// <summary>
/// select cell
/// </summary>
/// <param name="cell"></param>
/// <returns></returns>
void LongUI::UITreeItem::SelectCell(std::nullptr_t) noexcept {
    const auto prev_sel = m_pSelected; m_pSelected = nullptr;
    if (prev_sel) prev_sel->StartAnimation({ State_Selected, State_Non });
}

PCN_NOINLINE
/// <summary>
/// calculate offset
/// </summary>
/// <returns></returns>
auto LongUI::UITreeItem::CalTreeOffset() const noexcept -> float {
    if (this && m_pTree) {
        const auto lvdis = static_cast<float>(this->GetLevel() - m_pTree->GetLevel());
        return lvdis * m_pTree->level_offset * 0.5f;
    }
    return 0;
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
//auto LongUI::UITreeItem::DoEvent(UIControl* sender,
//    const EventArg& e) noexcept -> EventAccept {
//    switch (e.nevent)
//    {
//    case NoticeEvent::Event_RefreshBoxMinSize:
//        this->refresh_minsize(m_pRow);
//        return Event_Accept;
//    default:
//        // 其他事件
//        return Super::DoEvent(sender, e);
//    }
//}

/// <summary>
/// initialize UITreeItem
/// </summary>
/// <returns></returns>
void LongUI::UITreeItem::initialize() noexcept {
    // !!! 根节点必须为打开状态
    if (m_pTree == this) {
        m_bOpened = true;
        // 默认是字体大小的一半
        auto& v = m_pTree->level_offset;
        if (v <= 0) v = UIManager.RefDefaultFont().size * 0.5f;
    }
    // 将open信息传递给ROW
    if (m_pRow) m_pRow->InitOpen_Parent(m_bOpened);
    if (m_pChildren) m_pChildren->SetVisible(m_bOpened);
    // 初始计化超类
    Super::initialize();
}

PCN_NOINLINE
/// <summary>
/// Relayouts the tree.
/// </summary>
/// <param name="head">The head.</param>
/// <returns></returns>
void LongUI::UITreeItem::relayout(UIControl* head) noexcept {
    // XXX: 获取左上角位置
    const auto lt = this->RefBox().GetContentPos();
    float xofffset = this->CalTreeOffset();
    float yoffset = lt.y;
    const auto ct = this->RefBox().GetContentSize();
    // 拥有头对象?
    if (head) {
        // 位置
        head->SetPos(lt);
        // 大小
        const auto height = head->GetBoxFittingSize().height;
        this->resize_child(*head, { ct.width, height });
        // 调整children数据
        yoffset += height;
        // 第一层实际是0的偏移
        if (head == m_pRow) xofffset += height * 0.5f;
    }
    // 拥有子节点对象?
    if (m_pChildren && m_pChildren->IsVisible()) {
        // 位置
        m_pChildren->SetPos({ lt.x, yoffset });
        // 大小: 作为树的直属TC则瓜分剩余的
        Size2F size = { ct.width , 0.f };
        if (m_pTree == this)
            size.height = std::max(ct.height - yoffset, 0.f);
        else 
            size.height = m_pChildren->GetBoxFittingSize().height;
        //const auto mh = m_pChildren->GetBoxFittingSize().height;
        this->resize_child(*m_pChildren, size);
    }
}

/// <summary>
/// Refreshes the fitting.
/// </summary>
/// <returns></returns>
void LongUI::UITreeItem::refresh_fitting(UIControl* head) noexcept {
    // TODO: TREE 拥有独立的FITTING算法, 考虑更新算法
    float height = 0.f;
    // 加上内容
    if (head) height += head->GetBoxFittingSize().height;
    if (m_pChildren && m_pChildren->IsVisible())
        height += m_pChildren->GetBoxFittingSize().height;
    // 加上间距
    this->update_fitting_size({ 0, height });
}

#ifdef LUI_ACCESSIBLE

/// <summary>
/// Accessibles the specified .
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
auto LongUI::UITreeItem::accessible(const AccessibleEventArg& args) noexcept -> EventAccept {
    // 获取ACC名称
    const auto get_acc_name = [this](CUIString& text) noexcept {
        if (m_pRow) m_pRow->GetRowString(text);
    };
    // 分类处理
    switch (args.event)
    {
        using get0_t = AccessibleGetPatternsArg;
        using get1_t = AccessibleGetCtrlTypeArg;
        using get2_t = AccessibleGetAccNameArg;
        //case AccessibleEvent::Event_GetPatterns:
        //    // + 继承超类行为模型
        //    Super::accessible(args);
        //    static_cast<const get0_t&>(args).patterns |=
        //        // + 可调用的行为模型
        //        Pattern_Invoke
        //        // + 读写值的行为模型
        //        | Pattern_Value
        //        ;
        //    return Event_Accept;
    case AccessibleEvent::Event_All_GetControlType:
        // 获取控件类型
        static_cast<const get1_t&>(args).type =
            AccessibleControlType::Type_TreeItem;
        return Event_Accept;
    case AccessibleEvent::Event_All_GetAccessibleName:
        // 获取Acc名称
        get_acc_name(*static_cast<const get2_t&>(args).name);
        return Event_Accept;
    }
    return Super::accessible(args);
}
#endif

// ----------------------------------------------------------------------------
// --------------------        Tree Children          -------------------------
// ----------------------------------------------------------------------------



/// <summary>
/// Finalizes an instance of the <see cref="UITreeChildren"/> class.
/// </summary>
/// <returns></returns>
LongUI::UITreeChildren::~UITreeChildren() noexcept {

}


/// <summary>
/// Sets the level offset.
/// </summary>
/// <param name="offset">The offset.</param>
//void LongUI::UITreeChildren::SetLevelOffset(float offset) {
//    for (auto& child : (*this)) {
//        // 子节点必须是UITreeItem(可能是滚动条)
//        if (const auto ptr = uisafe_cast<UITreeItem>(&child)) {
//            ptr->TreeLevelOffset(offset);
//        }
//    }
//}

/// <summary>
/// Initializes a new instance of the <see cref="UITreeChildren"/> class.
/// </summary>
/// UITreeChildren(treechildren) 是为了嵌套树的一系列子控件的一个容器
/// 其父控件必须是treeitem(或是tree), 其子控件们也必须是treeitem
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UITreeChildren::UITreeChildren(const MetaControl& meta) noexcept : Super(meta) {
    // 垂直布局
    m_state.orient = Orient_Vertical;
}



/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UITreeChildren::Update(UpdateReason reason) noexcept {
    // 子节点添加删除
    if (reason & Reason_ChildIndexChanged) {
        const auto has = !!this->GetChildrenCount();
        // 仅仅修改有没有子节点才计算
        if (has != m_bHasChild) {
            m_bHasChild = has;
            // 通知父控件(父节点必须是UITreeItem)
            if (const auto item = longui_cast<UITreeItem*>(m_pParent)) {
                item->TreeChildrenChanged(has);
            }
        }
    }
    // 超类处理
    return Super::Update(reason);
}


#if 0

/// <summary>
/// do the event
/// </summary>
/// <returns></returns>
auto LongUI::UITreeChildren::DoEvent(UIControl* sender, const EventArg & e) noexcept -> EventAccept {
    switch (e.nevent)
    {
    case NoticeEvent::Event_RefreshBoxMinSize:
        this->refresh_minsize();
        return Event_Accept;
    }
    return Super::DoEvent(sender, e);
}

/// <summary>
/// relayout this
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UITreeChildren::relayout_this() noexcept {

}


/// <summary>
/// refresh minsize
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UITreeChildren::refresh_minsize() noexcept {
    Size2F min_size = { 0 };

    this->set_contect_minsize(min_size);
}
#endif

/// <summary>
/// Adds the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UITreeChildren::add_child(UIControl& child) noexcept {
    // TreeCols对象?
    if (const auto childitem = uisafe_cast<UITreeItem>(&child)) {
        if (const auto item = longui_cast<UITreeItem*>(m_pParent)) {
            childitem->m_pTree = item->m_pTree;
        }
    }
#ifndef NDEBUG
    else if (uisafe_cast<UIScrollBar>(&child)) {

    }
    else {
        assert(!"child to treechildren must be treeitem or scrollbar");
    }
#endif
    return Super::add_child(child);
}

// ----------------------------------------------------------------------------
// --------------------           Tree Col            -------------------------
// ----------------------------------------------------------------------------


/// <summary>
/// Finalizes an instance of the <see cref="UITreeCol"/> class.
/// </summary>
/// <returns></returns>
LongUI::UITreeCol::~UITreeCol() noexcept {

}


/// <summary>
/// Initializes a new instance of the <see cref="UITreeCol"/> class.
/// </summary>
/// <remarks>
/// UITreeCol(treecol) 其目的是为了方便设置列属性, 例如flex
/// 本身父控件必须是treecols
/// </remarks>
/// <param name="meta">The meta.</param>
LongUI::UITreeCol::UITreeCol(const MetaControl& meta) noexcept: Super(meta) {

}


// ----------------------------------------------------------------------------
// --------------------           Tree Cols           -------------------------
// ----------------------------------------------------------------------------


/// <summary>
/// Finalizes an instance of the <see cref="UITreeCols"/> class.
/// </summary>
/// <returns></returns>
LongUI::UITreeCols::~UITreeCols() noexcept {
}


/// <summary>
/// Initializes a new instance of the <see cref="UITreeCols"/> class.
/// </summary>
/// <remarks>
/// UITreeCols(treecols) 是treecol的父级控件(容器), 本身父控件必须是tree
/// 其目的是为了方便设置辣么多列属性, 例如flex
/// </remarks>
/// <param name="meta">The meta.</param>
LongUI::UITreeCols::UITreeCols(const MetaControl& meta) noexcept : Super(meta) {

}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UITreeCols::DoEvent(UIControl* sender, const EventArg& e) noexcept->EventAccept {
    switch (e.nevent)
    {
    case NoticeEvent::Event_Splitter:
        assert(m_pParent && "no parent");
        this->NeedUpdate(Reason_ChildLayoutChanged);
        // 父节点是tree
        if (auto tc = longui_cast<UITree*>(m_pParent)->GetTreeChildren())
            UITreeCols::do_update_for_item(*tc);
        m_pParent->Invalidate();
        [[fallthrough]];
    default:
        return Super::DoEvent(sender, e);
    }
}


/// <summary>
/// Relayouts this instance.
/// </summary>
/// <returns></returns>
void LongUI::UITreeCols::Update(UpdateReason reason) noexcept {
    // 需要尺寸
    if (reason & Reason_BasicUpdateFitting) this->refresh_fitting();
    // 需要布局
    if (reason & Reason_BasicRelayout) {
        this->relayout_h();
        // 优化: 不再需要时候再处理
        if (!(m_state.reason & Reason_BasicRelayout))
            this->relayout_final();
        return;
    }
    // 超类处理
    UIScrollArea::Update(reason);
}

/// <summary>
/// Relayouts this instance.
/// </summary>
/// <returns></returns>
void LongUI::UITreeCols::relayout_final() noexcept {
    assert(m_pParent && "no parent");
    // 父节点是tree
    if (auto tc = longui_cast<UITree*>(m_pParent)->GetTreeChildren())
        UITreeCols::do_update_for_item(*tc);
}

/// <summary>
/// Does the update for item.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::UITreeCols::do_update_for_item(UIControl& ctrl) noexcept {
    for (auto& child : ctrl) {
        if (const auto item = uisafe_cast<UITreeItem>(&child)) {
            // XXX: 理由
            if (const auto row = item->GetRow())
                row->NeedUpdate(Reason_ChildLayoutChanged);
            if (const auto cldr = item->GetTreeChildren())
                UITreeCols::do_update_for_item(*cldr);
        }
    }
}


// ----------------------------------------------------------------------------
// --------------------           Tree Cell           -------------------------
// ----------------------------------------------------------------------------


/// <summary>
/// Finalizes an instance of the <see cref="UITreeCell"/> class.
/// </summary>
/// <returns></returns>
LongUI::UITreeCell::~UITreeCell() noexcept {
#ifndef NDEBUG
    // 避免调试时警告
    if (m_pParent)
#endif // !NDEBUG
    // 父节点必须是UITreeRow
    if (const auto parent = longui_cast<UITreeRow*>(m_pParent)) {
        // 父节点的父节点必须是UITreeItem
        if (const auto grandparent = longui_cast<UITreeItem*>(parent->GetParent())) {
            grandparent->CellRemoved(*this);
        }
    }
}


/// <summary>
/// Initializes a new instance of the <see cref="UITreeCell"/> class.
/// 树单元控件算是默认裁剪字符串的 <see cref="UILabel"/>
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UITreeCell::UITreeCell(const MetaControl& meta) noexcept : Super(meta) {
    // 树单元默认裁剪字符串
    m_crop = Crop_End;
}

/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UITreeCell::add_attribute(uint32_t key, U8View value) noexcept {
    constexpr auto BKDR_LABEL       = 0x74e22f74_ui32;
    constexpr auto BKDR_VALUE       = 0x246df521_ui32;
    switch (key)
    {
    case BKDR_LABEL:
        // label: 重定向至value
        key = BKDR_VALUE;
        [[fallthrough]];
    default:
        // 其他情况
        return Super::add_attribute(key, value);
    }
}


/// <summary>
/// add child for UITreeCell
/// </summary>
/// <param name="child"></param>
/// <returns></returns>
//void LongUI::UITreeCell::add_child(UIControl& child) noexcept {
//    return Super::add_child(child);
//}


/// <summary>
/// initialize UITreeCell
/// </summary>
/// <returns></returns>
void LongUI::UITreeCell::initialize() noexcept{
    // TODO: 重写
#ifndef NDEBUG
    if (m_pParent)
#endif // !NDEBUG
        if (const auto ptr = longui_cast<UITreeRow*>(m_pParent)) {
            const auto item = longui_cast<UITreeItem*>(ptr->GetParent());
            if (const auto tree = item->GetTreeNode())
                if (tree->IsSelCell())
                    UIControlPrivate::SetAppearance(*this, Appearance_ListItem);
        }
    // 初始化超类
    Super::initialize();
}
