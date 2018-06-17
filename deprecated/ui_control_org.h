#pragma once

#include <guiddef.h>
#include <iterator>
#include <cstdint>
#include <cstring>
#include <cassert>

#include "../util/pod_vector.h"
#include "../util/ui_string_view.h"
#include "../util/config.h"


// HANDLE FOR WINDOWS
#define LUI_DECLARE_HANDLE(name) struct name##__; using name = name##__ *;
LUI_DECLARE_HANDLE(HWND);
#undef LUI_DECLARE_HANDLE

// public api, include inline public called protected func
#define LAPI_PU 
// protected api
#define LAPI_PR

namespace UI {
    // control class
    class UIControl;

    // xml attribute
    struct XmlAttribute { PodStringView<char> key, value; };
    // event
    struct EventArg;
    // event
    struct MouseEventArg;
    // meta info
    //struct MetaInfo;
#ifndef NDEBUG
    // debug info
    struct DebugInfo;
#endif
    // color 4f
    struct ColorF;
    // Event Accept Type
    enum EventAccept : bool;
    // Gui event type
    enum class GuiEvent : uint32_t;
    // string
    using CUIString = std::wstring;
    // window base class
    class CUIWindow;
    // value of style sheet
    struct SSValue;
    // default max size
    enum : int32_t { DEFAULT_MAX_SIZE = 100000 };
    // value type
    enum class ValueType : uint32_t;
    // value unit
    enum class ValueUnit : uint32_t;
    // pseudo
    enum class PseudoType : uint8_t;
    // repeat
    enum AttributeRepeat : uint8_t;
    // repeat
    enum AttributeImageRepeat : uint8_t;
    // aligned box 
    enum AttributeBox : uint8_t;
    // border style 
    enum AttributeBStyle : uint8_t;
    // size case
    template<typename T, typename U> inline T size_cast(U u) noexcept {
        static_assert(sizeof(T) == sizeof(U), "bad size");
        return static_cast<T>(u);
    }
}


// TODO: remove this
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#else
// operator
namespace UI {
    // i8
    inline constexpr int32_t operator ""_i8(unsigned long long i) noexcept {
        return static_cast<int8_t>(i);
    }
    // ui8
    inline constexpr uint8_t operator ""_ui8(unsigned long long i) noexcept {
        return static_cast<uint8_t>(i);
    }
    // i16
    inline constexpr int16_t operator ""_i16(unsigned long long i) noexcept {
        return static_cast<int16_t>(i);
    }
    // ui16
    inline constexpr uint16_t operator ""_ui16(unsigned long long i) noexcept {
        return static_cast<uint16_t>(i);
    }
    // i32 
    inline constexpr int32_t operator ""_i32(unsigned long long i) noexcept {
        return static_cast<int32_t>(i);
    }
    // ui32 
    inline constexpr int32_t operator ""_ui32(unsigned long long i) noexcept {
        return static_cast<uint32_t>(i);
    }
    // i64
    inline constexpr int64_t operator ""_i64(unsigned long long i) noexcept {
        return static_cast<int64_t>(i);
    }
    // ui64
    inline constexpr uint64_t operator ""_ui64(unsigned long long i) noexcept {
        return static_cast<uint64_t>(i);
    }
}
#endif

namespace UI {
    // RGBA(ABGR in little-endian system, like x86)
    union RGBA { 
        // primitive type
        using type = uint32_t;
        // primitive data
        type                primitive;
        // right byte order
        struct { uint8_t    r, g, b, a; }; 
    };
    // ColorRGBA
    enum ColorRGBA : RGBA::type;
    // ColorF basic interface
    struct ColorFBI { float r, g, b, a; };

}

// ui::renderer namespace
namespace UI { namespace Renderer {
    // background renderer
    class Background;
}}

// ui namespace
namespace UI {
    // create event type
    enum CreateEventType : uintptr_t {
        // after register
        Type_AfterRegister,
        // recreate
        Type_Recreate,
        // before unregister
        Type_BeForeUnregister,
        // parent
        //Type_Parent
    };
    // create event
    using CreateEvent = uintptr_t(*)(uintptr_t);
    // meta info
    struct MetaInfo {
        // self type
        using Self = MetaInfo;
        // super class
        const Self* super_class;
        // class name
        const char* cl_name;
        // namespace name
        const char* ns_name;
        // element name
        const char* el_name;
        // create event
        CreateEvent create;
        // guid
        GUID        guid;
    };
}

// ui namespace
namespace UI {
    /// <summary>
    /// orientation
    /// </summary>
    enum AttributeOrient : bool {
        /// <summary>
        /// The orient horizontal
        /// </summary>
        Orient_Horizontal = false,
        /// <summary>
        /// The orient vertical
        /// </summary>
        Orient_Vertical = true,
    };
    /// <summary>
    /// attachment
    /// </summary>
    enum AttributeAttachment : bool {
        /// <summary>
        /// The attachment scroll
        /// </summary>
        Attachment_Scroll = false,
        /// <summary>
        /// The attachment fixed
        /// </summary>
        Attachment_Fixed = true,
    };
    /// <summary>
    /// direction
    /// </summary>
    enum AttributeDir : bool {
        /// <summary>
        /// The normal direction
        /// </summary>
        Dir_Normal = false,
        /// <summary>
        /// The reverse direction
        /// </summary>
        Dir_Reverse = true,
    };
    /// <summary>
    /// pack
    /// </summary>
    enum AttributePack : uint32_t {
        /// <summary>
        /// Child elements are placed starting from the left or top edge 
        /// of the box.If the box is larger than the total size of the 
        /// children, the extra space is placed on the right or bottom side.
        /// </summary>
        Pack_Start = 0,
        
        /// <summary>
        /// Extra space is split equally along each side of the child 
        /// elements, resulting the children being placed in the center of the box.
        /// </summary>
        Pack_Center,
                
        /// <summary>
        /// Child elements are placed on the right or bottom edge of 
        /// the box.If the box is larger than the total size of the 
        /// children, the extra space is placed on the left or top side.
        /// </summary>
        Pack_End
    };
    /// <summary>
    /// align
    /// </summary>
    enum AttributeAlign : uint32_t {
        /// <summary>
        /// Child elements are aligned starting from the left or top 
        /// edge of the box.If the box is larger than the total size of 
        /// the children, the extra space is placed on the right or bottom side.
        /// </summary>
        Align_Start = 0,
        /// <summary>
        /// Extra space is split equally along each side of the child elements, 
        /// resulting in the children being placed in the center of the box.
        /// </summary>
        Align_Center,
        /// <summary>
        /// Child elements are placed on the right or bottom edge of the 
        /// box.If the box is larger than the total size of the children, 
        /// the extra space is placed on the left or top side.
        /// </summary>
        Align_End,
        /// <summary>
        /// This value applies to horizontally oriented
        /// boxes only.It  causes the child elements to be aligned so that
        /// their text labels are lined up.
        /// </summary>
        Align_Baseline,
        /// <summary>
        /// This is the default value.The child elements are stretched to
        /// fit the size of the box.For a horizontal box, the children are
        /// stretched to be the height of the box.For a vertical box, the
        /// children are stretched to be the width of the box.
        ///
        /// if child element was fixed in other orientation, same as 'start'
        /// </summary>
        Align_Stretch,
    };
}


namespace UI {
    /// <summary>
    /// Basic Pseudo-classes
    /// </summary>
    struct PseudoClasses {
        // compatible type
        using CT = uint8_t;
        // CSS1:    active
        bool        active : 1;
        // CSS2:    hover
        bool        hover : 1;
        // CSS2:    focus
        bool        focus : 1;
        // CSS3:    empty[control which has child here]
        bool        empty : 1;
        // CSS3:    checked;
        bool        checked : 1;
        // CSS3:    indeterminate;
        bool        indeterminate : 1;
        // CSS3:    disabled
        bool        disabled : 1;
        // LUI:     -ui-selected
        bool        ui_selected : 1;
        // zero
        void Zero() noexcept { *reinterpret_cast<CT*>(this) = 0; }
        // PseudoClasses == PseudoClasses
        inline bool operator==(const PseudoClasses& x) const noexcept {
            return (*reinterpret_cast<const CT*>(this)) == reinterpret_cast<const CT&>(x);
        }
        // not
        inline auto operator!() const noexcept -> PseudoClasses {
            const CT x = !(*reinterpret_cast<const CT*>(this));
            return reinterpret_cast<const PseudoClasses&>(x);
        }
        // PseudoClasses & PseudoClasses
        inline auto operator&(const PseudoClasses& x) const noexcept -> CT {
            static_assert(sizeof(CT) == sizeof(PseudoClasses), "bad type");
            return *reinterpret_cast<const CT*>(this) & reinterpret_cast<const CT&>(x);
        }
    };
    // stylesheets: block
    struct SSBlock {
        // window hosted value buffer
        SSValue*        values;
        // length of value
        uint16_t        length;
        // yes pseudo-classes
        PseudoClasses   yes;
        // not pseudo-classes
        PseudoClasses   noo;
    };
    // stylesheets: selector
    struct SSSelector {
        // id - unique pointer in window
        const char*     idn;
        // class - unique pointer in program
        const char*     cln;
        // element - unique pointer in program
        const char*     ele;
        // index of value
        uint16_t        idx;
        // length of value
        uint16_t        len;
    };
    // stylesheets: blocks
    using SSBlocks = POD::Vector<SSBlock>;
    // stylesheets: values
    using SSValues = POD::Vector<SSValue>;
    // stylesheets: selectors
    using SSSelectors = POD::Vector<SSSelector>;
    // parse stylesheet, return false if failed to parse
    bool ParseStylesheet(const char*, SSValues&, SSSelectors&) noexcept;
}

namespace UI {
    /// <summary>
    /// ui object
    /// </summary>
    class CUIObject {
    public:
        // no-exception new
        void*operator new(std::size_t) = delete;
        // no-exception new[]
        void*operator new[](std::size_t) = delete;
        // delete []
        void operator delete[](void*, size_t size) noexcept = delete;
        // delete object
        void operator delete(void* ptr) noexcept { std::free(ptr); };
        // nothrow delete 
        void operator delete(void* ptr, const std::nothrow_t&) noexcept { std::free(ptr); };
    };
    // updatable object
    struct IUIUpdatable { virtual void Update() noexcept = 0; };
}


namespace UI {
}

namespace UI {
    /// <summary>
    /// L
    /// </summary>
    struct L {
        // get delta time in sec.
        static auto GetDeltaTime() noexcept -> float;
        // Updates the control in update list
        static void UpdateControlInList() noexcept;
        // add update list
        static void AddUpdateList(UIControl& ctrl) noexcept;
        // control attached
        static void ControlAttached(UIControl& ctrl) noexcept;
        // control disattached
        static void ControlDisattached(UIControl& ctrl) noexcept;
        // mark min size changed
        static void MarkMinSizeChanged(UIControl& ctrl) noexcept;
        // invalidate control: rect : window level rect
        static void InvalidateControl(UIControl& ctrl, const RectF& rect) noexcept;
        // get unique style class
        static auto GetUniqueStyleClass(PodStringView<char> pair) noexcept->const char*;
        // get unique control class/element name
        static auto GetUniqueControlClass(PodStringView<char> pair) noexcept->const char*;
        // get unique control class/element name
        static inline auto GetUniqueElementName(PodStringView<char> pair) noexcept->const char* {
            return GetUniqueControlClass(pair);
        }
        // recursive render contrl and its' child
        static void RecursiveRender(
            const UIControl& ctrl,
            const RectF region[] = nullptr,
            uint32_t length = 0
        ) noexcept;
    };
}

namespace UI {
    // signature
    template<typename signature> class CUIFunction;
    // Gui Event Listener
    using GuiEventListener = CUIFunction<EventAccept(UIControl& host)>;
    // is remove listener?
    inline bool IsRemoveListener(const GuiEventListener& l) { return !(&l); }
}

namespace UI {
    // detail namespace
    namespace detail {
        // func base
        template<typename Result, typename ...Args>
        class func : public CUIObject {
        public:
            // call
            virtual auto call(Args&&... args) noexcept->Result = 0;
            // dtor
            virtual ~func() noexcept { delete this->chain; };
            // own id
            uintptr_t   ownid;
            // call chain [next node]
            func*       chain = nullptr;
        };
        // func derived 
        template<typename Func, typename Result, typename ...Args>
        class func_ex final : public func<Result, Args...> {
            // cannot be function pointer
            static_assert(
                std::is_pointer<Func>::value == false,
                "cannot be function pointer because of type-safety"
                );
            // func data
            Func                m_func;
        public:
            // ctor
            func_ex(const Func &x, uintptr_t id) noexcept : m_func(x) { this->ownid = id; }
            // call
            auto call(Args&&... args) noexcept ->Result override {
                if (this->chain) this->chain->call(std::forward<Args>(args)...);
                return m_func(std::forward<Args>(args)...);
            }
        };
        // type helper
        template<typename Func> struct type_helper {
            using type = Func;
        };
        // type helper
        template<typename Result, typename ...Args> struct type_helper<Result(Args...)> {
            using type = Result(*)(Args...);
        };
        // uifunc helper
        struct uifunc_helper {
            // add chain, specialization for reducing code size
            static void add_chain_helper(GuiEventListener&, GuiEventListener&) noexcept;
            // add chain
            template<typename T> static inline void add_chain(T& a, T& b) noexcept {
                add_chain_helper(reinterpret_cast<GuiEventListener&>(a), reinterpret_cast<GuiEventListener&>(b));
            }
            // remove chain, specialization for reducing code size
            static void remove_chain_helper(GuiEventListener& gel, uintptr_t id) noexcept;
            // remove chian
            template<typename T> static inline void remove_chain(T& gel, uintptr_t id) noexcept {
                remove_chain_helper(reinterpret_cast<GuiEventListener&>(gel), id);
            }
        };
    }
    // UI Function, lightweight and chain-call-able version std::function
    template<typename Result, typename ...Args>
    class CUIFunction<Result(Args...)> {
    public:
        // friend
        friend detail::uifunc_helper;
        // this type
        using Self = CUIFunction<Result(Args...)>;
        // RealFunc pointer
        detail::func<Result, Args...>* m_pFunction = nullptr;
        // dispose
        void dispose() noexcept { delete m_pFunction; }
    public:
        // Ok
        bool IsOK() const noexcept { return !!m_pFunction; }
        // dtor
        ~CUIFunction() noexcept { this->dispose(); }
        // ctor
        CUIFunction() noexcept = default;
        // move ctor
        CUIFunction(Self&& obj) noexcept : m_pFunction(obj.m_pFunction) { 
            assert(&obj != this && "bad move"); obj.m_pFunction = nullptr; };
        // no copy ctor
        CUIFunction(const Self&) = delete;
        // operator =
        Self&operator=(const Self &x) noexcept = delete;
        // add call chain
        Self&operator += (Self&& chain) { this->AddCallChain(std::move(chain)); return *this; }
        // add call chain
        template<typename Func>
        Self& operator += (const Func &x) { this->AddCallChain(std::move(CUIFunction(x))); return *this; }
        // operator =
        template<typename Func> Self& operator=(const Func &x) noexcept {
            this->dispose(); m_pFunction = new(std::nothrow) detail::
            func_ex<typename detail::type_helper<Func>::type, Result, Args...>(x); return *this; }
        // operator =
        Self& operator=(Self&& x) noexcept {
            this->dispose(); std::swap(m_pFunction, x.m_pFunction); return *this; }
        // ctor with func
        template<typename Func> CUIFunction(const Func& f, uintptr_t ownid=0) noexcept : m_pFunction(
            new(std::nothrow) detail::func_ex<typename detail::type_helper<Func>::type, Result, Args...>(f, ownid)) { }
        // () operator
        auto operator()(Args&&... args) const noexcept -> Result {
            assert(m_pFunction && "bad call or oom"); 
            return m_pFunction ? m_pFunction->call(std::forward<Args>(args)...) : Result(); }
        // add call chain with ownid
        template<typename Func>
        void AddCallChain(const Func &x, uintptr_t ownid) { this->AddCallChain(std::move(CUIFunction(x, ownid))); }
        // add call chain
        void AddCallChain(Self&& chain) noexcept { detail::uifunc_helper::add_chain(*this, chain); }
        // remove call chain 
        void RemoveCallChain(uintptr_t ownid) noexcept { detail::uifunc_helper::remove_chain(*this, ownid); }
    };
}

namespace UI { namespace I {
    // text layout
    struct TextLayout;
    // renderer
    struct Renderer;
    // bitmap
    struct Bitmap;
    // font
    struct Font; 
}}

namespace UI {
    /// <summary>
    /// Result code
    /// </summary>
    struct Result {
        // code
        std::int32_t    code;
        // TODO: remove this
        // Commonly used code list
        enum CommonResult : int32_t { 
            RS_OK           = (int32_t)0x00000000, // Operation successful
            RE_NOTIMPL      = (int32_t)0x80004001, // Not implemented
            RE_NOINTERFACE  = (int32_t)0x80004002, // No such interface supported
            RE_POINTER      = (int32_t)0x80004003, // Pointer that is not valid
            RE_ABORT        = (int32_t)0x80004004, // Operation aborted
            RE_FAIL         = (int32_t)0x80004005, // Unspecified failure
            RE_UNEXPECTED   = (int32_t)0x8000FFFF, // Unexpected failure
            RE_ACCESSDENIED = (int32_t)0x80070005, // General access denied error
            RE_HANDLE       = (int32_t)0x80070006, // Handle that is not valid
            RE_OUTOFMEMORY  = (int32_t)0x8007000E, // Failed to allocate necessary memory
            RE_INVALIDARG   = (int32_t)0x80070057, // One or more arguments are not valid
        };
        /// <summary>
        /// Codes from wi N32.
        /// </summary>
        /// <param name="x">The x.</param>
        /// <returns></returns>
        static inline auto CodeFromWin32(uint32_t x) noexcept -> std::int32_t {
            constexpr uint32_t FACILITY_WIN32 = 7;
            return static_cast<std::int32_t>(x) <= 0 ? static_cast<std::int32_t>(x) :
                static_cast<std::int32_t>((x & 0x0000FFFFul) | (FACILITY_WIN32 << 16ul) | 0x80000000ul);
        }
        // operator bool 
        operator bool() const noexcept { return code >= 0; }
        // operator ! 
        bool operator !() const noexcept { return code < 0; }
        // operator == 
        bool operator ==(Result re) const noexcept { return code == re.code; }
        // operator != 
        bool operator !=(Result re) const noexcept { return code != re.code; }
        // from windows error code
        static inline Result FromWin32(uint32_t code) noexcept { return{ CodeFromWin32(code) }; }
        // from COM HRESULT
        static inline Result FromHRESULT(std::int32_t code) noexcept { return{ code }; };
    };
}

namespace UI {
    /// <summary>
    /// gui event type
    /// </summary>
    enum class GuiEvent : uint32_t {
        // command
        Event_Command = 0,
        // click
        Event_Click,
    };
    /// <summary>
    /// gui event host
    /// </summary>
    class CUIEventHost {
    public:
        // add gui event listener with ownid
        template<typename E, typename Callable>
        inline bool AddGuiEventListener(E e, uintptr_t ownid, Callable call) {
            static_assert(
                std::is_same<E, const char*>::value ||
                std::is_same<E, GuiEvent>::value,
                "e must be 'const char*' or 'GuiEvent'"
                );
            GuiEventListener listener{ call };
            auto result = this->add_gui_event_listener(ownid, e, std::move(listener));
            assert(result && "this control not support this gui event");
            return result;
        }
        // add gui event listener without ownid(ownid = 0)
        template<typename E, typename Callable>
        inline bool AddGuiEventListener(E e, Callable call) {
            return AddGuiEventListener(e, 0, call);
        }
        // remove gui event listener with enum-name-event
        void RemoveGuiEventListener(uintptr_t ownid, GuiEvent) noexcept;
        // remove gui event listener with string-name-event
        void RemoveGuiEventListener(uintptr_t ownid, const char* str) noexcept {
            RemoveGuiEventListener(ownid, strtoe(str));
        }
    private:
        // string to event
        static auto strtoe(const char*) noexcept->GuiEvent;
        // add gui event listener
        bool add_gui_event_listener(uintptr_t ownid, GuiEvent, GuiEventListener&&) noexcept;
        // add gui event listener
        bool add_gui_event_listener(uintptr_t ownid, const char* str, GuiEventListener&& l) noexcept {
            return add_gui_event_listener(ownid, strtoe(str), std::move(l));
        }
    };
    /// <summary>
    /// gui style value host
    /// </summary>
    class CUIStyleValue {
    public:
        // set background clip
        void SetBgClip(AttributeBox clip) noexcept;
        // set background color
        void SetBgColor(RGBA color) noexcept;
        // set background image from resource id
        void SetBgImage(uint32_t id) noexcept;
        // set background repeat
        void SetBgRepeat(AttributeRepeat ar) noexcept;
        // set background origin
        void SetBgOrigin(AttributeBox ab) noexcept;
        // set background attachment
        void SetBgAttachment(AttributeAttachment aa) noexcept;
    };
}

namespace UI {
    // control state
    struct alignas(sizeof(uint32_t)) CtrlState {
        // ctor
        void Init() noexcept;
        // tree level
        uint8_t     level;
        // [test]updated: updated in this frame, be setted in Update
        //bool        updated         : 1;
        // in update list
        bool        in_update_list  : 1;
        // child index changed
        bool        child_i_changed : 1;
        // dirty:   need re-calculate layout,  be setted if layout changed
        bool        dirty           : 1;
        // world matrix changed
        bool        world_changed   : 1;
        // layout orientation
        bool        layout_orient   : 1;
        // layout custom data
        bool        layout_custom   : 1;
        // layout direction
        bool        layout_dir      : 1;
        // directly managed
        bool        directly_managed: 1;
        // attachment 
        bool        attachment      : 1;
#ifndef NDEBUG
        // has been inited
        bool        inited_dbg      : 1;
#else
        bool        inited_holder   : 1;
#endif
        // unused
        char unused_[1];
    };
    // style state
    struct alignas(sizeof(uint32_t)) StyleState {
        // ctor
        void Init() noexcept;
        // visible
        bool        visible     : 1;
        // focusable
        bool        focusable   : 1;
        // disable
        bool        disabled    : 1;
        // hover
        bool        hover       : 1;
        // active, higher than hover
        bool        active      : 1;
        // focus
        bool        focus       : 1;
        // checked
        bool        checked     : 1;
        // indeterminate, higher than checked
        bool        indeterminate : 1;
        // unused
        char unused_[2];
    };
    // Box Model
    struct Box {
        // visible rect[world border edge]
        RectF       visible;
        // box position
        Point2F     pos;
        // box rect
        Size2F      size;
        // margin
        RectF       margin;
        // border
        RectF       border;
        // padding
        RectF       padding;
        // ctor
        void Init() noexcept;
        // get Non-content rect
        auto GetNonContect() const noexcept->RectF { RectF rc; GetNonContect(rc); return rc; }
        // get margin edge
        auto GetMarginEdge() const noexcept->RectF { RectF rc; GetMarginEdge(rc); return rc; }
        // get border edge
        auto GetBorderEdge() const noexcept->RectF { RectF rc; GetBorderEdge(rc); return rc; }
        // get padding edge
        auto GetPaddingEdge() const noexcept->RectF { RectF rc; GetPaddingEdge(rc); return rc; }
        // get content edge
        auto GetContentEdge() const noexcept->RectF { RectF rc; GetContentEdge(rc); return rc; }
        // get contect size
        auto GetContentSize() const noexcept->Size2F;
        // get Non-content rect
        void GetNonContect(RectF&) const noexcept;
        // get margin edge
        void GetMarginEdge(RectF&) const noexcept;
        // get border edge
        void GetBorderEdge(RectF&) const noexcept;
        // get padding edge
        void GetPaddingEdge(RectF&) const noexcept;
        // get content edge
        void GetContentEdge(RectF&) const noexcept;
    };
    // Style model
    struct Style {
        // ctor
        Style() noexcept;
        // dtor
        ~Style() noexcept;
        // state
        StyleState      state;
        // flex
        float           flex;
        // specified min size 
        Size2F          minsize_sp;
        // min size
        Size2F          minsize;
        // max size
        Size2F          maxsize;
        // fixed style
        SSBlocks        fixed;
    };
}

namespace UI {
    // control private data
    class PrivateControl;
    // control
    class UIControl : 
        public IUIUpdatable,
        public CUIObject,
        public CUIEventHost, 
        public CUIStyleValue,
        protected Node {
        // super class
        using Super = void;
        // friend
        friend L; friend PrivateControl;
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
        // guid data for this class
        static const GUID   s_guid;
    public:
        // no copy ctor
        UIControl(const UIControl&) = delete;
        // no move ctor
        UIControl(UIControl&&) = delete;
        // ctor
        UIControl(UIControl* parent = nullptr) noexcept;
        // dtor
        virtual ~UIControl() noexcept;
        // do normal event
        virtual auto DoEvent(UIControl* sender, const EventArg& e) noexcept->EventAccept;
        // do mouse event
        virtual auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept;
#ifndef NDEBUG
        // get debug infomation
        virtual bool RuntimeInfo_dbg(const DebugInfo& info) const noexcept;
#endif
        // update, postpone change some data
        virtual void Update() noexcept override;
        // render this control only, [Global rendering and Incremental rendering]
        virtual void Render() const noexcept;
        // recreate/init device(gpu) resource, first call must after-created
        virtual auto Recreate() noexcept->Result;
    protected:
        // add attribute
        virtual void add_attribute(const XmlAttribute&) noexcept;
        // add/remove gui event listener
        virtual bool set_gui_event_listener(GuiEvent, uintptr_t, GuiEventListener&&) noexcept;
    public:
        // apply value
        void ApplyValue(const SSValue&) noexcept;
        // need update in this frame
        void NeedUpdate() noexcept { L::AddUpdateList(*this); }
        // need relayout in this freame
        void NeedRelayout() noexcept { m_state.dirty = true; this->NeedUpdate(); }
    public:
        // find child via position 
        auto FindChild(const Point2F& pos) noexcept->UIControl*;
        // swap child index
        void SwapChildren(UIControl& a, UIControl& b) noexcept;
        // get children count
        auto GetCount() const noexcept -> uint32_t { return m_cChildrenCount; }
        // invalidate this control
        void Invalidate() noexcept { this->invalidate(nullptr); }
        // invalidate part of control
        void Invalidate(const RectF& rect) noexcept { this->invalidate(&rect); }
        // repaint this control
        void Repaint() noexcept { this->invalidate(nullptr); }
        // repaint part of control
        void Repaint(const RectF& rect) noexcept { this->invalidate(&rect); }
        // is top level of tree? -> no parent
        bool IsTopLevel() const noexcept { return !m_pParent; }
        // get style model
        auto GetStyle() const noexcept -> const Style&{ return m_oStyle; }
        // get box model
        auto GetBox() const noexcept -> const Box& { return m_oBox; }
    public:
        // [this pointer canbe null] init control after created
        auto Init(const Size2F* init_size = nullptr) noexcept->Result;
        // get parent
        auto GetParent() const noexcept ->UIControl* { return m_pParent; }
        // set a new parent
        void SetParent(UIControl& parent) noexcept;
        // force update children
        void ForceUpdateChildren() noexcept;
#ifdef DISABLABLE_OBJECT
        // add disablable object
        void AddDisableObject(NodeDisposeble& obj) noexcept;
        // remove disable object
        void RemoveDisableObject(NodeDisposeble& obj) noexcept;
#endif
    public:
        // resize
        bool Resize(Size2F size) noexcept;
        // set visible
        void SetVisible(bool visible) noexcept;
        // set postion of control [Relative to parent]
        void SetPos(const Point2F & pos) noexcept;
        // get postion of control [Relative to parent]
        auto GetPos() const noexcept -> Point2F { return m_oBox.pos; }
        // get world matrix [Relative to window]
        auto GetWorld() const noexcept -> const Matrix3X2F& { return m_mtWorld;  }
        // get tree level
        auto GetLevel() const noexcept -> uint32_t { return m_state.level; }
        // is enabled
        bool IsEnabled() const noexcept { return !m_oStyle.state.disabled; }
        // is disabled
        bool IsDisabled() const noexcept { return m_oStyle.state.disabled; }
        // is visible
        bool IsVisible() const noexcept { return m_oStyle.state.visible; }
        // set hidden
        void SetHidden(bool hidden) noexcept { this->SetVisible(!hidden); }
        // get size
        auto GetSize() const noexcept ->Size2F { return m_oBox.size; }
        // get max size
        auto GetMaxSize() const noexcept ->Size2F { return m_oStyle.maxsize; }
        // get min size
        auto GetMinSize() const noexcept ->Size2F { return m_oStyle.minsize; }
        // specify a min size
        void SpecifyMinSize(Size2F size) noexcept;
        // specify a max size
        void SpecifyMaxSize(Size2F size) noexcept;
        // is vaild in layout?
        bool IsVaildInLayout() const noexcept;
    public:
        // get style classes
        auto GetStyleClasses() const noexcept -> const UniqueClasses&{ return m_classesStyle; }
        // add style class
        void AddStyleClass(PodStringView<char>) noexcept;
        // remove style class
        void RemoveStyleClass(PodStringView<char>) noexcept;
    public:
        // map rect to window
        void MapToWindow(RectF& rect) const noexcept;
        // map rect to parent
        void MapToParent(RectF& rect) const noexcept;
        // map point to window
        void MapToWindow(Point2F& point) const noexcept;
        // map point to window
        void MapToParent(Point2F& point) const noexcept;
    protected:
        // add child
        void add_child(UIControl& child) noexcept;
        // invalidate rect of control
        void invalidate(const RectF* rect) noexcept;
        // remove child
        void remove_child(UIControl& child) noexcept;
        // add direct child(not pushed into children list)
        void add_direct_child(UIControl& child) noexcept;
    protected:
        // begin render
        void begin_render() const noexcept;
        // draw background color
        void draw_bkcolor() const noexcept;
        // draw background image
        void draw_bkimage() const noexcept;
        // draw border
        void draw_border() const noexcept;
    protected:
        // id of control
        const char*             m_pId;
        // box model
        Box                     m_oBox;
        // style model
        Style                   m_oStyle;
        // world transform: do mapping
        Matrix3X2F              m_mtWorld;
        // children offset
        Point2F                 m_ptChildOffset;
        // child-control head node
        Node                    m_oHead;
        // child-control tail node
        Node                    m_oTail;
#ifdef DISABLABLE_OBJECT
        // dis-object head node
        Node                    m_oHeadObj;
        // dis-object tail node
        Node                    m_oTailObj;
#endif
        // parent
        UIControl*              m_pParent;
        // hovered child
        UIControl*              m_pHovered = nullptr;
        // style unique classes
        UniqueClasses           m_classesStyle;
    private:
        // bgcolor renderer
        Renderer::Background*   m_pBgRender = nullptr;
    protected:
        // child count
        uint32_t                m_cChildrenCount = 0;
        // parent accessible data
        uint32_t                m_uData4Parent = 0;
        // window accessible data
        uint32_t                m_uData4Window = 0;
        // state
        CtrlState               m_state;
    public:
        // color
        //uint32_t        color = 0;
        // data for sorting, valid during sorting
        //int32_t         sort_data = 0;
        // user data, for user accessing
        std::intptr_t   user_data = 0;
#ifndef NDEBUG
        // debug name
        const char*     name_dbg = "";
#endif
    public:
        // end iterator
        auto end() ->Iterator<UIControl> { return{ static_cast<UIControl*>(&m_oTail) }; }
        // begin iterator
        auto begin() ->Iterator<UIControl> { return{ static_cast<UIControl*>(m_oHead.next) }; }
        // rend iterator
        auto rend() ->ReverseIterator<UIControl> { return{ static_cast<UIControl*>(&m_oHead) }; }
        // rbegin iterator
        auto rbegin() ->ReverseIterator<UIControl> { return{ static_cast<UIControl*>(m_oTail.prev) }; }
        // const end iterator
        auto end() const ->Iterator<const UIControl> { return{ static_cast<const UIControl*>(&m_oTail) }; }
        // begin iterator
        auto begin() const->Iterator<const UIControl> { return{ static_cast<const UIControl*>(m_oHead.next) }; }
        // rend iterator
        auto rend() const->ReverseIterator<const UIControl> { return{ static_cast<const UIControl*>(&m_oHead) }; }
        // rbegin iterator
        auto rbegin() const->ReverseIterator<const UIControl> { return{ static_cast<const UIControl*>(m_oTail.prev) }; }
    protected:
        // remove from update list
        void remove_from_update_list() { m_state.in_update_list = false; }
        // add into update list
        void add_into_update_list() { m_state.in_update_list = true; }
        // is in update list?
        bool is_in_update_list() const noexcept { return m_state.in_update_list; }
        // mark child world changed
        static void mark_child_world_changed(UIControl& c) noexcept { c.m_state.world_changed = true; }
        // resize child
        static void resize_child(UIControl& child, Size2F size) noexcept;
        // set child fixed attachment
        static void set_child_fixed_attachment(UIControl& child) noexcept {
            child.m_state.attachment = Attachment_Fixed;
        }
    public: // do not use those function in your code
        // set flex
        void SetFlex_test(float f) noexcept { m_oStyle.flex = f; }
        // set fixed width
        void SetFixedWidth_test(float f) noexcept {
            m_oBox.size.width = f;
            m_oStyle.maxsize.width = f;
            m_oStyle.minsize.width = f;
        }
        // set fixed height
        void SetFixedHeight_test(float f) noexcept {
            m_oBox.size.height = f;
            m_oStyle.maxsize.height = f;
            m_oStyle.minsize.height = f;
        }
        // set border
        void SetBorder_test(const RectF& b) noexcept {
            m_oBox.border = b;
            m_state.world_changed = true;
            this->NeedUpdate();
            this->NeedRelayout();
        }
    };
    // spacer
    class UISpacer : public UIControl {
        // super class
        using Super = UIControl;
    public:
        // dtor
        ~UISpacer() noexcept;
        // ctor
        UISpacer(UIControl* parent = nullptr) noexcept;
        // render
        void Render() const noexcept override;
    };
    // scroll bar
    class UIScrollBar;
    // scroll area
    class UIScrollArea : public UIControl {
        // super class
        using Super = UIControl;
    public:
        // min sb display size
        enum : int32_t { MIN_SCROLLBAR_DISPLAY_SIZE = 64 };
        // dtor
        ~UIScrollArea() noexcept;
        // ctor
        UIScrollArea(UIControl* parent = nullptr) noexcept;
        // do mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
        // update
        void Update() noexcept override;
    private:
        // [NEW] relayout
        virtual void relayout() noexcept;
    public:
        // get layout direcition
        auto GetLayoutDirection() const noexcept ->AttributeDir { return AttributeDir(m_state.layout_dir); }
        // get vertical ScrollBar
        auto GetVerticalScrollBar() noexcept -> UIScrollBar* { return m_pVerticalSB; }
        // get horizontal ScrollBar
        auto GetHorizontalScrollBar() noexcept -> UIScrollBar* { return m_pHorizontalSB; }
        // get vertical ScrollBar | const overload
        auto GetVerticalScrollBar() const noexcept -> const UIScrollBar* { return m_pVerticalSB; }
        // get horizontal ScrollBar | const overload
        auto GetHorizontalScrollBar() const noexcept -> const UIScrollBar* { return m_pHorizontalSB; }
    protected:
        // synchronize the scroll bar
        void sync_scroll_bar() noexcept;
        // layout the scroll bar
        auto layout_scroll_bar() noexcept->Size2F;
        // get layout position
        auto get_layout_position() const noexcept->Point2F;
    protected:
        // min content size
        Size2F              m_minContentSize;
        // single step
        Size2F              m_szSingleStep;
        // horizontal scroll bar
        UIScrollBar*        m_pHorizontalSB = nullptr;
        // vertical scroll bar
        UIScrollBar*        m_pVerticalSB = nullptr;
    };
    // box layout
    class UIBoxLayout : public UIScrollArea {
        // super class
        using Super = UIScrollArea;
    public:
        // dtor
        ~UIBoxLayout() noexcept;
        // ctor
        UIBoxLayout(UIControl* parent = nullptr) noexcept;
        // do event
        auto DoEvent(UIControl* sender, const EventArg& e) noexcept->EventAccept override;
    private:
        // relayout
        void relayout() noexcept override;
    public:
        // set orient
        void SetOrient(AttributeOrient o) noexcept;
        // get orient
        auto GetOrient() const noexcept ->AttributeOrient {
            return static_cast<AttributeOrient>(m_state.layout_orient);
        }
    };
    // v-box layout
    class UIVBoxLayout final : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
    public:
        // ctor
        UIVBoxLayout(UIControl* parent = nullptr) noexcept;
        // dtor
        ~UIVBoxLayout() noexcept;
    public:
    };
    // h-box layout
    class UIHBoxLayout final : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
    public:
        // ctor
        UIHBoxLayout(UIControl* parent = nullptr) noexcept;
        // dtor
        ~UIHBoxLayout() noexcept;
    public:
    };
    // button control
    class UIButton final : public UIControl {
        // super class
        using Super = UIControl;
    public:
        // ctor
        UIButton(UIControl* parent = nullptr) noexcept;
        // dtor
        ~UIButton() noexcept;
    };
    // scroll bar
    class UIScrollBar final : public UIControl {
        // super class
        using Super = UIControl;
    public:
        // ctor
        UIScrollBar(AttributeOrient o, UIControl* parent = nullptr) noexcept;
        // dtor
        ~UIScrollBar() noexcept;
        // render this control only, [Global rendering and Incremental rendering]
        void Render() const noexcept override;
    public:
        // get value
        auto GetValue() const noexcept -> float { return m_fValue; }
        // get orient
        auto GetOrient() const noexcept -> AttributeOrient { return m_orient; }
    public:
        // set value
        void SetValue(float v) noexcept;
        // set max
        void SetMax(float v) noexcept;
        // set min
        void SetMin(float v) noexcept;
        // set page step
        void SetPageStep(float v) noexcept;
        // set single step
        void SetSingleStep(float v) noexcept;
    protected:
        // orient
        AttributeOrient m_orient;
        // min
        float           m_fMin = 0.f;
        // max
        float           m_fMax = 100.f;
        // value
        float           m_fValue = 0.f;
        // page step
        float           m_fPageStep = 10.f;
        // single step
        float           m_fSingleStep = 1.f;
    };
    /// <summary>
    /// scene to display item(link rubygm-core?)
    /// </summary>
    /// <seealso cref="UIControl" />
    class UIGraphicsScene : public UIControl {
        // super class 
        using Super = UIControl;
    public:
        // ctor
        UIGraphicsScene(UIControl* parent = nullptr) noexcept;
        // dtor
        ~UIGraphicsScene() noexcept;
    protected:
    };
}


namespace UI {
    /// <summary>
    /// notice event type
    /// </summary>
    enum class NoticeEvent : uint32_t {
        // after tree complete
        Event_TreeComplete = 0,
        // refresh min size
        Event_RefreshMinSize,
        // do default action
        Event_DoDefaultAction,
        // for window/viewport only
        Event_UIEvent,
    };
    /// <summary>
    /// EventAccept type
    /// </summary>
    enum EventAccept : bool {
        Event_Accept = true,
        Event_Ignore = false,
    };
}

namespace UI {
    // control
    class UIControl;
    /// <summary>
    /// event argument
    /// </summary>
    struct EventArg {
        // event type
        NoticeEvent     nevent;
        // derived used data
        uint32_t        derived;
    };
    // cast event
    template<NoticeEvent E> auto CastTo(const EventArg& e) -> const EventArg& { 
        return e; 
    }
}

namespace UI {
    /// <summary>
    /// event for longui control event
    /// </summary>
    enum class MouseEvent : uint32_t {
        // mouse wheel in v, if no child handle this, will send it to marginal control
        Event_MouseWheelV,
        // mouse wheel in h, if no child handle this, will send it to marginal control
        Event_MouseWheelH,
        // mouse enter
        Event_MouseEnter,
        // mouse leave
        Event_MouseLeave,
        // mouse hover
        Event_MouseHover,
        // mouse move
        Event_MouseMove,
        // left-button down
        Event_LButtonDown,
        // left-button up
        Event_LButtonUp,
        // right-button down
        Event_RButtonDown,
        // right-button up
        Event_RButtonUp,
        // middle-button down
        Event_MButtonDown,
        // middle-button up
        Event_MButtonUp,
    };
    /// <summary>
    /// Argument for mouse event
    /// </summary>
    struct MouseEventArg {
        // mouse event type
        MouseEvent      type;
        // wheel
        float           wheel;
        // mouse point x
        float           px;
        // mouse point y
        float           py;
    };
}

/// <summary>
/// ui
/// </summary>
namespace UI {
    /// <summary>
    /// 
    /// </summary>
    enum DragEvent {
        // drag enter on this control
        Event_DragEnter,
        // drag over on this control
        Event_DragOver,
        // drag leave on this control
        Event_DragLeave,
        // drop data
        Event_Drop,
    };
}

namespace UI { namespace Component {
    // EMPTY on purpose. ABSTRACT with no virtuals (typedef void Unit?)
    struct Unit { };
    // nine patch
    /*class NightPatch : public Unit {
    public:
        // ctor
        NightPatch(const BmpRect&) noexcept;
        // dtor
        ~NightPatch() noexcept;
    public:
        // check render needed
        bool CheckRenderNeeded(RectF& rect) const noexcept;
        // render this
        void Render(I::Renderer& renderer) const noexcept;
        // recreate with system bitmap
        auto RecreateWithSystemBitmap() noexcept->Result { return recreate(nullptr); }
        // recreate with custom bitmap
        auto RecreateWithCustomBitmap(I::Bitmap& bitmap) noexcept->Result { return recreate(&bitmap); }
    private:
        //  recreate 
        auto recreate(I::Bitmap* ) noexcept->Result;
        // bimap source rect
        BmpRect             m_bmpRect;
    };
    // static text
    class StaticText : public Unit {
    public:
        // ctor
        StaticText() noexcept;
        // ctor with string
        StaticText(const wchar_t* str) noexcept : StaticText(str, std::wcslen(str)) {}
        // ctor with length
        StaticText(const wchar_t* str, std::size_t len) noexcept;
        // dtor
        ~StaticText() noexcept;
        // get text
        auto GetText() const noexcept -> const CUIString& { return m_strText; }
        // set new text
        void SetText(const wchar_t* str, std::size_t len) noexcept;
        // set new text
        void SetText(const wchar_t* str) noexcept { SetText(str, std::wcslen(str)); }
    public:
        // check render needed
        bool CheckRenderNeeded(RectF& rect) const noexcept;
        // render this
        void Render(I::Renderer& renderer) const noexcept;
        // recreate this
        auto Recreate() noexcept->Result;
    private:
        // text
        CUIString           m_strText;
    };
#if 0
    // dynamic
    class DynamicText : public Unit {
    public:
        // ctor
        DynamicText() noexcept;
        // ctor with string
        DynamicText(const wchar_t* str) : DynamicText(str, std::wcslen(str)) noexcept {}
        // ctor with length
        DynamicText(const wchar_t* str, std::size_t len) noexcept;
        // dtor
        ~DynamicText() noexcept;
        // set new text
        void SetText(const wchar_t* str, std::size_t len) noexcept;
        // set new text
        void SetText(const wchar_t* str) noexcept { SetText(str, std::wcslen(str)); }
    public:
        // check render needed
        bool CheckRenderNeeded(RectF& rect) const noexcept;
        // render this
        void Render(I::Renderer& renderer) const noexcept;
        // recreate this
        auto Recreate() noexcept->Result;
    private:
        // text
        CUIString       m_strText;
    };
#endif*/
}}

namespace UI {
    /// <summary>
    /// unit for renderer
    /// </summary>
    struct RenderUnit {
        // control pointer
        UIControl*      ctrl;
        // retained data
        std::uintptr_t  retained;
        // render rect
        RectF           rect;
#ifndef NDEBUG
        // debug clean
        void Clean_dbg() { std::memset(this, 0xCC, sizeof(*this)); }
#endif
    };
}

namespace UI {
    // private data for window
    class CUIWindowPrivate;
    /// <summary>
    /// window base class
    /// </summary>
    class CUIWindow : public CUIObject, protected Node {
        // friend class
        friend class UIViewport;
    public:
        // get window handle
        HWND GetHwnd() const { return m_hwnd; }
        // is top level window
        bool IsTopLevel() const noexcept { return !m_pParent; }
        // is inline window
        bool IsInlineWindow() const noexcept { return !IsTopLevel(); }
    protected:
        // ctor
        CUIWindow(CUIWindow* parent = nullptr) noexcept;
        // dtor
        ~CUIWindow() noexcept;
    public:
        // set pos of window
        void SetPos(Point2L pos) noexcept;
        // get pos of window
        auto GetPos() const noexcept->Point2L;
    protected:
        // parent window
        CUIWindow*          m_pParent = nullptr;
        // window handle
        HWND                m_hwnd = nullptr;
    private:
        // private data
        CUIWindowPrivate*   m_private = nullptr;
    };
}

namespace UI {
    // viewport, logic window viewport
    class UIViewport : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
    public:
        // ctor
        UIViewport(CUIWindow* parent=nullptr) noexcept;
        // dtor
        ~UIViewport() noexcept;
    protected:
        // window
        CUIWindow           m_window;
    };
}



namespace UI {
    /// <summary>
    /// type for Drag&Drop event
    /// </summary>
    enum DragDropType : uint32_t {
        Type_DragEnter = 0,
        Type_DragMove,
        Type_DragLeave,
        Type_Drop,
    };
}

namespace UI {
    // "const char" pointed pointer
    using const_charp = const char*;
#ifndef NDEBUG
    /// <summary>
    /// type of DebugInfo
    /// </summary>
    enum DebugInfoType : uint32_t {
        // dump runtime-info to string
        Type_RuntimeInfo = 0,
        // can be cast to?
        Type_CanbeCasted,
    };
    /// <summary>
    /// debug infomation
    /// </summary>
    struct DebugInfo {
        // debug info type
        DebugInfoType   type;
    };
    /// <summary>
    /// DebugInfo runtime info
    /// </summary>
    struct DebugInfoRI : DebugInfo {
        // ctor
        DebugInfoRI() noexcept { type = Type_RuntimeInfo; }
        // string container
        mutable CUIString   info;
    };
    /// <summary>
    /// DebugInfo cast check
    /// </summary>
    struct DebugInfoCC : DebugInfo {
        // ctor
        DebugInfoCC() noexcept { type = Type_CanbeCasted; }
        // debug info type
        GUID            guid;
    };
#endif
}

#if 0
namespace UI {
    // the type of aniamtion
    enum class AnimationType : uint32_t {
        Type_LinearInterpolation = 0,   // 线性插值
        Type_QuadraticEaseIn,           // 平方渐入插值
        Type_QuadraticEaseOut,          // 平方渐出插值
        Type_QuadraticEaseInOut,        // 平方渐入渐出插值
        Type_CubicEaseIn,               // 立方渐入插值
        Type_CubicEaseOut,              // 立方渐出插值
        Type_CubicEaseInOut,            // 立方渐入渐出插值
        Type_QuarticEaseIn,             // 四次渐入插值
        Type_QuarticEaseOut,            // 四次渐出插值
        Type_QuarticEaseInOut,          // 四次渐入渐出插值
        Type_QuinticEaseIn,             // 五次渐入插值
        Type_QuinticEaseOut,            // 五次渐出插值
        Type_QuinticEaseInOut,          // 五次渐入渐出插值
        Type_SineEaseIn,                // 正弦渐入插值
        Type_SineEaseOut,               // 正弦渐出插值
        Type_SineEaseInOut,             // 正弦渐入渐出插值
        Type_CircularEaseIn,            // 四象圆弧插值
        Type_CircularEaseOut,           // 二象圆弧插值
        Type_CircularEaseInOut,         // 圆弧渐入渐出插值
        Type_ExponentialEaseIn,         // 指数渐入插值
        Type_ExponentialEaseOut,        // 指数渐出插值
        Type_ExponentialEaseInOut,      // 指数渐入渐出插值
        Type_ElasticEaseIn,             // 弹性渐入插值
        Type_ElasticEaseOut,            // 弹性渐出插值
        Type_ElasticEaseInOut,          // 弹性渐入渐出插值
        Type_BackEaseIn,                // 回退渐入插值
        Type_BackEaseOut,               // 回退渐出插值
        Type_BackEaseInOut,             // 回退渐出渐出插值
        Type_BounceEaseIn,              // 反弹渐入插值
        Type_BounceEaseOut,             // 反弹渐出插值
        Type_BounceEaseInOut,           // 反弹渐入渐出插值
    };
    // UI Base Animation 
    class CUIBaseAnimation {
        // max value count(4, for rect)
        enum { MAX_VALUE_COUNT = 4 };
    public:
        // constructor
        CUIBaseAnimation(AnimationType t, uint32_t count=1) noexcept;
        // destructor
        ~CUIBaseAnimation() noexcept;
        // update with system delta time
        void Update() noexcept { Update(L::GetDeltaTime()); }
        // set output
        void SetOutput(float output[]) noexcept { m_pOutput = output; }
        // set start
        void SetStart(float start) noexcept { assert(m_uValueCount == 1); *m_afStarts = start; }
        // set end
        void SetEnd(float end) noexcept { assert(m_uValueCount == 1); *m_afEnds = end; }
        // set starts
        void SetStarts(float starts[]) noexcept;
        // set ends
        void SetEnds(float ends[]) noexcept;
        // update with delta time, return true if 
        void Update(float t) noexcept;
    public:
        // time index
        float               time = 0.0f;
        // duration time
        float               duration = 0.233f;
        // the type
        AnimationType       type;
    private:
        // value count
        uint32_t            m_uValueCount = 0;
        // start value array
        float               m_afStarts[MAX_VALUE_COUNT];
        // end
        float               m_afEnds[MAX_VALUE_COUNT];
        // value to write
        float*              m_pOutput = nullptr;
    };
}

#endif

namespace UI {
    // string view
    class CUIStringView final {
    public:
        // ctor
    public:
        // begin
        auto begin() const noexcept -> const wchar_t* { return m_pBegin; }
        // end
        auto end() const noexcept -> const wchar_t* { return m_pEnd; }
    protected:
        // begin pointer
        const wchar_t*      m_pBegin;
        // end pointer
        const wchar_t*      m_pEnd;
    };
}


namespace UI {
    
    // stylesheets: image
    struct SSImage {

    };

    // stylesheets: background
    /*struct SSBackground {
        // init
        void Init() noexcept;
        // background-color
        ColorF              color;
        // background-image
        SSImage*            image;
        // background-attachment
        AttributeAttachment attachment;
        // background-repeat
        AttributeRepeat     repeat;
        // background-clip
        AttributeBox        clip;
        // background-origin 
        AttributeBox        origin;
    };
    // stylesheets: basic border
    struct SSBasicBorder {
        // init
        void Init() noexcept;
        // border-width @ Box Model
        //RectF               width;
        // border style
        struct{
            // left border style
            AttributeBStyle left;
            // top border style
            AttributeBStyle top;
            // right border style
            AttributeBStyle right;
            // bottom border style
            AttributeBStyle bottom;
            // border style
        }                   style;
        // RGBA color(to save memory)
        struct {
            // left border style
            RGBA            left;
            // top border style
            RGBA            top;
            // right border style
            RGBA            right;
            // bottom border style
            RGBA            bottom;
            // border style
        }                   color;
    };*/
    /// <summary>
    /// value of style sheet
    /// </summary>
    struct SSValue {
        // type of value
        ValueType       type;
        // unit of value
        //ValueUnit       unit;
        // union
        union {
            // u32 data
            uint32_t    u32;
            // i32 data
            int32_t     i32;
            // single float data
            float       single;
            // byte data
            uint8_t     byte;
            // boolean data
            bool        boolean;
        };
    };
    /// <summary>
    /// rule in style sheet
    /// </summary>
    /*struct SSRule {
        // next rule
        SSRule*         next;
        // pseudo
        PseudoType      pseudo;
        // not?
        bool            isnot;
        // size of value
        uint16_t        length;
        // value list
        SSValue         values[0];
    };*/
}
