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

// get IID form typename, __uuidof is just impl in MSC
namespace LongUI {
    // Render Common Brush
    void FillRectWithCommonBrush(ID2D1RenderTarget* target, ID2D1Brush* brush, const D2D1_RECT_F& rect) noexcept;
    // using template specialization  使用模板特化
    template<typename T> LongUIInline const IID& GetIID();
    // get IID from pointer
    template<typename T> LongUIInline const IID& GetIID(T*) { return LongUI::GetIID<T>(); }
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
#ifdef _DEBUG
    // ID3D11Debug
    MakeGetIID(ID3D11Debug);
#endif
#ifdef LONGUI_VIDEO_IN_MF
    static const IID IID_IMFMediaEngineClassFactory =
    { 0x4D645ACE, 0x26AA, 0x4688,{ 0x9B, 0xE1, 0xDF, 0x35, 0x16, 0x99, 0x0B, 0x93 } };
    // IMFMediaEngineClassFactory
    MakeGetIID(IMFMediaEngineClassFactory);
    static const IID IID_IMFMediaEngine =
    { 0x98A1B0BB, 0x03EB, 0x4935,{ 0xAE, 0x7C, 0x93, 0xC1, 0xFA, 0x0E, 0x1C, 0x93 } };
    // IMFMediaEngine "98a1b0bb-03eb-4935-ae7c-93c1fa0e1c93"
    MakeGetIID(IMFMediaEngine);
    static const IID IID_IMFMediaEngineEx =
    { 0x83015EAD, 0xB1E6, 0x40D0,{ 0xA9, 0x8A, 0x37, 0x14, 0x5F, 0xFE, 0x1A, 0xD1 } };
    // IMFMediaEngineEx "83015ead-b1e6-40d0-a98a-37145ffe1ad1"
    MakeGetIID(IMFMediaEngineEx);
    // IMFMediaEngineNotify "fee7c112-e776-42b5-9bbf-0048524e2bd5"
    static const IID IID_IMFMediaEngineNotify =
    { 0xfee7c112, 0xe776, 0x42b5,{ 0x9B, 0xBF, 0x00, 0x48, 0x52, 0x4E, 0x2B, 0xD5 } };
    MakeGetIID(IMFMediaEngineNotify);
#endif
    // IDCompositionDevice "C37EA93A-E7AA-450D-B16F-9746CB0407F3"
    static const IID IID_IDCompositionDevice =
    { 0xC37EA93A, 0xE7AA, 0x450D,{ 0xB1, 0x6F, 0x97, 0x46, 0xCB, 0x04, 0x07, 0xF3 } };
    MakeGetIID(IDCompositionDevice);
    // IDWriteTextRenderer
    static const IID IID_IDWriteTextRenderer =
    { 0xef8a8135, 0x5cc6, 0x45fe,{ 0x88, 0x25, 0xc5, 0xa0, 0x72, 0x4e, 0xb8, 0x19 } };
    MakeGetIID(IDWriteTextRenderer);
    // IDWriteFactory1 ("30572f99-dac6-41db-a16e-0486307e606a")
    static const IID IID_IDWriteFactory1 =
    { 0x30572f99, 0xdac6, 0x41db,{ 0xa1, 0x6e, 0x04, 0x86, 0x30, 0x7e, 0x60, 0x6a } };
    MakeGetIID(IDWriteFactory1);
    // IDWriteFontFileEnumerator("72755049-5ff7-435d-8348-4be97cfa6c7c") 
    static const IID IID_IDWriteFontFileEnumerator = {
        0x72755049, 0x5ff7, 0x435d,{ 0x83, 0x48, 0x4b, 0xe9, 0x7c, 0xfa, 0x6c, 0x7c }
    };
    MakeGetIID(IDWriteFontFileEnumerator);
    // IDWriteFontCollectionLoader("cca920e4-52f0-492b-bfa8-29c72ee0a468") 
    static const IID IID_IDWriteFontCollectionLoader = {
        0xcca920e4, 0x52f0, 0x492b,{ 0xbf, 0xa8, 0x29, 0xc7, 0x2e, 0xe0, 0xa4, 0x68 }
    };
    MakeGetIID(IDWriteFontCollectionLoader);
    // ITextHost2 ("13E670F5-1A5A-11CF-ABEB-00AA00B65EA1")
    static const IID  IID_ITextHost2 = {
        0x13E670F5, 0x1A5A, 0x11CF,{ 0xAB, 0xEB, 0x00, 0xAA, 0x00, 0xB6, 0x5E, 0xA1 }
    };
    MakeGetIID(ITextHost2);
    // CUIBasicTextRenderer {EDAB1E53-C1CF-4F5A-9533-9946904AD63C}
    static const IID IID_CUIBasicTextRenderer = {
        0xedab1e53, 0xc1cf, 0x4f5a,{ 0x95, 0x33, 0x99, 0x46, 0x90, 0x4a, 0xd6, 0x3c }
    };
    class CUIBasicTextRenderer;
    MakeLongUIGetIID(CUIBasicTextRenderer);
    // IUIResourceLoader {16222E4B-9AC8-4756-8CA9-75A72D2F4F60}
    MakeLongUIGetIID(IUIResourceLoader);
    // IUIScript {09B531BD-2E3B-4C98-985C-1FD6B406E53D}
    MakeLongUIGetIID(IUIScript);
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
    LongUINoinline static auto TransformPointInverse(const D2D1_MATRIX_3X2_F& matrix, const D2D1_POINT_2F& point) noexcept {
        D2D1_POINT_2F result;
        // x = (bn-dm)/(bc-ad)
        // y = (an-cm)/(ad-bc)
        // a : m_matrix._11
        // b : m_matrix._21
        // c : m_matrix._12
        // d : m_matrix._22
        register auto bc_ad = matrix._21 * matrix._12 - matrix._11 * matrix._22;
        register auto m = point.x - matrix._31;
        register auto n = point.y - matrix._32;
        result.x = (matrix._21*n - matrix._22 * m) / bc_ad;
        result.y = (matrix._12*m - matrix._11 * n) / bc_ad;
        return result;
    }
    // 四舍五入
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
        static decltype(&::DCompositionCreateDevice) DCompositionCreateDevice;
    };
    // std::atoi diy version
    auto __fastcall AtoI(const char* __restrict) -> int;
    // std::atof diy version(float ver)
    auto __fastcall AtoF(const char* __restrict) -> float;
    // LongUI::AtoI diy version(double ver)
    //auto __fastcall AtoLF(const char*) -> double;
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
    // wchar to UTF-8
    // UTF-8 to wchar
    static auto WideChartoUTF8(const wchar_t* __restrict src, char* __restrict des)noexcept {
        // UTF-8 UTF-16 UTF-32(UCS-4)
        static_assert(sizeof(wchar_t) == sizeof(char16_t), "change UTF-16 to UTF-32");
        return UTF16toUTF8(reinterpret_cast<const char16_t*>(src), des);
    }
    // UTF-8 to wchar
    static auto UTF8toWideChar(const char* __restrict src, wchar_t* __restrict des)noexcept {
        // UTF-8 UTF-16 UTF-32(UCS-4)
        static_assert(sizeof(wchar_t) == sizeof(char16_t), "change UTF-16 to UTF-32");
        return UTF8toUTF16(src, reinterpret_cast<char16_t*>(des));
    }
    // bubble sort
    template<typename Itr, typename Lamda>
    void BubbleSort(const Itr begin, const Itr end, Lamda lam) noexcept {
        if (end - begin < 2) return;
        for (auto itr_i = begin; itr_i != end; ++itr_i) {
            bool sorted = true;
            for (auto itr_j = itr_i; itr_j != end; ++itr_j) {
                if (lam(*itr_j, *itr_i)) {
                    std::swap(*itr_i, *itr_j);
                    sorted = false;
                }
            }
            if (sorted) return;
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
}

#undef MakeGetIID
#undef MakeLongUIGetIID