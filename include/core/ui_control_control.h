#pragma once

// c++
#include <type_traits>
// ui
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
        template<> struct cc<4> { enum { size = 16*8+24, align = 4 }; };
        // 64bit
        template<> struct cc<8> { enum { size = 24*8+32, align = 8 };  };
    }
    // basic animation
    struct ControlAnimationBasic;
    // private data for control control
    struct PrivateCC;
    // style sheet
    class CUIStyleSheet;
    // control
    class UIControl;
    // control control
    class CUIControlControl {
        // friend struct
        friend PrivateCC;
        // CUIXulStream
        struct CUIXulStream;
        // after create time capsule
        static void after_create_tc(CUITimeCapsule*, UIControl* ctrl) noexcept;
        // refresh time capsule
        static void refresh_time_capsule(UIControl&, CUITimeCapsule&) noexcept;
    public:
        // start basic animation
        void StartBasicAnimation(UIControl&, StyleStateTypeChange) noexcept;
        // start extra animation
        void StartExtraAnimation(UIControl&, StyleStateTypeChange) noexcept;
        // find basic animation
        auto FindBasicAnimation(const UIControl&) const noexcept -> const ControlAnimationBasic*;
    public:
        // create time capsule for control
        template<typename T>
        void CreateTimeCapsule(T&& func, float total, UIControl* ctrl = nullptr) noexcept {
            this->after_create_tc(impl::create<T>(total, std::move(func)), ctrl); }
        // dispose time capsule for control
        void DisposeTimeCapsule(UIControl& ctrl) noexcept;
    public:
        // add init list
        void AddInitList(UIControl& ctrl) noexcept;
        // add update list
        void AddUpdateList(UIControl& ctrl) noexcept;
        // add next update list
        void AddNextUpdateList(UIControl& ctrl) noexcept;
        // control attached
        void ControlAttached(UIControl& ctrl) noexcept;
        // control disattached
        void ControlDisattached(UIControl& ctrl) noexcept;
        // invalidate control
        void InvalidateControl(UIControl& ctrl) noexcept;
    public:
        // set xul dir
        void SetXulDir(U8View) noexcept;
        // get xul dir
        auto GetXULDir() const noexcept -> U8View;
        // add css string
        void AddCssString(U8View) noexcept;
        // add css file
        void AddCssFile(const char*) noexcept;
        // add css file
        void AddCssFile(const wchar_t*) noexcept;
        // get style sheet
        auto GetStyleSheet() const noexcept { return m_pStyleSheet; }
    public:
        // make xul tree
        static bool MakeXul(UIControl& ctrl, const char* xul) noexcept;
        // render 
        static void RecursiveRender(
            const UIControl& ctrl,
            const RectF region[],
            uint32_t length
        ) noexcept;
    protected:
        // update time capsule
        void update_time_capsule(float delta) noexcept;
        // has time capsule?
        bool has_time_capsule() const noexcept;
        // update control
        void update_control_in_list() noexcept;
        // init control, return true if update-list not empty
        bool init_control_in_list() noexcept;
        // update count
        //auto update_count() noexcept ->uint32_t;
        // push next update
        void push_next_update() noexcept;
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
        // update extra animation
        void update_extra_animation(uint32_t delta) noexcept;
        // dispose all time capsule
        void dispose_all_time_capsule() noexcept;
    private:
        // time capsule head
        Node                    m_oHeadTimeCapsule;
        // time capsule tail
        Node                    m_oTailTimeCapsule;
    protected:
        // style sheet
        CUIStyleSheet*          m_pStyleSheet = nullptr;
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
        auto&cc() noexcept { return reinterpret_cast<PrivateCC&>(m_private); }
        // get const cc
        auto&cc() const noexcept { return reinterpret_cast<const PrivateCC&>(m_private); }
    };
}

