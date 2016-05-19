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

#include <d2d1_3.h>
#include <cstdint>

// longui namespace
namespace LongUI {
    // Dll Function Helper
    struct Dll {
        // CreateDXGIFactory1@dxgi.dll
        static decltype(&::CreateDXGIFactory1) CreateDXGIFactory1;
        // DWriteCreateFactory@dwrite.dll
        //static decltype(&::DWriteCreateFactory) DWriteCreateFactory;
        // D3D11CreateDevice@d3d11.dll
        //static decltype(&::D3D11CreateDevice) D3D11CreateDevice;
        // D2D1InvertMatrix@d2d1.dll
        static decltype(&::D2D1InvertMatrix) D2D1InvertMatrix;
        // D2D1IsMatrixInvertible@d2d1.dll
        static decltype(&::D2D1IsMatrixInvertible) D2D1IsMatrixInvertible;
        // D2D1CreateFactory@d2d1.dll
        static HRESULT (WINAPI* D2D1CreateFactory)(D2D1_FACTORY_TYPE, REFIID, CONST D2D1_FACTORY_OPTIONS *, void**);
        // DCompositionCreateDevice, Win8 and later hold it
        static HRESULT (STDAPICALLTYPE* DCompositionCreateDevice)(IDXGIDevice*, REFIID, void**);
    };
    // Render Common Brush
    void FillRectWithCommonBrush(ID2D1RenderTarget* target, ID2D1Brush* brush, const D2D1_RECT_F& rect) noexcept;
    // point in rect?
    inline auto IsPointInRect(const D2D1_RECT_F& rect, const D2D1_POINT_2F& pt) noexcept {
        return(pt.x >= rect.left && pt.y >= rect.top && pt.x < rect.right && pt.y < rect.bottom);
    }
    // get transformed pointer
    inline auto TransformPoint(const D2D1_MATRIX_3X2_F& matrix, const D2D1_POINT_2F& point) noexcept {
        D2D1_POINT_2F result = {
            point.x * matrix._11 + point.y * matrix._21 + matrix._31,
            point.x * matrix._12 + point.y * matrix._22 + matrix._32
        };
        return result;
    }
    // get transformed pointer
    auto TransformPointInverse(const D2D1_MATRIX_3X2_F& matrix, const D2D1_POINT_2F& point) noexcept ->D2D1_POINT_2F;
    // pack the color
    auto PackTheColorARGB(D2D1_COLOR_F& IN color) noexcept ->uint32_t;
    // unpack the color
    auto UnpackTheColorARGB(uint32_t IN color32, D2D1_COLOR_F& OUT color4f) noexcept ->void;
}

