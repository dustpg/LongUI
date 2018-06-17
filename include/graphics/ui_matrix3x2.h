#pragma once

#include "../core/ui_basic_type.h"

// LongUI::Matrix namespace
namespace LongUI { namespace Matrix {
    // d2d matrix helper : rotate
    void MakeRotateMatrix(float angle, const Point2F& center, Matrix3X2F& matrix) noexcept;
    // d2d matrix helper : skew
    void MakeSkewMatrix(float x, float y, const Point2F&, Matrix3X2F&) noexcept;
    // Matrix3x2F
    struct Matrix3x2F : Matrix3X2F {
        // ctor
        inline Matrix3x2F(
            float m11, float m12, float m21, 
            float m22, float m31, float m32
        ) noexcept {
            this->_11 = m11; this->_12 = m12; this->_21 = m21;
            this->_22 = m22; this->_31 = m31; this->_32 = m32;
        }
        // ctor
        inline Matrix3x2F() noexcept {};
        // make a identity matrix
        static inline Matrix3x2F Identity() noexcept {
            Matrix3x2F identity;
            identity._11 = 1.; identity._12 = 0.;
            identity._21 = 0.; identity._22 = 1.;
            identity._31 = 0.; identity._32 = 0.;
            return identity;
        }
        // make a translation matrix
        static inline Matrix3x2F Translation(const Size2F& size) noexcept {
            Matrix3x2F translation;
            translation._11 = 1.0; translation._12 = 0.0;
            translation._21 = 0.0; translation._22 = 1.0;
            translation._31 = size.width; translation._32 = size.height;
            return translation;
        }
        // make a translation matrix - overload
        static inline Matrix3x2F Translation(float x, float y) noexcept {
            return Translation(Size2F{ x, y });
        }
        // make a scale matrix 
        static inline Matrix3x2F Scale(const Size2F& size, const Point2F& center = Point2F{0.}) noexcept {
            Matrix3x2F scale;
            scale._11 = size.width; scale._12 = 0.0;
            scale._21 = 0.0; scale._22 = size.height;
            scale._31 = center.x - size.width * center.x;
            scale._32 = center.y - size.height * center.y;
            return scale;
        }
        // make a scale matrix - overload
        static inline Matrix3x2F Scale(float x, float y, const Point2F& center = Point2F{0.}) noexcept {
            return Scale(Size2F{ x, y }, center);
        }
        // make a rotation matrix
        static inline Matrix3x2F Rotation(float angle, const Point2F& center = Point2F{0.}) noexcept {
            Matrix3x2F rotation;
            Matrix::MakeRotateMatrix(angle, center, rotation);
            return rotation;
        }
        // make a skew matrix
        static inline Matrix3x2F Skew(float angleX, float angleY, const Point2F& center = Point2F{0.}) noexcept {
            Matrix3x2F skew;
            Matrix::MakeSkewMatrix(angleX, angleY, center, skew);
            return skew;
        }
        // force cast - const overload
        static inline const Matrix3x2F* ReinterpretBaseType(const Matrix3X2F* pMatrix) noexcept  {
            return static_cast<const Matrix3x2F*>(pMatrix);
        }
        // force cast
        static inline Matrix3x2F* ReinterpretBaseType(Matrix3X2F* pMatrix) noexcept {
            return static_cast<Matrix3x2F*>(pMatrix);
        }
        // Determinant
        inline auto Determinant() const noexcept ->float {
            return (this->_11 * this->_22) - (this->_12 * this->_21);
        }
#if 0
        // is invertible?
        inline bool IsInvertible() const noexcept {
            return !!Dll::D2D1IsMatrixInvertible(this);
            //return !!DX::D2D1IsMatrixInvertible(this);
        }
        // Invert
        inline bool Invert() noexcept {
            return !!Dll::D2D1InvertMatrix(this);
            //return !!DX::D2D1InvertMatrix(this);
        }
#endif
        // is Identity?
        bool IsIdentity() const noexcept;
        // set the product
        void SetProduct(const Matrix3X2F& a, const Matrix3X2F& b) noexcept;
        // operator a*b
        inline Matrix3x2F operator*(const Matrix3X2F &matrix) const noexcept {
            Matrix3x2F result;
            result.SetProduct(*this, matrix);
            return result;
        }
        // Transform Point
        inline Point2F TransformPoint(const Point2F& point) const noexcept {
            Point2F result = {
                point.x * _11 + point.y * _21 + _31,
                point.x * _12 + point.y * _22 + _32
            };
            return result;
        }
    };
}}