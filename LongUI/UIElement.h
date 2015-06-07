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
    // check mode
#define UIElement_IsMetaMode        (this->metas[Status_Normal].bitmap)
    // set new status
#define UIElement_SetNewStatus(e,s) m_pWindow->StartRender(e.SetNewStatus(s), this)
    // update the node
#define UIElement_Update(a)         a.Update(m_pWindow->GetDeltaTime())
#define UIElement_UpdateA           register float delta_time = m_pWindow->GetDeltaTime()
#define UIElement_UpdateB(a)        a.Update(delta_time)
    // UI Animation for Color
    using CUIAnimationColor = CUIAnimation<D2D1_COLOR_F>;
    // UI Animation for Opacity
    using CUIAnimationOpacity = CUIAnimation<FLOAT> ;
    // UI Animation for Posotion
    using CUIAnimationPosition = CUIAnimation<D2D1_POINT_2F>;
    // UI Animation for Posotion
    using CUIAnimationTransform = CUIAnimation<D2D1_MATRIX_3X2_F>;
    // UIElement
    class CUIElement {
        // render meta: implemented in UIUtil.cpp file
        void __fastcall RenderMeta(Meta&, D2D1_RECT_F*, float) noexcept;
    public:
        // constructor: implemented in UIUtil.cpp file
        CUIElement(const pugi::xml_node = LongUINullXMLNode, const char* prefix = nullptr) noexcept;
        // render: implemented in UIUtil.cpp file
        void __fastcall Render(D2D1_RECT_F* des) noexcept;
        // set new status
        auto __fastcall SetNewStatus(ControlStatus) noexcept ->float;
        // Init status
        void __fastcall InitStatus(ControlStatus) noexcept;
        // update
        void __fastcall Update(float t) { UIElement_IsMetaMode ? animationo.Update(t) : animationc.Update(t); }
        // render target
        LongUIRenderTarget*     target;
        // common brush
        ID2D1SolidColorBrush*   brush;
        // now status
        ControlStatus           old_status;
        // target status
        ControlStatus           tar_status;
        // color of it
        D2D1_COLOR_F            colors[STATUS_COUNT];
        // meta of it
        Meta                    metas[STATUS_COUNT];
        // animation color
        CUIAnimationColor       animationc;
        // animation opacity
        CUIAnimationOpacity     animationo;
    };
#ifdef LONGUI_VIDEO_IN_MF
    // Video Component implemented in UIUtil.cpp file
    class CUIVideoComponent : ComStatic<QiList<IMFMediaEngineNotify>>{
    public: // IMFMediaEngineNotify Interface
        // Event Notify
        virtual HRESULT STDMETHODCALLTYPE EventNotify(
            /* [annotation][in] */
            _In_  DWORD event,
            /* [annotation][in] */
            _In_  DWORD_PTR param1,
            /* [annotation][in] */
            _In_  DWORD param2) noexcept;
    public: // INLINE ZONE
        // has video ?
        LongUIInline auto HasVideo() noexcept { assert(m_pMediaEngine); return m_pMediaEngine->HasVideo(); }
        // has audio ?
        LongUIInline auto HasAudio() noexcept { assert(m_pMediaEngine); return m_pMediaEngine->HasAudio(); }
        // play
        LongUIInline auto Play() noexcept { assert(m_pMediaEngine); return m_pMediaEngine->Play(); }
        // pause
        LongUIInline auto Pause() noexcept { assert(m_pMediaEngine); return m_pMediaEngine->Pause(); }
        // loop?
        LongUIInline auto SetLoop(BOOL b) noexcept { assert(m_pMediaEngine); return m_pMediaEngine->SetLoop(b); }
        // loop?
        LongUIInline auto SetLoop(bool b) noexcept { assert(m_pMediaEngine); return m_pMediaEngine->SetLoop(BOOL(b)); }
        // set volume 
        LongUIInline auto SetVolume(double v) noexcept { assert(m_pMediaEngine); return m_pMediaEngine->SetVolume(v); }
        // set volume 
        LongUIInline auto SetVolume(float v) noexcept { assert(m_pMediaEngine); return m_pMediaEngine->SetVolume(double(v)); }
        // set playback rate 
        LongUIInline auto SetPlaybackRate(double v) noexcept { assert(m_pMediaEngine); return m_pMediaEngine->SetPlaybackRate(v); }
        // set playback rate  
        LongUIInline auto SetPlaybackRate(float v) noexcept { assert(m_pMediaEngine); return m_pMediaEngine->SetPlaybackRate(double(v)); }
        // pause
        LongUIInline auto GetCurrentSource(BSTR* s) noexcept { assert(m_pMediaEngine); return m_pMediaEngine->GetCurrentSource(s); }
        // set source
        LongUIInline auto SetSource(BSTR src) { assert(m_pMediaEngine); m_pMediaEngine->SetSource(src); this->recreate_surface(); return m_pMediaEngine->Load(); }
        // set source path
        LongUIInline auto SetSourcePath(const wchar_t* src) { wchar_t path[LongUIStringBufferLength]; ::wcscpy(path, src); return this->SetSource(path); }
        // get source path
        LongUIInline auto GetSourcePath(wchar_t* path) { BSTR url = nullptr; this->GetCurrentSource(&url); if (url) { ::wcscpy(path, url); ::SysFreeString(url); } else *path = 0; }
    public:
        // init: because it maybe call virtual-method, so, not in ctor
        HRESULT Init() noexcept;
        // recreate
        HRESULT Recreate(ID2D1RenderTarget*) noexcept;
        // Render
        void Render(D2D1_RECT_F* dst) noexcept;
        // ctor
        CUIVideoComponent() noexcept;
        // dtor
        ~CUIVideoComponent() noexcept;
    private:
        // recreate surface
        HRESULT recreate_surface() noexcept;
    public:
        // dst rect
        RECT                     const  dst_rect = RECT();
    private:
        // Render Target
        ID2D1RenderTarget*              m_pRenderTarget = nullptr;
        // MF Media Engine
        IMFMediaEngine*                 m_pMediaEngine = nullptr;
        // MF Media Engine Ex-Ver
        //IMFMediaEngineEx*               m_pEngineEx = nullptr;
        // D3D11 2D Texture
        ID3D11Texture2D*                m_pTargetSurface = nullptr;
        // D2D Bitmap
        ID2D1Bitmap1*                   m_pSharedSurface = nullptr;
        // D2D Bitmap
        ID2D1Bitmap1*                   m_pDrawSurface = nullptr;
        // is end of stream
        bool                            m_bEOS = false;
        // is playing
        bool                            m_bPlaying = false;
        // unused
        bool                            unused[sizeof(void*) - sizeof(bool) * 2];
    };
#endif
}