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
    // create null control
    auto WINAPI CreateNullControl(CreateEventType, pugi::xml_node) noexcept ->UIControl*;
    // Container
    class UIContainer;
    // config for creating control
    namespace Config { struct Control {
        // name for control
        const char* name;
    };}
    // base control class -- 基本控件类
    class LongUIAPI alignas(sizeof(void*)) UIControl : public CUISingleNormalObject {
        // Super class
        using Super = void;// CUISingleNormalObject;
        /// <summary>
        /// Cleanups this instance.
        /// </summary>
        /// <remarks>
        /// you should call dtor in this method, if malloc(ed), you should free it
        /// easy way: delete this
        /// </remarks>
        virtual void cleanup() noexcept = 0;
    public:
        // cleanup manually, BE CAREFUL TO USE THIS
        void CleanupManually() noexcept { this->cleanup(); }
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
        // get float
        auto GetFloatt() noexcept ->float {
            EventArgument arg; arg.sender = this; arg.event = Event::Event_GetFloat;
            this->DoEvent(arg); return arg.fvalue;
        }
        // set float
        void SetFloat(float value) noexcept {
            EventArgument arg; arg.sender = this; arg.event = Event::Event_SetFloat;
            arg.stf.value = value; this->DoEvent(arg);
        }
    public:
#ifdef _DEBUG
        // debug checker
        enum : uint32_t {
            DEBUG_CHECK_BACK = 0,
            DEBUG_CHECK_MAIN,
            DEBUG_CHECK_FORE,
            DEBUG_CHECK_INIT,
        };
#endif
        // Render 
        virtual void Render() const noexcept = 0;
        // update
        virtual void Update() noexcept {
#ifdef _DEBUG
            void longui_dbg_update(UIControl* c);
            longui_dbg_update(this);
            assert(debug_checker.Test(DEBUG_CHECK_INIT) == true && "not be initialized yet");
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
        UIControl(UIContainer* parent) noexcept;
        // dtor
        ~UIControl() noexcept;
        // after update
        void AfterUpdate() noexcept;
        // delete the copy-ctor
        UIControl(const UIControl&) = delete;
    protected:
        // initialize, maybe you want call v-method
        void initialize(pugi::xml_node node) noexcept;
        // initialize without xml-node
        void initialize() noexcept;
        // initialize with config
        void initialize(const Config::Control& cc) noexcept;
        // something must do before deleted
        void before_deleted() noexcept {
#ifdef _DEBUG
            if (this->debug_this || true) {
                UIManager << DL_Log << this << L"deleted" << LongUI::endl;
            }
#endif
        }
    public:
        // get window of control
        auto GetWindow() const noexcept { return m_pWindow; }
        // is top level? 
        auto IsTopLevel() const noexcept { return !this->parent; }
        // Link new parent
        void LinkNewParent(UIContainer*) noexcept;
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
        auto SetControlLayoutChanged() noexcept { m_state.SetTrue(State_ChangeLayout); }
        // handleupdate_marginal_controls control draw size changed
        auto ControlLayoutChangeHandled() noexcept { m_state.SetTrue(State_ChangeSizeHandled); }
        // change control world
        auto SetControlWorldChanged() noexcept { m_state.SetTrue(State_ChangeWorld); }
        // handle control world changed
        auto ControlWorldChangeHandled() noexcept { m_state.SetTrue(State_ChangeWorldHandled); }
        // is control draw size changed?
        auto IsControlLayoutChanged() const noexcept { return m_state.Test(State_ChangeLayout); }
        // refresh the world transform
        void RefreshWorld() noexcept;
        // refresh the world transform
        auto IsNeedRefreshWorld() const noexcept { return m_state.Test(State_ChangeLayout) || m_state.Test(State_ChangeWorld); }
        // update the world transform
        auto UpdateWorld() noexcept { if (this->IsNeedRefreshWorld()) this->RefreshWorld(); }
        // get HoverTrackTime
        auto GetHoverTrackTime() const noexcept { return m_cHoverTrackTime; }
        // get left of control
        auto GetLeft() noexcept { return this->view_pos.x - (this->margin_rect.left + m_fBorderWidth); };
        // get left of control
        auto GetTop() noexcept { return this->view_pos.y - (this->margin_rect.top + m_fBorderWidth); };
        // set left of control
        auto SetLeft(float left) noexcept ->void;
        // set left of control
        auto SetTop(float top) noexcept ->void;
        // set new taking up width of control
        auto SetWidth(float width) noexcept ->void LongUINoinline;
        // set new taking up height of control
        auto SetHeight(float height) noexcept ->void LongUINoinline;
        // get rect for cliping
        void GetClipRect(D2D1_RECT_F& rect) const noexcept;
        // get border rect
        void GetBorderRect(D2D1_RECT_F& rect) const noexcept;
        // get viewport rect
        void GetViewRect(D2D1_RECT_F& rect) const noexcept;
        // get rect for cliping for parent
        void GetClipRectFP(D2D1_RECT_F& rect) const noexcept;
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
        Helper::BitArray_8      debug_checker;
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
    protected:
        // basic state
        enum BasicState : uint32_t {
            // visible
            State_Visible =0,
            // enabled
            State_Enabled,
            // state for user
            State_User,
            // state for parent
            State_Parent,
            // state for window
            State_Window,
            // state for self no.1
            State_Self1,
            // state for self no.2
            State_Self2,
            // state for self no.3
            State_Self3,
            // state for self no.4
            State_Self4,
            // state for self no.5
            State_Self5,
            // state for self no.6
            State_Self6,
            // state for self no.7
            State_Self7,
            // control size changed, for performance, this maybe changed multiple-time in one frame
            State_ChangeLayout,
            // control world changed, for performance, this maybe changed multiple-time in one frame
            State_ChangeWorld,
            // control size changed, if you have handled it
            State_ChangeSizeHandled,
            // control world changed, if you have handled it
            State_ChangeWorldHandled,
            // count of this
            STATE_COUNT,
        };
        // state
        Helper::BitArray16      m_state;
        // reference count. to avoid "Circular references", upper-level-control managed it
        uint8_t                 m_u8RefCount = 1;
    public:
        // Release
        void Release() noexcept;
#ifdef _DEBUG
        // AddRef
        void AddRef() noexcept;
#else
        // AddRef
        void AddRef() noexcept { ++m_u8RefCount; }
#endif
        // set visible
        void SetVisible(bool visible) noexcept { m_state.SetTo(State_Visible, visible); }
        // get visible
        auto GetVisible() const noexcept { return m_state.Test(State_Visible); }
        // get visible
        auto GetEnabled() const noexcept { return m_state.Test(State_Enabled); }
        // set enable state
        void SetEnabled(bool enabled) noexcept {
            EventArgument arg; arg.sender = this; arg.event = Event::Event_SetEnabled;
            arg.ste.enabled = enabled; this->DoEvent(arg);
            m_state.SetTo(State_Enabled, enabled);
        }
        // set state
        auto SetUserState(bool b) noexcept { m_state.SetTo(State_User, b); }
        // test state
        auto TestUserState() const noexcept { return m_state.Test(State_User); }
        // set state
        auto SetParentState(bool b) noexcept { m_state.SetTo(State_Parent, b); }
        // test state
        auto TestParentState() const noexcept { return m_state.Test(State_Parent); }
        // set state
        auto SetWindowState(bool b) noexcept { m_state.SetTo(State_Window, b); }
        // test state
        auto TestWindowState() const noexcept { return m_state.Test(State_Window); }
    protected:
        // check control state based on basic state
        auto check_state() const noexcept { return this->GetEnabled() ? State_Normal : State_Disabled; }
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
        D2D1_COLOR_F            m_colorBorderNow = D2D1::ColorF(0xFFACACACui32);
        // roundsize of border
        D2D1_SIZE_F             m_2fBorderRdius = D2D1::SizeF();
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

