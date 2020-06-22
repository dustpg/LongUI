// LUI
#include <core/ui_manager.h>
#include <dropdrag/ui_droptarget.h>
#ifndef LUI_NO_DROPDRAG
// C/C++
#include <cassert>

// Windows API
#define NOMINMAX
#include <Windows.h>
#include <Shobjidl.h>

/// <summary>
/// Queries the interface.
/// </summary>
/// <param name="id">The identifier.</param>
/// <param name="ppvObject">The PPV object.</param>
/// <returns></returns>
HRESULT LongUI::CUIWndDropTarget::QueryInterface(const IID& id, void **ppvObject) noexcept {
    IUnknown* ptr = nullptr;
    if (id == IID_IUnknown) ptr = static_cast<IUnknown*>(this);
    else if (id == IID_IDropTarget) ptr = static_cast<IDropTarget*>(this);
    if (*ppvObject = ptr) { this->AddRef(); return S_OK; }
    return E_NOINTERFACE;
}

/// <summary>
/// IDropTarget::DragEnter 实现
/// </summary>
/// <param name="pDataObj"></param>
/// <param name="grfKeyState"></param>
/// <param name="pt"></param>
/// <param name="pdwEffect"></param>
/// <returns></returns>
HRESULT  LongUI::CUIWndDropTarget::DragEnter(IDataObject* pDataObj,
    DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) noexcept {
    // 检查参数
    if (!pDataObj) return E_INVALIDARG;
    //POINT ppt = { pt.x, pt.y };
    //auto& helper = UIManager.RefDropTargetHelper();
    //helper.DragEnter(hwnd, pDataObj, &ppt, *pdwEffect);
    return S_OK;
}

/// <summary>
/// IDropTarget::DragOver 实现
/// </summary>
/// <param name="grfKeyState"></param>
/// <param name="pt"></param>
/// <param name="pdwEffect"></param>
/// <returns></returns>
HRESULT LongUI::CUIWndDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) noexcept {
    UNREFERENCED_PARAMETER(grfKeyState);
    // TODO:
    if (false) {
        /*LongUI::EventArgument arg;
        ::SetLongUIEventArgument(arg, m_hwnd, pt);
        arg.sender = this;
        // 第一个控件?
        if (m_pDragDropControl == control) {
            // 一样就是Over
            arg.event = LongUI::Event::Event_DragOver;
        }
        else {
            // 对老控件发送离开事件
            if (m_pDragDropControl) {
                arg.event = LongUI::Event::Event_DragLeave;
                m_pDragDropControl->DoEvent(arg);
            }
            // 新控件发送进入
            arg.event = LongUI::Event::Event_DragEnter;
            m_pDragDropControl = control;
        }
        arg.cf.dataobj = m_pCurDataObject;
        arg.cf.outeffect = pdwEffect;
        if (!control->DoEvent(arg)) *pdwEffect = DROPEFFECT_NONE;*/
    }
    else {
        // 不支持
        *pdwEffect = DROPEFFECT_NONE;
    }
    // 由帮助器处理
    //auto& helper = UIManager.RefDropTargetHelper();
    //helper.DragOver(&ppt, *pdwEffect);
    return S_OK;
}

/// <summary>
/// IDropTarget::DragLeave 实现
/// </summary>
/// <returns></returns>
HRESULT LongUI::CUIWndDropTarget::DragLeave(void) noexcept {
    // 发送事件
    //if (m_pDragDropControl) {
        /*LongUI::EventArgument arg = { 0 };
        arg.sender = this;
        arg.event = LongUI::Event::Event_DragLeave;
        m_pDragDropControl->DoEvent(arg);
        m_pDragDropControl = nullptr;
        // 存在捕获控件?
        /*if (m_pCapturedControl) {
            this->ReleaseCapture();
            /*arg.sender = nullptr;
            arg.msg = WM_LBUTTONUP;
            m_pCapturedControl->DoEvent(arg);
        }*/
    //}
    /*OnDragLeave(m_hTargetWnd);*/
    //m_pDragDropControl = nullptr;
    //m_isDataAvailable = TRUE;
    // 由帮助器处理
    auto& helper = UIManager.RefDropTargetHelper();
    return helper.DragLeave();
}

/// <summary>
/// IDropTarget::Drop 实现
/// </summary>
/// <param name="pDataObj"></param>
/// <param name="grfKeyState"></param>
/// <param name="pt"></param>
/// <param name="pdwEffect"></param>
/// <returns></returns>
HRESULT LongUI::CUIWndDropTarget::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) noexcept {
    // 发送事件
    //if (m_pDragDropControl) {
        /*LongUI::EventArgument arg;
        ::SetLongUIEventArgument(arg, m_hwnd, pt);
        arg.sender = this;
        arg.event = LongUI::Event::Event_Drop;
        arg.cf.dataobj = m_pCurDataObject;
        arg.cf.outeffect = pdwEffect;
        // 发送事件
        m_pDragDropControl->DoEvent(arg);
        m_pDragDropControl = nullptr;*/
    //}
    // 检查参数
    if (!pDataObj) return E_INVALIDARG;
    // 由帮助器处理
    //auto& helper = UIManager.RefDropTargetHelper();
    //helper.Drop(pDataObj, &ppt, *pdwEffect);
    //*pdwEffect = ::GetDropEffect(grfKeyState, *pdwEffect);
    return S_OK;
}

#endif
