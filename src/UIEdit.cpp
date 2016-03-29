#include "Core/luiManager.h"
#include "Control/UIEdit.h"

// ----------------------------------------------------------------------------
// **** UIEdit
// ----------------------------------------------------------------------------

// 添加事件监听
bool LongUI::UIEdit::uniface_addevent(SubEvent sb, UICallBack&& call) noexcept {
    // 单行回车
    if (sb == SubEvent::Event_EditReturned) {
        m_text.AddReturnEventCall(std::move(call));
        return true;
    }
    // 文本改变
    else if (sb == SubEvent::Event_ValueChanged) {
        m_text.AddChangedEventCall(std::move(call));
        return true;
    }
    // 交给父类处理
    return Super::uniface_addevent(sb, std::move(call));
}


// UI基本编辑控件: 前景渲染
void LongUI::UIEdit::render_chain_foreground() const noexcept {
    // 文本算前景
    m_text.Render(0.f, 0.f);
    // 父类
    Super::render_chain_foreground();
}

// UI基本编辑控件: 渲染
void LongUI::UIEdit::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// UI基本编辑框: 刷新
void LongUI::UIEdit::Update() noexcept {
    // 改变了大小
    if (this->IsControlLayoutChanged()) {
        // 设置大小
        m_text.Resize(this->view_size.width, this->view_size.height);
        // 已经处理
        this->ControlLayoutChangeHandled();
    }
    // 刷新
    m_text.Update();
    return Super::Update();
}

// UI基本编辑控件
bool  LongUI::UIEdit::DoEvent(const LongUI::EventArgument& arg) noexcept {
    assert(arg.sender && "bad argument");
    // LongUI 消息
    switch (arg.event)
    {
    case LongUI::Event::Event_TreeBulidingFinished:
        __fallthrough;
    case LongUI::Event::Event_SubEvent:
        return true;
    case LongUI::Event::Event_SetFocus:
        // 设置焦点
        m_colorBorderNow = m_aBorderColor[LongUI::State_Pushed];
        m_text.OnSetFocus();
        return true;
    case LongUI::Event::Event_KillFocus:
        // 移除焦点
        m_colorBorderNow = m_aBorderColor[LongUI::State_Normal];
        m_text.OnKillFocus();
        return true;
    case LongUI::Event::Event_SetText:
        assert(!"NOIMPL");
        __fallthrough;
    case LongUI::Event::Event_GetText:
        arg.str = m_text.c_str();
        return true;
    case LongUI::Event::Event_Char:
        m_text.OnChar(arg.key.ch);
        return true;
    case LongUI::Event::Event_KeyDown:
        m_text.OnKey(static_cast<uint32_t>(arg.key.ch));
        return true;
    }
    return true;
}

// UI基本编辑控件: 鼠标事件
bool  LongUI::UIEdit::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(
        this->world, D2D1::Point2F(arg.ptx, arg.pty)
    );
    // LongUI 消息
    switch (arg.event)
    {
    case LongUI::MouseEvent::Event_DragEnter:
        assert(!"NOIMPL!");
        //m_text.OnDragEnter(arg.cf.dataobj, arg.cf.outeffect);
        break;
    case LongUI::MouseEvent::Event_DragOver:
        assert(!"NOIMPL!");
        //m_text.OnDragOver(pt4self.x, pt4self.y);
        break;
    case LongUI::MouseEvent::Event_DragLeave:
        assert(!"NOIMPL!");
        //m_text.OnDragLeave();
        break;
    case LongUI::MouseEvent::Event_Drop:
        assert(!"NOIMPL!");
        //m_text.OnDrop(arg.cf.dataobj, arg.cf.outeffect);
        break;
    case LongUI::MouseEvent::Event_MouseEnter:
        // 鼠标移进, 不是焦点控件则修改边框颜色
        if (!m_pWindow->IsFocused(this)) {
            m_colorBorderNow = m_aBorderColor[LongUI::State_Hover];
            this->InvalidateThis();
        }
        m_pWindow->SetCursor(Cursor::Cursor_Ibeam);
        break;
    case LongUI::MouseEvent::Event_MouseLeave:
        // 鼠标移出, 不是焦点控件则修改边框颜色
        m_pWindow->ResetCursor();
        if (!m_pWindow->IsFocused(this)) {
            m_colorBorderNow = m_aBorderColor[LongUI::State_Normal];
            this->InvalidateThis();
        }
        break;
    case LongUI::MouseEvent::Event_MouseMove:
        // 拖拽?
        if (UIInput.IsMbPressed(UIInput.MB_L)) {
            m_text.OnLButtonHold(pt4self.x, pt4self.y);
        }
        break;
    case LongUI::MouseEvent::Event_LButtonDown:
        m_text.OnLButtonDown(pt4self.x, pt4self.y, UIInput.IsKbPressed(UIInput.KB_SHIFT));
        break;
    case LongUI::MouseEvent::Event_LButtonUp:
        m_text.OnLButtonUp(pt4self.x, pt4self.y);
        break;
    case LongUI::MouseEvent::Event_RButtonUp:
        m_text.OnContextMenu();
        break;
    }
    return true;
}

// close this control 关闭控件
HRESULT LongUI::UIEdit::Recreate() noexcept {
    m_text.Recreate();
    return Super::Recreate();
}

// close this control 关闭控件
void LongUI::UIEdit::cleanup() noexcept {
    // 删除前调用
    this->before_deleted();
    // 删除对象
    delete this;
}

// 构造函数
void LongUI::UIEdit::initialize(pugi::xml_node node) noexcept {
    // 必须有效
    assert(node && "call UIEdit::initialize() if no xml-node");
    // 链式初始化
    Super::initialize(node);
    // 初始化文本
    m_text.Init(node);
    // 允许键盘焦点
    auto flag = this->flags | Flag_Focusable;
    // 初始化边框颜色
     Helper::SetBorderColor(node, m_aBorderColor);
    // 修改
    force_cast(this->flags) = flag;
    // 初始边框颜色
    m_colorBorderNow = m_aBorderColor[LongUI::State_Normal];
}

// UIEdit::CreateControl 函数
auto LongUI::UIEdit::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl*  {
    // 分类判断
    UIEdit* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIEdit, pControl, type, node);
    }
    return pControl;
}


#if 0

#if defined(_DEBUG)  && 1
#define TRACE_FUCTION UIManager << DL_Log << L"Trace: called" << LongUI::endl
#else
#define TRACE_FUCTION
#endif


// UI富文本编辑框: Render 渲染 
void LongUI::UIRichEdit::Render() const noexcept {
    /*HRESULT hr = S_OK;
    RECT draw_rect = { 0, 0, 100, 100 }; //AdjustRectT(LONG);
    if (m_pTextServices) {
        hr = m_pTextServices->TxDrawD2D(
            UIManager_RenderTarget,
            reinterpret_cast<RECTL*>(&draw_rect),
            nullptr,
            TXTVIEW_ACTIVE
            );
    }
    // 刻画光标
    if (SUCCEEDED(hr) && m_unused[Unused_ShowCaret]) {
        D2D1_RECT_F caretRect = {
            m_ptCaret.x, m_ptCaret.y,
            m_ptCaret.x + m_sizeCaret.width,m_ptCaret.y + m_sizeCaret.height
        };
        UIManager_RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
        UIManager_RenderTarget->FillRectangle(caretRect, m_pFontBrush);
        UIManager_RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    }
    return S_OK;*/
}

// UI富文本编辑框: Render 刷新
void LongUI::UIRichEdit::Update() noexcept {

}

// UIRichEdit 构造函数
inline LongUI::UIRichEdit::UIRichEdit(UIContainer* cp) noexcept: Super(cp){ }

// UIRichEdit 析构函数
LongUI::UIRichEdit::~UIRichEdit() noexcept {
    LongUI::SafeRelease(m_pFontBrush);
    if (m_pTextServices) {
        m_pTextServices->OnTxInPlaceDeactivate();
    }
    // 关闭服务
    UIRichEdit::ShutdownTextServices(m_pTextServices);
    //LongUI::SafeRelease(m_pTextServices);
}

// UIRichEdit::CreateControl 函数
LongUI::UIControl* LongUI::UIRichEdit::CreateControl(CreateEventType type, pugi::xml_node node) noexcept {
    // 分类判断
    UIRichEdit* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIRichEdit, pControl, type, node);
    }
    return pControl;
}



// do event 事件处理
bool LongUI::UIRichEdit::DoEvent(const LongUI::EventArgument& arg) noexcept {
    assert(arg.sender && "bad argument");
    switch (arg.event)
    {
    /*case LongUI::Event::Event_FindControl: // 查找本空间
        if (arg.event == LongUI::Event::Event_FindControl) {
            // 检查鼠标范围
            assert(arg.pt.x < this->width && arg.pt.y < this->width && "check it");
            arg.ctrl = this;
        }
        return true;*/
    /*case LongUI::Event::Event_MouseEnter:
        m_pWindow->now_cursor = m_hCursorI;
        break;
    case LongUI::Event::Event_MouseLeave:
        m_pWindow->now_cursor = m_pWindow->default_cursor;
        break;*/
    case LongUI::Event::Event_SetFocus:
        if (m_pTextServices) {
            m_pTextServices->OnTxUIActivate();
            m_pTextServices->TxSendMessage(WM_SETFOCUS, 0, 0, 0);
        }
        return true;
    case LongUI::Event::Event_KillFocus:
        if (m_pTextServices) {
            m_pTextServices->TxSendMessage(WM_KILLFOCUS, 0, 0, 0);
            m_pTextServices->OnTxUIDeactivate();
            this->TxShowCaret(FALSE);
        }
        return true;
    }
    return false;
}

// recreate 重建
HRESULT LongUI::UIRichEdit::Recreate() noexcept {
    HRESULT hr = S_OK;
    if (m_pTextServices) {
        m_pTextServices->OnTxInPlaceDeactivate();
    }
    LongUI::SafeRelease(m_pTextServices);
    LongUI::SafeRelease(m_pFontBrush);
    // 设置新的笔刷
    m_pFontBrush = UIManager.GetBrush(LongUIDefaultTextFormatIndex);
    // 获取窗口句柄
    m_hwnd = m_pWindow->GetHwnd();
    IUnknown* pUk = nullptr;
    hr = UIRichEdit::CreateTextServices(nullptr, this, &pUk);
    // 创建文本服务
    if (SUCCEEDED(hr)) {
        hr = pUk->QueryInterface(
            *UIRichEdit::IID_ITextServices2, reinterpret_cast<void**>(&m_pTextServices)
            );
    }
    if (SUCCEEDED(hr)) {
        hr = m_pTextServices->TxSetText(L"Hello, World!");
    }
    // 就地激活富文本控件
    if (SUCCEEDED(hr)) {
        hr = m_pTextServices->OnTxInPlaceActivate(nullptr);
    }
    LongUI::SafeRelease(pUk);
    return Super::Recreate();
}

// close this control 关闭控件
void LongUI::UIRichEdit::cleanup() noexcept {
    // 删除前调用
    this->before_deleted();
    // 删除对象
    delete this;
}

// ----- ITextHost
// ITextHost::TxGetDC 实现: 获取DC
HDC LongUI::UIRichEdit::TxGetDC() {
    // 不支持!!
    assert(!"- GDI MODE - Not Supported");
    TRACE_FUCTION;
    return nullptr;
}

// ITextHost::TxReleaseDC 实现: 释放DC
INT LongUI::UIRichEdit::TxReleaseDC(HDC hdc){
    UNREFERENCED_PARAMETER(hdc);
    assert(!"- GDI MODE - Not Supported");
    TRACE_FUCTION;
    return 0;
}

// ITextHost::TxShowScrollBar 实现: 显示滚动条
BOOL LongUI::UIRichEdit::TxShowScrollBar(INT fnBar, BOOL fShow){
    UNREFERENCED_PARAMETER(fnBar);
    UNREFERENCED_PARAMETER(fShow);
    TRACE_FUCTION;
    return FALSE;
}

BOOL LongUI::UIRichEdit::TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags){
    UNREFERENCED_PARAMETER(fuSBFlags);
    UNREFERENCED_PARAMETER(fuArrowflags);
    TRACE_FUCTION;
    return FALSE;
}

BOOL LongUI::UIRichEdit::TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw){
    UNREFERENCED_PARAMETER(fnBar);
    UNREFERENCED_PARAMETER(nMinPos);
    UNREFERENCED_PARAMETER(nMaxPos);
    UNREFERENCED_PARAMETER(fRedraw);
    TRACE_FUCTION;
    return FALSE;
}

BOOL LongUI::UIRichEdit::TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw){
    UNREFERENCED_PARAMETER(fnBar);
    UNREFERENCED_PARAMETER(nPos);
    UNREFERENCED_PARAMETER(fRedraw);
    TRACE_FUCTION;
    return FALSE;
}

void LongUI::UIRichEdit::TxInvalidateRect(LPCRECT prc, BOOL fMode){
    UNREFERENCED_PARAMETER(prc);
    UNREFERENCED_PARAMETER(fMode);
    TRACE_FUCTION;
}

void LongUI::UIRichEdit::TxViewChange(BOOL fUpdate){
    UNREFERENCED_PARAMETER(fUpdate);
    TRACE_FUCTION;
    //if (fUpdate) {
        this->InvalidateThis();
    //}
}

// ITextHost::TxCreateCaret 实现:创建光标
BOOL LongUI::UIRichEdit::TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight){
    UNREFERENCED_PARAMETER(hbmp);
    TRACE_FUCTION;
    m_sizeCaret = {static_cast<float>(xWidth), static_cast<float>(yHeight) };
    // 创建傀儡
    ::DestroyCaret();
    ::CreateCaret(m_hwnd, nullptr, xWidth, yHeight);
    return TRUE;
}

// ITextHost::TxShowCaret 实现:显示/隐藏光标
BOOL LongUI::UIRichEdit::TxShowCaret(BOOL fShow){
    UNREFERENCED_PARAMETER(fShow);
    TRACE_FUCTION;
    //m_unused[Unused_ShowCaret] = (fShow != 0);
    return TRUE;
}

BOOL LongUI::UIRichEdit::TxSetCaretPos(INT _x, INT _y){
    TRACE_FUCTION;
    m_ptCaret = {static_cast<float>(_x), static_cast<float>(_y) };
    this->InvalidateThis();
    ::SetCaretPos(_x, _y);
    return TRUE;
}

BOOL LongUI::UIRichEdit::TxSetTimer(UINT idTimer, UINT uTimeout){
    ::SetTimer(m_hwnd, idTimer, uTimeout, nullptr);
    TRACE_FUCTION;
    return FALSE;
}

void LongUI::UIRichEdit::TxKillTimer(UINT idTimer){
    ::KillTimer(m_hwnd, idTimer);
    TRACE_FUCTION;
}

void LongUI::UIRichEdit::TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll,
    LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll){
    UNREFERENCED_PARAMETER(dx);
    UNREFERENCED_PARAMETER(dy);
    UNREFERENCED_PARAMETER(lprcScroll);
    UNREFERENCED_PARAMETER(lprcClip);
    UNREFERENCED_PARAMETER(hrgnUpdate);
    UNREFERENCED_PARAMETER(lprcUpdate);
    UNREFERENCED_PARAMETER(fuScroll);
    TRACE_FUCTION;
}

void LongUI::UIRichEdit::TxSetCapture(BOOL fCapture) {
    TRACE_FUCTION;
    fCapture ? void(::SetCapture(m_hwnd)) : void(::ReleaseCapture());
}

void LongUI::UIRichEdit::TxSetFocus(){
    TRACE_FUCTION;
}

void LongUI::UIRichEdit::TxSetCursor(HCURSOR hcur, BOOL fText){
    UNREFERENCED_PARAMETER(hcur);
    UNREFERENCED_PARAMETER(fText);
    TRACE_FUCTION;
}

BOOL LongUI::UIRichEdit::TxScreenToClient(LPPOINT lppt){
    UNREFERENCED_PARAMETER(lppt);
    TRACE_FUCTION;
    return FALSE;
}

BOOL LongUI::UIRichEdit::TxClientToScreen(LPPOINT lppt){
    UNREFERENCED_PARAMETER(lppt);
    TRACE_FUCTION;
    return FALSE;
}

HRESULT LongUI::UIRichEdit::TxActivate(LONG* plOldState){
    UNREFERENCED_PARAMETER(plOldState);
    TRACE_FUCTION;
    return E_FAIL;
}

HRESULT LongUI::UIRichEdit::TxDeactivate(LONG lNewState){
    UNREFERENCED_PARAMETER(lNewState);
    TRACE_FUCTION;
    return E_FAIL;
}

HRESULT LongUI::UIRichEdit::TxGetClientRect(LPRECT prc){
    //TRACE_FUCTION;
    *prc = { 0, 0, 100, 100 };
    return S_OK;
}

HRESULT LongUI::UIRichEdit::TxGetViewInset(LPRECT prc){
    // Set zero sized margins
    *prc = { 0, 0, 0, 0 };
    TRACE_FUCTION;
    return S_OK;
}

HRESULT LongUI::UIRichEdit::TxGetCharFormat(const CHARFORMATW **ppCF){
    UNREFERENCED_PARAMETER(ppCF);
    /*METHOD_PROLOGUE(CRichDrawText, TextHost)

        // Return the default character format set up in the constructor
        *ppCF = &(pThis->m_CharFormat);*/
    TRACE_FUCTION;
    return S_OK;
}

HRESULT LongUI::UIRichEdit::TxGetParaFormat(const PARAFORMAT **ppPF){
    UNREFERENCED_PARAMETER(ppPF);
    /* METHOD_PROLOGUE(CRichDrawText, TextHost)

        // Return the default paragraph format set up in the constructor
        *ppPF = &(pThis->m_ParaFormat);*/
    TRACE_FUCTION;
    return S_OK;
}

COLORREF LongUI::UIRichEdit::TxGetSysColor(int nIndex){
    // Pass requests for colours on to Windows
    TRACE_FUCTION;
    return ::GetSysColor(nIndex);
}

HRESULT LongUI::UIRichEdit::TxGetBackStyle(TXTBACKSTYLE *pstyle){
    // Do not erase what is underneath the drawing area
    *pstyle = TXTBACK_TRANSPARENT;
    TRACE_FUCTION;
    return S_OK;
}

HRESULT LongUI::UIRichEdit::TxGetMaxLength(DWORD *plength){
    // Set the maximum size of text to be arbitrarily large
    *plength = 1024 * 1024 * 16;
    TRACE_FUCTION;
    return S_OK;
}

HRESULT LongUI::UIRichEdit::TxGetScrollBars(DWORD *pdwScrollBar){
    *pdwScrollBar = 0;
    TRACE_FUCTION;
    return S_OK;
}

HRESULT LongUI::UIRichEdit::TxGetPasswordChar(TCHAR *pch) {
    *pch = L'*';
    TRACE_FUCTION;
    return S_FALSE;
}

HRESULT LongUI::UIRichEdit::TxGetAcceleratorPos(LONG *pcp){
    *pcp = -1;
    TRACE_FUCTION;
    return S_OK;
}

HRESULT LongUI::UIRichEdit::TxGetExtent(LPSIZEL lpExtent){
    UNREFERENCED_PARAMETER(lpExtent);
    TRACE_FUCTION;
    return E_NOTIMPL;
}

HRESULT LongUI::UIRichEdit::OnTxCharFormatChange(const CHARFORMATW * pcf){
    UNREFERENCED_PARAMETER(pcf);
    TRACE_FUCTION;
    return E_FAIL;
}

HRESULT LongUI::UIRichEdit::OnTxParaFormatChange(const PARAFORMAT * ppf){
    UNREFERENCED_PARAMETER(ppf);
    TRACE_FUCTION;
    return E_FAIL;
}

HRESULT LongUI::UIRichEdit::TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits){
    // Set the windowless control as being multiple lines of wrapping rich text
    DWORD bits = TXTBIT_MULTILINE | TXTBIT_RICHTEXT | TXTBIT_WORDWRAP | TXTBIT_D2DDWRITE;
    *pdwBits = bits & dwMask;
    TRACE_FUCTION;
    return S_OK;
}

HRESULT LongUI::UIRichEdit::TxNotify(DWORD iNotify, void*pv){
    // Claim to have handled the notifcation, even though we always ignore it
    UNREFERENCED_PARAMETER(iNotify);
    UNREFERENCED_PARAMETER(pv);
    TRACE_FUCTION;
    return S_OK;
}

HIMC LongUI::UIRichEdit::TxImmGetContext(){
    TRACE_FUCTION;
    return 0;
}

void LongUI::UIRichEdit::TxImmReleaseContext(HIMC himc){
    UNREFERENCED_PARAMETER(himc);
    TRACE_FUCTION;
}

HRESULT LongUI::UIRichEdit::TxGetSelectionBarWidth(LONG *lSelBarWidth){
    UNREFERENCED_PARAMETER(lSelBarWidth);
    // No selection bar
    *lSelBarWidth = 0;
    TRACE_FUCTION;
    return S_OK;
}

// ----- ITextHost2
/*#undef TRACE_FUCTION
#define TRACE_FUCTION (L"Trace:<%S> called\n", __FUNCTION__)*/

//@cmember Is a double click in the message queue?
BOOL    LongUI::UIRichEdit::TxIsDoubleClickPending() {
    TRACE_FUCTION;
    return FALSE;
}

//@cmember Get the overall window for this control     
HRESULT LongUI::UIRichEdit::TxGetWindow(HWND *phwnd) {
    *phwnd = m_hwnd;
    TRACE_FUCTION;
    return S_OK;
}

//@cmember Set controlwindow to foreground
HRESULT LongUI::UIRichEdit::TxSetForegroundWindow() {
    TRACE_FUCTION;
    return S_FALSE;
}

//@cmember Set control window to foreground
HPALETTE LongUI::UIRichEdit::TxGetPalette() {
    TRACE_FUCTION;
    return nullptr;
}

//@cmember Get East Asian flags
HRESULT LongUI::UIRichEdit::TxGetEastAsianFlags(LONG *pFlags) {
    *pFlags = ES_NOIME;
    TRACE_FUCTION;
    return S_FALSE;
}

//@cmember Routes the cursor change to the winhost
HCURSOR LongUI::UIRichEdit::TxSetCursor2(HCURSOR hcur, BOOL bText) {
    UNREFERENCED_PARAMETER(hcur);
    UNREFERENCED_PARAMETER(bText);
    TRACE_FUCTION;
    return hcur;
}

//@cmember Notification that text services is freed
void    LongUI::UIRichEdit::TxFreeTextServicesNotification() {

    TRACE_FUCTION;
}

//@cmember Get Edit Style flags
HRESULT LongUI::UIRichEdit::TxGetEditStyle(DWORD dwItem, DWORD *pdwData) {
    UNREFERENCED_PARAMETER(dwItem);
    UNREFERENCED_PARAMETER(pdwData);
    TRACE_FUCTION;
    return FALSE;
}

//@cmember Get Window Style bits
HRESULT LongUI::UIRichEdit::TxGetWindowStyles(DWORD *pdwStyle, DWORD *pdwExStyle) {
    *pdwStyle = WS_OVERLAPPEDWINDOW;
    *pdwExStyle = 0;
    TRACE_FUCTION;
    return S_OK;
}

//@cmember Show / hide drop caret (D2D-only)
HRESULT LongUI::UIRichEdit::TxShowDropCaret(BOOL fShow, HDC hdc, LPCRECT prc) {
    UNREFERENCED_PARAMETER(fShow);
    UNREFERENCED_PARAMETER(hdc);
    UNREFERENCED_PARAMETER(prc);
    TRACE_FUCTION;
    return S_FALSE;
}

//@cmember Destroy caret (D2D-only)
HRESULT LongUI::UIRichEdit::TxDestroyCaret() {
    TRACE_FUCTION;
    return S_FALSE;
}

//@cmember Get Horizontal scroll extent
HRESULT LongUI::UIRichEdit::TxGetHorzExtent(LONG *plHorzExtent) {
    UNREFERENCED_PARAMETER(plHorzExtent);
    TRACE_FUCTION;
    return S_FALSE;
}

/*
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetScrollBars> called
Trace:<TxGetScrollBars> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetScrollBars> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxShowCaret> called
Trace:<TxCreateCaret> called
Trace:<TxSetCaretPos> called
Trace:<TxShowCaret> called
Trace:<TxInvalidateRect> called
Trace:<TxInvalidateRect> called
Trace:<TxViewChange> called
Trace:<TxShowCaret> called
Trace:<TxGetScrollBars> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxShowCaret> called
Trace:<TxCreateCaret> called
Trace:<TxSetCaretPos> called
Trace:<TxShowCaret> called
Trace:<TxGetWindow> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxNotify> called
Trace:<TxGetSysColor> called
Trace:<TxGetSysColor> called
Trace:<TxDestroyCaret> called
Trace:<TxNotify> called
Trace:<TxShowCaret> called
*/
#endif