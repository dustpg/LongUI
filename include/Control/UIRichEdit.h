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


// LongUI namespace
namespace LongUI {
    // default rich edit 默认富文本编辑控件
    class  UIRichEdit :  public UIControl, public Helper::ComStatic<Helper::QiList<ITextHost2>> {
        // 父类申明
        using Super = UIControl;
    public:
        // Text Services 2 IID指针
        static IID*                                IID_ITextServices2;
        // 创建文本服务-函数指针
        static decltype(&::CreateTextServices)     CreateTextServices;
        // 关闭文本服务-函数指针
        static decltype(&::ShutdownTextServices)   ShutdownTextServices;
    public:
        // Render 渲染 
        virtual void Render(RenderType type) const noexcept override;
        // udate 刷新
        virtual void Update() noexcept override;
        // do event 事件处理
        virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
        // recreate 重建
        virtual auto Recreate() noexcept->HRESULT override;
    private:
        // close this control 关闭控件
        virtual void cleanup() noexcept override;
    public:
        // operator ITextServices2
        operator ITextServices2*() const noexcept { return m_pTextServices; }
        // create 创建
        static UIControl* WINAPI CreateControl(CreateEventType, pugi::xml_node) noexcept;
        // constructor 构造函数
        UIRichEdit(UIContainer* cp, pugi::xml_node) noexcept;
    protected:
        // destructor 析构函数
        ~UIRichEdit() noexcept;
        // deleted function
        UIRichEdit(const UIRichEdit&) = delete;
    public: // ITextHost 实现
        //@cmember Get the DC for the host
        virtual HDC         TxGetDC() override;
        //@cmember Release the DC gotten from the host
        virtual INT         TxReleaseDC(HDC hdc) override;
        //@cmember Show the scroll bar
        virtual BOOL        TxShowScrollBar(INT fnBar, BOOL fShow) override;
        //@cmember Enable the scroll bar
        virtual BOOL        TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags) override;
        //@cmember Set the scroll range
        virtual BOOL        TxSetScrollRange(
            INT fnBar,
            LONG nMinPos,
            INT nMaxPos,
            BOOL fRedraw) override;
        //@cmember Set the scroll position
        virtual BOOL        TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw) override;
        //@cmember InvalidateRect
        virtual void        TxInvalidateRect(LPCRECT prc, BOOL fMode) override;
        //@cmember Send a WM_PAINT to the window
        virtual void        TxViewChange(BOOL fUpdate) override;
        //@cmember Create the caret
        virtual BOOL        TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight) override;
        //@cmember Show the caret
        virtual BOOL        TxShowCaret(BOOL fShow) override;
        //@cmember Set the caret position
        virtual BOOL        TxSetCaretPos(INT _x, INT _y) override;
        //@cmember Create a timer with the specified timeout
        virtual BOOL        TxSetTimer(UINT idTimer, UINT uTimeout) override;
        //@cmember Destroy a timer
        virtual void        TxKillTimer(UINT idTimer) override;
        //@cmember Scroll the content of the specified window's client area
        virtual void        TxScrollWindowEx(
            INT dx,
            INT dy,
            LPCRECT lprcScroll,
            LPCRECT lprcClip,
            HRGN hrgnUpdate,
            LPRECT lprcUpdate,
            UINT fuScroll) override;
        //@cmember Get mouse capture
        virtual void        TxSetCapture(BOOL fCapture) override;
        //@cmember Set the focus to the text window
        virtual void        TxSetFocus() override;
        //@cmember Establish a new cursor shape
        virtual void        TxSetCursor(HCURSOR hcur, BOOL fText) override;
        //@cmember Converts screen coordinates of a specified point to the client coordinates 
        virtual BOOL        TxScreenToClient(LPPOINT lppt) override;
        //@cmember Converts the client coordinates of a specified point to screen coordinates
        virtual BOOL        TxClientToScreen(LPPOINT lppt) override;
        //@cmember Request host to activate text services
        virtual HRESULT     TxActivate(LONG * plOldState) override;
        //@cmember Request host to deactivate text services
        virtual HRESULT     TxDeactivate(LONG lNewState) override;
        //@cmember Retrieves the coordinates of a window's client area
        virtual HRESULT     TxGetClientRect(LPRECT prc) override;
        //@cmember Get the view rectangle relative to the inset
        virtual HRESULT     TxGetViewInset(LPRECT prc) override;
        //@cmember Get the default character format for the text
        virtual HRESULT     TxGetCharFormat(const CHARFORMATW **ppCF) override;
        //@cmember Get the default paragraph format for the text
        virtual HRESULT     TxGetParaFormat(const PARAFORMAT **ppPF) override;
        //@cmember Get the background color for the window
        virtual COLORREF    TxGetSysColor(int nIndex) override;
        //@cmember Get the background (either opaque or transparent)
        virtual HRESULT     TxGetBackStyle(TXTBACKSTYLE *pstyle) override;
        //@cmember Get the maximum length for the text
        virtual HRESULT     TxGetMaxLength(DWORD *plength) override;
        //@cmember Get the bits representing requested scroll bars for the window
        virtual HRESULT     TxGetScrollBars(DWORD *pdwScrollBar) override;
        //@cmember Get the character to display for password input
        virtual HRESULT     TxGetPasswordChar(_Out_ TCHAR *pch) override;
        //@cmember Get the accelerator character
        virtual HRESULT     TxGetAcceleratorPos(LONG *pcp) override;
        //@cmember Get the native size
        virtual HRESULT     TxGetExtent(LPSIZEL lpExtent) override;
        //@cmember Notify host that default character format has changed
        virtual HRESULT     OnTxCharFormatChange(const CHARFORMATW * pCF) override;
        //@cmember Notify host that default paragraph format has changed
        virtual HRESULT     OnTxParaFormatChange(const PARAFORMAT * pPF) override;
        //@cmember Bulk access to bit properties
        virtual HRESULT     TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits) override;
        //@cmember Notify host of events
        virtual HRESULT     TxNotify(DWORD iNotify, void *pv) override;
        // East Asia Methods for getting the Input Context
        virtual HIMC        TxImmGetContext() override;
        // East Asia Methods for release the Input Context
        virtual void        TxImmReleaseContext(HIMC himc) override;
        //@cmember Returns HIMETRIC size of the control bar.
        virtual HRESULT     TxGetSelectionBarWidth(LONG *lSelBarWidth) override;
    public: // ITextHost2 实现
        //@cmember Is a double click in the message queue?
        virtual BOOL        TxIsDoubleClickPending() override;
        //@cmember Get the overall window for this control     
        virtual HRESULT     TxGetWindow(HWND *phwnd) override;
        //@cmember Set controlwindow to foreground
        virtual HRESULT     TxSetForegroundWindow() override;
        //@cmember Set control window to foreground
        virtual HPALETTE    TxGetPalette() override;
        //@cmember Get East Asian flags
        virtual HRESULT     TxGetEastAsianFlags(LONG *pFlags) override;
        //@cmember Routes the cursor change to the winhost
        virtual HCURSOR     TxSetCursor2(HCURSOR hcur, BOOL bText) override;
        //@cmember Notification that text services is freed
        virtual void        TxFreeTextServicesNotification() override;
        //@cmember Get Edit Style flags
        virtual HRESULT     TxGetEditStyle(DWORD dwItem, DWORD *pdwData) override;
        //@cmember Get Window Style bits
        virtual HRESULT     TxGetWindowStyles(DWORD *pdwStyle, DWORD *pdwExStyle) override;
        //@cmember Show / hide drop caret (D2D-only)
        virtual HRESULT     TxShowDropCaret(BOOL fShow, HDC hdc, LPCRECT prc) override;
        //@cmember Destroy caret (D2D-only)
        virtual HRESULT     TxDestroyCaret() override;
        //@cmember Get Horizontal scroll extent
        virtual HRESULT     TxGetHorzExtent(LONG *plHorzExtent) override;
    protected:
        // window handle
        HWND                m_hwnd = nullptr;
        // font brush
        ID2D1Brush*         m_pFontBrush = nullptr;
        // Text Services for Direct2D
        ITextServices2*     m_pTextServices = nullptr;
        // 'I' cursor
        HCURSOR             m_hCursorI = ::LoadCursor(nullptr, IDC_IBEAM);
        // Caret Size
        D2D1_SIZE_F         m_sizeCaret = D2D1::SizeF();
        // Caret Position
        D2D1_POINT_2F       m_ptCaret = D2D1::Point2F();
    };
}