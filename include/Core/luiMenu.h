#pragma once
/**
* Copyright (c) 2014-2016 dustpg   mailto:dustpg@gmail.com
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

#include "../luibase.h"
#include "../Platless/luiPlUtil.h"


// longui namespace
namespace LongUI {
    // system menu handle
    struct SystemMenu; using HSystemMenu = SystemMenu*;
    // popup menu
    class CUIMenu {
    public:
        // item callback
        using ItemCallBack = CUIFunction<bool(size_t)>;
        // menu type
        enum MenuType : uint32_t {
            // system menu, will block thread
            Type_SystemMenu = 0,
            // longui menu[unsupported yet], custom skin, non-block
            Type_LongUIMenu,
        };
        // style for item
        enum ItemStyle : uint16_t {
            // default style
            Style_Null = 0,
            // item with meta in left of that
            Style_Meta = 1 << 0,
            // item with "Checked" maker
            Style_Checked = 1 << 1,
            // item is disabled, cannot be selected
            Style_Disabled = 1 << 2,
            // item is a menu
            Style_Menu = 1 << 3,
            // just Separator
            Style_Separator = 1 << 4,
        };
        // menu item
        struct Item {
            // style
            ItemStyle       style;
            // id for meta
            uint16_t        metaid;
            // id for item
            uint32_t        id;
            // text for item
            const wchar_t*  text;
            // data
            union {
                // menu
                CUIMenu*    menu;
                // menu
                size_t      index;
            };
        };
    public:
        // ctor
        CUIMenu() noexcept {}
        // dtor
        ~CUIMenu() noexcept { this->Destroy(); }
    public:
        // is ok
        bool IsOk() noexcept { return !!m_hMenu; }
        // destroy this
        void Destroy() noexcept;
        // create a menu by default
        bool Create() noexcept;
        // create a menu by xml string
        bool Create(const char* xml) noexcept;
        // create a menu by xml node
        bool Create(pugi::xml_node node) noexcept;
        // append a item
        bool AppendItem(const CUIMenu::Item&) noexcept;
        // show the popup menu
        void Show(XUIBaseWindow* wnd, /*OPTIONAL*/POINT* pos) noexcept;
        // add item call
        void AddItemCall(ItemCallBack c) noexcept { m_uiCall += std::move(c); }
        // add item call
        template<typename Lambda> 
        void AddItemCall(Lambda c) noexcept { m_uiCall += std::move(ItemCallBack(c)); }
    public:
        // is top level?
        //auto IsTopLevel() noexcept { return m_pParent == nullptr; }
        // set item proc
        //auto SetItemProc(ItemProc proc) noexcept { m_pItemProc = proc; }
        // get type
        auto GetType() const noexcept { return Type_SystemMenu; }
    private:
        //  parent menu
        //CUIMenu*       m_pParent = nullptr;
        // handle to popup menu for system
        HSystemMenu             m_hMenu = nullptr;
        // item call
        ItemCallBack            m_uiCall;
    };
    LONGUI_DEFINE_ENUM_FLAG_OPERATORS(CUIMenu::ItemStyle, uint16_t);
}