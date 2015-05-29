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
    // Text Render Type
    enum TextRendererType : uint32_t {
        // Normal: For UINormalTextRender
        Type_NormalTextRenderer = 0,
        // Outline: For UIOutlineTextRender
        Type_OutlineTextRenderer ,
        // Path: For UIPathTextRender
        Type_PathTextRenderer,
        // User Custom Define
        Type_UserDefineFirst,
    };
    // Basic TextRenderer
    class DECLSPEC_NOVTABLE UIBasicTextRenderer : public ComStatic<
        QiListSelf<UIBasicTextRenderer, QiList<IDWriteTextRenderer>>> {
    public:
        // destructor
        ~UIBasicTextRenderer()  noexcept { ::SafeRelease(m_pRenderTarget); ::SafeRelease(m_pBrush);}
        // constructor
        UIBasicTextRenderer(TextRendererType t) noexcept :type(t) { basic_color.userdata = 0; basic_color.color = { 0.f,0.f,0.f,1.f }; }
        // set new render target
        void SetNewRT(LongUIRenderTarget* rt) { ::SafeRelease(m_pRenderTarget); m_pRenderTarget = ::SafeAcquire(rt); }
        // set new render brush
        void SetNewBrush(ID2D1SolidColorBrush* b) { ::SafeRelease(m_pBrush); m_pBrush = ::SafeAcquire(b); }
    public: // IDWritePixelSnapping implementation
        // is pixel snapping disabled?
        LONGUICOMMETHOD IsPixelSnappingDisabled(void*, BOOL*) noexcept final override;
        // get current transform
        LONGUICOMMETHOD GetCurrentTransform(void*, DWRITE_MATRIX*) noexcept final override;
        // get bilibili of px/pt
        LONGUICOMMETHOD GetPixelsPerDip(void*, FLOAT*) noexcept final override;
    public:// IDWriteTextRenderer implementation
        // draw inline object implemented as template
        LONGUICOMMETHOD DrawInlineObject(
            _In_opt_ void* clientDrawingContext,
            FLOAT originX,
            FLOAT originY,
            _In_ IDWriteInlineObject* inlineObject,
            BOOL isSideways,
            BOOL isRightToLeft,
            _In_opt_ IUnknown* clientDrawingEffect
            ) noexcept override;
    public: // LongUI UIBasicTextRenderer 
        // get the render context size in byte
        virtual auto GetContextSizeInByte() noexcept->uint32_t = 0;
        // create context from string
        virtual void CreateContextFromString(void* context, const char* utf8_string) noexcept = 0;
    protected:
        // render target of d2d
        LongUIRenderTarget*         m_pRenderTarget = nullptr;
        // solid color brush
        ID2D1SolidColorBrush*       m_pBrush = nullptr;
    public:
        // baisc color
        UIColorEffect               basic_color;
        // type of text renderer
        TextRendererType      const type;
    };
    // Normal Text Render
    // Render Context -> None
    class  UINormalTextRender : public UIBasicTextRenderer {
        // superclass define
        using Super = UIBasicTextRenderer ;
    public:
        // UINormalTextRender
        UINormalTextRender():Super(Type_NormalTextRenderer){ }
    public:// IDWriteTextRenderer implementation
        // draw glyphrun
        LONGUICOMMETHOD DrawGlyphRun(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_MEASURING_MODE measuringMode,
            const DWRITE_GLYPH_RUN* glyphRun,
            const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDescription,
            IUnknown* clientDrawingEffect
            ) noexcept override;
        // draw underline
        LONGUICOMMETHOD DrawUnderline(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            const DWRITE_UNDERLINE* underline,
            IUnknown* clientDrawingEffect
            ) noexcept override;
        // draw strikethrough
        LONGUICOMMETHOD DrawStrikethrough(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            const DWRITE_STRIKETHROUGH* strikethrough,
            IUnknown* clientDrawingEffect
            ) noexcept override;
    public:// UIBasicTextRenderer implementation
        // get the render context size in byte
        virtual auto GetContextSizeInByte() noexcept ->uint32_t override  { return 0ui32; }
        // create context from string
        virtual void CreateContextFromString(void* context, const char* utf8_string) noexcept {};
    };
}