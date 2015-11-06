#include "LongUI.h"
#include <algorithm>


// 忙等
LongUINoinline void LongUI::usleep(long usec) noexcept {
    LARGE_INTEGER lFrequency;
    LARGE_INTEGER lEndTime;
    LARGE_INTEGER lCurTime;
    ::QueryPerformanceFrequency(&lFrequency);
    ::QueryPerformanceCounter(&lEndTime);
    lEndTime.QuadPart += (LONGLONG)usec * lFrequency.QuadPart / 1000000;
    do { ::QueryPerformanceCounter(&lCurTime); } while (lCurTime.QuadPart < lEndTime.QuadPart);
}


/// <summary>
/// float4 color ---> 32-bit ARGB uint color
/// 将浮点颜色转换成32位ARGB排列整型
/// </summary>
/// <param name="color">The d2d color</param>
/// <returns>32-bit ARGB 颜色</returns>
LongUINoinline auto __fastcall LongUI::PackTheColorARGB(D2D1_COLOR_F& IN color) noexcept -> uint32_t {
    // 常量
    constexpr uint32_t ALPHA_SHIFT  = CHAR_BIT * 3;
    constexpr uint32_t RED_SHIFT    = CHAR_BIT * 2;
    constexpr uint32_t GREEN_SHIFT  = CHAR_BIT * 1;
    constexpr uint32_t BLUE_SHIFT   = CHAR_BIT * 0;
    // 写入
    uint32_t colorargb =
        ((uint32_t(color.a * 255.f) & 0xFF) << ALPHA_SHIFT) |
        ((uint32_t(color.r * 255.f) & 0xFF) << RED_SHIFT)   |
        ((uint32_t(color.g * 255.f) & 0xFF) << GREEN_SHIFT) |
        ((uint32_t(color.b * 255.f) & 0xFF) << BLUE_SHIFT);
    return colorargb;
}

/// <summary>
/// 32-bit ARGB uint color ---> float4 color
/// 将32位ARGB排列整型转换成浮点颜色
/// </summary>
/// <param name="color32">The 32-bit color.</param>
/// <param name="color4f">The float4 color.</param>
/// <returns>void</returns>
LongUINoinline auto __fastcall LongUI::UnpackTheColorARGB(uint32_t IN color32, D2D1_COLOR_F& OUT color4f) noexcept ->void {
    // 位移量
    constexpr uint32_t ALPHA_SHIFT  = CHAR_BIT * 3;
    constexpr uint32_t RED_SHIFT    = CHAR_BIT * 2;
    constexpr uint32_t GREEN_SHIFT  = CHAR_BIT * 1;
    constexpr uint32_t BLUE_SHIFT   = CHAR_BIT * 0;
    // 掩码
    constexpr uint32_t ALPHA_MASK   = 0xFFU << ALPHA_SHIFT;
    constexpr uint32_t RED_MASK     = 0xFFU << RED_SHIFT;
    constexpr uint32_t GREEN_MASK   = 0xFFU << GREEN_SHIFT;
    constexpr uint32_t BLUE_MASK    = 0xFFU << BLUE_SHIFT;
    // 计算
    color4f.r = static_cast<float>((color32 & RED_MASK)   >> RED_SHIFT)   / 255.f;
    color4f.g = static_cast<float>((color32 & GREEN_MASK) >> GREEN_SHIFT) / 255.f;
    color4f.b = static_cast<float>((color32 & BLUE_MASK)  >> BLUE_SHIFT)  / 255.f;
    color4f.a = static_cast<float>((color32 & ALPHA_MASK) >> ALPHA_SHIFT) / 255.f;
}



// Meta 渲染
/// <summary>
/// render the meta 渲染Meta
/// </summary>
/// <param name="meta">The meta.</param>
/// <param name="target">The render target.</param>
/// <param name="des_rect">The des_rect.</param>
/// <param name="opacity">The opacity.</param>
/// <returns></returns>
void __fastcall LongUI::Meta_Render(
    const Meta& meta, ID2D1DeviceContext* target,
    const D2D1_RECT_F& des_rect, float opacity) noexcept {
    // 无效位图
    if (!meta.bitmap) {
        UIManager << DL_Warning << "bitmap->null" << LongUI::endl;
        return;
    }
    switch (meta.rule)
    {
    case LongUI::BitmapRenderRule::Rule_Scale:
        __fallthrough;
    default:
    case LongUI::BitmapRenderRule::Rule_ButtonLike:
        // 直接缩放:
        target->DrawBitmap(
            meta.bitmap,
            des_rect, opacity,
            static_cast<D2D1_INTERPOLATION_MODE>(meta.interpolation),
            meta.src_rect,
            nullptr
            );
        break;
        /*case LongUI::BitmapRenderRule::Rule_ButtonLike:
            // 类按钮
            // - 原矩形, 宽 > 高 ->
            // - 原矩形, 宽 < 高 ->
        {
            constexpr float MARKER = 0.25f;
            auto width = meta.src_rect.right - meta.src_rect.left;
            auto bilibili = width * MARKER / (meta.src_rect.bottom - meta.src_rect.top) *
                (des_rect.bottom - des_rect.top);
            D2D1_RECT_F des_rects[3]; D2D1_RECT_F src_rects[3]; D2D1_RECT_F clip_rects[3];
            // ---------------------------------------
            des_rects[0] = {
                des_rect.left, des_rect.top,
                des_rect.left + bilibili, des_rect.bottom
            };
            des_rects[1] = {
                des_rects[0].right, des_rect.top,
                des_rect.right - bilibili, des_rect.bottom
            };
            des_rects[2] = {
                des_rects[1].right, des_rect.top,
                des_rect.right, des_rect.bottom
            };
            // ---------------------------------------
            std::memcpy(clip_rects, des_rects, sizeof(des_rects));
            if (clip_rects[1].left > des_rects[1].right) {
                std::swap(clip_rects[1].right, des_rects[1].left);
                std::swap(des_rects[1].right, des_rects[1].left);
                clip_rects[0].right = des_rects[1].left;
                clip_rects[2].left = des_rects[1].right;
            }
            // ---------------------------------------
            src_rects[0] = {
                meta.src_rect.left, meta.src_rect.top,
                meta.src_rect.left + width * MARKER, meta.src_rect.bottom
            };
            src_rects[1] = {
                src_rects[0].right, meta.src_rect.top,
                meta.src_rect.right - width * MARKER, meta.src_rect.bottom
            };
            src_rects[2] = {
                src_rects[1].right, meta.src_rect.top,
                meta.src_rect.right, meta.src_rect.bottom
            };
            // 正式渲染
            for (auto i = 0u; i < lengthof(src_rects); ++i) {
                target->PushAxisAlignedClip(clip_rects + i, D2D1_ANTIALIAS_MODE_ALIASED);
                target->DrawBitmap(
                    meta.bitmap,
                    des_rects[i], opacity,
                    static_cast<D2D1_INTERPOLATION_MODE>(meta.interpolation),
                    src_rects[i],
                    nullptr
                    );
                target->PopAxisAlignedClip();
            }
        }
        break;*/
    }
}


// 构造对象
LongUI::CUIDataObject* LongUI::CUIDataObject::New() noexcept {
    auto* pointer = reinterpret_cast<LongUI::CUIDataObject*>(LongUI::SmallAlloc(sizeof(LongUI::CUIDataObject)));
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
HRESULT LongUI::CUIDataObject::SetUnicodeText(HGLOBAL hGlobal) noexcept {
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
HRESULT LongUI::CUIDataObject::SetUnicodeText(const wchar_t* str, size_t length) noexcept {
    HRESULT hr = S_OK;
    if (!length) length = std::wcslen(str);
    // 全局
    HGLOBAL hGlobal = nullptr;
    // 申请成功
    auto size = sizeof(wchar_t) * (length + 1);
    if (hGlobal = ::GlobalAlloc(GMEM_FIXED, size)) {
        LPVOID pdest = ::GlobalLock(hGlobal);
        if (pdest) {
            std::memcpy(pdest, str, size);
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
    UNREFERENCED_PARAMETER(pFormatetcIn);
    UNREFERENCED_PARAMETER(pFormatetcOut);
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
        hr = ::SHCreateStdEnumFmtEtc(static_cast<UINT>(lengthof(rgfmtetc)), rgfmtetc, ppEnumFormatEtc);
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
    auto* pointer = reinterpret_cast<LongUI::CUIDropSource*>(LongUI::SmallAlloc(sizeof(LongUI::CUIDropSource)));
    if (pointer) {
        pointer->CUIDropSource::CUIDropSource();
    }
    return pointer;
}

// 析构函数
LongUI::CUIDropSource::~CUIDropSource() noexcept {
};

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


// UIString 设置字符串
void LongUI::CUIString::Set(const wchar_t* str, uint32_t length) noexcept {
    assert(str && "bad argument");
    // 内存不足
    if (!this->m_pString) {
        this->m_pString = m_aDataStatic;
        m_cBufferLength = LongUIStringFixedLength;
        m_aDataStatic[0] = wchar_t(0);
    }
    // 超长的话
    if (length > m_cBufferLength) {
        m_cBufferLength = static_cast<uint32_t>(this->nice_buffer_length(length));
        // 尝试释放
        this->safe_free_bufer();
        // 申请内存
        m_pString = this->alloc_bufer(m_cBufferLength);
    }
    // 复制数据
    assert(str && "<bad");
    this->copy_string(m_pString, str, length);
    m_cLength = length;
}

// UIString 设置字符串
void LongUI::CUIString::Set(const char* str, uint32_t len) noexcept {
    assert(str && "bad argument");
    // 固定缓存
    wchar_t buffer[LongUIStringBufferLength];
    // 动态缓存
    wchar_t* huge_buffer = nullptr;
    uint32_t buffer_length = LongUIStringBufferLength;
    // 内存不足
    if (!this->m_pString) {
        this->m_pString = m_aDataStatic;
        m_cBufferLength = LongUIStringFixedLength;
        m_aDataStatic[0] = wchar_t(0);
    }
    // 假设全是英文字母, 超长的话
    if (len > LongUIStringBufferLength) {
        buffer_length = static_cast<uint32_t>(this->nice_buffer_length(len));
        huge_buffer = this->alloc_bufer(m_cBufferLength);
        // OOM ?
        if (!huge_buffer) return this->OnOOM();
    }
    {
        auto real_buffer = huge_buffer ? huge_buffer : buffer;
        auto length_got = LongUI::UTF8toWideChar(str, real_buffer);
        real_buffer[length_got] = 0;
        // 动态申请?
        if (huge_buffer) {
            this->safe_free_bufer();
            m_pString = huge_buffer;
            huge_buffer = nullptr;
            m_cLength = length_got;
            m_cBufferLength = buffer_length;
        }
        // 设置
        else {
            this->Set(real_buffer, length_got);
        }
    }
    // sad
    assert(m_pString);
}

// UIString 添加字符串
void LongUI::CUIString::Append(const wchar_t* str, uint32_t len) noexcept {
    assert(str && "bad argument");
    // 无需
    if (!(*str)) return;
    // 超过缓存?
    const auto target_lenth = m_cLength + len + 1;
    if (target_lenth > m_cBufferLength) {
        m_cBufferLength = static_cast<uint32_t>(this->nice_buffer_length(target_lenth));
        // 申请内存
        auto alloced_buffer = this->alloc_bufer(m_cBufferLength);
        if (!alloced_buffer) {
            return this->OnOOM();
        }
        // 复制数据
        this->copy_string_ex(alloced_buffer, m_pString, m_cLength);
        this->copy_string(alloced_buffer + m_cLength, str, len);
        // 释放
        this->safe_free_bufer();
        m_pString = alloced_buffer;
    }
    // 继续使用旧缓存
    else {
        // 复制数据
        this->copy_string(m_pString + m_cLength, str, len);
    }
    // 添加长度
    m_cLength += len;
}


// 设置保留缓存
void LongUI::CUIString::Reserve(uint32_t len) noexcept {
    assert(len && "bad argument");
    // 小于等于就什么都不做
    if (len > m_cBufferLength) {
        // 换成偶数
        auto nice_length = len + (len & 1);
        // 申请空间
        auto buffer = this->alloc_bufer(nice_length);
        // OOM
        if (!buffer) {
            return this->OnOOM();
        }
        // 复制数据
        this->copy_string(buffer, m_pString, m_cLength);
        m_cBufferLength = nice_length;
        this->safe_free_bufer();
        m_pString = buffer;
    }
}

// 插入字符串
void LongUI::CUIString::Insert(uint32_t off, const wchar_t* str, uint32_t len) noexcept {
    assert(str && "bad argument");
    assert(off <= m_cLength && "out of range");
    // 插入尾巴
    if (off >= m_cLength) return this->Append(str, len);
    // 无需
    if (!(*str)) return;
    // 需要申请内存?
    const auto target_lenth = m_cLength + len + 1;
    if (target_lenth > m_cBufferLength) {
        m_cBufferLength = static_cast<uint32_t>(this->nice_buffer_length(target_lenth));
        // 申请内存
        auto alloced_buffer = this->alloc_bufer(m_cBufferLength);
        if (!alloced_buffer) {
            return this->OnOOM();
        }
        // 复制数据
        this->copy_string_ex(alloced_buffer, m_pString, off);
        this->copy_string_ex(alloced_buffer + off, str, len);
        this->copy_string(alloced_buffer + off + len, m_pString, m_cLength - off);
        // 释放
        this->safe_free_bufer();
        m_pString = alloced_buffer;
    }
    // 继续使用旧缓存
    else {
        // memcpy:__restrict 要求, 手动循环
        auto src_end = m_pString + m_cLength;
        auto des_end = src_end + len;
        for (uint32_t i = 0; i < (m_cLength - off + 1); ++i) {
            *des_end = *src_end;
            --des_end; --src_end;
        }
        // 复制数据
        this->copy_string_ex(m_pString + off, str, len);
    }
    // 添加长度
    m_cLength += len;
}


// UIString 字符串析构函数
LongUI::CUIString::~CUIString() noexcept {
    // 释放数据
    this->safe_free_bufer();
    m_cLength = 0;
}

// 复制构造函数
LongUI::CUIString::CUIString(const LongUI::CUIString& obj) noexcept {
    assert(obj.m_pString && "bad");
    if (!obj.m_pString) {
        this->OnOOM();
        return;
    }
    // 构造
    if (obj.m_pString != obj.m_aDataStatic) {
        UIManager << DL_Warning << "copy ctor is not suit for CUIString" << LongUI::endl;
        m_pString = this->alloc_bufer(obj.m_cBufferLength);
        m_cBufferLength = obj.m_cBufferLength;
    }
    // 复制数据
    assert(m_pString && "out of memory");
    if (m_pString) {
        this->copy_string(m_pString, obj.m_pString, obj.m_cLength);
        m_cLength = obj.m_cLength;
    }
    // 内存不足
    else {
        this->OnOOM();
    }
}

// move构造函数
LongUI::CUIString::CUIString(LongUI::CUIString&& obj) noexcept {
    // 构造
    if (obj.m_pString != obj.m_aDataStatic) {
        m_pString = obj.m_pString;
    }
    else {
        // 警告
        UIManager << DL_Warning << "move ctor is not suit for CUIString" << LongUI::endl;
        // 复制数据
        this->copy_string(m_aDataStatic, obj.m_aDataStatic, obj.m_cLength);
    }
    m_cLength = obj.m_cLength;
    obj.m_cLength = 0;
    obj.m_pString = obj.m_aDataStatic;
    obj.m_aDataStatic[0] = wchar_t(0);
}

// 删除字符串
void LongUI::CUIString::Remove(uint32_t offset, uint32_t length) noexcept {
    assert(offset + length <= m_cLength && "out of range");
    // 有可能直接删除后面, 优化
    if (offset + length >= m_cLength) {
        m_cLength = std::min(m_cLength, offset);
        return;
    }
    // 将后面的字符串复制过来即可
    // memcpy:__restrict 要求, 手动循环
    auto des = m_pString + offset;
    auto src = des + length;
    for (uint32_t i = 0; i < (m_cLength - offset - length + 1); ++i) {
        *des = *src;
        ++des; ++src;
    }
    m_cLength -= length;
}

// 格式化
void LongUI::CUIString::Format(const wchar_t* format, ...) noexcept {
    // 初始化数据
    wchar_t buffer[LongUIStringBufferLength]; buffer[0] = 0;
    va_list ap; va_start(ap, format);
    // 格式化字符串
    auto length = std::vswprintf(buffer, LongUIStringBufferLength, format, ap);
    // 发生错误
    if (length < 0) {
        UIManager << DL_Warning 
            << L"std::vswprintf return " << long(length) 
            << L" for out of space or some another error" 
            << LongUI::endl;
        length = LongUIStringBufferLength - 1;
    }
    // 设置
    this->Set(buffer, length);
    // 收尾
    va_end(ap);
}

// CUIString 内存不足
void LongUI::CUIString::OnOOM() noexcept {
    constexpr auto length = 13ui32;
    // 内存
    if (LongUIStringFixedLength > length) {
        this->Set(L"Out of Memory", length);
    }
    else if (LongUIStringFixedLength > 3) {
        this->Set(L"OOM", 3);
    }
    // 显示错误
    UIManager.ShowError(E_OUTOFMEMORY, L"<LongUI::CUIString::OnOOM()>");
}


// += 操作
//const LongUI::CUIString& LongUI::CUIString::operator+=(const wchar_t*);

// CUIAnimation ---------- BEGIN -------------

#define UIAnimation_Template_A      \
    auto v = LongUI::EasingFunction(this->type, this->time / this->duration)
#define UIAnimation_Template_B(m)   \
    this->value.m = v * (this->start.m - this->end.m) + this->end.m;

// for D2D1_POINT_2F or Float1
template<> void LongUI::CUIAnimation<float>::Update(float t) noexcept {
    if (this->time <= 0.f) {
        this->value = this->end;
        return;
    }
    // 计算
    this->value = LongUI::EasingFunction(this->type, this->time / this->duration)
        * (this->start - this->end) + this->end;
    // 减少时间
    this->time -= t;
}


// for D2D1_POINT_2F or Float2
template<> void LongUI::CUIAnimation<D2D1_POINT_2F>::Update(float t) noexcept {
    if (this->time <= 0.f) {
        this->value = this->end;
        return;
    }
    UIAnimation_Template_A;
    UIAnimation_Template_B(x);
    UIAnimation_Template_B(y);
    // 减少时间
    this->time -= t;
}


// for D2D1_COLOR_F or Float4
template<> void LongUI::CUIAnimation<D2D1_COLOR_F>::Update(float t) noexcept {
    if (this->time <= 0.f) {
        this->value = this->end;
        return;
    }
    UIAnimation_Template_A;
    UIAnimation_Template_B(r);
    UIAnimation_Template_B(g);
    UIAnimation_Template_B(b);
    UIAnimation_Template_B(a);
    // 减少时间
    this->time -= t;
}

// for D2D1_MATRIX_3X2_F or Float6
template<> void LongUI::CUIAnimation<D2D1_MATRIX_3X2_F>::Update(float t) noexcept {
    if (this->time <= 0.f) {
        this->value = this->end;
        return;
    }
    UIAnimation_Template_A;
    UIAnimation_Template_B(_11);
    UIAnimation_Template_B(_12);
    UIAnimation_Template_B(_21);
    UIAnimation_Template_B(_22);
    UIAnimation_Template_B(_31);
    UIAnimation_Template_B(_32);
    // 减少时间
    this->time -= t;
}
#undef UIAnimation_Template_A
#undef UIAnimation_Template_B
// CUIAnimation ----------  END  -------------


// get transformed pointer
LongUINoinline auto LongUI::TransformPointInverse(const D2D1_MATRIX_3X2_F& matrix, const D2D1_POINT_2F& point) noexcept ->D2D1_POINT_2F {
    D2D1_POINT_2F result;
    // x = (bn-dm) / (bc-ad)
    // y = (an-cm) / (ad-bc)
    // a : m_matrix._11
    // b : m_matrix._21
    // c : m_matrix._12
    // d : m_matrix._22
    auto bc_ad = matrix._21 * matrix._12 - matrix._11 * matrix._22;
    auto m = point.x - matrix._31;
    auto n = point.y - matrix._32;
    result.x = (matrix._21*n - matrix._22 * m) / bc_ad;
    result.y = (matrix._12*m - matrix._11 * n) / bc_ad;
    return result;
}

// longui::impl 命名空间
namespace LongUI { namespace impl {
    // 字符串转数字
    template<typename T> LongUINoinline auto atoi(const T* str) noexcept ->int {
        assert(str && "bad argument");
        // 初始化
        bool negative = false; int value = 0; T ch = 0;
        // 遍历
        while (ch = *str) {
            // 空白?
            if (!white_space(ch)) {
                if (ch == '-') {
                    negative = true;
                }
                else if (valid_digit(ch)) {
                    value *= 10;
                    value += ch - static_cast<T>('0');
                }
                else {
                    break;
                }
            }
            ++str;
        }
        // 负数
        if (negative) {
            value = -value;
        }
        return value;
    }
    // 字符串转浮点
    template<typename T> LongUINoinline auto atof(const T* p) noexcept ->float {
        assert(p && "bad argument");
        bool negative = false;
        float value, scale;
        // 跳过空白
        while (white_space(*p)) ++p;
        // 检查符号
        if (*p == '-') {
            negative = true;
            ++p;
        }
        else if (*p == '+') {
            ++p;
        }
        // 获取小数点或者指数之前的数字(有的话)
        for (value = 0.0f; valid_digit(*p); ++p) {
            value = value * 10.0f + static_cast<float>(*p - static_cast<T>('0'));
        }
        // 获取小数点或者指数之后的数字(有的话)
        if (*p == '.') {
            float pow10 = 10.0f; ++p;
            while (valid_digit(*p)) {
                value += (*p - static_cast<T>('0')) / pow10;
                pow10 *= 10.0f;
                ++p;
            }
        }
        // 处理指数(有的话)
        bool frac = false;
        scale = 1.0f;
        if ((*p == 'e') || (*p == 'E')) {
            // 获取指数的符号(有的话)
            ++p;
            if (*p == '-') {
                frac = true;
                ++p;
            }
            else if (*p == '+') {
                ++p;
            }
            unsigned int expon;
            // 获取指数的数字(有的话)
            for (expon = 0; valid_digit(*p); ++p) {
                expon = expon * 10 + (*p - static_cast<T>('0'));
            }
            // float 最大38 double 最大308
            if (expon > 38) expon = 38;
            // 计算比例因数
            while (expon >= 8) { scale *= 1E8f;  expon -= 8; }
            while (expon) { scale *= 10.0f; --expon; }
        }
        // 返回
        float returncoude = (frac ? (value / scale) : (value * scale));
        if (negative) {
            // float
            returncoude = -returncoude;
        }
        return returncoude;
    }
}}

/// <summary>
/// string to float.字符串转浮点, std::atof自己实现版
/// </summary>
/// <param name="p">The string. in const char*</param>
/// <returns></returns>
auto LongUI::AtoF(const char* __restrict p) noexcept -> float {
    if (!p) return 0.0f;
    return impl::atof(p);
}


/// <summary>
/// string to float.字符串转浮点, std::atof自己实现版
/// </summary>
/// <param name="p">The string.in const wchar_t*</param>
/// <returns></returns>
auto LongUI::AtoF(const wchar_t* __restrict p) noexcept -> float {
    if (!p) return 0.0f;
    return impl::atof(p);
}

/// <summary>
/// string to int, 字符串转整型, std::atoi自己实现版
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
auto LongUI::AtoI(const char* __restrict str) noexcept -> int {
    if (!str) return 0;
    return impl::atoi(str);
}

/// <summary>
/// string to int, 字符串转整型, std::atoi自己实现版
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
auto LongUI::AtoI(const wchar_t* __restrict str) noexcept -> int {
    if (!str) return 0;
    return impl::atoi(str);
}


// 源: http://llvm.org/svn/llvm-project/llvm/trunk/lib/Support/ConvertUTF.c
// 有修改

static constexpr int halfShift = 10;

static constexpr char32_t halfBase = 0x0010000UL;
static constexpr char32_t halfMask = 0x3FFUL;

#define UNI_SUR_HIGH_START      (char32_t)0xD800
#define UNI_SUR_HIGH_END        (char32_t)0xDBFF
#define UNI_SUR_LOW_START       (char32_t)0xDC00
#define UNI_SUR_LOW_END         (char32_t)0xDFFF

#define UNI_REPLACEMENT_CHAR    (char32_t)0x0000FFFD
#define UNI_MAX_BMP             (char32_t)0x0000FFFF
#define UNI_MAX_UTF16           (char32_t)0x0010FFFF
#define UNI_MAX_UTF32           (char32_t)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32     (char32_t)0x0010FFFF

#define UNI_MAX_UTF8_BYTES_PER_CODE_POINT 4

#define UNI_UTF16_BYTE_ORDER_MARK_NATIVE  0xFEFF
#define UNI_UTF16_BYTE_ORDER_MARK_SWAPPED 0xFFFE

// 转换表
static constexpr char trailingBytesForUTF8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/*
* Magic values subtracted from a buffer value during UTF8 conversion.
* This table contains as many values as there might be trailing bytes
* in a UTF-8 sequence.
*/
static constexpr char32_t offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
* Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
* into the first byte, depending on how many bytes follow.  There are
* as many entries in this table as there are UTF-8 sequence types.
* (I.event., one byte sequence, two byte... etc.). Remember that sequencs
* for *legal* UTF-8 will be 4 or fewer bytes total.
*/
static constexpr char firstByteMark[7] = { 0x00i8, 0x00i8, 0xC0i8, 0xE0i8, 0xF0i8, 0xF8i8, 0xFCi8 };


/// <summary>
/// Base64 : encode, 编码
/// </summary>
/// <param name="bindata">The source binary data.</param>
/// <param name="binlen">The length of source binary data in byte</param>
/// <param name="base64">The out data</param>
/// <returns></returns>
auto __fastcall LongUI::Base64Encode(IN const uint8_t* __restrict bindata, IN size_t binlen, OUT char* __restrict const base64) noexcept -> char * {
    uint8_t current;
    auto base64_index = base64;
    // 
    for (size_t i = 0; i < binlen; i += 3) {
        current = (bindata[i] >> 2);
        current &= static_cast<uint8_t>(0x3F);
        *base64_index = Base64Chars[current]; ++base64_index;

        current = (static_cast<uint8_t>((bindata)[i] << 4)) & (static_cast<uint8_t>(0x30));
        if (i + 1 >= binlen) {
            *base64_index = Base64Chars[current]; ++base64_index;
            *base64_index = '='; ++base64_index;
            *base64_index = '='; ++base64_index;
            break;
        }
        current |= (static_cast<uint8_t>((bindata)[i + 1] >> 4)) & (static_cast<uint8_t>(0x0F));
        *base64_index = Base64Chars[current]; ++base64_index;

        current = (static_cast<uint8_t>((bindata)[i + 1] << 2)) & (static_cast<uint8_t>(0x3C));
        if (i + 2 >= binlen) {
            *base64_index = Base64Chars[current]; ++base64_index;
            *base64_index = '='; ++base64_index;
            break;
        }
        current |= (static_cast<uint8_t>((bindata)[i + 2] >> 6)) & (static_cast<uint8_t>(0x03));
        *base64_index = Base64Chars[current]; ++base64_index;

        current = (static_cast<uint8_t>(bindata[i + 2])) & (static_cast<uint8_t>(0x3F));
        *base64_index = Base64Chars[current]; ++base64_index;
    }
    *base64_index = 0;
    return base64;
}

// 解码
auto __fastcall LongUI::Base64Decode(IN const char * __restrict base64, OUT uint8_t * __restrict bindata) noexcept -> size_t {
    // 二进制长度
    union { uint8_t temp[4]; uint32_t temp_u32; };
    uint8_t* bindata_index = bindata;
    // 主循环
    while (*base64) {
        temp_u32 = uint32_t(-1);
        // 基本转换
        temp[0] = Base64Datas[base64[0]];  temp[1] = Base64Datas[base64[1]];
        temp[2] = Base64Datas[base64[2]];  temp[3] = Base64Datas[base64[3]];
        // 第一个二进制数据
        *bindata_index = ((temp[0] << 2) & uint8_t(0xFC)) | ((temp[1] >> 4) & uint8_t(0x03));
        ++bindata_index;
        if (base64[2] == '=') break;
        // 第三个二进制数据
        *bindata_index = ((temp[1] << 4) & uint8_t(0xF0)) | ((temp[2] >> 2) & uint8_t(0x0F));
        ++bindata_index;
        if (base64[3] == '=') break;
        // 第三个二进制数据
        *bindata_index = ((temp[2] << 6) & uint8_t(0xF0)) | ((temp[2] >> 0) & uint8_t(0x3F));
        ++bindata_index;
        base64 += 4;
    }
    return bindata_index - bindata;
}

// UTF-16 to UTF-8
// Return: UTF-8 string length, 0 maybe error
auto __fastcall LongUI::UTF16toUTF8(const char16_t* __restrict pUTF16String, char* __restrict pUTF8String) noexcept ->uint32_t {
    UINT32 length = 0;
    const char16_t* source = pUTF16String;
    char* target = pUTF8String;
    //char* targetEnd = pUTF8String + uBufferLength;
    // 转换
    while (*source) {
        char32_t ch;
        unsigned short bytesToWrite = 0;
        const char32_t byteMask = 0xBF;
        const char32_t byteMark = 0x80;
        // const char16_t* oldSource = source; /* In case we have to back up because of target overflow. */
        ch = *source++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            if (*source) {
                char32_t ch2 = *source;
                /* If it's a low surrogate, convert to UTF32. */
                if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
                    ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
                        + (ch2 - UNI_SUR_LOW_START) + halfBase;
                    ++source;
                }
            }
            else {
                --source;
                length = 0;
                assert(!"end of string");
                break;
            }
#ifdef STRICT_CONVERSION
        else { /* it's an unpaired high surrogate */
            --source; /* return to the illegal value itself */
            result = sourceIllegal;
            break;
        }
#endif
        }
#ifdef STRICT_CONVERSION
        else {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
                --source; /* return to the illegal value itself */
                result = sourceIllegal;
                break;
            }
        }
#endif
        /* Figure out how many bytes the result will require */
        if (ch < (char32_t)0x80) {
            bytesToWrite = 1;
        }
        else if (ch < (char32_t)0x800) {
            bytesToWrite = 2;
        }
        else if (ch < (char32_t)0x10000) {
            bytesToWrite = 3;
        }
        else if (ch < (char32_t)0x110000) {
            bytesToWrite = 4;
        }
        else {
            bytesToWrite = 3;
            ch = UNI_REPLACEMENT_CHAR;
        }

        target += bytesToWrite;
        /*if (target > targetEnd) {
            source = oldSource; // Back up source pointer!
            target -= bytesToWrite;
            length = 0; break;
        }*/
        switch (bytesToWrite) { /* note: everything falls through. */
        case 4: *--target = (char)((ch | byteMark) & byteMask); ch >>= 6;
        case 3: *--target = (char)((ch | byteMark) & byteMask); ch >>= 6;
        case 2: *--target = (char)((ch | byteMark) & byteMask); ch >>= 6;
        case 1: *--target = (char)(ch | firstByteMark[bytesToWrite]);
        }
        target += bytesToWrite;
        length += bytesToWrite;
    }
    return length;
}



// UTF-8 to UTF-16
// Return: UTF-16 string length, 0 maybe error
auto __fastcall LongUI::UTF8toUTF16(const char* __restrict pUTF8String, char16_t* __restrict pUTF16String) noexcept -> uint32_t {
    UINT32 length = 0;
    auto source = reinterpret_cast<const unsigned char*>(pUTF8String);
    char16_t* target = pUTF16String;
    //char16_t* targetEnd = pUTF16String + uBufferLength;
    // 遍历
    while (*source) {
        char32_t ch = 0;
        unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
        /*if (extraBytesToRead >= sourceEnd - source) {
        result = sourceExhausted; break;
        }*/
        /* Do this check whether lenient or strict */
        /*if (!isLegalUTF8(source, extraBytesToRead + 1)) {
        result = sourceIllegal;
        break;
        }*/
        /*
        * The cases all fall through. See "Note A" below.
        */
        switch (extraBytesToRead) {
        case 5: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
        case 4: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
        case 3: ch += *source++; ch <<= 6;
        case 2: ch += *source++; ch <<= 6;
        case 1: ch += *source++; ch <<= 6;
        case 0: ch += *source++;
        }
        ch -= offsetsFromUTF8[extraBytesToRead];

        /*if (target >= targetEnd) {
            source -= (extraBytesToRead + 1); // Back up source pointer!
            length = 0; break;
        }*/
        if (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
                                 /* UTF-16 surrogate values are illegal in UTF-32 */
            if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
#ifdef STRICT_CONVERSION
                source -= (extraBytesToRead + 1); /* return to the illegal value itself */
                length = 0;
                break;
#else
                *target++ = UNI_REPLACEMENT_CHAR;
                ++length;
#endif
            }
            else {
                *target++ = (char16_t)ch; /* normal case */
                ++length;
            }
        }
        else if (ch > UNI_MAX_UTF16) {
#ifdef STRICT_CONVERSION
            length = 0;
            source -= (extraBytesToRead + 1); /* return to the start */
            break; /* Bail out; shouldn't continue */
#else
            *target++ = UNI_REPLACEMENT_CHAR;
            ++length;
#endif
        }
        else {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            /*if (target + 1 >= targetEnd) {
                source -= (extraBytesToRead + 1); // Back up source pointer!
                length = 0; break;
            }*/
            ch -= halfBase;
            *target++ = (char16_t)((ch >> halfShift) + UNI_SUR_HIGH_START);
            *target++ = (char16_t)((ch & halfMask) + UNI_SUR_LOW_START);
            length += 2;
        }
    }
    // 最后修正
    return length;
}

// --------------  CUIConsole ------------
// CUIConsole 构造函数
LongUI::CUIConsole::CUIConsole() noexcept {
    //::InitializeCriticalSection(&m_cs);  
    m_name[0] = L'\0';
    { if (m_hConsole != INVALID_HANDLE_VALUE) this->Close(); }
}

// CUIConsole 析构函数
LongUI::CUIConsole::~CUIConsole() noexcept {
    this->Close();
    // 关闭
    if (m_hConsole != INVALID_HANDLE_VALUE) {
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
    }
    //::DeleteCriticalSection(&m_cs);
}

// CUIConsole 关闭
long LongUI::CUIConsole::Close() noexcept {
    if (!(*this))
        return -1;
    else
        return ::DisconnectNamedPipe(m_hConsole);
}

// CUIConsole 输出
long LongUI::CUIConsole::Output(const wchar_t * str, bool flush, size_t len) noexcept {
    // 过长则分批
    if (len > LongUIStringBufferLength) {
        // 直接递归
        while (len) {
            auto len_in = len > LongUIStringBufferLength ? LongUIStringBufferLength : len;
            this->Output(str, true, len_in);
            len -= len_in;
            str += len_in;
        }
        return 0;
    }
    // 计算目标
    if (m_length + len > LongUIStringBufferLength) {
        flush = true;
    }
    // 写入
    if (m_length + len < LongUIStringBufferLength) {
        std::memcpy(m_buffer + m_length, str, len * sizeof(wchar_t));
        m_length += len;
        str = nullptr;
        // 不用flush
        if (!flush) return 0;
    }
    DWORD dwWritten = DWORD(-1);
    // 写入
    auto safe_write_file = [this, &dwWritten]() {
        return ::WriteFile(
            m_hConsole, m_buffer, 
            static_cast<uint32_t>(m_length * sizeof(wchar_t)), 
            &dwWritten, nullptr
            );
    };
    // 先写入缓冲区
    if (m_length) {
        safe_write_file();
        m_length = 0;
    }
    // 再写入目标
    if (str) {
        len *= sizeof(wchar_t);
        return (!safe_write_file() || (int)dwWritten != len) ? -1 : (int)dwWritten;
    }
    return 0;
}

// CUIConsole 创建
long LongUI::CUIConsole::Create(const wchar_t* lpszWindowTitle, Config& config) noexcept {
    // 二次创建?
    if (m_hConsole != INVALID_HANDLE_VALUE) {
        ::DisconnectNamedPipe(m_hConsole);
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
    }
    // 先复制
    std::wcscpy(m_name, LR"(\\.\pipe\)");
    wchar_t logger_name_buffer[128];
    // 未给logger?
    if (!config.logger_name) {
        static float s_times = 1.f;
        std::swprintf(
            logger_name_buffer, lengthof(logger_name_buffer),
            L"logger_%7.5f",
            float(::GetTickCount()) / float(1000 * 60 * 60) *
            (float(std::rand()) / float(RAND_MAX)) * s_times
            );
        config.logger_name = logger_name_buffer;
        ++s_times;
    }
    std::wcscat(m_name, config.logger_name);
    // 创建管道
    m_hConsole = ::CreateNamedPipeW(
        m_name,
        PIPE_ACCESS_OUTBOUND,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1,
        4096,   // 输出缓存
        0,      // 输入缓存
        1,
        nullptr
        );
    // 无效
    if (m_hConsole == INVALID_HANDLE_VALUE) {
        ::MessageBoxW(nullptr, L"CreateNamedPipe failed", L"CUIConsole::Create failed", MB_ICONERROR);
        return -1;
    }
    // 创建控制台
    PROCESS_INFORMATION pi;
    STARTUPINFOW si; ::GetStartupInfoW(&si);
    const wchar_t* DEFAULT_HELPER_EXE = L"ConsoleHelper.exe";

    wchar_t cmdline[MAX_PATH];;
    if (!config.helper_executable)
        config.helper_executable = DEFAULT_HELPER_EXE;

    std::swprintf(cmdline, MAX_PATH, L"%ls %ls", config.helper_executable, config.logger_name);
    BOOL bRet = ::CreateProcessW(nullptr, cmdline, nullptr, nullptr, false, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi);
    if (!bRet) {
        auto path = ::_wgetenv(L"ConsoleLoggerHelper");
        if (path) {
            std::swprintf(cmdline, MAX_PATH, L"%ls %ls", path, config.logger_name);
            bRet = ::CreateProcessW(nullptr, nullptr, nullptr, nullptr, false, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi);
        }
        if (!bRet) {
            ::MessageBoxW(nullptr, L"Helper executable not found", L"ConsoleLogger failed", MB_ICONERROR);
            ::CloseHandle(m_hConsole);
            m_hConsole = INVALID_HANDLE_VALUE;
            return -1;
        }
    }
    // 连接
    BOOL bConnected = ::ConnectNamedPipe(m_hConsole, nullptr) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
    // 连接失败
    if (!bConnected) {
        ::MessageBoxW(nullptr, L"ConnectNamedPipe failed", L"ConsoleLogger failed", MB_ICONERROR);
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
        return -1;
    }

    DWORD cbWritten;

    // 特性

    wchar_t buffer[128];
    // 传送标题
    if (!lpszWindowTitle) lpszWindowTitle = m_name + 9;
    std::swprintf(buffer, lengthof(buffer), L"TITLE: %ls\r\n", lpszWindowTitle);
    uint32_t len_in_byte = static_cast<uint32_t>(std::wcslen(buffer) * sizeof(wchar_t));
    ::WriteFile(m_hConsole, buffer, len_in_byte, &cbWritten, nullptr);
    if (cbWritten != len_in_byte) {
        ::MessageBoxW(nullptr, L"WriteFile failed(1)", L"ConsoleLogger failed", MB_ICONERROR);
        ::DisconnectNamedPipe(m_hConsole);
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
        return -1;
    }

    // 传送位置
    if (config.position_xy != -1) {
        std::swprintf(buffer, lengthof(buffer), L"POS: %d\r\n", config.position_xy);
        len_in_byte = static_cast<uint32_t>(std::wcslen(buffer) * sizeof(wchar_t));
        ::WriteFile(m_hConsole, buffer, len_in_byte, &cbWritten, nullptr);
        if (cbWritten != len_in_byte) {
            ::MessageBoxW(nullptr, L"WriteFile failed(1.1)", L"ConsoleLogger failed", MB_ICONERROR);
            ::DisconnectNamedPipe(m_hConsole);
            ::CloseHandle(m_hConsole);
            m_hConsole = INVALID_HANDLE_VALUE;
            return -1;
        }
    }
    // 传送属性
    if (config.atribute) {
        std::swprintf(buffer, lengthof(buffer), L"ATTR: %d\r\n", config.atribute);
        len_in_byte = static_cast<uint32_t>(std::wcslen(buffer) * sizeof(wchar_t));
        ::WriteFile(m_hConsole, buffer, len_in_byte, &cbWritten, nullptr);
        if (cbWritten != len_in_byte) {
            ::MessageBoxW(nullptr, L"WriteFile failed(1.2)", L"ConsoleLogger failed", MB_ICONERROR);
            ::DisconnectNamedPipe(m_hConsole);
            ::CloseHandle(m_hConsole);
            m_hConsole = INVALID_HANDLE_VALUE;
            return -1;
        }
    }

    // 传送缓存区大小
    if (config.buffer_size_x != -1 && config.buffer_size_y != -1) {
        std::swprintf(buffer, lengthof(buffer), L"BUFFER-SIZE: %dx%d\r\n", config.buffer_size_x, config.buffer_size_y);
        len_in_byte = static_cast<uint32_t>(std::wcslen(buffer) * sizeof(wchar_t));
        ::WriteFile(m_hConsole, buffer, len_in_byte, &cbWritten, nullptr);
        if (cbWritten != len_in_byte) {
            ::MessageBoxW(nullptr, L"WriteFile failed(2)", L"ConsoleLogger failed", MB_ICONERROR);
            ::DisconnectNamedPipe(m_hConsole);
            ::CloseHandle(m_hConsole);
            m_hConsole = INVALID_HANDLE_VALUE;
            return -1;
        }
    }

    // 添加头
    if (false) {
        ::DisconnectNamedPipe(m_hConsole);
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
        return -1;
    }

    // 传送完毕

    buffer[0] = 0;
    ::WriteFile(m_hConsole, buffer, 2, &cbWritten, nullptr);
    if (cbWritten != 2) {
        ::MessageBoxW(nullptr, L"WriteFile failed(3)", L"ConsoleLogger failed", MB_ICONERROR);
        ::DisconnectNamedPipe(m_hConsole);
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
        return -1;
    }
    // 关闭进程句柄
    if(bRet) {
        ::CloseHandle(pi.hProcess);
        ::CloseHandle(pi.hThread);
    }
    return 0;
}



// --------------  CUIDefaultConfigure ------------
#ifdef LONGUI_WITH_DEFAULT_CONFIG

// longui
namespace LongUI {
    // 创建XML资源读取器
    auto CreateResourceLoaderForXML(CUIManager& manager, const char* xml) noexcept ->IUIResourceLoader*;
}

// 创建接口
auto LongUI::CUIDefaultConfigure::CreateInterface(const IID & riid, void ** ppvObject) noexcept -> HRESULT {
    // 资源读取器
    if (riid == LongUI::GetIID<LongUI::IUIResourceLoader>()) {
        *ppvObject = LongUI::CreateResourceLoaderForXML(m_manager, this->resource);
    }
    // 脚本
    else if (riid == LongUI::GetIID<LongUI::IUIScript>()) {

    }
    // 字体集
    else if (riid == LongUI::GetIID<IDWriteFontCollection>()) {

    }
    // 检查
    return (*ppvObject) ? S_OK : E_NOINTERFACE;
}

auto LongUI::CUIDefaultConfigure::ChooseAdapter(const DXGI_ADAPTER_DESC1 adapters[], const size_t length) noexcept -> size_t {
    UNREFERENCED_PARAMETER(adapters);
    // 核显卡优先 
#ifdef LONGUI_NUCLEAR_FIRST
    for (size_t i = 0; i < length; ++i) {
        DXGI_ADAPTER_DESC1& desc = adapters[i];
        if (!std::wcsncmp(L"NVIDIA", desc.Description, 6))
            return i;
    }
#endif
    return length;
}

// CUIDefaultConfigure 显示错误信息
auto LongUI::CUIDefaultConfigure::ShowError(const wchar_t * str_a, const wchar_t* str_b) noexcept -> void {
    assert(str_a && "bad argument!");
    if (!str_b) str_b = L"Error!";
    ::MessageBoxW(::GetForegroundWindow(), str_a, str_b, MB_ICONERROR);
#ifdef _DEBUG
    assert(!"error");
#endif
}

#ifdef _DEBUG
#include <ctime>
// 输出调试字符串
auto LongUI::CUIDefaultConfigure::OutputDebugStringW(
    DebugStringLevel level, const wchar_t * string, bool flush) noexcept -> void {
    auto& console = this->consoles[level];
    // 无效就创建
    if (!console) {
        this->CreateConsole(level);
    }
    // 有效就输出
    if (console) {
        console.Output(string, flush);
    }
    // 输出到日志?
    if (m_pLogFile && level == DebugStringLevel::DLevel_Log) {
        // 五秒精度
        constexpr uint32_t UNIT = 5'000;
        auto now = ::GetTickCount();
        if ((now / UNIT) != (static_cast<uint32_t>(m_timeTick) / UNIT)) {
            m_timeTick = static_cast<size_t>(now);
            // 不一样则输出时间
            std::time_t time = std::time(nullptr);
            wchar_t buffer[LongUIStringBufferLength];
            std::wcsftime(
                buffer, LongUIStringBufferLength,
                L"[%c]\r\n", std::localtime(&time)
                );
            std::fwrite(buffer, sizeof(wchar_t), std::wcslen(buffer), m_pLogFile);
        }
        std::fwrite(string, sizeof(wchar_t), std::wcslen(string), m_pLogFile);

    }
}

void LongUI::CUIDefaultConfigure::CreateConsole(DebugStringLevel level) noexcept {
    CUIConsole::Config config;
    config.x = -5;
    config.y = int16_t(level) * 128;
    switch (level)
    {
    case LongUI::DLevel_None:
        break;
    case LongUI::DLevel_Log:
        break;
    case LongUI::DLevel_Hint:
        break;
    case LongUI::DLevel_Warning:
        config.atribute = FOREGROUND_RED | FOREGROUND_GREEN;
        break;
    case LongUI::DLevel_Error:
    case LongUI::DLevel_Fatal:
        config.atribute = FOREGROUND_RED;
        break;
    }
    assert(level < LongUI::DLEVEL_SIZE);
    // 名称
    const wchar_t* strings[LongUI::DLEVEL_SIZE] = {
        L"None      Console",
        L"Log       Console",
        L"Hint      Console",
        L"Warning   Console",
        L"Error     Console",
        L"Fatal     Console"
    };

    this->consoles[level].Create(strings[level], config);
}

#endif
#endif

// ------------------- MMFVideo -----------------------
#ifdef LONGUI_WITH_MMFVIDEO
// MMFVideo 事件通知
HRESULT LongUI::Component::MMFVideo::EventNotify(DWORD event, DWORD_PTR param1, DWORD param2) noexcept {
    UNREFERENCED_PARAMETER(param2);
    switch (event)
    {
    case MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA:
        m_bEOS = false;
        break;
    case MF_MEDIA_ENGINE_EVENT_CANPLAY:
        this->Play();
        break;
    case MF_MEDIA_ENGINE_EVENT_PLAY:
        m_bPlaying = true;
        break;
    case MF_MEDIA_ENGINE_EVENT_PAUSE:
        m_bPlaying = false;
        break;
    case MF_MEDIA_ENGINE_EVENT_ENDED:
        m_bPlaying = false;
        m_bEOS = true;
        break;
    case MF_MEDIA_ENGINE_EVENT_TIMEUPDATE:
        break;
    case MF_MEDIA_ENGINE_EVENT_NOTIFYSTABLESTATE:
        ::SetEvent(reinterpret_cast<HANDLE>(param1));
        break;
    case MF_MEDIA_ENGINE_EVENT_ERROR:
    {
        //auto err = MF_MEDIA_ENGINE_ERR(param1);
        //auto hr = HRESULT(param2);
        //int a = 9;
    }
    break;
    }
    return S_OK;
}


// MMFVideo 初始化
auto LongUI::Component::MMFVideo::Initialize() noexcept ->HRESULT {
    HRESULT hr = S_OK;
    IMFAttributes* attributes = nullptr;
    // 创建MF属性
    if (SUCCEEDED(hr)) {
        hr = ::MFCreateAttributes(&attributes, 1);
    }
    // 设置属性: DXGI管理器
    if (SUCCEEDED(hr)) {
        hr = attributes->SetUnknown(MF_MEDIA_ENGINE_DXGI_MANAGER, UIManager_MFDXGIDeviceManager);
    }
    // 设置属性: 事件通知
    if (SUCCEEDED(hr)) {
        hr = attributes->SetUnknown(MF_MEDIA_ENGINE_CALLBACK, this);
    }
    // 设置属性: 输出格式
    if (SUCCEEDED(hr)) {
        hr = attributes->SetUINT32(MF_MEDIA_ENGINE_VIDEO_OUTPUT_FORMAT, DXGI_FORMAT_B8G8R8A8_UNORM);
    }
    // 创建媒体引擎
    if (SUCCEEDED(hr)) {
        constexpr DWORD flags = MF_MEDIA_ENGINE_WAITFORSTABLE_STATE;
        hr = UIManager_MFMediaEngineClassFactory->CreateInstance(flags, attributes, &m_pMediaEngine);
    }
    // 获取Ex版
    if (SUCCEEDED(hr)) {
        hr = m_pMediaEngine->QueryInterface(LongUI_IID_PV_ARGS(m_pEngineEx));
    }
    assert(SUCCEEDED(hr));
    LongUI::SafeRelease(attributes);
    return hr;
}

// MMFVideo: 重建
auto LongUI::Component::MMFVideo::Recreate() noexcept ->HRESULT {
    LongUI::SafeRelease(m_pTargetSurface);
    LongUI::SafeRelease(m_pDrawSurface);
    return this->recreate_surface();
}

// MMFVideo: 渲染
void LongUI::Component::MMFVideo::Render(D2D1_RECT_F* dst) const noexcept {
    UNREFERENCED_PARAMETER(dst);
    /*const MFARGB bkColor = { 0,0,0,0 };
    assert(m_pMediaEngine);
    // 表面无效
    if (!m_pDrawSurface) {
        this->recreate_surface();
    }
    // 表面有效
    if (m_pDrawSurface) {
        LONGLONG pts;
        if ((m_pMediaEngine->OnVideoStreamTick(&pts)) == S_OK) {
            D3D11_TEXTURE2D_DESC desc;
            m_pTargetSurface->GetDesc(&desc);
            m_pMediaEngine->TransferVideoFrame(m_pTargetSurface, nullptr, &dst_rect, &bkColor);
            m_pDrawSurface->CopyFromBitmap(nullptr, m_pSharedSurface, nullptr);
        }
        D2D1_RECT_F src = { 0.f, 0.f,  float(dst_rect.right), float(dst_rect.bottom) };
        UIManager_RenderTarget->DrawBitmap(m_pDrawSurface, dst, 1.f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &src
            );
    }*/
}

// Component::MMFVideo 构造函数
LongUI::Component::MMFVideo::MMFVideo() noexcept {
    force_cast(dst_rect) = { 0 };
}

// Component::MMFVideo 析构函数
LongUI::Component::MMFVideo::~MMFVideo() noexcept {
    if (m_pMediaEngine) {
        m_pMediaEngine->Shutdown();
    }
    LongUI::SafeRelease(m_pMediaEngine);
    LongUI::SafeRelease(m_pEngineEx);
    LongUI::SafeRelease(m_pTargetSurface);
    LongUI::SafeRelease(m_pSharedSurface);
    LongUI::SafeRelease(m_pDrawSurface);
}

// 重建表面
auto LongUI::Component::MMFVideo::recreate_surface() noexcept ->HRESULT {
    // 有效情况下
    DWORD w, h; HRESULT hr = S_FALSE;
    if (this->HasVideo() && SUCCEEDED(hr = m_pMediaEngine->GetNativeVideoSize(&w, &h))) {
        force_cast(dst_rect.right) = w;
        force_cast(dst_rect.bottom) = h;
        // 获取规范大小
        w = LongUI::MakeAsUnit(w); h = LongUI::MakeAsUnit(h);
        // 检查承载大小
        D2D1_SIZE_U size = m_pDrawSurface ? m_pDrawSurface->GetPixelSize() : D2D1::SizeU();
        // 重建表面
        if (w > size.width || h > size.height) {
            size = { w, h };
            LongUI::SafeRelease(m_pTargetSurface);
            LongUI::SafeRelease(m_pSharedSurface);
            LongUI::SafeRelease(m_pDrawSurface);
            IDXGISurface* surface = nullptr;
#if 0
            D3D11_TEXTURE2D_DESC desc = {
                w, h, 1, 1, DXGI_FORMAT_B8G8R8A8_UNORM, {1, 0}, D3D11_USAGE_DEFAULT,
                D3D11_BIND_RENDER_TARGET, 0, 0
            };
            hr = UIManager_D3DDevice->CreateTexture2D(&desc, nullptr, &m_pTargetSurface);
            // 获取Dxgi表面
            if (SUCCEEDED(hr)) {
                hr = m_pTargetSurface->QueryInterface(LongUI_IID_PV_ARGS(surface));
            }
            // 从Dxgi表面创建位图
            if (SUCCEEDED(hr)) {
                hr = UIManager_RenderTarget->CreateBitmapFromDxgiSurface(
                    surface, nullptr, &m_pDrawSurface
                    );
            }
#else
            // 创建D2D位图
            D2D1_BITMAP_PROPERTIES1 prop = {
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                LongUI::GetDpiX(),
                LongUI::GetDpiY(),
                D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_TARGET, nullptr
            };
            hr = UIManager_RenderTarget->CreateBitmap(size, nullptr, size.width * 4, &prop, &m_pSharedSurface);
            // 获取Dxgi表面
            if (SUCCEEDED(hr)) {
                hr = m_pSharedSurface->GetSurface(&surface);
            }
            // 获取D3D11 2D纹理
            if (SUCCEEDED(hr)) {
                hr = surface->QueryInterface(LongUI_IID_PV_ARGS(m_pTargetSurface));
            }
            // 创建刻画位图
            if (SUCCEEDED(hr)) {
                prop.bitmapOptions = D2D1_BITMAP_OPTIONS_NONE;
                hr = UIManager_RenderTarget->CreateBitmap(size, nullptr, size.width * 4, &prop, &m_pDrawSurface);
            }
#endif
            LongUI::SafeRelease(surface);
        }
    }
    return hr;
}

#endif



// -----------------------------



// π
constexpr float EZ_PI = 3.1415296F;
// 二分之一π
constexpr float EZ_PI_2 = 1.5707963F;

// 反弹渐出
float inline __fastcall BounceEaseOut(float p) noexcept {
    if (p < 4.f / 11.f) {
        return (121.f * p * p) / 16.f;
    }
    else if (p < 8.f / 11.f) {
        return (363.f / 40.f * p * p) - (99.f / 10.f * p) + 17.f / 5.f;
    }
    else if (p < 9.f / 10.f) {
        return (4356.f / 361.f * p * p) - (35442.f / 1805.f * p) + 16061.f / 1805.f;
    }
    else {
        return (54.f / 5.f * p * p) - (513.f / 25.f * p) + 268.f / 25.f;
    }
}


// CUIAnimation 缓动函数
float __fastcall LongUI::EasingFunction(AnimationType type, float p) noexcept {
    assert((p >= 0.f && p <= 1.f) && "bad argument");
    switch (type)
    {
    default:
        assert(!"type unknown");
        __fallthrough;
    case LongUI::AnimationType::Type_LinearInterpolation:
        // 线性插值     f(x) = x
        return p;
    case LongUI::AnimationType::Type_QuadraticEaseIn:
        // 平次渐入     f(x) = x^2
        return p * p;
    case LongUI::AnimationType::Type_QuadraticEaseOut:
        // 平次渐出     f(x) =  -x^2 + 2x
        return -(p * (p - 2.f));
    case LongUI::AnimationType::Type_QuadraticEaseInOut:
        // 平次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^2)
        // [0.5, 1.f]   f(x) = -(1/2)((2x-1)*(2x-3)-1) ; 
        return p < 0.5f ? (p * p * 2.f) : ((-2.f * p * p) + (4.f * p) - 1.f);
    case LongUI::AnimationType::Type_CubicEaseIn:
        // 立次渐入     f(x) = x^3;
        return p * p * p;
    case LongUI::AnimationType::Type_CubicEaseOut:
        // 立次渐出     f(x) = (x - 1)^3 + 1
    {
        float f = p - 1.f;
        return f * f * f + 1.f;
    }
    case LongUI::AnimationType::Type_CubicEaseInOut:
        // 立次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^3) 
        // [0.5, 1.f]   f(x) = (1/2)((2x-2)^3 + 2) 
        if (p < 0.5f) {
            return p * p * p * 2.f;
        }
        else {
            float f = (2.f * p) - 2.f;
            return 0.5f * f * f * f + 1.f;
        }
    case LongUI::AnimationType::Type_QuarticEaseIn:
        // 四次渐入     f(x) = x^4
    {
        float f = p * p;
        return f * f;
    }
    case LongUI::AnimationType::Type_QuarticEaseOut:
        // 四次渐出     f(x) = 1 - (x - 1)^4
    {
        float f = (p - 1.f); f *= f;
        return 1.f - f * f;
    }
    case LongUI::AnimationType::Type_QuarticEaseInOut:
        // 四次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^4)
        // [0.5, 1.f]   f(x) = -(1/2)((2x-2)^4 - 2)
        if (p < 0.5f) {
            float f = p * p;
            return 8.f * f * f;
        }
        else {
            float f = (p - 1.f); f *= f;
            return 1.f - 8.f * f * f;
        }
    case LongUI::AnimationType::Type_QuinticEaseIn:
        // 五次渐入     f(x) = x^5
    {
        float f = p * p;
        return f * f * p;
    }
    case LongUI::AnimationType::Type_QuinticEaseOut:
        // 五次渐出     f(x) = (x - 1)^5 + 1
    {
        float f = (p - 1.f);
        return f * f * f * f * f + 1.f;
    }
    case LongUI::AnimationType::Type_QuinticEaseInOut:
        // 五次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^5) 
        // [0.5, 1.f]   f(x) = (1/2)((2x-2)^5 + 2)
        if (p < 0.5) {
            float f = p * p;
            return 16.f * f * f * p;
        }
        else {
            float f = ((2.f * p) - 2.f);
            return  f * f * f * f * f * 0.5f + 1.f;
        }
    case LongUI::AnimationType::Type_SineEaseIn:
        // 正弦渐入     
        return std::sin((p - 1.f) * EZ_PI_2) + 1.f;
    case LongUI::AnimationType::Type_SineEaseOut:
        // 正弦渐出     
        return std::sin(p * EZ_PI_2);
    case LongUI::AnimationType::Type_SineEaseInOut:
        // 正弦出入     
        return 0.5f * (1.f - std::cos(p * EZ_PI));
    case LongUI::AnimationType::Type_CircularEaseIn:
        // 四象圆弧
        return 1.f - std::sqrt(1.f - (p * p));
    case LongUI::AnimationType::Type_CircularEaseOut:
        // 二象圆弧
        return std::sqrt((2.f - p) * p);
    case LongUI::AnimationType::Type_CircularEaseInOut:
        // 圆弧出入
        if (p < 0.5f) {
            return 0.5f * (1.f - std::sqrt(1.f - 4.f * (p * p)));
        }
        else {
            return 0.5f * (std::sqrt(-((2.f * p) - 3.f) * ((2.f * p) - 1.f)) + 1.f);
        }
    case LongUI::AnimationType::Type_ExponentialEaseIn:
        // 指数渐入     f(x) = 2^(10(x - 1))
        return (p == 0.f) ? (p) : (std::pow(2.f, 10.f * (p - 1.f)));
    case LongUI::AnimationType::Type_ExponentialEaseOut:
        // 指数渐出     f(x) =  -2^(-10x) + 1
        return (p == 1.f) ? (p) : (1.f - std::powf(2.f, -10.f * p));
    case LongUI::AnimationType::Type_ExponentialEaseInOut:
        // 指数出入
        // [0,0.5)      f(x) = (1/2)2^(10(2x - 1)) 
        // [0.5,1.f]    f(x) = -(1/2)*2^(-10(2x - 1))) + 1 
        if (p == 0.0f || p == 1.0f) return p;
        if (p < 0.5f) {
            return 0.5f * std::powf(2.f, (20.f * p) - 10.f);
        }
        else {
            return -0.5f * std::powf(2.f, (-20.f * p) + 1.f) + 1.f;
        }
    case LongUI::AnimationType::Type_ElasticEaseIn:
        // 弹性渐入
        return std::sin(13.f * EZ_PI_2 * p) * std::pow(2.f, 10.f * (p - 1.f));
    case LongUI::AnimationType::Type_ElasticEaseOut:
        // 弹性渐出
        return std::sin(-13.f * EZ_PI_2 * (p + 1.f)) * std::powf(2.f, -10.f * p) + 1.f;
    case LongUI::AnimationType::Type_ElasticEaseInOut:
        // 弹性出入
        if (p < 0.5f) {
            return 0.5f * std::sin(13.f * EZ_PI_2 * (2.f * p)) * std::pow(2.f, 10.f * ((2.f * p) - 1.f));
        }
        else {
            return 0.5f * (std::sin(-13.f * EZ_PI_2 * ((2.f * p - 1.f) + 1.f)) * std::pow(2.f, -10.f * (2.f * p - 1.f)) + 2.f);
        }
    case LongUI::AnimationType::Type_BackEaseIn:
        // 回退渐入
        return  p * p * p - p * std::sin(p * EZ_PI);
    case LongUI::AnimationType::Type_BackEaseOut:
        // 回退渐出
    {
        float f = (1.f - p);
        return 1.f - (f * f * f - f * std::sin(f * EZ_PI));
    }
    case LongUI::AnimationType::Type_BackEaseInOut:
        // 回退出入
        if (p < 0.5f) {
            float f = 2.f * p;
            return 0.5f * (f * f * f - f * std::sin(f * EZ_PI));
        }
        else {
            float f = (1.f - (2 * p - 1.f));
            return 0.5f * (1.f - (f * f * f - f * std::sin(f * EZ_PI))) + 0.5f;
        }
    case LongUI::AnimationType::Type_BounceEaseIn:
        // 反弹渐入
        return 1.f - ::BounceEaseOut(1.f - p);
    case LongUI::AnimationType::Type_BounceEaseOut:
        // 反弹渐出
        return ::BounceEaseOut(p);
    case LongUI::AnimationType::Type_BounceEaseInOut:
        // 反弹出入
        if (p < 0.5f) {
            return 0.5f * (1.f - ::BounceEaseOut(1.f - (p*2.f)));
        }
        else {
            return 0.5f * ::BounceEaseOut(p * 2.f - 1.f) + 0.5f;
        }
    }
}

// longui namespace
namespace LongUI {
    // BKDR 哈希
    auto __fastcall BKDRHash(const char* str) noexcept -> uint32_t {
        constexpr uint32_t seed = 131;
        uint32_t code = 0;
        auto p = reinterpret_cast<const unsigned char*>(str);
        while (*p) code = code * seed + (*p++);
        return code;
    }
    // BKDR 哈希
    auto __fastcall BKDRHash(const wchar_t* str) noexcept -> uint32_t {
        constexpr uint32_t seed = 131;
        uint32_t code = 0;
        while (*str) code = code * seed + (*str++);
        return code;
    }
}