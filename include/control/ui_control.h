#pragma once
/**
* Copyright (c) 2014-2018 dustpg   mailto:dustpg@gmail.com
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

// super class list
#include "../core/ui_node.h"
#include "../core/ui_object.h"
#include "../core/ui_core_type.h"
#include "../event/ui_event_host.h"
#include "../style/ui_style_value.h"
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
#include "ui_control_state.h"
// renderer
#include "../graphics/ui_renderer_decl.h"
// config
#include "../luiconf.h"
// accessible
#include "../accessible/ui_accessible.h"

// assert
#include <cstddef>
#include <cassert>

#ifdef LUI_USER_U16STR_DATA
#include <core/ui_string.h>
#endif

// ui namespace
namespace LongUI {
    // meta info
    struct MetaControl;
    // time capsule
    class CUITimeCapsule;
    // control control
    class CUIControlControl;
    // control private function
    struct UIControlPrivate;
    // control
    class UIControl :
        public CUIEventHost,
        public CUIStyleValue,
        protected Node,
        public CUIObject {
        // super class
        using Super = void;
        // private impl
        friend UIControlPrivate;
        // friend
        friend CUIControlControl;
        // friend
        friend Node::Iterator<UIControl>;
        // friend
        friend Node::Iterator<const UIControl>;
        // friend
        friend Node::ReverseIterator<UIControl>;
        // friend
        friend Node::ReverseIterator<const UIControl>;
        // unique classes
        using UniqueClasses = POD::Vector<const char*>;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // safe type cast, null this safe
        auto SafeCastTo(const LongUI::MetaControl&) const noexcept->const UIControl*;
        // safe type cast, none const overload
        auto SafeCastTo(const LongUI::MetaControl& m) noexcept {
            const auto* p = this; return const_cast<UIControl*>(p->SafeCastTo(m));
        }
#ifndef NDEBUG
        // assert type cast, null this pointer acceptable
        void AssertCast(const LongUI::MetaControl&) const noexcept;
#endif
    protected:
        // std lui ctrl ctor
        UIControl(UIControl* parent, const MetaControl&) noexcept;
    public:
        // no copy ctor
        UIControl(const UIControl&) = delete;
        // no move ctor
        UIControl(UIControl&&) = delete;
        // ctor
        UIControl(UIControl* parent = nullptr) noexcept : UIControl(parent, UIControl::s_meta) {}
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
        // update, postpone change some data
        virtual void Update() noexcept;
        // render this control only, [Global rendering and Incremental rendering]
        virtual void Render() const noexcept;
        // recreate/init device(gpu) resource
        virtual auto Recreate(bool release_only) noexcept->Result;
    protected:
        // add child[child maybe in ctor, cannot call method except under UIControl]
        virtual void add_child(UIControl& child) noexcept;
        // get sub element ::before
        virtual auto get_subelement(U8View name) noexcept->UIControl*;
        // add attribute : key = bkdr hashed string key, this method valid before inited
        virtual void add_attribute(uint32_t key, U8View value) noexcept;
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
        // apply value
        void ApplyValue(SSValue) noexcept;
        // get value[cannot get string yet]
        void GetValue(SSValue&) const noexcept;
        // need update in this frame
        void NeedUpdate() noexcept;
        // need update in next frame
        void NextUpdate() noexcept;
        // need relayout in this freame
        void NeedRelayout() noexcept;
        // is first child?
        bool IsFirstChild() const noexcept;
        // is last child
        bool IsLastChild() const noexcept;
    public:
        // clear appearance and all margin/padding/border
        void ClearAppearance() noexcept;
        // set xul-string as content
        void SetXUL(const char* xul) noexcept;
        // find child via position 
        auto FindChild(const Point2F& pos) noexcept->UIControl*;
        // swap child index
        void SwapChildren(UIControl& a, UIControl& b) noexcept;
        // get children count
        auto GetCount() const noexcept { return m_cChildrenCount; }
        // invalidate this control
        void Invalidate() noexcept;
        // is ctor failed? if true you must delete/dtor it after ctor
        auto IsCtorFailed() const noexcept { return m_state.ctor_failed; }
        // is top level of tree? -> no parent
        bool IsTopLevel() const noexcept { return !m_pParent; }
        // get style model
        auto&GetStyle() const noexcept { return m_oStyle; }
        // get box model
        auto&GetBox() const noexcept { return m_oBox; }
    public:
        // is ancestor for this
        bool IsAncestorForThis(const UIControl& node) const noexcept;
        // get parent
        auto GetParent() const noexcept { return m_pParent; }
        // get window
        auto GetWindow() const noexcept { return m_pWindow; }
        // set a new parent
        void SetParent(UIControl& parent) noexcept;
        // set a new parent to null
        void SetParent(std::nullptr_t parent) noexcept { this->clear_parent(); }
        // set focus of this control, return true if set
        bool SetFocus() noexcept;
        // kill focus of this control
        void KillFocus() noexcept;
        // focusable?
        auto IsFocusable() const noexcept { return m_state.focusable; }
        // defaultable?
        auto IsDefaultable() const noexcept { return m_state.defaultable; }
        // is gui event to parent?
        auto IsGuiEvent2Parent() const noexcept { return m_state.gui_event_to_parent; }
        // is in delete later
        auto IsDeleteLater() const noexcept { return m_state.delete_later; }
        // mark delete later
        auto MarkDeleteLater() noexcept { m_state.delete_later = true; }
    public:
        // resize
        bool Resize(Size2F size) noexcept;
        // set fixed/style size
        void SetFixedSize(Size2F size) noexcept;
        // set fixed/style size
        void SetStyleSize(Size2F size) noexcept { this->SetFixedSize(size); }
        // set style minsize
        void SetStyleMinSize(Size2F size) noexcept;
        // set style maxsize
        void SetStyleMaxSize(Size2F size) noexcept;
        // get minsize
        auto GetMinSize() const noexcept->Size2F;
        // is vaild in layout?
        bool IsVaildInLayout() const noexcept;
        // get size
        auto GetSize() const noexcept { return m_oBox.size; }
        // get maxsize
        auto GetMaxSize() const noexcept { return m_oStyle.maxsize; }
    protected:
        // set box rect minsize
        void set_box_minsize(Size2F size) noexcept;
        // set box contect minsize
        void set_contect_minsize(Size2F size) noexcept;
    public:
        // set visible
        void SetVisible(bool visible) noexcept;
        // get access key
        auto GetAccessKey() const noexcept { return m_chAccessKey; }
        // get id
        auto GetID() const noexcept { return m_id; }
        // set postion of control [Relative to parent]
        void SetPos(Point2F pos) noexcept;
        // get postion of control [Relative to parent]
        auto GetPos() const noexcept { return m_oBox.pos; }
        // get world matrix [Relative to window]
        auto&GetWorld() const noexcept { return m_mtWorld; }
        // get tree level
        auto GetLevel() const noexcept { return m_state.level; }
        // is enabled
        bool IsEnabled() const noexcept { return !m_oStyle.state.disabled; }
        // is disabled
        bool IsDisabled() const noexcept { return m_oStyle.state.disabled; }
        // is visible
        bool IsVisible() const noexcept { return m_state.visible; }
        // set hidden
        void SetHidden(bool hidden) noexcept { this->SetVisible(!hidden); }
        // set this and all descendant enabled/disabled
        void SetDisabled(bool disabled) noexcept;
        // set this and all descendant enabled/disabled
        void SetEnabled(bool enable) noexcept { this->SetDisabled(!enable); }
        // start state animation [do not use this to change state, use SetXXX instead]
        void StartAnimation(StyleStateTypeChange) noexcept;
        // start general animation

    public:
        // get style classes
        auto&GetStyleClasses() const noexcept { return m_classesStyle; }
        // get meta info
        auto&GetMetaInfo() const noexcept { return m_refMetaInfo; }
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
        // clear parent
        void clear_parent() noexcept;
        // mark minsize changed
        void mark_window_minsize_changed() noexcept;
        // take clicked control
        auto take_clicked() noexcept -> UIControl*;
        // remove child
        void remove_child(UIControl& child) noexcept;
        // start animation: bottom-up
        void start_animation_b2u(StyleStateTypeChange) noexcept;
        // start animation: up-bottom
        void start_animation_children(StyleStateTypeChange) noexcept;
        // do mouse under atomicity
        auto mouse_under_atomicity(const MouseEventArg& e) noexcept ->EventAccept;
        // calculate child index
        auto calculate_child_index(const UIControl&) const noexcept->uint32_t;
        // calculate child at
        auto calculate_child_at(uint32_t index) noexcept->UIControl*;
    private:
        // init
        auto init() noexcept->Result;
        // setup init state
        void setup_init_state() noexcept;
        // extra animation callback
        void extra_animation_callback(StyleStateTypeChange, void*) noexcept;
        // start animation change
        bool start_animation_change(StyleStateTypeChange) noexcept;
        // animation property filter
        auto animation_property_filter(void*) noexcept -> uint32_t;
        // link style sheet
        void link_style_sheet() noexcept;
        // setup style values
        void setup_style_values() noexcept;
    protected:
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
        // delete renderer
        void delete_renderer() noexcept;
        // exist basic animation ?
        auto exist_basic_animation() const noexcept { return m_state.in_basic_animation; }
        // clear basic animation
        void clear_basic_animation() noexcept { m_state.in_basic_animation = false; }
        // setup basic animation
        void setup_basic_animation() noexcept { m_state.in_basic_animation = true; }
    protected:
        // state
        CtrlState               m_state;
        // child count
        uint32_t                m_cChildrenCount = 0;
        // id of control
        const char*             m_id = "";
        // meta info of control
        const MetaControl&      m_refMetaInfo;
        // style model
        Style                   m_oStyle;
        // box model
        Box                     m_oBox;
        // world transform: do mapping
        Matrix3X2F              m_mtWorld;
        // children offset
        Point2F                 m_ptChildOffset;
        // child-control head node
        Node                    m_oHead;
        // child-control tail node
        Node                    m_oTail;
        // parent
        UIControl*              m_pParent;
        // window
        CUIWindow*              m_pWindow = nullptr;
        // hovered child, always right
        UIControl*              m_pHovered = nullptr;
        // clicked child, right during in click-down-to-up
        UIControl*              m_pClicked = nullptr;
        // last end time capsule
        CUITimeCapsule*         m_pLastEnd = nullptr;
        // style unique classes
        UniqueClasses           m_classesStyle;
    private:
        // bg renderer
        CUIRendererBackground*  m_pBgRender = nullptr;
        // bd renderer
        CUIBorderRender*        m_pBdRender = nullptr;
    protected:
        // parent accessible data
        uint32_t                m_uData4Parent = 0;
        // ununsed u16
        uint16_t                m_unused_u16 = 0;
        // accesskey char
        char                    m_chAccessKey = 0;
        // ununsed char
        char                    m_chUnused = 0;
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
        // end iterator
        auto end()noexcept->Iterator<UIControl> { return{ static_cast<UIControl*>(&m_oTail) }; }
        // begin iterator
        auto begin()noexcept->Iterator<UIControl> { return{ static_cast<UIControl*>(m_oHead.next) }; }
        // rend iterator
        auto rend()noexcept->ReverseIterator<UIControl> { return{ static_cast<UIControl*>(&m_oHead) }; }
        // rbegin iterator
        auto rbegin()noexcept->ReverseIterator<UIControl> { return{ static_cast<UIControl*>(m_oTail.prev) }; }
        // const end iterator
        auto end()const noexcept->Iterator<const UIControl> { return{ static_cast<const UIControl*>(&m_oTail) }; }
        // begin iterator
        auto begin()const noexcept->Iterator<const UIControl> { return{ static_cast<const UIControl*>(m_oHead.next) }; }
        // rend iterator
        auto rend()const noexcept->ReverseIterator<const UIControl> { return{ static_cast<const UIControl*>(&m_oHead) }; }
        // rbegin iterator
        auto rbegin()const noexcept->ReverseIterator<const UIControl> { return{ static_cast<const UIControl*>(m_oTail.prev) }; }
    protected:
        // ctor failed if
        void ctor_failed_if(const void* ptr) noexcept { if (!ptr) m_state.ctor_failed = true; }
        // size change handled
        void size_change_handled() noexcept { m_state.dirty = false; }
        // add into update list
        void add_into_update_list() noexcept { m_state.in_update_list = true; }
        // remove from update list
        void remove_from_update_list() noexcept { m_state.in_update_list = false; }
        // is inited?
        bool is_inited() const noexcept { return m_state.inited; }
        // is need relayout
        bool is_need_relayout() const noexcept { return m_state.dirty; }
        // is size changed?
        bool is_size_changed() const noexcept { return m_state.dirty; }
        // is in update list?
        bool is_in_update_list() const noexcept { return m_state.in_update_list; }
        // is in dirty list?
        bool is_in_dirty_list() const noexcept { return m_state.in_dirty_list; }
        // mark child world changed
        static void mark_child_world_changed(UIControl& c) noexcept { c.m_state.world_changed = true; }
        // resize child
        static void resize_child(UIControl& child, Size2F size) noexcept;
        // set child fixed attachment
        static void set_child_fixed_attachment(UIControl& child) noexcept {
            child.m_state.attachment = Attachment_Fixed; }
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
