#include <luiconf.h>
// ui
#include <graphics/ui_adapter_desc.h>
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
#include <util/ui_unicode_cast.h>
// Effect
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
        0x9064d822, 0x80a7, 0x465c,
        { 0xa9, 0x86, 0xdf, 0x65, 0xf7, 0x8b, 0x8f, 0xeb }
    };
    // impl namespace
    namespace impl { 
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
        // save bitmap
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
                longui_debug_hr(Result{ hr }, L"failed: args.factory->CreateBitmapFromMemory");
            }
            // 创建流
            if (SUCCEEDED(hr)) {
                hr = args.factory->CreateStream(&stream);
                longui_debug_hr(Result{ hr }, L"failed: args.factory->CreateStream");
            }
            // 从文件初始化
            if (SUCCEEDED(hr)) {
                hr = stream->InitializeFromFilename(file_name, GENERIC_WRITE);
                longui_debug_hr(Result{ hr }, L"failed: stream->InitializeFromFilename  " << file_name);
            }
            // 创建编码器
            if (SUCCEEDED(hr)) {
                hr = args.factory->CreateEncoder(
                    args.container_format ? *args.container_format : GUID_ContainerFormatPng,
                    nullptr,
                    &pEncoder
                );
                longui_debug_hr(Result{ hr }, L"failed: args.factory->CreateEncoder");
            }
            // 初始化编码器
            if (SUCCEEDED(hr)) {
                hr = pEncoder->Initialize(stream, WICBitmapEncoderNoCache);
                longui_debug_hr(Result{ hr }, L"failed: pEncoder->Initialize");
            }
            // 创建新的一帧
            if (SUCCEEDED(hr)) {
                hr = pEncoder->CreateNewFrame(&frame_encode, nullptr);
                longui_debug_hr(Result{ hr }, L"failed: pEncoder->CreateNewFrame");
            }
            // 初始化帧编码器
            if (SUCCEEDED(hr)) {
                hr = frame_encode->Initialize(nullptr);
                longui_debug_hr(Result{ hr }, L"failed: frame_encode->Initialize");
            }
            // 设置大小
            if (SUCCEEDED(hr)) {
                hr = frame_encode->SetSize(args.width, args.height);
                longui_debug_hr(Result{ hr }, L"failed: frame_encode->SetSize " << long(args.width) << L", " << long(args.height));
            }
            // 设置格式
            WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
            if (SUCCEEDED(hr)) {
                hr = frame_encode->SetPixelFormat(&format);
                longui_debug_hr(Result{ hr }, L"failed: frame_encode->SetPixelFormat");
            }
            // 写入源数据
            if (SUCCEEDED(hr)) {
                hr = frame_encode->WriteSource(wic_bitmap, nullptr);
                longui_debug_hr(Result{ hr }, L"failed: frame_encode->WriteSource");
            }
            // 提交帧编码器
            if (SUCCEEDED(hr)) {
                hr = frame_encode->Commit();
                longui_debug_hr(Result{ hr }, L"failed: frame_encode->Commit");
            }
            // 提交编码
            if (SUCCEEDED(hr)) {
                hr = pEncoder->Commit();
                longui_debug_hr(Result{ hr }, L"failed: pEncoder->Commit");
            }
            // 扫尾处理
            LongUI::SafeRelease(wic_bitmap);
            LongUI::SafeRelease(stream);
            LongUI::SafeRelease(frame_encode);
            LongUI::SafeRelease(pEncoder);
            // 返回结果
            return hr;
        }
        // load bitmap
        template<typename Lambda> auto load_bitmap(
            Lambda create,
            IWICImagingFactory* pIWICFactory,
            HANDLE file,
            I::Bitmap*&bitmap
        ) noexcept -> HRESULT {
            IWICBitmapDecoder *decoder = nullptr;
            IWICBitmapFrameDecode *source = nullptr;
            IWICFormatConverter *converter = nullptr;
            // 创建解码器
            HRESULT hr = pIWICFactory->CreateDecoderFromFileHandle(
                reinterpret_cast<ULONG_PTR>(file),
                nullptr,
                WICDecodeMetadataCacheOnLoad,
                &decoder
            );
            // 获取第一帧
            if (SUCCEEDED(hr)) {
                hr = decoder->GetFrame(0, &source);
            }
            // 创建格式转换器
            if (SUCCEEDED(hr)) {
                hr = pIWICFactory->CreateFormatConverter(&converter);
            }
            // 直接初始化
            if (SUCCEEDED(hr)) {
                hr = converter->Initialize(
                    source,
                    GUID_WICPixelFormat32bppPRGBA,
                    WICBitmapDitherTypeNone,
                    nullptr,
                    0.f,
                    WICBitmapPaletteTypeMedianCut
                );
            }
            Size2U size{0};
            uint8_t* ptr = nullptr;
            // 获取数据
            if (SUCCEEDED(hr)) {
                hr = converter->GetSize(&size.width, &size.height);
            }
            const auto sizeof_rgba = static_cast<uint32_t>(sizeof(RGBA));
            const auto bylen = size.width * size.height * sizeof_rgba;
            const auto bypch = size.width * sizeof_rgba;
            // 申请空间
            if (SUCCEEDED(hr)) {
                ptr = LongUI::NormalAllocT<uint8_t>(bylen);
                if (!ptr) hr = E_OUTOFMEMORY;
            }
            // 复制数据
            if (SUCCEEDED(hr)) {
                hr = converter->CopyPixels(nullptr, bypch, bylen, ptr);
            }
            // 创建位图
            if (SUCCEEDED(hr)) {
                hr = create(size, ptr, bypch, bitmap);
            }
            LongUI::NormalFree(ptr);
            LongUI::SafeRelease(converter);
            LongUI::SafeRelease(source);
            LongUI::SafeRelease(decoder);
            return hr;
        };
    }
}

/// <summary>
/// private data/func for resmgr
/// </summary>
struct LongUI::PrivateResMgr {
    // ctor
    PrivateResMgr() noexcept {}
    // dtor
    ~PrivateResMgr() noexcept {}
    // resource list
    using ResourceList = POD::Vector<ResourceData>;
    // resource map
    using ResourceMap = POD::HashMap<uint32_t>;
    // init
    auto init() noexcept->Result;
    // recreate_device
    auto recreate() noexcept->Result;
    // release
    void release() noexcept;
    // push index0
    auto push_index0_res() noexcept ->Result;
    // d2d factroy
    ID2D1Factory1*      d2dfactroy;
    // dwrite factroy
    IDWriteFactory1*    dwritefactroy;
    // wic factroy
    IWICImagingFactory* wicfactroy;
    // default font
    I::Font*            deffont;
    // resource list
    ResourceList        reslist;
    // resource data
    ResourceMap         resmap;
    // default font
    FontArg             defarg;
    // resource count
    uint32_t            rescount;
};


// ----------------------------------------------------------------------------
// ---------------------------  Private Manager  ------------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
inline auto LongUI::PrivateResMgr::recreate() noexcept -> Result {
    assert(!"NOT IMPL");
    return{};
}

/// <summary>
/// Pushes the index0 resource.
/// </summary>
/// <returns></returns>
auto LongUI::PrivateResMgr::push_index0_res() noexcept ->Result {
    assert(this->reslist.empty() && "bad list");
    reslist.reserve(128);
    // 内存不足
    if (!reslist.is_ok()) return { Result::RE_OUTOFMEMORY };
    // 资源数据
    const auto data = ResourceData {
        nullptr,
        ":lui/index0",
        0,
        ResourceType::Type_Custom
    };
    this->rescount++;
    reslist.push_back(data);
    return { Result::RS_OK };
}

/// <summary>
/// Initializes this instance.
/// </summary>
/// <returns></returns>
inline auto LongUI::PrivateResMgr::init() noexcept -> Result {
    // 设置默认字体
    this->defarg = { 
        "Arial", 
        12.5f, 1.2f, 0.f, 
        Weight_Normal, Style_Normal, Stretch_Normal
    };
    Result hr = { Result::RS_OK };
    // 创建 WIC 工厂.
    if (hr) {
        hr = { ::CoCreateInstance(
            CLSID_WICImagingFactory1,
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
    if (hr) {
        hr = Effect::RegisterBorderImage(this->d2dfactroy);
    }
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

/// <summary>
/// Releases this instance.
/// </summary>
/// <returns></returns>
inline void LongUI::PrivateResMgr::release() noexcept {
    LongUI::SafeRelease(this->d2dfactroy);
    LongUI::SafeRelease(this->dwritefactroy);
    LongUI::SafeRelease(this->wicfactroy);
    LongUI::SafeRelease(this->deffont);
}

// ----------------------------------------------------------------------------
// -----------------------  SR - Shared Resource  -----------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Gets the resourece data.
/// </summary>
/// <param name="id">The identifier.</param>
/// <returns></returns>
auto LongUI::CUIResMgr::GetResoureceData(
    uint32_t id) const noexcept -> const ResourceData& {
    auto& list = rm().reslist;
    assert(id && "id cannot be 0");
    assert(id < list.size() && "id out of range");
    return list[id];
}

/// <summary>
/// Adds the resource reference count.
/// </summary>
/// <param name="id">The identifier.</param>
/// <returns></returns>
void LongUI::CUIResMgr::AddResourceRefCount(uint32_t id) noexcept {
    auto& list = rm().reslist;
    assert(id < list.size() && "out of range");
    assert(list[id].obj && "object not found");
    list[id].ref++;
}

/// <summary>
/// Releases the resource reference count.
/// </summary>
/// <param name="id">The identifier.</param>
/// <returns></returns>
void LongUI::CUIResMgr::ReleaseResourceRefCount(uint32_t id) noexcept {
    // 资源数量断言
    auto& counter = rm().rescount;
    assert(counter > 1 && "bad counter");
    // 资源列表断言
    auto& list = rm().reslist;
    assert(id < list.size() && "out of range");
    assert(list[id].obj && "object not found");
    // 资源数据断言
    auto& data = list[id];
    assert(data.ref && "cannot release count 0");
    // 引用计数归为0
    if (--data.ref == 0) {
        // 则释放数据
        data.obj->Destroy();
        // 字符映射表中删除
        const auto itr = rm().resmap.find(data.uri);
        assert(itr != rm().resmap.end());
        itr->second = 0;
        // 清空数据
        data.obj = nullptr;
        //data.uri = nullptr;
        //data.type = ResourceType::Type_Custom;
        // 减少资源数量
        counter--;
    }
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
/// Loads the resource.
/// </summary>
/// <param name="uri">The URI.</param>
/// <param name="type">The type.</param>
/// <param name="is_xul_dir">if set to <c>true</c> [is xul dir].</param>
/// <returns></returns>
auto LongUI::CUIResMgr::LoadResource(
    U8View uri, 
    ResourceType type, 
    bool is_xul_dir) noexcept -> uint32_t {
    // 没有就算了
    if (uri.end() == uri.begin()) return 0;
    // 待用数据
    PathOP::UriPath path_buf;
    auto& list = rm().reslist;
    auto& map = rm().resmap;
    auto get_dir = [is_xul_dir]() noexcept {
        return is_xul_dir ? UIManager.GetXULDir() : U8View{};
    };
    const auto nsize = static_cast<uint32_t>(list.size());
    assert(nsize >= rm().rescount && "???");
    auto index = nsize;
    // 复用已有的: 如果超过 3/4就直接末尾添加
    if (rm().rescount >= nsize * 3 / 4 ) {
        list.resize(list.size() + 1);
        if (!list.is_ok()) return 0;
    }
    // 否则就复用已有的
    else {
        for (index = 1; index != nsize; ++index) {
            if (list[index].obj == nullptr) break;
        }
        assert(index != nsize && "not found");
    }
    // 转换统一URI字符串
    uri = PathOP::MakeUriPath(path_buf, get_dir(), uri);
    // 检测问题
    if (uri.end() == uri.begin()) { assert(!"ERROR"); return 0; }
    // 插入URL
    const auto re = map.insert(uri.begin(), uri.end(), index);
    // 内存不足
    if (re.first == map.end()) { assert(!"ERROR"); return 0; }
    // 插入失败 而且 数据不为0就是已有的
    if (!re.second && re.first->second) {
        const auto id = re.first->second;
        assert(type == list[id].type && "must be same");
        return id;
    }
    // -----------------------------------------
    // 待插入数据
    ResourceData data { nullptr, re.first->first, 0, type };
    // 创建位图
    I::Bitmap* bitmap = nullptr;
    auto hr = this->CreateBitmapFromSSImageFile(data.uri, bitmap);
    // 创建IMAGE资源
    if (hr) {
        hr = CUIImage::CreateImage(*bitmap, luiref data.obj);
    }
    // 推入表中
    if (hr) {
        list[index] = data;
        ++rm().rescount;
    }
    // TODO: 错误处理(信息丢失)
    const auto rvcode = hr ? index : 0;
    // 写入目前的位置
    re.first->second = rvcode;
    // 返回
    return rvcode;
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
    const char* utf8_file_name, I::Bitmap *& bitmap) noexcept -> Result {
#ifndef NDEBUG
    LUIDebug(Hint) << "load file: " << utf8_file_name << endl;
#endif
    CUIFile file{ utf8_file_name, CUIFile::Flag_Read };
    if (!file) return Result::GetSystemLastError();
    const auto handle = reinterpret_cast<void*>(file.GetHandle());
    void*& refbmp = reinterpret_cast<void*&>(bitmap);
    return create_bitmap_private(handle, refbmp);
}

/// <summary>
/// Creates the bitmap from ss image memory.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <param name="len">The length.</param>
/// <param name="">The .</param>
/// <returns></returns>
auto LongUI::CUIResMgr::CreateBitmapFromSSImageMemory(
    const void * ptr, 
    size_t len, 
    I::Bitmap *& bitmap) noexcept -> Result {
    assert(!"NOT IMPL");
    return{ Result::RE_NOTIMPL };
}

PCN_NOINLINE
/// <summary>
/// Creates the bitmap private.
/// </summary>
/// <param name="file">The file.</param>
/// <param name="bitmap">The bitmap.</param>
/// <returns></returns>
auto LongUI::CUIResMgr::create_bitmap_private(
    void * file, void *& bitmap) noexcept -> Result {
    // 创建位图
    auto create_bitmap = [this](
        Size2U size, 
        const uint8_t* color, 
        uint32_t pitch, 
        I::Bitmap*& bitmap
        ) noexcept {
        const auto rgba = reinterpret_cast<const RGBA*>(color);
        return this->CreateBitmap(size, rgba, pitch, bitmap).code;
    };
    return{
        impl::load_bitmap(
            create_bitmap,
            rm().wicfactroy,
            reinterpret_cast<HANDLE>(file),
            reinterpret_cast<I::Bitmap*&>(bitmap)
            )
    };
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
        arg.string,
        static_cast<uint32_t>(arg.length),
        arg.font ? arg.font : rm().deffont,
        arg.mwidth,
        arg.mheight,
        &layout
    ) };
    longui_debug_hr(Result{ hr }, L"CreateTextLayout faild");
    // 查询继承类接口
    if (hr) {
        hr = { layout->QueryInterface(
            IID_IDWriteTextLayout1,
            reinterpret_cast<void**>(&text)
        ) };
        longui_debug_hr(Result{ hr }, L"QueryInterface 'IDWriteTextLayout1' faild");
    }
    // 释放数据
    if (layout) layout->Release();
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
        m_szLocaleName,
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
            //longui_debug_hr(tmp, L"failed format->SetFlowDirection  " << long(args.flow));
            // 设置段落(垂直)对齐
            //tmp = format->SetParagraphAlignment(static_cast<DWRITE_PARAGRAPH_ALIGNMENT>(args.valign));
            //longui_debug_hr(tmp, L"failed format->SetParagraphAlignment  " << long(args.valign));
            // 设置文本(水平)对齐
            tmp = { format->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(text->alignment)) };
            longui_debug_hr(tmp, L"failed format->SetTextAlignmen  t" << long(text->alignment));
            // 设置阅读进行方向
            //tmp = format->SetReadingDirection(static_cast<DWRITE_READING_DIRECTION>(args.reading));
            //longui_debug_hr(tmp, L"failed format->SetReadingDirection  " << long(args.reading));
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
    const GUID iid_IDXGIDebug = {
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
            iid_IDXGIDebug, 
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


#ifdef interface
#undef interface
#endif
#pragma interface
/// <summary>
/// screen iterface
/// </summary>
struct PCN_NOVTABLE LongUI::CUIResMgr::IScreen : IDXGIOutput {};
#pragma 

namespace LongUI {
    // impl
    namespace impl {
        // create native style renderer
        auto create_native_style_renderer() noexcept -> void*;
        // delete native style renderer
        void delete_native_style_renderer(void* ptr) noexcept;
        // recreate 
        auto recreate_native_style_renderer(void*ptr)->Result;
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
    detail::ctor_dtor<PrivateResMgr>::create(&rm());
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
    rm().~PrivateResMgr();
#ifndef NDEBUG
    delete m_pDebug;
#endif
}


/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIResMgr::recreate_device(IUIConfigure* cfg) noexcept -> Result {
    constexpr auto same_s = sizeof(PrivateResMgr) == sizeof(m_private);
    constexpr auto same_a = alignof(PrivateResMgr) == alignof(private_t);
    static_assert(same_s && same_a, "must be same");
    this->release_device();
    const auto flag = cfg->GetConfigureFlag();
    // 待用适配器
    IDXGIAdapter1* adapter = nullptr;
    // 枚举显示适配器
    if (!(flag & IUIConfigure::Flag_RenderByCPU)) {
        IDXGIFactory1* dxgifactory = nullptr;
        const auto idd = IID_IDXGIFactory1;
        const auto add = reinterpret_cast<void**>(&dxgifactory);
        // 创建一个临时工厂
        const Result tmp = { ::CreateDXGIFactory1(idd, add) };
        // 第一次失败直接返回
        if (!tmp) return tmp;
        IDXGIAdapter1* apAdapters[MAX_GRAPHICS_ADAPTERS];
        DXGI_ADAPTER_DESC1 descs[MAX_GRAPHICS_ADAPTERS];
        GraphicsAdapterDesc gas[MAX_GRAPHICS_ADAPTERS];
        uint32_t adnum = 0;
        // 枚举适配器
        for (adnum = 0; adnum < MAX_GRAPHICS_ADAPTERS; ++adnum) {
            constexpr HRESULT nf = DXGI_ERROR_NOT_FOUND;
            if (dxgifactory->EnumAdapters1(adnum, apAdapters + adnum) == nf) {
                break;
            }
            auto desc = descs + adnum;
            apAdapters[adnum]->GetDesc1(descs + adnum);
            gas[adnum].friend_name = desc->Description;
            gas[adnum].shared_system = desc->SharedSystemMemory;
            gas[adnum].dedicated_video = desc->DedicatedVideoMemory;
            gas[adnum].dedicated_system = desc->DedicatedSystemMemory;
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
        D3D_FEATURE_LEVEL featureLevels[] = {
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
        const auto dtype = flag & IUIConfigure::Flag_RenderByCPU ? 
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
        d.friend_name = desc.Description;
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
            IID_IDXGIFactory2,
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

/// <summary>
/// Releases the resource list.
/// </summary>
/// <returns></returns>
void LongUI::CUIResMgr::release_res_list() noexcept {
    auto& list = rm().reslist;
    // 释放未正确释放的资源(资源泄漏)
    for (auto& x : list) {
        if (x.obj) {
#ifndef NDEBUG
            const auto index = &x - &list.front();
            LUIDebug(Error)
                << "resource non-released:(index:"
                << static_cast<int32_t>(index)
                << ", ref: "
                << x.ref
                << ", type: "
                << static_cast<int>(x.type)
                << ")" << endl;
#endif
            x.obj->Destroy();
            x.obj = nullptr;
        }
    }
    assert(rm().rescount == 1 && "rescount should be 1");
}

/// <summary>
/// Recreates the resource.
/// </summary>
/// <returns></returns>
auto LongUI::CUIResMgr::recreate_resource() noexcept -> Result {
    Result rv = { Result::RS_OK };
    // XXX: 优化重建
    // TODO: 蛋疼的错误处理
    for (auto& x : rm().reslist) {
        switch (x.type)
        {
        case ResourceType::Type_Image:
        {
            I::Bitmap* bitmap = nullptr;
            const auto hr = this->CreateBitmapFromSSImageFile(x.uri, bitmap);
            if (hr) {
                const auto img = static_cast<CUIImage*>(x.obj);
                img->RecreateBitmap(*bitmap);
            }
            else rv = hr;
        }
            break;
        }
    }
    // 正常重建
    if (rv) {
        rv = impl::recreate_native_style_renderer(m_pNativeStyle);
    }
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
    // image to id
    auto xul_image_to_id(U8View view) noexcept -> uint32_t {
        const auto type = ResourceType::Type_Image;
        const auto id = UIManager.LoadResource(view, type, true);
        if (id) UIManager.AddResourceRefCount(id);
        return id;
    }
}}