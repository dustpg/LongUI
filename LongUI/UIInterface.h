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
    // LongUI UI Interface
    class DECLSPEC_NOVTABLE IUIInterface {
    public:
        // release it
        virtual auto Release() noexcept -> int32_t = 0;
    };
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
    class LongUIAPI CUIManager;
    // script
    class DECLSPEC_NOVTABLE IUIScript : public IUIInterface {
    public:
        // run a section script with event
        virtual auto Evaluation(const UIScript, const LongUI::EventArgument&) noexcept->size_t = 0;
        // get config infomation
        virtual auto GetConfigInfo() noexcept->ScriptConfigInfo = 0;
        // initialize the object
        virtual auto Initialize(CUIManager*) noexcept ->bool = 0;
        // un-initialize the object
        virtual auto UnInitialize() noexcept->void = 0;
        // alloc the script memory and copy into(may be compiled into byte code), return memory size
        virtual auto AllocScript(const char* utf8) noexcept->LongUI::UIScript = 0;
        // free the script memory
        virtual auto FreeScript(UIScript&) noexcept->void = 0;
    };
    // Meta
    struct Meta;
    // UI Binary Resource Loader
    class DECLSPEC_NOVTABLE IUIBinaryResourceLoader : public IUIInterface {
    public:
        // get count of bitmap in resouce
        virtual auto GetBitmapCount() noexcept -> size_t = 0;
        // get bitmap by index, index in range [0, bitmap_count)
        virtual auto LoadBitmapAt(CUIManager& manager, size_t index) noexcept ->ID2D1Bitmap1* = 0;
        // get count of brush in resouce
        virtual auto GetBrushCount() noexcept -> size_t = 0;
        // get brush by index, index in range [0, brush_count)
        virtual auto LoadBrushAt(CUIManager& manager, size_t index) noexcept ->ID2D1Brush* = 0;
        // get count of textformat in resouce
        virtual auto GetTextFormatCount() noexcept->size_t = 0;
        // get brush by index, index in range [0, format_count)
        virtual auto LoadTextFormatAt(CUIManager& manager, size_t index) noexcept->IDWriteTextFormat* = 0;
        // get count of textformat in resouce
        virtual auto GetMetaCount() noexcept->size_t = 0;
        // get brush by index, index in range [0, meta_count)
        virtual auto LoadMetaAt(CUIManager& manager, size_t index, LongUI::Meta& meta) noexcept->void = 0;
    };
    // UI Configure
    class DECLSPEC_NOVTABLE IUIConfigure : public IUIInterface {
    public:
        // get bin-res loader, return nullptr for xml-based resource
        virtual auto GetBinResLoader() noexcept->IUIBinaryResourceLoader* = 0;
        // if no bin-res loader, get xml based resource(not file name), maybe nullptr(no resource)
        virtual auto GetResourceXML() noexcept -> const char* = 0;
        // get script interface, maybe nullptr(no script)
        virtual auto GetScript() noexcept ->IUIScript* = 0;
        // get inline param handler
        virtual auto GetInlineParamHandler() noexcept->InlineParamHandler = 0;
        // create font collection, maybe nullptr(using system default)
        virtual auto CreateFontCollection(CUIManager& manager) noexcept ->IDWriteFontCollection* = 0;
        // get locale name of ui(for text), default is L"" (locale)
        virtual auto GetLocaleName(wchar_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept->void = 0;
        // if no bin-res loader,  create bitmap from resource identifier
        virtual auto LoadBitmapByRI(CUIManager& manager, const char* res_iden) noexcept->ID2D1Bitmap1* = 0;
        // add all custom controls, just return if no custom control
        virtual auto AddCustomControl(CUIManager& manager) noexcept->void = 0;
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
        virtual auto ShowError(const wchar_t* str_a, const wchar_t* str_b=nullptr) noexcept -> void = 0;
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