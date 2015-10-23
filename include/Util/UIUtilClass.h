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
#define LONGUI_FUNCTION_NOEXCEPT noexcept
    // small single object
    struct CUISingleObject {
        // throw new []
        auto operator new(size_t size) ->void* = delete;
        // throw new []
        auto operator new[](size_t size) ->void* = delete;
        // delete []
        void operator delete[](void*, size_t size) noexcept = delete;
    };
    // small single object
    struct CUISingleNormalObject : CUISingleObject {
        // nothrow new 
        auto operator new(size_t size, const std::nothrow_t&) noexcept ->void*{ return LongUI::NormalAlloc(size); };
        // nothrow delete 
        auto operator delete(void* address, const std::nothrow_t&) ->void { return LongUI::NormalFree(address); }
        // delete
        auto operator delete(void* address) noexcept ->void { return LongUI::NormalFree(address); }
    };
    // small single object
    struct CUISingleSmallObject : CUISingleObject {
        // nothrow new 
        auto operator new(size_t size, const std::nothrow_t&) noexcept ->void*{ return LongUI::SmallAlloc(size); };
        // delete
        auto operator delete(void* address) noexcept ->void { return LongUI::SmallFree(address); }
    };
    // BaseFunc
    template<typename Result, typename ...Args>
    class XUIBaseFunc : public CUISingleSmallObject {
    public:
        // call
        virtual auto Call(Args... args) LONGUI_FUNCTION_NOEXCEPT ->Result = 0;
        // dtor
        virtual ~XUIBaseFunc() noexcept { if (this->chain) delete this->chain; this->chain = nullptr; };
        // call chain
        XUIBaseFunc*        chain = nullptr;
    };
    // RealFunc
    template<typename Func, typename Result, typename ...Args>
    class CUIRealFunc final : public XUIBaseFunc<Result, Args...> {
        // func data
        Func                m_func;
    public:
        // ctor
        CUIRealFunc(const Func &x) noexcept : m_func(x) {}
        // call
        auto Call(Args... args) LONGUI_FUNCTION_NOEXCEPT ->Result override { 
            if (this->chain) this->chain->Call(args...);
            return m_func(args...);
        }
        // dtor
        virtual ~CUIRealFunc() noexcept = default;
    };
    // type helper
    template<typename Func> struct type_helper { 
        using type = Func; 
    };
    // type helper
    template<typename Result, typename ...Args> struct type_helper<Result(Args...)> { 
        using type = Result (*)(Args...);
    };
    // UI Function, lightweight and chain-call-able version std::function
    template<typename Result, typename ...Args>
    class CUIFunction<Result(Args...)> {
        // this type
        using MyType = CUIFunction<Result(Args...)>;
        // RealFunc pointer
        XUIBaseFunc<Result, Args...>*   m_pFunction = nullptr;
        // release
        void release() noexcept { if (m_pFunction) delete m_pFunction; m_pFunction = nullptr; }
    public:
        // Ok
        auto IsOK() const noexcept { return !!m_pFunction; }
        // dtor
        ~CUIFunction() noexcept { this->release(); }
        // ctor
        CUIFunction() noexcept = default;
        // move ctor
        CUIFunction(MyType&& obj) noexcept : m_pFunction(obj.m_pFunction) { obj.m_pFunction = nullptr; };
        // no copy ctor
        CUIFunction(const MyType&) = delete;
        // and call chain
        auto AddCallChain(MyType&& chain) { 
            if (chain.IsOK()) {
                chain.m_pFunction->chain = m_pFunction;
                m_pFunction = chain.m_pFunction;
                chain.m_pFunction = nullptr;
            }
            else {
                assert(!"error");
            }
        }
        // and call chain
        auto& operator += (MyType&& chain) { this->AddCallChain(std::move(chain)); return *this; }
        // and call chain
        template<typename Func> 
        auto& operator += (const Func &x) { this->AddCallChain(std::move(CUIFunction(x))); return *this; }
        // opeator =
        template<typename Func> auto& operator=(const Func &x) noexcept {
            this->release();
            m_pFunction = new(std::nothrow) CUIRealFunc<type_helper<Func>::type, Result, Args...>(x);
            return *this;
        }
        // opeator =
        MyType& operator=(const MyType &x) noexcept = delete;
        // ctor with func
        template<typename Func> CUIFunction(const Func& f) noexcept : 
        m_pFunction(new(std::nothrow) CUIRealFunc<type_helper<Func>::type, Result, Args...>(f))  {}
        // () operator
        auto operator()(Args... args) const LONGUI_FUNCTION_NOEXCEPT { assert(m_pFunction && "bad call or oom"); return m_pFunction ? m_pFunction->Call(args...) : Result(); }
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
        const Meta&, ID2D1DeviceContext*,
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
        // super class
        using Super = Helper::ComBase<Helper::QiList<IUnknown>>;
    public:
        // ctor
        CUIColorEffect(ULONG count) noexcept : Super(count){}
        // dtor
        virtual ~CUIColorEffect() noexcept {}
        // operator @delete
        void  operator delete(void* p, size_t) noexcept { LongUI::SmallFree(p); };
        // color of effect
        D2D1_COLOR_F        color;
    public:
        // create a object
        static inline auto Create(D2D1_COLOR_F& color, bool addref = true) {
            auto* obj = reinterpret_cast<CUIColorEffect*>(
                LongUI::SmallAlloc(sizeof(CUIColorEffect))
                );
            if (obj) {
                obj->CUIColorEffect::CUIColorEffect(addref ? 1 : 0);
                obj->color = color;
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
            for (auto &i : data) { LongUI::SafeRelease(i); }
        }
        // add command
        LongUINoinline void AddCommand(IUICommand* cmd) noexcept {
            assert(cmd && "bad argument");
            LongUI::SafeRelease(data[now]);
            data[now] = cmd;
            if (++now > StackSize) now = 0;
            if (now == base) {
                LongUI::SafeRelease(data[base]);
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
        long Close() noexcept;
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
        CUIDefaultConfigure(CUIManager& manager, const char* log_file=nullptr) noexcept : m_manager(manager) {
#ifdef _DEBUG
            if (log_file) {
                m_pLogFile = std::fopen(log_file, "ab");
            }
#else
            UNREFERENCED_PARAMETER(log_file);
#endif
        }
        // dtor
        ~CUIDefaultConfigure() noexcept {
#ifdef _DEBUG
            if (m_pLogFile) {
                ::fclose(m_pLogFile);
                m_pLogFile = nullptr;
            }
#endif
        }
        // = operator
        auto operator=(const CUIDefaultConfigure&)->CUIDefaultConfigure = delete;
    public:
        // add ref count
        virtual auto STDMETHODCALLTYPE AddRef() noexcept ->ULONG override final { return 2; }
        // release
        virtual auto STDMETHODCALLTYPE Release() noexcept ->ULONG override final { return 1; }
    public:
        // get flags for configure
        virtual auto GetConfigureFlag() noexcept->ConfigureFlag override { return IUIConfigure::Flag_OutputDebugString; }
        // create interface
        virtual auto CreateInterface(const IID& iid, void** obj)noexcept->HRESULT override;
        // get null-end string for template for creating control
        virtual auto GetTemplateString() noexcept->const char* override { return nullptr; }
        // get locale name of ui(for text)
        virtual auto GetLocaleName(wchar_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept->void override { name[0] = L'\0'; };
        // add all custom controls
        virtual auto RegisterSome() noexcept->void override {};
        // if use gpu render, you should choose a video card, return the index
        virtual auto ChooseAdapter(const DXGI_ADAPTER_DESC1 adapters[], const size_t length) noexcept->size_t override;
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
#ifdef _DEBUG
    private:
        // time tick
        size_t                  m_timeTick = 0;
        // log file string
        FILE*                   m_pLogFile = nullptr;
#endif
    public:
        // resource xml null-end-string
        const char*             resource = nullptr;
        // log file name in wchar_t*
        const wchar_t*          log_file_name = nullptr;
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
    // short allocator, memory created with allocator, destroyed with allocator
    template<size_t CHAIN_SIZE=2048>
    class CUIShortStringAllocator {
        // CHAIN
        struct CHAIN { CHAIN* next; size_t used; char buffer[0]; };
        // buffer length
        enum : size_t { BUFFER_SIZE = CHAIN_SIZE - sizeof(void*) * 2 };
        // memory chain
        CHAIN*              m_pHeader = nullptr;
    public:
        // ctor
        CUIShortStringAllocator() noexcept = default;
        // cpoy ctor
        CUIShortStringAllocator(const CUIShortStringAllocator<CHAIN_SIZE>&) noexcept = default;
        // dtor
        ~CUIShortStringAllocator() noexcept {
            auto node = m_pHeader;
            while (node) {
                auto tmp = node;
                node = node->next;
                LongUI::NormalFree(tmp);
            }
            m_pHeader = nullptr;
        }
    public:
        // free
        auto Free(void* address) noexcept { UNREFERENCED_PARAMETER(address); }
        // alloc for normal buffer
        auto Alloc(size_t len) noexcept { return this->alloc(len); }
        // alloc string with length
        template<typename T>
        const T* Alloc(const T* str, size_t len) noexcept {
            assert(str);
            // align operation
            auto buffer = reinterpret_cast<T*>(this->alloc(sizeof(T)*(len + 1)));
            if (!buffer) return nullptr;
            ::memcpy(buffer, str, sizeof(T) * len);
            buffer[len] = 0;
            return buffer;
        }
        // alloc string
        auto CopyString(const char* str) noexcept { return this->Alloc(str, std::strlen(str)); }
        // alloc w-string
        auto CopyString(const wchar_t* str) noexcept { return this->Alloc(str, std::wcslen(str)); }
    private:
        // reserve
        auto reserve(size_t len) noexcept ->CHAIN* {
            assert(len < BUFFER_SIZE && "out of range");
            // check
            if (!m_pHeader || (m_pHeader->used + len) > BUFFER_SIZE) {
                auto new_header = reinterpret_cast<CHAIN*>(LongUI::NormalAlloc(CHAIN_SIZE));
                if (!new_header) return nullptr;
                new_header->next = m_pHeader;
                new_header->used = 0;
                m_pHeader = new_header;
            }
            return m_pHeader;
        }
        // alloc buffer
        LongUINoinline auto alloc(size_t len) noexcept ->void* {
#ifdef _DEBUG
            if (len >= BUFFER_SIZE) {
                UIManager << DL_Error 
                    << "cannot alloc this size("
                    << long(len)
                    << L')'
                    << LongUI::endl;
                assert(!"bad action");
            }
            if (len > (BUFFER_SIZE / 10)) {
                UIManager << DL_Warning << "To large for 'short'" << LongUI::endl;
            }
#endif
            if (len >= BUFFER_SIZE) {
                return nullptr;
            }
            void* address = nullptr;
            auto chian = this->reserve(len);
            if (chian) {
                address = chian->buffer + chian->used;
                chian->used += len;
            }
            return address;
        };
    };

    // singleton
    /*template<typename T>
    struct CUISingleton {
        // ptr
        auto Ptr() noexcept { return reinterpret_cast<T*>(this->buffer); }
        // create
        template<typename ...Args> auto Create(Args... args) {
#ifdef _DEBUG
            assert(state == 0 && "create this after destory!");
            state++;
#endif
            return new(this->buffer) T(args);
        }
        // destory
        auto Destory() noexcept {
#ifdef _DEBUG
            assert(state != 0 && "destory this after create!");
            --state;
#endif
            this->Ptr()->~T();
        }
        // buffer for  singleton
        alignas(T)  char    buffer[sizeof(T)];
#ifdef _DEBUG
        // state
        size_t              state = 0;
#endif
        // singleton
        static CGMSingleton<T>      s_instance;
    };*/
}
