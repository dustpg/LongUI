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
    class LongUIAlignas LongUIAPI UIControl /*: public IUIInterface*/{
        // Super class
        using Super = IUIInterface;
        // friend class
        friend class CUIManager;
        // friend class
        friend class UIContainer;
    public: // for script method zone, easy-type to send/get
        // control name
        LongUIInline auto ForScript_GetName() const noexcept {
            static char buffer[LongUIStringBufferLength];
            buffer[LongUI::WideChartoUTF8(m_strControlName.c_str(), buffer)] = L'\0';
            return buffer;
        }
    public:
        // ator
        UIControl(pugi::xml_node) noexcept;
        // dtor
        ~UIControl() noexcept;
        // delete the copy-ator
        UIControl(const UIControl&) = delete;
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
                LongUICtrlAlloc(sizeof(T) + UIManager.user_context_size + exsize)
                    );
            // check alignof
            assert(Is2Power(alignof(T)) && "alignas(Control) must be 2powered");
            assert(size_t(control) % alignof(T) == 0);
            // set align
            if (control) {
                lam(control);
                control->extend_data = reinterpret_cast<uint8_t*>(control) + sizeof(T);
                control->extend_data_size = exsize;
                control->user_context = reinterpret_cast<uint8_t*>(control->extend_data) +
                    UIManager.user_context_size;
            }
            return control;
        }
    public:
        // Render 
        virtual void Render(RenderType) const noexcept;
        // update
        virtual void Update() noexcept;
        // do event 
        virtual bool DoEvent(LongUI::EventArgument&) noexcept = 0;
        // recreate , first call or device reset
        virtual auto Recreate(LongUIRenderTarget*) noexcept ->HRESULT;
        // close this control  DO NOT call Super::Close()
        virtual void Close() noexcept = 0;
    public:
        // DoEventEx, transed mouse point
        bool DoEventEx(LongUI::EventArgument&) noexcept;
    protected:
        // new operator with buffer -- placement new 
        void* operator new(size_t s, void* buffer) noexcept { return buffer; };
        // delete -- placement new paired operator 配对用, 无实际用途
        void  operator delete(void* p, void* buffer) noexcept { /*nothing*/ };
        // delete new operator
        void* operator new(size_t) = delete;
        // delete new[] operator
        void* operator new[](size_t) = delete;
        // delete operator
        void  operator delete(void* p, size_t) { LongUICtrlFree(p); };
        // delete delete[] operator
        void  operator delete[](void*, size_t) = delete;
    public: // 内联区
        // control name 控件名称
        LongUIInline auto GetNameStr() const noexcept { return m_strControlName.c_str(); }
        // control name 控件名称
        LongUIInline auto&GetName() const noexcept { return m_strControlName; }
        // control name 控件名称
        LongUIInline auto&GetName() noexcept { return m_strControlName; }
        // change control draw size
        LongUIInline auto DrawSizeChanged() noexcept { m_bDrawSizeChanged = true; }
        // change control draw pos
        LongUIInline auto DrawPosChanged() noexcept { m_bDrawPosChanged = true; }
        // get taking up width of control
        LongUIInline auto GetTakingUpWidth() const noexcept { return this->width + margin_rect.right + margin_rect.left; }
        // get taking up height of control
        LongUIInline auto GetTakingUpHeight() const noexcept { return this->height + margin_rect.bottom + margin_rect.top; }
        // get window of control
        LongUIInline auto GetWindow() const noexcept { return m_pWindow; }
    protected: // Helper Zone
        // Set Event Call Back
        void SetEventCallBack(const wchar_t*, LongUI::Event, LongUICallBack) noexcept;
        // Set Event Call Back
        template<typename T>
        LongUIInline auto SetEventCallBackT(const wchar_t* n, LongUI::Event e, T call) noexcept {
            return this->SetEventCallBack(n, e, static_cast<LongUICallBack>(call));
        }
    public:
        // get draw rect
        auto GetDrawRect() const noexcept->D2D1_RECT_F;
    protected:
        // d2d render target
        LongUIRenderTarget*     m_pRenderTarget = nullptr;
        // index 0 brush
        ID2D1SolidColorBrush*   m_pBrush_SetBeforeUse = nullptr;
        // parent window        所在窗口
        UIWindow*               m_pWindow = nullptr;
        // 脚本接口
        IUIScript*              m_pScript = nullptr;
    public:
        // using for container, prev control
        UIControl*    const     prev = nullptr;
        // using for container, next control
        UIControl*    const     next = nullptr;
        // user data
        union {
            void*               p = nullptr;
            size_t              i;
            float               f[sizeof(void*) / sizeof(float)];
        } user_data;
        // extend data
        void*                   extend_data = nullptr;
        // extend data size in byte
        uint32_t                extend_data_size = 0;
    protected:
        // the size of draw zone has been changed
        bool                    m_bDrawSizeChanged = false;
        // the position of draw zone has been changed
        bool                    m_bDrawPosChanged = false;
        // align for sizeof(void*)
        bool                    m_unused[2];
    protected:
        // user context
        void*                   user_context = nullptr;
    public:
        // transform, relative to parent
        D2D1_MATRIX_3X2_F       transform = D2D1::Matrix3x2F::Identity();
        // width of control
        float                   width = 0.f;
        // height of control
        float                   height = 0.f;
    protected: // border
        // color of border
        D2D1_COLOR_F            m_aBorderColor[STATUS_COUNT];
        // now color of border
        D2D1_COLOR_F            m_colorBorderNow = D2D1::ColorF(D2D1::ColorF::Black);
        // size of border
        float                   m_fBorderSize = 0.f;
        // reserved
        uint32_t                m_uBorderReserved = 0;
        // roundsize of border
        D2D1_SIZE_F             m_fBorderRdius = D2D1::SizeF();
    protected:
        // control name
        CUIString               m_strControlName;
        // script
        UIScript                m_script;
    public:
        // flags, using const_cast to change in constructor, 
        // do not change in other method/function
        LongUIFlag    const flags = LongUIFlag::Flag_None;
        // parent control, using const_cast to change in constructor, 
        // do not change in other method/function
        UIContainer*  const parent = nullptr;
        // [Retained]render related control, using const_cast to change in constructor, 
        // do not change in other method/function
        UIControl*    const related_retained = nullptr;
        // margin rect
        D2D1_RECT_F   const margin_rect = D2D1::RectF();
    public:
        // control current visible position(relative for world) , modified by 
        // parent, read only for self,
        D2D1_RECT_F         visible_rect = D2D1::RectF();
    };
#if 0
    // Template UIControl
    class TemplateControl : public LongUI::UIControl {
        // super class define
        using Super = LongUI::UIControl;
    public:
        // create 创建
        static UIControl* WINAPI CreateControl(pugi::xml_node node) noexcept {
            if (!node) {
                UIManager << DL_Warning << L"node null" << LongUI::endl;
            }
            // 申请空间
            auto pControl = LongUI::UIControl::AllocRealControl<LongUI::UIVerticalLayout>(
                node,
                [=](void* p) noexcept { new(p) UIVerticalLayout(node);}
            );
            if (!pControl) {
                UIManager << DL_Error << L"alloc null" << LongUI::endl;
            }
            return pControl;
    }
    public:
        // Render This Control
        virtual HRESULT Render(RenderType) noexcept;
        // do the event
        virtual bool    DoEvent(LongUI::EventArgument&) noexcept;
        // recreate resource
        virtual HRESULT Recreate(LongUIRenderTarget*) noexcept;
        // close this control
        virtual void    Close() noexcept { delete this; };
    protected:
        // constructor
        TemplateControl(pugi::xml_node node) noexcept :Super(node) {}
        // destructor
        ~TemplateControl() {}
    protected:
    };
#endif
}

