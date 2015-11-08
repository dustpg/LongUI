#pragma once
/**
* Copyright (c) 2014-2015 dustpg   mailto:dustpg@gmail.com
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

// longui::dx namespace, helper for directx
namespace LongUI { namespace DX {
    // using
    using CPGUID = const GUID*;
    // DXGI_FORMAT to WIC GUID format
    auto DXGIToWIC(DXGI_FORMAT format) noexcept ->CPGUID;
    // create mesh from geometry
    auto CreateMeshFromGeometry(ID2D1Geometry* geometry, ID2D1Mesh** mesh) noexcept ->HRESULT;
    // format the text in C++ within core string
    auto FormatTextCoreC(const FormatTextConfig&, const wchar_t*, ...) noexcept ->IDWriteTextLayout*;
    // format the text within core string
    auto FormatTextCore(const FormatTextConfig&, const wchar_t*, va_list=nullptr) noexcept ->IDWriteTextLayout*;
    // format the text within xml string
    auto FormatTextXML(const FormatTextConfig&, const wchar_t*) noexcept ->IDWriteTextLayout*;
    // get default LongUI imp  IDWriteFontCollection
    auto CreateFontCollection(
        const wchar_t* filename = L"*.*tf",
        const wchar_t* folder = L"Fonts",
        bool include_system = true
        ) noexcept ->IDWriteFontCollection*;
    // create path-geometry from utf-32 char array using text format
    // fontface: (you can see <LongUI::UIScrollBar::UIScrollBar>)
    //          fontface == nullptr, ok but a bit slow
    //          *fontface == nullptr, ok, a bit slow, and out a IDWriteFontFace*, you can use it in next time(same format)
    //          *fontface != nullptr, ok
    auto CreateTextPathGeometry(
        IN const char32_t* utf32_string,
        IN uint32_t string_length,
        IN IDWriteTextFormat* format,
        IN ID2D1Factory* factory,
        IN OUT OPTIONAL IDWriteFontFace** fontface,
        OUT ID2D1PathGeometry** geometry
        ) noexcept ->HRESULT;
    // properties for creating text format, file data friendly
    struct TextFormatProperties {
        // text size
        float           size;
        // tab width, 0.f for default(size x 4)
        float           tab;
        // weight 0~1000 in uint16_t
        uint16_t        weight;
        // style 0~x in uint8_t
        uint8_t         style;
        // stretch 0~x in uint8_t
        uint8_t         stretch;
        // valign 0~x in uint8_t
        uint8_t         valign;
        // halign 0~x in uint8_t
        uint8_t         halign;
        // flow direction 0~x in uint8_t
        uint8_t         flow;
        // reading direction 0~x in uint8_t
        uint8_t         reading;
        // word wrapping
        uint32_t        wrapping;
        // font name, maybe use 0 instead of 4
        wchar_t         name[4];
    };
    // init TextFormatProperties
    void InitTextFormatProperties(TextFormatProperties& prop, size_t name_buf_len) noexcept;
    // create text format with properties
    auto CreateTextFormat(const TextFormatProperties&, IDWriteTextFormat** OUT fmt) noexcept ->HRESULT;
    // make text format with xml node, out "*fmt" maybe SAME as template_fmt if no changes
    auto MakeTextFormat(
        IN pugi::xml_node node, 
        OUT IDWriteTextFormat** fmt, 
        IN OPTIONAL IDWriteTextFormat* template_fmt = nullptr, 
        IN OPTIONAL const char* prefix=nullptr
        ) noexcept ->HRESULT;
    // properties for saving image file
    struct SaveAsImageFileProperties {
        // data for bitmap
        uint8_t*                bits;
        // factory for WIC
        IWICImagingFactory*     factory;
        // format for source data, default: GUID_WICPixelFormat32bppBGRA
        const GUID*             data_format;
        // format for container, default: GUID_ContainerFormatPng
        const GUID*             container_format;
        // width of image
        uint32_t                width;
        // height of image
        uint32_t                height;
        // pitch of image
        uint32_t                pitch;
        // unused
        uint32_t                unused;
    };
    // save as image file with d2d-bitmap
    auto SaveAsImageFile(ID2D1Bitmap1*, IWICImagingFactory*, const wchar_t* file_name, const GUID* = nullptr) noexcept ->HRESULT;
    // save as image file in raw data
    auto SaveAsImageFile(const SaveAsImageFileProperties& , const wchar_t* file_name) noexcept -> HRESULT;
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
            assert(!"NO IMPL!");
            Matrix3x2F skew;
            //DX::D2D1MakeSkewMatrix(angleX, angleY, center, skew);
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
            assert(!"NO IMPL!");
            return false;
            //return !!DX::D2D1IsMatrixInvertible(this);
        }
        // Invert
        inline bool Invert() noexcept {
            assert(!"NO IMPL!");
            return false;
            //return !!D2D1InvertMatrix(this);
        }

        COM_DECLSPEC_NOTHROW
            inline
            bool
            IsIdentity() const
        {
            return     _11 == 1.f && _12 == 0.f
                && _21 == 0.f && _22 == 1.f
                && _31 == 0.f && _32 == 0.f;
        }

        COM_DECLSPEC_NOTHROW
            inline
            void SetProduct(
                const Matrix3x2F &a,
                const Matrix3x2F &b
                )
        {
            _11 = a._11 * b._11 + a._12 * b._21;
            _12 = a._11 * b._12 + a._12 * b._22;
            _21 = a._21 * b._11 + a._22 * b._21;
            _22 = a._21 * b._12 + a._22 * b._22;
            _31 = a._31 * b._11 + a._32 * b._21 + b._31;
            _32 = a._31 * b._12 + a._32 * b._22 + b._32;
        }

        COM_DECLSPEC_NOTHROW
            D2D1FORCEINLINE
            Matrix3x2F
            operator*(
                const Matrix3x2F &matrix
                ) const
        {
            Matrix3x2F result;

            result.SetProduct(*this, matrix);

            return result;
        }

        COM_DECLSPEC_NOTHROW
            D2D1FORCEINLINE
            D2D1_POINT_2F
            TransformPoint(
                D2D1_POINT_2F point
                ) const
        {
            D2D1_POINT_2F result =
            {
                point.x * _11 + point.y * _21 + _31,
                point.x * _12 + point.y * _22 + _32
            };

            return result;
        }
    };
}}