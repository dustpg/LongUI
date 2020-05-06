// ui
#include <style/ui_style.h>
#include <core/ui_manager.h>
#include <core/ui_basic_type.h>
#include <core/ui_color_list.h>
#include <util/ui_little_math.h>
#include <graphics/ui_bd_renderer.h>
#include <graphics/ui_graphics_impl.h>
// resource
#include <effect/ui_effect_borderimage.h>
#include <resource/ui_image_res.h>
// c++
#include <cassert>

#ifndef LUI_DISABLE_STYLE_SUPPORT

extern "C" const GUID GUID_LongUIEffect_BorderImage;

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
    this->release_effect();
}

/// <summary>
/// Releases the brush.
/// </summary>
/// <returns></returns>
void LongUI::CUIRendererBorder::release_effect() noexcept {
    LongUI::SafeRelease(m_pBorder);
    LongUI::SafeRelease(m_pOutput);
}

/// <summary>
/// Sets the image identifier.
/// </summary>
/// <param name="id">The identifier.</param>
/// <returns></returns>
void LongUI::CUIRendererBorder::SetImageId(uintptr_t id) noexcept {
#ifndef NDEBUG
    if (m_idImage.GetId() == id) {
        LUIDebug(Warning)
            << "set same image id: "
            << reinterpret_cast<void*>(id)
            << endl;
    }
#endif
    m_idImage.SetId(id);
    // 没有就忽略
    if (!m_pBorder) return;
    // XXX: 错误处理
    const auto code = this->refresh_image();
    assert(code && "TODO: ERROR HANDLE");
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
    using effect_t = ID2D1Effect;
    if (!m_idImage.GetId()) return { Result::RS_FALSE };
    Result hr = { Result::RS_OK };
    // 没有就创建
    if (!m_pBorder && m_idImage.GetId()) {
        auto& effect = reinterpret_cast<effect_t*&>(m_pBorder);
        auto& guid = GUID_LongUIEffect_BorderImage;
        hr.code = UIManager.Ref2DRenderer().CreateEffect(guid, &effect) ;
    }
    // 更新输入
    if (m_pBorder) {
        auto& res = m_idImage.RefResource();
        assert(res.RefData().GetType() == ResourceType::Type_Image);
        auto& img = static_cast<CUIImage&>(res);
        // 检查大小
        auto& frame = img.RefFrame(0);
        const auto realsize = frame.bitmap->GetSize();
        const auto fw = static_cast<float>(realsize.width);
        const auto fh = static_cast<float>(realsize.height);
        if (fw != m_szImage.width || fh != m_szImage.height) 
            m_bLayoutChanged = true;
        //m_szImage = { realsize.width, realsize.height };
        m_szImage = img.size;
        m_szRatio.width = img.size.width / realsize.width;
        m_szRatio.height = img.size.height / realsize.height;
        m_rcSource.left = frame.source.left / realsize.width;
        m_rcSource.top = frame.source.top / realsize.height;
        m_rcSource.right = frame.source.right / realsize.width;
        m_rcSource.bottom = frame.source.bottom / realsize.height;
        // 设置输入
        m_pBorder->SetInput(0, frame.bitmap);
        // 获取输出
        if (!m_pOutput) m_pBorder->GetOutput(AddrOf(m_pOutput));
    }
    return hr;
}

/// <summary>
/// Refreshes the real slice.
/// </summary>
/// <returns></returns>
void LongUI::CUIRendererBorder::refresh_real_slice() noexcept {
    // LEFT
    if (detail::is_percent_value(m_rcSlice.left))
        m_rcRealSlice.left = detail::get_percent_value(m_rcSlice.left);
    else
        m_rcRealSlice.left = m_rcSlice.left / m_szImage.width;
    // TOP
    if (detail::is_percent_value(m_rcSlice.top)) 
        m_rcRealSlice.top = detail::get_percent_value(m_rcSlice.top);
    else
        m_rcRealSlice.top = m_rcSlice.top / m_szImage.height;
    // RIGHT
    if (detail::is_percent_value(m_rcSlice.right))
        m_rcRealSlice.right = detail::get_percent_value(m_rcSlice.right);
    else
        m_rcRealSlice.right = m_rcSlice.right / m_szImage.width;
    // BOTTOM
    if (detail::is_percent_value(m_rcSlice.bottom))
        m_rcRealSlice.bottom = detail::get_percent_value(m_rcSlice.bottom);
    else
        m_rcRealSlice.bottom = m_rcSlice.bottom / m_szImage.height;
}

/// <summary>
/// Befores the render.
/// </summary>
/// <returns></returns>
void LongUI::CUIRendererBorder::BeforeRender(const Box& box) noexcept {
    if (m_pBorder && m_bLayoutChanged) {
        this->refresh_real_slice();
        this->refresh_draw_count();
        this->refresh_image_matrix(box);
    }
    m_bLayoutChanged = false;
}


/// <summary>
/// Renders the default border.
/// </summary>
/// <param name="box">The box.</param>
/// <returns></returns>
void LongUI::CUIRendererBorder::render_default_border(const Box& box) const noexcept {
    if (this->style == Style_None) return;

    D2D1_ROUNDED_RECT rrect;
    box.GetBorderEdge(auto_cast(rrect.rect));
    rrect.radiusX = this->radius_x;
    rrect.radiusY = this->radius_y;

    auto& renderer = UIManager.Ref2DRenderer();
    auto& brush = UIManager.RefCCBrush(this->color);
    renderer.DrawRoundedRectangle(
        &rrect,
        &brush,
        box.border.left
    );
}


/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIRendererBorder::CreateDeviceData() noexcept -> Result {
    assert(m_pBorder == nullptr && "must release first");
    assert(m_pOutput == nullptr && "must release first");
    // 创建笔刷
    return this->refresh_image();
}

/// <summary>
/// Calculates the repeat.
/// </summary>
/// <param name="rect">The rect.</param>
/// <param name="box">The box.</param>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::CUIRendererBorder::calculate_repeat(
    RectF& rect, const Box& box, Size2F size) const noexcept {
    Size2F img_center_size = {
        m_szImage.width * (1.f - m_rcRealSlice.left - m_rcRealSlice.right),
        m_szImage.height * (1.f - m_rcRealSlice.top - m_rcRealSlice.bottom)
    };
    RectF img_scale = {
        m_szImage.height * m_rcRealSlice.top / box.border.top,
        m_szImage.width * m_rcRealSlice.left / box.border.left,
        m_szImage.height * m_rcRealSlice.bottom / box.border.bottom,
        m_szImage.width * m_rcRealSlice.right / box.border.right
    };
    Size2F bd_center_size = {
        size.width - box.border.left - box.border.right,
        size.height - box.border.top - box.border.bottom
    };
    rect.left = bd_center_size.width / img_center_size.width * img_scale.left;
    rect.top = bd_center_size.height / img_center_size.height * img_scale.top;
    rect.right = bd_center_size.width / img_center_size.width * img_scale.right;
    rect.bottom = bd_center_size.height / img_center_size.height * img_scale.bottom;
    // 计算X-REPEAT
    switch (m_repeat & 0x0f)
    {
    case Repeat_Space:
    case Repeat_Round:
        rect.left = LongUI::RoundInGuiLevel(rect.left);
        rect.right = LongUI::RoundInGuiLevel(rect.right);
        break;
    case Repeat_Stretch:
        rect.left = 1.f;
        rect.right = 1.f;
        break;
    }
    // 计算Y-REPEAT
    switch (m_repeat >> 4)
    {
    case Repeat_Repeat:
        break;
    case Repeat_Space:
    case Repeat_Round:
        rect.top = LongUI::RoundInGuiLevel(rect.top);
        rect.bottom = LongUI::RoundInGuiLevel(rect.bottom);
        break;
    case Repeat_Stretch:
        rect.top = 1.f;
        rect.bottom = 1.f;
        break;
    }
    // RATIO
    //rect.left *= m_szRatio.width;
    //rect.top *= m_szRatio.height;
    //rect.right *= m_szRatio.width;
    //rect.bottom *= m_szRatio.height;
}

/// <summary>
/// Renders the border.
/// </summary>
/// <param name="box">The box.</param>
/// <returns></returns>
void LongUI::CUIRendererBorder::RenderBorder(const Box& box) const noexcept {
    assert(m_bLayoutChanged == false);
    // 渲染默认风格
    if (!m_idImage.GetId()) return this->render_default_border(box);
    // 渲染边框图片
    if (!m_pOutput) return;
    // 记录
    const auto border_rect = box.GetBorderEdge();
    auto& renderer = UIManager.Ref2DRenderer();
    // 正式渲染
    Point2F offset = { border_rect.left, border_rect.top };
    renderer.DrawImage(m_pOutput, &auto_cast(offset));
}


/// <summary>
/// Refreshes the image matrix.
/// </summary>
/// <param name="box">The box.</param>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::CUIRendererBorder::refresh_image_matrix(const Box& box) noexcept {
    const auto border_rect = box.GetBorderEdge();
    const Size2F size{
        border_rect.right - border_rect.left,
            border_rect.bottom - border_rect.top
    };
    // 更新cbuffer
    Effect::BorderImageMatrix matrix;
    // 目标:
    matrix.zone0.left = box.border.left;
    matrix.zone0.top = box.border.top;
    matrix.zone1.left = size.width - box.border.left - box.border.right;
    matrix.zone1.top = size.height - box.border.top - box.border.bottom;
    matrix.zone2.left = box.border.right;
    matrix.zone2.top = box.border.bottom;
    matrix.zone3.left = size.width;
    matrix.zone3.top = size.height;
    // 支持图片中一小部分
    matrix.zone0.right = m_rcSource.left;
    matrix.zone0.bottom = m_rcSource.top;
    matrix.zone1.right = m_rcSource.left + m_rcRealSlice.left * m_szRatio.width;
    matrix.zone1.bottom = m_rcSource.top + m_rcRealSlice.top* m_szRatio.height;
    matrix.zone2.right = m_rcSource.right - m_rcRealSlice.right* m_szRatio.width;
    matrix.zone2.bottom = m_rcSource.bottom - m_rcRealSlice.bottom* m_szRatio.height;
    matrix.zone3.right = m_rcSource.right;
    matrix.zone3.bottom = m_rcSource.bottom;
    // 重复
    matrix.repeat = { 1.f, 1.f, 1.f, 1.f };
    this->calculate_repeat(luiref matrix.repeat, box, size);
    m_pBorder->SetValue(Effect::BImage_Matrix, matrix);
}

/// <summary>
/// Refreshes the drawcount.
/// </summary>
/// <returns></returns>
void LongUI::CUIRendererBorder::refresh_draw_count() noexcept {
    // FILL
    const uint32_t draw_count = m_bSliceFill
        ? Effect::VERTEX_FULLCOUNT
        : Effect::VERTEX_NOFILLCOUNT
        ;
    m_pBorder->SetValue(Effect::BImage_Draw, draw_count);
}

#endif