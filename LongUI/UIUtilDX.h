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

// longui namespace
namespace LongUI {
    // Helper for DirectX
    namespace DX {
        // format the text into textlayout with format: 面向C/C++
        auto FormatTextCore(FormatTextConfig&, const wchar_t*, ...) noexcept->IDWriteTextLayout*;
        // create mesh from geometry
        auto CreateMeshFromGeometry(ID2D1Geometry* geometry, ID2D1Mesh** mesh) noexcept->HRESULT;
        // format the text into core-mode with xml string: 面向数据
        auto XMLToCoreFormat(const char*, wchar_t*) noexcept->bool;
        // format the text into textlayout with format: 面向C/C++
        auto FormatTextCore(FormatTextConfig&, const wchar_t*, va_list) noexcept->IDWriteTextLayout*;
        // save as image file
        auto SaveAsImageFile(ID2D1Bitmap* bitmap, const wchar_t* file_name);
        // get default LongUI imp IDWriteFontCollection
        auto CreateFontCollection(
            IDWriteFactory* factory,
            const wchar_t* filename = L"*.*tf",
            const wchar_t* folder = L"Fonts"
            ) noexcept->IDWriteFontCollection*;
        // create path-geometry from utf-32 char array using text format
        // fontface: (you can see <LongUI::UIScrollBar::UIScrollBar>)
        //          fontface == nullptr, ok but a bit slow
        //          *fontface == nullptr, ok, a bit slow, and out a IDWriteFontFace*, you can use it in next time(same format)
        //          *fontface != nullptr, ok
        auto CreateTextPathGeometry(
            IN const char32_t* utf32_string,
            IN size_t string_length,
            IN IDWriteTextFormat* format,
            IN ID2D1Factory* factory,
            IN OUT OPTIONAL IDWriteFontFace** fontface,
            OUT ID2D1PathGeometry** geometry
            ) noexcept->HRESULT;
    }
}