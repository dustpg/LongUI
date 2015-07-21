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


#include "UIFileLoader.h"
#include "DataContainer.h"
#include "UIRenderQueue.h"

// the _longui32 后缀
constexpr uint32_t operator"" _longui32(const char* src, size_t len) {
    return len == 2 ?
        static_cast<uint32_t>(src[0]) << (8 * 0) |
        static_cast<uint32_t>(src[1]) << (8 * 1) :
        static_cast<uint32_t>(src[0]) << (8 * 0) |
        static_cast<uint32_t>(src[1]) << (8 * 1) |
        static_cast<uint32_t>(src[2]) << (8 * 2) |
        static_cast<uint32_t>(src[3]) << (8 * 3);
}

#if 0
#define LongUI64(i) static_cast<uint64_t>(src[i]) << (8 * i)
template<size_t length>
constexpr uint64_t LongUIStringToUI64(const char* src) { return static_cast<uint64_t>(*src); }
template<>
constexpr uint64_t LongUIStringToUI64<2>(const char* src) {return  LongUI64(0) | LongUI64(1);}
template<>
constexpr uint64_t LongUIStringToUI64<3>(const char* src) {
    return  LongUI64(0) | LongUI64(1) | LongUI64(2);
}
template<>
constexpr uint64_t LongUIStringToUI64<4>(const char* src) {
    return  LongUI64(0) | LongUI64(1) | LongUI64(2) | LongUI64(3);
}
template<>
constexpr uint64_t LongUIStringToUI64<5>(const char* src) {
    return  LongUI64(0) | LongUI64(1) | LongUI64(2) | LongUI64(3) | LongUI64(4);
}
template<>
constexpr uint64_t LongUIStringToUI64<6>(const char* src) {
    return  LongUI64(0) | LongUI64(1) | LongUI64(2) | 
            LongUI64(3) | LongUI64(4) | LongUI64(5);
}
template<>
constexpr uint64_t LongUIStringToUI64<7>(const char* src) {
    return  LongUI64(0) | LongUI64(1) | LongUI64(2) | LongUI64(3) | 
            LongUI64(4) | LongUI64(5) | LongUI64(6);
}
template<>
constexpr uint64_t LongUIStringToUI64<8>(const char* src) {
    return  LongUI64(0) | LongUI64(1) | LongUI64(2) | LongUI64(3) | 
            LongUI64(4) | LongUI64(5) | LongUI64(6) | LongUI64(7);
}
#undef LongUI64
// the _longui64 后缀
constexpr uint64_t operator"" _longui64(const char* src, size_t len) {
    return len >= 8 ? LongUIStringToUI64<8>(src) : (
            len == 7 ? LongUIStringToUI64<7>(src) : (
                len == 6 ? LongUIStringToUI64<6>(src) : (
                    len == 5 ? LongUIStringToUI64<5>(src) : (
                        len == 4 ? LongUIStringToUI64<4>(src) : LongUIStringToUI64<3>(src)
                    )
                )
            )
        );
}
#endif

#define LongUI_IID_PV_ARGS(pointer) LongUI::GetIID(pointer), reinterpret_cast<void**>(&pointer)
#define LongUI_IID_PV_ARGS_Ex(pointer) LongUI::GetIID(pointer), reinterpret_cast<IUnknown**>(&pointer)

// get IID form typename, __uuidof is just impl in MSC
namespace LongUI {
    // using template specialization  使用模板特化
    template<typename T> LongUIInline const IID& GetIID();
    // get IID from pointer
    template<typename T> LongUIInline const IID& GetIID(T*) { return LongUI::GetIID<T>(); }
}
#include "GetIIDTemplate.h"
// longui
namespace LongUI {
    // create object
    template<class T> auto CreateObject(T& obj) noexcept { new(&obj) T(); }
    // destory object
    template<class T> auto DestoryObject(T& obj) noexcept { obj; obj.~T(); }
    // recreate object
    template<class T> auto RecreateObject(T& obj) noexcept { DestoryObject(obj); CreateObject(obj); }
    // is 2 power?
    static auto Is2Power(size_t i) { return i && !(i& (i - 1)); }
    // pack the color
    auto PackTheColorARGB(D2D1_COLOR_F& IN color) noexcept ->uint32_t LongUINoinline;
    // unpack the color
    auto UnpackTheColorARGB(uint32_t IN color32, D2D1_COLOR_F& OUT color4f) noexcept->void LongUINoinline;
    // data to bool
    template<typename T> bool DataToBool(const T& t) { return !!(t); }
    // Dll Function Helper
    struct Dll {
        // CreateDXGIFactory1@dxgi.dll
        static decltype(&::CreateDXGIFactory1) CreateDXGIFactory1;
        // DWriteCreateFactory@dwrite.dll
        static decltype(&::DWriteCreateFactory) DWriteCreateFactory;
        // D3D11CreateDevice@d3d11.dll
        static decltype(&::D3D11CreateDevice) D3D11CreateDevice;
        // D2D1CreateFactory@d2d1.dll
        static HRESULT (WINAPI* D2D1CreateFactory)(D2D1_FACTORY_TYPE, REFIID, CONST D2D1_FACTORY_OPTIONS *, void **);
        // DCompositionCreateDevice, Win8 and later hold it
        static decltype(&::DCompositionCreateDevice) DCompositionCreateDevice;
    };
    // Dynamic Link Library Control Loader
    class DllControlLoader {
        // constructor
        // DllControlLoader(DllFileName, ControlName [, CreateFuntionNameInDll])
        DllControlLoader(const wchar_t*, const wchar_t*, const char* = "LongUICreateControl") noexcept;
        // destructor
        ~DllControlLoader() noexcept;
        // = operator
        auto operator=(const DllControlLoader&) = delete;
    public:
        // handle to Dynamic Link Library
        HMODULE               const dll = nullptr;
        // function
        CreateControlFunction const function = nullptr;
    };
    // Device Independent Meta
    struct DeviceIndependentMeta {
        // source rect
        D2D1_RECT_F         src_rect;
        // index for bitmap
        uint32_t            bitmap_index;
        // render rule
        BitmapRenderRule    rule;
        //  interpolation mode
        uint16_t            interpolation;
    };
    // Meta (Graphics Element)
    struct alignas(sizeof(void*)) Meta {
        // source rect
        D2D1_RECT_F         src_rect;
        // bitmap
        ID2D1Bitmap1*       bitmap;
        // render rule
        BitmapRenderRule    rule;
        //  interpolation mode
        uint16_t            interpolation;
    };
    // render the meta
    void __fastcall Meta_Render(
        const Meta&, LongUIRenderTarget*, 
        const D2D1_RECT_F& des_rect, float opacity = 1.f
        ) noexcept;
#ifdef _MSC_VER
#pragma warning(disable: 4200)
#endif
    // MetaEx: store a group of metas, like gif, MUST stored as pointer(or ref)
    struct MetaEx {
        // unit
        struct Unit { Meta meta; float delta_time; };
        // the length of group
        size_t      length;
        // group of it("0" in C99, "1" for other)
        Unit        group[0];
    };
    // Bit Array 计算机中每一字节都很宝贵
    template<typename T>
    class BitArray {
    public:
        // bit wide
        static constexpr size_t LENGTH = sizeof(T) * 8;
        // ctor
        BitArray() noexcept {};
        // dtor
        ~BitArray() noexcept {};
        // is true or fasle
        auto Test(uint32_t index) noexcept { return !!(m_data & (1 << index)); }
        // set to true
        auto SetTrue(uint32_t index) noexcept { m_data |= (1 << index); };
        // set to false
        auto SetFalse(uint32_t index) noexcept { m_data &= ~(1 << index); };
        // set to NOT
        auto SetNot(uint32_t index) noexcept { m_data ^= (1 << index); };
    private:
        // data for bit-array
        T           m_data = T(0);
    };
    // 特例化
    using BitArray16 = BitArray<uint16_t>;
    using BitArray32 = BitArray<uint32_t>;
    using BitArray64 = BitArray<uint64_t>;
    // data 放肆!450交了么!
    constexpr size_t  INFOPDATA12_ZONE =  (size_t(3));
    constexpr size_t  INFOPOINTER_ZONE = ~INFOPDATA12_ZONE;
    constexpr size_t  INFOPTDATA1_ZONE = ~(size_t(2));
    constexpr size_t  INFOPTDATA2_ZONE = ~(size_t(1));
    // Infomation-ed pointer  计算机中每一字节都很宝贵
    template<typename T>
    class InfomationPointer {
    public:
        // constructor
        InfomationPointer(T* pointer) :data(reinterpret_cast<size_t>(pointer)) { assert(!(data&INFOPDATA12_ZONE)); }
        // copy constructor
        InfomationPointer(const InfomationPointer&) = delete;
        // move constructor
        InfomationPointer(InfomationPointer&&) = delete;
        // operator =
        T* operator=(T* pt) { assert(!(data&INFOPDATA12_ZONE)); data = reinterpret_cast<size_t>(pt) | (data&INFOPDATA12_ZONE); return pt; }
        // operator ->
        T* operator->() noexcept { return reinterpret_cast<T*>(data & INFOPOINTER_ZONE);}
        // operator T*
        operator T*() const noexcept { return reinterpret_cast<T*>(data & INFOPOINTER_ZONE); }
        // operator []
        T& operator [](const int index) noexcept { return (reinterpret_cast<T*>(data & INFOPOINTER_ZONE))[index]; }
        // operator [] const ver.
        const T& operator [](const int index) const noexcept { return (reinterpret_cast<T*>(data & INFOPOINTER_ZONE))[index]; }
        // pointer
        T* Ptr() const noexcept { return reinterpret_cast<T*>(data & INFOPOINTER_ZONE); }
        // bool1
        bool Bool1() const noexcept { return (data & (1 << 0)) > 0; }
        // bool2
        bool Bool2() const noexcept { return (data & (1 << 1)) > 0; }
        // set bool1
        void SetBool1(bool b) noexcept { data = (data & INFOPTDATA2_ZONE) | size_t(b); }
        // bool2
        void SetBool2(bool b) noexcept { data = (data & INFOPTDATA1_ZONE) | (size_t(b) << 1); }
        // SafeRelease if keep a Relase() interface(COM like)
        void SafeRelease() noexcept { T* t = reinterpret_cast<T*>(data & INFOPOINTER_ZONE); if (t) { t->Release(); data &= INFOPDATA12_ZONE; } }
    private:
        // pointer&boolx2 data
#ifdef _DEBUG
        union{
            size_t          data;
            T*              pointer__;
        };
#else
        size_t          data;
#endif
    };
    // the timer of ui in frame
    class UITimer {
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
        auto LongUIInline  Start() noexcept { UITimer::QueryPerformanceCounter(&m_cpuCounterStart); }
        // move end var to start var
        auto LongUIInline  MovStartEnd() noexcept { m_cpuCounterStart = m_cpuCounterEnd; }
        // delta time in sec.
        template<typename T>
        auto LongUIInline  Delta_s() noexcept {
            UITimer::QueryPerformanceCounter(&m_cpuCounterEnd);
            return static_cast<T>(m_cpuCounterEnd.QuadPart - m_cpuCounterStart.QuadPart) / static_cast<T>(m_cpuFrequency.QuadPart);
        }
        // delta time in ms.
        template<typename T>
        auto LongUIInline  Delta_ms() noexcept {
            UITimer::QueryPerformanceCounter(&m_cpuCounterEnd);
            return static_cast<T>(m_cpuCounterEnd.QuadPart - m_cpuCounterStart.QuadPart)*static_cast<T>(1e3) / static_cast<T>(m_cpuFrequency.QuadPart);
        }
        // delta time in micro sec.
        template<typename T>
        auto LongUIInline Delta_mcs() noexcept {
            UITimer::QueryPerformanceCounter(&m_cpuCounterEnd);
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
        UITimer() noexcept { m_cpuCounterStart.QuadPart = 0; m_cpuCounterEnd.QuadPart = 0; RefreshFrequency(); }
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
        UITimer() noexcept { this->Start(); }
#endif
    };
    // Meta (Graphics Element)
    using GraphicsElement = Meta;
    // std::atoi diy version
    auto __fastcall AtoI(const char* __restrict) -> int;
    // std::atof diy version(float ver)
    auto __fastcall AtoF(const char* __restrict) -> float;
    // LongUI::AtoI diy version(double ver)
    //auto __fastcall AtoLF(const char*) -> double;
    // UTF-8 UTF-16 UTF-32
    static_assert(sizeof(wchar_t) == sizeof(char16_t), "change UTF-16 to UTF-32");
#if 1
#define UTF8toWideChar(a, b) UTF8toUTF16((a), reinterpret_cast<char16_t*>(b))
#define WideChartoUTF8(a, b) UTF16toUTF8(reinterpret_cast<const char16_t*>(a), (b))
#else
#define UTF8toWideChar(a, b) UTF8toUTF32((a), reinterpret_cast<char32_t*>(b))
#define WideChartoUTF8(a, b) UTF32toUTF8(reinterpret_cast<const char32_t*>(a), (b))
#endif
    // Base64 DataChar: Map 0~63 to visible char
    static const char Base64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    // Base64 DataChar: Map visible char to 0~63
    static const uint8_t Base64Datas[128] = {
        // [  0, 16)
        0, 0, 0, 0,   0, 0 ,0, 0,       0, 0, 0, 0, 0, 0, 0, 0,
        // [ 16, 32)
        0, 0, 0, 0,   0, 0 ,0, 0,       0, 0, 0, 0, 0, 0, 0, 0,
        // [ 32, 48)                            43 44 45 46 47
        0, 0, 0, 0,   0, 0 ,0, 0,       0, 0, 0,62, 0, 0, 0,64,
        // [ 48, 64)
        52,53,54,55, 56,57,58,59,      60,61, 0, 0, 0, 0, 0, 0,
        // [ 64, 80)
        0, 0, 1, 2,   3, 4, 5, 6,       7, 8, 9,10,11,12,13,14,
        // [ 80, 96)
        15,16,17,18, 19,20,21,22,      23,24,25, 0, 0, 0, 0, 0,
        // [ 96,112)
        0,26,27,28,  29,30,31,32,      33,34,35,36,37,38,39,40,
        // [112,128)
        41,42,43,44, 45,46,47,48,      49,50,51, 0, 0, 0, 0, 0,
    };
    // Base64 Encode 编码
    auto __fastcall Base64Encode(IN const uint8_t* __restrict bindata, IN size_t binlen, OUT char* const __restrict base64) noexcept -> char *;
    // Base64 Decode 解码
    auto __fastcall Base64Decode(IN const char* __restrict  base64, OUT uint8_t* __restrict bindata) noexcept->size_t;
    // UTF-16 to UTF-8
    // Return: UTF-8 string length, 0 maybe error
    auto __fastcall UTF16toUTF8(const char16_t* __restrict, char* __restrict) noexcept -> uint32_t;
    // UTF-8 to UTF-16
    // Return: UTF-16 string length, 0 maybe error
    auto __fastcall UTF8toUTF16(const char* __restrict, char16_t* __restrict) noexcept -> uint32_t;
    // ------------------- Windows COM Interface Helper
#define LONGUICOMMETHOD virtual HRESULT STDMETHODCALLTYPE
#define LONGUIVTABLE(pointer)  (*reinterpret_cast<const size_t * const>(pointer))
#define LONGUISAMEVT(a, b) (LONGUIVTABLE(a) == LONGUIVTABLE(b))
    // counter based COM Interface, 0 , wiil be deleted
    template <typename InterfaceChain, typename CounterType = std::atomic<ULONG>>
    class ComBase : public InterfaceChain {
    public:
        // constructor inline ver.
        explicit ComBase() noexcept  { }
        // IUnknown interface
        LONGUICOMMETHOD QueryInterface(IID const& iid, OUT void** ppObject) noexcept final override {
            *ppObject = nullptr;
            InterfaceChain::QueryInterfaceInternal(iid, ppObject);
            if (*ppObject == nullptr)
                return E_NOINTERFACE;
            this->ComBase::AddRef();
            return S_OK;
        }
        // add ref-counter
        ULONG STDMETHODCALLTYPE AddRef() noexcept final override { return ++m_refValue; }
        // delete when 0
        ULONG STDMETHODCALLTYPE Release() noexcept final override {
            ULONG newCount = --m_refValue;
            if (newCount == 0)  delete this;
            return newCount;
        }
        // virtual destructor
        virtual ~ComBase() noexcept { }
    protected:
        // the counter 
        CounterType     m_refValue = 0;
    public:
        // No copy construction allowed.
        ComBase(const ComBase& b) = delete;
        ComBase& operator=(ComBase const&) = delete;
    };
    // None Counter COM(Static)
    template <typename InterfaceChain>
    class ComStatic : public InterfaceChain {
    public:
        // constructor inline ver.
        explicit ComStatic() noexcept { }
        // IUnknown interface
        LONGUICOMMETHOD QueryInterface(IID const& iid, OUT void** ppObject) noexcept final override {
            *ppObject = nullptr;
            InterfaceChain::QueryInterfaceInternal(iid, ppObject);
            if (*ppObject == nullptr)
                return E_NOINTERFACE;
            AddRef();
            return S_OK;
        }
        // allways return 2
        virtual ULONG STDMETHODCALLTYPE AddRef() noexcept final override { return  2; }
        // allways return 1
        virtual ULONG STDMETHODCALLTYPE Release() noexcept final override { return 1; }
    public:
        // No copy construction allowed.
        ComStatic(const ComStatic& b) = delete;
        ComStatic& operator=(ComStatic const&) = delete;
    };
    // None
    struct QiListNil { };
    // When the QueryInterface list refers to itself as class,
    // which hasn't fully been defined yet.
    template <typename InterfaceName, typename InterfaceChain>
    class QiListSelf : public InterfaceChain {
    public:
        inline void QueryInterfaceInternal(IID const& iid, OUT void** ppObject) noexcept {
            if (iid != LongUI::GetIID<InterfaceName>())
                return InterfaceChain::QueryInterfaceInternal(iid, ppObject);
            *ppObject = static_cast<InterfaceName*>(this);
        }
    };
    // When this interface is implemented and more follow.
    template <typename InterfaceName, typename InterfaceChain = QiListNil>
    class QiList : public InterfaceName, public InterfaceChain {
    public:
        inline void QueryInterfaceInternal(IID const& iid, OUT void** ppObject) noexcept {
            if (iid != LongUI::GetIID<InterfaceName>())
                return InterfaceChain::QueryInterfaceInternal(iid, ppObject);

            *ppObject = static_cast<InterfaceName*>(this);
        }
    };
    // When the this is the last implemented interface in the list.
    template <typename InterfaceName>
    class QiList<InterfaceName, QiListNil> : public InterfaceName {
    public:
        inline void QueryInterfaceInternal(IID const& iid, OUT void** ppObject) noexcept {
            if (iid != LongUI::GetIID<InterfaceName>()) return;
            *ppObject = static_cast<InterfaceName*>(this);
        }
    };
    // 
#define UIColorEffect_ExData(o) (reinterpret_cast<uint8_t*>(o) + (sizeof(UIColorEffect)))
    // Color Effect
    class UIColorEffect : public ComBase<QiList<IUnknown>> {
    public:
        // c
        UIColorEffect() noexcept {}
        // operator @delete
        void  operator delete(void* p, size_t) noexcept { LongUI::SmallFree(p); };
        // userdata
        ULONG               userdata = 0;
        // color of effect
        D2D1_COLOR_F        color;
    public:
        // create a object
        static inline auto Create(uint32_t exdata_size=0, bool addref=false) { 
            auto* obj = reinterpret_cast<UIColorEffect*>(
                LongUI::SmallAlloc(sizeof(UIColorEffect) + exdata_size)
                ); 
            if (obj) { 
                obj->UIColorEffect::UIColorEffect();
                if (addref) obj->AddRef();
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
    // XML Resource Loader
    class UIXMLResourceLoader : public IUIResourceLoader {
    public:
        // xml doc
        pugi::xml_document          m_xmlDoc;
    };
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
        long Create(const wchar_t* , Config& config) noexcept;
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
        ~CUIDefaultConfigure() noexcept;
        // = operator
        auto operator=(const CUIDefaultConfigure&) ->CUIDefaultConfigure  = delete;
    public:
        // qi
        virtual auto STDMETHODCALLTYPE QueryInterface(const IID& riid,void** ppvObject) noexcept ->HRESULT override;
        // add ref count
        virtual auto STDMETHODCALLTYPE AddRef() noexcept ->ULONG override final { return 2; }
        // release
        virtual auto STDMETHODCALLTYPE Release() noexcept ->ULONG override final { return 1; }
    public:
        // get template string for control
        virtual auto GetTemplateString() noexcept->const char* override  { return ctemplate; }
        // get locale name of ui(for text)
        virtual auto GetLocaleName(wchar_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept->void override { name[0] = L'\0'; };
        // add all custom controls
        virtual auto AddCustomControl() noexcept->void override {};
        // return true, if using cpu rendering
        virtual auto IsRenderByCPU() noexcept->bool override { return false; }
        // if using gpu render, you should choose a video card, return the index
        virtual auto ChooseAdapter(IDXGIAdapter1* adapters[], size_t const length) noexcept->size_t override;
        // SetEventCallBack for custom control
        virtual auto SetEventCallBack(LongUI::Event, LongUIEventCallBack, UIControl*, UIControl*) noexcept -> void override {}
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
        // resource loader
        IUIResourceLoader*      m_pXMLResourceLoader = nullptr;
    public:
        // resource xml null-end-string
        const char*             resource = nullptr;
        // control template xml null-end-string
        const char*             ctemplate = nullptr;
        // script
        IUIScript*              script = nullptr;
        // inline param handler
        InlineParamHandler      handler = nullptr;
#ifdef _DEBUG
        // debug console
        CUIConsole              consoles[DLEVEL_SIZE];
#endif
    };
#endif
    // RtlGetVersion func
    typedef NTSTATUS(NTAPI* fnRtlGetVersion)(PRTL_OSVERSIONINFOW lpVersionInformation);
    // IsWindowsVersionOrGreater
    LongUINoinline bool IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor) noexcept;
    // >= WinXP
    static inline auto IsWindowsXPOrGreater() noexcept {
        return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 0);
    }
    // >= WinXP SP1
    static inline auto IsWindowsXPSP1OrGreater() noexcept {
        return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 1);
    }
    // >= WinXP SP2
    static inline auto IsWindowsXPSP2OrGreater() noexcept {
        return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 2);
    }
    // >= WinXP SP3
    static inline auto IsWindowsXPSP3OrGreater() noexcept {
        return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 3);
    }
    // >= Win Vista
    static inline auto IsWindowsVistaOrGreater() noexcept {
        return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 0);
    }
    // >= Win Vista SP1
    static inline auto IsWindowsVistaSP1OrGreater() noexcept {
        return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 1);
    }
    // >= Win Vista SP2
    static inline auto IsWindowsVistaSP2OrGreater() noexcept {
        return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 2);
    }
    // >= Win7
    static inline auto IsWindows7OrGreater() noexcept {
        return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN7), LOBYTE(_WIN32_WINNT_WIN7), 0);
    }
    // >= Win7 SP1
    static inline auto IsWindows7SP1OrGreater() noexcept {
        return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN7), LOBYTE(_WIN32_WINNT_WIN7), 1);
    }
    // >= Win8
    static inline auto IsWindows8OrGreater() noexcept {
        return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN8), LOBYTE(_WIN32_WINNT_WIN8), 0);
    }
    // >= Win8.1
    static inline auto IsWindows8Point1OrGreater() noexcept {
        return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINBLUE), LOBYTE(_WIN32_WINNT_WINBLUE), 0);
    }
    // >= Win10
    static inline auto IsWindows10OrGreater() noexcept {
        return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN10), LOBYTE(_WIN32_WINNT_WIN10), 0);
    }
    // Server?
    static inline auto IsWindowsServer() noexcept {
        OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0,{ 0 }, 0, 0, 0, VER_NT_WORKSTATION };
        DWORDLONG        const dwlConditionMask = VerSetConditionMask(0, VER_PRODUCT_TYPE, VER_EQUAL);
        return !VerifyVersionInfoW(&osvi, VER_PRODUCT_TYPE, dwlConditionMask);
    }
}