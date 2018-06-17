// ui
#include <core/ui_color_list.h>
#include <resource/ui_image.h>
#include <core/ui_manager.h>
// graphics
#include <graphics/ui_graphics_impl.h>



/// <summary>
/// Finalizes an instance of the <see cref="CUIImage"/> class.
/// </summary>
/// <returns></returns>
inline LongUI::CUIImage::~CUIImage() noexcept {
    assert(m_bitmap && "m_bitmap must be not-null");
    m_bitmap->Release();
}


/// <summary>
/// Destroys this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUIImage::Destroy() noexcept {
    delete this;
}

/// <summary>
/// Creates the image.
/// </summary>
/// <param name="bitmap">The bitmap.</param>
/// <param name="ptr">The PTR.</param>
/// <returns></returns>
auto LongUI::CUIImage::CreateImage(
    I::Bitmap& bitmap,
    CUISharedResource*& ptr) noexcept -> Result {
    Result hr = { Result::RS_OK };
    // 创建对象
    if (auto obj = new(std::nothrow) CUIImage{ bitmap }) {
        const auto size = bitmap.GetPixelSize();
        obj->m_size = { size.width, size.height };
        return hr;
    }
    // 内存不足
    return{ Result::RE_OUTOFMEMORY };
}

/// <summary>
/// Makes the error.
/// </summary>
/// <param name="bitmap">The bitmap.</param>
/// <returns></returns>
auto LongUI::CUIImage::MakeError(I::Bitmap& bitmap) noexcept->Result {
    constexpr float BLOCK_SIZE = 32.f;
    Result hr = { Result::RS_OK };
    const auto size = bitmap.GetSize();
    ID2D1BitmapRenderTarget* bmprt = nullptr;
    // 渲染锁
    UIManager.RenderLock();
    // 创建一个相同大小的兼容兼容RT
    if (hr) {
        hr = { UIManager.Ref2DRenderer().CreateCompatibleRenderTarget(
            size, &bmprt
        )};
    }
    // 渲染紫黑相间格子
    if (hr) {
        ColorF purple = ColorF::FromRGBA_CT<RGBA_Purple>();
        ColorF black = ColorF::FromRGBA_CT<RGBA_Black>();
        bmprt->BeginDraw();
        bmprt->Clear(auto_cast(purple));
        // TODO: 渲染紫黑相间格子

        hr = { bmprt->EndDraw() };
    }
    // 扫尾处理
    LongUI::SafeRelease(bmprt);
    UIManager.RenderUnlock();
    // TODO: 错误场合?
    return hr;
}

/// <summary>
/// Renders the specified render.
/// </summary>
/// <param name="render">The render.</param>
/// <param name="des_rect">The DES rect.</param>
/// <param name="src_rect">The source rect.</param>
/// <param name="opacity">The opacity.</param>
/// <param name="mode">The mode.</param>
/// <returns></returns>
void LongUI::CUIImage::Render(
    I::Renderer2D & renderer,
    const RectF * des_rect, 
    const RectF * src_rect, 
    float opacity, 
    InterpolationMode mode) const noexcept {
    D2D1_MATRIX_4X4_F* matrix = nullptr;
    // 调用NATIVE接口
    renderer.DrawBitmap(
        m_bitmap,
        auto_cast(des_rect),
        opacity,
        auto_cast(mode),
        auto_cast(src_rect),
        matrix
    );
}


