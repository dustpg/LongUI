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

#include <Wincodec.h>
#include <d2d1_3.h>
#include <cstdint>

// longui::dx namespace, helper for directx
namespace LongUI { namespace DX {
    // using
    using CPGUID = const GUID*;
    // DXGI_FORMAT to WIC GUID format
    auto DXGIToWIC(DXGI_FORMAT format) noexcept ->CPGUID;
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
}}