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
    class LongUIAPI UIHorizontalLayout : public UIContainer{
        // 父类申明
        using Super = UIContainer;
    public: // UIControl
        // Render 渲染 --- 放在第一位!
        //virtual auto LongUIMethodCall Render() noexcept ->HRESULT override;
        // do event 事件处理
        //virtual bool LongUIMethodCall DoEvent(LongUI::EventArgument&) noexcept override;
        // 预渲染
        //virtual void LongUIMethodCall PreRender() noexcept override;
        // recreate 重建
        virtual auto LongUIMethodCall Recreate(LongUIRenderTarget*) noexcept ->HRESULT override;
        // close this control 关闭控件
        virtual void LongUIMethodCall Close() noexcept override;
    public: // UIContainer
        // update children's layout
        virtual void LongUIMethodCall RefreshChildLayout(bool refresh_scroll) noexcept override;
    public:
        // create 创建
        static UIControl* WINAPI CreateControl(pugi::xml_node) noexcept;
        // UIVerticalLayout 构造函数
        UIHorizontalLayout(pugi::xml_node node) noexcept :Super(node) { }
        // UIVerticalLayout 析构函数
        ~UIHorizontalLayout() noexcept = default;
        // 删除
        UIHorizontalLayout(const UIHorizontalLayout&) = delete;
    protected:
        // 修改子布局控件布局
        //void LongUIMethodCall change_child_layout() noexcept;
        // 获取指定位置的控件
        //auto LongUIMethodCall get_child_by_position(float) noexcept ->UIControl*;
    };
}