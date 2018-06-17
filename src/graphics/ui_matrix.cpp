// Gui
#include <graphics/ui_matrix3x2.h>
#include <util/ui_unimacro.h>
// C++
#include <cstdarg>
#include <cassert>
#include <cstdio>
#include <cmath>

PCN_NOINLINE
/// <summary>
/// Makes the rotate matrix.
/// </summary>
/// <param name="angle">The angle.</param>
/// <param name="center">The center.</param>
/// <param name="matrix">The matrix.</param>
/// <returns></returns>
void LongUI::Matrix::MakeRotateMatrix(
    float angle,
    const Point2F& center,
    Matrix3X2F& matrix) noexcept {
    constexpr auto pi_long = 3.1415926535897932384626433832795L;
    constexpr auto pi = static_cast<float>(pi_long);
    const float theta = angle * (pi / 180.0f);
    const float sin_theta = std::sin(theta);
    const float cos_theta = std::cos(theta);
    matrix._11 = cos_theta;
    matrix._12 = sin_theta;
    matrix._21 = -sin_theta;
    matrix._22 = cos_theta;
    matrix._31 = center.x - center.x * cos_theta + center.y * sin_theta;
    matrix._32 = center.y - center.x * sin_theta - center.y * cos_theta;
}

PCN_NOINLINE
/// <summary>
/// Makes the skew matrix.
/// </summary>
/// <param name="x">The x.</param>
/// <param name="y">The y.</param>
/// <param name="center">The center.</param>
/// <param name="matrix">The matrix.</param>
/// <returns></returns>
void LongUI::Matrix::MakeSkewMatrix(
    float x, float y,
    const Point2F& center,
    Matrix3X2F& matrix) noexcept {
    constexpr auto pi_long = 3.1415926535897932384626433832795L;
    constexpr auto pi = static_cast<float>(pi_long);
    float theta_x = x * (pi / 180.0f);
    float theta_y = y * (pi / 180.0f);
    float tan_x = std::tan(theta_x);
    float tan_y = std::tan(theta_y);
    matrix._11 = 1.;
    matrix._12 = tan_y;
    matrix._21 = tan_x;
    matrix._22 = 1.;
    matrix._31 =-center.y * tan_x;
    matrix._32 = center.x * tan_y ;
}

PCN_NOINLINE
/// <summary>
/// Sets the product.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
void LongUI::Matrix::Matrix3x2F::SetProduct(
    const Matrix3X2F& a, const Matrix3X2F& b) noexcept {
    this->_11 = a._11 * b._11 + a._12 * b._21;
    this->_12 = a._11 * b._12 + a._12 * b._22;
    this->_21 = a._21 * b._11 + a._22 * b._21;
    this->_22 = a._21 * b._12 + a._22 * b._22;
    this->_31 = a._31 * b._11 + a._32 * b._21 + b._31;
    this->_32 = a._31 * b._12 + a._32 * b._22 + b._32;
}

/// <summary>
/// Determines whether this instance is identity.
/// </summary>
/// <returns></returns>
bool LongUI::Matrix::Matrix3x2F::IsIdentity() const noexcept {
    return this->_11 == 1. && this->_12 == 0.
        && this->_21 == 0. && this->_22 == 1.
        && this->_31 == 0. && this->_32 == 0.;
}