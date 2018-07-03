// ui
#include <style/ui_style.h>
#include <core/ui_manager.h>
#include <core/ui_basic_type.h>
#include <core/ui_color_list.h>
#include <graphics/ui_bg_renderer.h>
#include <graphics/ui_graphics_impl.h>
// resource
#include <resource/ui_image.h>
// c++
#include <cassert>

/// <summary>
/// Initializes a new instance of the <see cref="Background"/> class.
/// </summary>
LongUI::CUIRendererBackground::CUIRendererBackground() noexcept {
    this->color = ColorF::FromRGBA_CT<RGBA_Transparent>();
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
    assert(m_pImageBrush == nullptr && "must release first");
    // 创建笔刷
    return this->RefreshImage();
}

/// <summary>
/// Sets the image.
/// </summary>
auto LongUI::CUIRendererBackground::RefreshImage() noexcept->Result {
    using brush_t = ID2D1BitmapBrush1;
    Result hr = { Result::RS_OK };
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
        assert(data.type == ResourceType::Type_Image);
        // 目前只支持IMAGE
        const auto img = static_cast<CUIImage*>(data.obj);
        const auto brush = reinterpret_cast<brush_t*>(m_pImageBrush);
        // 图像
        brush->SetBitmap(&img->RefBitmap());
        // 扩展
        brush->SetExtendModeX(D2D1_EXTEND_MODE_WRAP);
        brush->SetExtendModeY(D2D1_EXTEND_MODE_WRAP);
    }
    return hr;
}

/// <summary>
/// Releases the brush.
/// </summary>
/// <returns></returns>
void LongUI::CUIRendererBackground::release_brush() noexcept {
    if (m_pImageBrush) {
        m_pImageBrush->Release();
        m_pImageBrush = nullptr;
    }
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
/// <returns></returns>
void LongUI::CUIRendererBackground::RenderColor(const Box& box) const noexcept {
    // 渲染背景颜色
    if (this->color.a == 0.f) return;
    //if (!reinterpret_cast<const uint32_t&>(this->color.a)) return;
    // 获取渲染矩阵
    RectF rect; detail::get_render_rect(this->clip, box, rect);
    // 获取渲染器
    auto& render = UIManager.Ref2DRenderer();
    auto& brush0 = UIManager.RefCCBrush(this->color);
    // 执行渲染
    render.FillRectangle(auto_cast(rect), &brush0);
}


/// <summary>
/// Renders the image.
/// </summary>
/// <param name="box">The box.</param>
/// <returns></returns>
void LongUI::CUIRendererBackground::RenderImage(const LongUI::Box& box) const noexcept {
    // 渲染背景图片
    if (!m_pImageBrush) return;
    // 断言数据
    assert(this->image_id && "bad id");
    // 获取数据
    auto& data = UIManager.GetResoureceData(this->image_id);
    assert(data.type == ResourceType::Type_Image);
    // 获取大小
    const auto img = static_cast<CUIImage*>(data.obj);
    const auto img_size = Size2F{
        static_cast<float>(img->GetSize().width),
        static_cast<float>(img->GetSize().height)
    };
    // 获取渲染矩阵
    RectF rect; detail::get_render_rect(this->origin, box, rect);
    // 获取渲染器
    auto& render = UIManager.Ref2DRenderer();
    // 设置基本转换矩阵
    Matrix3X2F matrix = {
        1.f, 0.f, 
        0.f, 1.f, 
        rect.left, rect.top
    };
    // 计算ROUND
    const auto cal_round = [&](int i) noexcept {
        const auto size = i[&img_size.width];
        const auto length = i[&rect.right] - i[&rect.left];
        const auto count = length / size;
        const auto relen = float(int(count + 0.5f)) * size;
        return length / relen;
    };
    // 计算X-REPEAT
    switch (this->repeat & 0x0f)
    {
    case Repeat_Round:
        matrix._11 = cal_round(0);
        break;
    case Repeat_NoRepeat:
        rect.right = rect.left + img_size.width;
        break;
    }
    // 计算Y-REPEAT
    switch (this->repeat >> 4)
    {
    case Repeat_Round:
        matrix._22 = cal_round(1);
        break;
    case Repeat_NoRepeat:
        rect.bottom = rect.top + img_size.height;
        break;
    }
    // 计算宽度
    m_pImageBrush->SetTransform(&auto_cast(matrix));
    // 渲染笔刷
    render.FillRectangle(auto_cast(rect), m_pImageBrush);
}