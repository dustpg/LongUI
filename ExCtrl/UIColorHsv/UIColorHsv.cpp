#include "UIColorHsv.h"
#include "HsvEffect.h"
#include <Core/luiManager.h>
#include <algorithm>
#undef min
#undef max

// longui namespace
namespace LongUI {
    // -> 红 RGB(255,0,0)
    // -> 黄 RGB(255,255,0)
    // -> 绿 RGB(0,255,0)
    // -> 青 RGB(0,255,255)
    // -> 蓝 RGB(0,0,255)
    // -> 紫 RGB(255,0,255)
    // -> 红
    D2D1_GRADIENT_STOP HSV_STOPS[] = {
        { 0.f / 6.f, 1.f, 0.f, 0.f, 1.f},
        { 1.f / 6.f, 1.f, 1.f, 0.f, 1.f},
        { 2.f / 6.f, 0.f, 1.f, 0.f, 1.f},
        { 3.f / 6.f, 0.f, 1.f, 1.f, 1.f},
        { 4.f / 6.f, 0.f, 0.f, 1.f, 1.f},
        { 5.f / 6.f, 1.f, 0.f, 1.f, 1.f},
        { 6.f / 6.f, 1.f, 0.f, 0.f, 1.f},
    };
}

// longui::impl
namespace LongUI { namespace impl {
    // cross product
    template<typename T> auto cross_product_2d(T a, T b) noexcept {
        return a.x * b.y - a.y * b.x;
    }
    // get color from hue
    auto get_color_from_hue(float h) noexcept {
        assert(0.f <= h && h < 360.f && "out of range");
        D2D1_COLOR_F c { 1.f, 1.f, 1.f, 1.f };
        constexpr float step = 60.f;
        auto fx = [h, step](float v) noexcept { return (h - v) / step; };
        switch (int(h / 60.f))
        {
        case 0:
            // RGB(255,0,0) - RGB(255,255,0)
            c.r = 1.f;
            c.g = fx(0.f * step);
            c.b = 0.f;
            break;
        case 1:
            // RGB(255,255,0) - RGB(0,255,0)
            c.r = 1.f - fx(1.f * step);
            c.g = 1.f;
            c.b = 0.f;
            break;
        case 2:
            // RGB(0,255,0) - RGB(0,255,255)
            c.r = 0.f;
            c.g = 1.f;
            c.b = fx(2.f * step);
            break;
        case 3:
            // RGB(0,255,255) - 蓝 RGB(0,0,255)
            c.r = 0.f;
            c.g = 1.f - fx(3.f * step);
            c.b = 1.f;
            break;
        case 4:
            // RGB(0,0,255) - RGB(255,0,255)
            c.r = fx(4.f * step);
            c.g = 0.f;
            c.b = 1.f;
            break;
        case 5:
            // RGB(255,0,255) -  RGB(255,0,0)
            c.r = 1.f;
            c.g = 0.f;
            c.b = 1.f - fx(5.f * step);
            break;
        }
        return c;
    }
    // hsv to rgb
    auto hsv2rgb(float h, float s, float v) {
        assert(0.f <= h && h < 360.f && "out of range");
        float      hh, p, q, t, ff;
        D2D1_COLOR_F out{1.f, 1.f, 1.f, 1.f};
        if (s <= 0.f) {
            out.r = v;
            out.g = v;
            out.b = v;
            return out;
        }
        hh = h;
        if (hh >= 360.f) hh = 0.f;
        hh /= 60.f;
        auto i = (long)hh;
        ff = hh - i;
        p = v * (1.f - s);
        q = v * (1.f - (s * ff));
        t = v * (1.f - (s * (1.f - ff)));
        switch(i) {
        case 0:
            out.r = v;
            out.g = t;
            out.b = p;
            break;
        case 1:
            out.r = q;
            out.g = v;
            out.b = p;
            break;
        case 2:
            out.r = p;
            out.g = v;
            out.b = t;
            break;
        case 3:
            out.r = p;
            out.g = q;
            out.b = v;
            break;
        case 4:
            out.r = t;
            out.g = p;
            out.b = v;
            break;
        case 5:
        default:
            out.r = v;
            out.g = p;
            out.b = q;
            break;
        }
        return out;
    }
    // set float to
    inline void safe_set_float(UIControl*ctrl, float v) noexcept {
        if (ctrl) if (ctrl->GetFloat() != v) ctrl->SetFloat(v);
    }
}}

/*
/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIColorHsv::Update() noexcept {
    // 刷新父类
    Super::Update();
}*/

/// <summary>
/// Sets the color.
/// </summary>
/// <param name="in">The input color.</param>
/// <returns></returns>
void LongUI::UIColorHsv::SetColor(const D2D1_COLOR_F& in) noexcept {
    // 无需修改
    if (in.r == m_colorPicked.r && in.g == m_colorPicked.g &&
        in.b == m_colorPicked.b) return;
    m_colorPicked = in;
    struct { float h, s, v; } out;
    // 计算
    auto rgb2hsv = [&in, &out]() noexcept {
        float      min, max, delta;
        min = in.r < in.g ? in.r : in.g;
        min = min < in.b ? min : in.b;
        max = in.r > in.g ? in.r : in.g;
        max = max > in.b ? max : in.b;
        out.v = max;
        delta = max - min;
        if (delta < 0.00001f) {
            out.s = 0.f;
            out.h = 0.f;
            return out;
        }
        if (max > 0.f) {
            out.s = (delta / max);
        }
        else {
            out.s = 0.f;
            out.h = 0.f;
            return out;
        }
        if (in.r >= max) out.h = (in.g - in.b) / delta;
        else if (in.g >= max) out.h = 2.f + (in.b - in.r) / delta;
        else out.h = 4.f + (in.r - in.g) / delta;
        out.h *= 60.f;
        if (out.h < 0.f) out.h += 360.f;
        return out;
    };
    
    rgb2hsv();
    this->SetH(out.h);
    this->SetS(out.s);
    this->SetV(out.v);
}

/// <summary>
/// Sets the hue.
/// </summary>
/// <param name="h">The h.</param>
/// <returns></returns>
void LongUI::UIColorHsv::SetH(float h) noexcept {
    assert(0.f <= h && h < 360.f && "out of range");
    // 不一样再修改
    if (m_fHue != h) {
        // 修改数据
        impl::safe_set_float(m_pCtrlH, m_fHue = h);
        this->value_changed();
        this->InvalidateThis();
        // 上渲染锁
        if (m_pHsvEffect) {
            auto color = impl::get_color_from_hue(h);
            LongUI::CUIDxgiAutoLocker locker;
            auto i = Effect::Hsv::Properties_PickedColor;
            m_pHsvEffect->SetValue(i, color);
        }
    }
}

/// <summary>
/// Sets the s.
/// </summary>
/// <param name="s">The s.</param>
/// <returns></returns>
void LongUI::UIColorHsv::SetS(float s) noexcept {
    assert(0.f <= s && s <= 1.f && "out of range");
    // 不一样再修改
    if (m_fSaturation != s) {
        impl::safe_set_float(m_pCtrlS, m_fSaturation = s);
        this->value_changed();
        this->InvalidateThis();
    }
}


/// <summary>
/// Sets the v.
/// </summary>
/// <param name="v">The v.</param>
/// <returns></returns>
void LongUI::UIColorHsv::SetV(float v) noexcept {
    assert(0.f <= v && v <= 1.f && "out of range");
    // 不一样再修改
    if (m_fValue != v) {
        impl::safe_set_float(m_pCtrlV, m_fValue = v);
        this->value_changed();
        this->InvalidateThis();
    }
}

/// <summary>
/// Render_chain_mains this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIColorHsv::render_chain_main() const noexcept {
    // 父类渲染
    Super::render_chain_main();
    // 常量准备
    constexpr float cr = BASIC_HSV_CIRCLE_RADIUS;
    constexpr float cw = BASIC_HSV_CIRCLE_WIDTH;
    constexpr float tris = (cr - cw*0.5f) / BASIC_HSV_TRIANGLE_RADIUS;
    constexpr float hsvr1 = float(Effect::Hsv::UNIT*1);
    constexpr float hsvr2 = float(Effect::Hsv::UNIT*2);
    // 获取渲染矩形
    D2D1_RECT_F rect; this->GetViewRect(rect);
    const auto wd2= (rect.right - rect.left)* 0.5f;
    const auto hd2= (rect.bottom - rect.top)* 0.5f;
    const auto r = std::min(wd2, hd2);
    const auto s = r / (cr + cw * 0.5f);
    // 计算转换矩阵
    const D2D1_MATRIX_3X2_F hsv_circle = DX::Matrix3x2F::Scale(
        D2D1_SIZE_F{s, s}, D2D1_POINT_2F{0}
    ) * DX::Matrix3x2F::Translation(wd2, hd2);
    // 圆环
    D2D1_ELLIPSE circle{ {0.f, 0.f}, cr, cr };
    UIManager_RenderTarget->SetTransform(hsv_circle * this->world);
    m_pHsvBrush->SetTransform(DX::Matrix3x2F::Translation(-hsvr2, -hsvr2));
    UIManager_RenderTarget->DrawEllipse(&circle, m_pHsvBrush, cw);
    // 内部旋转矩阵
    UIManager_RenderTarget->SetTransform(
        hsv_circle * DX::Matrix3x2F::Rotation(
            m_fHue, D2D1_POINT_2F{wd2, hd2}
        ) * this->world
    );
    // 选择线条
    m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
    UIManager_RenderTarget->DrawLine(
        D2D1_POINT_2F{0.f, -cr - cw*0.5f},
        D2D1_POINT_2F{0.f, -cr + cw*0.5f},
        m_pBrush_SetBeforeUse,
        BASIC_HSV_LINE_WIDTH
    );
    // 三角形
    m_pHsvBrush->SetTransform(
        DX::Matrix3x2F::Scale(D2D1_SIZE_F{tris,tris}, D2D1_POINT_2F{hsvr2, hsvr2}) 
        *  DX::Matrix3x2F::Translation(-hsvr2, -hsvr2) 
    ) ;
    UIManager_RenderTarget->DrawGeometryRealization(m_pTriangle, m_pHsvBrush);
    // 颜色选择环
    {
        /*
            x = (s - 0.5) * sqrt(3) * v * R
            y =-(1 - 1.5 * v) * R

            xx = cos_120d * x + sin_120d * y;
            yy =-sin_120d * x + cos_120d * y;
        */
        constexpr float r = (cr - cw*0.5f);
        constexpr float sqrt3 = 1.7320508f;
        constexpr float cos_120d = -0.5f;
        constexpr float sin_120d = 0.8660254f;
        const float s = m_fSaturation;
        const float v = m_fValue;
        const float x = (s - 0.5f) * sqrt3 * v * r;
        const float y = -(1.f - 1.5f * v) * r;
        circle.point.x = cos_120d * x + sin_120d * y;
        circle.point.y =-sin_120d * x + cos_120d * y;
        constexpr float r1 = BASIC_PICK_CIRCLE_RADIUS;
        constexpr float r2 = BASIC_PICK_CIRCLE_RADIUS - 1.f;
        circle.radiusX = r1; circle.radiusY = r1;
        UIManager_RenderTarget->DrawEllipse(&circle, m_pBrush_SetBeforeUse);
        circle.radiusX = r2; circle.radiusY = r2;
        m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(D2D1::ColorF::White));
        UIManager_RenderTarget->DrawEllipse(&circle, m_pBrush_SetBeforeUse);
    }
    // 回滚矩阵
    UIManager_RenderTarget->SetTransform(&this->world);


    //UIManager_RenderTarget->DrawImage(m_pHsvOutput);

    /*D2D1_MATRIX_3X2_F old;

    UIManager_RenderTarget->GetTransform(&old);
    UIManager_RenderTarget->SetTransform(DX::Matrix3x2F::Translation(
        D2D1_SIZE_F{256.f, 256.f}
    ));
    UIManager_RenderTarget->SetTransform(DX::Matrix3x2F::Scale(
        D2D1_SIZE_F{2.f, 2.f},
        D2D1_POINT_2F{-512.f, -512.f}
    ));
    UIManager_RenderTarget->DrawImage(m_pHsvOutput);
    UIManager_RenderTarget->SetTransform(&old);*/

    /*D2D1_POINT_2F c;
    c.x = (rect.left + rect.right) * 0.5f;
    c.y = (rect.top + rect.bottom) * 0.5f;
    UIManager_RenderTarget->DrawEllipse(
        D2D1::Ellipse(c, 20.f, 20.f),
        m_pHsvBrush,
        5.f
    );*/
}


/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIColorHsv::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}


/// <summary>
/// Does the event.
/// </summary>
/// <param name="arg">The argument.</param>
/// <returns></returns>
bool LongUI::UIColorHsv::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // LONGUI 事件
    switch (arg.event)
    {
    case LongUI::Event::Event_TreeBuildingFinished:
        // 检查控件
        if (!m_strH.empty()) {
            m_pCtrlH = m_pWindow->FindControl(m_strH.c_str());
        }
        if (!m_strS.empty()) {
            m_pCtrlS = m_pWindow->FindControl(m_strS.c_str());
        }
        if (!m_strV.empty()) {
            m_pCtrlV = m_pWindow->FindControl(m_strV.c_str());
        }
        if (!m_strR.empty()) {
            m_pCtrlR = m_pWindow->FindControl(m_strR.c_str());
        }
        if (!m_strG.empty()) {
            m_pCtrlG = m_pWindow->FindControl(m_strG.c_str());
        }
        if (!m_strB.empty()) {
            m_pCtrlB = m_pWindow->FindControl(m_strB.c_str());
        }
        return true;
    }
    return Super::DoEvent(arg);
}

/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="arg">The argument.</param>
/// <returns></returns>
bool LongUI::UIColorHsv::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    // 禁用状态禁用鼠标消息
    if (!this->GetEnabled()) return true;
    constexpr float cr = BASIC_HSV_CIRCLE_RADIUS;
    constexpr float cw = BASIC_HSV_CIRCLE_WIDTH;
    constexpr float pi = 3.14159265f;
    constexpr float r = cr - cw * 0.5f;
    constexpr float sin60 = 0.8660254f;
    constexpr float cos60 = 0.5f;
    constexpr float sqrt3 = 1.7320508f;
    // 转换坐标
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(
        this->world, D2D1_POINT_2F{ arg.ptx, arg.pty }
    );
    switch (arg.event)
    {
    case LongUI::MouseEvent::Event_MouseWheelV:
    case LongUI::MouseEvent::Event_MouseWheelH:
    {
        float fa = UIInput.IsKbPressed(UIInput.KB_CONTROL) ? 60.f : 6.f;
        auto h = m_fHue - arg.wheel.delta * fa;
        if (h < 0.f) h += 360.f; 
        else if (h >= 360.f) h -= 360.f;
        this->SetH(h);
    }
    case LongUI::MouseEvent::Event_DragEnter:
        break;
    case LongUI::MouseEvent::Event_DragOver:
        break;
    case LongUI::MouseEvent::Event_DragLeave:
        break;
    case LongUI::MouseEvent::Event_Drop:
        break;
    case LongUI::MouseEvent::Event_MouseEnter:
        break;
    case LongUI::MouseEvent::Event_MouseLeave:
        break;
    case LongUI::MouseEvent::Event_MouseHover:
        break;
    case LongUI::MouseEvent::Event_MouseMove:
        // 点中
        if (UIInput.IsMbPressed(UIInput.MB_L)) {
            // 计算基本数据
            D2D1_RECT_F rect; this->GetViewRect(rect);
            auto cx = (rect.left + rect.right) * 0.5f;
            auto cy = (rect.top + rect.bottom) * 0.5f;
            const auto r1 = std::min(cx, cy);
            const auto r2 = r1 * ((cr - cw) / cr);
            const auto px = (pt4self.x - cx);
            const auto py = (pt4self.y - cy);
            switch (m_tyClicked)
            {
            case LongUI::UIColorHsv::Type_Ring:
                // 点中HSV环
            {
                const auto h = std::atan2(px, -py) / pi * 180.f;
                this->SetH(h < 0.f ? h + 360.f : h);
            }
                break;
            case LongUI::UIColorHsv::Type_Trianle:
                // 点中中间三角
            {
                // 计算缩放前位置
                const float zoom = r1 / (cr + cw * 0.5f);
                const float zpx = px / zoom;
                const float zpy =-py / zoom;
                // 计算旋转前位置
                const float theta = (120.f - m_fHue) * (pi / 180.0f);
                const float sin_theta = std::sin(theta);
                const float cos_theta = std::cos(theta);
                const float tx = (zpx * cos_theta + zpy * sin_theta);
                const float ty = (zpx * sin_theta - zpy * cos_theta);
                // 计算SV
                const float v = std::min(std::max(0.f, (1.f + ty / r) / 1.5f), 1.f);
                const float s = std::min(std::max(0.f,  tx / (v*r*sqrt3) + 0.5f), 1.f);
                this->SetV(v); this->SetS(s);
            }
                break;
            }
        }
        return true;
    case LongUI::MouseEvent::Event_LButtonDown:
    {
        m_pWindow->SetCapture(this);
        D2D1_RECT_F rect; this->GetViewRect(rect);
        auto cx = (rect.left + rect.right) * 0.5f;
        auto cy = (rect.top + rect.bottom) * 0.5f;
        const auto r1 = std::min(cx, cy);
        const auto r2 = r1 * ((cr - cw) / cr);
        const auto px = (pt4self.x - cx);
        const auto py = (pt4self.y - cy);
        // 以免错误
        if (px == 0.f && py == 0.f) {
            m_tyClicked = this->Type_Trianle;
            return true;
        }
        const auto h = std::atan2(px, -py) / pi * 180.f;
        // 计算是否点中
        const auto r1sq = r1 * r1;
        const auto r2sq = r2 * r2;
        const auto pxsq = px * px;
        const auto pysq = py * py;
        const auto psqs = pxsq + pysq;
        // 点中圆环
        if (r2sq <= psqs && psqs <= r1sq) {
            m_tyClicked = this->Type_Ring;
            this->SetH(h < 0.f ? h + 360.f : h);
        }
        else {
            // 计算缩放前位置
            const float zoom = r1 / (cr + cw * 0.5f);
            const float zpx = px / zoom;
            const float zpy =-py / zoom;
            // 计算旋转前位置
            const float theta = (120.f - m_fHue) * (pi / 180.0f);
            const float sin_theta = std::sin(theta);
            const float cos_theta = std::cos(theta);
            const float tx = (zpx * cos_theta + zpy * sin_theta);
            const float ty = (zpx * sin_theta - zpy * cos_theta);
            // 计算是否在三角形内部
            const D2D1_VECTOR_2F v1{ tx, ty + r };
            const D2D1_VECTOR_2F v2{ tx - sin60*r, ty - cos60*r };
            const D2D1_VECTOR_2F v3{ tx + sin60*r, ty - cos60*r };
            const float t1 = impl::cross_product_2d(v1, v2);
            const float t2 = impl::cross_product_2d(v2, v3);
            const float t3 = impl::cross_product_2d(v3, v1);
            // 点中三角形
            if (t1*t2 >= 0.f && t1*t3 >= 0.f) {
                // 计算SV
                const float v = (1.f + ty / r) / 1.5f;
                const float s = tx / (v*r*sqrt3) + 0.5f;
                this->SetV(v); this->SetS(s);
                m_tyClicked = this->Type_Trianle;
            }
            // 什么都没有
            else {
                m_tyClicked = this->Type_Node;
            }
        }
        return true;
    }
    case LongUI::MouseEvent::Event_LButtonUp:
        m_pWindow->ReleaseCapture();
        return true;
    case LongUI::MouseEvent::Event_RButtonDown:
        break;
    case LongUI::MouseEvent::Event_RButtonUp:
        break;
    case LongUI::MouseEvent::Event_MButtonDown:
        break;
    case LongUI::MouseEvent::Event_MButtonUp:
        break;
    }
    // 未处理的消息
    return false;
}


/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::UIColorHsv::Recreate() noexcept ->HRESULT {
    this->release_gpu_resource();
    auto hr = S_OK;
    ID2D1GradientStopCollection* coll = nullptr;
    ID2D1PathGeometry1* path = nullptr;
    ID2D1GeometrySink* sink = nullptr;
    // 重建父类
    if (SUCCEEDED(hr)) {
        hr = Super::Recreate();
    }
    // 重建渐变集
    if (SUCCEEDED(hr)) {
        hr = UIManager_RenderTarget->CreateGradientStopCollection(
            HSV_STOPS, lengthof<uint32_t>(HSV_STOPS), &coll
        );
    }
    // 创建笔刷
    if (SUCCEEDED(hr)) {
        D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES lgbp;
        lgbp.startPoint = { 0.f, 0.f };
        lgbp.endPoint = { 1000.f, 1000.f };
        hr = UIManager_RenderTarget->CreateLinearGradientBrush(
            &lgbp,
            nullptr,
            coll,
            &m_pHsvBrushLG
        );
    }
    // 创建效果
    if (SUCCEEDED(hr)) {
        hr = UIManager_RenderTarget->CreateEffect(
            LongUI::CLSID_HsvEffect,
            //CLSID_D2D1Flood,
            &m_pHsvEffect
        );
        longui_debug_hr(hr, L"CreateEffect failed");
    }
    // 获取输出
    if (SUCCEEDED(hr)) {
        m_pHsvEffect->GetOutput(&m_pHsvOutput);
    }
    // 创建效果笔刷
    if (SUCCEEDED(hr)) {
        D2D1_IMAGE_BRUSH_PROPERTIES ibp;
        constexpr float a = -float(Effect::Hsv::UNIT * 2);
        constexpr float b = float(Effect::Hsv::UNIT * 2);
        ibp.sourceRectangle = { a,a,b,b };
        ibp.extendModeX = D2D1_EXTEND_MODE_CLAMP;
        ibp.extendModeY = D2D1_EXTEND_MODE_CLAMP;
        ibp.interpolationMode = D2D1_INTERPOLATION_MODE_LINEAR;
        hr = UIManager_RenderTarget->CreateImageBrush(
            m_pHsvOutput, &ibp, nullptr, &m_pHsvBrush
        );
    }
    // 创建三角几何
    if (SUCCEEDED(hr)) {
        hr = UIManager_D2DFactory->CreatePathGeometry(&path);
    }
    // 打开路径记录
    if (SUCCEEDED(hr)) {
        hr = path->Open(&sink);
    }
    // 添加三角形并关闭
    if (SUCCEEDED(hr)) {
        constexpr float c = BASIC_HSV_CIRCLE_RADIUS;
        constexpr float w = BASIC_HSV_CIRCLE_WIDTH;
        constexpr float r = c - w * 0.5f;
        sink->BeginFigure(D2D1_POINT_2F{ 0.f,-r }, D2D1_FIGURE_BEGIN_FILLED);
        constexpr float sin60 = 0.8660254f;
        constexpr float cos60 = 0.5f;
        sink->AddLine(D2D1_POINT_2F{ sin60*r, cos60*r });
        sink->AddLine(D2D1_POINT_2F{-sin60*r, cos60*r });
        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        hr = sink->Close();
    }
    // 创建几何实现
    if (SUCCEEDED(hr)) {
        hr = UIManager_RenderTarget->CreateFilledGeometryRealization(
            path, D2D1_DEFAULT_FLATTENING_TOLERANCE, &m_pTriangle
        );
    }
    // 扫尾处理
    LongUI::SafeRelease(coll);
    LongUI::SafeRelease(path);
    LongUI::SafeRelease(sink);
    return hr;
}

/// /// <summary>
/// Initializes with specified xml-node.
/// </summary>
/// <param name="node">The xml-node.</param>
/// <returns></returns>
void LongUI::UIColorHsv::initialize(pugi::xml_node node) noexcept {
    // 链式初始化
    Super::initialize(node);
    // h显示控件
    if (const auto str = node.attribute("h").value()) {
        m_strH = m_pWindow->CopyStringSafe(str);
    }
    // s显示控件
    if (const auto str = node.attribute("s").value()) {
        m_strS = m_pWindow->CopyStringSafe(str);
    }
    // v显示控件
    if (const auto str = node.attribute("v").value()) {
        m_strV = m_pWindow->CopyStringSafe(str);
    }
    // r显示控件
    if (const auto str = node.attribute("r").value()) {
        m_strR = m_pWindow->CopyStringSafe(str);
    }
    // g显示控件
    if (const auto str = node.attribute("g").value()) {
        m_strG = m_pWindow->CopyStringSafe(str);
    }
    // b显示控件
    if (const auto str = node.attribute("b").value()) {
        m_strB = m_pWindow->CopyStringSafe(str);
    }
    m_colorPicked = D2D1::ColorF(D2D1::ColorF::Red);
}


/// <summary>
/// Creates the control.
/// </summary>
/// <param name="type">The type.</param>
/// <param name="node">The node.</param>
/// <returns></returns>
auto LongUI::UIColorHsv::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UIColorHsv* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
    {
        auto hr = Effect::Hsv::Register(UIManager_D2DFactory);
        auto hrcode = static_cast<size_t>(hr);
        pControl = reinterpret_cast<UIColorHsv*>(hrcode);
    }
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIColorHsv, pControl, type, node);
    }
    return pControl;
}


/// <summary>
/// Release_gpu_resources this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIColorHsv::release_gpu_resource() noexcept {
    LongUI::SafeRelease(m_pHsvBrushLG);
    LongUI::SafeRelease(m_pHsvEffect);
    LongUI::SafeRelease(m_pHsvOutput);
    LongUI::SafeRelease(m_pHsvBrush);
    LongUI::SafeRelease(m_pTriangle);
}

/// <summary>
/// H,S,V changed
/// </summary>
/// <returns></returns>
void LongUI::UIColorHsv::value_changed() noexcept {
    // 计算RGB
    m_colorPicked = impl::hsv2rgb(m_fHue, m_fSaturation, m_fValue);
    // R
    if (m_pCtrlR) m_pCtrlR->SetFloat(m_colorPicked.r);
    // G
    if (m_pCtrlG) m_pCtrlG->SetFloat(m_colorPicked.g);
    // B
    if (m_pCtrlB) m_pCtrlB->SetFloat(m_colorPicked.b);
    // 调用事件
    this->CallUiEvent(m_event, SubEvent::Event_ValueChanged);
}

/// <summary>
/// add event listener
/// </summary>
/// <param name="sb">The sb.</param>
/// <param name="call">The call.</param>
/// <returns></returns>
bool LongUI::UIColorHsv::uniface_addevent(SubEvent sb, UICallBack&& call) noexcept {
    if (sb == SubEvent::Event_ValueChanged) {
        m_event += std::move(call);
        return true;
    }
    return Super::uniface_addevent(sb, std::move(call));
}

/// <summary>
/// Cleanups this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIColorHsv::cleanup() noexcept {
    // 删除前
    this->before_deleted();
    // 删除
    delete this;
}


#ifdef LongUIDebugEvent
/// <summary>
/// [debug only]
/// </summary>
/// <param name="info">The information.</param>
/// <returns></returns>
bool LongUI::UIColorHsv::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIColorHsv";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIColorHsv";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIColorHsv>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// 重载?特例化 GetIID
template<> const IID& LongUI::GetIID<LongUI::UIColorHsv>() noexcept {
    // {BEB929FF-5FA8-4ED5-AEF5-ADC8C387F523}
    static const GUID IID_LongUI_UIColorHsv = {
        0xbeb929ff, 0x5fa8, 0x4ed5, { 0xae, 0xf5, 0xad, 0xc8, 0xc3, 0x87, 0xf5, 0x23 } 
    };
    return IID_LongUI_UIColorHsv;
}
#endif