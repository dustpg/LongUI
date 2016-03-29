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

#ifdef _WIN32_WINNT
#if NTDDI_VERSION < NTDDI_WIN10_TH2
#error "Win10 TH2 required in LongUI"
#endif
#else
#define _WIN32_WINNT 0x0A000001
#endif

#include <cstdint>

#include <../3rdParty/pugixml/pugixml.hpp>
#include <guiddef.h>

// retain debug infomation within UIControl::debug_do_event
#ifdef _DEBUG
#define LongUIDebugEvent
#endif

#define LONGUI_DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE, INTTYPE) \
    inline auto operator | (ENUMTYPE a, ENUMTYPE b) noexcept { return static_cast<ENUMTYPE>(static_cast<INTTYPE>(a) | static_cast<INTTYPE>(b)); };\
    inline auto&operator |=(ENUMTYPE&a, ENUMTYPE b) noexcept { return a = a | b; };\
    inline auto operator & (ENUMTYPE a, ENUMTYPE b) noexcept { return static_cast<ENUMTYPE>(static_cast<INTTYPE>(a) & static_cast<INTTYPE>(b)); };\
    inline auto&operator &=(ENUMTYPE&a, ENUMTYPE b) noexcept { return a = a & b; };\
    inline auto operator ^ (ENUMTYPE a, ENUMTYPE b) noexcept { return static_cast<ENUMTYPE>(static_cast<INTTYPE>(a) ^ static_cast<INTTYPE>(b)); };\
    inline auto&operator ^=(ENUMTYPE&a, ENUMTYPE b) noexcept { return a = a ^ b; };\
    inline auto operator ~ (ENUMTYPE a) noexcept { return static_cast<ENUMTYPE>(~static_cast<INTTYPE>(a)); };

#ifdef _DEBUG
// show hr error in debug
#define longui_debug_hr(hr, msg) if (FAILED(hr)) UIManager << DL_Error << msg << LongUI::endl
// show hr error
#define ShowErrorWithHR(hr) { \
    wchar_t buffer_tmp[LongUI::LongUIStringBufferLength];\
    std::swprintf(\
        buffer_tmp, LongUI::LongUIStringBufferLength,\
        L"<%ls>HR:0x%08X",\
        __FUNCTIONW__, hr\
        );\
    UIManager.ShowError(hr, buffer_tmp);\
    assert(!"ShowErrorWithHR");\
}

// show string error
#define ShowErrorWithStr(str) { \
    wchar_t buffer_tmp[LongUI::LongUIStringBufferLength];\
    std::swprintf(\
        buffer_tmp, LongUI::LongUIStringBufferLength,\
        L"<%ls>: %ls",\
        __FUNCTIONW__, str\
        );\
    UIManager.ShowError(str, buffer_tmp);\
}

// debug level
#define DL_None     LongUI::DebugStringLevel::DLevel_None    << LongUI::Interfmt(L"<%5zuf@%4dl@%ls>", UIManager.frame_id, int(__LINE__), __FUNCTIONW__)
#define DL_Log      LongUI::DebugStringLevel::DLevel_Log     << LongUI::Interfmt(L"<%5zuf@%4dl@%ls>", UIManager.frame_id, int(__LINE__), __FUNCTIONW__)
#define DL_Hint     LongUI::DebugStringLevel::DLevel_Hint    << LongUI::Interfmt(L"<%5zuf@%4dl@%ls>", UIManager.frame_id, int(__LINE__), __FUNCTIONW__)
#define DL_Warning  LongUI::DebugStringLevel::DLevel_Warning << LongUI::Interfmt(L"<%5zuf@%4dl@%ls>", UIManager.frame_id, int(__LINE__), __FUNCTIONW__)
#define DL_Error    LongUI::DebugStringLevel::DLevel_Error   << LongUI::Interfmt(L"<%5zuf@%4dl@%ls>", UIManager.frame_id, int(__LINE__), __FUNCTIONW__)
#define DL_Fatal    LongUI::DebugStringLevel::DLevel_Fatal   << LongUI::Interfmt(L"<%5zuf@%4dl@%ls>", UIManager.frame_id, int(__LINE__), __FUNCTIONW__)
#else
// show hr error in debug
#define longui_debug_hr(hr, msg) (void)0;
// show hr error
#define ShowErrorWithHR(hr) UIManager.ShowError(hr)
// show string error
#define ShowErrorWithStr(str) UIManager.ShowError(str)

// debug level
#define DL_None LongUI::DebugStringLevel::DLevel_None
#define DL_Log LongUI::DebugStringLevel::DLevel_Log
#define DL_Hint LongUI::DebugStringLevel::DLevel_Hint
#define DL_Warning LongUI::DebugStringLevel::DLevel_Warning
#define DL_Error LongUI::DebugStringLevel::DLevel_Error
#define DL_Fatal LongUI::DebugStringLevel::DLevel_Fatal
#endif

#define LONGUI_NAMESPACE_BEGIN namespace LongUI {
#define LONGUI_NAMESPACE_END }

// COM interface
struct IDataObject;

// longui namespace
namespace LongUI {
    // pre-declare
    class UIControl;
    // pre-declare
    class UIContainer;
    // pre-declare
    class CUIManager;
    // pre-declare
    class UIViewport;
    // pre-declare,
    class CUIString;
    // base window
    class XUIBaseWindow;
    // using template specialization
    template<typename T> inline const IID& GetIID();
    // get IID from pointer
    template<typename T> inline const IID& GetIID(T*) { return LongUI::GetIID<T>(); }
    // force modify some (class member) variables
    template<typename T> inline T& force_cast(const T& a) { return const_cast<T&>(a); }
    // call release in safe way
    template<class T> inline auto SafeRelease(T *&pInterfaceToRelease) {
        if (pInterfaceToRelease) {
            pInterfaceToRelease->Release();
            pInterfaceToRelease = nullptr;
        }
    }
    // call addref in safe way
    template<class T> inline auto SafeAcquire(T* pInterfaceToRelease) {
        if (pInterfaceToRelease) {
            pInterfaceToRelease->AddRef();
        }
        return pInterfaceToRelease;
    }
#ifdef LongUIDebugEvent
    // longui cast
    template<class T1, class T2> 
    inline auto longui_cast(T2 ptr) noexcept ->T1 {
        if (!ptr) return nullptr;
        LongUI::UIControl* ctrl = static_cast< LongUI::UIControl*>(ptr);
        ctrl->AssertTypeCastingT(T1(nullptr));
        return static_cast<T1>(ctrl);
    }
    // spacial
    // template<template<>, class T2> auto longui_cast<LongUI::UIControl*>(T2 ptr) noexcept ->LongUI::UIControl* { return static_cast<LongUI::UIControl*>(ptr); };
#else
    template<class T1, class T2> 
    inline auto longui_cast(T2 ptr) noexcept { return static_cast<T1>(ptr); }
#endif
    // LTWH模型矩形
    template<typename T> struct RectLTWH { RectLTWH() {} T left = T(0), top = T(0), width = T(0), height = T(0); };
    using RectLTWH_F = RectLTWH<float>;
    // LTRB模型矩形
    template<typename T> struct RectLTRB { RectLTRB() {} T left = T(0), top = T(0), right = T(0), bottom = T(0); };
    using RectLTRB_F = RectLTRB<float>;
    // signature
    template<typename signature> class CUIFunction;
    // longui callback
    using UICallBack = CUIFunction<bool(UIControl*)>;
    // event type
    enum CreateEventType : size_t { 
        // create this control, type if parent pointer
        Type_CreateControl_NullParentPointer,
        // init, you can create some class-shared resource
        Type_Initialize, 
        // recreate, you should create some device-resource
        Type_Recreate, 
        // un-init, release shared resource
        Type_Uninitialize, 
        // create this control, type if parent pointer
        TypeGreater_CreateControl_ReinterpretParentPointer,
    };
    // create control with "CET"
#define LongUI__CreateWidthCET(CLASS, CONTROL, CET, NODE) \
if (!NODE) UIManager << DL_Hint << L"node null" << LongUI::endl;\
if ((CONTROL = new(std::nothrow) CLASS(reinterpret_cast<LongUI::UIContainer*>(CET)))) CONTROL->CLASS::initialize(NODE);\
else UIManager << DL_Error << L"alloc null" << LongUI::endl;
    // use this
#define case_LongUI__Type_CreateControl default
    // CreateControlEvent 控件创建事件
    using CreateControlEvent = auto (*)(CreateEventType, pugi::xml_node) ->UIControl*;
    /// <summary>
    /// LongUI Flags for Core Control: UIControl, UIMarginalable, UIContainer
    /// </summary>
    enum LongUIFlag : uint32_t {
        /// <summary>
        /// none flag
        /// </summary>
        Flag_None = 0,
        /// <summary>
        /// This control is a UIContainer [default: false]
        /// </summary>
        /// <remarks>
        /// [Auto] if the control is a container, will mark it
        /// in <see cref="LongUI::UIContainer::UIContainer"/>
        /// </remarks>
        Flag_UIContainer = 1 << 0,
        /// <summary>
        /// This control is a marginal-control of one container [default: false]
        /// </summary>
        /// <remarks>
        /// [xml-attribute "marginal"] <see cref="LongUI::UIMarginalable::UIMarginalable"/>
        /// </remarks>
        Flag_MarginalControl = 1 << 1,
        /// <summary>
        /// focusable could be set keyboard focus
        /// </summary>
        /// <remarks>
        /// set it by your code
        /// </remarks>
        Flag_Focusable = 1 << 2,
        /// <summary>
        /// the width of control is fixed [default: false]
        /// </summary>
        /// <remarks>
        /// [Auto] width fixed if given a valid width value in xml 
        /// attribute ("size") [0], e.g. size="96, 0"
        /// </remarks>
        Flag_WidthFixed = 1 << 3,
        /// <summary>
        /// the height of control is fixed [default: false]
        /// </summary>
        /// <remarks>
        /// [Auto] height fixed if given a valid height value in xml 
        /// attribute ("size") [1], e.g. size="0, 32"
        /// </remarks>
        Flag_HeightFixed = 1 << 4,
        /// 
        /// 
        /// 
        /// 
        /// 
        /// 
        // [default: true] if true, this caontrol cann't draw out of
        // it's cliprect, if false, it coule draw on sibling/parent.
        // NOTE: this attribute used by parent
        // NOTE: container should be true in this case but not mandatory
        // XML Attribute : "strictclip"@bool
        Flag_ClipStrictly = 1 << 6,
        // [default: false][auto, no specified]
        // control need pre-render for content
        // call UIViewport::RegisterOffScreenRender2D or
        // call UIViewport::RegisterOffScreenRender3D to set
        Flag_NeedRegisterOffScreenRender = 1 << 7,
        // [default: false][auto, no specified]
        // control need Direct3D api to render,
        // call UIViewport::RegisterOffScreenRender3D to set
        // if use Direct2D , call UIViewport::RegisterOffScreenRender2D
        Flag_OffScreen3DContent = 1 << 8,
#if 0
        // [default: false][auto, and xml attribute "renderparent"@bool]
        // if this control will be rendering when do dirty-rendering,
        // must be rendering whole parent.
        // could be setted xml-attribute("renderparent") and auto setted
        // by parent's flag : Flag_Container_HostChildrenRenderingDirectly,
        //Flag_RenderParent = 1 << 9,
        // [default: false][xml attribute : "hostchild"@bool] 
        // if true, container will host child rendering in anytime
        // if the container was setted this flag, it would set 
        // all children flag "Flag_RenderParent" to true
        //Flag_Container_HostChildrenRenderingDirectly = 1 << 16,
#endif
        // [default: false][xml attribute : "hostposterity"@bool] 
        // if true, the container will host posterity rendering in anytime
        // to set posterity's UIControl::prerender to the container's
        // to see "LongUI::UIContainer::after_insert"
        Flag_Container_HostPosterityRenderingDirectly = 1 << 17,
        /// <summary>
        /// container will zoom marginal controls [default: true]
        /// </summary>
        /// <remarks>
        /// changed by xml-attribute("zoommarginal") defaultly
        /// </remarks>
        Flag_Container_ZoomMarginalControl = 1 << 18,
    };
    // operator float LongUIFlag
    LONGUI_DEFINE_ENUM_FLAG_OPERATORS(LongUIFlag, uint32_t);
    // Control State
    enum ControlState : uint16_t {
        State_Disabled = 0,    // 禁用状态
        State_Normal,          // 通常状态
        State_Hover,           // 鼠标移上
        State_Pushed,          // 鼠标按下
        STATE_COUNT,           // 状态数量
    };
    // CheckBoxState
    enum class CheckBoxState : uint16_t {
        State_Checked = 0,      // 选中
        State_Indeterminate,    // 不确定
        State_Unchecked,        // 未选中
        STATE_COUNT,            // count
    };
    // bitmap render rule
    enum class BitmapRenderRule : uint16_t {
        Rule_Scale = 0,         // 直接缩放
        Rule_ButtonLike,        // 三段渲染
    };
    // brush type
    enum class BrushType : size_t {
        Type_SolidColor = 0,    // 纯色笔刷
        Type_LinearGradient,    // 线性渐变
        Type_RadialGradient,    // 径向渐变
        Type_Bitmap,            // 位图笔刷
    };
    /// <summary>
    /// event for longui control event
    /// </summary>
    enum class Event : uint32_t {
        /// <summary>
        /// The event to signal that control-tree buliding finished
        /// </summary>
        /// <remarks> no ex-information</remarks>
        Event_TreeBulidingFinished = 0,
        /// <summary>
        /// set a new parent, you should deal something
        /// </summary>
        Event_SetNewParent,
        /// <summary>
        /// [nc] notify all children, but sender/marginal control
        /// </summary>
        //Event_NotifyChildren,
        // [ui-exdata]sub event, use for event callback
        Event_SubEvent,
        // [none-exdata]command, keyboard direct-access(like type 'Eenter')
        Event_Command,
        // [none-exdata]set keyboard focus
        Event_SetFocus,
        // [none-exdata]kill keyboard focus
        Event_KillFocus,
        // [none-exdata]timer when SetTime
        Event_Timer,
        // [stt] set control text
        Event_SetText,
        // [none-exdata]get control text
        Event_GetText,
        // [ste] set control basic state
        Event_SetEnabled,
        // [stf] set control float value
        Event_SetFloat,
        // [none-exdata] get control float value
        Event_GetFloat,
        // [key] char input
        Event_Char,
        // [key] key down
        Event_KeyDown,
        //Event_NotifyChildren,
        // ----- User Custom Defined Event -----
        Event_UserDefined = 0x100,
    };
    // LongUI Sub Event
    enum class SubEvent : size_t {
        // control item clicked(like button)
        Event_ItemClicked,
        // control item double-clicked
        Event_ItemDbClicked,
        // single-line-edit returned
        Event_EditReturned,
        // value changed(like slider, scrollbar, checkbox and ect.)
        Event_ValueChanged,
        // Context Menu
        //Event_ContextMenu,
        // ----- User Custom Defined Event -----
        Event_Custom,
    };
    // LongUI Event Argument
    struct EventArgument {
        // sender, null for system
        UIControl*      sender;
        // data
        union {
            // control
            struct { LongUI::SubEvent subevent; void* pointer; } ui;
            // set text
            struct { const wchar_t* text; void* unused; } stt;
            // set enabled
            struct { bool enabled; } ste;
            // set float
            struct { float value;  } stf;
            // input char
            struct { char32_t ch; } key;
        };
        // Return Code
        union {
            // [out] Control for Parent
            mutable  UIControl*    ctrl;
            // [out] string for result
            mutable const wchar_t* str;
            // [out] float for result
            mutable float          fvalue;
        };
        // event id
        LongUI::Event event;
    };
    /// <summary>
    /// event for longui control event
    /// </summary>
    enum class MouseEvent : uint32_t {
        // none, never got
        Event_None,
        // mouse wheel in v, if no child handle this, will send it to marginal control
        Event_MouseWheelV,
        // mouse wheel in h, if no child handle this, will send it to marginal control
        Event_MouseWheelH,
        // drag enter on this control
        Event_DragEnter,
        // drag over on this control
        Event_DragOver,
        // drag leave on this control
        Event_DragLeave,
        // drop data
        Event_Drop,
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
    // LongUI Mouse Event Argument
    struct MouseEventArgument {
        // data
        union {
            // mouse wheel delta
            struct { float delta; } wheel;
            // clipboard format 
            struct { IDataObject* dataobj; uint32_t* outeffect; } cf;
        };
        // world mouse x-position, you should transfrom it while using
        float               ptx;
        // world mouse y-position, you should transfrom it while using
        float               pty;
        // last control
        mutable UIControl*  last;
        // event id
        LongUI::MouseEvent  event;
    };
    // Script define
    struct ScriptUI {
        // ctor
        ScriptUI() = default;
        // script data, maybe binary data maybe string
        const unsigned char*  script = nullptr;
        // size of it
        size_t                size = 0;
    };
    // debug string level
    enum DebugStringLevel : uint32_t {
        // None level
        DLevel_None = 0,
        // level Log
        DLevel_Log,
        // level Hint
        DLevel_Hint,
        // level Warning
        DLevel_Warning,
        // level Error
        DLevel_Error,
        // level Fatal
        DLevel_Fatal,
        // level's size
        DLEVEL_SIZE
    };
#ifdef LongUIDebugEvent
    // LongUI Debug Information
    enum class DebugInformation : size_t {
        // get class name
        Information_GetClassName = 0,
        // get class name with namespace
        Information_GetFullClassName ,
        // can be cast to(IID)
        Information_CanbeCasted
    };
    // longui debug information
    struct DebugEventInformation {
        // information id
        DebugInformation    infomation;
        // parma
        union {
            // IID
            const IID*      iid;
        };
        // Return Code
        union {
            // [out] Control for Parent
            mutable size_t          id;
            // [out] Control for Parent
            mutable const wchar_t*  str;
        };
    };
    // get full class name
    static auto&DebugGetFullClassName() noexcept {
        static DebugEventInformation s_dbgInfomation;
        s_dbgInfomation.infomation = DebugInformation::Information_GetFullClassName;
        s_dbgInfomation.iid = nullptr;
        s_dbgInfomation.str = nullptr;
        return s_dbgInfomation;
    }
#endif
    // type for rich
    enum class RichType : uint16_t {
        // none, you should set rich-style by youself
        Type_None,
        // core, use longui-core-mark-language mark it
        Type_Core,
        // xml, use xml-style, can use < > or { }
        Type_Xml,
        // user custom defined, for IUIConfigure::CustomRichType
        Type_Custom,
    };
    // LongUI Format Text Config,
    // cursor enum
    enum class Cursor : size_t {
        // default
        Cursor_Default = 0,
        // normal Arrow
        Cursor_Arrow,
        // "I" beam
        Cursor_Ibeam,
        // wait
        Cursor_Wait,
        // hand
        Cursor_Hand,
        // help
        Cursor_Help,
        // cross
        Cursor_Cross,
        // size all
        Cursor_SizeAll,
        // up arrow
        Cursor_UpArrow,
        // size NW-SE
        Cursor_SizeNWSE,
        // size NE-SW 
        Cursor_SizeNESW,
        // size WE
        Cursor_SizeWE,
        // size NS
        Cursor_SizeNS,
        // user defined zone
        Cursor_UserDefined,
    };
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
    // string table
    enum TableString : uint32_t {
        // faild with hresult
        String_FaildHR = 0,
        // cut
        String_Cut,
        // copy
        String_Copy,
        // paste
        String_Paste,
        // select all
        String_SelectAll,
    };
    // priority type of window creating
    enum WindowPriorityType : uint32_t {
        // custom window first
        Type_CustomWindowFirst = 1 << 0,
        // inset window first
        Type_InsetWindowFirst = 1 << 1,
    };
    LONGUI_DEFINE_ENUM_FLAG_OPERATORS(WindowPriorityType, uint32_t);
}

