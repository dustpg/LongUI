// Gui
#include <core/ui_basic_type.h>
#include <core/ui_color.h>
// C++
#include <algorithm>
#include <cmath>

/// <summary>
/// Transforms the point.
/// </summary>
/// <param name="matrix">The matrix.</param>
/// <param name="point">The point.</param>
/// <returns></returns>
auto LongUI::TransformPoint(const Matrix3X2F& matrix, Point2F point) noexcept -> Point2F {
    return {
        point.x * matrix._11 + point.y * matrix._21 + matrix._31,
        point.x * matrix._12 + point.y * matrix._22 + matrix._32
    };
}

/// <summary>
/// Transforms the point inverse.
/// </summary>
/// <param name="matrix">The matrix.</param>
/// <param name="point">The point.</param>
/// <returns></returns>
auto LongUI::TransformPointInverse(const Matrix3X2F & matrix, Point2F point) noexcept -> Point2F {
    Point2F result;
    // x = (bn-dm) / (bc-ad)
    // y = (an-cm) / (ad-bc)
    // a : m_matrix._11
    // b : m_matrix._21
    // c : m_matrix._12
    // d : m_matrix._22
    auto bc_ad = matrix._21 * matrix._12 - matrix._11 * matrix._22;
    auto m = point.x - matrix._31;
    auto n = point.y - matrix._32;
    result.x = (matrix._21*n - matrix._22 * m) / bc_ad;
    result.y = (matrix._12*m - matrix._11 * n) / bc_ad;
    return result;
}


/// <summary>
/// Determines whether the specified a is overlap.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
bool LongUI::IsOverlap(const RectF & a, const RectF & b) noexcept {
    return a.left < b.right && a.right > b.left &&
        a.top < b.bottom && a.bottom > b.top;
}

/// <summary>
/// Determines whether the specified a is include.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
bool LongUI::IsInclude(const RectF& a, const RectF & b) noexcept {
    return b.left >= a.left && b.top >= a.top &&
        b.right <= a.right && b.bottom <= a.bottom;
}

/// <summary>
/// Determines whether the specified a is include.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
bool LongUI::IsInclude(const RectF& a, const Point2F& b) noexcept {
    return b.x >= a.left && b.x <= a.right
        && b.y >= a.top && b.y <= a.bottom;
}

/// <summary>
/// Gets the area.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
auto LongUI::GetArea(const RectF & rect) noexcept -> float {
    const auto width = rect.right - rect.left;
    const auto height = rect.bottom - rect.top;
    return width * height;
}

/// <summary>
/// Determines whether the specified a is same.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
bool LongUI::IsSame(const RectF& a, const RectF& b) noexcept {
    return a.left == b.left && a.top == b.top &&
        a.right == b.right && a.bottom && b.bottom;
}




/// <summary>
/// Determines whether [is float same in GUI] [the specified a].
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
bool LongUI::IsSameInGuiLevel(float a, float b) noexcept {
    return std::abs(a - b) < 0.001f;
}

/// <summary>
/// Determines whether [is float same in GUI] [the specified a].
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
bool LongUI::IsSameInGuiLevel(Point2F a, Point2F b) noexcept {
    return IsSameInGuiLevel(a.x, b.x) && IsSameInGuiLevel(a.y, b.y);
}

/// <summary>
/// Mixes the specified from.
/// </summary>
/// <param name="from">From.</param>
/// <param name="to">To.</param>
/// <param name="progress">The progress.</param>
/// <returns></returns>
auto LongUI::Mix(Point2F from, Point2F to, float progress) noexcept -> Point2F {
    const auto x0_0 = std::max(std::min(progress, 1.f), 0.f);
    const auto x0_1 = 1.f - x0_0;
    Point2F rv;
    rv.x = from.x * x0_1 + to.x * x0_0;
    rv.y = from.y * x0_1 + to.y * x0_0;
    return rv;
}

/// <summary>
/// Mixes the specified from.
/// </summary>
/// <param name="from">From.</param>
/// <param name="to">To.</param>
/// <param name="progress">The progress.</param>
/// <returns></returns>
auto LongUI::Mix(const RectF & from, const RectF& to, float progress) noexcept {
    const auto& f = reinterpret_cast<const ColorF&>(from);
    const auto& t = reinterpret_cast<const ColorF&>(to);
    auto rv = LongUI::Mix(f, t, progress);
    return reinterpret_cast<ColorF&>(rv);
}

