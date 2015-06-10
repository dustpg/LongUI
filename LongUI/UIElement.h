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
    // UI Animation for Posotion
    using CUIAnimationPosition = CUIAnimation<D2D1_POINT_2F>;
    // UI Animation for Posotion
    using CUIAnimationTransform = CUIAnimation<D2D1_MATRIX_3X2_F>;
    // Element
    enum class Element : uint32_t {
        Basic = 0,
        Meta,
        ColorRect,
        BrushRect,
        ColorGeometry,
    };
    // Component namespace
    namespace Component {
        // helper for control status as default
        class ControlStatusHelper {
        public:
            // get count
            static constexpr auto GetCount() { return static_cast<uint32_t>(STATUS_COUNT); }
            // get zero for type
            static           auto GetZero() { return Status_Disabled; }
            // get string list
            static const     auto GetList() {
                static const char* s_list[] = { "disabled", "normal", "hover", "pushed" };
                return s_list;
            }
        };
        // class decl
        template<class, Element... > class Elements;
        // basic
        using ElementsBasic = Elements<ControlStatusHelper, Element::Basic>;
        // code bloat limiter, because of "lightweight"
        class CodeBloatLimiter {
        public:
            // Init for Elements<Basic>
            static void __fastcall ElementsBasicInit(
                pugi::xml_node node, const char* prefix,
                CUIAnimationOpacity& anim, uint32_t size, const char** list
                ) noexcept;
            // SetNewStatus for Elements<Basic>
            static auto __fastcall ElementsBasicSetNewStatus(
                ElementsBasic& ele, uint32_t index
                ) noexcept ->float;
            // ------
            // ctor for Elements<Meta>
            static void __fastcall ElementsMetaCtor(
                pugi::xml_node node, const char* prefix,
                uint16_t* first, uint32_t size, const char** list
                ) noexcept;
            // recreate for Elements<Meta>
            static void __fastcall ElementsMetaRecreate(
                Meta* metas, uint16_t* ids, uint32_t size
                ) noexcept;
            // render for Elements<Meta>
            static void __fastcall ElementsMetaRender(
                ElementsBasic& ele, Meta* metas,
                const D2D1_RECT_F& rect
                ) noexcept;
            // ------
            // ctor for Elements<BrushRect>
            static void __fastcall ElementBrushRectCtor(
                pugi::xml_node node, const char * prefix, ID2D1Brush** brushes,
                uint16_t* ids, uint32_t size, const char ** list
                ) noexcept;
            // dtor for Elements<BrushRect>
            static void __fastcall ElementBrushRectDtor(
                ID2D1Brush** brushes, uint32_t size
                ) noexcept;
            // render for Elements<BrushRect>
            static void __fastcall ElementBrushRectRender(
                ElementsBasic& ele, ID2D1Brush** brushes,
                const D2D1_RECT_F& rect
                ) noexcept;
            // recreate for Elements<BrushRect>
            static void __fastcall ElementBrushRectRecreate(
                 ID2D1Brush** brushes, uint16_t* ids, uint32_t size
                ) noexcept;
        };
        // render elements
        template<class Helper, Element Head, Element... Tail>
        class Elements<Helper, Head, Tail...> : 
            protected virtual Elements<Helper, Tail...>, protected Elements<Helper, Head>{
            // super class
            using SuperA = Elements<Helper, Tail...>;
            // super class
            using SuperB = Elements<Helper, Head>;
        public:
            // set unit type
            auto SetElementType(Element unit) noexcept { this->type = unit; }
            // ctor
            Elements(pugi::xml_node node, const char* prefix=nullptr) noexcept : SuperA(node, prefix), SuperB(node, prefix) {}
        public:
            // get element
            template<Element ElementType>
            auto GetByType() noexcept ->Elements<Helper, ElementType>& { return Super::GetByType<ElementType>(); }
            // get element for head
            template<>
            auto GetByType<Head>() noexcept ->Elements<Helper, Head>& { return static_cast<Elements<Helper, Head>&>(*this); }
            // render this
            void Render(const D2D1_RECT_F& rect) noexcept { this->type == Head ? SuperB::Render(rect) : SuperA::Render(rect); }
            // update
            auto Update(float t) noexcept { m_animation.Update(t); }
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
        template<class Helper> class Elements<Helper, Element::Basic> {
            // friend class
            friend class CodeBloatLimiter;
        public:
            // control status
            using StatusForUsing = decltype(Helper::GetZero());
        public:
            // ctor 
            Elements(pugi::xml_node node = LongUINullXMLNode, const char* prefix = nullptr)
                noexcept : m_animation(AnimationType::Type_QuadraticEaseOut) {
                m_animation.end = 1.f;
            }
            // init 
            void Init(pugi::xml_node node, const char* prefix = nullptr) noexcept {
                CodeBloatLimiter::ElementsBasicInit(
                    node, prefix, m_animation, 
                    Helper::GetCount(), Helper::GetList()
                    );
            }
            // set new status
            auto SetNewStatus(StatusForUsing new_status) noexcept {
                return CodeBloatLimiter::ElementsBasicSetNewStatus(
                    reinterpret_cast<ElementsBasic&>(*this),
                    static_cast<uint32_t>(new_status)
                    );
            }
        public:
            // get element
            template<Element ElementType>
            auto GetByType() noexcept ->Elements<Helper, Element::Basic>& { return *this; }
            // recreate
            auto Recreate(LongUIRenderTarget* target) noexcept { m_pRenderTarget = target; return S_OK; }
            // render this
            void Render(const D2D1_RECT_F&) noexcept { }
        public:
            // type of unit
            Element                 type = Element::Basic;
        protected:
            // render target
            LongUIRenderTarget*     m_pRenderTarget = nullptr;
            // state of unit
            uint32_t                m_state = 0;
            // state of unit
            uint32_t                m_stateTartget = 0;
            // animation
            CUIAnimationOpacity     m_animation;
        };
        // element for bitmap
        template<class Helper> class Elements<Helper, Element::Meta> : 
            protected virtual Elements<Helper, Element::Basic>{
            // super class
            using Super = Elements<Helper, Element::Basic>;
        public:
            // control status
            using StatusForUsing = decltype(Helper::GetZero());
        public:
            // ctor
            Elements(pugi::xml_node node, const char* prefix = nullptr) noexcept {
                CodeBloatLimiter::ElementsMetaCtor(
                    node, prefix, m_aID, Helper::GetCount(), Helper::GetList()
                    );
            }
            // get element
            template<Element ElementType>
            auto GetByType() noexcept ->Elements<Helper, Element::Meta>& { return *this; }
            // render this
            void Render(const D2D1_RECT_F& rect) noexcept {
                CodeBloatLimiter::ElementsMetaRender(*this, m_metas, rect);
            }
            // recreate
            auto Recreate(LongUIRenderTarget* target) noexcept {
                CodeBloatLimiter::ElementsMetaRecreate(m_metas, m_aID, Helper::GetCount());
                return S_OK;
            }
            // is OK?
<<<<<<< HEAD
            auto IsOK() noexcept { return m_aID[Status_Normal] != 0; }
=======
            auto IsOK(StatusForUsing index) noexcept { return m_aID[index] != 0; }
>>>>>>> origin/master
        protected:
            // metas
            Meta            m_metas[Helper::GetCount()];
            // metas id
            uint16_t        m_aID[Helper::GetCount()];
        };
        // element for brush rect
        template<class Helper> class Elements<Helper, Element::BrushRect> 
            : protected virtual Elements<Helper, Element::Basic>{
            // super class
            using Super = Elements<Helper, Element::Basic>;
        public:
            // control status
            using StatusForUsing = decltype(Helper::GetZero());
        public:
            // ctor
            Elements(pugi::xml_node node, const char* prefix = nullptr) noexcept {
                CodeBloatLimiter::ElementBrushRectCtor(
                    node, prefix, m_apBrushes, m_aID,
                    Helper::GetCount(), Helper::GetList()
                    );
            }
            // dtor
            ~Elements() noexcept {
                CodeBloatLimiter::ElementBrushRectDtor(
                    m_apBrushes, Helper::GetCount()
                    );
            }
            // get element
            template<Element ElementType>
            auto GetByType() noexcept ->Elements<Helper, Element::BrushRect>& { return *this; }
            // render this
            void Render(const D2D1_RECT_F& rect) noexcept {
                CodeBloatLimiter::ElementBrushRectRender(
                    reinterpret_cast<ElementsBasic&>(static_cast<Super&>(*this)),
                    m_apBrushes, rect
                    );
            }
            // recreate
            auto Recreate(LongUIRenderTarget* target) noexcept {
                //CodeBloatLimiter::ElementBrushRectDtor(
                 //   m_apBrushes, Helper::GetCount()
                   // );
                CodeBloatLimiter::ElementBrushRectRecreate(
                    m_apBrushes, m_aID, Helper::GetCount()
                    );
                return S_OK;
            }
            // change brush
            void ChangeBrush(ControlStatus index, ID2D1Brush* brush) noexcept {
                ::SafeRelease(m_apBrushes[index]);
                m_apBrushes[index] = ::SafeAcquire(brush);
            }
        protected:
            // brush
            ID2D1Brush*     m_apBrushes[Helper::GetCount()];
            // brush id
            uint16_t        m_aID[Helper::GetCount()];
        };
        /*/ element for color rect
        template<class Helper> class Elements<Helper, Element::ColorRect> 
            : protected virtual Elements<Helper, Element::Basic>{
            // super class
            using Super = Elements<Helper, Element::Basic>;
        public:
            // ctor
            Elements(pugi::xml_node node, const char* prefix = nullptr) noexcept;
            // dtor
            ~Elements() noexcept { ::SafeRelease(m_pBrush); }
            // get element
            template<Element ElementType>
            auto GetByType() noexcept ->Elements<Element::ColorRect>& { return *this; }
            // render this
            void Render(const D2D1_RECT_F& rect) noexcept;
            // recreate
            auto Recreate(LongUIRenderTarget* target) noexcept ->HRESULT;
            // change color
            void ChangeColor(ControlStatus index, D2D1_COLOR_F& color) noexcept { m_aColor[index] = color; }
            // change color
            void ChangeColor(ControlStatus index, uint32_t color, float alpha = 1.f) noexcept { m_aColor[index] = D2D1::ColorF(color, alpha); }
        protected:
            // brush id
            D2D1_COLOR_F            m_aColor[Helper::GetCount()];
            // brush
            ID2D1SolidColorBrush*   m_pBrush = nullptr;
        };*/
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