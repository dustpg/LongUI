#pragma once

// c++
#include <type_traits>
// ui
#include "ui_basic_type.h"
#include "ui_core_type.h"
#include "../style/ui_style_state.h"

// ui namespace
namespace LongUI {
    // detail namespace
    namespace detail {
        // cc size
        template<size_t> struct cc;
        // 32bit
        template<> struct cc<4> { enum { size = 16*6+24, align = 4 }; };
        // 64bit
        template<> struct cc<8> { enum { size = 24*6+32, align = 8 };  };
    }
    // private data for control control
    struct PrivateCC;
    // control
    class UIControl;
    // control control
    class CUIControlControl {
        // friend struct
        friend PrivateCC;
        // CUIXulStream
        struct CUIXulStream;
    public:
        // start animation
        void StartAnimation(UIControl&, StyleStateTypeChange) noexcept;
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
        // invalidate control: rect : window level rect
        void InvalidateControl(UIControl& ctrl/*, const RectF& rect*/) noexcept;
        // set xul dir
        void SetXULDir(const CUIString&) noexcept;
        // get xul dir
        auto GetXULDir() const noexcept -> const CUIString&;
    public:
        // make xul tree
        static bool MakeXUL(UIControl& ctrl, const char* xul) noexcept;
        // render 
        static void RecursiveRender(
            const UIControl& ctrl,
            const RectF region[],
            uint32_t length
        ) noexcept;
    protected:
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
    protected:
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

