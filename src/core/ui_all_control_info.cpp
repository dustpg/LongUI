#include <interface/ui_ctrlinfolist.h>
#include <control/ui_richlistitem.h>
#include <control/ui_richlistbox.h>
#include <control/ui_radiogroup.h>
#include <control/ui_listheader.h>
#include <control/ui_box.h>
#include <control/ui_scrollbar.h>
#include <control/ui_groupbox.h>
#include <control/ui_checkbox.h>
#include <control/ui_listhead.h>
#include <control/ui_splitter.h>
#include <control/ui_listitem.h>
#include <control/ui_listcols.h>
#include <control/ui_progress.h>
#include <control/ui_caption.h>
#include <control/ui_tooltip.h>
#include <control/ui_listcol.h>
#include <control/ui_listbox.h>
#include <control/ui_textbox.h>
#include <control/ui_spacer.h>
#include <control/ui_button.h>
#include <control/ui_radio.h>
#include <control/ui_scale.h>
#include <control/ui_image.h>
#include <control/ui_label.h>
#include <control/ui_stack.h>
#include <control/ui_deck.h>
#include <control/ui_test.h>

#include <control/ui_tab.h>
#include <control/ui_tabs.h>
#include <control/ui_tabbox.h>
#include <control/ui_tabpanel.h>
#include <control/ui_tabpanels.h>

#include <control/ui_statusbar.h>
#include <control/ui_statusbarpanel.h>

#include <control/ui_toolbar.h>
#include <control/ui_toolbox.h>
#include <control/ui_toolbarbutton.h>
#include <control/ui_toolbarseparator.h>
#include <control/ui_menuseparator.h>
#include <control/ui_menupopup.h>
#include <control/ui_menulist.h>
#include <control/ui_menuitem.h>
#include <control/ui_menubar.h>
#include <control/ui_menu.h>

#include <control/ui_tree.h>
#include <control/ui_treerow.h>
#include <control/ui_treecol.h>
#include <control/ui_treecols.h>
#include <control/ui_treeitem.h>
#include <control/ui_treecell.h>
#include <control/ui_treechildren.h>

#include <control/ui_grid.h>
#include <control/ui_column.h>
#include <control/ui_columns.h>
#include <control/ui_row.h>
#include <control/ui_rows.h>

#include <control/ui_popup.h>
#include <control/ui_popupset.h>
#include <core/ui_ctrlmeta.h>


// ui namespace
namespace LongUI {
    // -------------------- TYPE DEF ------------------
    struct UIMetaTypeDef {
        LUI_CONTROL_META_INFO_FAKE_A(UIPopup);
        LUI_CONTROL_META_INFO_FAKE_A(UIToolBox);
        LUI_CONTROL_META_INFO_FAKE_A(UIPopupSet);
        LUI_CONTROL_META_INFO_FAKE_A(UIHBoxLayout);
        LUI_CONTROL_META_INFO_FAKE_A(UIToolBarSeparator);
    };
    LUI_CONTROL_META_INFO_FAKE_B(UIPopup, "popup");
    LUI_CONTROL_META_INFO_FAKE_B(UIToolBox, "toolbox");
    LUI_CONTROL_META_INFO_FAKE_B(UIPopupSet, "popupset");
    LUI_CONTROL_META_INFO_FAKE_B(UIHBoxLayout, "hbox");
    LUI_CONTROL_META_INFO_FAKE_B(UIToolBarSeparator, "toolbarseparator");

    /// <summary>
    /// The default control information
    /// </summary>
    const MetaControl* const DefaultControlInfo[] = {
        // 一般控件
        &UIControl::s_meta,
#ifndef DEBUG
        // 滚动视图
        &UIScrollArea::s_meta,
#endif

        // Rich List Item - 富列表项目
        &UIRichListItem::s_meta,
        // Rich List Box - 富列表容器
        &UIRichListBox::s_meta,
        // V Box Layout - 水平箱型布局
        &UIVBoxLayout::s_meta,
        // Radio Group - 单选框组
        &UIRadioGroup::s_meta,
        // List Header - 列表头项
        &UIListHeader::s_meta,
        // Box Layout - 箱型布局
        &UIBoxLayout::s_meta,
        // Scroll Bar - 滚动条
        &UIScrollBar::s_meta,
        // Menu Popup - 菜单弹窗
        &UIMenuPopup::s_meta,
        // Check Box - 多选框
        &UICheckBox::s_meta,
        // Group Box - 分组框
        &UIGroupBox::s_meta,
        // Menu Item - 菜单项
        &UIMenuItem::s_meta,
        // Menu List - 菜单表
        &UIMenuList::s_meta,
        // Progress - 进度条
        &UIProgress::s_meta,
        // Splitter - 分割线
        &UISplitter::s_meta,
        // Text Box - 文本框
        &UITextBox::s_meta,
        // Tooltip - 提示框
        &UITooltip::s_meta,
        // Caption - 分组标题
        &UICaption::s_meta,
        // Spacer - 占位控件
        &UISpacer::s_meta,
        // Button - 按钮控件
        &UIButton::s_meta,
        // Scale - 滑动控件
        &UIScale::s_meta,
        // Image - 图像控件
        &UIImage::s_meta,
        // Label - 标签控件
        &UILabel::s_meta,
        // Radio - 单选框
        &UIRadio::s_meta,
        // Stack - 栈容器
        &UIStack::s_meta,
        // Deck - 甲板容器
        &UIDeck::s_meta,

        // Tab - 标签页
        &UITab::s_meta,
        // Tabs - 标签页组
        &UITabs::s_meta,
        // Tab Box - 标签页框
        &UITabBox::s_meta,
        // Tab Panel - 标签页容器(可选)
        &UITabPanel::s_meta,
        // Tab Panels - 标签页容器组
        &UITabPanels::s_meta,

        // Status Bar - 状态栏
        &UIStatusBar::s_meta,
        // Status Bar Panel - 状态栏项目
        &UIStatusBarPanel::s_meta,

        // List Head - 列表头组
        &UIListHead::s_meta,
        // List Item - 列表项
        &UIListItem::s_meta,
        // List Item - 列表列组
        &UIListCols::s_meta,
        // List Box - 列表列
        &UIListCol::s_meta,
        // List Box - 列表框
        &UIListBox::s_meta,

        // Tool Bar 工具条
        &UIToolBar::s_meta,
        // Tool Bar 工具条用按钮
        &UIToolBarButton::s_meta,
        // Menu Separator 菜单分隔栏
        &UIMenuSeparator::s_meta,
        // Menu Bar 菜单栏
        &UIMenuBar::s_meta,
        // Menu 菜单
        &UIMenu::s_meta,

        // Tree - 树控件
        &UITree::s_meta,
        // Tree Row - 树行组
        &UITreeRow::s_meta,
        // Tree Col - 树列项
        &UITreeCol::s_meta,
        // Tree Cols - 树列组
        &UITreeCols::s_meta,
        // Tree Item - 树项目
        &UITreeItem::s_meta,
        // Tree Cell - 树单元
        &UITreeCell::s_meta,
        // Tree Children - 树子节点
        &UITreeChildren::s_meta,


#ifndef LUI_NO_UIGRID
        // UIRow     - 表格行
        &UIRow::s_meta,
        // UIRows    - 表格行容器
        &UIRows::s_meta,
        // UIGrid    - 表格
        &UIGrid::s_meta,
        // UIColumn  - 表格列
        &UIColumn::s_meta,
        // UIColumns - 表格列容器
        &UIColumns::s_meta,
#endif

        // ---------------- TYPEDEF -------------------------
        &UIMetaTypeDef::UIPopup__s_meta,
        &UIMetaTypeDef::UIToolBox__s_meta,
        &UIMetaTypeDef::UIPopupSet__s_meta,
        // H Box Layout - 垂直箱型布局
        &UIMetaTypeDef::UIHBoxLayout__s_meta,
        &UIMetaTypeDef::UIToolBarSeparator__s_meta,

#ifndef NDEBUG
        // Test - 测试控件
        &UITest::s_meta,
#endif
    };
    /// <summary>
    /// Adds the default control information.
    /// </summary>
    /// <param name="list">The list.</param>
    bool AddDefaultControlInfo(ControlInfoList& list) noexcept {
        const auto len = sizeof(DefaultControlInfo) / sizeof(DefaultControlInfo[0]);
        // 空间不足
        if (list.end_of_list + len > std::end(list.info_list)) return false;
        std::memcpy(list.end_of_list, DefaultControlInfo, sizeof(DefaultControlInfo));
        list.end_of_list += len;
#ifndef NDEBUG
        // 测试正确性
        for (auto x : DefaultControlInfo) {
            // 跳过这个
            if (x == &UITooltip::s_meta) continue;
            // 跳过这个
            if (x == &UIMenuPopup::s_meta) continue;
            // 跳过这个
            if (x == &UIMetaTypeDef::UIPopup__s_meta) continue;
            // 跳过这个
            if (x == &UIMetaTypeDef::UIToolBox__s_meta) continue;
            // 跳过这个
            if (x == &UIMetaTypeDef::UIPopupSet__s_meta) continue;
            // 跳过这个
            if (x == &UIMetaTypeDef::UIHBoxLayout__s_meta) continue;
            // 跳过这个
            if (x == &UIMetaTypeDef::UIToolBarSeparator__s_meta) continue;
            // 测试这个
            if (auto ctrl = x->create_func(nullptr)) {
                assert(&ctrl->RefMetaInfo() == x);
                delete ctrl;
            }
        }
#endif
        return true;
    }
}