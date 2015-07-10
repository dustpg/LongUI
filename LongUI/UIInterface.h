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



// longui namespace
namespace LongUI {
    // LongUI UI Interface, IUnkown like interface
    class DECLSPEC_NOVTABLE IUIInterface { 
    public:
        // qi
        virtual auto STDMETHODCALLTYPE QueryInterface(const IID& riid, void** ppvObject) noexcept->HRESULT = 0;
        // add ref count
        virtual auto STDMETHODCALLTYPE AddRef() noexcept->ULONG = 0;
        // release
        virtual auto STDMETHODCALLTYPE Release() noexcept ->ULONG = 0;
    };
#define LONGUI_BASIC_INTERFACE_IMPL\
    auto STDMETHODCALLTYPE QueryInterface(const IID& riid, void** ppvObject) noexcept->HRESULT override final { return E_NOINTERFACE; }\
    auto STDMETHODCALLTYPE AddRef() noexcept->ULONG override final { return 2; }\
    auto STDMETHODCALLTYPE Release() noexcept->ULONG override final { return 1; };
    // Script define
    struct UIScript {
        UIScript() = default;
        union {
            const char*     script = nullptr;
            const uint8_t*  scriptb;
            void*           data;
        };
        size_t              size = 0;
    };
    // the script config, bit array
    enum ScriptConfigInfo : uint32_t {
        // All infomation
        Info_All = uint32_t(-1),
        // none infomation
        Info_None = 0,
        // is the script will be compiled into byte code?
        Info_CompiledCode = 1 << 0,
        // is the script will create/release/change longui control?
        Info_ManageControl = 1 << 1,
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
    // pre-dec 提前声明
    class CUIManager;
    // script {09B531BD-2E3B-4C98-985C-1FD6B406E53D}
    static const GUID IID_IUIScript =
    { 0x9b531bd, 0x2e3b, 0x4c98, { 0x98, 0x5c, 0x1f, 0xd6, 0xb4, 0x6, 0xe5, 0x3d } };
    class DECLSPEC_NOVTABLE IUIScript : public IUIInterface {
    public:
        // run a section script with event
        virtual auto Evaluation(const UIScript, const LongUI::EventArgument&) noexcept->size_t = 0;
        // get config infomation
        virtual auto GetConfigInfo() noexcept->ScriptConfigInfo = 0;
        // alloc the script memory and copy into(may be compiled into byte code), return memory size
        virtual auto AllocScript(const char* utf8) noexcept->LongUI::UIScript = 0;
        // free the script memory
        virtual auto FreeScript(UIScript&) noexcept->void = 0;
    };
    // Meta
    struct Meta; struct DeviceIndependentMeta;
    // {16222E4B-9AC8-4756-8CA9-75A72D2F4F60}
    static const GUID IID_IUIResourceLoader = 
    { 0x16222e4b, 0x9ac8, 0x4756,{ 0x8c, 0xa9, 0x75, 0xa7, 0x2d, 0x2f, 0x4f, 0x60 } };
    // UI Binary Resource Loader
    class DECLSPEC_NOVTABLE IUIResourceLoader : public IUIInterface {
    public:
        // resource type
        enum ResourceType : uint32_t {
            Type_Null =0,       // none resource
            Type_Bitmap,        // bitmap within ID2D1Bitmap1*
            Type_Brush,         // brush with ID2D1Brush*
            Type_TextFormat,    // text format within IDWriteTextFormat*
            Type_Meta,          // meta within LongUI::Meta
            RESOURCE_TYPE_COUNT,// count of this
        };
    public:
        // get resouce count with type
        virtual auto GetResourceCount(ResourceType type) const noexcept -> size_t = 0;
        // get resouce by index, index in range [0, count),  for Type_Bitmap, Type_Brush, Type_TextFormat
        virtual auto GetResourcePointer(ResourceType type, size_t index) noexcept ->void* = 0;
        // get meta by index, index in range [0, count)
        virtual auto GetMeta(size_t index, DeviceIndependentMeta&) noexcept ->void = 0;
    };
    // static const GUID IID_IUIConfigure =
    // { 0x7ca331b9, 0x6500, 0x4948,{ 0xa9, 0xb4, 0xd5, 0x59, 0xc9, 0x2e, 0x65, 0xb1 } };
    // UI Configure
    // can be QI :  IID_LONGUI_InlineParamHandler(opt),
    //              IID_LONGUI_IUIResourceLoader(opt), 
    //              IID_LONGUI_IUIScript(opt),
    //              IDWriteFontCollection(opt)
    class DECLSPEC_NOVTABLE IUIConfigure : public IUIInterface {
    public:
        // get template string for control
        virtual auto GetTemplateString() noexcept->const char* = 0;
        // get locale name of ui(for text), default is L"" (locale)
        virtual auto GetLocaleName(wchar_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept->void = 0;
        // add all custom controls, just return if no custom control
        virtual auto AddCustomControl() noexcept->void = 0;
        // return true, if using cpu rendering
        virtual auto IsRenderByCPU() noexcept->bool = 0;
        // if using gpu render, you should choose a video card,return the index,
        // if return code out of range, will set by default(null pointer adapter)
        // btw, in the adapter list, also include the SOFTWARE-adapter
        virtual auto ChooseAdapter(IDXGIAdapter1* adapters[/*length*/], size_t const length /*length <=256*/) noexcept->size_t = 0;
        // SetEventCallBack for custom control
        // in normal case, you just return and say "不方便" is ok
        virtual auto SetEventCallBack(LongUI::Event, LongUICallBack call, UIControl* target, UIControl* caller) noexcept -> void = 0;
        // show the error string
        virtual auto ShowError(const wchar_t* str_a, const wchar_t* str_b = nullptr) noexcept -> void = 0;
#ifdef _DEBUG
        // output the debug string
        virtual auto OutputDebugStringW(DebugStringLevel level, const wchar_t* string, bool flush) noexcept -> void = 0;
#endif
    };
    // UI Undo Redo Commnad
    class DECLSPEC_NOVTABLE IUICommand : public IUIInterface {
    public:
        // undo
        virtual void Undo() noexcept = 0;
        // redo
        virtual void Redo() noexcept = 0;
    };
}

