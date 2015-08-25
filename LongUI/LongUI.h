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
#include "binaryattribute.h"

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


template<class Interface>
inline auto SafeRelease(Interface *&pInterfaceToRelease) {
    if (pInterfaceToRelease) {
        pInterfaceToRelease->Release();
        pInterfaceToRelease = nullptr;
    }
}

template<class Interface>
inline auto SafeAcquire(Interface *pInterfaceToRelease) {
    if (pInterfaceToRelease) {
        pInterfaceToRelease->AddRef();
    }
    return pInterfaceToRelease;
}

#ifndef OUT
#define OUT
#endif

#ifndef IN
#define IN
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

#define AssertHR(hr) if(FAILED(hr)) ShowErrorWithHR(hr)

#ifdef _DEBUG
// show hr error
#define ShowErrorWithHR(hr) { \
    wchar_t buffer_tmp[LongUI::LongUIStringBufferLength];\
    ::swprintf(\
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
    ::swprintf(\
        buffer_tmp, LongUI::LongUIStringBufferLength,\
        L"<%ls>: %ls",\
        __FUNCTIONW__, str\
        );\
    UIManager.ShowError(str, buffer_tmp);\
}

// debug level
#define DL_None LongUI::DebugStringLevel::DLevel_None << L'<' << __FUNCTION__ << L">: "
#define DL_Log LongUI::DebugStringLevel::DLevel_Log << L'<' << __FUNCTION__ << L">: "
#define DL_Hint LongUI::DebugStringLevel::DLevel_Hint << L'<' << __FUNCTION__ << L">: "
#define DL_Warning LongUI::DebugStringLevel::DLevel_Warning << L'<' << __FUNCTION__ << L">: "
#define DL_Error LongUI::DebugStringLevel::DLevel_Error << L'<' << __FUNCTION__ << L">: "
#define DL_Fatal LongUI::DebugStringLevel::DLevel_Fatal << L'<' << __FUNCTION__ << L">: "
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

// LongUI render target 目标渲染呈现器
using LongUIRenderTarget = ID2D1DeviceContext;

// longui
namespace LongUI {
    // graphics interface level
    // static const GraphicsInterfaceLevel $GraphicsInterfaceLevel = GraphicsInterfaceLevel::Direct2D1_1;
    // force modify some (class member) variables
    template<typename T>
    inline T& force_cast(const T& a) { return const_cast<T&>(a); }
#ifdef LongUIDebugEvent
    // longui cast
    template<class T1, class T2> auto longui_cast(T2 ptr) noexcept->T1 {
        if (!ptr) return nullptr;
        LongUI::UIControl* ctrl = static_cast< LongUI::UIControl*>(ptr);
        ctrl->AssertTypeCastingT(T1(nullptr));
        return static_cast<T1>(ctrl);
    }
    // spacial
    // template<template<>, class T2> auto longui_cast<LongUI::UIControl*>(T2 ptr) noexcept ->LongUI::UIControl* { return static_cast<LongUI::UIControl*>(ptr); };
#else
    template<class T1, class T2> auto longui_cast(T2 ptr) noexcept->T1 {
        return static_cast<T1>(ptr);
    }
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
    // longui callback func
    using SubEventCallBack = bool(*)(UIControl* recver, UIControl* sender);
    // event type
    enum CreateEventType : size_t { Type_CreateControl = 0, Type_Initialize, Type_Recreate, Type_Uninitialize, };
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
        /// the width of control is fixed [default: false]
        /// </summary>
        /// <remarks>
        /// [Auto] width fixed if given a valid width value in xml 
        /// attribute ("size") [0], e.g. size="96, 0"
        /// </remarks>
        Flag_WidthFixed = 1 << 1,
        /// <summary>
        /// the height of control is fixed [default: false]
        /// </summary>
        /// <remarks>
        /// [Auto] height fixed if given a valid height value in xml 
        /// attribute ("size") [1], e.g. size="0, 32"
        /// </remarks>
        Flag_HeightFixed = 1 << 2,
        // [default: false]control is floating
        Flag_Floating = 1 << 3,
        // [default: true] if true, this caontrol cann't draw out of
        // it's cliprect, if false, it coule draw on sibling/parent.
        // NOTE: this attribute used by parent
        // NOTE: container should be true in this case but not mandatory
        // XML Attribute : "strictclip"@bool
        Flag_ClipStrictly = 1 << 4,
        // control construct with full xml node info
        // if your control need full xml node, set it to true by const_cast
        Flag_ControlNeedFullXMLNode = 1 << 5,
        // [default: false][auto, no specified]
        // control need pre-render for content
        // call UIWindow::RegisterOffScreenRender2D or
        // call UIWindow::RegisterOffScreenRender3D to set
        Flag_NeedRegisterOffScreenRender = 1 << 6,
        // [default: false][auto, no specified]
        // control need Direct3D api to render,
        // call UIWindow::RegisterOffScreenRender3D to set
        // if use Direct2D , call UIWindow::RegisterOffScreenRender2D
        Flag_OffScreen3DContent = 1 << 7,
        // [default: false][auto, and xml attribute "renderparent"@bool]
        // if this control will be rendering when do dirty-rendering,
        // must be rendering whole parent.
        // could be setted xml-attribute("renderparent") and auto setted
        // by parent's flag : Flag_Container_HostChildrenRenderingDirectly,
        Flag_RenderParent = 1 << 8,
        // [default: false][xml attribute : "hostchild"@bool] 
        // if true, container will host child rendering in anytime
        // if the container was setted this flag, it would set 
        // all children flag "Flag_RenderParent" to true
        Flag_Container_HostChildrenRenderingDirectly = 1 << 16,
        // [default: false][xml attribute : "hostposterity"@bool] 
        // if true, container will host posterity rendering in anytime
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
    // operator | for LongUIFlag
    static auto operator |(LongUIFlag a, LongUIFlag b) noexcept {
        return static_cast<LongUIFlag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    };
    // operator |= for LongUIFlag
    static auto&operator |=(LongUIFlag& a, LongUIFlag b) noexcept {
        return a = a | b;
    };
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
    // LongUI Render Type
    enum class RenderType : uint32_t {
        Type_RenderBackground = 0,
        Type_Render,
        Type_RenderForeground,
        Type_RenderOffScreen,
    };
    /// <summary>
    /// event for longui control event
    /// </summary>
    enum class Event : UINT {
        // control-tree buliding finished
        Event_TreeBulidingFinished = 0,
        // sub event, use for event callback
        Event_SubEvent,
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
        // set focus
        Event_SetFocus,
        // kill focus
        Event_KillFocus,
        // notify all children(but sender)
        //Event_NotifyChildren,
        // ----- User Custom Defined Event -----
        Event_Custom = 0x100,
    };
    // LongUI Sub Event
    enum class SubEvent : size_t {
        // null
        Event_Null = 0,
        // button clicked
        Event_ButtonClicked,
        // single-line-edit returned
        Event_EditReturned,
        // slider value changed
        Event_SliderValueChanged,
        // ----- User Custom Defined Event -----
        Event_Custom = 0x100,
    };
    // priority for rendering
    enum RenderingPriority : uint8_t {
        // last
        Priority_Last = 0,
        // after most
        Priority_AfterMost = 20,
        // low
        Priority_Low = 127,
        // normal
        Priority_Normal = 128,
        // high
        Priority_High = 129,
        // before most
        Priority_BeforeMost = 235,
        // first
        Priority_First = 255,
    };
    // LongUI Event Argument
    struct EventArgument {
        // sender, null for system
        UIControl*      sender;
        // data
        union {
            // System 
            struct { WPARAM wParam; LPARAM lParam; } sys;
            // clipboard format 
            struct { IDataObject* dataobj; DWORD* outeffect; } cf;
            // control
            struct { LongUI::SubEvent subevent; void* pointer; } ui;
        };
        // world mouse position, you should transfrom it while using
        D2D1_POINT_2F   pt;
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

// Basic
#include "UIInterface.h"
#include "UIString.h"
// LongUI Util Code
#include "UIAnimation.h"
#include "UIUtilMethod.h"
#include "UIUtilHelper.h"
#include "UIUtilClass.h"
#include "UIUtilEzContainer.h"
#include "UIUtilDX.h"
#include "UIUtilSvg.h"
#include "UIDropSource.h"
#include "UIDataObject.h"
#include "UITextRenderer.h"
#include "UIInlineObject.h"
// LongUI Component
#include "Component/Element.h"
#include "Component/EditaleText.h"
#include "Component/Text.h"
// LongUI Manager
#include "UIInput.h"
#include "UIManager.h"

// LongUI Contorl
#include "UIControl.h"
// longui core only?
#ifndef LongUICoreOnly
#   include "UIMarginalable.h"
#   include "UIContainer.h"
#   include "UIScrollBar.h"
#   include "UISlider.h"
#   include "UIVerticalLayout.h"
#   include "UIHorizontalLayout.h"
#   include "UIWindow.h"
#   include "UIEdit.h"
#   include "UIRichEdit.h"
#   include "UIText.h"
#   include "UICheckBox.h"
#   include "UIButton.h"
#   include "UIMenu.h"
#endif
