// ui
#include <style/ui_style.h>
#include <core/ui_manager.h>
#include <core/ui_basic_type.h>
#include <core/ui_color_list.h>
#include <graphics/ui_bg_renderer.h>
#include <graphics/ui_graphics_impl.h>
// resource
#include <resource/ui_image_res.h>
// c++
#include <cassert>

#ifndef LUI_DISABLE_STYLE_SUPPORT
/// <summary>
/// Initializes a new instance of the <see cref="Background"/> class.
/// </summary>
LongUI::CUIRendererBackground::CUIRendererBackground() noexcept {
    this->color = ColorF::FromRGBA_CT<RGBA_Transparent>();
    //this->size = { };
}

/// <summary>
/// Finalizes an instance of the <see cref="Background"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIRendererBackground::~CUIRendererBackground() noexcept {
    this->ReleaseDeviceData();
}

/// <summary>
/// Releases the device data.
/// </summary>
void LongUI::CUIRendererBackground::ReleaseDeviceData() {
    // 先释放
    this->release_brush();
}

/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIRendererBackground::CreateDeviceData() noexcept -> Result {
    //assert(m_pImageBrush == nullptr && "must release first");
    // 创建笔刷
    return this->RefreshImage();
}

/// <summary>
/// Sets the image.
/// </summary>
auto LongUI::CUIRendererBackground::RefreshImage() noexcept->Result {
    using brush_t = ID2D1BitmapBrush1;
    if (!this->image_id.GetId()) return { Result::RS_FALSE };
    Result hr = { Result::RS_OK };
#if 0
    // 没有就创建
    if (!m_pImageBrush && this->image_id) {
        ID2D1Bitmap1* const bitmap = nullptr;
        auto& brush = reinterpret_cast<brush_t*&>(m_pImageBrush);
        hr = { UIManager.Ref2DRenderer().CreateBitmapBrush(bitmap, &brush) };
    }
    // 设置属性
    if (m_pImageBrush) {
        assert(this->image_id && "bad id");
        auto& data = UIManager.GetResoureceData(this->image_id);
        assert(data.GetType() == ResourceType::Type_Image);
        // 目前只支持IMAGE
        const auto img = static_cast<CUIImage*>(data.obj);
        const auto brush = reinterpret_cast<brush_t*>(m_pImageBrush);
        // 获取大小
        m_szImage.width = img->GetSize().width;
        m_szImage.height = img->GetSize().height;
        // 图像
        brush->SetBitmap(&img->RefBitmap());
        // 扩展
        brush->SetExtendModeX(D2D1_EXTEND_MODE_WRAP);
        brush->SetExtendModeY(D2D1_EXTEND_MODE_WRAP);
    }
#endif
    return hr;
}

/// <summary>
/// Releases the brush.
/// </summary>
/// <returns></returns>
void LongUI::CUIRendererBackground::release_brush() noexcept {
    //LongUI::SafeRelease(m_pImageBrush);
}

// longui::detail
namespace LongUI { namespace detail {
    /// <summary>
    /// Gets the render rect.
    /// </summary>
    /// <param name="attr">The attribute.</param>
    /// <param name="box">The box.</param>
    /// <param name="rect">The rect.</param>
    /// <returns></returns>
    void get_render_rect(
        AttributeBox attr,
        const Box& box, 
        RectF& rect) noexcept {
        switch (attr)
        {
        default: [[fallthrough]];
        case LongUI::Box_BorderBox:  box.GetBorderEdge(rect);  break;
        case LongUI::Box_PaddingBox: box.GetPaddingEdge(rect); break;
        case LongUI::Box_ContentBox: box.GetContentEdge(rect); break;
        }
    }
}}


/// <summary>
/// Renders the color.
/// </summary>
/// <param name="box">The box.</param>
/// <param name="radius">The radius.</param>
/// <returns></returns>
void LongUI::CUIRendererBackground::RenderColor(const Box& box, Size2F radius) const noexcept {
    // 渲染背景颜色
    if (this->color.a == 0.f) return;
    //if (!reinterpret_cast<const uint32_t&>(this->color.a)) return;
    // 获取渲染矩阵
    D2D1_ROUNDED_RECT rrect;
    detail::get_render_rect(this->clip, box, auto_cast(rrect.rect));
    rrect.radiusX = radius.width;
    rrect.radiusY = radius.height;

    // 获取渲染器
    auto& render = UIManager.Ref2DRenderer();
    auto& brush0 = UIManager.RefCCBrush(this->color);
    // 执行渲染
    render.FillRoundedRectangle(&rrect, &brush0);
}


/// <summary>
/// Renders the image.
/// </summary>
/// <param name="box">The box.</param>
/// <param name="radius">The radius.</param>
/// <returns></returns>
void LongUI::CUIRendererBackground::RenderImage(const LongUI::Box& box, Size2F radius) const noexcept {
#if 0
    // 渲染背景图片
    if (!this->image_id || !m_pImageBrush) return;
    // 图片大小
    const auto& img_size = m_szImage;
    // 获取渲染矩阵
    D2D1_ROUNDED_RECT rrect;
    detail::get_render_rect(this->origin, box, auto_cast(rrect.rect));
    rrect.radiusX = radius.width;
    rrect.radiusY = radius.height;
    // 获取渲染器
    auto& render = UIManager.Ref2DRenderer();
    // 设置基本转换矩阵
    Matrix3X2F matrix = {
        1.f, 0.f, 
        0.f, 1.f, 
        rrect.rect.left, rrect.rect.top
    };
    // 计算ROUND
    const auto cal_round = [&](int i) noexcept {
        const auto size = i[&img_size.width];
        const auto length = i[&rrect.rect.right] - i[&rrect.rect.left];
        const auto count = length / size;
        const auto count1 = count < 1.f ? 1.f : count;
        const auto relen = float(int(count1 + 0.5f)) * size;
        return length / relen;
    };
    // 计算X-REPEAT
    switch (this->repeat & 0x0f)
    {
    case Repeat_Round:
        matrix._11 = cal_round(0);
        break;
    case Repeat_NoRepeat:
        rrect.rect.right = rrect.rect.left + img_size.width;
        break;
    }
    // 计算Y-REPEAT
    switch (this->repeat >> 4)
    {
    case Repeat_Round:
        matrix._22 = cal_round(1);
        break;
    case Repeat_NoRepeat:
        rrect.rect.bottom = rrect.rect.top + img_size.height;
        break;
    }
    // 计算宽度
    m_pImageBrush->SetTransform(&auto_cast(matrix));
    // 渲染笔刷
    render.FillRoundedRectangle(&rrect, m_pImageBrush);
#endif
}
#endif
