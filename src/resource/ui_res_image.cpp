// ui
#include <core/ui_malloc.h>
#include <core/ui_manager.h>
#include <util/ui_ctordtor.h>
#include <core/ui_color_list.h>
#include <resource/ui_image_res.h>
// graphics
#include <graphics/ui_graphics_impl.h>


namespace LongUI { namespace detail {
    // free rects
    void free_rects(BitmapFrame& f) noexcept;
    // u2->f2
    inline auto u2_f2(Size2U sz) noexcept {
        return Size2F{
            static_cast<float>(sz.width),
            static_cast<float>(sz.height)
        };
    }
}}


/// <summary>
/// Releases this instance.
/// </summary>
/// <returns></returns>
LongUI::CUIImage::~CUIImage() noexcept {
    // XXX: PushLaterReleaseCOM?
    // 需要释放设备资源
    CUIRenderAutoLocker locker;

    const auto fc = this->frame_count;
    for (uint32_t i = 0; i != fc; ++i) {
        auto& frame = m_frames[i];
        // 逆序释放?
        //auto& frame = m_frames[fc - 1 - i];
        LongUI::SafeRelease(frame.bitmap);
        if (frame.window) detail::free_rects(frame);
    }
}

/// <summary>
/// Releases the only.
/// </summary>
/// <returns></returns>
void LongUI::CUIImage::ReleaseOnly() noexcept {
    const auto fc = this->frame_count;
    for (uint32_t i = 0; i != fc; ++i)
        LongUI::SafeRelease(m_frames[i].bitmap);
}

/// <summary>
/// Destroys this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUIImage::Destroy() noexcept {
    this->~CUIImage();
    LongUI::NormalFree(this);
}

/// <summary>
/// Initializes a new instance of the <see cref="CUIImage" /> class.
/// </summary>
/// <param name="f">The f.</param>
/// <param name="d">The d.</param>
/// <param name="s">The s.</param>
LongUI::CUIImage::CUIImage(uint32_t f, uint32_t d, Size2U s) noexcept
    : frame_count(f), delay(d), size(detail::u2_f2(s)) {
    assert(frame_count && "at least 1");
    std::memset(m_frames, 0, frame_count * sizeof(BitmapFrame));
}

/// <summary>
/// Creates the specified frame.
/// </summary>
/// <param name="f">The f.</param>
/// <param name="d">The d.</param>
/// <param name="s">The s.</param>
/// <returns></returns>
auto LongUI::CUIImage::Create(uint32_t f, uint32_t d, Size2U s) noexcept -> CUIImage* {
    assert(f && "bad frame count");
    constexpr size_t base = sizeof(CUIImage) - sizeof(BitmapFrame);
    const size_t len = base + f * sizeof(BitmapFrame);
    const auto ptr = static_cast<CUIImage*>(LongUI::NormalAlloc(len));
    if (!ptr) return 0;
    detail::ctor_dtor<CUIImage>::create(ptr, f, d, s);
    return ptr;
}

/// <summary>
/// Recreates the bitmap.
/// </summary>
/// <param name="bitmap">The bitmap.</param>
/// <returns></returns>
//void LongUI::CUIImage::RecreateBitmap(I::Bitmap& bitmap) noexcept {
//    assert(m_bitmap && "m_bitmap must be not-null");
//    m_bitmap->Release();
//    m_bitmap = &bitmap;
//}

/// <summary>
/// Makes the error.
/// </summary>
/// <param name="bitmap">The bitmap.</param>
/// <returns></returns>
//auto LongUI::CUIImage::MakeError(I::Bitmap& bitmap) noexcept->Result {
//    constexpr float BLOCK_SIZE = 32.f;
//    Result hr = { Result::RS_OK };
//    const auto size = bitmap.GetSize();
//    ID2D1BitmapRenderTarget* bmprt = nullptr;
//    // 渲染锁
//    UIManager.RenderLock();
//    // 创建一个相同大小的兼容兼容RT
//    if (hr) {
//        hr = { UIManager.Ref2DRenderer().CreateCompatibleRenderTarget(
//            size, &bmprt
//        )};
//    }
//    // 渲染紫黑相间格子
//    if (hr) {
//        ColorF purple = ColorF::FromRGBA_CT<RGBA_Purple>();
//        ColorF black = ColorF::FromRGBA_CT<RGBA_Black>();
//        bmprt->BeginDraw();
//        bmprt->Clear(auto_cast(purple));
//        // TODO: 渲染紫黑相间格子
//
//        hr = { bmprt->EndDraw() };
//    }
//    // 扫尾处理
//    LongUI::SafeRelease(bmprt);
//    UIManager.RenderUnlock();
//    // TODO: 错误场合?
//    return hr;
//}


/// <summary>
/// Renders the specified render.
/// </summary>
/// <param name="frame_id">The frame identifier.</param>
/// <param name="renderer">The renderer.</param>
/// <param name="des_rect">The DES rect.</param>
/// <param name="opacity">The opacity.</param>
/// <param name="mode">The mode.</param>
/// <returns></returns>
void LongUI::CUIImage::Render(
    uint32_t frame_id,
    I::Renderer2D& renderer,
    const RectF* des_rect, 
    //const RectF* src_rect, 
    float opacity, 
    InterpolationMode mode) const noexcept {
#if 0
    const auto c = ColorF::FromRGBA_CT<RGBA_TianyiBlue>();
    renderer.FillRectangle(auto_cast(des_rect), &UIManager.RefCCBrush(c));
#endif
    assert(frame_id < this->frame_count);
    if (frame_id < this->frame_count) {
        const auto& frame = m_frames[frame_id];
        if (!frame.bitmap) {
#ifndef NDEBUG

#endif
            return;
        }
        D2D1_MATRIX_4X4_F* const matrix = nullptr;
        // 调用NATIVE接口
        renderer.DrawBitmap(
            frame.bitmap,
            auto_cast(des_rect),
            opacity,
            auto_cast(mode),
            auto_cast(&frame.source),
            //nullptr,
            matrix
        );
    }
}

