#pragma once
/**
* Copyright (c) 2014-2020 dustpg   mailto:dustpg@gmail.com
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

// config
#include "../luiconf.h"
// super class list
#include "../core/ui_node.h"
#include "../core/ui_object.h"
#include "../core/ui_core_type.h"
#include "../event/ui_event_host.h"
// id string
#include "../core/ui_const_sstring.h"
// vector
#include "../container/pod_vector.h"
// basic type
#include "../core/ui_basic_type.h"
// event
#include "../event/ui_gui_event.h"
#include "../event/ui_input_event.h"
#include "../event/ui_mouse_event.h"
#include "../event/ui_notice_event.h"
// style
#include "../style/ui_style.h"
#include "../style/ui_attribute.h"
// state
#include "../core/ui_control_state.h"
// renderer
#include "../graphics/ui_renderer_decl.h"
// accessible
#include "../accessible/ui_accessible.h"
// manager data
#include "../core/ui_manager_data.h"
// sp traversal
#include "../util/ui_sptraversal.h"

// assert
#include <cstddef>
#include <cassert>

#ifdef LUI_USER_U16STR_DATA
#include <core/ui_string.h>
#endif

// to save 2 pointer(16bytes on x64) for each control
#define LUI_CONTROL_USE_SINLENODE


// outer cotr helper
#define LUI_CONTROL_OUTER_CTOR(c) \
explicit c(UIControl* parent = nullptr) noexcept : c(c::s_meta) { this->final_ctor(parent); }

// ui namespace
namespace LongUI {
    // longui control alignas
    enum { CONTROL_ALIGNAS = 8 };
    // becarefully
    struct Unsafe;
    // meta info
    struct MetaControl;
    // control private function
    struct UIControlPrivate;
    // control control
    class CUIControlControl;
    // time capsule
    class CUITimeCapsule;
    // control
    class alignas(CONTROL_ALIGNAS) UIControl :
        public CUIEventHost,
        protected Node<UIControl>,
        public CUIObject {
        // super class
        using Super = void;
        // private impl
        friend UIControlPrivate;
        // friend
        friend CUIControlControl; friend CUIWindow;
        // Unsafe
        friend Unsafe;
    protected:
        // unique classes
        using UniqueClasses = POD::Vector<const char*>;
        // Itr
        using Iterator = Node<UIControl>::Iterator;
        // R-Itr
        using RIterator = Node<UIControl>::ReverseIterator;
        // C-Itr
        using CIterator = Node<const UIControl>::Iterator;
        // CR-Itr
        using CRIterator = Node<const UIControl>::ReverseIterator;
        // friend
        friend Node<UIControl>::Iterator;
        // friend
        friend CIterator;
        // friend
        friend Node<UIControl>::ReverseIterator;
        // friend
        friend CRIterator;
    public:
        // on this object clicked
        static constexpr auto _onClick() noexcept { return GuiEvent::Event_OnClick; }
#if 0
        // [unsupport yet]on this object double-clicked
        static constexpr auto _onDblClick() noexcept { return GuiEvent::Event_OnDblClick; }
#endif
        // on this object set focus
        static constexpr auto _onFocus() noexcept { return GuiEvent::Event_OnFocus; }
        // on this object kill focus
        static constexpr auto _onBlur() noexcept { return GuiEvent::Event_OnBlur; }
    public:
        // class meta
        static const  MetaControl   s_meta;
        // safe type cast, null this safe
        auto SafeCastTo(const MetaControl&) const noexcept->const UIControl*;
        // safe type cast, none const overload
        auto SafeCastTo(const MetaControl& m) noexcept {
            const auto* p = this; return const_cast<UIControl*>(p->SafeCastTo(m));
        }
        // Next MetaControl
        auto Next(const MetaControl&) noexcept -> UIControl&;
#ifndef NDEBUG
        // assert type cast, null this pointer acceptable
        void AssertCast(const MetaControl&) const noexcept;
#endif
    protected:
        // std lui ctrl ctor
        UIControl(const MetaControl&) noexcept;
    public:
        // no copy ctor
        UIControl(const UIControl&) = delete;
        // no move ctor
        UIControl(UIControl&&) = delete;
        // outer ctor for UIControl
        explicit UIControl(UIControl* parent = nullptr) noexcept : UIControl(UIControl::s_meta) { this->final_ctor(parent); }
        // delete later
        void DeleteLater() noexcept;
        // dtor
        virtual ~UIControl() noexcept;
        // do normal event
        virtual auto DoEvent(UIControl* sender, const EventArg& e) noexcept->EventAccept;
        // do input event
        virtual auto DoInputEvent(InputEventArg e) noexcept->EventAccept;
        // do mouse event
        virtual auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept;
        // update this. [state maybe State_NonChanged]
        virtual void Update(UpdateReason reason) noexcept;
        // render this control only
        virtual void Render() const noexcept;
        // recreate/init device(gpu) resource
        virtual auto Recreate(bool release_only) noexcept->Result;
    protected:
        // initialize this
        virtual void initialize() noexcept;
        // add child
        virtual void add_child(UIControl& child) noexcept;
        // get sub element ::before
        //virtual auto get_subelement(U8View name) noexcept->UIControl*;
        // add attribute : key = bkdr hashed string key, this method valid before inited
        virtual void add_attribute(uint32_t key, U8View value) noexcept;
        // add child ex-impl
        void insert_child(UIControl& child, UIControl& before) noexcept;
#ifdef LUI_ACCESSIBLE
    public:
        // get logic accessible control
        auto GetLogicAccessibleControl() const noexcept { return m_pAccCtrl; }
    protected:
        // accessible 
        virtual auto accessible(const AccessibleEventArg&) noexcept ->EventAccept;
        // friends
        friend class CUIAccessible;
        // accessible object
        CUIAccessible*          m_pAccessible = nullptr;
        // accessible control  null->one ctrl this->this child->child
        UIControl*              m_pAccCtrl = this;
#endif
    public:
#ifndef LUI_DISABLE_STYLE_SUPPORT
        // apply value
        void ApplyValue(const SSValue&) noexcept;
        // get value[cannot get string yet]
        void GetValue(SSValue&) const noexcept;
#endif
        // call this before making lots of controls
        static void ControlMakingBegin() noexcept;
        // call this after lots of controls maked
        static void ControlMakingEnd() noexcept;
        // need update
        void NeedUpdate(UpdateReason) noexcept;
        // is first child?
        bool IsFirstChild() const noexcept;
        // is last child
        bool IsLastChild() const noexcept;
        // is descendant or sibling
        bool IsDescendantOrSiblingFor(const UIControl&) const noexcept;
    public:
        // clear appearance and all margin/padding/border
        void ClearAppearance() noexcept;
        // set xul-string as content
        // remarks: SetXul accepts null-terminated-string only
        // 注: SetXul目前只接受 NUL 结尾字符串
        void SetXul(const char* xul) noexcept;
        // set xul-file as content, return false if not-found
        bool SetXulFromFile(U8View url) noexcept;
        // find child via position 
        auto FindChild(const Point2F pos) noexcept->UIControl*;
        // swap child index
        void SwapChildren(UIControl& a, UIControl& b) noexcept;
        // get children count
        auto GetChildrenCount() const noexcept { return m_cChildrenCount; }
        // invalidate this control
        void Invalidate() noexcept;
        // is top level of tree? -> no parent
        bool IsTopLevel() const noexcept { return !m_pParent; }
        // ref style model
        auto&RefStyle() const noexcept { return m_oStyle; }
        // ref box model
        auto&RefBox() const noexcept { return m_oBox; }
        // ref style inherited state mask
        auto&RefInheritedMask() noexcept { return m_oStyle.inherited; }
    public:
        // is ancestor for this
        bool IsAncestorForThis(const UIControl& node) const noexcept;
        // get parent
        auto GetParent() const noexcept { return m_pParent; }
        // get window
        auto GetWindow() const noexcept { return m_pWindow; }
        // set a new logical-parent immediately, be careful about thread safety
        void SetParent(UIControl& parent) noexcept;
        // clear parent immediately, be careful about thread safety
        void SetParent(std::nullptr_t) noexcept { this->clear_parent(); }
        // set focus of this control, return true if set
        bool SetFocus() noexcept;
        // kill focus of this control
        void KillFocus() noexcept;
        // set as default/focus
        void SetAsDefaultAndFocus() noexcept;
        // focusable?
        auto IsFocusable() const noexcept { return m_state.focusable; }
        // tabstop? mustbe focusable
        auto IsTabstop() const noexcept { return m_state.tabstop; }
        // defaultable?
        auto IsDefaultable() const noexcept { return m_state.defaultable; }
        // is gui event to parent?
        auto IsGuiEvent2Parent() const noexcept { return m_state.gui_event_to_parent; }
        // is in delete later
        auto IsDeleteLater() const noexcept { return m_state.delete_later; }
        // mark delete later
        auto MarkDeleteLater() noexcept { m_state.delete_later = true; }
    public:
        // is vaild in layout?
        bool IsVaildInLayout() const noexcept;
        // get maxsize
        auto GetMaxSize() const noexcept { return m_oStyle.maxsize; }
        // get box width
        auto GetBoxWidth() const noexcept { return m_oBox.size.width; }
        // get box height
        auto GetBoxHeight() const noexcept { return m_oBox.size.height; }
        // get box size
        auto GetBoxSize() const noexcept { return m_oBox.size; }
        // get box limited size
        auto GetBoxLimitedSize() const noexcept->Size2F;
        // get box fitting size
        auto GetBoxFittingSize() const noexcept->Size2F;
        // get box ex-size
        auto GetBoxExSize() const noexcept->Size2F;
        // resize the box rect
        bool ResizeBox(Size2F size) noexcept;
        // init min size
        void InitMinSize(Size2F size) noexcept;
        // init size
        void InitSize(Size2F size) noexcept;
        // match layout
        void MatchLayout(const MetaControl&, UIControl& target, float matrix) noexcept;
    protected:
        // set size value - low p
        template<size_t i> void set_sizevalue_lp(float v) noexcept {
            if (!(m_oStyle.overflow_xex & uint8_t(4 << i))) i[&m_oStyle.fitting.width] = v;
        }
        // set fitting width - low p
        void set_fitting_width_lp(float v) noexcept { set_sizevalue_lp<0>(v); }
        // set fitting height - low p
        void set_fitting_height_lp(float v) noexcept { set_sizevalue_lp<1>(v); }
        // set limited width - low p
        void set_limited_width_lp(float v) noexcept { set_sizevalue_lp<2>(v); }
        // set fitting height - low p
        void set_limited_height_lp(float v) noexcept { set_sizevalue_lp<3>(v); }
        // set box rect minsize
        //void set_box_minsize(Size2F size) noexcept;
        // set box contect minsize
        //void set_contect_minsize(Size2F size) noexcept;
        // update fitting size
        void update_fitting_size(Size2F fitting, UIControl* ex) noexcept;
        // update fitting size
        void update_fitting_size(Size2F fitting) noexcept { update_fitting_size(fitting, m_pParent); }
    public:
        // set visible
        void SetVisible(bool visible) noexcept;
        // get access key
        auto GetAccessKey() const noexcept { return m_oStyle.accesskey; }
        // get id, default as ""
        auto GetID() const noexcept { return m_id; }
        // set tooltip text
        void SetTooltipText(U8View v) noexcept;
        // set tooltip text
        auto GetTooltipText() const noexcept { return m_strTooltipText.c_str(); }
        // set postion of control [Relative to parent]
        void SetPos(Point2F pos) noexcept;
        // get postion of control [Relative to parent]
        auto GetPos() const noexcept { return m_oBox.pos; }
        // get world matrix [Relative to window]
        auto&RefWorld() const noexcept { return m_mtWorld; }
        // get tree level
        auto GetLevel() const noexcept { return m_state.level; }
        // is enabled
        auto IsEnabled() const noexcept { return !(m_oStyle.state & State_Disabled); }
        // is disabled
        auto IsDisabled() const noexcept { return m_oStyle.state & State_Disabled; }
        // is focus?
        auto IsFocused() const noexcept { return m_oStyle.state & State_Focus; }
        // is visible
        bool IsVisible() const noexcept { return m_state.visible; }
        // is visible ex - check ancestor's visible
        bool IsVisibleEx() const noexcept;
        // is active
        auto IsActive() const noexcept { return m_oStyle.state & State_Active; }
        // set hidden
        void SetHidden(bool hidden) noexcept { this->SetVisible(!hidden); }
        // set this and all descendant enabled/disabled
        void SetDisabled(bool disabled) noexcept;
        // set this and all descendant enabled/disabled
        void SetEnabled(bool enable) noexcept { this->SetDisabled(!enable); }
        // start state animation [do not use this to change state, use SetXXX instead]
        void StartAnimation(StyleStateChange) noexcept;
        // start general animation

    public:
        // set timer [0~7]
        void SetTimer(uint32_t elapse, uint32_t id0_7) noexcept;
        // kill timer [0~7]
        void KillTimer(uint32_t id0_7) noexcept;
    public:
        // get style classes
        auto&RefStyleClasses() const noexcept { return m_classesStyle; }
        // get meta info
        auto&RefMetaInfo() const noexcept { return m_refMetaInfo; }
        // add style class
        void AddStyleClass(U8View) noexcept;
        // remove style class
        void RemoveStyleClass(U8View) noexcept;
    public:
        // window point inside border?
        bool IsPointInsideBorder(Point2F) const noexcept;
        // map rect to window
        void MapToWindow(RectF& rect) const noexcept;
        // map rect to parent
        void MapToParent(RectF& rect) const noexcept;
        // map point to window
        void MapToWindow(Point2F& point) const noexcept;
        // map point to parent
        void MapToParent(Point2F& point) const noexcept;
        // map rect from window
        void MapFromWindow(RectF& rect) const noexcept;
        // map rect from parent
        void MapFromParent(RectF& rect) const noexcept;
        // map point from window
        void MapFromWindow(Point2F& point) const noexcept;
        // map point from parent
        void MapFromParent(Point2F& point) const noexcept;
        // map point to window
        auto MapToWindowEx(Point2F&& point) const noexcept {
            this->MapToWindow(point); return point; }
    protected:
        // make text-font-offset from direct-child
        void make_offset_tf_direct(UIControl& child) noexcept;
        // make text-font-offset from indirect-child
        void make_offset_tf_indirect() noexcept { m_oStyle.offset_tf = 1; }
        // clear parent
        void clear_parent() noexcept;
        // remove child
        void remove_child(UIControl& child) noexcept;
        // calculate child index
        auto cal_child_index(const UIControl&, const MetaControl&) const noexcept->uint32_t;
        // calculate child via index
        auto cal_index_child(uint32_t, const MetaControl&) noexcept->UIControl*;
        // calculate child index
        template<typename T> auto cal_child_index(const UIControl&) const noexcept->uint32_t;
        // calculate child index
        template<typename T> auto cal_index_child(uint32_t) noexcept->UIControl*;
        // do tooltip
        auto do_tooltip(Point2F pos) noexcept->EventAccept;
        // release tooltip
        void release_tooltip() noexcept;
    private:
        // init
        void init() noexcept;
        // remove triggered
        void remove_triggered() noexcept;
        // setup init state
        void setup_init_state() noexcept;
#ifndef LUI_DISABLE_STYLE_SUPPORT
        // make off init data
        void make_off_initstate(UIControl* ,uint32_t, UniByte4[], UniByte8[]) const noexcept;
        // extra animation callback
        void extra_animation_callback(
            StyleStateChange changed,
            void* out_values, 
            void* out_blocks
        ) noexcept;
        // extra animation callback
        void extra_animation_callback(
            UIControl& trigger,
            const SSValues& values,
            void* out_values,
            bool set
        ) noexcept;
        // link style sheet
        void link_style_sheet() noexcept;
        // animation property filter
        auto animation_property_filter(void*) noexcept->uint32_t;
#endif
        // will change?
        bool will_change_state(StyleStateChange) const noexcept;
        // change the state
        void change_state(StyleStateChange) noexcept;
        // set new window
        void set_new_window(CUIWindow*) noexcept;
    protected:
        // make SpTraversal
        auto make_sp_traversal() noexcept ->SpTraversal;
        // final ctor
        void final_ctor(UIControl* parent) noexcept;
        // apply world transform to renderer
        void apply_world_transform() const noexcept;
        // apply clip rect
        void apply_clip_rect() const noexcept;
        // cancel clip rect
        void cancel_clip_rect() const noexcept;
        // native style render, return true if not rendered 
        bool native_style_render() const noexcept;
        // custom style render
        void custom_style_render() const noexcept;
        // custom style render
        void custom_style_size_changed() noexcept;
        // delete renderer
        void delete_renderer() noexcept;
        // exist basic animation ?
        auto exist_basic_animation() const noexcept { return m_state.in_basic_animation; }
        // clear basic animation
        void clear_basic_animation() noexcept { m_state.in_basic_animation = false; }
        // setup basic animation
        void setup_basic_animation() noexcept { m_state.in_basic_animation = true; }
    protected:
        // manager used data
        ManagerData             m_oManager = { };
        // state
        CtrlState               m_state;
        // child count
        uint32_t                m_cChildrenCount = 0;
        // parent using layout value
        uint32_t                m_uLayoutValue = 0;
        // id of control
        ULID                    m_id = { CUIConstShortString::EMPTY };
        // meta info of control
        const MetaControl&      m_refMetaInfo;
        // style model
        Style                   m_oStyle;
        // box model
        Box                     m_oBox;
        // world transform: do mapping
        Matrix3X2F              m_mtWorld;
        // child-control head node
        Node<UIControl>         m_oHead;
#ifndef LUI_CONTROL_USE_SINLENODE
        // child-control tail node
        Node<UIControl>         m_oTail;
#endif
        // children offset
        Point2F                 m_ptChildOffset;
        // parent
        UIControl*              m_pParent;
        // window
        CUIWindow*              m_pWindow = nullptr;
        // hovered child, always valid
        UIControl*              m_pHovered = nullptr;
        // clicked child, valid during in click-down-to-up
        UIControl*              m_pClicked = nullptr;
        // last end time capsule
        CUITimeCapsule*         m_pLastEnd = nullptr;
        // context menu
        const char*             m_pCtxCtrl = nullptr;
        // tooltip window
        const char*             m_pTooltipCtrl = nullptr;
        // tooltip text
        CUIConstShortString     m_strTooltipText;
        // style unique classes
        UniqueClasses           m_classesStyle;
#ifndef LUI_DISABLE_STYLE_SUPPORT
    private:
        // bg renderer
        CUIRendererBackground*  m_pBgRender = nullptr;
        // bd renderer
        CUIRendererBorder*      m_pBdRender = nullptr;
#endif
    public:
#ifdef LUI_USER_INIPTR_DATA
        // user int data, for user accessing
        std::intptr_t           user_data = 0;
#endif
#ifdef LUI_USER_U8STR_DATA
        // user str data, for user accessing
        CUIConstShortString     user_u8str;
#endif
#ifdef LUI_USER_U16STR_DATA
        // user str data, for user accessing
        CUIString               user_u16str;
#endif
#ifndef NDEBUG
        // debug name
        const char*             name_dbg = "";
        // set debug name
        void SetDebugName(const char* name) noexcept { name_dbg = name; }
#else
        // set debug name
        void SetDebugName(const char*) noexcept { }
#endif
    public:
#ifdef LUI_CONTROL_USE_SINLENODE
        // end iterator
        auto end()noexcept->Iterator { return{ static_cast<UIControl*>(&m_oHead) }; }
        // rbegin iterator
        auto rbegin()noexcept->RIterator { return{ static_cast<UIControl*>(m_oHead.prev) }; }
        // const end iterator
        auto end()const noexcept->CIterator { return{ static_cast<const UIControl*>(&m_oHead) }; }
        // rbegin iterator
        auto rbegin()const noexcept->CRIterator { return{ static_cast<const UIControl*>(m_oHead.prev) }; }
#else
        // end iterator
        auto end()noexcept->Iterator { return{ static_cast<UIControl*>(&m_oTail) }; }
        // rbegin iterator
        auto rbegin()noexcept->RIterator { return{ static_cast<UIControl*>(m_oTail.prev) }; }
        // const end iterator
        auto end()const noexcept->CIterator { return{ static_cast<const UIControl*>(&m_oTail) }; }
        // rbegin iterator
        auto rbegin()const noexcept->CRIterator { return{ static_cast<const UIControl*>(m_oTail.prev) }; }
#endif
        // begin iterator
        auto begin()noexcept->Iterator { return{ static_cast<UIControl*>(m_oHead.next) }; }
        // rend iterator
        auto rend()noexcept->RIterator { return{ static_cast<UIControl*>(&m_oHead) }; }
        // begin iterator
        auto begin()const noexcept->CIterator { return{ static_cast<const UIControl*>(m_oHead.next) }; }
        // rend iterator
        auto rend()const noexcept->CRIterator { return{ static_cast<const UIControl*>(&m_oHead) }; }
    public:
        // get child flex sum
        auto SumChildrenFlex() const noexcept -> float;
    protected:
        // mark world changed
        void mark_world_changed() noexcept;
        // is inited?
        bool is_inited() const noexcept { return m_state.inited; }
        // is in dirty list?
        bool is_in_dirty_list() const noexcept { return m_state.in_dirty_list; }
        // Synchronous Init Data
        static void sync_data(UIControl& ctrl, UIControl& parent) noexcept;
        // resize child
        static void resize_child(UIControl& child, Size2F size) noexcept;
        // set child fixed attachment
        static void set_child_fixed_attachment(UIControl& child) noexcept {
            child.m_state.attachment = Attachment_Fixed; }
    protected: // COPY FROM NODE
        // swap node
        static void SwapNode(UIControl& a, UIControl& b) noexcept;
        // swap A-B node
        static void SwapAB(UIControl& a, UIControl& b) noexcept;
    };
    // == operator
    inline bool operator==(const UIControl& a, const UIControl& b) noexcept {
        return &a == &b; }
    // get meta info
    template<typename T> const MetaControl& GetMetaInfo() noexcept;
    // get meta info through pointer
    template<typename T> const MetaControl& GetMetaInfoPtr(T*) noexcept {
        return GetMetaInfo<T>(); }
    // longui type cast
    template<typename T, typename U> inline T longui_cast(U* ptr) noexcept {
#ifndef NDEBUG
        ptr->AssertCast(GetMetaInfoPtr(static_cast<T>(nullptr)));
#endif
        return static_cast<T>(ptr);
    }
    // calculate child index
    template<typename T> auto UIControl::cal_child_index(
        const UIControl& c) const noexcept->uint32_t {
        return this->cal_child_index(c, GetMetaInfo<T>());
    }
    // calculate child via index
    template<typename T> auto UIControl::cal_index_child(
        uint32_t i) noexcept->UIControl* {
        return this->cal_index_child(i, GetMetaInfo<T>());
    }
    // calculate child via index [UIControl SP]
    template<> auto UIControl::cal_index_child<UIControl>(uint32_t i) noexcept->UIControl*;
    // longui safe cast
    template<typename T, typename U> inline T* uisafe_cast(U* ptr) noexcept {
        return static_cast<T*>(ptr->SafeCastTo(GetMetaInfo<T>())); }
    // decl meta control
#define LUI_DECLARE_METAINFO(C)\
    template<> inline const MetaControl& GetMetaInfo<C>() noexcept {\
        return C::s_meta; }\
    template<> inline const MetaControl& GetMetaInfo<const C>() noexcept {\
        return C::s_meta; }
    // get meta info for UIControl
    LUI_DECLARE_METAINFO(UIControl);
}

// move to top namespace
using LongUI::longui_cast;
// move to top namespace
using LongUI::uisafe_cast;
