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

#define LongUI_IID_PV_ARGS(pointer) LongUI::GetIID(pointer), reinterpret_cast<void**>(&pointer)
#define LongUI_IID_PV_ARGS_Ex(pointer) LongUI::GetIID(pointer), reinterpret_cast<IUnknown**>(&pointer)
#define MakeGetIID(c) template<> LongUIInline const IID& GetIID<c>() { return IID_##c; }
#define MakeLongUIGetIID(c) template<> LongUIInline const IID& GetIID<LongUI::c>() { return LongUI::IID_##c; }

// get IID form type, __uuidof is just impl in MSC
namespace LongUI {
    // windows error code to HRESULT
    inline auto WinCode2HRESULT(UINT x) noexcept ->HRESULT {
        return ((HRESULT)(x) <= 0 ? ((HRESULT)(x)) : ((HRESULT)(((x)& 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000)));
    }
    // lengthof
    template<typename T> constexpr auto lengthof(T& t) { UNREFERENCED_PARAMETER(t); return sizeof(t) / sizeof(*t); }
    // white space
    template<typename T> static auto white_space(T c) noexcept { return ((c) == ' ' || (c) == '\t'); }
    // valid digit
    template<typename T> static auto valid_digit(T c) noexcept { return ((c) >= '0' && (c) <= '9'); }
    // byte distance
    template<typename T, typename Y> auto bdistance(T* a, T* b) noexcept { reinterpret_cast<const char*>(b) - reinterpret_cast<const char*>(a); };
    // busy waiting in micro seconds
    void usleep(long usec) noexcept;
    // hex -> int
    unsigned int __fastcall Hex2Int(char c) noexcept;
    // Render Common Brush
    void FillRectWithCommonBrush(ID2D1RenderTarget* target, ID2D1Brush* brush, const D2D1_RECT_F& rect) noexcept;
    // using template specialization
    template<typename T> LongUIInline const IID& GetIID();
    // get IID from pointer
    template<typename T> LongUIInline const IID& GetIID(T*) { return LongUI::GetIID<T>(); }
    // BKDR Hash
    auto __fastcall BKDRHash(const char* str) noexcept->uint32_t;
    // BKDR Hash
    auto __fastcall BKDRHash(const wchar_t* str) noexcept->uint32_t;
    // BKDR Hash
    static inline auto BKDRHash(const char* str, uint32_t size) noexcept { return BKDRHash(str) % size; }
    // BKDR Hash
    static inline auto BKDRHash(const wchar_t* str, uint32_t size) noexcept { return BKDRHash(str) % size; }
    // load proc
    template<typename T> static inline auto LoadProc(T& pointer, HMODULE dll, const char* name) noexcept {
        pointer = reinterpret_cast<T>(::GetProcAddress(dll, name));
    }
    // create object
    template<class T> auto CreateObject(T& obj) noexcept { new(&obj) T(); }
    // destory object
    template<class T> auto DestoryObject(T& obj) noexcept { obj; obj.~T(); }
    // recreate object
    template<class T> auto RecreateObject(T& obj) noexcept { DestoryObject(obj); CreateObject(obj); }
    // is 2 power?
    static constexpr auto Is2Power(const size_t i) noexcept { return i && !(i& (i - 1)); }
    // point in rect?
    inline static auto IsPointInRect(const D2D1_RECT_F& rect, const D2D1_POINT_2F& pt) noexcept {
        return(pt.x >= rect.left && pt.y >= rect.top && pt.x < rect.right && pt.y < rect.bottom);
    }
    // point in rect? overload for RectLTWH_F
    inline static auto IsPointInRect(const RectLTWH_F& rect, const D2D1_POINT_2F& pt) noexcept {
        return(pt.x >= rect.left && pt.y >= rect.top && pt.x < rect.left + rect.width && pt.y < rect.top + rect.height);
    }
    // get transformed pointer
    inline static auto TransformPoint(const D2D1_MATRIX_3X2_F& matrix, const D2D1_POINT_2F& point) noexcept {
        D2D1_POINT_2F result = {
            point.x * matrix._11 + point.y * matrix._21 + matrix._31,
            point.x * matrix._12 + point.y * matrix._22 + matrix._32
        };
        return result;
    }
    // get transformed pointer
    auto TransformPointInverse(const D2D1_MATRIX_3X2_F& matrix, const D2D1_POINT_2F& point) noexcept->D2D1_POINT_2F;
    // round
    static inline auto RoundToInt(float x) { return static_cast<int>(x + .5f); }
    // pack the color
    auto __fastcall PackTheColorARGB(D2D1_COLOR_F& IN color) noexcept ->uint32_t LongUINoinline;
    // unpack the color
    auto __fastcall UnpackTheColorARGB(uint32_t IN color32, D2D1_COLOR_F& OUT color4f) noexcept->void LongUINoinline;
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
        static HRESULT (STDAPICALLTYPE* DCompositionCreateDevice)(IDXGIDevice*, REFIID, void **);
    };
    // std::atoi diy version
    auto AtoI(const char* __restrict) noexcept -> int;
    // std::atoi diy version overload for wchar_t
    auto AtoI(const wchar_t* __restrict) noexcept -> int;
    // std::atof diy version(float ver)
    auto AtoF(const char* __restrict) noexcept -> float;
    // std::atof diy version(float ver) overload for wchar_t
    auto AtoF(const wchar_t* __restrict) noexcept -> float;
    // utf-32(ucs4) to utf-16(ucs2) char
    //auto UTF32toUTF16(char32_t ch, wchar_t str[2]) ->int;
    // Base64 DataChar: Map 0~63 to visible char
    extern const char Base64Chars[65];
    // Base64 DataChar: Map visible char to 0~63
    extern const uint8_t Base64Datas[128];
    // Base64 Encode 编码
    auto __fastcall Base64Encode(IN const uint8_t* __restrict bindata, IN size_t binlen, OUT char* const __restrict base64) noexcept -> char *;
    // Base64 Decode 解码
    auto __fastcall Base64Decode(IN const char* __restrict base64, OUT uint8_t* __restrict bindata) noexcept->size_t;
    // UTF-16 to UTF-8
    // Return: UTF-8 string length, 0 maybe error
    auto __fastcall UTF16toUTF8(const char16_t* __restrict, char* __restrict) noexcept -> uint32_t;
    // UTF-8 to UTF-16
    // Return: UTF-16 string length, 0 maybe error
    auto __fastcall UTF8toUTF16(const char* __restrict, char16_t* __restrict) noexcept -> uint32_t;
    // wchar to UTF-8
    // UTF-8 to wchar
    inline auto WideChartoUTF8(const wchar_t* __restrict src, char* __restrict des)noexcept {
        // UTF-8 UTF-16 UTF-32(UCS-4)
        static_assert(sizeof(wchar_t) == sizeof(char16_t), "change UTF-16 to UTF-32");
        return UTF16toUTF8(reinterpret_cast<const char16_t*>(src), des);
    }
    // UTF-8 to wchar
    inline auto UTF8toWideChar(const char* __restrict src, wchar_t* __restrict des)noexcept {
        // UTF-8 UTF-16 UTF-32(UCS-4)
        static_assert(sizeof(wchar_t) == sizeof(char16_t), "change UTF-16 to UTF-32");
        return UTF8toUTF16(src, reinterpret_cast<char16_t*>(des));
    }
    // bubble sort for vector or list
    template<typename Itr, typename Lamda>
    void BubbleSort(Itr begin, Itr end, Lamda lam) noexcept {
        --end;
        bool flag = true;
        while (flag) {
            flag = false;
            for (auto i = begin; i != end; ++i) {
                auto j = i; ++j;
                if (lam(*j, *i)) {
                    std::swap(*j, *i);
                    flag = true;
                }
            }
            --end;
        }
    }
    // RtlGetVersion func
    using fnRtlGetVersion = NTSTATUS(NTAPI*)(PRTL_OSVERSIONINFOW lpVersionInformation);
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
    // For Windows SDK 8.1
#ifndef _WIN32_WINNT_WIN10
// _WIN32_WINNT_WIN10
#define _WIN32_WINNT_WIN10 0x0A00
#endif
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
    // IIDs
    extern const GUID IID_IMFMediaEngineClassFactory;
    extern const GUID IID_IMFMediaEngine;
    extern const GUID IID_IMFMediaEngineEx;
    extern const GUID IID_IMFMediaEngineNotify;
    extern const GUID IID_IDCompositionDevice;
    extern const GUID IID_IDWriteTextRenderer;
    extern const GUID IID_IDWriteInlineObject;
    extern const GUID IID_IDWriteFactory1;
    extern const GUID IID_IDWriteFontCollection;
    extern const GUID IID_IDWriteFontFileEnumerator;
    extern const GUID IID_IDWriteFontCollectionLoader;
    extern const GUID IID_IDXGISwapChain2;
    extern const GUID IID_ITextHost2;
    extern const GUID IID_IUIScript;
    extern const GUID IID_IUIResourceLoader;
    // IUnknown
    MakeGetIID(IUnknown);
    // IDropTarget
    MakeGetIID(IDropTarget);
    // IDataObject
    MakeGetIID(IDataObject);
    // ITaskbarList3
    MakeGetIID(ITaskbarList3);
    // ITaskbarList4
    MakeGetIID(ITaskbarList4);
    // IDropSource
    MakeGetIID(IDropSource);
    // IDXGIDevice1
    MakeGetIID(IDXGIDevice1);
    // IDXGIFactory2
    MakeGetIID(IDXGIFactory2);
    // IDropTargetHelper
    MakeGetIID(IDropTargetHelper);
    // IDXGISurface
    MakeGetIID(IDXGISurface);
    // ID3D11Texture2D
    MakeGetIID(ID3D11Texture2D);
    // ID3D11Debug
    MakeGetIID(ID3D11Debug);
    // ID3D10Multithread
    MakeGetIID(ID3D10Multithread);
    // ITfThreadMgr
    MakeGetIID(ITfThreadMgr);
    // IMFMediaEngineClassFactory
    MakeGetIID(IMFMediaEngineClassFactory);
    // IMFMediaEngine "98a1b0bb-03eb-4935-ae7c-93c1fa0e1c93"
    MakeGetIID(IMFMediaEngine);
    // IMFMediaEngineEx "83015ead-b1e6-40d0-a98a-37145ffe1ad1"
    MakeGetIID(IMFMediaEngineEx);
    // IMFMediaEngineNotify "fee7c112-e776-42b5-9bbf-0048524e2bd5"
    MakeGetIID(IMFMediaEngineNotify);
    // IDCompositionDevice "C37EA93A-E7AA-450D-B16F-9746CB0407F3"
    MakeGetIID(IDCompositionDevice);
    // IDWriteTextRenderer
    MakeGetIID(IDWriteTextRenderer);
    // IID_IDWriteInlineObject 
    MakeGetIID(IDWriteInlineObject);
    // IDWriteFactory1 ("30572f99-dac6-41db-a16e-0486307e606a")
    MakeGetIID(IDWriteFactory1);
    // IDWriteFontFileEnumerator("72755049-5ff7-435d-8348-4be97cfa6c7c") 
    MakeGetIID(IDWriteFontFileEnumerator);
    // IDWriteFontCollectionLoader("cca920e4-52f0-492b-bfa8-29c72ee0a468") 
    MakeGetIID(IDWriteFontCollectionLoader);
    // IDWriteFontCollectionLoader("cca920e4-52f0-492b-bfa8-29c72ee0a468") 
    MakeGetIID(IDWriteFontCollection);
    // ITextHost2 ("13E670F5-1A5A-11CF-ABEB-00AA00B65EA1")
    MakeGetIID(ITextHost2);
    // IUIResourceLoader {16222E4B-9AC8-4756-8CA9-75A72D2F4F60}
    MakeLongUIGetIID(IUIResourceLoader);
    // IUIScript {09B531BD-2E3B-4C98-985C-1FD6B406E53D}
    MakeLongUIGetIID(IUIScript);
}

#undef MakeGetIID
#undef MakeLongUIGetIID