#pragma once
/**
* Copyright (c) 2014-2016 dustpg   mailto:dustpg@gmail.com
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/


#include "luiGrUtil.h"

// longui::helper namespace
namespace LongUI { namespace Helper {
    // make color form string
    bool MakeColor(const char* str, D2D1_COLOR_F& colot) noexcept;
    // make state-based color, first alpha in 0.0 - 1.0 will not be set default color
    bool MakeStateBasedColor(pugi::xml_node node, const char* prefix, D2D1_COLOR_F color[4]) noexcept;
    // make meta group
    bool MakeMetaGroup(pugi::xml_node node, const char* prefix, uint16_t fary[], uint32_t count) noexcept;
    // make border color form xml-node
    void SetBorderColor(pugi::xml_node, D2D1_COLOR_F[/*4*/]) noexcept;
    // make border color
    void SetBorderColor(D2D1_COLOR_F[/*4*/]) noexcept;
}}

// longui::dx namespace, helper for directx
namespace LongUI { namespace DX {
    // create mesh from geometry
    auto CreateMeshFromGeometry(ID2D1Geometry* geometry, ID2D1Mesh** mesh) noexcept ->HRESULT;
    // d2d matrix helper : rotate
    void D2D1MakeRotateMatrix(float angle, D2D1_POINT_2F center, D2D1_MATRIX_3X2_F& matrix) noexcept;
    // Matrix3x2F
    struct Matrix3x2F : D2D1_MATRIX_3X2_F {
        // ctor
        inline Matrix3x2F(FLOAT m11, FLOAT m12, FLOAT m21, FLOAT m22, FLOAT m31, FLOAT m32) noexcept {
            this->_11 = m11; this->_12 = m12; this->_21 = m21;
            this->_22 = m22; this->_31 = m31; this->_32 = m32;
        }
        // ctor
        inline Matrix3x2F() noexcept {};
        // make a identity matrix
        static inline auto Identity() noexcept {
            Matrix3x2F identity;
            identity._11 = 1.f; identity._12 = 0.f;
            identity._21 = 0.f; identity._22 = 1.f;
            identity._31 = 0.f; identity._32 = 0.f;
            return identity;
        }
        // make a translation matrix
        static inline auto Translation(D2D1_SIZE_F size) noexcept {
            Matrix3x2F translation;
            translation._11 = 1.0; translation._12 = 0.0;
            translation._21 = 0.0; translation._22 = 1.0;
            translation._31 = size.width; translation._32 = size.height;
            return translation;
        }
        // make a translation matrix - overload
        static inline auto Translation(float x, float y) noexcept {
            return Translation(D2D1::SizeF(x, y));
        }
        // make a scale matrix 
        static inline auto Scale(D2D1_SIZE_F size, D2D1_POINT_2F center = D2D1::Point2F()) noexcept {
            Matrix3x2F scale;
            scale._11 = size.width; scale._12 = 0.0;
            scale._21 = 0.0; scale._22 = size.height;
            scale._31 = center.x - size.width * center.x;
            scale._32 = center.y - size.height * center.y;
            return scale;
        }
        // make a scale matrix - overload
        static inline auto Scale(FLOAT x, FLOAT y, D2D1_POINT_2F center = D2D1::Point2F()) noexcept {
            return Scale(D2D1::SizeF(x, y), center);
        }
        // make a rotation matrix
        static inline auto Rotation(FLOAT angle, D2D1_POINT_2F center = D2D1::Point2F()) noexcept {
            Matrix3x2F rotation;
            DX::D2D1MakeRotateMatrix(angle, center, rotation);
            return rotation;
        }
        // make a skew matrix
        static inline auto Skew(FLOAT angleX, FLOAT angleY,D2D1_POINT_2F center = D2D1::Point2F()) noexcept {
            Matrix3x2F skew;
            Dll::D2D1MakeSkewMatrix(angleX, angleY, center, &skew);
            return skew;
        }
        // force cast - const overload
        static inline auto ReinterpretBaseType(const D2D1_MATRIX_3X2_F* pMatrix) noexcept  {
            return static_cast<const Matrix3x2F*>(pMatrix);
        }
        // force cast
        static inline auto ReinterpretBaseType(D2D1_MATRIX_3X2_F* pMatrix) noexcept {
            return static_cast<Matrix3x2F*>(pMatrix);
        }
        // Determinant
        inline auto Determinant() const noexcept ->FLOAT {
            return (this->_11 * this->_22) - (this->_12 * this->_21);
        }
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
        // is Identity?
        inline bool IsIdentity() const noexcept {
            return this->_11 == 1.f && this->_12 == 0.f
                && this->_21 == 0.f && this->_22 == 1.f
                && this->_31 == 0.f && this->_32 == 0.f;
        }
        // set the product
        void SetProduct(const D2D1_MATRIX_3X2_F& a, const D2D1_MATRIX_3X2_F& b) noexcept;
        // operator a*b
        inline auto operator*(const Matrix3x2F &matrix) const noexcept {
            Matrix3x2F result;
            result.SetProduct(*this, matrix);
            return result;
        }
        // Transform Point
        inline auto TransformPoint(D2D1_POINT_2F point) const noexcept {
            D2D1_POINT_2F result = {
                point.x * _11 + point.y * _21 + _31,
                point.x * _12 + point.y * _22 + _32
            };
            return result;
        }
    };
}}