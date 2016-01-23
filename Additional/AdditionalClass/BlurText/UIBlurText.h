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
    // d2d blur guid
    extern GUID CLSID_D2D1GaussianBlur;
    // bulr-able text control 可模糊的文本控件
    class UIBlurText : public UIText {
        // 父类申明
        using Super = UIText ;
        // clean this
        virtual void cleanup() noexcept override;
        // nothrow new 
        auto operator new(size_t size, const std::nothrow_t&) noexcept ->void* { 
            return LongUI::Additional::Alloc(size); 
        };
        // nothrow delete 
        auto operator delete(void* address, const std::nothrow_t&) ->void {
            return LongUI::Additional::Free(address);
        }
        // delete
        auto operator delete(void* address) noexcept ->void { 
            return LongUI::Additional::Free(address);
        }
    public:
        // Render 渲染
        virtual void Render() const noexcept override;
        // update 刷新
        virtual void Update() noexcept override;
        // do event 事件处理
        virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
        // recreate 重建
        virtual auto Recreate() noexcept ->HRESULT override;
    protected:
        // render chain -> background
        void render_chain_background() const noexcept { return Super::render_chain_background(); }
        // render chain -> mainground
        void render_chain_main() const noexcept { return Super::render_chain_main(); }
        // render chain -> foreground
        void render_chain_foreground() const noexcept { return Super::render_chain_foreground(); }
    public:
        // set blur
        void SetBlurValue(float b) noexcept { 
            m_fBlur = b;
            if(m_effect.IsOK()) m_effect.SetValue(
                D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, m_fBlur
                );
            m_pWindow->Invalidate(this);
        }
        // get blur
        auto GetBulrValue() const noexcept { return m_fBlur; }
        // create 创建
        static auto WINAPI CreateControl(CreateEventType, pugi::xml_node) noexcept ->UIControl*;
        // ctor: cp- parent in contorl-level
        UIBlurText(UIContainer* cp) noexcept : Super(cp), m_effect(CLSID_D2D1GaussianBlur) {}
    protected:
        // initialize, maybe you want call v-method
        void initialize(pugi::xml_node node) noexcept;
        // dtor
        ~UIBlurText() noexcept { }
        // copy ctor = delete
        UIBlurText(const UIBlurText&) = delete;
    protected:
        // blur effect
        Component::Effect               m_effect;
        // blur float
        float                           m_fBlur = 1.f;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIBlurText>() {
        // {1AF45E28-9342-4CA4-AA6E-E48048C5E7AE}
        static const GUID IID_LongUI_UIBlurText = { 
            0x1af45e28, 0x9342, 0x4ca4, { 0xaa, 0x6e, 0xe4, 0x80, 0x48, 0xc5, 0xe7, 0xae }
        };
        return IID_LongUI_UIBlurText;
    }
#endif
}