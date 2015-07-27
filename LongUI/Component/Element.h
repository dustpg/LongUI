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
    // set new status
#define UIElement_SetNewStatus(e,s) m_pWindow->StartRender(e.GetByType<Element::Basic>().SetNewStatus(s), this)
    // update the node
#define UIElement_Update(a)         a.Update(m_pWindow->GetDeltaTime())
    // UI Animation for Color
    using CUIAnimationColor = CUIAnimation<D2D1_COLOR_F>;
    // UI Animation for Opacity
    using CUIAnimationOpacity = CUIAnimation<FLOAT>;
    // UI Animation for offset
    using CUIAnimationOffset= CUIAnimation<FLOAT>;
    // UI Animation for Posotion
    using CUIAnimationPosition = CUIAnimation<D2D1_POINT_2F>;
    // UI Animation for Posotion
    using CUIAnimationTransform = CUIAnimation<D2D1_MATRIX_3X2_F>;
    // Element
    enum class Element : uint32_t {
        Basic = 0,          // basic element for animation
        Meta,               // meta element for rendering meta
        ColorRect,          // colorrect element for rendering colored-rect
        BrushRect,          // brushrect element for rendering brushed-rect
        ColorGeometry,      // colorgeo element for rendering colored-geometry
    };
    // Component namespace
    namespace Component {
    // class decl
        template<Element... > class Elements;
        // render unit
        template<Element Head, Element... Tail>
        class Elements<Head, Tail...> : protected virtual Elements<Tail...>, protected Elements<Head> {
            // super class
            using SuperA = Elements<Tail...>;
            // super class
            using SuperB = Elements<Head>;
        public:
            // set unit type
            auto SetElementType(Element unit) noexcept { this->type = unit; }
            // ctor
            Elements(pugi::xml_node node, const char* prefix=nullptr) noexcept : SuperA(node, prefix), SuperB(node, prefix) {}
        public:
            // get element
            template<Element ElementType>
            auto GetByType() noexcept ->Elements<ElementType>& { return SuperA::GetByType<ElementType>(); }
            // get element for head
            template<>
            auto GetByType<Head>() noexcept ->Elements<Head>& { return static_cast<Elements<Head>&>(*this); }
            // get element: const overload
            template<Element ElementType> 
            auto GetByType() const noexcept ->const Elements<ElementType>& { return SuperA::GetByType<ElementType>(); }
            // get element for head: const overload
            template<>
            auto GetByType<Head>() const noexcept ->const Elements<Head>& { return static_cast<const Elements<Head>&>(*this); }
            // render this
            void Render(const D2D1_RECT_F& rect) const noexcept { this->type == Head ? SuperB::Render(rect) : SuperA::Render(rect); }
            // update
            auto Update(float t) noexcept { animation.Update(t); }
            // recreate
            auto Recreate(LongUIRenderTarget* target) noexcept {
                HRESULT hr = S_OK;
                if (SUCCEEDED(hr)) {
                    hr = SuperA::Recreate(target);
                }
                if (SUCCEEDED(hr)) {
                    hr = SuperB::Recreate(target);
                }
                return hr;
            }
        };
        // element for all
        template<> class Elements<Element::Basic> {
        public:
            // ctor 
            Elements(pugi::xml_node node = LongUINullXMLNode, const char* prefix = nullptr)
                noexcept : animation(AnimationType::Type_QuadraticEaseOut) {
                UNREFERENCED_PARAMETER(node);
                UNREFERENCED_PARAMETER(prefix);
                animation.end = 1.f;
            }
            // init 
            void Init(pugi::xml_node node, const char* prefix = nullptr) noexcept;
            // render this
            void Render(const D2D1_RECT_F&) const noexcept { }
            // get element
            template<Element ElementType>
            auto GetByType() noexcept ->Elements<Element::Basic>& { return *this; }
            // get element: const overload
            template<Element ElementType>
            auto GetByType()const noexcept ->const Elements<Element::Basic>& { return *this; }
            // set new status
            auto SetNewStatus(ControlStatus) noexcept ->float;
            // get status
            auto GetStatus() const noexcept { return m_stateTartget; }
            // recreate
            auto Recreate(LongUIRenderTarget* target) noexcept { m_pRenderTarget = target; return S_OK; }
            // type of unit
            Element                 type = Element::Basic;
        protected:
            // render target
            LongUIRenderTarget*     m_pRenderTarget = nullptr;
            // state of unit
            ControlStatus           m_state = ControlStatus::Status_Disabled;
            // state of unit
            ControlStatus           m_stateTartget = ControlStatus::Status_Disabled;
        public:
            // animation
            CUIAnimationOpacity     animation;
        };
        // element for bitmap
        template<> class Elements<Element::Meta> : protected virtual Elements<Element::Basic>{
            // super class
            using Super = Elements<Element::Basic>;
        public:
            // ctor
            Elements(pugi::xml_node node, const char* prefix = nullptr) noexcept;
            // get element
            template<Element ElementType>
            auto GetByType() noexcept ->Elements<Element::Meta>& { return *this; }
            // get element: const overload
            template<Element ElementType>
            auto GetByType()const noexcept ->const Elements<Element::Meta>& { return *this; }
            // render this
            void Render(const D2D1_RECT_F&) const noexcept;
            // recreate
            auto Recreate(LongUIRenderTarget* target) noexcept->HRESULT;
            // is OK?
            auto IsOK() noexcept { return m_aID[Status_Normal] != 0; }
        protected:
            // metas
            Meta            m_metas[STATUS_COUNT];
            // metas id
            uint16_t        m_aID[STATUS_COUNT];
        };
        // element for brush rect
        template<> class Elements<Element::BrushRect> : protected virtual Elements<Element::Basic>{
            // super class
            using Super = Elements<Element::Basic>;
        public:
            // ctor
            Elements(pugi::xml_node node, const char* prefix = nullptr) noexcept;
            // dtor
            ~Elements() noexcept { this->release_data(); }
            // get element
            template<Element ElementType>
            auto GetByType() noexcept ->Elements<Element::BrushRect>& { return *this; }
            // get element : const overload
            template<Element ElementType>
            auto GetByType()const noexcept ->const Elements<Element::BrushRect>& { return *this; }
            // render this
            void Render(const D2D1_RECT_F& rect) const noexcept;
            // recreate
            auto Recreate(LongUIRenderTarget* target) noexcept->HRESULT;
            // change brush
            void ChangeBrush(ControlStatus index, ID2D1Brush* brush) noexcept {
                ::SafeRelease(m_apBrushes[index]);
                m_apBrushes[index] = ::SafeAcquire(brush);
            }
        private:
            // relase data
            void release_data() noexcept;
        protected:
            // brush
            ID2D1Brush*     m_apBrushes[STATUS_COUNT];
            // brush id
            uint16_t        m_aID[STATUS_COUNT];
        };
        // element for color rect
        template<> class Elements<Element::ColorRect> : protected virtual Elements<Element::Basic>{
            // super class
            using Super = Elements<Element::Basic>;
        public:
            // ctor
            Elements(pugi::xml_node node, const char* prefix = nullptr) noexcept;
            // dtor
            ~Elements() noexcept { ::SafeRelease(m_pBrush); }
            // get element
            template<Element ElementType>
            auto GetByType() noexcept ->Elements<Element::ColorRect>& { return *this; }
            // get element: const overload
            template<Element ElementType>
            auto GetByType()const noexcept ->const Elements<Element::ColorRect>& { return *this; }
            // render this
            void Render(const D2D1_RECT_F& rect) const noexcept;
            // recreate
            auto Recreate(LongUIRenderTarget* target) noexcept ->HRESULT;
            // change color
            void ChangeColor(ControlStatus index, D2D1_COLOR_F& color) noexcept { colors[index] = color; }
            // change color
            void ChangeColor(ControlStatus index, uint32_t color, float alpha = 1.f) noexcept { colors[index] = D2D1::ColorF(color, alpha); }
        public:
            // brush id
            D2D1_COLOR_F            colors[STATUS_COUNT];
        protected:
            // brush
            ID2D1SolidColorBrush*   m_pBrush = nullptr;
        };
    }
#ifdef LONGUI_VIDEO_IN_MF
    // Video Component implemented in UIUtil.cpp file
    class CUIVideoComponent : ComStatic<QiList<IMFMediaEngineNotify>> {
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
        void Render(D2D1_RECT_F* dst) const noexcept;
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