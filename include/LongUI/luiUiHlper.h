#pragma once
/**
* Copyright (c) 2014-2016 dustpg   mailto:dustpg@gmail.com
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

#include "../Core/luiInterface.h"
#include <cstdint>
#include <d2d1_3.h>




struct IDropTarget;
struct IDataObject;
struct ITaskbarList4;
struct IDXGIFactory2;
struct IDXGIFactory2;
struct IDropTargetHelper;
struct ID3D11Texture2D;
struct ID3D11Debug;
struct IMFMediaEngineClassFactory;
struct IMFMediaEngine;
struct IMFMediaEngineEx;
struct IMFMediaEngineNotify;
struct IDCompositionDevice;
struct IDWriteTextRenderer;
struct IDWriteTextRenderer1;
struct IDWriteInlineObject;
struct IDWriteFactory1;
struct IDWriteFontFileEnumerator;
struct IDWriteFontCollectionLoader;
struct IDWriteFontCollection;
struct IDWritePixelSnapping;

struct ID2D1Transform;
struct ID2D1EffectImpl;
struct ID2D1DrawTransform;
struct ID2D1TransformNode;

extern const GUID IID_ID2D1Transform;
extern const GUID IID_ID2D1EffectImpl;
extern const GUID IID_ID2D1DrawTransform;
extern const GUID IID_ID2D1TransformNode;

extern const GUID IID_IDXGIFactory2;
extern const GUID IID_ID3D11Texture2D;
extern const GUID IID_ID3D11Debug;


#define LongUI_IID_PV_ARGS(pointer) LongUI::GetIID(pointer), reinterpret_cast<void**>(&pointer)
#define LongUI_IID_PV_ARGS_Ex(pointer) LongUI::GetIID(pointer), reinterpret_cast<IUnknown**>(&pointer)
#define MakeGetIID(c) template<> LongUIInline const IID& GetIID<c>() { return IID_##c; }
#define MakeLongUIGetIID(c) template<> LongUIInline const IID& GetIID<LongUI::c>() { return LongUI::IID_##c; }

// longui::helper namespace
namespace LongUI { namespace Helper {
    // Cce for CreateControl
    struct Cce { CreateControlEvent func; size_t id; };
    // make cce, if data -> null, just return count
    auto MakeCce(const char* str, Cce* data = nullptr) noexcept ->uint32_t;
}}

namespace LongUI {
    class XUIBasicTextRenderer;
    // IIDs
    extern const GUID IID_IMFMediaEngineClassFactory;
    extern const GUID IID_IMFMediaEngine;
    extern const GUID IID_IMFMediaEngineEx;
    extern const GUID IID_IMFMediaEngineNotify;
    extern const GUID IID_IDCompositionDevice;
    extern const GUID IID_IDWriteTextRenderer;
    extern const GUID IID_IDWriteTextRenderer1;
    extern const GUID IID_IDWriteInlineObject;
    extern const GUID IID_IDWriteFactory1;
    extern const GUID IID_IDWriteFontCollection;
    extern const GUID IID_IDWritePixelSnapping;
    extern const GUID IID_IDWriteFontFileEnumerator;
    extern const GUID IID_IDWriteFontCollectionLoader;
    extern const GUID IID_IDXGISwapChain2;
    extern const GUID IID_ITextHost2;
    extern const GUID IID_IUIScript;
    extern const GUID IID_IUIResourceLoader;
    extern const GUID IID_IUITextFormatter;
    extern const GUID IID_XUIBasicTextRenderer;
    // IUnknown
    MakeGetIID(IUnknown);
    // IDropTarget
    MakeGetIID(IDropTarget);
    // IDataObject
    MakeGetIID(IDataObject);
    // ITaskbarList4
    //MakeGetIID(ITaskbarList4);
    // IDropSource
    MakeGetIID(IDropSource);
    // IDXGIDevice1
    MakeGetIID(IDXGIDevice1);
    // IDXGIFactory2
    MakeGetIID(IDXGIFactory2);
    // IDropTargetHelper
    //MakeGetIID(IDropTargetHelper);
    // ID2D1Transform
    MakeGetIID(ID2D1Transform);
    // ID2D1EffectImpl
    MakeGetIID(ID2D1EffectImpl);
    // ID2D1TransformNode
    MakeGetIID(ID2D1TransformNode);
    // ID2D1DrawTransform
    MakeGetIID(ID2D1DrawTransform);
    // IDXGISurface
    MakeGetIID(IDXGISurface);
    // bitmap
    MakeGetIID(ID2D1Bitmap1);
    // dc
    MakeGetIID(ID2D1DeviceContext);
    // dc1
    MakeGetIID(ID2D1DeviceContext1);
    // dc3
    MakeGetIID(ID2D1DeviceContext3);
    // ID3D11Texture2D
    MakeGetIID(ID3D11Texture2D);
    // ID3D11Debug
    MakeGetIID(ID3D11Debug);
    // ID3D10Multithread
    MakeGetIID(ID3D10Multithread);
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
    // IDWriteTextRenderer1
    MakeGetIID(IDWriteTextRenderer1);
    // IID_IDWriteInlineObject 
    MakeGetIID(IDWriteInlineObject);
    // IDWriteFactory1 ("30572f99-dac6-41db-a16e-0486307e606a")
    MakeGetIID(IDWriteFactory1);
    // IDWriteFontFileEnumerator("72755049-5ff7-435d-8348-4be97cfa6c7c") 
    MakeGetIID(IDWriteFontFileEnumerator);
    // IDWriteFontCollectionLoader("cca920e4-52f0-492b-bfa8-29c72ee0a468") 
    MakeGetIID(IDWriteFontCollectionLoader);
    //  IDWriteFontCollection
    MakeGetIID(IDWriteFontCollection);
    // IDWritePixelSnapping
    MakeGetIID(IDWritePixelSnapping);
    // IUIResourceLoader {16222E4B-9AC8-4756-8CA9-75A72D2F4F60}
    MakeLongUIGetIID(IUIResourceLoader);
    // IUIScript {09B531BD-2E3B-4C98-985C-1FD6B406E53D}
    MakeLongUIGetIID(IUIScript);
    // IUITextFormatter 
    MakeLongUIGetIID(IUITextFormatter);
    // IID_XUIBasicTextRenderer 
    MakeLongUIGetIID(XUIBasicTextRenderer);
}

#undef MakeGetIID
#undef MakeLongUIGetIID
