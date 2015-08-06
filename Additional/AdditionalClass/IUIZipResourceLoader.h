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

#define LONGUI_WITH_DEFAULT_HEADER
#include <LongUI/LongUI.h>
#include <wincodec.h>
#define MINIZ_HEADER_FILE_ONLY
#include "miniz.c"
// create loader
extern "C" HRESULT CreateZipResourceLoader(
    LongUI::CUIManager& manager,
    const wchar_t* file_name,
    LongUI::IUIResourceLoader** outdata
    ) noexcept;
// longui namespace
namespace LongUI {
    /// <summary>
    /// ResourceLoader for Zip FIle
    /// </summary>
    /// <remarks>
    /// resource file name must be ascii only
    /// 资源文件名 ASCII ONLY
    /// </remarks>
    class CUIZipResourceLoader final : public IUIResourceLoader {
    public:
        // qi
        auto STDMETHODCALLTYPE QueryInterface(const IID& riid, void** ppvObject) noexcept->HRESULT override final {
            UNREFERENCED_PARAMETER(riid);
            UNREFERENCED_PARAMETER(ppvObject);
            return E_NOINTERFACE;
        }
        // add ref count
        auto STDMETHODCALLTYPE AddRef() noexcept->ULONG override final { return ++m_dwCounter; }
        // release this
        auto STDMETHODCALLTYPE Release() noexcept->ULONG override final { auto old = --m_dwCounter; if (!old) { delete this; } return old; };
    public:
        // get resouce count with type
        auto GetResourceCount(ResourceType type) const noexcept->size_t override;
        // get resouce by index, index in range [0, count)
        // for Type_Bitmap, Type_Brush, Type_TextFormat
        auto GetResourcePointer(ResourceType type, size_t index) noexcept ->void* override;
        // get meta by index, index in range [0, count)
        auto GetMeta(size_t index, DeviceIndependentMeta&) noexcept ->void override; 
    public:
        // ctor
        CUIZipResourceLoader(CUIManager& manager) noexcept;
        // dtor
        ~CUIZipResourceLoader() noexcept;
        // init
        auto Init(const wchar_t* file_name) noexcept->HRESULT;
    private:
        // get resouce count from doc
        void get_resource_count_from_xml() noexcept;
        // get bitmap
        auto get_bitmap(pugi::xml_node node) noexcept->ID2D1Bitmap1*;
        // get brush
        auto get_brush(pugi::xml_node node) noexcept->ID2D1Brush*;
        // get text format
        auto get_text_format(pugi::xml_node node) noexcept->IDWriteTextFormat*;
        // find node with index
        static auto find_node_with_index(pugi::xml_node node, const size_t index) noexcept->pugi::xml_node;
        // manager for longui
        CUIManager&             m_manager;
        // zip archive file
        mz_zip_archive          m_zipFile;
        // WIC factory
        IWICImagingFactory2*    m_pWICFactory = nullptr;
        // node for reource
        pugi::xml_node          m_aNodes[RESOURCE_TYPE_COUNT];
        // xml doc for resource
        pugi::xml_document      m_docResource;
        // resource count
        uint32_t                m_aResourceCount[RESOURCE_TYPE_COUNT];
        // ref-counter.
        uint32_t                m_dwCounter = 1;
    };
    /// <summary>
    /// Zip Stream for reading
    /// </summary>
    class CUIZipReaderStream : public Helper::ComBase<Helper::QiList<IStream>> {
    public:
        // ctor
        CUIZipReaderStream(const char* url, mz_zip_archive& zip);
        // dtor
        virtual ~CUIZipReaderStream() {}
    private:

    };
}

