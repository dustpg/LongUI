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
namespace LongUI {
    // create null control
    auto WINAPI CreateNullControl(CreateEventType, pugi::xml_node) noexcept->UIControl*;
    // Container
    class UIContainer;
    // base control class -- 基本控件类
    class alignas(sizeof(void*)) UIControl : public CUISingleNormalObject {
        // Super class
        using Super = void;// CUISingleNormalObject;
        // friend class
        friend class CUIManager;
        // friend class
        friend class UIContainer;
    public:
        // register ui call from lambda/functor/function pointer
        template<typename T> auto AddEventCall(T call, SubEvent sb) noexcept {
            auto ok = this->AddEventCall(sb, UICallBack(call));
            assert(ok && "this control do not support this event!");
            return ok;
        }
    public:
        // Render 
        virtual void Render(RenderType) const noexcept;
        // update
        virtual void Update() noexcept {};
        // do event 
        virtual bool DoEvent(const EventArgument& arg) noexcept { UNREFERENCED_PARAMETER(arg); return false; };
        // do mouse event 
        virtual bool DoMouseEvent(const MouseEventArgument& arg) noexcept { UNREFERENCED_PARAMETER(arg); return false; };
        // recreate , first call or device reset
        virtual auto Recreate() noexcept->HRESULT;
        // register ui call
        virtual bool AddEventCall(SubEvent sb, UICallBack& call) noexcept { UNREFERENCED_PARAMETER(sb); UNREFERENCED_PARAMETER(call); return false; };
    private:
        /// <summary>
        /// Cleanups this instance.
        /// </summary>
        /// <remarks>
        /// you should call dtor in this method, if malloc(ed), you should free it
        /// easy way: delete this
        /// </remarks>
        virtual void cleanup() noexcept = 0;
    public:
        // ctor
        UIControl(UIContainer* ctrlparent, pugi::xml_node node) noexcept;
        // delete the copy-ctor
        UIControl(const UIControl&) = delete;
        // dtor
        ~UIControl() noexcept;
        // after update
        void AfterUpdate() noexcept;
    public:
        // control name in const wchar_t*
        auto GetNameStr() const noexcept { return m_strControlName.c_str(); }
        // control name in longui string
        auto&GetName() noexcept { return m_strControlName; }
        // get window of control
        auto GetWindow() const noexcept { return m_pWindow; }
        // XXX: is top level? 
        auto IsTopLevel() const noexcept { return !this->parent; }
        // control name : overload for const
        const auto&GetName() const noexcept { return m_strControlName; }
        // get script data
        const auto& GetScript() const noexcept { return m_script; }
    public:
        // get width of control
        auto GetWidth() const noexcept { return this->GetTakingUpWidth(); }
        // get height of control
        auto GetHeight() const noexcept { return this->GetTakingUpHeight(); }
        // get taking up width of control
        auto GetTakingUpWidth() const noexcept ->float;
        // get taking up height of control
        auto GetTakingUpHeight() const noexcept ->float;
        // get non-content width of control
        auto GetNonContentWidth() const noexcept ->float;
        // get taking up height of control
        auto GetNonContentHeight() const noexcept ->float;
        // change control draw size
        auto SetControlSizeChanged() noexcept { m_bool16.SetTrue(Index_ChangeSize); }
        // handleupdate_marginal_controls control draw size changed
        auto ControlSizeChangeHandled() noexcept { m_bool16.SetTrue(Index_ChangeSizeHandled); }
        // change control world
        auto SetControlWorldChanged() noexcept { m_bool16.SetTrue(Index_ChangeWorld); }
        // handle control world changed
        auto ControlWorldChangeHandled() noexcept { m_bool16.SetTrue(Index_ChangeWorldHandled); }
        // is control draw size changed?
        auto IsControlSizeChanged() const noexcept { return m_bool16.Test(Index_ChangeSize); }
        // refresh the world transform
        void RefreshWorld() noexcept;
        // refresh the world transform
        auto IsNeedRefreshWorld() const noexcept { return m_bool16.Test(Index_ChangeSize) || m_bool16.Test(Index_ChangeWorld); }
        // update the world transform
        auto UpdateWorld() noexcept { if (this->IsNeedRefreshWorld()) this->RefreshWorld(); }
        // get HoverTrackTime
        auto GetHoverTrackTime() const noexcept { return m_cHoverTrackTime; }
        // set left of control
        auto __fastcall SetLeft(float left) noexcept->void;
        // set left of control
        auto __fastcall SetTop(float top) noexcept->void;
        // set new taking up width of control
        auto __fastcall SetWidth(float width) noexcept ->void LongUINoinline;
        // set new taking up height of control
        auto __fastcall SetHeight(float height) noexcept ->void LongUINoinline;
        // get rect for cliping
        void __fastcall GetClipRect(D2D1_RECT_F& rect) const noexcept;
        // get border rect
        void __fastcall GetBorderRect(D2D1_RECT_F& rect) const noexcept;
        // get viewport rect
        void __fastcall GetViewRect(D2D1_RECT_F& rect) const noexcept;
    protected:
        // index 0 brush
        ID2D1SolidColorBrush*   m_pBrush_SetBeforeUse = nullptr;
        // backgroud bursh
        ID2D1Brush*             m_pBackgroudBrush = nullptr;
        // parent window
        UIWindow*               m_pWindow = nullptr;
        // script data
        ScriptUI                m_script;
    public:
#ifdef _DEBUG
        // context for debug
        uint32_t                debug_context = 0;
        // debug this control? will output some debug message that not too important(log)
        bool                    debug_this = false;
        // unused
        bool                    debug_unused[2];
        // recreate count
        uint8_t                 recreate_count = 0;
#endif
        // user ptr
        void*                   user_ptr = nullptr;
        // user data
        uint32_t                user_data = 0;
    protected:
        // backgroud bursh id
        uint16_t                m_idBackgroudBrush = 0;
        // hover track time, valid while > 0
        uint16_t                m_cHoverTrackTime = 0;
    public:
        // parent control       [adjusting]: if is the top level, how to set it
        UIContainer*    const   parent = nullptr;
        // using for container, prev control
        UIControl*      const   prev = nullptr;
        // using for container, next control
        UIControl*      const   next = nullptr;
        // weight for layout, interpreted by container
        float           const   weight = 1.f;
    protected:
        // bit-array-index 16
        enum BitArrayIndex : uint32_t {
            // control size changed, for performance, this maybe changed multiple-time in one frame
            Index_ChangeSize = 0,
            // control world changed, for performance, this maybe changed multiple-time in one frame
            Index_ChangeWorld,
            // control size changed, if you have handled it
            Index_ChangeSizeHandled,
            // control world changed, if you have handled it
            Index_ChangeWorldHandled,
            // state for user
            Index_StateUser,
            // state for parent
            Index_StateParent,
            // state for window
            Index_StateWindow,
        };
        // boolx16
        Helper::BitArray16      m_bool16;
    public:
        // set state
        auto SetUserState(bool b) noexcept { m_bool16.SetTo(Index_StateUser, b); }
        // test state
        auto TestUserState() noexcept { return m_bool16.Test(Index_StateUser); }
        // set state
        auto SetParentState(bool b) noexcept { m_bool16.SetTo(Index_StateParent, b); }
        // test state
        auto TestParentState() noexcept { return m_bool16.Test(Index_StateParent); }
        // set state
        auto SetWindowState(bool b) noexcept { m_bool16.SetTo(Index_StateWindow, b); }
        // test state
        auto TestWindowState() noexcept { return m_bool16.Test(Index_StateWindow); }
    public:
        // priority for rendering
        uint8_t         const   priority = Priority_Normal;
        // visible
        bool                    visible = true;
        // control current visible position(relative to world)
        D2D1_RECT_F             visible_rect = D2D1::RectF();
        // transform for world
        D2D1_MATRIX_3X2_F       world = D2D1::Matrix3x2F::Identity();
        // position of control's view
        D2D1_POINT_2F   const   view_pos = D2D1::Point2F();
        // size of viewport
        D2D1_SIZE_F     const   view_size = D2D1::SizeF(0.f, 0.f);
        // margin rect
        D2D1_RECT_F     const   margin_rect = D2D1::RectF();
        // flags, use const_cast to change in constructor function
        LongUIFlag      const   flags = LongUIFlag::Flag_None;
    protected:
        // width of border
        float                   m_fBorderWidth = 0.f;
        // now color of border
        D2D1_COLOR_F            m_colorBorderNow = D2D1::ColorF(0xFFACACAC);
        // roundsize of border
        D2D1_SIZE_F             m_2fBorderRdius = D2D1::SizeF();
        // control name
        CUIString               m_strControlName;
    protected:
        // SubEvent Call Helper
        bool subevent_call_helper(const UICallBack& call, SubEvent sb) noexcept(noexcept(call.operator()));
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

