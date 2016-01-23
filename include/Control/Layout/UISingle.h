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

// LongUI namespace
namespace LongUI {
    // single control class, UISingle always hosts a child
    class UISingle : public UIContainer {
        // super class
        using Super = UIContainer;
        // clean this control 清除控件
        virtual void cleanup() noexcept override;
    protected:
        // dtor
        ~UISingle() noexcept;
    public:
        // render this
        virtual void Render() const noexcept override;
        // update this
        virtual void Update() noexcept override;
        // do event
        virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
        // do mouse event
        //virtual bool DoMouseEvent(const LongUI::MouseEventArgument& arg) noexcept override;
        // recreate this
        virtual auto Recreate() noexcept ->HRESULT override;
        // find child control by mouse point
        virtual auto FindChild(const D2D1_POINT_2F& pt) noexcept ->UIControl* override final;
    public:
        // refresh this layout
        virtual void RefreshLayout() noexcept override final;
        // push back
        virtual void PushBack(UIControl* child) noexcept override final;
        // just remove 
        virtual void RemoveJust(UIControl* child) noexcept override final;
    public:
        // get single child
        auto GetSingleChild() const noexcept { return m_pChild; }
        // ctor
        UISingle(UIContainer* cp) noexcept : Super(cp) {}
        // create 创建
        static auto WINAPI CreateControl(CreateEventType type, pugi::xml_node) noexcept->UIControl*;
    protected:
        // init
        void initalize(pugi::xml_node node) noexcept { return Super::initialize(node); }
        // render chain -> background
        void render_chain_background() const noexcept { return Super::render_chain_background(); }
        // render chain -> mainground
        void render_chain_main() const noexcept;
        // render chain -> foreground
        void render_chain_foreground() const noexcept { return Super::render_chain_foreground(); }
    public: // for C++ 11
        // begin 
        auto begin() const noexcept { return &m_pChild; };
        // end
        auto end() const noexcept { return &m_pChild + 1; };
#ifdef _DEBUG
    private:
        // debug for_each
        virtual void debug_for_each(const CUIFunction<void(UIControl*)>& call) noexcept override {
            if (m_pChild) call(m_pChild);
        }
#endif
    protected:
        // single one
        UIControl*              m_pChild = UIControl::GetPlaceholder();
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
    };
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UISingle>() {
        // {B3C3CDEB-21A7-48E3-8E6C-693212ED7619}
        static const GUID IID_LongUI_UISingle = {
            0xb3c3cdeb, 0x21a7, 0x48e3, { 0x8e, 0x6c, 0x69, 0x32, 0x12, 0xed, 0x76, 0x19 }
        };
        return IID_LongUI_UISingle;
    }
#else
};
#endif
}