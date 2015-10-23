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

// Graphics Level
//#define LongUIGraphicsInterfaceLevel LongUI::GraphicsInterfaceLevel::Direct2D1_1


// LONGUI TIPS:
//- be careful to use rotation transform because of the cliping

#include "longuiconf.h"

// longui
/*namespace LongUI {
    // --- LongUI Graphics Interface Level ---
    enum class GraphicsInterfaceLevel : uint32_t {
        // [unavailable but maybe support]
        OpenGL = 0x01,
        // [available]
        Direct2D1_1 = 0x02,
    };
}*/


#define LONGUI_DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE, INTTYPE) \
    static auto operator | (ENUMTYPE a, ENUMTYPE b) noexcept { return static_cast<ENUMTYPE>(static_cast<INTTYPE>(a) | static_cast<INTTYPE>(b)); };\
    static auto&operator |=(ENUMTYPE&a, ENUMTYPE b) noexcept { return a = a | b; };\
    static auto operator & (ENUMTYPE a, ENUMTYPE b) noexcept { return static_cast<ENUMTYPE>(static_cast<INTTYPE>(a) & static_cast<INTTYPE>(b)); };\
    static auto&operator &=(ENUMTYPE&a, ENUMTYPE b) noexcept { return a = a & b; };\
    static auto operator ^ (ENUMTYPE a, ENUMTYPE b) noexcept { return static_cast<ENUMTYPE>(static_cast<INTTYPE>(a) ^ static_cast<INTTYPE>(b)); };\
    static auto&operator ^=(ENUMTYPE&a, ENUMTYPE b) noexcept { return a = a ^ b; };\
    static auto operator ~ (ENUMTYPE a) noexcept { return static_cast<ENUMTYPE>(~static_cast<INTTYPE>(a)); };

#ifndef OUT
#define OUT
#endif

#ifndef IN
#define IN
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

// show error if error with hr code
#define ShowHR(hr) if(FAILED(hr)) ShowErrorWithHR(hr)

#ifdef _DEBUG
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
#define DL_None     LongUI::DebugStringLevel::DLevel_None    << LongUI::Formated(L"<%5zu@%ls>", UIManager.frame_id, __FUNCTIONW__)
#define DL_Log      LongUI::DebugStringLevel::DLevel_Log     << LongUI::Formated(L"<%5zu@%ls>", UIManager.frame_id, __FUNCTIONW__)
#define DL_Hint     LongUI::DebugStringLevel::DLevel_Hint    << LongUI::Formated(L"<%5zu@%ls>", UIManager.frame_id, __FUNCTIONW__)
#define DL_Warning  LongUI::DebugStringLevel::DLevel_Warning << LongUI::Formated(L"<%5zu@%ls>", UIManager.frame_id, __FUNCTIONW__)
#define DL_Error    LongUI::DebugStringLevel::DLevel_Error   << LongUI::Formated(L"<%5zu@%ls>", UIManager.frame_id, __FUNCTIONW__)
#define DL_Fatal    LongUI::DebugStringLevel::DLevel_Fatal   << LongUI::Formated(L"<%5zu@%ls>", UIManager.frame_id, __FUNCTIONW__)
#else
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

// longui namespace
namespace LongUI {
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
    inline auto longui_cast(T2 ptr) noexcept->T1 {
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
    template<typename T> struct RectLTWH { RectLTWH() {} T left=T(0), top = T(0), width = T(0), height = T(0); };
    using RectLTWH_F = RectLTWH<float>;
    using RectLTWH_L = RectLTWH<int32_t>;
    using RectLTWH_U = RectLTWH<uint32_t>;
    // LTRB模型矩形
    template<typename T> struct RectLTRB { RectLTRB() {} T left = T(0), top = T(0), right = T(0), bottom = T(0); };
    using RectLTRB_F = RectLTRB<float>;
    using RectLTRB_L = RectLTRB<int32_t>;
    using RectLTRB_U = RectLTRB<uint32_t>;
    // --------------------------------
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
    // create
    template<typename T, typename XMLNODE> T* CreateWidthCET(CreateEventType ty, XMLNODE node) noexcept {
        return new(std::nothrow) T(reinterpret_cast<UIContainer*>(ty), node);
    }
    // use this
#define case_LongUI__Type_CreateControl default
    // CreateControl Function 控件创建函数
    using CreateControlFunction = auto (WINAPI*)(CreateEventType, pugi::xml_node) ->UIControl*;
    // { B0CC8D79-9761-46F0-8558-F93A073CA0E6 }
    static const GUID IID_InlineParamHandler =
    { 0xb0cc8d79, 0x9761, 0x46f0,{ 0x85, 0x58, 0xf9, 0x3a, 0x7, 0x3c, 0xa0, 0xe6 } };
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
        /// <summary>
        /// control is floating
        /// </summary>
        Flag_Floating = 1 << 5,
        // [default: true] if true, this caontrol cann't draw out of
        // it's cliprect, if false, it coule draw on sibling/parent.
        // NOTE: this attribute used by parent
        // NOTE: container should be true in this case but not mandatory
        // XML Attribute : "strictclip"@bool
        Flag_ClipStrictly = 1 << 6,
        // [default: false][auto, no specified]
        // control need pre-render for content
        // call UIWindow::RegisterOffScreenRender2D or
        // call UIWindow::RegisterOffScreenRender3D to set
        Flag_NeedRegisterOffScreenRender = 1 << 7,
        // [default: false][auto, no specified]
        // control need Direct3D api to render,
        // call UIWindow::RegisterOffScreenRender3D to set
        // if use Direct2D , call UIWindow::RegisterOffScreenRender2D
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
        /// if exist marginal control, will set it
        /// </summary>
        /// <remarks>
        /// [Auto] this is just a optimization flag
        /// </remarks>
        Flag_Container_ExistMarginalControl = 1 << 18,
        /// <summary>
        /// container will zoom marginal controls [default: true]
        /// </summary>
        /// <remarks>
        /// changed by xml-attribute("zoommarginal") defaultly
        /// </remarks>
        Flag_Container_ZoomMarginalControl = 1 << 19,
    };
    // operator float LongUIFlag
    LONGUI_DEFINE_ENUM_FLAG_OPERATORS(LongUIFlag, uint32_t);
    // Control Status
    enum ControlStatus : uint16_t {
        Status_Disabled = 0,    // 禁用状态
        Status_Normal,          // 通常状态
        Status_Hover,           // 鼠标移上
        Status_Pushed,          // 鼠标按下
        STATUS_COUNT,           // 状态数量
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
    enum class Event : UINT {
        /// <summary>
        /// The event to signal that control-tree buliding finished
        /// </summary>
        /// <remarks> no ex-information</remarks>
        Event_TreeBulidingFinished = 0,
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
        // notify all children(but sender)
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
        // Context Menu, just one event-call recommend
        Event_ContextMenu,
        // single-line-edit returned
        Event_EditReturned,
        // value changed(like slider or scrollbar)
        Event_ValueChanged,
        // ----- User Custom Defined Event -----
        Event_Custom,
    };
    // LongUI Event Argument
    struct EventArgument {
        // sender, null for system
        UIControl*      sender;
        // data
        union {
            // System 
            struct { WPARAM wParam; LPARAM lParam; } sys;
            // control
            struct { LongUI::SubEvent subevent; void* pointer; } ui;
            // set text
            struct { const wchar_t* text; void* unused; } stt;
        };
        // Return Code
        union {
            // [out] LRESULT for System
            OUT mutable  LRESULT       lr;
            // [out] Control for Parent
            OUT mutable  UIControl*    ctrl;
            // [out] Control for Parent
            OUT mutable const wchar_t* str;
        };
        // event id
        union { UINT msg; LongUI::Event event; };
    };
    /// <summary>
    /// event for longui control event
    /// </summary>
    enum class MouseEvent : UINT {
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
            // System 
            struct { WPARAM wParam; LPARAM lParam; } sys;
            // clipboard format 
            struct { IDataObject* dataobj; DWORD* outeffect; } cf;
        };
        // world mouse position, you should transfrom it while using
        D2D1_POINT_2F       pt;
        // last control
        mutable UIControl*  last;
        // event id
        LongUI::MouseEvent  event;
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
            OUT mutable size_t          id;
            // [out] Control for Parent
            OUT mutable const wchar_t*  str;
        };
    };
    // get full class name
    static auto&DebugGetFullClassName() noexcept {
        static DebugEventInformation s_dbgInfomation;
        ::memset(&s_dbgInfomation, 0, sizeof(s_dbgInfomation));
        s_dbgInfomation.infomation = DebugInformation::Information_GetFullClassName;
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
    /// <summary>
    /// config for formating text
    /// </summary>
    /// <remarks> if your string more than 1K, do not use this</remarks>
    struct FormatTextConfig {
        // basic text format
        IN  IDWriteTextFormat*      format;
        // text layout width
        IN  float                   width;
        // text layout hright
        IN  float                   height;
        // make the text showing progress, maybe you want
        // a "typing-effect", set 1.0f to show all, 0.0f to hide
        IN  float                   progress;
        // format for this
        IN  RichType                rich_type;
        // the text real(without format) length
        OUT mutable uint16_t        text_length;
    };
}

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

// Basic
#include "Core/UIInterface.h"
#include "Core/UIString.h"
// LongUI Util Code
#include "Util/UIAnimation.h"
#include "Util/UIUtilMethod.h"
#include "Util/UIUtilHelper.h"
#include "Util/UIUtilClass.h"
#include "Util/UIUtilEzContainer.h"
#include "Util/UIUtilDX.h"
#include "Util/UIUtilSvg.h"
#include "Util/UIDropSource.h"
#include "Util/UIDataObject.h"
#include "Util/UITextRenderer.h"
#include "Util/UIInlineObject.h"

// LongUI Component
#ifndef LongUICoreOnly
#   include "Component/Element.h"
#   include "Component/Video.h"
#   include "Component/EditaleText.h"
#   include "Component/Text.h"
#endif

// LongUI Manager
#include "Util/UIInput.h"
#include "Core/UIManager.h"

// LongUI Contorl
#include "Core/UIControl.h"
#include "Core/UIMarginalable.h"
#include "Core/UIContainer.h"

// longui core only?
#ifndef LongUICoreOnly
#   include "Control/Container/UISingle.h"
#   include "Control/Container/UIContainerBuiltIn.h"
#   include "Control/Container/UILinearLayout.h"
#   include "Control/Container/UIList.h"
#   include "Control/Container/UIPage.h"
#   include "Control/Container/UIWindow.h"
#   include "Control/Container/UIMenu.h"
#   include "Control/UIScrollBar.h"
#   include "Control/UISlider.h"
#   include "Control/UIEdit.h"
#   include "Control/UIRichEdit.h"
#   include "Control/UIText.h"
#   include "Control/UICheckBox.h"
#   include "Control/UIButton.h"
#endif
