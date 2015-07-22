#pragma once

// global iid
namespace LongUI {
#define MakeGetIID(c) template<> LongUIInline const IID& GetIID<c>() { return IID_##c; }
#define MakeLongUIGetIID(c) template<> LongUIInline const IID& GetIID<LongUI::c>() { return LongUI::IID_##c; }
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
    // IMFMediaEngine
    MakeGetIID(IMFMediaEngine);
    // IMFMediaEngineEx
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
}
// longui iid
namespace LongUI {
    // UIBasicTextRenderer {EDAB1E53-C1CF-4F5A-9533-9946904AD63C}
    class UIBasicTextRenderer; static const IID IID_UIBasicTextRenderer = { 
        0xedab1e53, 0xc1cf, 0x4f5a,{ 0x95, 0x33, 0x99, 0x46, 0x90, 0x4a, 0xd6, 0x3c } 
    };
    MakeLongUIGetIID(UIBasicTextRenderer);
    // IUIResourceLoader {16222E4B-9AC8-4756-8CA9-75A72D2F4F60}
    MakeLongUIGetIID(IUIResourceLoader);
    // IUIScript {09B531BD-2E3B-4C98-985C-1FD6B406E53D}
    MakeLongUIGetIID(IUIScript);
#undef MakeGetIID
#undef MakeLongUIGetIID
}