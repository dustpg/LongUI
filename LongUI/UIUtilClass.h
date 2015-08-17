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
    // caller struct
    class CUISubEventCaller {
    public:
        // ctor
        explicit CUISubEventCaller(SubEventCallBack callback = nullptr, UIControl* recver = nullptr)
            :m_pCallback(callback), m_pRecver(recver){}
        // copy ctor
        explicit CUISubEventCaller(const CUISubEventCaller& obj) noexcept
            :m_pCallback(obj.m_pCallback), m_pRecver(obj.m_pRecver){}
        // =
        auto&operator =(const CUISubEventCaller& obj) noexcept { 
            m_pCallback = obj.m_pCallback; m_pRecver =obj.m_pRecver; return *this;
        }
        // call
        bool operator()(UIControl* sender, SubEvent subevent) noexcept;
        // move ctor
        explicit CUISubEventCaller(CUISubEventCaller&& obj) noexcept = delete;
        // bool
        operator bool() noexcept { return !!m_pCallback; }
        // not
        bool operator !() noexcept { return !m_pCallback; }
    protected:
        // callback
        SubEventCallBack    m_pCallback;
        // recver
        UIControl*          m_pRecver;
    };
    // Device Independent Meta
    struct DeviceIndependentMeta {
        // source rect
        D2D1_RECT_F         src_rect;
        // index for bitmap, 0 for custom
        uint32_t            bitmap_index;
        // render rule
        BitmapRenderRule    rule;
        //  interpolation mode
        uint16_t            interpolation;
    };
    // Meta(Bitmap Element)
    struct Meta : DeviceIndependentMeta {
        // bitmap
        ID2D1Bitmap1*       bitmap;
    };
    // render the meta
    void __fastcall Meta_Render(
        const Meta&, LongUIRenderTarget*,
        const D2D1_RECT_F& des_rect, float opacity = 1.f
        ) noexcept;
#ifdef _MSC_VER
#pragma warning(disable: 4200)
#endif
    // MetaEx: store a group of metas, like gif, must be stored as pointer(or ref)
    struct MetaEx {
        // unit
        struct Unit { Meta meta; float delta_time; };
        // the length of group
        size_t      length;
        // group of it("0" in C99, "1" for other)
        Unit        group[0];
    };
    // the timer of ui in frame
    class CUITimer {
    public:
#if 0
        // QueryPerformanceCounter
        static inline auto QueryPerformanceCounter(LARGE_INTEGER* ll) noexcept {
            auto old = ::SetThreadAffinityMask(::GetCurrentThread(), 0);
            auto r = ::QueryPerformanceCounter(ll);
            ::SetThreadAffinityMask(::GetCurrentThread(), old);
            return r;
        }
        // refresh the frequency
        auto LongUIInline  RefreshFrequency() noexcept { ::QueryPerformanceFrequency(&m_cpuFrequency); }
        // start timer
        auto LongUIInline  Start() noexcept { CUITimer::QueryPerformanceCounter(&m_cpuCounterStart); }
        // move end var to start var
        auto LongUIInline  MovStartEnd() noexcept { m_cpuCounterStart = m_cpuCounterEnd; }
        // delta time in sec.
        template<typename T>
        auto LongUIInline  Delta_s() noexcept {
            CUITimer::QueryPerformanceCounter(&m_cpuCounterEnd);
            return static_cast<T>(m_cpuCounterEnd.QuadPart - m_cpuCounterStart.QuadPart) / static_cast<T>(m_cpuFrequency.QuadPart);
        }
        // delta time in ms.
        template<typename T>
        auto LongUIInline  Delta_ms() noexcept {
            CUITimer::QueryPerformanceCounter(&m_cpuCounterEnd);
            return static_cast<T>(m_cpuCounterEnd.QuadPart - m_cpuCounterStart.QuadPart)*static_cast<T>(1e3) / static_cast<T>(m_cpuFrequency.QuadPart);
        }
        // delta time in micro sec.
        template<typename T>
        auto LongUIInline Delta_mcs() noexcept {
            CUITimer::QueryPerformanceCounter(&m_cpuCounterEnd);
            return static_cast<T>(m_cpuCounterEnd.QuadPart - m_cpuCounterStart.QuadPart)*static_cast<T>(1e6) / static_cast<T>(m_cpuFrequency.QuadPart);
        }
    private:
        // CPU 当前频率
        LARGE_INTEGER            m_cpuFrequency;
        // CPU 开始计时时间
        LARGE_INTEGER            m_cpuCounterStart;
        // CPU 结束计时时间
        LARGE_INTEGER            m_cpuCounterEnd;
    public:
        // 构造函数
        CUITimer() noexcept { m_cpuCounterStart.QuadPart = 0; m_cpuCounterEnd.QuadPart = 0; RefreshFrequency(); }
#else
        // refresh the frequency
        auto LongUIInline  RefreshFrequency() noexcept { }
        // start timer
        auto LongUIInline  Start() noexcept { m_dwStart = ::timeGetTime(); }
        // move end var to start var
        auto LongUIInline  MovStartEnd() noexcept { m_dwStart = m_dwNow; }
        // delta time in sec.
        template<typename T>
        auto LongUIInline  Delta_s() noexcept {
            m_dwNow = ::timeGetTime();
            return static_cast<T>(m_dwNow - m_dwStart) * static_cast<T>(0.001);
        }
        // delta time in ms.
        template<typename T>
        auto LongUIInline  Delta_ms() noexcept {
            m_dwNow = ::timeGetTime();
            return static_cast<T>(m_dwNow - m_dwStart);
        }
        // delta time in micro sec.
        template<typename T>
        auto LongUIInline Delta_mcs() noexcept {
            m_dwNow = ::timeGetTime();
            return static_cast<T>(m_dwNow - m_dwStart) * static_cast<T>(1000);
        }
    private:
        // 开始时间
        DWORD                   m_dwStart = 0;
        // 当前时间
        DWORD                   m_dwNow = 0;
    public:
        // 构造函数
        CUITimer() noexcept { this->Start(); }
#endif
    };
    // render repeater
    class CUIRenderRepeater {
    public:
        // ctor
        CUIRenderRepeater() noexcept = default;
        // dtor
        ~CUIRenderRepeater() noexcept = default;
    private:

    };
    // get ex-data
#define UIColorEffect_ExData(o) (reinterpret_cast<uint8_t*>(o) + (sizeof(CUIColorEffect)))
    // Color Effect
    class CUIColorEffect : public Helper::ComBase<Helper::QiList<IUnknown>> {
        using Super = Helper::ComBase<Helper::QiList<IUnknown>>;
    public:
        // ctor
        CUIColorEffect(ULONG count) noexcept : Super(count){}
        // dtor
        virtual ~CUIColorEffect() noexcept {}
        // operator @delete
        void  operator delete(void* p, size_t) noexcept { LongUI::SmallFree(p); };
        // userdata
        ULONG               userdata = 0;
        // color of effect
        D2D1_COLOR_F        color;
    public:
        // create a object
        static inline auto Create(uint32_t exdata_size = 0, bool addref = false) {
            auto* obj = reinterpret_cast<CUIColorEffect*>(
                LongUI::SmallAlloc(sizeof(CUIColorEffect) + exdata_size)
                );
            if (obj) {
                obj->CUIColorEffect::CUIColorEffect(addref ? 1 : 0);
            }
            return obj;
        }
    };

    // Fixed Commad Stack, but won't stack overflow, like a queue, you know
    // will release front one
    template<size_t StackSize>
    class FixedCommadStack {
    public:
        // constructor
        FixedCommadStack() noexcept { ::ZeroMemory(data, sizeof(data)); };
        // destructor
        ~FixedCommadStack() noexcept {
            // release commnad in 2-time
            for (auto &i : data) { ::SafeRelease(i); }
        }
        // add command
        LongUINoinline void AddCommand(IUICommand* cmd) noexcept {
            assert(cmd && "bad argument");
            ::SafeRelease(data[now]);
            data[now] = cmd;
            if (++now > StackSize) now = 0;
            if (now == base) {
                ::SafeRelease(data[base]);
                if (++base > StackSize) base = 0;
            }
        }
        // undo, return false if cannot undo
        LongUINoinline bool Undo() noexcept {
            if (now == base) return false;
            now = now ? now - 1 : StackSize;
            data[now]->Undo();
            return true;
        }
        // redo, return false if cannot redo
        LongUINoinline bool Redo() noexcept {
            register auto tmp_now = now == StackSize ? 0 : now + 1;
            if (tmp_now != base && data[now]) {
                data[now]->Redo();
                now = tmp_now;
                return true;
            }
            else {
                return false;
            }
        }
    private:
        // base index of command
        uint32_t            base = 0;
        // now command index
        uint32_t            now = 0;
        // commad data
        IUICommand*         data[StackSize + 1];
    };
    // default size of cmd-stack
    using DefaultCommadStack = FixedCommadStack<LongUIDefaultUnRedoCommandSize>;
    // XXX: Console for App, 
    class CUIConsole {
    public:
        // config
        struct Config {
            // ctor
            Config() noexcept { position_xy = uint32_t(-1); };
            // buffer X
            int32_t         buffer_size_x = -1;
            // buffer Y
            int32_t         buffer_size_y = -1;
            // position XY
            union {
                // U32
                uint32_t    position_xy;
                // I16 * 2
                struct {
                    // X pos
                    int16_t x;
                    // Y pos
                    int16_t y;
                };
            };
            // atribute
            uint32_t        atribute = 0;
            // logger name
            const wchar_t*  logger_name = nullptr;
            // logger name
            const wchar_t*  helper_executable = nullptr;
        };
        // ctor
        CUIConsole() noexcept;
        // ctor
        ~CUIConsole() noexcept;
    public:
        // bool
        operator bool() const noexcept { return m_hConsole != INVALID_HANDLE_VALUE || !m_hConsole; }
        // create a new console foe this app
        long Create(const wchar_t*, Config& config) noexcept;
        // close this
        long Cleanup() noexcept;
        // output the string
        long Output(const wchar_t* str, bool flush, long len = -1) noexcept;
    protected:
        // handle for console
        HANDLE              m_hConsole = INVALID_HANDLE_VALUE;
        // current length
        size_t              m_length = 0;
        // lock
        //CRITICAL_SECTION    m_cs;
        // name for this
        wchar_t             m_name[64];
        // buffer
        wchar_t             m_buffer[LongUIStringBufferLength];
    };
#ifdef LONGUI_WITH_DEFAULT_CONFIG
    // CUIDefaultConfigure, default impl for IUIConfigure
    class CUIDefaultConfigure : public IUIConfigure {
    public:
        // ctor
        CUIDefaultConfigure(CUIManager& manager) noexcept : m_manager(manager) {}
        // dtor
        ~CUIDefaultConfigure() noexcept { }
        // = operator
        auto operator=(const CUIDefaultConfigure&)->CUIDefaultConfigure = delete;
    public:
        // qi
        virtual auto STDMETHODCALLTYPE QueryInterface(const IID& riid, void** ppvObject) noexcept->HRESULT override;
        // add ref count
        virtual auto STDMETHODCALLTYPE AddRef() noexcept ->ULONG override final { return 2; }
        // release
        virtual auto STDMETHODCALLTYPE Release() noexcept ->ULONG override final { return 1; }
    public:
        virtual auto GetTemplateString() noexcept->const char* override { return nullptr; }
        // get locale name of ui(for text)
        virtual auto GetLocaleName(wchar_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept->void override { name[0] = L'\0'; };
        // add all custom controls
        virtual auto AddCustomControl() noexcept->void override {};
        // return true, if use cpu rendering
        virtual auto IsRenderByCPU() noexcept->bool override { return false; }
        // if use gpu render, you should choose a video card, return the index
        virtual auto ChooseAdapter(IDXGIAdapter1* adapters[], const size_t length) noexcept->size_t override;
        // SetSubEventCallBack for custom control
        virtual auto SetSubEventCallBack(LongUI::SubEvent, const CUISubEventCaller&, UIControl*) noexcept -> void override {}
        // if in RichType::Type_Custom, will call this, we don't implement at here
        virtual auto CustomRichType(const FormatTextConfig&, const wchar_t*) noexcept->IDWriteTextLayout* { assert("noimpl"); return nullptr; };
        // show the error string
        virtual auto ShowError(const wchar_t* str_a, const wchar_t* str_b = nullptr) noexcept -> void override;
#ifdef _DEBUG
        // output the debug string
        virtual auto OutputDebugStringW(DebugStringLevel level, const wchar_t* string, bool flush) noexcept -> void override;
    private:
        // create debug console
        void CreateConsole(DebugStringLevel level) noexcept;
#endif
    protected:
        // manager
        CUIManager&             m_manager;
    public:
        // resource xml null-end-string
        const char*             resource = nullptr;
#ifdef _DEBUG
        // debug console
        CUIConsole              consoles[DLEVEL_SIZE];
#endif
    };
#endif
    // render queue helper
    class CUIRenderQueue {
    public:
        // UNIT
        struct UNIT {
            // length of this unit
            size_t      length;
            // main data of unit
            UIControl*  units[LongUIDirtyControlSize];
        };
        // ctor
        CUIRenderQueue(UIWindow* window) noexcept;
        // dtor
        ~CUIRenderQueue() noexcept;
        // reset
        void Reset(uint32_t f) noexcept;
        // ++ operator
        void operator++() noexcept;
        // plan to render
        void PlanToRender(float wait_time, float render_time, UIControl* control) noexcept;
        // get current unit
        auto GetCurrentUnit() const noexcept { return m_pCurrentUnit; }
        // get display frequency
        auto GetDisplayFrequency() const noexcept { return m_wDisplayFrequency; }
    private: // queue zone
        // frequency for display
        uint16_t            m_wDisplayFrequency = 0;
        // time deviation total
        int16_t             m_sTimeDeviation = 0;
        // render start time
        uint32_t            m_dwStartTime = 0;
        // current unit
        UNIT*               m_pCurrentUnit = nullptr;
        // units data
        UNIT*               m_pUnitsDataBegin = nullptr;
        // end of data
        UNIT*               m_pUnitsDataEnd = nullptr;
    private: // easy plan
        // UNIT like
        struct { size_t length; UIControl* window; } m_unitLike;
    };
    // CUIFileLoader
    class CUIFileLoader {
    public:
        // ctor
        CUIFileLoader() noexcept;
        // dtor
        ~CUIFileLoader() noexcept;
        // copy ctor
        CUIFileLoader(const CUIFileLoader&) = delete;
        // move ctor
        CUIFileLoader(CUIFileLoader&&) = delete;
    public:
        // read file, return false if error
        bool ReadFile(WCHAR* file_name) noexcept;
        // get data
        auto GetData() const  noexcept { return m_pData; }
        // get data length
        auto GetLength() const noexcept { return m_cLength; }
    private:
        // point to data
        void*           m_pData = nullptr;
        // data length
        size_t          m_cLength = 0;
        // buffer length
        size_t          m_cLengthReal = 0;
        // unused
        size_t          unused = 0;
    };
}
