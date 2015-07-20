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
    // Container
    class UIContainer;
    // base control class -- 基本控件类
    class LongUIAlignas UIControl {
        // Super class
        using Super = void;
        // friend class
        friend class CUIManager;
        // friend class
        friend class UIContainer;
    public:
        // Render 
        virtual void Render(RenderType) const noexcept;
        // update
        virtual void Update() noexcept;
        // do event 
        virtual bool DoEvent(const LongUI::EventArgument&) noexcept = 0;
        // recreate , first call or device reset
        virtual auto Recreate(LongUIRenderTarget*) noexcept->HRESULT;
        // Wind up, you should call dtor in this method, if malloc(ed), you should free it
        // easy way: delete this
        virtual void Cleanup() noexcept = 0;
    public:
        // get control name for script
        LongUIInline auto GetName_fs() const noexcept {
            static char buffer[LongUIStringBufferLength];
            buffer[LongUI::WideChartoUTF8(m_strControlName.c_str(), buffer)] = L'\0';
            return static_cast<const char*>(buffer);
        }
    public:
        // ator
        UIControl(pugi::xml_node) noexcept;
        // dtor
        ~UIControl() noexcept;
        // delete the copy-ator
        UIControl(const UIControl&) = delete;
    protected:
        // new operator with buffer -- placement new 
        void* operator new(size_t s, void* buffer) noexcept { UNREFERENCED_PARAMETER(s); return buffer; };
        // delete -- placement new paired operator 配对用, 无实际用途
        void  operator delete(void* p, void* buffer) noexcept { UNREFERENCED_PARAMETER(p); UNREFERENCED_PARAMETER(buffer); /*nothing*/ };
        // delete new operator
        void* operator new(size_t) = delete;
        // delete new[] operator
        void* operator new[](size_t) = delete;
        // delete operator
        void  operator delete(void* p, size_t) { LongUI::CtrlFree(p); };
        // delete delete[] operator
        void  operator delete[](void*, size_t) = delete;
    public: // 内联区
        // control name 控件名称
        LongUIInline auto GetNameStr() const noexcept { return m_strControlName.c_str(); }
        // control name 控件名称
        LongUIInline auto&GetName() const noexcept { return m_strControlName; }
        // control name 控件名称
        LongUIInline auto&GetName() noexcept { return m_strControlName; }
        // get window of control
        LongUIInline auto GetWindow() const noexcept { return m_pWindow; }
        // XXX: is top level?
        LongUIInline auto IsTopLevel() const noexcept;
    protected: // Helper Zone
        // Set Event Call Back
        void SetEventCallBack(const wchar_t*, LongUI::Event, LongUICallBack) noexcept;
        // Set Event Call Back
        template<typename T>
        LongUIInline auto SetEventCallBackT(const wchar_t* n, LongUI::Event e, T call) noexcept {
            return this->SetEventCallBack(n, e, static_cast<LongUICallBack>(call));
        }
    public:
        // get taking up width of control
        auto GetTakingUpWidth() const noexcept ->float;
        // get taking up height of control
        auto GetTakingUpHeight() const noexcept ->float;
        // get non-content width of control
        auto GetNonContentWidth() const noexcept ->float;
        // get taking up height of control
        auto GetNonContentHeight() const noexcept ->float;
        // get class name
        auto GetControlClassName() const noexcept ->const wchar_t*;
        // change control draw size out of Update() method
        auto SetControlSizeChangedOutUpdate() noexcept { m_bControlSizeChanged = false; }
        // change control draw size in the Update() method
        auto SetControlSizeChangedInUpdate() noexcept { m_bControlSizeChangedEx = false; }
        // is control draw size changed?
        auto IsControlSizeChanged() noexcept { return m_bControlSizeChanged; }
        // set new taking up width of control
        auto SetTakingUpWidth(float w) noexcept ->void LongUINoinline;
        // set new taking up height of control
        auto SetTakingUpHeight(float h) noexcept ->void LongUINoinline;
        // get taking up rect/ clip rect
        void __fastcall GetClipRect(D2D1_RECT_F&) const noexcept;
        // get border rect
        void __fastcall GetBorderRect(D2D1_RECT_F&) const noexcept;
        // get draw rect
        void __fastcall GetContentRect(D2D1_RECT_F&) const noexcept;
        // get world transform matrix
        void __fastcall GetWorldTransform(D2D1_MATRIX_3X2_F& matrix) const noexcept;
    protected:
        // d2d render target
        LongUIRenderTarget*     m_pRenderTarget = nullptr;
        // index 0 brush
        ID2D1SolidColorBrush*   m_pBrush_SetBeforeUse = nullptr;
        // parent window
        UIWindow*               m_pWindow = nullptr;
        // script data
        UIScript                m_script;
    public:
        // parent control, using const_cast to change in constructor, 
        // do not change in other method/function
        UIContainer*  const     parent = nullptr;
        // [Retained]render related control, using const_cast to change in constructor, 
        // do not change in other method/function
        // UIControl*    const     related_retained = nullptr;
        // using for container, prev control
        UIControl*    const     prev = nullptr;
        // using for container, next control
        UIControl*    const     next = nullptr;
        // user data
        void*                   user_data = nullptr;
        // extend data, if extend_data_size, will point it
        // do not use it in UIWindow except you know it well
        void*                   extend_data = nullptr;
        // extend data size in byte, xml attribute "exdatasize"
        // do not use it in UIWindow except you know it well
        uint32_t                extend_data_size = 0;
    protected:
        // control size changed, for performance, this maybe changed multiple in one frame
        bool                    m_bControlSizeChanged = false;
        // control size changed, safe way to change in Update() method
        bool                    m_bControlSizeChangedEx = false;
        // the position of draw zone has been changed
        bool                    unused_bool_control = false;
    public:
        // priority for render
        int8_t       const      priority = 0;
    public:
        // x position of control
        float                   x = 0.f;
        // y position of control
        float                   y = 0.f;
        // content width of control
        float                   cwidth = 0.f;
        // content height of control
        float                   cheight = 0.f;
        // control current visible position(relative to world)
        D2D1_RECT_F             visible_rect = D2D1::RectF();
        // margin rect
        D2D1_RECT_F   const     margin_rect = D2D1::RectF();
        // flags, using const_cast to change in constructor, 
        // do not change in other method/function
        LongUIFlag    const     flags = LongUIFlag::Flag_None;
    protected:
        // size of border
        float                   m_fBorderSize = 0.f;
        // color of border
        D2D1_COLOR_F            m_aBorderColor[STATUS_COUNT];
        // now color of border
        D2D1_COLOR_F            m_colorBorderNow = D2D1::ColorF(D2D1::ColorF::Black);
        // roundsize of border
        D2D1_SIZE_F             m_fBorderRdius = D2D1::SizeF();
        // control name
        CUIString               m_strControlName;
    public:
        // make color form string
        static bool MakeColor(const char*, D2D1_COLOR_F&) noexcept;
        // make UIString form string
        static bool MakeString(const char*, CUIString&) noexcept;
        // make floats from string
        static bool MakeFloats(const char*, float*, int) noexcept;
        // get real control size in byte
        template<class T, class L>
        static LongUIInline auto AllocRealControl(pugi::xml_node node, L lam) noexcept {
            size_t exsize = 0;
            if (node) {
                exsize = LongUI::AtoI(node.attribute("exdatasize").value());
            }
            T* control = reinterpret_cast<T*>(
                LongUI::CtrlAlloc(sizeof(T) + exsize)
                );
            // check alignof
            assert(Is2Power(alignof(T)) && "alignas(Control) must be 2powered");
            assert(size_t(control) % alignof(T) == 0);
            // set align
            if (control) {
                lam(control);
                control->extend_data = reinterpret_cast<uint8_t*>(control) + sizeof(T);
                control->extend_data_size = exsize;
            }
            return control;
        }
    };
}

