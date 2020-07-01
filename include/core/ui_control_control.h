#pragma once

// c++
#include <type_traits>
// ui
#include "../luiconf.h"
#include "ui_basic_type.h"
#include "ui_core_type.h"
#include "../style/ui_style_state.h"
// time capsule
#include "ui_time_capsule.h"

// ui namespace
namespace LongUI {
    // detail namespace
    namespace detail {
        // cc size
        template<size_t> struct cc;
        // 32bit
        template<> struct cc<4> { enum { size = 16*4+32+4*8, align = 4 }; };
        // 64bit
        template<> struct cc<8> { enum { size = 24*4+40+8*8, align = 8 };  };
    }
    // locker
    class CUILocker;
    // control
    class UIControl;
    // node update
    struct ControlNode {
        // first node
        UIControl*      first;
        // last node
        UIControl*      last;
    };
    // style sheet
    class CUIStyleSheet;
    // basic animation
    struct ControlAnimationBasic;
    // control control
    class CUIControlControl {
        // friend struct
        struct Private;
        // after create time capsule
        static auto after_create_tc(CUITimeCapsule*, UIControl* ctrl) noexcept->CUITimeCapsule*;
        // refresh time capsule
        static void refresh_time_capsule(UIControl&, CUITimeCapsule&) noexcept;
    public:
#ifndef LUI_DISABLE_STYLE_SUPPORT
        // start extra animation
        void StartExtraAnimation(UIControl&, StyleStateChange) noexcept;
#endif
        // start basic animation
        void StartBasicAnimation(UIControl&, StyleStateChange) noexcept;
        // find basic animation
        auto FindBasicAnimation(const UIControl&) const noexcept -> const ControlAnimationBasic*;
    public:
        // create time capsule for control
        template<typename T>
        auto CreateTimeCapsule(T&& func, float total, UIControl* ctrl = nullptr) noexcept {
            return this->after_create_tc(detail::create<T>(total, std::move(func)), ctrl); }
        // dispose time capsule for control
        void DisposeTimeCapsule(UIControl& ctrl) noexcept;
        // refresh time capsule for control
        void RefreshTimeCapsule(UIControl& ctrl) noexcept;
    public:
        // add init list
        void AddInitList(UIControl& ctrl) noexcept;
        // add update list
        void AddUpdateList(UIControl& ctrl) noexcept;
        // control attached
        void ControlAttached(UIControl& ctrl) noexcept;
        // control disattached
        void ControlDisattached(UIControl& ctrl) noexcept;
        // invalidate control
        static void InvalidateControl(UIControl& ctrl, const RectF* rect=nullptr) noexcept;
    public:
        // set xul dir [internal use]
        void SetXulDir(U8View) noexcept;
        // get xul dir [internal use]
        auto GetXulDir() const noexcept -> U8View;
#ifndef LUI_DISABLE_STYLE_SUPPORT
        // add global css string
        void AddGlobalCssString(U8View) noexcept;
        // add global css file
        void AddGlobalCssFile(U8View file_name) noexcept;
        // get style sheet
        auto GetStyleSheet() const noexcept { return m_pStyleSheet; }
        // get style cached control list
        auto GetStyleCachedControlList() noexcept -> void*;
#endif
    public:
        // remove control from list
        static void RemoveControlInList(UIControl& ctrl, ControlNode& list, size_t offset) noexcept;
        // add control to list
        static void AddControlToList(UIControl& ctrl, ControlNode& list, size_t offset) noexcept;
        // make xul tree
        static bool MakeXul(UIControl& ctrl, const char* xul) noexcept;
        // render 
        static void RecursiveRender(
            const UIControl& ctrl,
            const RectF region[],
            uint32_t length
        ) noexcept;
    protected:
        // delete later
        void delete_later(UIControl& ctrl) noexcept;
        // delete control
        void delete_controls() noexcept;
        // update time capsule
        void update_time_capsule(float delta) noexcept;
        // has time capsule?
        bool has_time_capsule() const noexcept;
        // swap list
        void swap_init_list() noexcept;
        // update control
        void update_control_in_list() noexcept;
        // init control, return true if update-list not empty
        bool init_control_in_list() noexcept;
        // update count
        //auto update_count() noexcept ->uint32_t;
        // normal update
        void normal_update() noexcept;
        // dirty update
        void dirty_update() noexcept;
        // copy ctor
        CUIControlControl(const CUIControlControl&) noexcept = delete;
        // ctor
        CUIControlControl() noexcept;
        // dtor
        ~CUIControlControl() noexcept;
    private:
        // update basic animation
        void update_basic_animation(uint32_t delta) noexcept;
#ifndef LUI_DISABLE_STYLE_SUPPORT
        // update extra animation
        void update_extra_animation(uint32_t delta) noexcept;
        // do animation in formto list
        void do_animation_in_from_to(UIControl& ctrl) noexcept;
#endif
        // dispose all time capsule
        void dispose_all_time_capsule() noexcept;
    private:
        // time capsule head
        Node<CUITimeCapsule>    m_oHeadTimeCapsule;
        // time capsule tail
        Node<CUITimeCapsule>    m_oTailTimeCapsule;
    protected:
#ifndef LUI_DISABLE_STYLE_SUPPORT
        // style sheet
        CUIStyleSheet*          m_pStyleSheet = nullptr;
#endif
        // time tick
        uint32_t                m_dwTimeTick;
        // delta time(unit: ms)
        uint32_t                m_dwDeltaTime;
    private:
        // private data
        std::aligned_storage<
            detail::cc<sizeof(void*)>::size,
            detail::cc<sizeof(void*)>::align
        >::type                 m_private;
        // get cc
        auto&cc() noexcept { return reinterpret_cast<Private&>(m_private); }
        // get const cc
        auto&cc() const noexcept { return reinterpret_cast<const Private&>(m_private); }
    };
}

