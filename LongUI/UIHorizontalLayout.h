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

// LongUI namespace
namespace LongUI{
    // Horizontal Layout -- 水平布局 容器
    class UIHorizontalLayout : public UIContainer {
        // 父类申明
        using Super = UIContainer;
    public: // UIControl
        // Render 渲染 
        //virtual auto Render() noexcept ->HRESULT override;
        // 刷新
        virtual void Update() noexcept override;
        // do event 事件处理
        //virtual bool DoEvent(LongUI::EventArgument&) noexcept override;
        // recreate 重建
        //virtual auto Recreate() noexcept->HRESULT override;
        // close this control 关闭控件
        virtual void Cleanup() noexcept override;
    public:
        // create 创建
        static UIControl* WINAPI CreateControl(CreateEventType type, pugi::xml_node) noexcept;
        // ctor
        UIHorizontalLayout(pugi::xml_node node) noexcept :Super(node) { }
        // dtor
        ~UIHorizontalLayout() noexcept = default;
        // 删除
        UIHorizontalLayout(const UIHorizontalLayout&) = delete;
    protected:
        // 修改子布局控件布局
        //void change_child_layout() noexcept;
        // 获取指定位置的控件
        //auto get_child_by_position(float) noexcept ->UIControl*;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIHorizontalLayout>() {
        // {E5CF04FC-1221-4E06-B6F3-315D45B1F2E6}
        static const GUID IID_LongUI_UIHorizontalLayout = {
            0xe5cf04fc, 0x1221, 0x4e06,{ 0xb6, 0xf3, 0x31, 0x5d, 0x45, 0xb1, 0xf2, 0xe6 } 
        };
        return IID_LongUI_UIHorizontalLayout;
    }
#endif
}