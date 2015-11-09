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
    auto WINAPI CreateNullControl(CreateEventType, pugi::xml_node) noexcept ->UIControl*;
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
        /// <summary>
        /// Cleanups this instance.
        /// </summary>
        /// <remarks>
        /// you should call dtor in this method, if malloc(ed), you should free it
        /// easy way: delete this
        /// </remarks>
        virtual void cleanup() noexcept = 0;
    public:
        // ui call from lambda/functor/function pointer
        template<typename T> auto AddEventCall(T call, SubEvent sb) noexcept {
            auto ok = this->uniface_addevent(sb, std::move(UICallBack(call)));
            assert(ok && "this control do not support this event!");
            return ok;
        }
        // get text
        auto GetText() noexcept ->const wchar_t* {
            EventArgument arg; arg.sender = this; arg.event = Event::Event_GetText;
            this->DoEvent(arg); return arg.str;
        }
        // set text
        void SetText(const wchar_t* txt) noexcept {
            EventArgument arg; arg.sender = this; arg.event = Event::Event_SetText;
            arg.stt.text = txt; this->DoEvent(arg);
        }
    public:
        // Render 
        virtual void Render() const noexcept = 0;
        // update
        virtual void Update() noexcept {
#ifdef _DEBUG
            void dbg_update(UIControl* c);
            dbg_update(this);
            assert(debug_updated == false && "cannot call this more than once");
            debug_updated = true;
#endif
        };
        // do event 
        virtual bool DoEvent(const EventArgument& arg) noexcept { UNREFERENCED_PARAMETER(arg); return false; };
        // do mouse event 
        virtual bool DoMouseEvent(const MouseEventArgument& arg) noexcept { UNREFERENCED_PARAMETER(arg); return false; };
        // recreate , first call or device reset
        virtual auto Recreate() noexcept ->HRESULT;
    protected:
        // [uniform interface]ui call
        virtual bool uniface_addevent(SubEvent sb, UICallBack&& call) noexcept { UNREFERENCED_PARAMETER(sb); UNREFERENCED_PARAMETER(call); return false; };
        // UIEvent
        bool call_uievent(const UICallBack& call, SubEvent sb) noexcept(noexcept(call.operator()));
        // delay_cleanup
        void delay_cleanup() noexcept;
        // render chain -> background
        void render_chain_background() const noexcept;
        // render chain -> background
        void render_chain_main() const noexcept {}
        // render chain -> background
        void render_chain_foreground() const noexcept;
    public:
        // do event helper
        bool DoLongUIEvent(LongUI::Event e) noexcept {
            EventArgument arg; arg.sender = this; arg.event = e;
            return this->DoEvent(arg);
        };
        // ctor
        UIControl(UIContainer* ctrlparent, pugi::xml_node node) noexcept;
        // delete the copy-ctor
        UIControl(const UIControl&) = delete;
        // dtor
        ~UIControl() noexcept;
        // after update
        void AfterUpdate() noexcept;
    public:
        // get window of control
        auto GetWindow() const noexcept { return m_pWindow; }
        // is top level? 
        auto IsTopLevel() const noexcept { return !this->parent; }
        // get script data
        const auto& GetScript() const noexcept { return m_script; }
        // get space holder control to aviod nullptr if you do not want a nullptr
        static auto GetPlaceholder() noexcept ->UIControl*;
    public:
        // set width fixed
        auto SetWidthFixed() noexcept { force_cast(this->flags) |= Flag_WidthFixed; }
        // set height fixed
        auto SetHeightFixed() noexcept { force_cast(this->flags) |= Flag_HeightFixed; }
        // get width of control
        auto GetWidth() const noexcept { return this->GetTakingUpWidth(); }
        // get height of control
        auto GetHeight() const noexcept { return this->GetTakingUpHeight(); }
        // is posterity for self?  
        bool IsPosterityForSelf(const UIControl*) const noexcept;
        // is successor for self?  
        bool IsSuccessorForSelf(const UIControl* test) const noexcept { return this->IsPosterityForSelf(test); }
        // get taking up width of control
        auto GetTakingUpWidth() const noexcept ->float;
        // get taking up height of control
        auto GetTakingUpHeight() const noexcept ->float;
        // get non-content width of control
        auto GetNonContentWidth() const noexcept ->float;
        // get taking up height of control
        auto GetNonContentHeight() const noexcept ->float;
        // change control layout
        auto SetControlLayoutChanged() noexcept { m_bool16.SetTrue(Index_ChangeLayout); }
        // handleupdate_marginal_controls control draw size changed
        auto ControlLayoutChangeHandled() noexcept { m_bool16.SetTrue(Index_ChangeSizeHandled); }
        // change control world
        auto SetControlWorldChanged() noexcept { m_bool16.SetTrue(Index_ChangeWorld); }
        // handle control world changed
        auto ControlWorldChangeHandled() noexcept { m_bool16.SetTrue(Index_ChangeWorldHandled); }
        // is control draw size changed?
        auto IsControlLayoutChanged() const noexcept { return m_bool16.Test(Index_ChangeLayout); }
        // refresh the world transform
        void RefreshWorld() noexcept;
        // refresh the world transform
        auto IsNeedRefreshWorld() const noexcept { return m_bool16.Test(Index_ChangeLayout) || m_bool16.Test(Index_ChangeWorld); }
        // update the world transform
        auto UpdateWorld() noexcept { if (this->IsNeedRefreshWorld()) this->RefreshWorld(); }
        // get HoverTrackTime
        auto GetHoverTrackTime() const noexcept { return m_cHoverTrackTime; }
        // get left of control
        auto __fastcall GetLeft() noexcept { return this->view_pos.x - this->margin_rect.left + m_fBorderWidth; };
        // get left of control
        auto __fastcall GetTop() noexcept { return this->view_pos.y - this->margin_rect.top + m_fBorderWidth; };
        // set left of control
        auto __fastcall SetLeft(float left) noexcept ->void;
        // set left of control
        auto __fastcall SetTop(float top) noexcept ->void;
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
        // get rect for cliping for parent
        void __fastcall GetClipRectFP(D2D1_RECT_F& rect) const noexcept;
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
        // updated
        bool                    debug_updated = false;
        // render_checker
        Helper::BitArray_8      debug_render_checker;
        // recreate count
        uint8_t                 debug_recreate_count = 0;
#endif
        // control name
        CUIWrappedCCP   const   name;
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
        // parent control
        UIContainer*    const   parent = nullptr;
        // render ancestry control, self or presuccessor/ancestry
        UIControl*      const   prerender = this;
        // using for UIContainerBuiltIn, prev control
        UIControl*      const   prev = nullptr;
        // using for UIContainerBuiltIn, next control
        UIControl*      const   next = nullptr;
        // weight for layout, interpreted by container
        float           const   weight = 1.f;
        // context for layout, interpreted by container
        float           const   context[2];
        // tree level
        uint8_t         const   level = 0ui8;
        // visible
        bool                    visible = true;
    protected:
        // boolx16
        Helper::BitArray16      m_bool16;
    public:
        // set state
        auto SetUserState(bool b) noexcept { m_bool16.SetTo(Index_StateUser, b); }
        // test state
        auto TestUserState() const noexcept { return m_bool16.Test(Index_StateUser); }
        // set state
        auto SetParentState(bool b) noexcept { m_bool16.SetTo(Index_StateParent, b); }
        // test state
        auto TestParentState() const noexcept { return m_bool16.Test(Index_StateParent); }
        // set state
        auto SetWindowState(bool b) noexcept { m_bool16.SetTo(Index_StateWindow, b); }
        // test state
        auto TestWindowState() const noexcept { return m_bool16.Test(Index_StateWindow); }
    public:
        // control current visible position(relative to world)
        D2D1_RECT_F             visible_rect = D2D1::RectF();
        // transform for world
        D2D1_MATRIX_3X2_F       world = DX::Matrix3x2F::Identity();
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
        // bit-array-index 16
        enum BitArrayIndex : uint32_t {
            // unknown
            Index_Unknown,
            // control size changed, for performance, this maybe changed multiple-time in one frame
            Index_ChangeLayout,
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
            // state for self no.1
            Index_StateSelf_1,
            // state for self no.2
            Index_StateSelf_2,
            // state for self no.3
            Index_StateSelf_3,
            // state for self no.4
            Index_StateSelf_4,
            // state for self no.5
            Index_StateSelf_5,
            // state for self no.6
            Index_StateSelf_6,
            // state for self no.7
            Index_StateSelf_7,
            // state for self no.8
            Index_StateSelf_8,
            // count of this
            BITARRAYINDEX_COUNT,
        };
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
    public:
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

