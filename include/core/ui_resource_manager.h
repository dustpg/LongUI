#pragma once
/**
* Copyright (c) 2014-2020 dustpg   mailto:dustpg@gmail.com
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

#include "ui_core_type.h"
#include "ui_basic_type.h"
#include "../text/ui_ctl_decl.h"
#include "../resource/ui_resource.h"
#include "../graphics/ui_graphics_decl.h"

// ui namespace
namespace LongUI {
/*
     system-supported image file format list:
      - png
      - jpg
      - gif
      - and more
*/
    // rgba-color
    union RGBA;
    // config
    struct IUIConfigure;
    // configure flag
    enum ConfigureFlag : uint32_t;
    // color float
    struct ColorF;
    // UI Window Manager
    class CUIResMgr {
        // screen
        struct IScreen;
        // Debug
        struct Debug;
        // private data
        enum : size_t {
            // pointer count
            p_ptr_count = 12 ,
            // 32bit count
            p_32b_count = 10,
            // size count
            p_size =    p_ptr_count * sizeof(void*) + 
                        p_32b_count * sizeof(uint32_t)
        };
        // type of m_private
        struct private_t { alignas(void*) char buf[p_size]; };
    public:
        // private impl
        struct Private;
        // get graphics factory
        auto&RefGraphicsFactory() noexcept { return *m_pGraphicsFactory; }
        // get 2d renderer
        auto&Ref2DRenderer() noexcept { return *m_p2DRenderer; }
        // get 3d device
        auto&Ref3DDevice() noexcept { return *m_p3DDevice; }
        // get native renderer
        auto GetNativeRenderer() const noexcept { return m_pNativeStyle; }
        // get common color brush with color
        static auto RefCCBrush(const ColorF&) noexcept->I::Brush&;
        // ref 2d factory, use reinterpret_cast<XX&>
        static auto Ref2DFactory() noexcept ->int&;
    public:
        // create ctl font
        auto CreateCtlFont(const FontArg&, I::Font*&, const StyleText*s=nullptr) noexcept->Result;
        // create ctl text
        auto CreateCtlText(const TextArg&, I::Text*&) noexcept->Result;
        // get default font data
        auto GetDefaultFont() const noexcept -> const FontArg&;
    public:
        // set alpha ch. premultiply for reading image
        void SetAlphaMode(bool premultiply) noexcept;
#if 0
        // load resource, return id
        auto LoadResource(U8View, ResourceType, bool is_xul_dir) noexcept->uintptr_t;
#endif
        // load resource, return id, use CUIResourceID hanle this
        auto LoadResource(U8View, bool is_xul_dir) noexcept ->uintptr_t;
        // save as png
        auto SaveAsPng(I::Bitmap& bmp, const wchar_t* file) noexcept->Result;
        // create bitmap from system-supported image file
        auto CreateBitmapFromSSImageFile(U8View view, I::Bitmap*&) noexcept->Result;
        // create bitmap from system-supported image memory
        auto CreateBitmapFromSSImageMemory(uint8_t* ptr, uint32_t len, I::Bitmap*&) noexcept->Result;
        // create bitmap with init-data
        auto CreateBitmap(Size2U size, const RGBA* color, uint32_t pitch, I::Bitmap*&) noexcept->Result;
        // create bitmap without init-data
        auto CreateBitmap(Size2U size, I::Bitmap*& bmp) noexcept { return CreateBitmap(size, nullptr, 0, bmp); }
    private:
        // create bitmap
        auto create_bitmap_private(uint8_t*, uint32_t, void*&) noexcept->Result;
    protected:
        // init default font data
        auto init_default_font(IUIConfigure*) noexcept->Result;
    protected:
        // graphics factory
        I::FactoryGraphics*     m_pGraphicsFactory = nullptr;
        // 3d device
        I::Device3D*            m_p3DDevice = nullptr;
        // 3d renderer
        I::Renderer3D*          m_p3DRenderer = nullptr;
        // 2d renderer
        I::Renderer2D*          m_p2DRenderer = nullptr;
        // main screen
        IScreen*                m_pMainScreen = nullptr;
        // native style renderer
        void*                   m_pNativeStyle = nullptr;
    protected:
        // common color brush
        I::Brush*               m_pCommonBrush = nullptr;
    private:
#ifndef NDEBUG
        // debug
        Debug*                  m_pDebug = nullptr;
#endif
        // private data
        private_t               m_private;
        // PrivateResMgr data
        auto&rm() noexcept { return reinterpret_cast<Private&>(m_private); }
        // PrivateResMgr data
        auto&rm() const noexcept { return reinterpret_cast<const Private&>(m_private); }
        // release device
        void release_device() noexcept;
        // release res-list
        void release_res_list() noexcept;
    protected:
        // original local name length, LOCALE_NAME_MAX_LENGTH = 85
        enum { olnl = 85 + 3 };
        // local name
        char16_t                m_szLocaleName[olnl];
    protected:
        // redirect screen
        void redirect_screen() noexcept;
        // wait for vblank
        bool wait_for_vblank() noexcept;
        // recreate device
        auto recreate_device(IUIConfigure*, ConfigureFlag) noexcept->Result;
        // recreate resource
        auto recreate_resource() noexcept->Result;
        // ctor
        CUIResMgr(IUIConfigure* cfg, Result& hr) noexcept;
        // ctor
        CUIResMgr(const CUIResMgr&) noexcept = delete;
        // ctor
        CUIResMgr(CUIResMgr&&) noexcept = delete;
        // dtor
        ~CUIResMgr() noexcept;
    };
}
