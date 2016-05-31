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

#include "../luibase.h"
#include "../luiconf.h"
#include <d2d1_3.h>
#include <dwrite_1.h>
#include "../Graphics/luiGrHlper.h"
#include "../Graphics/luiGrColor.h"

// longui namespace
namespace LongUI {
    // Text Render Type
    enum TextRendererType : uint32_t {
        // normal: For CUINormalTextRender
        Type_NormalTextRenderer = 0,
        // outline: For UIOutlineTextRender
        Type_OutlineTextRenderer ,
        // Path: For UIPathTextRender
        Type_PathTextRenderer,
        // User Custom Define
        Type_UserDefineFirst,
    };
    // Basic TextRenderer
    class LONGUI_NOVTABLE XUIBasicTextRenderer : public Helper::ComStatic<
        Helper::QiListSelf<XUIBasicTextRenderer, 
        Helper::QiList<IDWriteTextRenderer, 
        Helper::QiList<IDWritePixelSnapping, 
        Helper::QiList<IUnknown>>>>> {
    public:
        // destructor
        virtual ~XUIBasicTextRenderer() noexcept { LongUI::SafeRelease(UIManager_RenderTarget); LongUI::SafeRelease(m_pBrush);}
        // constructor
        XUIBasicTextRenderer(TextRendererType t) noexcept :type(t), basic_color(1) { /*basic_color.userdata = 0;*/ basic_color.color = { 0.f,0.f,0.f,1.f }; }
        // set new render target
        auto ChangeTarget(ID2D1RenderTarget* rt) noexcept -> HRESULT;
    public:
        // is pixel snapping disabled?
        virtual HRESULT STDMETHODCALLTYPE IsPixelSnappingDisabled(void*, BOOL*) noexcept final override;
        // get current transform
        virtual HRESULT STDMETHODCALLTYPE GetCurrentTransform(void*, DWRITE_MATRIX*) noexcept final override;
        // get bilibili of px/pt
        virtual HRESULT STDMETHODCALLTYPE GetPixelsPerDip(void*, FLOAT*) noexcept final override;
    public:
        // draw inline object implemented as template
        virtual HRESULT STDMETHODCALLTYPE DrawInlineObject(
            _In_opt_ void* clientDrawingContext,
            FLOAT originX,
            FLOAT originY,
            _In_ IDWriteInlineObject* inlineObject,
            BOOL isSideways,
            BOOL isRightToLeft,
            _In_opt_ IUnknown* clientDrawingEffect
            ) noexcept override;
        // draw underline
        virtual HRESULT STDMETHODCALLTYPE DrawUnderline(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            const DWRITE_UNDERLINE* underline,
            IUnknown* clientDrawingEffect
        ) noexcept override;
        // draw strikethrough
        virtual HRESULT STDMETHODCALLTYPE DrawStrikethrough(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            const DWRITE_STRIKETHROUGH* strikethrough,
            IUnknown* clientDrawingEffect
        ) noexcept override;
    public:
        // fill rect
        void FillRect(const D2D1_RECT_F&) noexcept;
    public:
        // get the render context size in byte
        virtual auto GetContextSizeInByte() noexcept ->size_t = 0;
        // make context from string
        virtual void MakeContextFromString(void* context, const char* utf8_string) noexcept = 0;
    protected:
        // render target of d2d
        ID2D1DeviceContext*         UIManager_RenderTarget = nullptr;
        // solid color brush
        ID2D1SolidColorBrush*       m_pBrush = nullptr;
    public:
        // temp render target
        ID2D1RenderTarget*          target = nullptr;
        // basic color
        CUIColorEffect              basic_color;
        // type of text renderer
        TextRendererType      const type;
    };
    // Normal Text Render: Render Context -> None
    class  CUINormalTextRender : public XUIBasicTextRenderer {
        // superclass define
        using Super = XUIBasicTextRenderer ;
    public:
        // CUINormalTextRender
        CUINormalTextRender() : Super(Type_NormalTextRenderer) { }
    public:
        // draw glyphrun
        virtual HRESULT STDMETHODCALLTYPE DrawGlyphRun(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_MEASURING_MODE measuringMode,
            const DWRITE_GLYPH_RUN* glyphRun,
            const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDescription,
            IUnknown* clientDrawingEffect
        ) noexcept override;
    public:
        // get the render context size in byte
        virtual auto GetContextSizeInByte() noexcept ->size_t override { return 0; }
        // make context from string
        virtual void MakeContextFromString(void* context, const char* utf8_string) noexcept {
            UNREFERENCED_PARAMETER(context); UNREFERENCED_PARAMETER(utf8_string);
        };
    };
    // Outline Text Renderer: Render Context -> outline color, outline size
    class  CUIOutlineTextRender : public XUIBasicTextRenderer {
        // superclass define
        using Super = XUIBasicTextRenderer ;
        // text context
        struct OutlineContext {
            // color
            D2D1_COLOR_F    color;
            // stroke width
            float           width;
        };
    public:
        // CUINormalTextRender
        CUIOutlineTextRender() : Super(Type_NormalTextRenderer) { }
    public:
        // draw glyphrun
        virtual HRESULT STDMETHODCALLTYPE DrawGlyphRun(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_MEASURING_MODE measuringMode,
            const DWRITE_GLYPH_RUN* glyphRun,
            const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDescription,
            IUnknown* clientDrawingEffect
        ) noexcept override;
    public:
        // get the render context size in byte
        virtual auto GetContextSizeInByte() noexcept ->size_t override  { return sizeof(OutlineContext); }
        // make context from string
        virtual void MakeContextFromString(void* context, const char* utf8_string) noexcept;
    };
}