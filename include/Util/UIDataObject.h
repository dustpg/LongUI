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

// longui namespace
namespace LongUI {
    // 数据储存
    struct DATASTORAGE {
        FORMATETC formatEtc;
        STGMEDIUM stgMedium;
    };
    // UIDataObject 类: 实现IDataObject
    class CUIDataObject final : public Helper::ComBase<
        Helper::QiListSelf<IUnknown, Helper::QiList<IDataObject>>> {
    public:
        // 创建对象
        static CUIDataObject* New() noexcept;
    private:
        // delete new operator
        void* operator new(size_t ) = delete;
        // delete new operator
        void* operator new[](size_t ) = delete;
        // delete operator
        void  operator delete(void* p) noexcept { LongUI::SmallFree(p); };
        // delete new operator
        void  operator delete[](void*) = delete;
        // 构造函数
        CUIDataObject() noexcept;
        // 删除复制构造函数
        CUIDataObject(const CUIDataObject&) = delete;
        // 删除=
        CUIDataObject& operator =(const CUIDataObject&) = delete;
        // 析构函数
        virtual ~CUIDataObject() noexcept;
    public:
        // 设置Unicode
        HRESULT SetUnicodeText(const wchar_t*, size_t =0) noexcept;
        // 设置Unicode
        HRESULT SetUnicodeText(HGLOBAL) noexcept;
    public: // IDataObject 接口 实现
        // IDataObject::GetData 实现
        HRESULT STDMETHODCALLTYPE GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium) noexcept override;
        // IDataObject::GetDataHere 实现
        HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium) noexcept override;
        // IDataObject::QueryGetData 实现
        HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC *pformatetc) noexcept override;
        // IDataObject::GetCanonicalFormatEtc 实现
        HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC *pformatetcIn, FORMATETC *pformatetcOut) noexcept override;
        // IDataObject::SetData 实现
        HRESULT STDMETHODCALLTYPE SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease) noexcept override;
        // IDataObject::EnumFormatEtc 实现
        HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc) noexcept override;
        // IDataObject::DAdvise 实现
        HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSnk, DWORD *pdwConnection) noexcept override;
        // IDataObject::DUnadvise 实现
        HRESULT STDMETHODCALLTYPE DUnadvise(DWORD dwConnection) noexcept override;
        // IDataObject::EnumDAdvise 实现
        HRESULT STDMETHODCALLTYPE  EnumDAdvise(IEnumSTATDATA **ppenumAdvise) noexcept override;
    private:
        // 复制媒体数据
        HRESULT CopyMedium(STGMEDIUM* pMedDest, STGMEDIUM* pMedSrc, FORMATETC* pFmtSrc) noexcept;
        // 设置Blob
        HRESULT SetBlob(CLIPFORMAT cf, const void*pvBlob, UINT cbBlob) noexcept;
    private:
        // 当前数据
        DATASTORAGE             m_dataStorage;
    };
}