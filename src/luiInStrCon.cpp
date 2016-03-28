#include "Core/luiManager.h"
#include "LongUI/luiUiInput.h"
#include "Core/luiString.h"
#include "Platonly/luiPoUtil.h"
#include "Graphics/luiGrAnim.h"
#include "LongUI/luiUiHlper.h"
#include "Platless/luiPlUtil.h"
#include <algorithm>

/// <summary>
/// Initializes a new instance of the <see cref="CUIInput"/> class.
/// </summary>
LongUI::CUIInput::CUIInput() noexcept { 
    m_ptMouseL = { 0, 0 }; 
    std::memset(m_abKeyStateBuffer, 0, sizeof(m_abKeyStateBuffer)); 
};

/// <summary>
/// Initializes the specified .
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
auto LongUI::CUIInput::Init(HWND hwnd) noexcept ->HRESULT {
    assert(hwnd && "bad argument");
    // 注册鼠标原始输入
    RAWINPUTDEVICE rid[2];
    rid[0].usUsagePage = 0x01; 
    rid[0].usUsage = 0x02; 
    rid[0].dwFlags = RIDEV_INPUTSINK;
    rid[0].hwndTarget = hwnd;
    rid[1].usUsagePage = 0x01; 
    rid[1].usUsage = 0x06; 
    rid[1].dwFlags = RIDEV_INPUTSINK;
    rid[1].hwndTarget = hwnd;
    BOOL rc = ::RegisterRawInputDevices(rid, 2, sizeof(rid[0]));
    return rc ? S_OK : LongUI::WinCode2HRESULT(::GetLastError());
}


// longui::impl namespace
namespace LongUI { namespace impl {
    // log2 for 2powed number
    template<size_t n>struct log2 { enum : size_t { value = log2<n / 2>::value + 1 }; };
    // for 0
    template<> struct log2<1> { enum : size_t { value = 0 }; };
}}

#if 0
    CUIString str;
    str.Format(
        L"Mouse: usFlags=%04x ulButtons=%04x usButtonFlags=%04x "
        L"usButtonData=%04x ulRawButtons=%04x lLastX=%04x lLastY"
        L"=%04x ulExtraInformation=%04x\r\n", 
        raw->data.mouse.usFlags, 
        raw->data.mouse.ulButtons, 
        raw->data.mouse.usButtonFlags, 
        raw->data.mouse.usButtonData, 
        raw->data.mouse.ulRawButtons, 
        raw->data.mouse.lLastX, 
        raw->data.mouse.lLastY, 
        raw->data.mouse.ulExtraInformation
    );
    ::OutputDebugStringW(str.c_str());
#endif

/// <summary>
/// Updates the specified .
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::CUIInput::Update(HRAWINPUT hRawInput) noexcept {
    char buffer[sizeof(RAWINPUT) * 5];
    UINT size = sizeof(buffer);
    auto code = ::GetRawInputData(hRawInput, RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER));
    // 发生错误
    if (code == UINT(-1)) {
        assert(!"ERROR");
        return;
    }
    auto* raw = reinterpret_cast<RAWINPUT*>(buffer);
    // 鼠标消息
    if (raw->header.dwType == RIM_TYPEMOUSE) {
        const auto flags = raw->data.mouse.usButtonFlags;
        enum : DWORD {
            // left button
            F_LtDn = RI_MOUSE_LEFT_BUTTON_DOWN,
            F_LtUp = RI_MOUSE_LEFT_BUTTON_UP,
            F_LtDnShiftR = impl::log2<F_LtDn>::value,
            F_LtUpShiftR = impl::log2<F_LtUp>::value,
            F_LtShiftL = impl::log2<MB::MB_L>::value,
            // right button
            F_RtDn = RI_MOUSE_RIGHT_BUTTON_DOWN,
            F_RtUp = RI_MOUSE_RIGHT_BUTTON_UP,
            F_RtDnShiftR = impl::log2<F_RtDn>::value,
            F_RtUpShiftR = impl::log2<F_RtUp>::value,
            F_RtShiftL = impl::log2<MB::MB_R>::value,
            // middle button
            F_MdDn = RI_MOUSE_MIDDLE_BUTTON_DOWN,
            F_MdUp = RI_MOUSE_MIDDLE_BUTTON_UP,
            F_MdDnShiftR = impl::log2<F_MdDn>::value,
            F_MdUpShiftR = impl::log2<F_MdUp>::value,
            F_MdShiftL = impl::log2<MB::MB_M>::value,
            // x button 1
            F_X1Dn = RI_MOUSE_BUTTON_4_DOWN,
            F_X1Up = RI_MOUSE_BUTTON_4_UP,
            F_X1DnShiftR = impl::log2<F_X1Dn>::value,
            F_X1UpShiftR = impl::log2<F_X1Up>::value,
            F_X1ShiftL = impl::log2<MB::MB_X1>::value,
            // x button 2
            F_X2Dn = RI_MOUSE_BUTTON_5_DOWN,
            F_X2Up = RI_MOUSE_BUTTON_5_UP,
            F_X2DnShiftR = impl::log2<F_X2Dn>::value,
            F_X2UpShiftR = impl::log2<F_X2Up>::value,
            F_X2ShiftL = impl::log2<MB::MB_X2>::value,
        };
        if (flags) {
            // 鼠标左键状态
            uint8_t left1 = (flags & F_LtDn) << F_LtShiftL >> F_LtDnShiftR;
            uint8_t left2 = (flags & F_LtUp) << F_LtShiftL >> F_LtUpShiftR;
            // 鼠标右键状态
            uint8_t rght1 = (flags & F_RtDn) << F_RtShiftL >> F_RtDnShiftR;
            uint8_t rght2 = (flags & F_RtUp) << F_RtShiftL >> F_RtUpShiftR;
            // 鼠标中键状态
            uint8_t midd1 = (flags & F_MdDn) << F_MdShiftL >> F_MdDnShiftR;
            uint8_t midd2 = (flags & F_MdUp) << F_MdShiftL >> F_MdUpShiftR;
            // 鼠标X1键状态
            uint8_t x1__1 = (flags & F_X1Dn) << F_X1ShiftL >> F_X1DnShiftR;
            uint8_t x1__2 = (flags & F_X1Up) << F_X1ShiftL >> F_X1UpShiftR;
            // 鼠标X2键状态
            uint8_t x2__1 = (flags & F_X2Dn) << F_X2ShiftL >> F_X2DnShiftR;
            uint8_t x2__2 = (flags & F_X2Up) << F_X2ShiftL >> F_X2UpShiftR;
            // 带入计算按中状态
            m_bufMButton[MBI_ThisFrame] |= (left1 | rght1 | midd1 | x1__1 | x2__1);
            m_bufMButton[MBI_ThisFrame] &= ~(left2 | rght2 | midd2 | x1__2 | x2__2);
        }
    }
    // 键盘消息
    else if (raw->header.dwType == RIM_TYPEKEYBOARD)  {
        KEYBOARD_OVERRUN_MAKE_CODE;

        raw->data.keyboard.MakeCode, 
        raw->data.keyboard.Flags, 
        raw->data.keyboard.Reserved, 
        raw->data.keyboard.ExtraInformation, 
        raw->data.keyboard.Message, 
        raw->data.keyboard.VKey;

        auto key = raw->data.keyboard.VKey;
        m_pKeyState[key] = !(raw->data.keyboard.Flags & RI_KEY_BREAK);
        int bk = 9;
    }
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUIInput::AfterUpdate() noexcept {
    // 获取鼠标位置
    ::GetCursorPos(&m_ptMouseL);
    m_ptMouse.x = static_cast<float>(m_ptMouseL.x);
    m_ptMouse.y = static_cast<float>(m_ptMouseL.y);
    // 更新手柄按键
    VK_LSHIFT;
    // 保存上帧输入
    m_bufMButton[MBI_LastFrame] = m_bufMButton[MBI_ThisFrame];
    // 保存上帧输入
    std::memcpy(m_pKeyStateOld, m_pKeyState, KEYBOARD_BUFFER_SIZE);
    m_bufMButton[MBI_LastFrame] = m_bufMButton[MBI_ThisFrame];
}

#if 0

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
    std::memset(&m_dataStorage, 0, sizeof(m_dataStorage));
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
HRESULT LongUI::CUIDataObject::SetBlob(CLIPFORMAT cf, const void* pvBlob, UINT cbBlob) noexcept {
    void*pv = GlobalAlloc(GPTR, cbBlob);
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


#endif
// 创建 Cce
auto LongUI::Helper::MakeCce(const char* str, Cce* data) noexcept -> uint32_t {
    assert(str && "bad argument");
    uint32_t count = 0;
    // 缓存
    char temp_buffer[LongUIStringFixedLength * 2];
    // 正式解析
    const char* word_begin = nullptr;
    for (auto itr = str;; ++itr) {
        // 获取
        char ch = *itr;
        // 段结束?
        if (ch == ',' || !ch) {
            assert(word_begin && "bad string");
            // 有效
            if (word_begin && data) {
                Helper::Cce& cce = data[count - 1];
                size_t length = size_t(itr - word_begin);
                assert(length < lengthof(temp_buffer));
                std::memcpy(temp_buffer, word_begin, length);
                temp_buffer[length] = 0;
                // 数字?
                if (word_begin[0] >= '0' && word_begin[0] <= '9') {
                    assert(!cce.id && "'cce.id' had been set, maybe more than 1 consecutive-id");
                    cce.id = size_t(LongUI::AtoI(temp_buffer));
                }
                // 英文
                else {
                    assert(!cce.func && "'cce.func' had been set");
                    cce.func = UIManager.GetCreateFunc(temp_buffer);
                    assert(cce.func && "bad func address");
                }
            }
            // 清零
            word_begin = nullptr;
            // 看看
            if (ch) continue;
            else break;
        }
        // 空白
        else if (white_space(ch)) {
            continue;
        }
        // 无效字段起始?
        if (!word_begin) {
            word_begin = itr;
            // 查看
            if ((word_begin[0] >= 'A' && word_begin[0] <= 'Z') ||
                word_begin[0] >= 'a' && word_begin[0] <= 'z') {
                if (data) {
                    data[count].func = nullptr;
                    data[count].id = 0;
                }
                ++count;
            }
        }
    }
    return count;
}


// ----------------------------------------------------------------------------------

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

/// <summary>
/// Froms the UTF8.
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
void LongUI::CUIString::FromUtf8(const char* str) noexcept {
    // 字符串有效时候
    if (str && str[0]) {
        LongUI::SafeUTF8toWideChar(
            str, [this](const wchar_t* be, const wchar_t* ed) noexcept {
            this->Set(be, uint32_t(ed - be));
        });
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
        m_pString[m_cLength] = 0;
        return;
    }
    // 将后面的字符串复制过来即可
    auto des = m_pString + offset;
    auto src = des + length;
    for (uint32_t i = 0; i < (m_cLength - offset - length + 1); ++i) {
        *des = *src;
        ++des; ++src;
    }
    m_cLength -= length;
    m_pString[m_cLength] = 0;
}

// 格式化
void LongUI::CUIString::Format(const wchar_t* format, ...) noexcept {
    va_list ap; va_start(ap, format);
    // 检查缓冲区长度 
    auto len = std::vswprintf(nullptr, 0, format, ap) + 1;
    assert(len > 0 && "bad action for 'std::vswprintf'");
    if (len <= 0) return;
    LongUI::SafeBuffer<wchar_t>(size_t(len), [len, format, ap, this](wchar_t* buf) {
        // 格式化字符串
        auto length = std::vswprintf(buf, len, format, ap);
        // 检查一下
        assert(len == (length+1) && ":(");
        // 设置
        this->Set(buf, length);
    });
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


// --------------  CUIConsole ------------
// CUIConsole 构造函数
LongUI::CUIConsole::CUIConsole() noexcept {
    //::InitializeCriticalSection(&m_cs);  
    m_name[0] = L'\0';
    { if (m_hConsole != INVALID_HANDLE_VALUE) this->Close(); }
}


// output the string
long LongUI::CUIConsole::Output(const wchar_t* str, bool flush) noexcept { 
    return this->Output(str, flush, std::wcslen(str)); 
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
    constexpr size_t BUFLEN = sizeof(m_buffer) / sizeof(m_buffer[0]);
    // 过长则分批
    if (len > BUFLEN) {
        // 直接递归
        while (len) {
            auto len_in = len > BUFLEN ? BUFLEN : len;
            this->Output(str, true, len_in);
            len -= len_in;
            str += len_in;
        }
        return 0;
    }
    // 计算目标
    if (m_length + len > BUFLEN) {
        flush = true;
    }
    // 写入
    if (m_length + len < BUFLEN) {
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
    if (bRet) {
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
    auto CreateResourceLoaderForXML(CUIManager& manager, const char* xml) noexcept->IUIResourceLoader*;
}

// 创建接口
auto LongUI::CUIDefaultConfigure::CreateInterface(const IID & riid, void** ppvObject) noexcept -> HRESULT {
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

/// <summary>
/// Chooses the adapter.
/// </summary>
/// <param name="adapters">The adapters.</param>
/// <param name="length">The length.</param>
/// <returns></returns>
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
auto LongUI::CUIDefaultConfigure::ShowError(const wchar_t* str_a, const wchar_t* str_b) noexcept -> void {
    ::MessageBoxW(::GetForegroundWindow(), str_a, str_b, MB_ICONERROR); assert(!"error");
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
            wchar_t buffer[1024];
            std::wcsftime(
                buffer, lengthof(buffer),
                L"[%c]\r\n", 
                std::localtime(&time)
            );
            std::fwrite(buffer, sizeof(wchar_t), std::wcslen(buffer), m_pLogFile);
        }
        std::fwrite(string, sizeof(wchar_t), std::wcslen(string), m_pLogFile);

    }
}

/// <summary>
/// Creates the console.
/// </summary>
/// <param name="level">The level.</param>
/// <returns></returns>
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
    assert(level < LongUI::DLEVEL_SIZE && "out of range");
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

