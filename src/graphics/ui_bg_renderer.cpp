#define NOMINMAX
// ui
#include <style/ui_style.h>
#include <core/ui_manager.h>
#include <core/ui_basic_type.h>
#include <core/ui_color_list.h>
#include <graphics/ui_bg_renderer.h>
#include <graphics/ui_graphics_impl.h>
// resource
#include <resource/ui_image_res.h>
#include <effect/ui_effect_backimage.h>
// c++
#include <cassert>
#include <algorithm>

#ifndef LUI_DISABLE_STYLE_SUPPORT
extern "C" const GUID GUID_LongUIEffect_BackImage;

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
    LongUI::SafeRelease(m_pOutput);
    LongUI::SafeRelease(m_pBackground);
    //LongUI::SafeRelease(m_pImageBrush);
}

/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIRendererBackground::CreateDeviceData() noexcept -> Result {
    //assert(m_pImageBrush == nullptr && "must release first");
    // 创建笔刷
    return this->refresh_image();
}

/// <summary>
/// Sets the image identifier.
/// </summary>
/// <param name="id">The identifier.</param>
/// <returns></returns>
void LongUI::CUIRendererBackground::SetImageId(uintptr_t id) noexcept {
#ifndef NDEBUG
    if (m_idImage.GetId() == id) {
        LUIDebug(Warning)
            << "set same image id: "
            << reinterpret_cast<void*>(id)
            << endl;
    }
#endif
    m_idImage.SetId(id);
    m_bIdChanged = true;
}

/// <summary>
/// Sets the image.
/// </summary>
auto LongUI::CUIRendererBackground::refresh_image() noexcept->Result {
    using effect_t = ID2D1Effect;
    using brush_t = ID2D1BitmapBrush1;
    if (!m_idImage.GetId()) return { Result::RS_FALSE };
    Result hr = { Result::RS_OK };
    // 没有就创建
    if (!m_pBackground && m_idImage.GetId()) {
        auto& effect = reinterpret_cast<effect_t*&>(m_pBackground);
        auto& guid = GUID_LongUIEffect_BackImage;
        hr.code = UIManager.Ref2DRenderer().CreateEffect(guid, &effect);
    }
    // 更新输入
    if (m_pBackground) {
        auto& res = m_idImage.RefResource();
        assert(res.RefData().GetType() == ResourceType::Type_Image);
        auto& img = static_cast<CUIImage&>(res);
        // 检查大小
        auto& frame = img.RefFrame(0);
        const auto realsize = frame.bitmap->GetSize();
        const auto fw = static_cast<float>(realsize.width);
        const auto fh = static_cast<float>(realsize.height);
        //if (fw != m_szImage.width || fh != m_szImage.height)
            //m_bLayoutChanged = true;
        m_szImage.width = frame.source.right - frame.source.left;
        m_szImage.height = frame.source.bottom - frame.source.top;
        m_rcImage.left = frame.source.left / fw;
        m_rcImage.top = frame.source.top / fh;
        // 由于HLSL是偏移半像素的 宽度需要减去1 还要避免除以0
        /*
          |     |      图片宽度
          v     v
          |A|A|A|A
           ^   ^
           |   |       真实宽度
        */
        m_rcImage.right = std::max(m_szImage.width - 1.f, 1.f) / fw;
        m_rcImage.bottom = std::max(m_szImage.height - 1.f, 1.f) / fh;
        // 输入 
        m_pBackground->SetInput(0, frame.bitmap);
        // 获取输出
        if (!m_pOutput) m_pBackground->GetOutput(AddrOf(m_pOutput));
    }
    return hr;
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
/// Befores the render.
/// </summary>
/// <returns></returns>
void LongUI::CUIRendererBackground::BeforeRender() noexcept {
    // XXX: m_bIdChanged 应该用原子类型保证数据安全?
    if (m_bIdChanged) {
        m_bIdChanged = false;
        this->refresh_image();
    }
}

/// <summary>
/// Renders the image.
/// </summary>
/// <param name="box">The box.</param>
/// <param name="radius">The radius.</param>
/// <returns></returns>
void LongUI::CUIRendererBackground::RenderImage(const LongUI::Box& box, Size2F radius) const noexcept {
    // 渲染背景图片
    if (!m_pOutput) return;
    // 获取渲染器
    auto& renderer = UIManager.Ref2DRenderer();
    // 获取渲染矩阵
    D2D1_ROUNDED_RECT rrect;
    detail::get_render_rect(this->origin, box, auto_cast(rrect.rect));
    // TODO: 实现圆角, Before优化
    rrect.radiusX = radius.width;
    rrect.radiusY = radius.height;
    // 填写数据
    Effect::BackImageMatrix bim = { {0}, {1, 1}, {0} };
    bim.source = m_rcImage;
    bim.output.width = rrect.rect.right - rrect.rect.left;
    bim.output.height = rrect.rect.bottom - rrect.rect.top;
    bim.space.width = m_rcImage.right;
    bim.space.height = m_rcImage.bottom;
    const auto& img_size = m_szImage;
    // 计算ROUND
    const auto cal_round = [&](int i) noexcept {
        const auto size = i[&img_size.width];
        const auto length = i[&rrect.rect.right] - i[&rrect.rect.left];
        const auto count = length / size;
        const auto relen = float(int(count + 0.5f)) * size;
        return relen / length;
    };
    // 计算SPACE
    const auto cal_space = [&](int i, float& out) noexcept {
        const auto size = i[&img_size.width];
        const auto length = i[&rrect.rect.right] - i[&rrect.rect.left];
        const auto count = length / size;
        const auto count2 = float(int(count));
        if (count2 == 0.0f) return 0.f;
        const auto relen = count2 * size;
        return (length - relen) / (count2 * size) + 1.f;
    };
    bim.flag[0] = this->repeat & (Repeat_Space | Repeat_Space << 4);
    // 计算X-REPEAT
    switch (this->repeat & 0x0f)
    {
    case Repeat_Space:
        bim.space.width *= cal_space(0, bim.output.width);
        if (bim.space.width == 0.f) return;
        break;
    case Repeat_Round:
        bim.rounds.width = cal_round(0);
        if (bim.rounds.width == 0.f) return;
        break;
    case Repeat_NoRepeat:
        bim.output.width = img_size.width;
        break;
    }
    // 计算Y-REPEAT
    switch (this->repeat >> 4)
    {
    case Repeat_Space:
        bim.space.height *= cal_space(1, bim.output.height);
        if (bim.space.height == 0.f) return;
        break;
    case Repeat_Round:
        bim.rounds.height = cal_round(1);
        if (bim.rounds.height == 0.f) return;
        break;
    case Repeat_NoRepeat:
        bim.output.height = img_size.height;
        break;
    }

    m_pBackground->SetValue(0, bim);
    const auto& point = reinterpret_cast<Point2F&>(rrect.rect.left);
    renderer.DrawImage(
        m_pOutput, &auto_cast(point)
        , nullptr, D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR
    );
}
#endif
