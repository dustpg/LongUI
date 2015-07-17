#pragma once
/**
* Copyright (c) 2014-2015 dustpg   mailto:dustpg@gmail.com
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


// longui namespace
namespace LongUI {
    // popup menu
    // TODO: Add LongUI Popup Menu Support
    class CUIPopupMenu {
    public:
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
        };
        // prop for menu item
        struct ItemProperties {
            // style
            ItemStyle       style;
            // id for meta
            uint16_t        metaid;
            // id for item
            uint32_t        id;
            // text for item
            const wchar_t*  text;
            // menu
            CUIPopupMenu*   menu;
        };
        // item proc
        using ItemProc = void(*)(uint32_t index);
    public:
        // ctor
        CUIPopupMenu() noexcept { };
        // dtor
        ~CUIPopupMenu() noexcept { this->Destroy(); }
    public:
        // destroy this
        void Destroy() noexcept;
        // create a menu by default
        bool Create() noexcept;
        // show the popup menu
        void Show(HWND parent) noexcept;
        // create a menu by xml string
        bool Create(const char* xml) noexcept;
        // create a menu by xml node
        bool Create(pugi::xml_node node) noexcept;
        // append a item
        bool AppendItem(const ItemProperties&) noexcept;
    public:
        // is top level?
        //auto IsTopLevel() noexcept { return m_pParent == nullptr; }
        // show the popup menu
        auto Show(UIWindow* window) noexcept { return this->Show(window->GetHwnd()); }
        // set item proc
        auto SetItemProc(ItemProc proc) noexcept { m_pItemProc = proc; }
    private:
        //  parent menu
        //CUIPopupMenu*       m_pParent = nullptr;
        // handle to popup menu
        HMENU               m_hMenu = nullptr;
        // item proc
        ItemProc            m_pItemProc = nullptr;
    };
}