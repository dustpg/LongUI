// ui
#include <style/ui_style.h>
#include <core/ui_manager.h>
#include <core/ui_basic_type.h>
#include <core/ui_color_list.h>
#include <util/ui_little_math.h>
#include <graphics/ui_bd_renderer.h>
#include <graphics/ui_graphics_impl.h>
// resource
#include <resource/ui_image.h>
// c++
#include <cassert>

#ifndef LUI_DISABLE_STYLE_SUPPORT

/// <summary>
/// Initializes a new instance of the <see cref="CUIRendererBorder"/> class.
/// </summary>
LongUI::CUIRendererBorder::CUIRendererBorder() noexcept {

}

/// <summary>
/// Finalizes an instance of the <see cref="CUIRendererBorder"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIRendererBorder::~CUIRendererBorder() noexcept {
    this->ReleaseDeviceData();
}

/// <summary>
/// Releases the device data.
/// </summary>
void LongUI::CUIRendererBorder::ReleaseDeviceData() noexcept {
    // 先释放
    this->release_brush();
}

/// <summary>
/// Releases the brush.
/// </summary>
/// <returns></returns>
void LongUI::CUIRendererBorder::release_brush() noexcept {
    if (m_pImageBrush) {
        m_pImageBrush->Release();
        m_pImageBrush = nullptr;
    }
}

/// <summary>
/// Sets the image identifier.
/// </summary>
/// <param name="id">The identifier.</param>
/// <returns></returns>
void LongUI::CUIRendererBorder::SetImageId(uint32_t id) noexcept {
#ifndef NDEBUG
    if (m_idImage == id) {
        LUIDebug(Warning)
            << "set same image id: "
            << id
            << endl;
    }
#endif
    m_idImage = id;
    // XXX: 错误处理
    const auto code = this->refresh_image();
}

/// <summary>
/// Sets the image slice.
/// </summary>
/// <param name="rect">The rect.</param>
/// <param name="fill">if set to <c>true</c> [fill].</param>
/// <returns></returns>
void LongUI::CUIRendererBorder::SetImageSlice(const RectF& rect, bool fill) noexcept {
    m_rcSlice = rect;
    m_bSliceFill = fill;
    m_bLayoutChanged = true;
}

/// <summary>
/// Sets the image repeat.
/// </summary>
/// <param name="repeat">The repeat.</param>
/// <returns></returns>
void LongUI::CUIRendererBorder::SetImageRepeat(AttributeRepeat repeat) noexcept {
    m_repeat = repeat;
    m_bLayoutChanged = true;
}

/// <summary>
/// Refreshes the image.
/// </summary>
/// <returns></returns>
auto LongUI::CUIRendererBorder::refresh_image() noexcept -> Result {
    m_szImage = {};
    using brush_t = ID2D1BitmapBrush1;
    if (!m_idImage) return { Result::RS_FALSE };
    Result hr = { Result::RS_OK };
    // 没有就创建
    if (!m_pImageBrush && m_idImage) {
        ID2D1Bitmap1* const bitmap = nullptr;
        auto& brush = reinterpret_cast<brush_t*&>(m_pImageBrush);
        hr = { UIManager.Ref2DRenderer().CreateBitmapBrush(bitmap, &brush) };
    }
    // 设置属性
    if (m_pImageBrush) {
        assert(m_idImage && "bad id");
        auto& data = UIManager.GetResoureceData(m_idImage);
        assert(data.type == ResourceType::Type_Image);
        // 目前只支持IMAGE
        const auto img = static_cast<CUIImage*>(data.obj);
        const auto brush = reinterpret_cast<brush_t*>(m_pImageBrush);
        // 检查大小
        const auto osize = img->GetSize();
        const auto fw = static_cast<float>(osize.width);
        const auto fh = static_cast<float>(osize.height);
        m_bLayoutChanged |= fw == m_szImage.width;
        m_bLayoutChanged |= fh == m_szImage.height;
        m_szImage.width = fw;
        m_szImage.height = fh;
        // 图像
        brush->SetBitmap(&img->RefBitmap());
        // 扩展
        brush->SetExtendModeX(D2D1_EXTEND_MODE_WRAP);
        brush->SetExtendModeY(D2D1_EXTEND_MODE_WRAP);
    }
    return hr;
}

/// <summary>
/// Refreshes the real slice.
/// </summary>
/// <returns></returns>
void LongUI::CUIRendererBorder::refresh_real_slice() noexcept {
    m_rcRealSlice = m_rcSlice;
    // LEFT
    if (detail::is_percent_value(m_rcSlice.left)) {
        m_rcRealSlice.left = m_szImage.width *
            detail::get_percent_value(m_rcSlice.left);
    }
    // TOP
    if (detail::is_percent_value(m_rcSlice.top)) {
        m_rcRealSlice.top = m_szImage.height *
            detail::get_percent_value(m_rcSlice.top);
    }
    // RIGHT
    if (detail::is_percent_value(m_rcSlice.right)) {
        m_rcRealSlice.right = m_szImage.width *
            detail::get_percent_value(m_rcSlice.right);
    }
    // BOTTOM
    if (detail::is_percent_value(m_rcSlice.bottom)) {
        m_rcRealSlice.bottom = m_szImage.height *
            detail::get_percent_value(m_rcSlice.bottom);
    }
}

/// <summary>
/// Befores the render.
/// </summary>
/// <returns></returns>
void LongUI::CUIRendererBorder::BeforeRender() noexcept {
    if (m_bLayoutChanged) {
        m_bLayoutChanged = false;
        this->refresh_real_slice();
    }
}


/// <summary>
/// Renders the default border.
/// </summary>
/// <param name="box">The box.</param>
/// <returns></returns>
void LongUI::CUIRendererBorder::render_default_border(const Box& box) const noexcept {

}


/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIRendererBorder::CreateDeviceData() noexcept -> Result {
    assert(m_pImageBrush == nullptr && "must release first");
    // 创建笔刷
    return this->refresh_image();
}

/// <summary>
/// Renders the border.
/// </summary>
/// <param name="box">The box.</param>
/// <returns></returns>
void LongUI::CUIRendererBorder::RenderBorder(const Box& box) const noexcept {
    assert(m_bLayoutChanged == false);
    // 渲染默认风格
    if (!m_idImage) return this->render_default_border(box);
    // 渲染边框图片
    if (!m_pImageBrush) return;
    // 记录
    const auto border_rect = box.GetBorderEdge();
    auto& renderer = UIManager.Ref2DRenderer();
    // 设置基本转换矩阵
    Matrix3X2F matrix = { 1.f, 0.f, 0.f, 1.f, 0.f, 0.f };
    /*
        ---------------------------------------
        | TL         | T            | TR      |
        ---------------------------------------
        |            |              |         |
        | L          | C            | R       |
        |            |              |         |
        ---------------------------------------
        | BL         | B            | BR      |
        ---------------------------------------
    */
    // 记录缩放比例
    const auto ratio_left = box.border.left / m_rcRealSlice.left;
    const auto ratio_top = box.border.top / m_rcRealSlice.top;
    const auto ratio_right = box.border.right / m_rcRealSlice.right;
    const auto ratio_bottom = box.border.bottom / m_rcRealSlice.bottom;
    const auto offset_right = ratio_right * (m_rcSlice.right - m_szImage.width);
    const auto offset_bottom = ratio_bottom * (m_rcSlice.bottom - m_szImage.height);
    const auto src_top = m_szImage.width - m_rcSlice.left - m_rcSlice.right;
    const auto src_left = m_szImage.height - m_rcSlice.top - m_rcSlice.bottom;
    // 计算矩形位置
    const RectF top_left = {
        border_rect.left,
        border_rect.top,
        border_rect.left + box.border.left,
        border_rect.top + box.border.top
    }, top_right = {
        border_rect.right - box.border.right,
        border_rect.top,
        border_rect.right,
        border_rect.top + box.border.top,
    }, bottom_right = {
        border_rect.right - box.border.right,
        border_rect.bottom - box.border.bottom,
        border_rect.right,
        border_rect.bottom,
    }, bottom_left = {
        border_rect.left,
        border_rect.bottom - box.border.bottom,
        border_rect.left + box.border.left,
        border_rect.bottom,
    };
    const RectF top_rect = {
        top_left.right,
        top_left.top,
        top_right.left,
        top_right.bottom
    };
    // 渲染TL角落
    matrix._11 = ratio_left;
    matrix._22 = ratio_top;
    matrix._31 = top_left.left; 
    matrix._32 = top_left.top;
    m_pImageBrush->SetTransform(&auto_cast(matrix));
    renderer.FillRectangle(&auto_cast(top_left), m_pImageBrush);
    // 渲染TR角落
    matrix._11 = ratio_right;
    matrix._22 = ratio_top;
    matrix._31 = top_right.left + offset_right;
    matrix._32 = top_right.top;
    m_pImageBrush->SetTransform(&auto_cast(matrix));
    renderer.FillRectangle(&auto_cast(top_right), m_pImageBrush);
    // 渲染BR角落
    matrix._11 = ratio_right;
    matrix._22 = ratio_bottom;
    matrix._31 = bottom_right.left + offset_right;
    matrix._32 = bottom_right.top + offset_bottom;
    m_pImageBrush->SetTransform(&auto_cast(matrix));
    renderer.FillRectangle(&auto_cast(bottom_right), m_pImageBrush);
    // 渲染BL角落
    matrix._11 = ratio_left;
    matrix._22 = ratio_bottom;
    matrix._31 = bottom_left.left;
    matrix._32 = bottom_left.top + offset_bottom;
    m_pImageBrush->SetTransform(&auto_cast(matrix));
    renderer.FillRectangle(&auto_cast(bottom_left), m_pImageBrush);
    // 渲染TOP边框

    //m_pImageBrush->SetTransform(&auto_cast(matrix));
    //renderer.FillRectangle(&auto_cast(top_rect), m_pImageBrush);
}



#endif