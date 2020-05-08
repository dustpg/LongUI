#include <luiconf.h>
// ui
#include <graphics/ui_adapter_desc.h>
#include <resource/ui_bitmap_bank.h>
#include <resource/ui_resource_id.h>
#include <interface/ui_iconfig.h>
#include <resource/ui_resource.h>
#include <filesystem/ui_pathop.h>
#include <filesystem/ui_file.h>
#include <container/pod_hash.h>
#include <debugger/ui_debug.h>
#include <core/ui_manager.h>
#include <core/ui_string.h>
#include <style/ui_text.h>
#include <core/ui_malloc.h>
#include <util/ui_unicode.h>
#include <util/ui_unicode_cast.h>
// Effect
#include <effect/ui_effect_backimage.h>
#include <effect/ui_effect_borderimage.h>

// private
#include "../private/ui_private_image.h"

// c++
#include <cassert>
#include <cstring>

// system
#include <graphics/ui_graphics_impl.h>
#include <text/ui_ctl_arg.h>
#include <text/ui_ctl_impl.h>

// window
#include <wincodec.h>

#pragma comment(lib, "dxgi")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "Imm32")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "windowscodecs")

// ui namespace
namespace LongUI {
    /// <summary>
    /// The iid identifier write factory1
    /// </summary>
    const GUID IID_IDWriteFactory1 = {
        0x30572f99, 0xdac6, 0x41db,
        { 0xa1, 0x6e, 0x04, 0x86, 0x30, 0x7e, 0x60, 0x6a }
    };
    /// <summary>
    /// The iid identifier write text layout1
    /// </summary>
    const GUID IID_IDWriteTextLayout1 = {
        0x9064d822, 0x80a7, 0x465c, {
        0xa9, 0x86, 0xdf, 0x65, 0xf7, 0x8b, 0x8f, 0xeb }
    };
    /// <summary>
    /// The iid idxgi factory2
    /// </summary>
    const GUID IID_IDXGIFactory2 = {
        0x50c83a1c, 0xe072, 0x4c48, {
        0x87, 0xb0, 0x36, 0x30, 0xfa, 0x36, 0xa6, 0xd0 }
    };
}

// ui-impl namespace
namespace LongUI { namespace impl { 
    /// <summary>
    /// args for save bitmap
    /// </summary>
    struct save_bitmap_args {
        // data for bitmap
        uint8_t*                bits;
        // factory for WIC
        IWICImagingFactory*     factory;
        // format for source data, default: GUID_WICPixelFormat32bppBGRA
        const GUID*             data_format;
        // format for container, default: GUID_ContainerFormatPng
        const GUID*             container_format;
        // width of image
        uint32_t                width;
        // height of image
        uint32_t                height;
        // pitch of image
        uint32_t                pitch;
        // unused
        uint32_t                unused;
    };
    /// <summary>
    /// Saves the bitmap.
    /// </summary>
    /// <param name="args">The arguments.</param>
    /// <param name="file_name">Name of the file.</param>
    /// <returns></returns>
    auto save_bitmap(
        const save_bitmap_args& args, 
        const wchar_t* file_name
    ) noexcept -> HRESULT {
        assert(args.bits && args.factory && file_name && file_name[0]);
        if (!(args.bits && args.factory)) {
            return E_INVALIDARG;
        }
        // 初始化
        HRESULT hr = S_OK;
        IWICBitmapEncoder *pEncoder = nullptr;
        IWICBitmapFrameEncode *frame_encode = nullptr;
        IWICStream *stream = nullptr;
        IWICBitmap *wic_bitmap = nullptr;
        // 创建WIC位图
        if (SUCCEEDED(hr)) {
            hr = args.factory->CreateBitmapFromMemory(
                args.width,
                args.height,
                args.data_format ? *args.data_format : GUID_WICPixelFormat32bppBGRA,
                args.pitch,
                args.pitch * args.height,
                args.bits,
                &wic_bitmap
            );
            longui_debug_hr(Result{ hr }, "failed: args.factory->CreateBitmapFromMemory");
        }
        // 创建流
        if (SUCCEEDED(hr)) {
            hr = args.factory->CreateStream(&stream);
            longui_debug_hr(Result{ hr }, "failed: args.factory->CreateStream");
        }
        // 从文件初始化
        if (SUCCEEDED(hr)) {
            hr = stream->InitializeFromFilename(file_name, GENERIC_WRITE);
            longui_debug_hr(Result{ hr }, "failed: stream->InitializeFromFilename  " << file_name);
        }
        // 创建编码器
        if (SUCCEEDED(hr)) {
            hr = args.factory->CreateEncoder(
                args.container_format ? *args.container_format : GUID_ContainerFormatPng,
                nullptr,
                &pEncoder
            );
            longui_debug_hr(Result{ hr }, "failed: args.factory->CreateEncoder");
        }
        // 初始化编码器
        if (SUCCEEDED(hr)) {
            hr = pEncoder->Initialize(stream, WICBitmapEncoderNoCache);
            longui_debug_hr(Result{ hr }, "failed: pEncoder->Initialize");
        }
        // 创建新的一帧
        if (SUCCEEDED(hr)) {
            hr = pEncoder->CreateNewFrame(&frame_encode, nullptr);
            longui_debug_hr(Result{ hr }, "failed: pEncoder->CreateNewFrame");
        }
        // 初始化帧编码器
        if (SUCCEEDED(hr)) {
            hr = frame_encode->Initialize(nullptr);
            longui_debug_hr(Result{ hr }, "failed: frame_encode->Initialize");
        }
        // 设置大小
        if (SUCCEEDED(hr)) {
            hr = frame_encode->SetSize(args.width, args.height);
            longui_debug_hr(Result{ hr }, "failed: frame_encode->SetSize " << int32_t(args.width) << L", " << int32_t(args.height));
        }
        // 设置格式
        WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
        if (SUCCEEDED(hr)) {
            hr = frame_encode->SetPixelFormat(&format);
            longui_debug_hr(Result{ hr }, "failed: frame_encode->SetPixelFormat");
        }
        // 写入源数据
        if (SUCCEEDED(hr)) {
            hr = frame_encode->WriteSource(wic_bitmap, nullptr);
            longui_debug_hr(Result{ hr }, "failed: frame_encode->WriteSource");
        }
        // 提交帧编码器
        if (SUCCEEDED(hr)) {
            hr = frame_encode->Commit();
            longui_debug_hr(Result{ hr }, "failed: frame_encode->Commit");
        }
        // 提交编码
        if (SUCCEEDED(hr)) {
            hr = pEncoder->Commit();
            longui_debug_hr(Result{ hr }, "failed: pEncoder->Commit");
        }
        // 扫尾处理
        LongUI::SafeRelease(wic_bitmap);
        LongUI::SafeRelease(stream);
        LongUI::SafeRelease(frame_encode);
        LongUI::SafeRelease(pEncoder);
        // 返回结果
        return hr;
    }
    /// <summary>
    /// Loads the bitmap.
    /// </summary>
    /// <param name="create">The create.</param>
    /// <param name="pIWICFactory">The p iwic factory.</param>
    /// <param name="buf">The buf.</param>
    /// <param name="len">The length.</param>
    /// <param name="bitmap">The bitmap.</param>
    /// <returns></returns>
    template<typename Func> inline auto load_bitmap(
        Func& func,
        IWICImagingFactory* pIWICFactory,
        const GUID& alpha,
        //HANDLE file,
        uint8_t* buf,
        uint32_t len
    ) noexcept -> HRESULT {
        IWICStream* stream = nullptr;
        IWICBitmapDecoder *decoder = nullptr;
        IWICBitmapFrameDecode *source = nullptr;
        IWICFormatConverter *converter = nullptr;
        HRESULT hr = S_OK;
        Size2U size{ 0 };
        uint8_t* ptr = nullptr;
        UINT frame_count = 0, frame_delay = 0, bypch = 0, bylen = 0;
        // 创建内存流
        if (SUCCEEDED(hr)) {
            hr = pIWICFactory->CreateStream(&stream);
            longui_debug_hr(Result{ hr }, L"failed: pIWICFactory->CreateStream");
        }
        // 创建内存流
        if (SUCCEEDED(hr)) {
            hr = stream->InitializeFromMemory(buf, len);
            longui_debug_hr(Result{ hr }, L"failed: stream->InitializeFromMemory");
        }
        // 创建解码器
        if (SUCCEEDED(hr)) {
            hr = pIWICFactory->CreateDecoderFromStream(
                stream,
                nullptr,
                WICDecodeMetadataCacheOnLoad,
                &decoder
            );
            longui_debug_hr(Result{ hr }, L"failed: pIWICFactory->CreateDecoderFromStream");
        }
        // 获取帧数
        if (SUCCEEDED(hr)) {
            decoder->GetFrameCount(&frame_count);
            if (!frame_count) frame_count = 1;
        }
        // 获取第一帧
        if (SUCCEEDED(hr)) {
            hr = decoder->GetFrame(0, &source);
            longui_debug_hr(Result{ hr }, L"failed: decoder->GetFrame");
        }
        // 获取大小
        if (SUCCEEDED(hr)) {
            hr = source->GetSize(&size.width, &size.height);
            longui_debug_hr(Result{ hr }, L"failed: source->GetSize");
        }
        // 准备获取帧间隔
        const uint32_t real_count = Func::ONLY1 ? 1 : frame_count;
        if (real_count > 1) {
            PROPVARIANT propValue;
            ::PropVariantInit(&propValue);
            IWICMetadataQueryReader* metareader = nullptr;
            // 元信息读取
            if (SUCCEEDED(hr)) {
                hr = source->GetMetadataQueryReader(&metareader);
                longui_debug_hr(Result{ hr }, L"source->GetMetadataQueryReader");
            }
            // 获取帧间隔
            if (SUCCEEDED(hr)) {
                const auto delay = L"/grctlext/Delay";
                hr = metareader->GetMetadataByName(delay, &propValue);
            }
            // 获取帧间隔
            if (SUCCEEDED(hr)) {
                assert(propValue.vt == VT_UI2);
                // 单位是10ms
                frame_delay = static_cast<uint32_t>(propValue.uiVal) * 10;
            }
            ::PropVariantClear(&propValue);
            LongUI::SafeRelease(metareader);
            longui_debug_hr(Result{ hr }, L"failed: source->GetSize");
        }
        // 初始化帧信息
        if (SUCCEEDED(hr)) {
            hr = func.frame(real_count, frame_delay, size);
            longui_debug_hr(Result{ hr }, L"failed: func.frame");
        }
        // 申请空间
        if (SUCCEEDED(hr)) {
            const auto sizeof_rgba = static_cast<uint32_t>(sizeof(RGBA));
            bylen = size.width * size.height * sizeof_rgba;
            bypch = size.width * sizeof_rgba;
            ptr = LongUI::NormalAllocT<uint8_t>(bylen);
            if (!ptr) hr = E_OUTOFMEMORY;
            longui_debug_hr(Result{ hr }, L"failed: LongUI::NormalAllocT");
        }
        // 遍历所有帧
        for (uint32_t i = 0; i != real_count; ++i) {
            IWICBitmapFrameDecode * frame = nullptr;
            WICRect rect = { 0, 0, int32_t(size.width), int32_t(size.height) };
            // 直接初始化
            if (SUCCEEDED(hr)) {
                hr = decoder->GetFrame(i, &frame);
                longui_debug_hr(Result{ hr }, L"failed: decoder->GetFrame");
            }
            // 创建格式转换器
            if (SUCCEEDED(hr)) {
                hr = pIWICFactory->CreateFormatConverter(&converter);
                longui_debug_hr(Result{ hr }, L"failed: pIWICFactory->CreateFormatConverter");
            }
            // 直接初始化
            if (SUCCEEDED(hr)) {
                hr = converter->Initialize(
                    frame,
                    //GUID_WICPixelFormat32bppRGBA,
                    GUID_WICPixelFormat32bppPRGBA,
                    WICBitmapDitherTypeNone,
                    nullptr,
                    0.f,
                    WICBitmapPaletteTypeMedianCut
                );
                longui_debug_hr(Result{ hr }, L"failed: converter->Initialize");
            }
            // 获取动画相关元信息
            if (real_count > 1) {
                PROPVARIANT propValue;
                ::PropVariantInit(&propValue);
                IWICMetadataQueryReader* metareader = nullptr;
                // 元信息读取
                if (SUCCEEDED(hr)) {
                    hr = frame->GetMetadataQueryReader(&metareader);
                    longui_debug_hr(Result{ hr }, L"source->GetMetadataQueryReader");
                }
                // X座标
                if (SUCCEEDED(hr)) {
                    hr = metareader->GetMetadataByName(L"/imgdesc/Left", &propValue);
                    assert(propValue.vt == VT_UI2);
                    rect.X = propValue.uiVal;
                }
                // Y座标
                if (SUCCEEDED(hr)) {
                    ::PropVariantClear(&propValue);
                    hr = metareader->GetMetadataByName(L"/imgdesc/Top", &propValue);
                    assert(propValue.vt == VT_UI2);
                    rect.Y = propValue.uiVal;
                }
                // 宽度
                if (SUCCEEDED(hr)) {
                    ::PropVariantClear(&propValue);
                    hr = metareader->GetMetadataByName(L"/imgdesc/Width", &propValue);
                    assert(propValue.vt == VT_UI2);
                    rect.Width = propValue.uiVal;
                    ::PropVariantClear(&propValue);
                }
                // 高度
                if (SUCCEEDED(hr)) {
                    hr = metareader->GetMetadataByName(L"/imgdesc/Height", &propValue);
                    assert(propValue.vt == VT_UI2);
                    rect.Height = propValue.uiVal;
                }
                ::PropVariantClear(&propValue);
                LongUI::SafeRelease(metareader);
            }
            // 复制数据
            if (SUCCEEDED(hr)) {
                const auto adjusted = ptr + rect.X * sizeof(RGBA) + rect.Y * bypch;
                rect.X = rect.Y = 0;
                hr = converter->CopyPixels(&rect, bypch, bylen, adjusted);
                longui_debug_hr(Result{ hr }, L"converter->CopyPixels");
            }
            // 创建位图
            if (SUCCEEDED(hr)) {
                hr = func.create(i, size, ptr, bypch);
                longui_debug_hr(Result{ hr }, L"func.create");
            }
            LongUI::SafeRelease(frame);
            LongUI::SafeRelease(converter);
        }
        LongUI::NormalFree(ptr);
        LongUI::SafeRelease(source);
        LongUI::SafeRelease(stream);
        LongUI::SafeRelease(decoder);
        return hr;
    };
}}

/// <summary>
/// private data/func for resmgr
/// </summary>
struct LongUI::CUIResMgr::Private {
    // remove_res_only
    static inline void remove_res_only(const char* str) noexcept;
    // remove_res_only
    static inline void free_rects(BitmapFrame& f) noexcept;
    // ctor
    Private() noexcept {}
    // dtor
    ~Private() noexcept {}
    // resource map
    using ResourceMap = POD::HashMap<uintptr_t>;
    // init
    auto init() noexcept->Result;
    // release
    void release() noexcept;
    // push index0
    auto push_index0_res() noexcept ->Result;
    // load & create
    auto load_create(U8View) noexcept->CUISharedResource*;
    // load & create
    auto recreate(CUIImage&) noexcept->Result;
    // d2d factroy
    ID2D1Factory1*          d2dfactroy;
    // dwrite factroy
    IDWriteFactory1*        dwritefactroy;
    // wic factroy
    IWICImagingFactory*     wicfactroy;
    // alpha mode
    const GUID*             alpha;
    // default font
    I::Font*                deffont;
    // resource data
    ResourceMap             resmap;
    // bitmap bank
    CUIBitmapBank           bitbank;
    // default font
    FontArg                 defarg;
    // resource count
    uint32_t                rescount;
};


// ----------------------------------------------------------------------------
// ---------------------------  Private Manager  ------------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
//inline auto LongUI::CUIResMgr::Private::recreate() noexcept -> Result {
//    assert(!"NOT IMPL");
//    return{};
//}

/// <summary>
/// Pushes the index0 resource.
/// </summary>
/// <returns></returns>
auto LongUI::CUIResMgr::Private::push_index0_res() noexcept ->Result {
    // 用不用一个永久有效节点代替NULL?
#if 0
    assert(this->reslist.empty() && "bad list");
    reslist.reserve(128);
    // 内存不足
    if (!reslist.is_ok()) return { Result::RE_OUTOFMEMORY };
    // 资源数据
    const auto data = ResourceData { nullptr, ":lui/index0", 0 };
    this->rescount++;
    reslist.push_back(data);
#endif
    return { Result::RS_OK };
}

/// <summary>
/// Initializes this instance.
/// </summary>
/// <returns></returns>
inline auto LongUI::CUIResMgr::Private::init() noexcept -> Result {
    this->alpha = &GUID_WICPixelFormat32bppPRGBA;
    // 设置默认字体
    this->defarg = { 
        "Arial", 12.5f, 1.2f, 0.f, 
        Weight_Normal, Style_Normal, Stretch_Normal
    };
    Result hr = { Result::RS_OK };
    // 创建 WIC 工厂.
    if (hr) {
        hr = { ::CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_IWICImagingFactory,
            reinterpret_cast<void**>(&wicfactroy)
        ) };
        longui_debug_hr(Result{ hr }, L"CoCreateInstance IWICImagingFactory faild");
    }
    // 创建D2D工厂
    if (hr) {
        D2D1_FACTORY_OPTIONS options = { D2D1_DEBUG_LEVEL_NONE };
#ifndef NDEBUG
        options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
        hr = { ::D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            IID_ID2D1Factory1,
            &options,
            reinterpret_cast<void**>(&this->d2dfactroy)
        ) };
        longui_debug_hr(Result{ hr }, L"D2D1CreateFactory faild");
    }
#ifndef LUI_DISABLE_STYLE_SUPPORT
    if (hr) hr = Effect::RegisterBackImage(this->d2dfactroy);
    if (hr) hr = Effect::RegisterBorderImage(this->d2dfactroy);
#endif
    // 创建DWrite工厂
    if (hr) {
        hr = { ::DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            IID_IDWriteFactory1,
            reinterpret_cast<IUnknown**>(&this->dwritefactroy)
        ) };
        longui_debug_hr(Result{ hr }, L"DWriteCreateFactory faild");
    }
    // 创建INDEX0资源
    if (hr) {
        hr = this->push_index0_res();
    }
    return hr;
}

inline
/// <summary>
/// Removes the resource only.
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
void LongUI::CUIResMgr::Private::remove_res_only(const char * str) noexcept {
    auto& map = UIManager.rm().resmap;
    const auto itr = map.find(str);
    assert(str && itr != map.end());
#if 0
    map.remove(itr);
#else
    // 仅仅标记0
    if (itr != map.end()) itr->second = 0;
#endif
}

/// <summary>
/// Frees the rects.
/// </summary>
/// <param name="f">The f.</param>
/// <returns></returns>
void LongUI::CUIResMgr::Private::free_rects(BitmapFrame& f) noexcept {
    UIManager.rm().bitbank.Free(f);
}

// longui namespace
namespace LongUI { namespace detail { 
    /// <summary>
    /// Removes the resource only.
    /// </summary>
    /// <param name="">The .</param>
    /// <returns></returns>
    void remove_res_only(const char* text) noexcept {
        CUIResMgr::Private::remove_res_only(text);
    }
    /// <summary>
    /// Frees the rects.
    /// </summary>
    /// <param name="f">The f.</param>
    /// <returns></returns>
    void free_rects(BitmapFrame& f) noexcept {
        CUIResMgr::Private::free_rects(f);
    }
}}

/// <summary>
/// Releases this instance.
/// </summary>
/// <returns></returns>
inline void LongUI::CUIResMgr::Private::release() noexcept {
    this->bitbank.ReleaseAll();
    LongUI::SafeRelease(this->d2dfactroy);
    LongUI::SafeRelease(this->dwritefactroy);
    LongUI::SafeRelease(this->wicfactroy);
    LongUI::SafeRelease(this->deffont);
}

/// <summary>
/// Saves as PNG.
/// </summary>
/// <param name="bitmap">The bitmap.</param>
/// <param name="file">The file.</param>
/// <returns></returns>
auto LongUI::CUIResMgr::SaveAsPng(
    I::Bitmap& bitmap, 
    const wchar_t* file) noexcept -> Result {
    // 参数检查
    assert(file && file[0] && "bad arg");
    // 渲染锁
    CUIRenderAutoLocker locker;
    // 初始化
    D2D1_MAPPED_RECT rect = { 0 };
    Result hr = { Result::RS_OK };
    ID2D1Bitmap1* readable_bitmap = nullptr;
    const auto bitmap_size = bitmap.GetPixelSize();
    const auto bitmap_format = bitmap.GetPixelFormat();
    // 创建CPU可读位图
    if (SUCCEEDED(hr)) {
        // CPU 可读?
        if (bitmap.GetOptions() & D2D1_BITMAP_OPTIONS_CPU_READ) {
            readable_bitmap = LongUI::SafeAcquire(&bitmap);
        }
        else {
            D2D1_BITMAP_PROPERTIES1 prop;
            prop.pixelFormat = bitmap_format;
            bitmap.GetDpi(&prop.dpiX, &prop.dpiY);
            prop.colorContext = nullptr;
            prop.bitmapOptions = D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
            // 创建位图
            hr = { m_p2DRenderer->CreateBitmap(
                bitmap_size,
                nullptr,
                0,
                &prop,
                &readable_bitmap
            ) };
#ifndef NDEBUG
            Point2F ppppt;
            ppppt.x = float(bitmap_size.width);
            ppppt.y = float(bitmap_size.height);
            longui_debug_hr(Result{ hr }, L"failed: RenderTarget->CreateBitmap " << ppppt);
#endif
            // 复制位图
            if (SUCCEEDED(hr)) {
                hr = { readable_bitmap->CopyFromBitmap(nullptr, &bitmap, nullptr) };
                longui_debug_hr(Result{ hr }, L"failed: readable_bitmap->CopyFromBitmap");
            }
        }
    }
    // 映射数据
    if (SUCCEEDED(hr)) {
        hr = { readable_bitmap->Map(D2D1_MAP_OPTIONS_READ, &rect) };
        longui_debug_hr(Result{ hr }, L"failed: readable_bitmap->Map");
    }
    // 处理
    if (SUCCEEDED(hr)) {
        impl::save_bitmap_args prop;
        prop.bits = rect.bits;
        prop.factory = rm().wicfactroy;
        prop.data_format = bitmap_format.format == DXGI_FORMAT_B8G8R8A8_UNORM ?
            &GUID_WICPixelFormat32bppBGRA : &GUID_WICPixelFormat32bppRGBA;
        prop.container_format = &GUID_ContainerFormatPng;
        prop.width = bitmap_size.width;
        prop.height = bitmap_size.height;
        prop.pitch = rect.pitch;
        Result hr1 = { impl::save_bitmap(prop, file) };
        longui_debug_hr(Result{ hr1 }, L"failed: DX::SaveAsImageFile");
        Result hr2 = { readable_bitmap->Unmap() };
        longui_debug_hr(Result{ hr2 }, L"failed: readable_bitmap->Unmap");
        // 检查错误
        if (hr1) {
            hr = hr2 ? Result{ Result::RS_OK } : hr2;
        }
        else {
            hr = hr1;
        }
    }
    // 扫尾处理
    LongUI::SafeRelease(readable_bitmap);
    // 返回结果
    return hr;
}


/// <summary>
/// Sets the alpha mode.
/// </summary>
/// <param name="premultiply">if set to <c>true</c> [premultiply].</param>
/// <returns></returns>
void LongUI::CUIResMgr::SetAlphaMode(bool premultiply) noexcept {
    rm().alpha
        = premultiply
        ? &GUID_WICPixelFormat32bppPRGBA
        : &GUID_WICPixelFormat32bppRGBA
        ;
}

/// <summary>
/// Loads the resource.
/// </summary>
/// <param name="uri">The URI.</param>
/// <param name="type">The type.</param>
/// <param name="is_xul_dir">if set to <c>true</c> [is xul dir].</param>
/// <returns></returns>
auto LongUI::CUIResMgr::LoadResource(U8View uri, 
    //ResourceType type, 
    bool is_xul_dir) noexcept -> uintptr_t {
    const ResourceType type = ResourceType::Type_Image;
    // uri为空
    if (uri.end() == uri.begin()) return 0;



    // 路径信息
    PathOP::UriPath path_buf;
    auto get_dir = [is_xul_dir]() noexcept {
        return is_xul_dir ? UIManager.GetXulDir() : U8View{};
    };
    // 转换统一URI字符串
    const auto real_uri = PathOP::MakeUriPath(path_buf, get_dir(), uri);
    // TODO:检测问题
    if (real_uri.end() == real_uri.begin()) { 
        //RE_BUFFEROVER;
        assert(!"ERROR"); 
        return 0; 
    }


    // 插入URL 先占位子
    auto& map = rm().resmap;
    const auto re = map.insert(real_uri.begin(), real_uri.end(), 0);
    // 内存不足
    if (re.first == map.end()) { assert(!"ERROR"); return 0; }
    // 插入失败 而且 数据不为0就是已有的
    if (!re.second && re.first->second) {
        const auto id = re.first->second;
        //assert(type == list[id].GetType() && "must be same");
        return id;
    }

    // 创建资源对象
    const auto res = rm().load_create(real_uri);
    if (!res) return 0;
    // 写入数据
    auto& data = const_cast<ResourceData&>(res->RefData());
    data.uri = re.first->first;
    data.ref = 0;
    return re.first->second = CUIResourceID::ObjectId(res);
}


/// <summary>
/// Loads the and create.
/// </summary>
/// <param name="uri">The URI.</param>
/// <returns></returns>
auto LongUI::CUIResMgr::Private::load_create(U8View uri) noexcept -> CUISharedResource* {
    POD::Vector<uint8_t> buffer;
    UIManager.LoadDataFromUrl(uri, buffer);
    const auto len = static_cast<uint32_t>(buffer.size());
    // XXX: 错误信息丢失?
    if (!len) return nullptr;
    const auto ptr = buffer.data();
    // 渲染锁
    CUIRenderAutoLocker locker;
    // 创建位图
    struct create_bitmap_func {
        enum { ONLY1 = false };
        CUIImage*       image;
        // 帧信息
        auto frame(uint32_t c, uint32_t d, Size2U s) noexcept -> HRESULT {
            // 创建图像
            const auto img = CUIImage::Create(c, d, s);
            if (!img) return Result::RE_OUTOFMEMORY;
            this->image = img;
            const auto frames = const_cast<BitmapFrame*>(&img->RefFrame(0));
            // 根据大小分配
            if (s.width < BITBANK_BITMAP_RMAX && s.height < BITBANK_BITMAP_RMAX) {
                auto& bank = UIManager.rm().bitbank;
                // 分配空间
                for (uint32_t i = 0; i != c; ++i) {
                    // 分配失败
                    const auto hr = bank.Alloc(s, frames[i]);
                    if (!hr) return hr.code;
                }
            }
            // 直接分配
            else {
#ifndef NDEBUG
                if (c > 1) {
                    LUIDebug(Warning)
                        << "UIManager.LoadResource is for small image and common use. "
                        << "this animation-image is too big: " << s << 'x' << c << endl;
                }
#endif // !NDEBUG

                // 分配位图
                for (uint32_t i = 0; i != c; ++i) {
                    const auto hr = UIManager.CreateBitmap(s, luiref frames[i].bitmap);
                    frames[i].source.right = img->size.width;
                    frames[i].source.bottom = img->size.height;
                    frames[i].rect.width = s.width;
                    frames[i].rect.height = s.height;
                    // 分配失败
                    if (!hr) return hr.code;
                }
            }
            return Result::RS_OK;
        }
        // 创建
        auto create(uint32_t i, Size2U, const uint8_t* color, uint32_t pitch) noexcept {
            assert(image);
            const auto rgba = reinterpret_cast<const RGBA*>(color);
            const auto frame = this->image->RefFrame(i);
            const D2D1_RECT_U des = {
                frame.rect.left,
                frame.rect.top,
                frame.rect.left + frame.rect.width,
                frame.rect.top + frame.rect.height,
            };
            return frame.bitmap->CopyFromMemory(&des, color, pitch);
        }
    } obj{ nullptr };
    // 正式载入
    const Result hr{ impl::load_bitmap(obj, this->wicfactroy, *alpha, ptr, len) };
    // 载入失败
    if (!hr) {
        if (obj.image) obj.image->Destroy();
        return nullptr;
    }
    assert(obj.image);
    return obj.image;
}

/// <summary>
/// Recreates the specified img.
/// </summary>
/// <param name="img">The img.</param>
/// <returns></returns>
auto LongUI::CUIResMgr::Private::recreate(CUIImage& img) noexcept -> Result {
    const auto uri = U8View::FromCStyle(img.RefData().uri);
    POD::Vector<uint8_t> buffer;
    UIManager.LoadDataFromUrl(uri, buffer);
    const auto len = static_cast<uint32_t>(buffer.size());
    // 内存不足
    if (!len) return { Result::RE_OUTOFMEMORY };
    const auto ptr = buffer.data();
    // 渲染锁
    CUIRenderAutoLocker locker;
    // 重建位图
    struct recreate_func {
        enum { ONLY1 = false };
        CUIImage&       image;
        // 帧信息
        auto frame(uint32_t c, uint32_t d, Size2U s) noexcept -> HRESULT {
            const auto frames = const_cast<BitmapFrame*>(&this->image.RefFrame(0));
            assert(this->image.delay == d);
            assert(this->image.frame_count == c);
            assert(static_cast<uint32_t>(this->image.size.width) == s.width);
            assert(static_cast<uint32_t>(this->image.size.height) == s.height);
            const auto fc = this->image.frame_count;
            for (uint32_t i = 0; i != fc; ++i) {
                auto& frame = frames[i];
                assert(frame.bitmap == nullptr);
                // 共享
                if (frame.window) {
                    assert(frame.window->bitmap && "BUG");
                    frame.window->bitmap->AddRef();
                    frame.bitmap = frame.window->bitmap;
                }
                // 单独
                else {
                    const auto hr = UIManager.CreateBitmap(s, luiref frames[i].bitmap);
                    if (!hr) return hr;
                }
            }
            return Result::RS_OK;
        }
        // 创建
        auto create(uint32_t i, Size2U, const uint8_t* color, uint32_t pitch) noexcept {
            const auto rgba = reinterpret_cast<const RGBA*>(color);
            const auto frame = this->image.RefFrame(i);
            const D2D1_RECT_U des = {
                frame.rect.left,
                frame.rect.top,
                frame.rect.left + frame.rect.width,
                frame.rect.top + frame.rect.height,
            };
            return frame.bitmap->CopyFromMemory(&des, color, pitch);
        }
    } obj{ img };
    // 正式载入
    const Result hr{ impl::load_bitmap(obj, this->wicfactroy, *alpha, ptr, len) };

    return hr;
}

/// <summary>
/// Creates the bitmap.
/// </summary>
/// <param name="size">The size.</param>
/// <param name="color">The color.</param>
/// <param name="pitch">The pitch.</param>
/// <param name="bitmap">The bitmap.</param>
/// <returns></returns>
auto LongUI::CUIResMgr::CreateBitmap(
    Size2U size, 
    const RGBA * color, 
    uint32_t pitch, 
    I::Bitmap *& bitmap) noexcept -> Result {
    assert(m_p2DRenderer);
    D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_NONE,
        D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );
    const auto code = m_p2DRenderer->CreateBitmap(
        auto_cast(size),
        color, pitch,
        &bitmapProperties,
        reinterpret_cast<ID2D1Bitmap1**>(&bitmap)
    );
    return{ code };
}


/// <summary>
/// Creates the bitmap from ss image file.
/// </summary>
/// <param name="utf8_file_name">Name of the UTF8 file.</param>
/// <param name="bitmap">The bitmap.</param>
/// <returns></returns>
auto LongUI::CUIResMgr::CreateBitmapFromSSImageFile(
    U8View view, I::Bitmap *& bitmap) noexcept -> Result {
    // TODO: 实现可以SEEK的流 以节约内存
    POD::Vector<uint8_t> buffer;
    UIManager.LoadDataFromUrl(view, buffer);
    if (const auto len = static_cast<uint32_t>(buffer.size())) {
        return CreateBitmapFromSSImageMemory(&buffer.front(), len, bitmap);
    }
    return Result{ Result::RE_FILENOTFOUND };
}

/// <summary>
/// Creates the bitmap from ss image memory.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <param name="len">The length.</param>
/// <param name="">The .</param>
/// <returns></returns>
auto LongUI::CUIResMgr::CreateBitmapFromSSImageMemory(
    uint8_t * ptr, 
    uint32_t len, 
    I::Bitmap *& bitmap) noexcept -> Result {
    return create_bitmap_private(ptr, len, reinterpret_cast<void*&>(bitmap));
}


PCN_NOINLINE
/// <summary>
/// Creates the bitmap private.
/// </summary>
/// <param name="file">The file.</param>
/// <param name="bitmap">The bitmap.</param>
/// <returns></returns>
auto LongUI::CUIResMgr::create_bitmap_private(
    uint8_t* ptr, uint32_t len, void *& bitmap) noexcept -> Result {
    // 渲染锁
    CUIRenderAutoLocker locker;
    // 创建位图
    struct create_bitmap_func {
        I::Bitmap*&     bitmap_ref;
        enum { ONLY1 = true };
        // 帧信息
        auto frame(uint32_t, uint32_t, Size2U) noexcept {
            return Result::RS_OK;
        }
        // 创建
        auto create(uint32_t,Size2U size,const uint8_t* color, uint32_t pitch) noexcept {
            const auto rgba = reinterpret_cast<const RGBA*>(color);
            auto& bitmap = this->bitmap_ref;
            //if (const auto file = std::fopen("output.raw", "wb")) {
            //    std::fwrite(rgba, 4, size.width * size.height, file);
            //    std::fclose(file);
            //}
            return UIManager.CreateBitmap(size, rgba, pitch, bitmap).code;
        }
    } obj{ reinterpret_cast<I::Bitmap*&>(bitmap) };
    return { impl::load_bitmap(obj, rm().wicfactroy, *rm().alpha, ptr, len) };
}



/// <summary>
/// Gets the default font.
/// </summary>
/// <returns></returns>
auto LongUI::CUIResMgr::GetDefaultFont()const noexcept->const FontArg& {
    return rm().defarg;
}

// ----------------------------------------------------------------------------
// ---------------------  CTL - Complex Text Layout  --------------------------
// ----------------------------------------------------------------------------

PCN_NOINLINE
/// <summary>
/// Creates the CTL text.
/// </summary>
/// <param name="arg">The argument.</param>
/// <param name="text">The text.</param>
/// <returns></returns>
auto LongUI::CUIResMgr::CreateCtlText(
    const TextArg& arg, I::Text *& text) noexcept -> Result {
    assert(text == nullptr && "check for releasing");
    IDWriteTextLayout* layout = nullptr;
    // 创建基本布局
    Result hr = { rm().dwritefactroy->CreateTextLayout(
        detail::sys(arg.string),
        static_cast<uint32_t>(arg.length),
        arg.font ? arg.font : rm().deffont,
        arg.mwidth,
        arg.mheight,
        &layout
    ) };
    longui_debug_hr(hr, L"CreateTextLayout faild");
#if 1
    text = static_cast<I::Text*>(layout);
#else
    // 查询继承类接口
    if (hr) {
        hr = { layout->QueryInterface(
            IID_IDWriteTextLayout1,
            reinterpret_cast<void**>(&text)
        ) };
        longui_debug_hr(hr, L"QueryInterface 'IDWriteTextLayout1' faild");
    // 释放数据
    if (layout) layout->Release();
#endif
    return hr;
}

PCN_NOINLINE
/// <summary>
/// Creates the control font.
/// </summary>
/// <param name="arg">The argument.</param>
/// <param name="font">The font.</param>
/// <returns></returns>
auto LongUI::CUIResMgr::CreateCtlFont(const FontArg& arg, 
    I::Font *& font, const StyleText* text) noexcept -> Result {
    // 使用默认的?
    assert(arg.family && "NOT IMPL");
    // 字体名称用缓冲区
    constexpr size_t buflen = 128;
    char16_t buffer[buflen];
    static_assert(sizeof(wchar_t) == sizeof(char16_t), "must be same");
    // 转换
    Unicode::To<Unicode::UTF16>(buffer, buflen, arg.family);
    // 创建文本格式
    Result hr = { rm().dwritefactroy->CreateTextFormat(
        reinterpret_cast<const wchar_t*>(buffer),
        // TODO: 字体集
        nullptr,
        auto_cast(arg.weight),
        auto_cast(arg.style),
        auto_cast(arg.stretch),
        arg.size,
        detail::sys(m_szLocaleName),
        reinterpret_cast<IDWriteTextFormat**>(&font)
    ) };
    // 直接断言方便检查
    assert(hr);
    // 设置
    if (hr) {
        // 初始
        const auto format = font; auto tmp = hr;
        // 设置自动换行
        tmp = { format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP) };
        longui_debug_hr(tmp, L"failed format->SetWordWrapping - NO_WRAP" );
        // 有效样式
        if (text) {
            // 设置 Tab宽度
            //tmp = { format->SetIncrementalTabStop(arg.tab == 0.f ? arg.size * 4.f : arg.tab) };
            //longui_debug_hr(tmp, L"failed format->SetIncrementalTabStop  " << arg.tab);
            // 设置段落排列方向
            //tmp = format->SetFlowDirection(static_cast<DWRITE_FLOW_DIRECTION>(args.flow));
            //longui_debug_hr(tmp, L"failed format->SetFlowDirection  " << int32_t(args.flow));
            // 设置段落(垂直)对齐
            //tmp = format->SetParagraphAlignment(static_cast<DWRITE_PARAGRAPH_ALIGNMENT>(args.valign));
            //longui_debug_hr(tmp, L"failed format->SetParagraphAlignment  " << int32_t(args.valign));
            // 设置文本(水平)对齐
            tmp = { format->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(text->alignment)) };
            longui_debug_hr(tmp, L"failed format->SetTextAlignmen  t" << int32_t(text->alignment));
            // 设置阅读进行方向
            //tmp = format->SetReadingDirection(static_cast<DWRITE_READING_DIRECTION>(args.reading));
            //longui_debug_hr(tmp, L"failed format->SetReadingDirection  " << int32_t(args.reading));
        }
        // 设置行高度
        const auto lh = LongUI::GetLineHeight(arg);
        const auto bl = lh * 0.85f;
        tmp = { format->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_UNIFORM, lh, bl) };
        longui_debug_hr(tmp, L"failed format->SetLineSpacing: " << lh);
    }
    return hr;
}

// ----------------------------------------------------------------------------
// -------------------------------  Graphics  ---------------------------------
// ----------------------------------------------------------------------------

PCN_NOINLINE
/// <summary>
/// References the cc brush.
/// </summary>
/// <param name="color">The color.</param>
/// <returns></returns>
auto LongUI::CUIResMgr::RefCCBrush(
    const ColorF& color) noexcept -> I::Brush& {
    const auto comb = UIManager.m_pCommonBrush;
    ID2D1Brush* const brush = comb;
    const auto scb = static_cast<ID2D1SolidColorBrush*>(brush);
    scb->SetColor(auto_cast(color));
    return *comb;
}

/// <summary>
/// get 2d factory
/// </summary>
/// <returns></returns>
auto LongUI::CUIResMgr::Ref2DFactory() noexcept -> int& {
    CUIResMgr& manager = UIManager;
    auto& factory = *manager.rm().d2dfactroy;
    return reinterpret_cast<int&>(factory);
}

// DXGI DEBUG
#ifndef NDEBUG

#include <core/ui_object.h>
#include <dxgidebug.h>
/// <summary>
/// debug data
/// </summary>
struct LongUI::CUIResMgr::Debug : LongUI::CUIObject {
    // ctor
    Debug() noexcept;
    // dtor
    ~Debug() noexcept;
    // debug dll
    HMODULE             dxgidebug_dll = nullptr;
    // dxgi debug interface
    IDXGIDebug*         dxgidebug = nullptr;
    // d3d11 debug interface
    ID3D11Debug*        d3d11debug = nullptr;
};

/// <summary>
/// Initializes a new instance of the <see cref="Debug"/> struct.
/// </summary>
LongUI::CUIResMgr::Debug::Debug() noexcept {
    // 载入调试DLL文件
    dxgidebug_dll = ::LoadLibraryA("dxgidebug.dll");
    // 未找到
    if (!dxgidebug_dll) return;
    // 定义接口GUID
    const GUID local_IID_IDXGIDebug = {
        0x119E7452, 0xDE9E, 0x40fe, 0x88, 0x06, 0x88,
        0xF9, 0x0C, 0x12, 0xB4, 0x41
    };
    // 万能union转换
    const auto name = "DXGIGetDebugInterface";
    union {
        HRESULT(WINAPI* func) (REFIID riid, void **ppDebug) noexcept;
        FARPROC     addr;
    };
    // 加载函数
    if ((addr = ::GetProcAddress(dxgidebug_dll, name))) {
        const auto code = func(
            local_IID_IDXGIDebug,
            reinterpret_cast<void**>(&dxgidebug)
        );
    }
}

/// <summary>
/// Finalizes an instance of the <see cref="Debug"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIResMgr::Debug::~Debug() noexcept {
    const GUID debug_all = {
        0xe48ae283, 0xda80, 0x490b, 0x87, 0xe6,
        0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8
    };
    if (dxgidebug) {
        ::OutputDebugStringW(L"----> ALL ReportLiveObjects\r\n");
        dxgidebug->ReportLiveObjects(
            debug_all,
            DXGI_DEBUG_RLO_ALL
        );
        ::OutputDebugStringW(L"<----\r\n");
        dxgidebug->Release();
    }
    if (dxgidebug) ::FreeLibrary(dxgidebug_dll);
    assert(!d3d11debug && "must be nullptr");
}


#endif


//#pragma interface
/// <summary>
/// screen iterface
/// </summary>
struct PCN_NOVTABLE LongUI::CUIResMgr::IScreen : IDXGIOutput {};
//#pragma implementation

namespace LongUI {
    // impl
    namespace impl {
        // create native style renderer
        auto create_native_style_renderer() noexcept -> void*;
        // delete native style renderer
        void delete_native_style_renderer(void* ptr) noexcept;
        // recreate 
        auto recreate_native_style_renderer(void*ptr)noexcept->Result;
    }
}

/// <summary>
/// Waits for vblank.
/// </summary>
/// <returns></returns>
bool LongUI::CUIResMgr::wait_for_vblank() noexcept {
    // 存在显示输出?
    if (!m_pMainScreen) return false;
    // 等待垂直同步
    m_pMainScreen->WaitForVBlank();
    return true;
}



/// <summary>
/// Initializes a new instance of the <see cref="CUIResMgr" /> class.
/// </summary>
/// <param name="out">The out hr.</param>
LongUI::CUIResMgr::CUIResMgr(IUIConfigure* cfg, Result& out) noexcept {
    if (!out) return;
    // 初始化私有数据
    std::memset(&m_private, 0, sizeof(m_private));
#ifndef NDEBUG
    m_pDebug = new(std::nothrow) Debug;
    if (!m_pDebug) { 
        out = { Result::RE_OUTOFMEMORY };
        return;
    }
#endif
    // 初始化一些东西
    m_szLocaleName[0] = L'\0';
    // 本地字符集名称
    cfg->GetLocaleName(m_szLocaleName);
    // 创建rm
    detail::ctor_dtor<CUIResMgr::Private>::create(&rm());
    // 初始化无关资源
    Result hr = rm().init();
    // 创建本地风格渲染器
    if (hr) {
        m_pNativeStyle = impl::create_native_style_renderer();
        if (!m_pNativeStyle) hr = { Result::RE_OUTOFMEMORY };
    }
    // 返回结果
    out = hr;
}


/// <summary>
/// Initializes the default font data.
/// </summary>
/// <returns></returns>
auto LongUI::CUIResMgr::init_default_font(IUIConfigure* cfg) noexcept->Result {
    // 配置默认字体
    auto& arg = rm().defarg;
    // 获取配置的默认字体
    cfg->DefaultFontArg(arg);
    // 创建默认字体
    return this->CreateCtlFont(arg, rm().deffont);
}

/// <summary>
/// Finalizes an instance of the <see cref="CUIResMgr"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIResMgr::~CUIResMgr() noexcept {
    // 释放渲染器
    impl::delete_native_style_renderer(m_pNativeStyle);
    // 释放设备资源
    this->release_device();
    // 释放资源列表
    this->release_res_list();
    // 释放无关资源
    rm().release();
    // 调用析构函数
    rm().~Private();
#ifndef NDEBUG
    delete m_pDebug;
#endif
}


/// <summary>
/// Recreates this instance.
/// </summary>
/// <param name="cfg">The CFG.</param>
/// <param name="flag">The flag.</param>
/// <returns></returns>
auto LongUI::CUIResMgr::recreate_device(IUIConfigure* cfg, ConfigureFlag flag) noexcept -> Result {
    constexpr auto same_s = sizeof(CUIResMgr::Private) == sizeof(private_t);
    constexpr auto same_a = alignof(CUIResMgr::Private) == alignof(private_t);
    this->release_device();
    // 待用适配器
    IDXGIAdapter1* adapter = nullptr;
    // 枚举显示适配器
    if (!(flag & ConfigureFlag::Flag_RenderByCPU)) {
        IDXGIFactory1* dxgifactory = nullptr;
        const auto idd = IID_IDXGIFactory1;
        const auto add = reinterpret_cast<void**>(&dxgifactory);
        // 创建一个临时工厂
        const Result tmp = { ::CreateDXGIFactory1(idd, add) };
        // 第一次失败直接返回
        if (!tmp) return tmp;
        IDXGIAdapter1* apAdapters[MAX_GRAPHICS_ADAPTERS];
        GraphicsAdapterDesc gas[MAX_GRAPHICS_ADAPTERS];
        uint32_t adnum = 0;
        // 枚举适配器
        for (adnum = 0; adnum < MAX_GRAPHICS_ADAPTERS; ++adnum) {
            constexpr HRESULT nf = DXGI_ERROR_NOT_FOUND;
            if (dxgifactory->EnumAdapters1(adnum, apAdapters + adnum) == nf) {
                break;
            }
            DXGI_ADAPTER_DESC1 desc;
            apAdapters[adnum]->GetDesc1(&desc);
            static_assert(sizeof(desc.Description) <= sizeof(gas->friend_name), "SMALL!");
            std::memcpy(gas[adnum].friend_name, desc.Description, sizeof(desc.Description));
            gas[adnum].shared_system = desc.SharedSystemMemory;
            gas[adnum].dedicated_video = desc.DedicatedVideoMemory;
            gas[adnum].dedicated_system = desc.DedicatedSystemMemory;
        }
        // 选择适配器
        const auto index = cfg->ChooseAdapter(gas, adnum);
        if (index < adnum) {
            adapter = LongUI::SafeAcquire(apAdapters[index]);
        }
        // 释放适配器
        for (size_t i = 0; i < adnum; ++i) {
            LongUI::SafeRelease(apAdapters[i]);
        }
        LongUI::SafeRelease(dxgifactory);
    }
    Result hr = { Result::RS_OK };
    // 创建 D3D11设备与设备上下文 
    if (hr) {
        // D3D11 创建flag 
        // 一定要有D3D11_CREATE_DEVICE_BGRA_SUPPORT
        // 否则创建D2D设备上下文会失败
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if !defined(NDEBUG)
        // Debug状态 有D3D DebugLayer就可以取消注释
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        const D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };
        constexpr uint32_t fl_size = sizeof(featureLevels) / sizeof(featureLevels[0]);
        // 根据情况检查驱动类型
        const auto dtype = flag & ConfigureFlag::Flag_RenderByCPU ? 
            D3D_DRIVER_TYPE_WARP : (adapter ? 
                D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE);
        D3D_DRIVER_TYPE types[] = { dtype, D3D_DRIVER_TYPE_WARP };
        HRESULT tmp;
        // 两次尝试
        for (auto type : types) {
            ID3D11Device* dev = nullptr;
            ID3D11DeviceContext* ctx = nullptr;
            // 创建设备
            tmp = ::D3D11CreateDevice(
                // 设置为渲染
                adapter,
                // 驱动类型
                dtype,
                // 没有软件接口
                nullptr,
                // 创建flag
                creationFlags,
                // 欲使用的特性等级列表
                featureLevels,
                // 特性等级列表长度
                fl_size,
                // SDK 版本
                D3D11_SDK_VERSION,
                // 返回的D3D11设备指针
                &dev,
                // 返回的特性等级
                nullptr,
                // 返回的D3D11设备上下文指针
                &ctx
            );
            m_p3DDevice = static_cast<I::Device3D*>(dev);
            m_p3DRenderer = static_cast<I::Renderer3D*>(ctx);
            // 成功就退出
            if (SUCCEEDED(tmp)) break;
        }
        // 再次检查错误
        if (FAILED(tmp)) {
            hr = { tmp };
            LUIDebug(Error) 
                << " create d3d11-device in twice, but failed."
                << LongUI::endl;
        }
    }
    // 释放选择的图像适配器
    LongUI::SafeRelease(adapter);
    // 创建 ID3D11Debug对象
#if !defined(NDEBUG)
    if (hr) {
        const Result tmp = { m_p3DDevice->QueryInterface(
            IID_ID3D11Debug,
            reinterpret_cast<void**>(&m_pDebug->d3d11debug)
        ) };
        longui_debug_hr(tmp, L"m_pd3dDevice->QueryInterface(m_pd3dDebug) faild");
    }
#endif
    // TODO: MMF
    IDXGIDevice1* dxgidev = nullptr;
    ID2D1Device* device2d = nullptr;
    IDXGIAdapter* dxgiadapter = nullptr;
    // 创建 IDXGIDevice
    if (hr) {
        hr = { m_p3DDevice->QueryInterface(
           IID_IDXGIDevice1,
           reinterpret_cast<void**>(&dxgidev)
        ) };
        longui_debug_hr(Result{ hr }, L"m_pd3dDevice->QueryInterface(m_pd3dDebug) faild");
    }
    // 创建 D2D设备
    if (hr) {
        hr = { rm().d2dfactroy->CreateDevice(dxgidev, &device2d) };
        longui_debug_hr(Result{ hr }, L"d2dfactroy->CreateDevice faild");
    }
    // 创建 D2D设备上下文
    if (hr) {
        ID2D1DeviceContext* d2ddc = nullptr;
        hr = { device2d->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            &d2ddc
        ) };
        m_p2DRenderer = static_cast<I::Renderer2D*>(d2ddc);
        longui_debug_hr(Result{ hr }, L"device2d->CreateDeviceContext faild");
    }
    // 创建公共颜色笔刷
    if (hr) {
        hr = { m_p2DRenderer->CreateSolidColorBrush(
            D2D1_COLOR_F{},
            reinterpret_cast<ID2D1SolidColorBrush**>(&m_pCommonBrush)
        ) };
    }
    // 获取 Dxgi适配器 可以获取该适配器信息
    if (hr) {
        // 顺带使用像素作为单位
        m_p2DRenderer->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
        // 获取适配器
        hr = { dxgidev->GetAdapter(&dxgiadapter) };
        longui_debug_hr(Result{ hr }, L"m_pDxgiDevice->GetAdapter faild");
    }
#ifndef NDEBUG
    // 输出显卡信息
    if (hr) {
        DXGI_ADAPTER_DESC desc = { 0 };
        dxgiadapter->GetDesc(&desc);
        GraphicsAdapterDesc d;
        std::memcpy(d.friend_name, desc.Description, sizeof(desc.Description));
        d.shared_system = desc.SharedSystemMemory;
        d.dedicated_video = desc.DedicatedVideoMemory;
        d.dedicated_system = desc.DedicatedSystemMemory;
        LUIDebug(Hint) << d << LongUI::endl;
    }
#endif
    // 获取 Dxgi工厂
    if (hr) {
        IDXGIFactory2* fc = nullptr;
        hr = { dxgiadapter->GetParent(
            LongUI::IID_IDXGIFactory2,
            reinterpret_cast<void**>(&fc)
        ) };
        m_pGraphicsFactory = static_cast<I::FactoryGraphics*>(fc);
    }
    // 重定向主显示器
    if (hr) {
        this->redirect_screen();
    }
    // 释放选择的DXGI设备
    LongUI::SafeRelease(dxgidev);
    LongUI::SafeRelease(device2d);
    LongUI::SafeRelease(dxgiadapter);
    return hr;
}


/// <summary>
/// Redirects the screen.
/// </summary>
/// <returns></returns>
void LongUI::CUIResMgr::redirect_screen() noexcept {
    LongUI::SafeRelease(m_pMainScreen);
    assert(m_pGraphicsFactory && "bad action");
    // 初始化
    IDXGIAdapter1* adapter = nullptr;
    UINT ia = 0;
    // 枚举适配器
    while (m_pGraphicsFactory->EnumAdapters1(ia, &adapter) != DXGI_ERROR_NOT_FOUND) {
        assert(adapter && "bad action");
#ifndef NDEBUG
        DXGI_ADAPTER_DESC1 desca;
        adapter->GetDesc1(&desca);
#endif
        IDXGIOutput* output;
        // 枚举显示输出
        while (adapter->EnumOutputs(0, &output) != DXGI_ERROR_NOT_FOUND) {
            DXGI_OUTPUT_DESC desco;
            output->GetDesc(&desco);
            //LongUI::GetMonitorDpi(desco.Monitor, m_uMainDpiX, m_uMainDpiY);
            m_pMainScreen = static_cast<IScreen*>(output);
            adapter->Release();
            return;
        }
        ++ia;
        adapter->Release();
    }
    // 检查
    assert(!adapter && "bad action");
}

// ----------------------------------------------------------------------------
// -----------------------  SR - Shared Resource  -----------------------------
// ----------------------------------------------------------------------------


/// <summary>
/// Releases the resource list.
/// </summary>
/// <returns></returns>
void LongUI::CUIResMgr::release_res_list() noexcept {
    // 释放未正确释放的资源(资源泄漏)
    auto& map = this->rm().resmap;
    const auto begin_itr = map.begin();
    const auto end_itr = map.end();
    for (auto itr = begin_itr; itr != end_itr; ++itr) {
#if 0
        {
#else
        if (itr->second) {
#endif
            assert(itr->second);
            const auto ptr = CUIResourceID::Object(itr->second);
            auto& img = static_cast<CUIImage&>(*ptr);
            assert(ptr->RefData().GetType() == ResourceType::Type_Image);
#ifndef NDEBUG
            LUIDebug(Error)
                << "resource non-released:(ref: "
                << ptr->RefData().ref
                << ", type: "
                << static_cast<int>(ptr->RefData().GetType())
                << ")" << endl;
#endif
            img.Destroy();
        }
    }
}

/// <summary>
/// Recreates the resource.
/// </summary>
/// <returns></returns>
auto LongUI::CUIResMgr::recreate_resource() noexcept -> Result {
    Result rv = { Result::RS_OK };
    // 重建位图仓
    rm().bitbank.Recreate();
    // 重建, 即便错误也要继续, 目的是释放数据
    {
        const auto begin_itr = rm().resmap.begin();
        const auto end_itr = rm().resmap.end();
        for (auto itr = begin_itr; itr != end_itr; ++itr) {
            assert(itr->second);
            /*if (itr->second)*/ {
                const auto ptr = CUIResourceID::Object(itr->second);
                auto& img = static_cast<CUIImage&>(*ptr);
                assert(ptr->RefData().GetType() == ResourceType::Type_Image);
#ifdef LUI_MULTIPLE_RESOURCE
#endif
                // 即便错误也要继续, 目的是释放数据
                img.Release();
                if (rv) rv = rm().recreate(img);
            }
        }
    }
    // 正常重建,  即便错误也要继续, 目的是释放数据
    const auto naive = impl::recreate_native_style_renderer(m_pNativeStyle);
    if (!naive) rv = naive;
    return rv;
}


/// <summary>
/// Releases this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUIResMgr::release_device() noexcept {
#ifndef NDEBUG
    const auto d3d = m_p3DDevice;
#endif
#ifndef LUI_DISABLE_STYLE_SUPPORT
    Effect::ReleaseBackImage();
    Effect::ReleaseBorderImage();
#endif
    //if (m_p3DRenderer) {
    //    m_p3DRenderer->ClearState();
    //    m_p3DRenderer->Flush();
    //}
    LongUI::SafeRelease(m_pMainScreen);
    LongUI::SafeRelease(m_pCommonBrush);
    LongUI::SafeRelease(m_p2DRenderer);
    LongUI::SafeRelease(m_p3DRenderer);
    LongUI::SafeRelease(m_p3DDevice);
    LongUI::SafeRelease(m_pGraphicsFactory);
#ifndef NDEBUG
    if (auto inf = m_pDebug->d3d11debug) {
        ::OutputDebugStringW(L"----> SUMMARY ReportLiveDeviceObjects\r\n");
        inf->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
        ::OutputDebugStringW(L"<----\r\n");
        if (false) {
            ::OutputDebugStringW(L"----> DETAIL ReportLiveDeviceObjects\r\n");
            inf->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
            ::OutputDebugStringW(L"<----\r\n");
        }
        auto c = inf->Release();
        c = c;
        m_pDebug->d3d11debug = nullptr;
    }
#endif
}


namespace LongUI { namespace detail {
    // add ref res via id
    void add_ref(uintptr_t handle) noexcept;
    // image to id
    auto xul_image_to_id(U8View view) noexcept -> uintptr_t {
        const auto type = ResourceType::Type_Image;
        const auto id = UIManager.LoadResource(view, /*type,*/ true);
        if (id) detail::add_ref(id);
        return id;
    }
}}

