
#include "LongUI.h"

// 构造对象
LongUI::CUIDataObject* LongUI::CUIDataObject::New() noexcept {
    auto* pointer = reinterpret_cast<LongUI::CUIDataObject*>(LongUISmallAlloc(sizeof(LongUI::CUIDataObject)));
    if (pointer) {
        pointer->CUIDataObject::CUIDataObject();
    }
    return pointer;
}



// CUIDataObject 构造函数
LongUI::CUIDataObject::CUIDataObject() noexcept {
    ZeroMemory(&m_dataStorage, sizeof(m_dataStorage));
}


// CUIDataObject 析构函数
LongUI::CUIDataObject::~CUIDataObject() noexcept {
    // 释放数据
    if (m_dataStorage.formatEtc.cfFormat) {
        ::ReleaseStgMedium(&m_dataStorage.stgMedium);
    }
}


// 设置Unicode
HRESULT LongUIMethodCall LongUI::CUIDataObject::SetUnicodeText(HGLOBAL hGlobal) noexcept {
    assert(hGlobal && "hGlobal -> null");
    // 释放数据
    if (m_dataStorage.formatEtc.cfFormat) {
        ::ReleaseStgMedium(&m_dataStorage.stgMedium);
    }
    m_dataStorage.formatEtc = { CF_UNICODETEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    m_dataStorage.stgMedium = { TYMED_HGLOBAL,{ 0 }, 0 };
    m_dataStorage.stgMedium.hGlobal = hGlobal;
    return S_OK;
}

// 设置Unicode字符
HRESULT LongUIMethodCall LongUI::CUIDataObject::SetUnicodeText(const wchar_t* str, size_t length) noexcept {
    HRESULT hr = S_OK;
    if (!length) length = ::wcslen(str);
    // 全局
    HGLOBAL hGlobal = nullptr;
    // 申请成功
    auto size = sizeof(wchar_t) * (length + 1);
    if (hGlobal = ::GlobalAlloc(GMEM_FIXED, size)) {
        LPVOID pdest = ::GlobalLock(hGlobal);
        if (pdest) {
            ::memcpy(pdest, str, size);
        }
        else {
            hr = E_FAIL;
        }
        ::GlobalUnlock(hGlobal);
    }
    else {
        hr = E_OUTOFMEMORY;
    }
    // 设置数据
    if (SUCCEEDED(hr)) {
        hr = this->SetUnicodeText(hGlobal);
    }
    return hr;
}


// IDataObject::GetData 实现: 
HRESULT LongUI::CUIDataObject::GetData(FORMATETC * pFormatetcIn, STGMEDIUM * pMedium) noexcept {
    // 参数检查
    if (!pFormatetcIn || !pMedium) return E_INVALIDARG;
    // 
    pMedium->hGlobal = nullptr;
    // 检查数据
    if (m_dataStorage.formatEtc.cfFormat) {
        // 返回需要获取的格式
        if ((pFormatetcIn->tymed & m_dataStorage.formatEtc.tymed) &&
            (pFormatetcIn->dwAspect == m_dataStorage.formatEtc.dwAspect) &&
            (pFormatetcIn->cfFormat == m_dataStorage.formatEtc.cfFormat))
        {
            return this->CopyMedium(pMedium, &m_dataStorage.stgMedium, &m_dataStorage.formatEtc);
        }
    }
    return DV_E_FORMATETC;
}

// IDataObject::GetDataHere 实现
HRESULT LongUI::CUIDataObject::GetDataHere(FORMATETC * pFormatetcIn, STGMEDIUM * pMedium) noexcept {
    UNREFERENCED_PARAMETER(pFormatetcIn);
    UNREFERENCED_PARAMETER(pMedium);
    return E_NOTIMPL;
}

// IDataObject::QueryGetData 实现: 查询支持格式数据
HRESULT LongUI::CUIDataObject::QueryGetData(FORMATETC * pFormatetc) noexcept {
    // 检查参数
    if (!pFormatetc) return E_INVALIDARG;
    // 
    if (!(DVASPECT_CONTENT & pFormatetc->dwAspect)) {
        return DV_E_DVASPECT;
    }
    HRESULT hr = DV_E_TYMED;
    // 遍历数据
    if (m_dataStorage.formatEtc.cfFormat && m_dataStorage.formatEtc.tymed & pFormatetc->tymed) {
        if (m_dataStorage.formatEtc.cfFormat == pFormatetc->cfFormat) {
            hr = S_OK;
        }
        else {
            hr = DV_E_CLIPFORMAT;
        }
    }
    else {
        hr = DV_E_TYMED;
    }
    return hr;
}

// IDataObject::GetCanonicalFormatEtc 实现
HRESULT LongUI::CUIDataObject::GetCanonicalFormatEtc(FORMATETC * pFormatetcIn, FORMATETC * pFormatetcOut) noexcept {
    return E_NOTIMPL;
}

// IDataObject::SetData 实现
HRESULT LongUI::CUIDataObject::SetData(FORMATETC * pFormatetc, STGMEDIUM * pMedium, BOOL fRelease) noexcept {
    // 检查参数
    if (!pFormatetc || !pMedium) return E_INVALIDARG;
    UNREFERENCED_PARAMETER(fRelease);
    return E_NOTIMPL;
}

// IDataObject::EnumFormatEtc 实现: 枚举支持格式
HRESULT LongUI::CUIDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC ** ppEnumFormatEtc) noexcept {
    // 检查参数
    if (!ppEnumFormatEtc) return E_INVALIDARG;
    *ppEnumFormatEtc = nullptr;
    HRESULT hr = E_NOTIMPL;
    if (DATADIR_GET == dwDirection) {
        // 设置支持格式
        // 暂时仅支持 Unicode字符(UTF-16 on WindowsDO)
        static FORMATETC rgfmtetc[] = {
            { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, 0, TYMED_HGLOBAL },
        };
        hr = ::SHCreateStdEnumFmtEtc(lengthof(rgfmtetc), rgfmtetc, ppEnumFormatEtc);
    }
    return hr;
}

// IDataObject::DAdvise 实现
HRESULT LongUI::CUIDataObject::DAdvise(FORMATETC * pFormatetc, DWORD advf,
    IAdviseSink * pAdvSnk, DWORD * pdwConnection) noexcept {
    UNREFERENCED_PARAMETER(pFormatetc);
    UNREFERENCED_PARAMETER(advf);
    UNREFERENCED_PARAMETER(pAdvSnk);
    UNREFERENCED_PARAMETER(pdwConnection);
    return E_NOTIMPL;
}

// IDataObject::DUnadvise 实现
HRESULT LongUI::CUIDataObject::DUnadvise(DWORD dwConnection) noexcept {
    UNREFERENCED_PARAMETER(dwConnection);
    return E_NOTIMPL;
}

// IDataObject::EnumDAdvise 实现
HRESULT LongUI::CUIDataObject::EnumDAdvise(IEnumSTATDATA ** ppenumAdvise) noexcept {
    UNREFERENCED_PARAMETER(ppenumAdvise);
    return E_NOTIMPL;
}

// 复制媒体数据
HRESULT LongUI::CUIDataObject::CopyMedium(STGMEDIUM * pMedDest, STGMEDIUM * pMedSrc, FORMATETC * pFmtSrc) noexcept {
    // 检查参数
    if (!pMedDest || !pMedSrc || !pFmtSrc) return E_INVALIDARG;
    // 按类型处理
    switch (pMedSrc->tymed)
    {
    case TYMED_HGLOBAL:
        pMedDest->hGlobal = (HGLOBAL)OleDuplicateData(pMedSrc->hGlobal, pFmtSrc->cfFormat, 0);
        break;
    case TYMED_GDI:
        pMedDest->hBitmap = (HBITMAP)OleDuplicateData(pMedSrc->hBitmap, pFmtSrc->cfFormat, 0);
        break;
    case TYMED_MFPICT:
        pMedDest->hMetaFilePict = (HMETAFILEPICT)OleDuplicateData(pMedSrc->hMetaFilePict, pFmtSrc->cfFormat, 0);
        break;
    case TYMED_ENHMF:
        pMedDest->hEnhMetaFile = (HENHMETAFILE)OleDuplicateData(pMedSrc->hEnhMetaFile, pFmtSrc->cfFormat, 0);
        break;
    case TYMED_FILE:
        pMedSrc->lpszFileName = (LPOLESTR)OleDuplicateData(pMedSrc->lpszFileName, pFmtSrc->cfFormat, 0);
        break;
    case TYMED_ISTREAM:
        pMedDest->pstm = pMedSrc->pstm;
        pMedSrc->pstm->AddRef();
        break;
    case TYMED_ISTORAGE:
        pMedDest->pstg = pMedSrc->pstg;
        pMedSrc->pstg->AddRef();
        break;
    case TYMED_NULL:
        __fallthrough;
    default:
        break;
    }
    //
    pMedDest->tymed = pMedSrc->tymed;
    pMedDest->pUnkForRelease = nullptr;
    if (pMedSrc->pUnkForRelease) {
        pMedDest->pUnkForRelease = pMedSrc->pUnkForRelease;
        pMedSrc->pUnkForRelease->AddRef();
    }
    return S_OK;
}

// 设置Blob
HRESULT LongUI::CUIDataObject::SetBlob(CLIPFORMAT cf, const void * pvBlob, UINT cbBlob) noexcept {
    void *pv = GlobalAlloc(GPTR, cbBlob);
    HRESULT hr = pv ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr)) {
        CopyMemory(pv, pvBlob, cbBlob);
        FORMATETC fmte = { cf, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        // The STGMEDIUM structure is used to define how to handle a global memory transfer.  
        // This structure includes a flag, tymed, which indicates the medium  
        // to be used, and a union comprising pointers and a handle for getting whichever  
        // medium is specified in tymed.  
        STGMEDIUM medium = {};
        medium.tymed = TYMED_HGLOBAL;
        medium.hGlobal = pv;
        hr = this->SetData(&fmte, &medium, TRUE);
        if (FAILED(hr)) {
            ::GlobalFree(pv);
        }
    }
    return hr;
}



// ----------------------------------------------------------------------------------


// 构造对象
LongUI::CUIDropSource* LongUI::CUIDropSource::New() noexcept {
    auto* pointer = reinterpret_cast<LongUI::CUIDropSource*>(LongUISmallAlloc(sizeof(LongUI::CUIDropSource)));
    if (pointer) {
        pointer->CUIDropSource::CUIDropSource();
    }
    return pointer;
}

// CUIDropSource::QueryContinueDrag 实现: 
HRESULT LongUI::CUIDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) noexcept {
    // Esc按下或者鼠标右键按下 : 取消拖拽
    if (fEscapePressed || (grfKeyState & MK_RBUTTON)) {
        return DRAGDROP_S_CANCEL;
    }
    // 鼠标左键弹起: 拖拽结束
    if (!(grfKeyState & MK_LBUTTON)) {
        return DRAGDROP_S_DROP;
    }
    return S_OK;
}

// CUIDropSource::GiveFeedback 实现
HRESULT LongUI::CUIDropSource::GiveFeedback(DWORD dwEffect) noexcept {
    UNREFERENCED_PARAMETER(dwEffect);
    return DRAGDROP_S_USEDEFAULTCURSORS;
}