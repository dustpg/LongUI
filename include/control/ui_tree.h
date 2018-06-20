#pragma once
/**
* Copyright (c) 2014-2018 dustpg   mailto:dustpg@gmail.com
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

#include "ui_treeitem.h"
#include <container/pod_vector.h>
//#include "../util/ui_double_click.h"

// ui namespace
namespace LongUI {
    // cols
    class UITreeCols;
    // tree
    class UITree : public UITreeItem {
        // super class
        using Super = UITreeItem;
        // item list
        using ItemList = POD::Vector<UITreeItem*>;
    protected:
        // ctor
        UITree(UIControl* parent, const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UITree() noexcept;
        // ctor
        UITree(UIControl* parent = nullptr) noexcept : UITree(parent, UITree::s_meta) {}
    public:
        // is multi-selected?
        bool IsMultiple() const noexcept { return m_seltype == Seltype_Multiple; }
        // get cols
        auto GetCols() const noexcept { return m_pCols; }
        // item removed. called from UIListItem's dtor
        //void ItemRemoved(UITreeItem&) noexcept;
        // select item
        void SelectItem(UITreeItem&, bool exadd) noexcept;
        // select to
        //void SelectTo(UITreeItem&) noexcept;
        // clear select
        void ClearSelected(UITreeItem&) noexcept;
        // clear all select
        void ClearAllSelected() noexcept;
        // get selected items
        auto&GetSelected() const noexcept { return m_selected; };
    public:
        // do normal event
        auto DoEvent(UIControl* sender, const EventArg& e) noexcept->EventAccept override;
        //// do mouse event
        //auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
        // update, postpone change some data
        void Update() noexcept override;
        //// render this control only, [Global rendering and Incremental rendering]
        //void Render() const noexcept override;
        //// recreate/init device(gpu) resource
        //auto Recreate() noexcept->Result override;
    protected:
        // add attribute
        void add_attribute(uint32_t key, U8View value) noexcept override;
        // add child
        void add_child(UIControl& child) noexcept override;
#ifdef LUI_ACCESSIBLE
        // accessible api
        auto accessible(const AccessibleEventArg& args) noexcept->EventAccept override;
#endif
        // select item
        void select_item(UITreeItem&) noexcept;
    protected:
        // cols
        UITreeCols*         m_pCols = nullptr;
        // last op
        UITreeItem*         m_pLastOp = nullptr;
        // selected
        ItemList            m_selected;
        // display row | xul::rows related
        uint16_t            m_displayRow = 0;
        // unused u8
        char                m_unusedU8 = 0;
        // select type
        AttributeSeltype    m_seltype = Seltype_Multiple;
    private:
    };
    // get meta info for UITree
    LUI_DECLARE_METAINFO(UITree);
}
