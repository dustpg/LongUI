// UI HREADER
#include <luiconf.h>
#include <graphics/ui_matrix3x2.h>
#include <text/ui_text_outline.h>
#include <core/ui_color_list.h>
#include <util/ui_ctordtor.h>
#include <core/ui_manager.h>
// OS IMPL
#include <graphics/ui_graphics_impl.h>
#include <text/ui_ctl_arg.h>
#include <text/ui_ctl_impl.h>
// C/C++
#include <cassert>

// longui namespace
namespace LongUI {
    // IDWriteTextRenderer
    const GUID IID_IDWriteTextRenderer = {
        0xef8a8135, 0x5cc6, 0x45fe,{ 0x88, 0x25, 0xc5, 0xa0, 0x72, 0x4e, 0xb8, 0x19 }
    };
}

// longui::impl
namespace LongUI { namespace impl {
    // offset rect
    inline auto offset(const RectF& rect, float offsetX, float offsetY) noexcept {
        return RectF {
            rect.left + offsetX,
            rect.top + offsetY,
            rect.right + offsetX,
            rect.bottom + offsetY
        };
    }
    // basic renderer impl
    struct PCN_NOVTABLE basic_renderer : IDWriteTextRenderer {
        // get render context
        static auto get_context(void* ptr) noexcept { return reinterpret_cast<const TextBasicContext*>(ptr); }
        // add ref-count
        ULONG UNICALL AddRef() noexcept final { return 2; };
        // release ref-count
        ULONG UNICALL Release() noexcept final { return 1; };
        // query the interface
        HRESULT UNICALL QueryInterface(const IID&, void **ppvObject) noexcept override final;
        // is pixel snapping disabled?
        HRESULT UNICALL IsPixelSnappingDisabled(void*, BOOL*) noexcept final override;
        // get current transform
        HRESULT UNICALL GetCurrentTransform(void*, DWRITE_MATRIX*) noexcept final override;
        // get bilibili of px/pt
        HRESULT UNICALL GetPixelsPerDip(void*, FLOAT*) noexcept final override;
        // draw inline object implemented as template
        HRESULT UNICALL DrawInlineObject(void*, FLOAT, FLOAT, IDWriteInlineObject*, BOOL, BOOL, IUnknown*) noexcept override;
        // draw underline
        HRESULT UNICALL DrawUnderline(void*, FLOAT, FLOAT, const DWRITE_UNDERLINE*, IUnknown*) noexcept override;
        // draw strikethrough
        HRESULT UNICALL DrawStrikethrough(void*, FLOAT, FLOAT, const DWRITE_STRIKETHROUGH*, IUnknown*) noexcept override;
    };
    // outline renderer
    struct outline_renderer : basic_renderer {
        // get render context
        static auto get_context(void* ptr) noexcept { return reinterpret_cast<const TextOutlineContext*>(ptr); }
        // draw glyphrun
        HRESULT UNICALL DrawGlyphRun(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_MEASURING_MODE measuringMode,
            const DWRITE_GLYPH_RUN* glyphRun,
            const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDescription,
            IUnknown* clientDrawingEffect
        ) noexcept override;
    };
}}


/// <summary>
/// Initializes a new instance of the <see cref="CUITextOutline"/> class.
/// </summary>
LongUI::CUITextOutline::CUITextOutline() noexcept {
    const auto address = &m_buffer;
    using target_t = impl::outline_renderer;
    static_assert(sizeof(*this) == sizeof(this), "one pointer");
    static_assert(sizeof(m_buffer) == sizeof(target_t), "be same!");
    detail::ctor_dtor<target_t>::create_obj(address);
}

/// <summary>
/// Finalizes an instance of the <see cref="CUITextOutline"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUITextOutline::~CUITextOutline() noexcept {
    const auto address = &m_buffer;
    using target_t = impl::outline_renderer;
    static_assert(sizeof(m_buffer) == sizeof(target_t), "be same!");
    detail::ctor_dtor<target_t>::delete_obj(address);
}


/// <summary>
/// Renders the specified CTX.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <param name="text">The text.</param>
/// <param name="point">The point.</param>
/// <returns></returns>
void LongUI::CUITextOutline::Render(
    const TextOutlineContext& ctx, 
    I::Text* text, 
    Point2F point) const noexcept {
    if (!text) return;
    using target_t = impl::outline_renderer;
    static_assert(sizeof(m_buffer) == sizeof(target_t), "be same!");
    const auto buf_adr = const_cast<void**>(&m_buffer);
    const auto renderer = reinterpret_cast<I::TextRenderer*>(buf_adr);
    const void* const caddress = &ctx;
    const auto contex = const_cast<void*>(caddress);
    const auto hr = text->Draw(contex, renderer, point.x, point.y);
    const Result result{ hr };
}

// longui::impl namespace
namespace LongUI { namespace impl {
    /// <summary>
    /// Queries the interface.
    /// </summary>
    /// <param name="id">The identifier.</param>
    /// <param name="ppvObject">The PPV object.</param>
    /// <returns></returns>
    HRESULT basic_renderer::QueryInterface(const IID& id, void **ppvObject) noexcept {
        IUnknown* ptr = nullptr;
        if (id == IID_IUnknown) ptr = static_cast<IUnknown*>(this);
        else if (id == LongUI::IID_IDWriteTextRenderer) ptr = static_cast<IDWriteTextRenderer*>(this);
        if (*ppvObject = ptr) { this->AddRef(); return S_OK; }
        return E_NOINTERFACE;
    }

    /// <summary>
    /// Determines whether [is pixel snapping disabled] [the specified ].
    /// </summary>
    /// <param name="">The .</param>
    /// <param name="isDisabled">The is disabled.</param>
    /// <returns></returns>
    HRESULT basic_renderer::IsPixelSnappingDisabled(void*, BOOL * isDisabled) noexcept {
        *isDisabled = false;
        return S_OK;
    }

    /// <summary>
    /// Gets the current transform.
    /// </summary>
    /// <param name="ctx">The CTX.</param>
    /// <param name="mat">The mat.</param>
    /// <returns></returns>
    /// <remarks>
    /// 从目标渲染呈现器获取转换矩阵
    /// </remarks>
    HRESULT basic_renderer::GetCurrentTransform(void* ctx, DWRITE_MATRIX* mat) noexcept {
        auto& context = *this->get_context(ctx);
        const auto renderer = context.renderer;
        renderer->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(mat));
        return S_OK;
    }

    /// <summary>
    /// Gets the pixels per dip.
    /// </summary>
    /// <param name="">The .</param>
    /// <param name="bilibili">The bilibili.</param>
    /// <returns></returns>
    HRESULT basic_renderer::GetPixelsPerDip(void*, FLOAT * bilibili) noexcept {
        // 始终如一, 方便转换
        *bilibili = 1.f;
        return S_OK;
    }

    /// <summary>
    /// Draws the inline object.
    /// 渲染内联对象
    /// </summary>
    /// <param name="clientDrawingContext">The client drawing context.</param>
    /// <param name="originX">The origin x.</param>
    /// <param name="originY">The origin y.</param>
    /// <param name="inlineObject">The inline object.</param>
    /// <param name="isSideways">The is sideways.</param>
    /// <param name="isRightToLeft">The is right to left.</param>
    /// <param name="clientDrawingEffect">The client drawing effect.</param>
    /// <returns></returns>
    HRESULT basic_renderer::DrawInlineObject(
        _In_opt_ void* clientDrawingContext,
        FLOAT originX, FLOAT originY,
        _In_ IDWriteInlineObject * inlineObject,
        BOOL isSideways, BOOL isRightToLeft,
        _In_opt_ IUnknown* clientDrawingEffect) noexcept {
        UNREFERENCED_PARAMETER(isSideways);
        UNREFERENCED_PARAMETER(isRightToLeft);
        assert(inlineObject && "bad argument");
        // 渲染
        inlineObject->Draw(
            clientDrawingContext, this,
            originX, originY,
            isSideways, isRightToLeft,
            clientDrawingEffect
        );
        return S_OK;
    }
    /// <summary>
    /// Draws the underline.
    /// </summary>
    /// <param name="ctx">The client drawing context.</param>
    /// <param name="baselineOriginX">The baseline origin x.</param>
    /// <param name="baselineOriginY">The baseline origin y.</param>
    /// <param name="underline">The underline.</param>
    /// <param name="effect">The effect.</param>
    /// <returns></returns>
    HRESULT basic_renderer::DrawUnderline(
        void* ctx,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        //DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
        const DWRITE_UNDERLINE* underline,
        IUnknown* effect ) noexcept {
        auto& context = *this->get_context(ctx);
        const auto renderer = context.renderer;
        // 设置颜色
        auto& brush = UIManager.RefCCBrush(context.color);
        // 计算矩形
        RectF rect = {
            0, underline->offset,
            underline->width,
            underline->offset + underline->thickness
        };
        //rect = impl::rotate(rect, orientationAngle);
        rect = impl::offset(rect, baselineOriginX, baselineOriginY);
        // 填充矩形
        renderer->FillRectangle(auto_cast(&rect), &brush);
        return S_OK;
    }
    /// <summary>
    /// Draws the strikethrough.
    /// </summary>
    /// <param name="ctx">The client drawing context.</param>
    /// <param name="baselineOriginX">The baseline origin x.</param>
    /// <param name="baselineOriginY">The baseline origin y.</param>
    /// <param name="strikethrough">The strikethrough.</param>
    /// <param name="effect">The effect.</param>
    /// <returns></returns>
    HRESULT basic_renderer::DrawStrikethrough(
        void* ctx,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        //DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
        const DWRITE_STRIKETHROUGH* strikethrough,
        IUnknown* effect) noexcept {
        auto& context = *this->get_context(ctx);
        const auto renderer = context.renderer;
        // 设置颜色
        auto& brush = UIManager.RefCCBrush(context.color);
        // 计算矩形
        RectF rect = {
            0, strikethrough->offset,
            strikethrough->width,
            strikethrough->offset + strikethrough->thickness
        };
        //rect = impl::rotate(rect, orientationAngle);
        rect = impl::offset(rect, baselineOriginX, baselineOriginY);
        // 填充矩形
        renderer->FillRectangle(auto_cast(&rect), &brush);
        return S_OK;
    }
}}


/// <summary>
/// Draws the glyph run.
/// </summary>
/// <param name="clientDrawingContext">The client drawing context.</param>
/// <param name="baselineOriginX">The baseline origin x.</param>
/// <param name="baselineOriginY">The baseline origin y.</param>
/// <param name="measuringMode">The measuring mode.</param>
/// <param name="glyphRun">The glyph run.</param>
/// <param name="glyphRunDescription">The glyph run description.</param>
/// <param name="effect">The effect.</param>
/// <returns></returns>
HRESULT LongUI::impl::outline_renderer::DrawGlyphRun(
    void* ctx,
    FLOAT baselineOriginX, FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE measuringMode,
    const DWRITE_GLYPH_RUN * glyphRun,
    const DWRITE_GLYPH_RUN_DESCRIPTION * glyphRunDescription,
    IUnknown * effect) noexcept {
    auto& context = *this->get_context(ctx);
    const auto renderer = context.renderer;
    // 获取填充颜色
    //D2D1_COLOR_F* fill_color = nullptr;
    //// 检查
    //if (effect && impl::same_vtable(effect, &this->basic_color)) {
    //    fill_color = &static_cast<CUIColorEffect*>(effect)->color;
    //}
    //else {
    //    fill_color = &this->basic_color.color;
    //}
    // 获取描边颜色
    //auto outline = reinterpret_cast<OutlineContext*>(clientDrawingContext);
    //D2D1_COLOR_F* draw_color = &outline->color;

    HRESULT hr = S_OK;
    // 创建路径几何
    ID2D1PathGeometry* path_geometry = nullptr;
    auto& raw = CUIResMgr::Ref2DFactory();
    auto& factory = reinterpret_cast<ID2D1Factory&>(raw);
    hr = factory.CreatePathGeometry(&path_geometry);
    // 写入几何体
    ID2D1GeometrySink* sink = nullptr;
    if (SUCCEEDED(hr)) {
        hr = path_geometry->Open(&sink);
    }
    // 描边
    if (SUCCEEDED(hr)) {
        hr = glyphRun->fontFace->GetGlyphRunOutline(
            glyphRun->fontEmSize,
            glyphRun->glyphIndices,
            glyphRun->glyphAdvances,
            glyphRun->glyphOffsets,
            glyphRun->glyphCount,
            glyphRun->isSideways,
            glyphRun->bidiLevel % 2,
            sink
        );
    }
    // 关闭几何体sink
    if (SUCCEEDED(hr)) {
        hr = sink->Close();
    }
    // 渲染
    if (SUCCEEDED(hr)) {
        // 保存状态
        Matrix::Matrix3x2F transform;
        renderer->GetTransform(&auto_cast(transform));
        const auto transform2 = Matrix::Matrix3x2F::Translation(
            baselineOriginX, baselineOriginY
        ) * transform;
        renderer->SetTransform(&auto_cast(transform2));
        // 设置颜色A
        auto& brush1 = UIManager.RefCCBrush(context.outline_color);
        // 刻画描边
        renderer->DrawGeometry(
            path_geometry,
            &brush1,
            context.outline_width
        );
        // 设置颜色
        auto& brush2 = UIManager.RefCCBrush(context.color);
        // 填充字形
        renderer->FillGeometry(path_geometry, &brush2);
        // 回退
        renderer->SetTransform(&auto_cast(transform));
    }
    // 扫尾
    LongUI::SafeRelease(path_geometry);
    LongUI::SafeRelease(sink);
    return hr;
}