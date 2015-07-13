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
        // Render 渲染
        //virtual auto Render() noexcept ->HRESULT override;
        // 刷新
        virtual void Update() noexcept override;
        // do event 事件处理
        //virtual bool DoEvent(LongUI::EventArgument&) noexcept override;
        // recreate
        virtual auto Recreate(LongUIRenderTarget*) noexcept ->HRESULT override;
        // close this control 关闭控件
        virtual void WindUp() noexcept override;
    public:
        // create 创建
        static UIControl* WINAPI CreateControl(pugi::xml_node) noexcept;
        // UIVerticalLayout 构造函数
        UIVerticalLayout(pugi::xml_node node) noexcept:Super(node) { }
        // UIVerticalLayout 析构函数
        ~UIVerticalLayout() noexcept = default;
        // 删除
        UIVerticalLayout(const UIVerticalLayout&) = delete;
    protected:
        // 修改子布局控件布局
        //void change_child_layout() noexcept;
        // 获取指定位置的控件
        //auto get_child_by_position(float) noexcept ->UIControl*;
    };
}