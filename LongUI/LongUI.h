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

#ifndef LONGUI_H
#define LONGUI_H

// Graphics Level
#define LongUIGraphicsInterfaceLevel LongUI::GraphicsInterfaceLevel::Direct2D1_1


// LONGUI TIPS:
//- be careful to use rotation transform because of the cliping

#include "longuiconf.h"


// longui
namespace LongUI {
    // --- LongUI Graphics Interface Level ---
    enum class GraphicsInterfaceLevel : uint32_t {
        // [unavailable but maybe support]
        OpenGL = 0x01,
        // [available]
        Direct2D1_1 = 0x02,
    };
}


template<class Interface>
inline void SafeRelease(Interface *&pInterfaceToRelease) {
    if (pInterfaceToRelease) {
        pInterfaceToRelease->Release();
        pInterfaceToRelease = nullptr;
    }
}

template<class Interface>
inline Interface* SafeAcquire(Interface *pInterfaceToRelease) {
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
    wchar_t buffffffffffer[LongUIStringBufferLength];\
    ::swprintf(\
        buffffffffffer, LongUIStringBufferLength,\
        L"<%ls>HR:0x%08X",\
        __FUNCTIONW__, hr\
        );\
    UIManager.ShowError(hr, buffffffffffer);\
    assert(!"ShowErrorWithHR");\
}

// show string error
#define ShowErrorWithStr(str) { \
    wchar_t buffffffffffer[LongUIStringBufferLength];\
    ::swprintf(\
        buffffffffffer, LongUIStringBufferLength,\
        L"<%ls>: %ls",\
        __FUNCTIONW__, str\
        );\
    UIManager.ShowError(str, buffffffffffer);\
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
    static const GraphicsInterfaceLevel $GraphicsInterfaceLevel = GraphicsInterfaceLevel::Direct2D1_1;
    // force modify some (class member) variables
    template<typename T>
    inline T& force_cast(const T& a) { return const_cast<T&>(a); }
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
    // longui callback func 控件回调
    using LongUIEventCallBack = bool (*)(UIControl* event_setter, UIControl* func_caller) ;
    // event type
    enum CreateEventType : size_t { Type_CreateControl = 0, Type_Initialize, Type_Recreate, Type_Uninitialize, };
    // CreateControl Function 控件创建函数
    using CreateControlFunction = auto (WINAPI*)(CreateEventType, pugi::xml_node) ->UIControl*;
    // { B0CC8D79-9761-46F0-8558-F93A073CA0E6 }
    static const GUID IID_InlineParamHandler =
    { 0xb0cc8d79, 0x9761, 0x46f0,{ 0x85, 0x58, 0xf9, 0x3a, 0x7, 0x3c, 0xa0, 0xe6 } };
    // LongUI Custom Formated String Inline Param Handler
    // type:  0 for drawing effect, 1 for inline object
    // token: the string segment of this part
    using InlineParamHandler = IUnknown* (*)(uint32_t type, const wchar_t* token);
    // LongUI Flags
    enum LongUIFlag : uint32_t {
        // no flag
        Flag_None = 0,
        // [default: false][auto, no specified]
        // if the control is a container, will/please mark it
        Flag_UIContainer = 1 << 0,
        // [default: false][auto]control's width fixed, if given a valid width value
        // XML Attribute: "pos"[2]@float
        Flag_WidthFixed = 1 << 1,
        // [default: false][auto]control's width fixed, if given a valid height value
        // XML Attribute: "pos"[3]@float
        Flag_HeightFixed = 1 << 2,
        // [default: false]control is floating
        // XML Attribute: "float"@bool
        Flag_Floating = 1 << 3,
        // [default: true] if true, this caontrol cann't draw out of
        // it's cliprect, if false, it coule draw on sibling/parent.
        // NOTE: this attribute used by parent
        // NOTE: container should be true in this case but not mandatory
        // XML Attribute : "strictclip"@bool
        Flag_StrictClip = 1 << 4,
        // [default: true]control is visible
        // XML Attribute: "visible"@bool
        Flag_Visible = 1 << 5,
        // [default: false][auto, no specified]
        // control need pre-render for content
        // call UIWindow::RegisterOffScreenRender2D or
        // call UIWindow::RegisterOffScreenRender3D to set
        Flag_NeedRegisterOffScreenRender = 1 << 6,
        // [default: false][auto, no specified]
        // control need Direct3D api to render,
        // call UIWindow::RegisterOffScreenRender3D to set
        // if using Direct2D , call UIWindow::RegisterOffScreenRender2D
        Flag_3DContent = 1 << 7,
        // [default: false][auto, and XML Attribute "renderparent"@bool]
        // if this control will be rendering when do dirty-rendering,
        //  must be rendering whole parent, parent call control->Render() 
        // to render this, auto by parent's flag : Flag_Container_AlwaysRenderChildrenDirectly
        // if a container setted this flag, it will set 
        // flag Flag_Container_AlwaysRenderChildrenDirectly, too,
        // and set all children
        Flag_RenderParent = 1 << 8,
        // control construct with full xml node info
        // if your impl control needed, set it to true by const_cast
        Flag_ControlNeedFullXMLNode = 1 << 9,
        // ------- Lv1 Control Flag ------------
        // [default: false] container 's child renderedt hrough
        // this control, not window directly, if container  hold
        // this flag, will mark all children's Flag_RenderParent to true
        // XML Attribute : "rendercd"@bool
        Flag_Container_AlwaysRenderChildrenDirectly = 1 << 16,
        // if exist marginal control, will set it to true,
        // this is just a optimization flag
        Flag_Container_ExistMarginalContrl = 1 << 17,
        //
        Flag_Edit_MultipleLine = 1 << 16,
        Flag_Slider_VerticalSlider = 1 << 16,
        Flag_CheckBox_WithIndeterminate = 1 << 16,
        // ------- Lv2 Control Flag ------------
        // [default: true] window is using DirectComposition
        // XML Attribute : "dcomp"@bool
        Flag_Window_DComposition = 1 << 20,
        // [default: false] window is always do full-rendering
        // XML Attribute : "fullrender"@bool
        Flag_Window_FullRendering = 1 << 21,
        // [default: false] window is renderer on parent window
        // maybe save some memory
        // XML Attribute : "renderonparent"@bool
        Flag_Window_RenderedOnParentWindow = 1 << 22,
        // [default: false] window is always do rendering with
        // full-rendering, will ingnore Flag_Window_FullRendering
        // XML Attribute : "alwaysrendering"@bool
        Flag_Window_AlwaysRendering = 1 << 23,
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
    // LongUI Event
    enum class Event : size_t {
        // event -- control-tree buliding finished
        Event_TreeBulidingFinished = 0,
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
        // -----control custom
        // button clicked
        Event_ButtoClicked,
        // edit return
        Event_EditReturn,
        // slider value changed
        Event_SliderValueChanged,
        // ----- Custom Event -----(User Defined)
        Event_CustomEvent = 0x400,
    };
    // LongUI Sub Event
    enum class SubEvent : size_t {
        // null
        Event_Null = 0,
        // -----control custom

    };
    // priority for rendering
    enum RenderingPriority : int8_t {
        // last
        Priority_Last = -128,
        // after most
        Priority_AfterMost = -100,
        // low
        Priority_Low = -1,
        // normal
        Priority_Normal = 0,
        // high
        Priority_High = 1,
        // before most
        Priority_BeforeMost = 100,
        // first
        Priority_First = 127,
    };
    // LongUI Event Argument
    struct EventArgument {
        // event id
        union { UINT msg; LongUI::Event event; };
        // sender, null for system
        UIControl*      sender;
        // data
        union {
            // Ststem 
            struct { WPARAM wParam_sys; LPARAM lParam_sys; };
            // clipboard format 
            struct { IDataObject* dataobj_cf; DWORD* outeffect_cf; };
            // control
            struct { LongUI::SubEvent subevent_ui; void* pointer_ui; };
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
        /*/ default ctor
        EventArgument() noexcept =default;
        // copy ctor
        EventArgument(const EventArgument&) noexcept;
        // move ctor
        EventArgument(EventArgument&&) =delete;*/
    };

    // LongUI Information
    enum class Information : size_t {
        // get class name
        Information_GetClassName = 0,
    };
    // longui information
    struct EventInformation {
        // information id
        Information     infomation;
        // Return Code
        union {
            // [out] Control for Parent
            OUT mutable size_t          id;
            // [out] Control for Parent
            OUT mutable const wchar_t*  str;
        };
    };
    // point in rect?
    inline static auto IsPointInRect(const D2D1_RECT_F& rect, const D2D1_POINT_2F& pt) noexcept {
        return(pt.x >= rect.left && pt.y >= rect.top && pt.x < rect.right && pt.y < rect.bottom);
    }
    // point in rect? overload for RectLTWH_F
    inline static auto IsPointInRect(const RectLTWH_F& rect, const D2D1_POINT_2F& pt) noexcept {
        return(pt.x >= rect.left && pt.y >= rect.top && pt.x < rect.left + rect.width && pt.y < rect.top + rect.height);
    }
    // get transformed pointer
    LongUIInline static auto TransformPoint(const D2D1_MATRIX_3X2_F& matrix, const D2D1_POINT_2F& point) noexcept {
        D2D1_POINT_2F result = {
            point.x * matrix._11 + point.y * matrix._21 + matrix._31,
            point.x * matrix._12 + point.y * matrix._22 + matrix._32
        };
        return result;
    }
    // get transformed pointer
    LongUINoinline static auto TransformPointInverse(const D2D1_MATRIX_3X2_F& matrix, const D2D1_POINT_2F& point) noexcept {
        D2D1_POINT_2F result;
        // x = (bn-dm)/(bc-ad)
        // y = (an-cm)/(ad-bc)
        // a : m_matrix._11
        // b : m_matrix._21
        // c : m_matrix._12
        // d : m_matrix._22
        register auto bc_ad = matrix._21 * matrix._12 - matrix._11 * matrix._22;
        register auto m = point.x - matrix._31;
        register auto n = point.y - matrix._32;
        result.x = (matrix._21*n - matrix._22 * m) / bc_ad;
        result.y = (matrix._12*m - matrix._11 * n) / bc_ad;
        return result;
    }
    // 四舍五入
    inline auto RoundToInt(float x) { return static_cast<int>(x + .5f); }
    // script interface
    class IUIScript;
    // LongUI Format Text Config
    struct LongUIAlignas FormatTextConfig {
        // DWrite Factory
        IN  IDWriteFactory*         dw_factory;
        // basic text format
        IN  IDWriteTextFormat*      text_format;
        // inline-param handler
        IN  InlineParamHandler      inline_handler;
        // text layout width
        IN  float                   width;
        // text layout hright
        IN  float                   height;
        // make the text showing progress, maybe you want
        // a "typing-effect", set 1.0f to show all, 0.0f to hide
        IN  float                   progress;
        // the text real(without format) length
        OUT uint32_t                text_length;
    };
}

// 
#include "UIInterface.h"
#include "UIString.h"
// LongUI Util Code
#include "UIAnimation.h"
#include "UIUtil.h"
#include "UIDropSource.h"
#include "UIDataObject.h"
#include "UITextRenderer.h"
#include "UIInlineObject.h"
#include "UIElement.h"
#include "UIEditaleText.h"
#include "UIText.h"
// LongUI Manager
#include "UIInput.h"
#include "UIManager.h"

#include "UIControl.h"
#ifndef LongUICoreOnly // only longui core ?
#   include "UIContainer.h"
#   include "UIScrollBar.h"
#   include "UISlider.h"
#   include "UIVerticalLayout.h"
#   include "UIHorizontalLayout.h"
#   include "UIWindow.h"
#   include "UIEdit.h"
#   include "UIRichEdit.h"
#   include "UILabel.h"
#   include "UICheckBox.h"
#   include "UIButton.h"
#   include "UIMenu.h"
#endif
#endif