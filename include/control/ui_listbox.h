#pragma once
/**
* Copyright (c) 2014-2020 dustpg   mailto:dustpg@gmail.com
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

// super
#include "ui_control.h"
#include "../container/pod_vector.h"
//#include "../util/ui_double_click.h"
// base
#include "ui_scrollarea.h"

// ui namespace
namespace LongUI {
    // list cols
    class UIListCols;
    // list item
    class UIListItem;
    // list head
    class UIListHead;
    // listbox control
    class UIListBox : public UIControl {
        // super class
        using Super = UIControl;
        // item list
        using ItemList = POD::Vector<UIListItem*>;
    protected:
        // ctor
        UIListBox(const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UIListBox() noexcept;
        // ctor
        explicit UIListBox(UIControl* parent = nullptr) noexcept : UIListBox(UIListBox::s_meta) { this->final_ctor(parent); }
    public:
        // is multi-selected?
        bool IsMultiple() const noexcept { return m_seltype == Seltype_Multiple; }
        // get cols
        auto GetCols() const noexcept { return m_pCols; }
        // item removed. called from UIListItem's dtor
        void ItemRemoved(UIListItem&) noexcept;
        // get item index
        auto GetItemIndex(const UIListItem&) noexcept -> uint32_t;
        // get item count
        auto GetItemCount() const noexcept { return m_list; }
        // get item at
        auto GetItemAt(uint32_t i) const noexcept { return m_list[i]; }
        // insert item to
        auto InsertItem(uint32_t index, const CUIString&) noexcept -> UIListItem*;
        // select item
        void SelectItem(UIListItem&, bool exadd) noexcept;
        // select to
        void SelectTo(UIListItem&) noexcept;
        // clear select
        void ClearSelected(UIListItem&) noexcept;
        // clear all select
        void ClearAllSelected() noexcept;
        // get selected items
        auto&GetSelected() const noexcept { return m_selected; };
        // set line size
        void SetLineSize(Size2F) noexcept;
    public:
        // do normal event
        auto DoEvent(UIControl* sender, const EventArg& e) noexcept->EventAccept override;
        // Update
        void Update(UpdateReason) noexcept override;
    protected:
        // add attribute
        void add_attribute(uint32_t key, U8View value) noexcept;
        // add child
        void add_child(UIControl& child) noexcept override;
        // refresh cols min size
        void refresh_cols_minsize() noexcept;
        // refresh this min size
        void refresh_minsize() noexcept;
        // select item
        void select_item(UIListItem& item) noexcept;
        // refresh items index
        void refresh_item_index() noexcept;
        // re-layout
        void relayout() noexcept;
        // need refresh index
        bool need_refresh_index() const noexcept { return m_bRefreshTndex; }
        // clear need refresh index
        void clear_need_refresh_index() noexcept { m_bRefreshTndex = false; }
        // clear need refresh index
        void mark_need_refresh_index() noexcept { m_bRefreshTndex = true; }
    private:
        // listbox body
        UIScrollArea        m_oListboxBody;
        // list cols
        UIListCols*         m_pCols = nullptr;
        // list head
        UIListHead*         m_pHead = nullptr;
        // last op
        UIListItem*         m_pLastOp = nullptr;
        // item list
        ItemList            m_list;
        // selected
        ItemList            m_selected;
        // minwidth
        POD::Vector<float>  m_minwidth;
        // display row | xul::rows related
        uint16_t            m_displayRow = 4;
        // need refresh index
        bool                m_bRefreshTndex = false;
        // select type
        AttributeSeltype    m_seltype = Seltype_Single;
    };
    // get meta info for UIListBox
    LUI_DECLARE_METAINFO(UIListBox);
}

