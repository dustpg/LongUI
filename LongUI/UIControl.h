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
        auto GetName_fs() const noexcept {
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
        void SetEventCallBack(const wchar_t*, LongUI::Event, LongUIEventCallBack) noexcept;
    public:
        // get taking up width of control
        auto GetTakingUpWidth() const noexcept ->float;
        // get taking up height of control
        auto GetTakingUpHeight() const noexcept ->float;
        // get non-content width of control
        auto GetNonContentWidth() const noexcept ->float;
        // get taking up height of control
        auto GetNonContentHeight() const noexcept ->float;
        // change control draw size out of Update() method
        auto SetControlSizeChanged() noexcept { m_bControlSizeChanged = true; }
        // change control draw size in the Update() method
        auto ControlSizeChangeHandled() noexcept { m_bControlSizeChangeHandled = true; }
        // is control draw size changed?
        auto IsControlSizeChanged() const noexcept { return m_bControlSizeChanged; }
        // set new taking up width of control
        auto SetTakingUpWidth(float w) noexcept ->void LongUINoinline;
        // set new taking up height of control
        auto SetTakingUpHeight(float h) noexcept ->void LongUINoinline;
        // refresh the world transform
        void RefreshWorld() noexcept;
        // get taking up rect
        void __fastcall GetRectAll(D2D1_RECT_F&) const noexcept;
        // get border rect
        void __fastcall GetBorderRect(D2D1_RECT_F&) const noexcept;
        // get viewport rect
        void __fastcall GetViewRect(D2D1_RECT_F&) const noexcept;
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
        // control size changed, if you have handled it
        bool                    m_bControlSizeChangeHandled = false;
    public:
        // visible
        bool                    visible = true;
        // priority for render
        int8_t          const   priority = 0;
    public:
        // transform for world
        D2D1_MATRIX_3X2_F       world = D2D1::Matrix3x2F::Identity();
        // position of control's view
        D2D1_POINT_2F           view_pos = D2D1::Point2F();
        // size of viewport
        D2D1_SIZE_F             view_size = D2D1::SizeF(0.f, 0.f);
        // control current visible position(relative to world)
        D2D1_RECT_F             visible_rect = D2D1::RectF();
        // margin rect
        D2D1_RECT_F     const   margin_rect = D2D1::RectF();
        // flags, using const_cast to change in constructor, 
        // do not change in other method/function
        LongUIFlag      const   flags = LongUIFlag::Flag_None;
    protected:
        // width of border
        float                   m_fBorderWidth = 0.f;
        // color of border
        D2D1_COLOR_F            m_aBorderColor[STATUS_COUNT];
        // now color of border
        D2D1_COLOR_F            m_colorBorderNow = D2D1::ColorF(D2D1::ColorF::Black);
        // roundsize of border
        D2D1_SIZE_F             m_2fBorderRdius = D2D1::SizeF();
        // control name
        CUIString               m_strControlName;
    public:
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept;
    public:
        // get class name
        auto GetControlClassName(bool full = false) const noexcept ->const wchar_t*;
        // assert type casting
        void AssertTypeCasting(const IID& iid) const noexcept;
        // canbe casted to?
        auto IsCanbeCastedTo(const IID& iid) const noexcept ->bool;
        // assert type casting
        template<class T> void AssertTypeCastingT(T*) const noexcept { this->AssertTypeCasting(LongUI::GetIID<T>()); }
#else
        // canbe casted to?
        auto IsCanbeCastedTo(const IID& iid) const noexcept { UNREFERENCED_PARAMETER(iid); return false; }
        // get class name
        auto GetControlClassName(bool full = false) const noexcept ->const wchar_t* { UNREFERENCED_PARAMETER(full); return L""; };
        // assert type casting
        template<class T> void AssertTypeCastingT(T* ptr) const noexcept { UNREFERENCED_PARAMETER(ptr); }
        // assert type casting
        void AssertTypeCasting(IID& iid) const noexcept { UNREFERENCED_PARAMETER(iid); }
#endif
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
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIControl>() { 
        // {87EB711F-3B53-4B21-ABCD-C907E5C43F8D}
        static const GUID IID_LongUI_UIControl = { 
            0x87eb711f, 0x3b53, 0x4b21,{ 0xab, 0xcd, 0xc9, 0x7, 0xe5, 0xc4, 0x3f, 0x8d } 
        };
        return IID_LongUI_UIControl;
    }
#endif
}

