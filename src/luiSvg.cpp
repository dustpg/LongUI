#define _WIN32_WINNT 0x0A000001
#include <Graphics/luiGrSvg.h>

namespace LongUI {
    // +=
    inline auto&operator+=(D2D1_POINT_2F& a, const D2D1_POINT_2F&b) noexcept {
        a.x += b.x; a.y += b.y;
        return a;
    }
}
#include <Core/luiManager.h>

// longui::impl
namespace LongUI { namespace impl {
    // chars array for float
    const bool FLOAT_CHARS[128] = {
        0, 0, 0, 0, 0, 0, 0, 0,        0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0,        0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0,        0, 0, 0, 1, 0, 1, 1, 0, 
        1, 1, 1, 1, 1, 1, 1, 1,        1, 1, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 1, 0, 0,        0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0,        0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 1, 0, 0,        0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0,        0, 0, 0, 0, 0, 0, 0, 0,
    };
    // is float char
    inline bool is_float_char(const char ch) noexcept {
        return FLOAT_CHARS[uint8_t(ch) & 127];
    }
    // parse float
    auto parse_float(const char* str, float* f, int c) noexcept {
        // 根据剩余浮点
        for (int i = 0; i < c; ++i) {
            bool done = false;
            const char* begin = str;
            // 处理字符串
            while (const auto ch = *str) {
                // 浮点 -> 切换到浮点处理状态
                if (impl::is_float_char(ch)) { 
                    if (!done) {
                        begin = str;
                        done = true;
                    }
                }
                // 其他 -> 准备退出
                else if(done) break;
                // 推进索引
                ++str;
            }
            // 处理浮点
            auto f2get = f+i; *f2get = 0.f;
            size_t len = str - begin;
            LongUI::SafeBuffer<char>(len + 1, [=](char* buf) noexcept {
                std::memcpy(buf, begin, len); buf[len] = 0;
                *f2get = LongUI::AtoF(buf);
            });
        }
        return str;
    }
    // parse float
    template<int c>
    inline auto parse_float(const char* str, float* f) noexcept {
        return parse_float(str, f, c);
    }
}}



/// <summary>
/// Parsers the path.
/// </summary>
/// <param name="path">The path.</param>
/// <param name="geometry">The geometry.</param>
/// <returns></returns>
auto LongUI::SVG::ParserPath(
    const char* path, 
    ID2D1PathGeometry* geometry) noexcept -> HRESULT {
    assert(path && geometry);
    if (!path || !geometry) return E_INVALIDARG;
    ID2D1GeometrySink* sink = nullptr;
    auto hr = geometry->Open(&sink);
    /*
        M = moveto
        L = lineto
        H = horizontal lineto
        V = vertical lineto
        C = curveto
        S = smooth curveto
        Q = quadratic Bézier curve
        T = smooth quadratic Bézier curveto
        A = elliptical Arc
        Z = closepath
    */
    // 遍历
    if (SUCCEEDED(hr)) {
        // 当前XY坐标
        D2D1_POINT_2F org{ 0.f };
        // 处理参数
        union {
            D2D1_BEZIER_SEGMENT cbs;
            D2D1_QUADRATIC_BEZIER_SEGMENT qbs;
        };
        cbs = { 0.f };
        auto itr = path;
        bool closed = false;
        //bool smooth = false;
        while (const char ch = *itr) {
            ++itr;
            switch (ch)
            {
            case 'M': org = { 0.f, 0.f }; case 'm':
                // M = moveto(x, y)
                itr = impl::parse_float<2>(itr, &cbs.point1.x);
                org += cbs.point1;
                sink->BeginFigure(org, D2D1_FIGURE_BEGIN_FILLED);
                closed = false;
                break;
            case 'L': org = { 0.f, 0.f }; case 'l':
                //  L = lineto(x, y)
                itr = impl::parse_float<2>(itr, &cbs.point1.x);
                org += cbs.point1;
                sink->AddLine(org);
                break;
            case 'H': org.x = 0.f; case 'h':
                // H = horizontal lineto(x)
                itr = impl::parse_float<1>(itr, &cbs.point1.x);
                org.x += cbs.point1.x; 
                sink->AddLine(org);
                break;
            case 'V': org.y = 0.f; case 'v':
                // vertical lineto(y)
                itr = impl::parse_float<1>(itr, &cbs.point1.x);
                org.y += cbs.point1.x; 
                sink->AddLine(org);
                break;
            case 'C': org = { 0.f, 0.f }; case 'c':
                // C = curveto
                itr = impl::parse_float<6>(itr, &cbs.point1.x);
                cbs.point1 += org;
                cbs.point2 += org;
                cbs.point3 += org;
                sink->AddBezier(&cbs);
                org = cbs.point3;
                //smooth = true;
                break;
            case 'S': case 's':
                // smooth curveto
                cbs.point1.x = org.x + org.x - cbs.point2.x;
                cbs.point1.y = org.y + org.y - cbs.point2.y;
                if (ch == 'S') org = { 0.f, 0.f };
                itr = impl::parse_float<4>(itr, &cbs.point2.x);
                cbs.point2 += org;
                cbs.point3 += org;
                //if (!smooth) cbs.point1 = cbs.point2;
                sink->AddBezier(&cbs);
                org = cbs.point3;
                //smooth = true;
                break;
            case 'Q': org = { 0.f, 0.f }; case 'q':
                // Q = quadratic Bézier curve
                itr = impl::parse_float<4>(itr, &qbs.point1.x);
                qbs.point1 += org;
                qbs.point2 += org;
                sink->AddQuadraticBezier(&qbs);
                org = qbs.point2;
                break;
            case 'T': case 't':
                // smooth  quadraticcurveto
                qbs.point1.x = org.x + org.x - qbs.point1.x;
                qbs.point1.y = org.y + org.y - qbs.point1.y;
                if (ch == 'T') org = { 0.f, 0.f };
                itr = impl::parse_float<2>(itr, &qbs.point2.x);
                qbs.point2 += org;
                //if (!smooth) cbs.point1 = cbs.point2;
                sink->AddQuadraticBezier(&qbs);
                org = qbs.point2;
                //smooth = true;
                break;
            case 'A': org = { 0.f, 0.f }; case 'a':
                //  A rx ry x-axis-rotation large-arc-flag sweep-flag  x  y
                //  a rx ry x-axis-rotation large-arc-flag sweep-flag dx dy
            {
                float arcary[7];
                itr = impl::parse_float<7>(itr, arcary);
                D2D1_ARC_SEGMENT arc;
                arc.size.width = arcary[0];
                arc.size.height = arcary[1];
                arc.rotationAngle = arcary[2];
                arc.arcSize = D2D1_ARC_SIZE(arcary[3] != 0.f);
                arc.sweepDirection = D2D1_SWEEP_DIRECTION(arcary[4] != 0.f);
                arc.point.x = org.x + arcary[5];
                arc.point.y = org.y + arcary[6];
                org = arc.point;
                sink->AddArc(&arc);
            }
                break;
            case 'Z': case 'z':
                //  Z/z = closepath()
                sink->EndFigure(D2D1_FIGURE_END_CLOSED);
                closed = true;
                break;
            }
        }
        // 关闭
        if (!closed) {
            sink->EndFigure(D2D1_FIGURE_END_OPEN);
        }
    }
    // 关闭路径
    if (SUCCEEDED(hr)) {
        hr = sink->Close();
    }
    // 扫尾
    LongUI::SafeRelease(sink);
    return hr;
}


/// <summary>
/// Parsers the path.
/// </summary>
/// <param name="path">The path.</param>
/// <param name="out">The out.</param>
/// <returns></returns>
auto LongUI::SVG::ParserPath(
    const char* path, 
    ID2D1PathGeometry1** out) noexcept -> HRESULT {
    assert(path && out);
    if (!path || !out) return E_INVALIDARG;
    ID2D1PathGeometry1* path_geometry = nullptr;
    auto hr = S_OK;
    // 创建对象
    if (SUCCEEDED(hr)) {
        auto hr = UIManager_D2DFactory->CreatePathGeometry(&path_geometry);
    }
    // 解析
    if (SUCCEEDED(hr)) {
        hr = SVG::ParserPath(path, path_geometry);
    }
    // 替换
    if (SUCCEEDED(hr)) *out = path_geometry;
    else LongUI::SafeRelease(path_geometry);
    return hr;
}

