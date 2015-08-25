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

// Vertical Layout



// LongUI namespace
namespace LongUI{
    // Vertical Layout -- 垂直布局 容器
    class UIVerticalLayout : public UIContainer {
        // 父类申明
        using Super = UIContainer;
    public: // UIControl
        // render this
        //virtual auto Render() noexcept ->HRESULT override;
        // update this
        virtual void Update() noexcept override;
        // do event 
        //virtual bool DoEvent(LongUI::EventArgument&) noexcept override;
        // recreate
        //virtual auto Recreate() noexcept->HRESULT override;
        // clean this
        virtual void Cleanup() noexcept override;
    public:
        // create this
        static UIControl* WINAPI CreateControl(CreateEventType, pugi::xml_node) noexcept;
        // ctor
        UIVerticalLayout(pugi::xml_node node) noexcept:Super(node) { }
        // dtor
        ~UIVerticalLayout() noexcept = default;
        // no copy ctor
        UIVerticalLayout(const UIVerticalLayout&) = delete;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIVerticalLayout>() {
        // {3BE5198C-B922-4C99-827E-F0D08875B045}
        static const GUID IID_LongUI_UIVerticalLayout = {
            0x3be5198c, 0xb922, 0x4c99,{ 0x82, 0x7e, 0xf0, 0xd0, 0x88, 0x75, 0xb0, 0x45 } 
        };
        return IID_LongUI_UIVerticalLayout;
    }
#endif
}