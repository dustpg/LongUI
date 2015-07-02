
#include "LongUI.h"

// node->Attribute\((.+?)\)
// node.attribute($1).value()

#define LONGUI_D3D_DEBUG

// CUIManager 初始化
auto LongUI::CUIManager::Initialize(IUIConfigure* config) noexcept->HRESULT {
    if (!config) {
#ifdef LONGUI_WITH_DEFAULT_CONFIG
        config = &m_config;
#else
        return E_INVALIDARG;
#endif
    }
    
    // 解析资源脚本
    auto res_xml = config->GetResourceXML();
    if (res_xml) {
        auto re = m_docResource.load_string(res_xml);
        if (re.status) {
            assert(!"failed");
            ::MessageBoxA(nullptr, re.description(), "<LongUI::CUIManager::Initialize>: Failed to Parse XML", MB_ICONERROR);
        }
    }
    // 获取操作系统信息
#if 0
    if (IsWindows10OrGreater()) {
        force_cast(this->version) = WindowsVersion::Style_Win10;
    }
    /*else*/
#endif
    if (IsWindows8Point1OrGreater()) {
        force_cast(this->version) = WindowsVersion::Style_Win8_1;
    }
    // 获取信息
    force_cast(this->configure) = config;
    force_cast(this->script) = config->GetScript();
    force_cast(this->inline_handler) = config->GetInlineParamHandler();
    *m_szLocaleName = 0;
    config->GetLocaleName(m_szLocaleName);
    // 初始化其他
    ZeroMemory(m_apTextRenderer, sizeof(m_apTextRenderer));
    ZeroMemory(m_apSystemBrushes, sizeof(m_apSystemBrushes));
    // 添加默认创建函数
    this->AddS2CPair(L"Label", LongUI::UILabel::CreateControl);
    this->AddS2CPair(L"Button", LongUI::UIButton::CreateControl);
    this->AddS2CPair(L"VerticalLayout", LongUI::UIVerticalLayout::CreateControl);
    this->AddS2CPair(L"HorizontalLayout", LongUI::UIHorizontalLayout::CreateControl);
    this->AddS2CPair(L"Slider", LongUI::UISlider::CreateControl);
    this->AddS2CPair(L"CheckBox", LongUI::UICheckBox::CreateControl);
    this->AddS2CPair(L"RichEdit", LongUI::UIRichEdit::CreateControl);
    this->AddS2CPair(L"ScrollBarA", LongUI::UIScrollBarA::CreateControl);
    ///
    this->AddS2CPair(L"EditBasic", LongUI::UIEditBasic::CreateControl);
    this->AddS2CPair(L"Edit", LongUI::UIEditBasic::CreateControl);
    ///
    // 添加自定义控件
    config->AddCustomControl(*this);
    // 获取实例句柄
    auto hInstance = ::GetModuleHandleW(nullptr);
    // 注册窗口类 | CS_DBLCLKS
    WNDCLASSEX wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW ;
    wcex.lpfnWndProc = CUIManager::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(void*);
    wcex.hInstance = hInstance;
    wcex.hCursor = nullptr;
    wcex.hbrBackground = nullptr;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"LongUIWindow"; 
    auto hicon = LoadIconW(hInstance, MAKEINTRESOURCEW(1));
    wcex.hIcon = hicon;
    // 注册窗口
    ::RegisterClassExW(&wcex);
    m_pBitmap0Buffer = reinterpret_cast<uint8_t*>(malloc(
        sizeof(RGBQUAD)* LongUIDefaultBitmapSize * LongUIDefaultBitmapSize)
        );
    // 重建资源
    register HRESULT hr = m_pBitmap0Buffer ? S_OK : E_OUTOFMEMORY;
    // 创建DirectInput对象
    if (SUCCEEDED(hr)) {
        hr = ::DirectInput8Create(
            hInstance, 
            DIRECTINPUT_VERSION,
            LongUI_IID_PV_ARGS(m_pDirectInput),
            0
            );
    }
    // 创建鼠标设备
    if (SUCCEEDED(hr)) {
        hr = m_pDirectInput->CreateDevice(GUID_SysMouse, &m_pDInputMouse, 0);
    }
    // 设置数据格式 :鼠标
    if SUCCEEDED(hr) {
        hr = m_pDInputMouse->SetDataFormat(&c_dfDIMouse);
    }
    // 设置协作等级 不独占
    if SUCCEEDED(hr) {
        hr = m_pDInputMouse->SetCooperativeLevel(nullptr, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
    }
    // 获得鼠标输入设备 通知操作系统已经准备完毕
    if SUCCEEDED(hr) {
        hr = m_pDInputMouse->Acquire();
    }
    // 创建D2D工厂
    if (SUCCEEDED(hr)) {
        D2D1_FACTORY_OPTIONS options = { D2D1_DEBUG_LEVEL_NONE };
#ifdef _DEBUG
        options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
        hr = LongUI::Dll::D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            IID_ID2D1Factory1,
            &options,
            reinterpret_cast<void**>(&m_pd2dFactory)
            );
    }
    // 创建 WIC 工厂.
    if (SUCCEEDED(hr)) {
        hr = ::CoCreateInstance(
            CLSID_WICImagingFactory2,
            nullptr,
            CLSCTX_INPROC_SERVER,
            LongUI_IID_PV_ARGS(m_pWICFactory)
            );
    }
    // 创建 DirectWrite 工厂.
    IDWriteFactory1;
    if (SUCCEEDED(hr)) {
        hr = LongUI::Dll::DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            LongUI_IID_PV_ARGS_Ex(m_pDWriteFactory)
            );
    }
    // 创建字体集
    if (SUCCEEDED(hr)) {
        m_pFontCollection = config->CreateFontCollection(*this);
        // 失败获取系统字体集
        if (!m_pFontCollection) {
            hr = m_pDWriteFactory->GetSystemFontCollection(&m_pFontCollection);
        }
    }
    // 准备缓冲区
    if (SUCCEEDED(hr)) {
        try {
            m_textFormats.reserve(64);
            m_brushes.reserve(64);
            m_windows.reserve(LongUIMaxWindow);
            m_bitmaps.reserve(64);
            m_metas.reserve(64);
            m_metaicons.reserve(64);
        }
        CATCH_HRESULT(hr)
    }
    // 注册渲染器
    if (SUCCEEDED(hr)) {
        // 普通渲染器
        if (this->RegisterTextRenderer(&m_normalTRenderer) != Type_NormalTextRenderer) {
            hr = E_FAIL;
        }
    }
    // 初始化脚本
    if (this->script && !(this->script->Initialize(this))) {
        hr = E_FAIL;
    }
    // 创建资源
    if (SUCCEEDED(hr)) {
        hr = this->RecreateResources();
    }
    return hr;
}

// CUIManager  反初始化
void LongUI::CUIManager::UnInitialize() noexcept {
    // 放弃设备
    if (m_pDInputMouse) {
        m_pDInputMouse->Unacquire();
        m_pDInputMouse->Release();
        m_pDInputMouse = nullptr;
    }
    SafeRelease(m_pDirectInput);
    // 释放读取器
    ::SafeRelease(m_pResourceLoader);
    // 释放文本渲染器
    for (auto& renderer : m_apTextRenderer) {
        ::SafeRelease(renderer);
    }
    // 释放系统笔刷
    for (auto& brush : m_apSystemBrushes) {
        ::SafeRelease(brush);
    }
    // 释放资源
    this->discard_resources();
    ::SafeRelease(m_pFontCollection);
    ::SafeRelease(m_pDWriteFactory);
    ::SafeRelease(m_pWICFactory);
    ::SafeRelease(m_pd2dFactory);
    // 释放内存
    if (m_pBitmap0Buffer) {
        free(m_pBitmap0Buffer);
        m_pBitmap0Buffer = nullptr;
    }
    // 反初始化脚本
    if (this->script) {
        this->script->UnInitialize();
        this->script->Release();
        force_cast(script) = nullptr;
    }
    // 释放配置
    ::SafeRelease(force_cast(this->configure));
}


// 创建控件
inline auto LongUI::CUIManager::create_control(pugi::xml_node node) noexcept -> UIControl* {
    assert(node && "bad argument");
    // 获取创建指针
    auto create = this->GetCreateFunc(node.name());
    if (create) return create(node);
    return nullptr;
}

// CUIManager 创建控件树
// 默认消耗 64kb+, 导致栈(默认1~2M)溢出几率较低
void LongUI::CUIManager::make_control_tree(LongUI::UIWindow* window, pugi::xml_node node) noexcept {
    // 断言
    assert(window && node && "bad argument");
    // 添加窗口
    //add_control(window, node);
    // 队列 -- 顺序遍历树
    LongUI::FixedCirQueue<pugi::xml_node, LongUIMaxControlInited> xml_queue;
    LongUI::FixedCirQueue<UIContainer*, LongUIMaxControlInited> parents_queue;
    // 
    UIControl* now_control = nullptr;
    UIContainer* parent_node = window;
    // 唯一名称
    std::pair<CUIString, void*> control_name;
    // 遍历算法: 1.压入所有子节点 2.依次弹出 3.重复1
    while (true) {
        // 压入/入队 所有子节点
        node = node.first_child();
        while (node) {
            xml_queue.push(node);
            parents_queue.push(parent_node);
            node = node.next_sibling();
        }
    recheck:
        // 为空则退出
        if (xml_queue.empty()) break;
        // 弹出/出队 第一个节点
        node = *xml_queue.front;  xml_queue.pop();
        parent_node = *parents_queue.front; parents_queue.pop();
        // 根据名称创建控件
        if (!(now_control = this->create_control(node))) {
            parent_node = nullptr;
#ifdef _DEBUG
            const char* node_name = node.name();
            UIManager << DL_Error << L" Control Class Not Found: " << node_name << LongUI::endl;
#endif
            continue;
        }
        // 添加到表
        if (UIControl::MakeString(node.attribute("name").value(), control_name.first)) {
            control_name.second = now_control;
            window->AddControl(control_name);
        }
        // 添加子节点
        parent_node->insert(parent_node->end(), now_control);
        // 设置节点为下次父节点
        parent_node = static_cast<decltype(parent_node)>(now_control);
        // 检查本控件是否需要XML子节点信息
        if (now_control->flags & Flag_ControlNeedFullXMLNode) {
            goto recheck;
        }
    }
}

// 获取创建控件函数指针
auto LongUI::CUIManager::GetCreateFunc(const char* class_name) noexcept -> CreateControlFunction {
    // 缓冲区
    wchar_t buffer[LongUIStringBufferLength];
    auto* __restrict itra = class_name;
    auto* __restrict itrb = buffer;
    // 类名一定是英文的
    for (; *itra; ++itra, ++itrb) {
        assert(*itra >= 0 && "bad name");
        *itrb = *itra;
    }
    *itrb = L'\0';
    // 获取
    return this->GetCreateFunc(buffer, itra - class_name);
}

// 获取创建控件函数指针
auto LongUI::CUIManager::GetCreateFunc(const CUIString& name) noexcept -> CreateControlFunction {
    // 查找
    try {
        const auto itr = m_mapString2CreateFunction.find(name);
        if (itr != m_mapString2CreateFunction.end()) {
            return reinterpret_cast<CreateControlFunction>(itr->second);
        }
    }
    catch (...)  {

    }
    return nullptr;
}

// 渲染线程
void LongUI::CUIManager::rendering_thread() {
    while (!m_exitFlag) {
        UIWindow* windows[LongUIMaxWindow];
        uint32_t length = 0;
        // 复制
        UIManager.Lock();
        length = m_windows.size();
        for (auto i = 0u; i < length; ++i) {
            windows[i] = reinterpret_cast<UIWindow*>(m_windows[i]);
        }
        UIManager.Unlock();
        ::Sleep(16);
    }
}

// 消息循环
void LongUI::CUIManager::Run() noexcept {
    MSG msg;
    // 创建线程
    std::thread thread;
    try {
        thread = std::move(std::thread(&CUIManager::rendering_thread, this));
    }
    catch (...) {
        this->ShowError(L"Failed to Create Thread");
        return;
    }
    //auto now_time = ::timeGetTime();
    while (!m_exitFlag) {
        // 获取鼠标状态
        m_lastMouseStates = this->now_mouse_states;
        if (m_pDInputMouse->GetDeviceState(sizeof(DIMOUSESTATE), &force_cast(this->now_mouse_states))
            == DIERR_INPUTLOST){
            m_pDInputMouse->Acquire();
        }
        // 消息循环
        if (::PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            // 两种方式退出 ::PostQuitMessage(0) or UIManager.Exit()
            if (msg.message == WM_QUIT) {
                m_exitFlag = true;
                break;
            }
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
        // 设置为渲染
        else {
            msg.message = WM_PAINT;
        }
        // 渲染
        if(msg.message == WM_PAINT){
            // 有窗口就渲染
            UIWindow* windows[LongUIMaxWindow];
            UIWindow** window_end = windows;
            // 检查窗口
            for (auto itr = m_windows.begin(); itr != m_windows.end(); ++itr) {
                register auto wnd = reinterpret_cast<UIWindow*>(*itr);
                if (wnd->UpdateRendering()) {
                    *window_end = wnd;
                    ++window_end;
                }
            }
            // 渲染窗口
            if (window_end != windows){
                for (auto itr = windows; itr < window_end; ++itr) {
                    (*itr)->Update();
                    (*itr)->BeginDraw();
                    (*itr)->Render(RenderType::Type_Render);
                    (*itr)->EndDraw(itr == window_end - 1);
                }
            }
            else {
                //std::this_thread::sleep_for
                ::Sleep(1);
                // 交出时间片
                //::Sleep(0);
            }
        }
    }
    m_exitFlag = true;
    // 等待线程
    try {
        if (thread.joinable()) {
            thread.join();
        }
    }
    catch (...) {

    }
    // 尝试强行关闭(使用迭代器会使迭代器失效)
    while (!m_windows.empty()) {
        reinterpret_cast<UIWindow*>(m_windows.back())->Close();
    }
}

// 窗口过程函数
LRESULT LongUI::CUIManager::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept {
    // 填写参数
    LongUI::EventArgument arg;  arg.msg = message;  arg.sender = nullptr;
    POINT pt; ::GetCursorPos(&pt); ::ScreenToClient(hwnd, &pt);
    arg.pt.x = static_cast<float>(pt.x); arg.pt.y = static_cast<float>(pt.y);
    arg.wParam_sys = wParam; arg.lParam_sys = lParam;
    // 返回
    arg.lr = 0;
    // 创建窗口时设置指针
    if (message == WM_CREATE)    {
        // 获取指针
        LongUI::UIWindow *pUIWindow = reinterpret_cast<LongUI::UIWindow*>(
            (reinterpret_cast<LPCREATESTRUCT>(lParam))->lpCreateParams
            );
        // 设置窗口指针
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, PtrToUlong(pUIWindow));
        // 返回1
        arg.lr = 1;
    }
    else {
        // 获取储存的指针
        LongUI::UIWindow *pUIWindow = reinterpret_cast<LongUI::UIWindow *>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(hwnd, GWLP_USERDATA))
            );
        // 检查是否处理了
        bool wasHandled = false;
        //指针有效的情况
        if (pUIWindow) {
            wasHandled = pUIWindow->DoEvent(arg);
        }
        // 需要默认处理
        if (!wasHandled) {
            arg.lr = ::DefWindowProcW(hwnd, message, wParam, lParam);
        }
    }
    return  arg.lr;
}

// 获取Meta的图标句柄
auto LongUI::CUIManager::GetMetaHICON(uint32_t index) noexcept -> HICON {
    // TODO DO IT
    auto& data = m_metaicons[index];
    // 没有就创建
    if (!data) {
        ID2D1Bitmap1* bitmap = this->GetBitmap(LongUIDefaultBitmapIndex);
        Meta meta; this->GetMeta(index, meta);
        D2D1_RECT_U rect = {
            static_cast<uint32_t>(meta.src_rect.left),
            static_cast<uint32_t>(meta.src_rect.top),
            static_cast<uint32_t>(meta.src_rect.right),
            static_cast<uint32_t>(meta.src_rect.bottom)
        };
        HRESULT hr = (bitmap && meta.bitmap) ? E_FAIL : S_OK;
        // 复制数据
        if (SUCCEEDED(hr)) {
            hr = bitmap->CopyFromBitmap(nullptr, meta.bitmap, &rect);
        }
        // 映射数据
        if (SUCCEEDED(hr)) {
            D2D1_MAPPED_RECT mapped_rect = { 
                LongUIDefaultBitmapSize * sizeof(RGBQUAD) ,
                m_pBitmap0Buffer
            };
            hr = bitmap->Map(D2D1_MAP_OPTIONS_READ, &mapped_rect);
        }
        // 取消映射
        if (SUCCEEDED(hr)) {
            hr = bitmap->Unmap();
        }
        assert(SUCCEEDED(hr));
        ::SafeRelease(bitmap);
    }
    assert(data && "no icon got");
    return static_cast<HICON>(data);
}

// CUIManager 构造函数
LongUI::CUIManager::CUIManager() noexcept {
}

// CUIManager 析构函数
LongUI::CUIManager::~CUIManager() noexcept {
    this->discard_resources();
}

// 获取控件 wchar_t指针
auto LongUI::CUIManager::AddS2CPair(
    const wchar_t* name, CreateControlFunction func) noexcept ->HRESULT {
    if (!name || !(*name)) return S_FALSE;
    // 创建pair
    std::pair<LongUI::CUIString, CreateControlFunction> pair(name, func);
    HRESULT hr = S_OK;
    // 插入
    try {
        m_mapString2CreateFunction.insert(pair);
    }
    // 创建失败
    CATCH_HRESULT(hr);
    return hr;
}

// 显示错误代码
void LongUI::CUIManager::ShowError(HRESULT hr, const wchar_t* str_b) noexcept {
    wchar_t buffer[LongUIStringBufferLength];
    if (!::FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,  hr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
        buffer,
        lengthof(buffer),
        nullptr)) {
        // 处理
        ::swprintf(
            buffer, LongUIStringBufferLength,
            L"Error! HRESULT Code: 0x%08X",
            hr
            );
    }
    // 错误
    this->ShowError(buffer, str_b);
}

// 创建LongUI的字体集: 本函数会进行I/O, 所以程序开始调用一次即可
auto LongUI::CUIManager::CreateLongUIFontCollection(
    IDWriteFactory* factory, const wchar_t * filename, const wchar_t * folder)
    noexcept -> IDWriteFontCollection *{
    // 字体文件枚举
    class LongUIFontFileEnumerator final : public ComStatic<QiList<IDWriteFontFileEnumerator>> {
    public:
        // 获取当前字体文件
        HRESULT STDMETHODCALLTYPE GetCurrentFontFile(IDWriteFontFile **ppFontFile) noexcept override  {
            if (!ppFontFile) return E_INVALIDARG;
            if (!m_pFilePath || !m_pFactory)  return E_FAIL;
            *ppFontFile = ::SafeAcquire(m_pCurFontFie);
            return m_pCurFontFie ? S_OK : E_FAIL;
        }
        // 移动到下一个文件
        HRESULT STDMETHODCALLTYPE MoveNext(BOOL *pHasCurrentFile) noexcept override {
            if (!pHasCurrentFile)return E_INVALIDARG;
            if (!m_pFilePath || !m_pFactory) return E_FAIL;
            HRESULT hr = S_OK;
            if (*pHasCurrentFile = *m_pFilePathNow) {
                ::SafeRelease(m_pCurFontFie);
                hr = m_pFactory->CreateFontFileReference(m_pFilePathNow, nullptr, &m_pCurFontFie);
                if (*pHasCurrentFile = SUCCEEDED(hr)) {
                    m_pFilePathNow += ::wcslen(m_pFilePathNow);
                    ++m_pFilePathNow;
                }
            }
            return hr;
        }
    public:
        // 构造函数
        LongUIFontFileEnumerator(IDWriteFactory* f) :m_pFactory(::SafeAcquire(f)) {}
        // 析构函数
        ~LongUIFontFileEnumerator() { ::SafeRelease(m_pCurFontFie); ::SafeRelease(m_pFactory); }
        // 初始化
        auto Initialize(const wchar_t* path) { m_pFilePathNow = m_pFilePath = path; };
    private:
        // 文件路径 连续字符串
        const wchar_t*              m_pFilePath = nullptr;
        // 当前文件路径
        const wchar_t*              m_pFilePathNow = nullptr;
        // 当前Direct Write Font File
        IDWriteFontFile*            m_pCurFontFie = nullptr;
        // DWrite 工厂
        IDWriteFactory*             m_pFactory;
    };
    // 字体文件载入器
    class LongUIFontCollectionLoader final : public ComStatic<QiList<IDWriteFontCollectionLoader>> {
    public:
        // 创建枚举器
        HRESULT STDMETHODCALLTYPE CreateEnumeratorFromKey(
            IDWriteFactory *pFactory,
            const void *collectionKey,
            UINT32 collectionKeySize,
            IDWriteFontFileEnumerator **ppFontFileEnumerator
            ) noexcept override {
            if (!pFactory || !ppFontFileEnumerator) return E_INVALIDARG;
            m_enumerator.LongUIFontFileEnumerator::~LongUIFontFileEnumerator();
            m_enumerator.LongUIFontFileEnumerator::LongUIFontFileEnumerator(pFactory);
            m_enumerator.Initialize(reinterpret_cast<const wchar_t*>(collectionKey));
            *ppFontFileEnumerator = &m_enumerator;
            return S_OK;
        }
    public:
        // 构造函数
        LongUIFontCollectionLoader() :m_enumerator(nullptr) {}
        // 析构函数
        ~LongUIFontCollectionLoader() = default;
    private:
        // 枚举器
        LongUIFontFileEnumerator        m_enumerator;
    };
    IDWriteFontCollection* collection = nullptr;
    constexpr size_t buffer_length = 256 * 256;
    // 申请足够的空间
    wchar_t* const buffer(new(std::nothrow) wchar_t[buffer_length]);
    if (buffer) {
        wchar_t* index = buffer; *buffer = 0;
        WIN32_FIND_DATA fileinfo;
        wchar_t file_name_path[MAX_PATH]; ::swprintf(file_name_path, MAX_PATH, L"%ls\\%ls", folder, filename);
        HANDLE hFile = ::FindFirstFileW(file_name_path, &fileinfo);
        DWORD errorcode = ::GetLastError();
        // 遍历文件
        while (hFile != INVALID_HANDLE_VALUE && errorcode != ERROR_NO_MORE_FILES) {
            ::swprintf(index, MAX_PATH, L"%ls\\%ls", folder, fileinfo.cFileName);
            index += ::wcslen(index) + 1; *index = 0;
            if (index + MAX_PATH >= buffer + buffer_length) {
                break;
            }
            ::FindNextFileW(hFile, &fileinfo);
            errorcode = ::GetLastError();
        }
        ::FindClose(hFile);
        // 当存在符合标准的文件时
        if (index != buffer) {
            LongUIFontCollectionLoader loader;
            factory->RegisterFontCollectionLoader(&loader);
            factory->CreateCustomFontCollection(
                &loader,
                buffer, reinterpret_cast<uint8_t*>(index) - reinterpret_cast<uint8_t*>(buffer),
                &collection
                );
            factory->UnregisterFontCollectionLoader(&loader);
        }
        delete[] buffer;
    }
    return collection;
}

// 从 文本格式创建几何
auto LongUI::CUIManager::CreateTextPathGeometry(
    IN const char32_t* utf32_string,
    IN size_t string_length,
    IN IDWriteTextFormat* format,
    IN ID2D1Factory* factory,
    IN OUT OPTIONAL IDWriteFontFace** _fontface,
    OUT ID2D1PathGeometry** geometry
    ) noexcept -> HRESULT {
    // 参数检查
    if (!utf32_string || !string_length || !format || !geometry || !factory) return E_INVALIDARG;
    // 字体集
    IDWriteFontCollection* collection = nullptr;
    IDWriteFontFamily* family = nullptr;
    IDWriteFont* font = nullptr;
    IDWriteFontFace* fontface = nullptr;
    ID2D1PathGeometry* pathgeometry = nullptr;
    if (_fontface) fontface = ::SafeAcquire(*_fontface);
    // 字体名称缓存
    wchar_t fontname_buffer[MAX_PATH]; *fontname_buffer = 0;
    // 必要缓存
    uint16_t glyph_indices_buffer[1024];
    // 保证空间
    uint16_t* glyph_indices = string_length > lengthof(glyph_indices_buffer) ?
        new(std::nothrow) uint16_t[string_length * sizeof(uint16_t)] : glyph_indices_buffer;
    HRESULT hr = glyph_indices ? S_OK : E_OUTOFMEMORY;
    // 创建字形
    if (!fontface) {
        // 获取字体名称
        if (SUCCEEDED(hr)) {
            hr = format->GetFontFamilyName(fontname_buffer, MAX_PATH);
        }
        // 获取字体集
        if (SUCCEEDED(hr)) {
            hr = format->GetFontCollection(&collection);
        }
        // 查找索引
        uint32_t index = 0; BOOL exists = FALSE;
        if (SUCCEEDED(hr)) {
            hr = collection->FindFamilyName(fontname_buffer, &index, &exists);
        }
        // 获取字体族
        if (SUCCEEDED(hr)) {
            if (exists) {
                hr = collection->GetFontFamily(index, &family);
            }
            else {
                hr = E_FAIL;
            }
        }
        // 获取字体
        if (SUCCEEDED(hr)) {
            hr = family->GetFirstMatchingFont(
                format->GetFontWeight(),
                format->GetFontStretch(),
                format->GetFontStyle(),
                &font
                );
        }
        // 创建字形
        if (SUCCEEDED(hr)) {
            hr = font->CreateFontFace(&fontface);
        }
    }
    // 创建几何
    if (SUCCEEDED(hr)) {
        hr = factory->CreatePathGeometry(&pathgeometry);
        ID2D1GeometrySink* sink = nullptr;
        // 打开Sink
        if (SUCCEEDED(hr)) {
            hr = pathgeometry->Open(&sink);
        }
        // 创建索引编号
        if (SUCCEEDED(hr)) {
            static_assert(sizeof(uint32_t) == sizeof(char32_t), "32 != 32 ?!");
            hr = fontface->GetGlyphIndices(
                reinterpret_cast<const uint32_t*>(utf32_string), string_length, glyph_indices
                );
        }
        // 创建轮廓路径几何
        if (SUCCEEDED(hr)) {
            hr = fontface->GetGlyphRunOutline(
                format->GetFontSize(),
                glyph_indices,
                nullptr, nullptr,
                string_length,
                true, true, sink
                );
        }
        // 关闭路径
        if (SUCCEEDED(hr)) {
            sink->Close();
        }
        ::SafeRelease(sink);
    }
    // 扫尾
    ::SafeRelease(collection);
    ::SafeRelease(family);
    ::SafeRelease(font);
    if (_fontface && !(*_fontface)) {
        *_fontface = fontface;
    }
    else {
        ::SafeRelease(fontface);
    }
    if (glyph_indices && glyph_indices != glyph_indices_buffer) {
        delete[] glyph_indices;
        glyph_indices = nullptr;
    }
    *geometry = pathgeometry;
#ifdef _DEBUG
    if (pathgeometry) {
        float float_var = 0.f;
        pathgeometry->ComputeLength(nullptr, &float_var);
        pathgeometry->ComputeArea(nullptr, &float_var);
        float_var = 0.f;
    }
#endif
    return hr;
}

// 利用几何体创建网格
auto LongUI::CUIManager::CreateMeshFromGeometry(ID2D1Geometry * geometry, ID2D1Mesh ** mesh) noexcept -> HRESULT {
    return E_NOTIMPL;
}



// 转换为Core-Mode格式
auto LongUI::CUIManager::XMLToCoreFormat(const char* xml, wchar_t* core) noexcept -> bool {
    if (!xml || !core) return false;
    wchar_t buffer[LongUIStringBufferLength];
    *buffer = 0;
    return true;
}


// 注册文本渲染器
auto LongUI::CUIManager::RegisterTextRenderer(
    UIBasicTextRenderer* renderer) noexcept -> int32_t {
    if (m_uTextRenderCount == lengthof(m_apTextRenderer)) {
        return -1;
    }
    register const auto count = m_uTextRenderCount;
    m_apTextRenderer[count] = ::SafeAcquire(renderer);
    ++m_uTextRenderCount;
    return count;
}

// CUIManager 获取文本格式
auto LongUI::CUIManager::GetTextFormat(
    uint32_t index) noexcept ->IDWriteTextFormat* {
    IDWriteTextFormat* pTextFormat = nullptr;
    if (index >= m_textFormats.size()) {
        // 越界
        UIManager << DL_Warning << L"index@" << long(index)
            << L" is out of range\n   Now set to 0" << LongUI::endl;
        index = 0;
    }
    pTextFormat = reinterpret_cast<IDWriteTextFormat*>(m_textFormats[index]);
    // 未找到
    if (!pTextFormat) {
        UIManager << DL_Error << L"index@" << long(index) << L" TF is null" << LongUI::endl;
    }
    return ::SafeAcquire(pTextFormat);

}

// 创建程序资源
auto LongUI::CUIManager::create_programs_resources() throw(std::bad_alloc &)-> void {
    // 存在二进制读取器?
    if (m_pResourceLoader) {
        // 获取位图个数
        register auto count = m_pResourceLoader->GetBitmapCount();
        m_bitmaps.reserve(count);
        // 读取位图
        for (decltype(count) i = 0; i < count; ++i) {
            m_bitmaps.push_back(m_pResourceLoader->LoadBitmapAt(*this, i));
        }
        // 获取笔刷个数
        count = m_pResourceLoader->GetBrushCount();
        m_brushes.reserve(count);
        // 读取笔刷
        for (decltype(count) i = 0; i < count; ++i) {
            m_brushes.push_back(m_pResourceLoader->LoadBrushAt(*this, i));
        }
        // 获取文本格式个数
        count = m_pResourceLoader->GetTextFormatCount();
        m_textFormats.reserve(count);
        // 读取文本格式
        for (decltype(count) i = 0; i < count; ++i) {
            m_textFormats.push_back(m_pResourceLoader->LoadTextFormatAt(*this, i));
        }
        // 获取Meta个数
        count = m_pResourceLoader->GetMetaCount();
        m_metas.reserve(count);
        // 图标
        m_metaicons.resize(count);
        // 读取文本格式
        for (decltype(count) i = 0; i < count; ++i) {
            Meta meta; m_pResourceLoader->LoadMetaAt(*this, i, meta);
            m_metas.push_back(meta);
        }
    }
    else {
        // pugixml 使用的是句柄式, 所以下面的代码是安全的.
        // 但是会稍微损失性能(估计cache命中就赚回一点了)
        // 所以pugixml才是真正的C++代码, 自己的就偏向于
        // C风格, 全是指针, 到处都是指针检查
        register auto now_node = m_docResource.first_child().first_child();
        while (now_node) {
            // 位图?
            if (!::strcmp(now_node.name(), "Bitmap")) {
                this->add_bitmap(now_node);
            }
            // 笔刷?
            else if (!::strcmp(now_node.name(), "Brush")) {
                this->add_brush(now_node);
            }
            // 文本格式?
            else if (!::strcmp(now_node.name(), "Font") ||
                !::strcmp(now_node.name(), "TextFormat")) {
                this->add_textformat(now_node);
            }
            // 图元?
            else if (!::strcmp(now_node.name(), "Meta")) {
                this->add_meta(now_node);
            }
            // 动画图元?
            else if (!::strcmp(now_node.name(), "MetaEx")) {
                this->add_meta(now_node);
            }
            // 推进
            now_node = now_node.next_sibling();
        }
    }
}


// UIManager 创建
auto LongUI::CUIManager::create_resources() noexcept ->HRESULT {
    // 检查渲染配置
    bool cpu_rendering = this->configure->IsRenderByCPU();
    // 待用适配器
    IDXGIAdapter1* ready2use = nullptr;
    // 枚举显示适配器
    if(!cpu_rendering) {
        IDXGIFactory1* temp_factory = nullptr;
        // 创建一个临时工程
        register auto hr = LongUI::Dll::CreateDXGIFactory1(IID_IDXGIFactory1, reinterpret_cast<void**>(&temp_factory));
        if (SUCCEEDED(hr)) {
            IDXGIAdapter1* apAdapters[256]; size_t adnum;
            // 枚举适配器
            for (adnum = 0; adnum < lengthof(apAdapters); ++adnum) {
                if (temp_factory->EnumAdapters1(adnum, apAdapters + adnum) == DXGI_ERROR_NOT_FOUND) {
                    break;
                }
            }
            // 选择适配器
            register auto index = this->configure->ChooseAdapter(apAdapters, adnum);
            if (index < adnum) {
                ready2use = ::SafeAcquire(apAdapters[index]);
            }
            // 释放适配器
            for (size_t i = 0; i < adnum; ++i) {
                ::SafeRelease(apAdapters[i]);
            }
        }
        ::SafeRelease(temp_factory);
    }
    // 创建设备资源
    register HRESULT hr /*= m_docResource.Error() ? E_FAIL :*/ S_OK;
    // 创建 D3D11设备与设备上下文 
    if (SUCCEEDED(hr)) {
        // D3D11 创建flag 
        // 一定要有D3D11_CREATE_DEVICE_BGRA_SUPPORT
        // 否则创建D2D设备上下文会失败
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
#ifdef LONGUI_D3D_DEBUG
        // Debug状态 有D3D DebugLayer就可以取消注释
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
        auto tmpflag = D3D11_CREATE_DEVICE_FLAG(creationFlags);
#endif
#endif
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };
        // 创建设备
        hr = LongUI::Dll::D3D11CreateDevice(
            // 设置为渲染
            ready2use,
            // 根据情况选择类型
            cpu_rendering ? D3D_DRIVER_TYPE_WARP : 
                (ready2use ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE),
            // 没有软件接口
            nullptr,
            // 创建flag
            creationFlags,
            // 欲使用的特性等级列表
            featureLevels,
            // 特性等级列表长度
            lengthof(featureLevels),
            // SDK 版本
            D3D11_SDK_VERSION,
            // 返回的D3D11设备指针
            &m_pd3dDevice,
            // 返回的特性等级
            &m_featureLevel,
            // 返回的D3D11设备上下文指针
            &m_pd3dDeviceContext
            );
        // 检查
        if (FAILED(hr)) {
            UIManager << DL_Hint << L"Create D3D11 Device Failed,"
                L" Now, Try to Create In WARP Mode" << LongUI::endl;
        }
        // 创建失败则尝试使用软件
        if (FAILED(hr)) {
            hr = LongUI::Dll::D3D11CreateDevice(
                // 设置为渲染
                nullptr,
                // 根据情况选择类型
                D3D_DRIVER_TYPE_WARP,
                // 没有软件接口
                nullptr,
                // 创建flag
                creationFlags,
                // 欲使用的特性等级列表
                featureLevels,
                // 特性等级列表长度
                lengthof(featureLevels),
                // SDK 版本
                D3D11_SDK_VERSION,
                // 返回的D3D11设备指针
                &m_pd3dDevice,
                // 返回的特性等级
                &m_featureLevel,
                // 返回的D3D11设备上下文指针
                &m_pd3dDeviceContext
                );
        }
    }
#ifdef _DEBUG
#ifdef LONGUI_D3D_DEBUG
    // 创建 ID3D11Debug
    if (SUCCEEDED(hr)) {
        hr = m_pd3dDevice->QueryInterface(LongUI_IID_PV_ARGS(m_pd3dDebug));
    }
#endif
#endif
    // 创建 IDXGIDevice
    if (SUCCEEDED(hr)) {
        hr = m_pd3dDevice->QueryInterface(LongUI_IID_PV_ARGS(m_pDxgiDevice));
    }
    // 创建D2D设备
    if (SUCCEEDED(hr)) {
        hr = m_pd2dFactory->CreateDevice(m_pDxgiDevice, &m_pd2dDevice);
    }
    // 创建D2D设备上下文
    if (SUCCEEDED(hr)) {
        hr = m_pd2dDevice->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            &m_pd2dDeviceContext
            );
    }
    // 获取Dxgi适配器 可以获取该适配器信息
    if (SUCCEEDED(hr)) {
        // 顺带使用像素作为单位
        m_pd2dDeviceContext->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
        hr = m_pDxgiDevice->GetAdapter(&m_pDxgiAdapter);
    }
#ifdef _DEBUG
    // 输出显卡信息
    if (SUCCEEDED(hr)) {
        DXGI_ADAPTER_DESC desc = { 0 }; 
        m_pDxgiAdapter->GetDesc(&desc);
        UIManager << DL_Hint << desc << LongUI::endl;
    }
#endif
    // 获取Dxgi工厂
    if (SUCCEEDED(hr)) {
        hr = m_pDxgiAdapter->GetParent(LongUI_IID_PV_ARGS(m_pDxgiFactory));
    }
#ifdef LONGUI_VIDEO_IN_MF
    uint32_t token = 0;
    // 多线程
    if (SUCCEEDED(hr)) {
        ID3D10Multithread* mt = nullptr;
        hr = m_pd3dDevice->QueryInterface(IID_ID3D10Multithread, (void**)&mt);
        // 保护
        if (SUCCEEDED(hr)) {
            mt->SetMultithreadProtected(TRUE);
        }
        ::SafeRelease(mt);
    }
    // 设置MF
    if (SUCCEEDED(hr)) {
        hr = ::MFStartup(MF_VERSION);
    }
    // 创建MF Dxgi 设备管理器
    if (SUCCEEDED(hr)) {
        hr = ::MFCreateDXGIDeviceManager(&token, &m_pDXGIManager);
    }
    // 重置设备
    if (SUCCEEDED(hr)) {
        hr = m_pDXGIManager->ResetDevice(m_pd3dDevice, token);
    }
    // 创建MF媒体类工厂
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(
            CLSID_MFMediaEngineClassFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            LongUI_IID_PV_ARGS(m_pMediaEngineFactory)
            );
    }
#endif
    /*// 禁止 Alt + Enter 全屏
    if (SUCCEEDED(hr)) {
        hr = m_pDxgiFactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
    }*/
    // 创建索引0资源
    if (SUCCEEDED(hr)) {
        // 可Map的位图
        if (SUCCEEDED(hr)) {
            ID2D1Bitmap1* bitmap_index0 = nullptr;
            hr = m_pd2dDeviceContext->CreateBitmap(
                D2D1::SizeU(LongUIDefaultBitmapSize, LongUIDefaultBitmapSize),
                nullptr, LongUIDefaultBitmapSize * 4,
                D2D1::BitmapProperties1(
                static_cast<D2D1_BITMAP_OPTIONS>(LongUIDefaultBitmapOptions),
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
                ),
                &bitmap_index0
                );
            m_bitmaps.push_back(bitmap_index0);
        }
        // 笔刷
        if (SUCCEEDED(hr)) {
            ID2D1SolidColorBrush* scbrush = nullptr;
            D2D1_COLOR_F color = D2D1::ColorF(D2D1::ColorF::Black);
            hr = m_pd2dDeviceContext->CreateSolidColorBrush(
                &color, nullptr, &scbrush
                );
            m_brushes.push_back(scbrush);
        }
        // 文本格式
        if (SUCCEEDED(hr)) {
            IDWriteTextFormat* format = nullptr;
            hr = m_pDWriteFactory->CreateTextFormat(
                LongUIDefaultTextFontName,
                m_pFontCollection,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                LongUIDefaultTextFontSize,
                m_szLocaleName,
                &format
                );
            if (format) {
                format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
                format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
                m_textFormats.push_back(format);
            }
        }
        // 图元
        Meta meta = { 0 }; m_metas.push_back(meta);
    }
    // 创建系统笔刷
    if (SUCCEEDED(hr)) {
        hr = this->create_system_brushes();
    }
    // 创建资源描述资源
    if (SUCCEEDED(hr)) {
        try {
            this->create_programs_resources();
        }
        CATCH_HRESULT(hr);
    }
    ::SafeRelease(ready2use);
    // 设置文本渲染器数据
    if (SUCCEEDED(hr)) {
        for (uint32_t i = 0u; i < m_uTextRenderCount; ++i) {
            m_apTextRenderer[i]->SetNewRT(m_pd2dDeviceContext);
            m_apTextRenderer[i]->SetNewBrush(static_cast<ID2D1SolidColorBrush*>(m_brushes[0]));
        }
        // 重建所有窗口
        for (auto i : m_windows) {
            reinterpret_cast<UIWindow*>(i)->Recreate(nullptr);
        }
    }
    // 断言 HR
    AssertHR(hr);
    return hr;
}


// 创建系统笔刷
auto LongUI::CUIManager::create_system_brushes() noexcept -> HRESULT {
    HRESULT hr = S_OK;
    /*
    焦点: 0x3399FF 矩形描边, 并且内边有虚线矩形
    0. 禁用: 0xD9灰度 矩形描边; 中心 0xEF灰色
    1. 普通: 0xAC灰度 矩形描边; 中心 从上到下0xF0灰色到0xE5灰色渐变
    2. 移上: 0x7EB4EA 矩形描边; 中心 从上到下0xECF4FC到0xDCECFC渐变
    3. 按下: 0x569DE5 矩形描边; 中心 从上到下0xDAECFC到0xC4E0FC渐变
    */
    // 禁用
    if (SUCCEEDED(hr)) {
        hr = m_pd2dDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(0xEFEFEF),
            reinterpret_cast<ID2D1SolidColorBrush**>(m_apSystemBrushes + Status_Disabled)
            );
    }
    // 普通
    if (SUCCEEDED(hr)) {
        ID2D1GradientStopCollection* collection = nullptr;
        D2D1_GRADIENT_STOP stops[] = {
            { 0.f, D2D1::ColorF(0xF0F0F0) },
            { 1.f, D2D1::ColorF(0xE5E5E5) }
        };
        // 渐变关键点集
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateGradientStopCollection(
                stops, lengthof(stops), &collection
                );
        }
        // 创建笔刷
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    D2D1::Point2F(), D2D1::Point2F(0.f, 1.f)
                    ),
                collection,
                reinterpret_cast<ID2D1LinearGradientBrush**>(m_apSystemBrushes + Status_Normal)
                );
        }
        ::SafeRelease(collection);
    }
    // 移上
    if (SUCCEEDED(hr)) {
        ID2D1GradientStopCollection* collection = nullptr;
        D2D1_GRADIENT_STOP stops[] = {
            { 0.f, D2D1::ColorF(0xECF4FC) },
            { 1.f, D2D1::ColorF(0xDCECFC) }
        };
        // 渐变关键点集
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateGradientStopCollection(
                stops, lengthof(stops), &collection
                );
        }
        // 创建笔刷
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    D2D1::Point2F(), D2D1::Point2F(0.f, 1.f)
                    ),
                collection,
                reinterpret_cast<ID2D1LinearGradientBrush**>(m_apSystemBrushes + Status_Hover)
                );
        }
        ::SafeRelease(collection);
    }
    // 按下
    if (SUCCEEDED(hr)) {
        ID2D1GradientStopCollection* collection = nullptr;
        D2D1_GRADIENT_STOP stops[] = {
            { 0.f, D2D1::ColorF(0xDAECFC) },
            { 1.f, D2D1::ColorF(0xC4E0FC) }
        };
        // 渐变关键点集
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateGradientStopCollection(
                stops, lengthof(stops), &collection
                );
        }
        // 创建笔刷
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    D2D1::Point2F(), D2D1::Point2F(0.f, 1.f)
                    ),
                collection,
                reinterpret_cast<ID2D1LinearGradientBrush**>(m_apSystemBrushes + Status_Pushed)
                );
        }
        ::SafeRelease(collection);
    }
    return hr;
}

#ifdef _DEBUG
template<typename T>
void AssertRelease(T* t) {
    auto count = t->Release();
    if (count) {
        UIManager << DL_Hint << L"[IUnknown *] count :" << long(count) << LongUI::endl;
        int breakpoint = 0;
    }
}

#define SafeReleaseContainer(c) \
    if (c.size()) {\
        for (auto itr = c.begin(); itr != c.end(); ++itr) {\
             register auto* interfacegot = static_cast<ID2D1Bitmap*>(*itr);\
             if (interfacegot) {\
                AssertRelease(interfacegot);\
                 *itr = nullptr;\
             }\
        }\
        c.clear();\
    }
#else
#define SafeReleaseContainer(c) \
    if (c.size()) {\
        for (auto itr = c.begin(); itr != c.end(); ++itr) {\
             register auto* interfacegot = static_cast<ID2D1Bitmap*>(*itr);\
             if (interfacegot) {\
                 interfacegot->Release();\
                 *itr = nullptr;\
             }\
        }\
        c.clear();\
    }
#endif
// UIManager 丢弃
void LongUI::CUIManager::discard_resources() noexcept {
    // 释放 位图
    SafeReleaseContainer(m_bitmaps);
    // 释放 笔刷
    SafeReleaseContainer(m_brushes);
    // 释放文本格式
    SafeReleaseContainer(m_textFormats);
    // meta直接释放
    if (m_metas.size()) {
        m_metas.clear();
    }
    // 图标摧毁
    if (m_metaicons.size()) {
        for (auto itr = m_metaicons.begin(); itr != m_metaicons.end(); ++itr) {
             register auto handle = static_cast<HICON>(*itr);
             if (handle) {
                 ::DestroyIcon(handle);
                 *itr = nullptr;
             }
        }
        m_metaicons.clear();
    }
    // 释放 设备
    ::SafeRelease(m_pDxgiFactory);
    ::SafeRelease(m_pd2dDeviceContext);
    ::SafeRelease(m_pd2dDevice);
    ::SafeRelease(m_pDxgiAdapter);
    ::SafeRelease(m_pDxgiDevice);
    ::SafeRelease(m_pd3dDevice);
    ::SafeRelease(m_pd3dDeviceContext);
#ifdef LONGUI_VIDEO_IN_MF
    ::SafeRelease(m_pDXGIManager);
    ::SafeRelease(m_pMediaEngineFactory);
    ::MFShutdown();
#endif
#ifdef _DEBUG
    if (m_pd3dDebug) {
        m_pd3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
    }
    ::SafeRelease(m_pd3dDebug);
#endif
}



// 获取笔刷
auto LongUI::CUIManager::GetBrush(
    uint32_t index) noexcept -> ID2D1Brush* {
    ID2D1Brush* brush = nullptr;
    if (index >= m_brushes.size()) {
        // 越界
        UIManager << DL_Warning << L"index@" << long(index)
            << L"is out of range\n   Now set to 0" << LongUI::endl;
        index = 0;
    }
    brush = reinterpret_cast<ID2D1Brush*>(m_brushes[index]);
    // 错误
    if (!brush) {
        UIManager << DL_Error << L"index@" << long(index) << L"brush is null" << LongUI::endl;
    }
    return ::SafeAcquire(brush);
}

// 获取位图
auto LongUI::CUIManager::GetBitmap(
    uint32_t index) noexcept ->ID2D1Bitmap1* {
    ID2D1Bitmap1* bitmap = nullptr;
    if (index >= m_bitmaps.size()) {
        // 越界
        UIManager << DL_Warning << L"index@" << long(index)
            << L"is out of range\n   Now set to 0" << LongUI::endl;
        index = 0;
    }
    bitmap = static_cast<ID2D1Bitmap1*>(m_bitmaps[index]);
    // 错误
    if (!bitmap) {
        UIManager << DL_Error << L"index@" << long(index) << L"bitmap is null" << LongUI::endl;
    }
    return ::SafeAcquire(bitmap);
}

// 获取图元
void LongUI::CUIManager::GetMeta(
    uint32_t index, LongUI::Meta& meta) noexcept {
    if (index >= m_metas.size()) {
        // 越界
        UIManager << DL_Warning << L"index@" << long(index)
            << L"is out of range\n   Now set to 0" << LongUI::endl;
        index = 0;
    }
    meta = m_metas[index];
}

// 添加位图
void LongUI::CUIManager::add_bitmap(
    const pugi::xml_node node) noexcept {
    assert(node && "bad argument");
    // 获取路径
    const char* uri = node.attribute("res").value();
    // 载入位图
    auto bitmap = this->configure->LoadBitmapByRI(*this, uri);
    // 没有
    if (!bitmap) {
        UIManager << DL_Error << L"Resource Identifier: [" << uri << L"], got a null pointer" << LongUI::endl;
    }
    m_bitmaps.push_back(bitmap);
}

// 添加笔刷
void LongUI::CUIManager::add_brush(
    const pugi::xml_node node) noexcept {
    union {
        ID2D1SolidColorBrush*       scb;
        ID2D1LinearGradientBrush*   lgb;
        ID2D1RadialGradientBrush*   rgb;
        ID2D1BitmapBrush1*          b1b;
        ID2D1Brush*                 brush;
    };
    brush = nullptr;
    const char* str = nullptr;
    assert(node && "bad argument");
    // 笔刷属性
    D2D1_BRUSH_PROPERTIES brush_prop = D2D1::BrushProperties();
    if (str = node.attribute("opacity").value()) {
        brush_prop.opacity = static_cast<float>(::LongUI::AtoF(str));
    }
    if (str = node.attribute("transform").value()) {
        UIControl::MakeFloats(str, &brush_prop.transform._11, 6);
    }
    // 检查类型
    auto type = BrushType::Type_SolidColor;
    if (str = node.attribute("type").value()) {
        type = static_cast<decltype(type)>(::LongUI::AtoI(str));
    }
    switch (type)
    {
    case LongUI::BrushType::Type_SolidColor:
    {
        D2D1_COLOR_F color;
        // 获取颜色
        if (!UIControl::MakeColor(node.attribute( "color").value(), color)) {
            color = D2D1::ColorF(D2D1::ColorF::Black);
        }
        m_pd2dDeviceContext->CreateSolidColorBrush(&color, &brush_prop, &scb);
    }
    break;
    case LongUI::BrushType::Type_LinearGradient:
        __fallthrough;
    case LongUI::BrushType::Type_RadialGradient:
        if (str = node.attribute("stops").value()) {
            // 语法 [pos0, color0] [pos1, color1] ....
            uint32_t stop_count = 0;
            ID2D1GradientStopCollection * collection = nullptr;
            D2D1_GRADIENT_STOP stops[LongUIMaxGradientStop];
            D2D1_GRADIENT_STOP* now_stop = stops;

            char buffer[LongUIStringBufferLength];
            // 复制到缓冲区
            strcpy(buffer, str);
            char* index = buffer;
            const char* paragraph = nullptr;
            register char ch = 0;
            bool ispos = false;
            // 遍历检查
            while (ch = *index) {
                // 查找第一个浮点数做为位置
                if (ispos) {
                    // ,表示位置段结束, 该解析了
                    if (ch = ',') {
                        *index = 0;
                        now_stop->position = LongUI::AtoF(paragraph);
                        ispos = false;
                        paragraph = index + 1;
                    }
                }
                // 查找后面的数值做为颜色
                else {
                    // [ 做为位置段标识开始
                    if (ch == '[') {
                        paragraph = index + 1;
                        ispos = true;
                    }
                    // ] 做为颜色段标识结束 该解析了
                    else if (ch == ']') {
                        *index = 0;
                        UIControl::MakeColor(paragraph, now_stop->color);
                        ++now_stop;
                        ++stop_count;
                    }
                }
            }
            // 创建StopCollection
            m_pd2dDeviceContext->CreateGradientStopCollection(stops, stop_count, &collection);
            if (collection) {
                // 线性渐变?
                if (type == LongUI::BrushType::Type_LinearGradient) {
                    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES lgbprop = {
                        {0.f, 0.f}, {0.f, 0.f}
                    };
                    // 检查属性
                    UIControl::MakeFloats(node.attribute("start").value(), &lgbprop.startPoint.x, 2);
                    UIControl::MakeFloats(node.attribute("end").value(), &lgbprop.startPoint.x, 2);
                    // 创建笔刷
                    m_pd2dDeviceContext->CreateLinearGradientBrush(
                        &lgbprop, &brush_prop, collection, &lgb
                        );
                }
                // 径向渐变笔刷
                else {
                    D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES rgbprop = {
                        {0.f, 0.f}, {0.f, 0.f}, 0.f, 0.f
                    };
                    // 检查属性
                    UIControl::MakeFloats(node.attribute("center").value(), &rgbprop.center.x, 2);
                    UIControl::MakeFloats(node.attribute("offset").value(), &rgbprop.gradientOriginOffset.x, 2);
                    UIControl::MakeFloats(node.attribute("rx").value(), &rgbprop.radiusX, 1);
                    UIControl::MakeFloats(node.attribute("ry").value(), &rgbprop.radiusY, 1);
                    // 创建笔刷
                    m_pd2dDeviceContext->CreateRadialGradientBrush(
                        &rgbprop, &brush_prop, collection, &rgb
                        );
                }
                collection->Release();
                collection = nullptr;
            }
        }
        break;
    case LongUI::BrushType::Type_Bitmap:
        if (str = node.attribute("bitmap").value()) {
            auto index = LongUI::AtoI(str);
            D2D1_BITMAP_BRUSH_PROPERTIES1 bbprop = {
                D2D1_EXTEND_MODE_CLAMP, D2D1_EXTEND_MODE_CLAMP,D2D1_INTERPOLATION_MODE_LINEAR
            };
            // 检查属性
            if (str = node.attribute("extendx").value()) {
                bbprop.extendModeX = static_cast<D2D1_EXTEND_MODE>(LongUI::AtoI(str));
            }
            if (str = node.attribute("extendy").value()) {
                bbprop.extendModeY = static_cast<D2D1_EXTEND_MODE>(LongUI::AtoI(str));
            }
            if (str = node.attribute("interpolation").value()) {
                bbprop.interpolationMode = static_cast<D2D1_INTERPOLATION_MODE>(LongUI::AtoI(str));
            }
            // 创建笔刷
            m_pd2dDeviceContext->CreateBitmapBrush(
                static_cast<ID2D1Bitmap*>(m_bitmaps[index]), 
                &bbprop, &brush_prop, &b1b
                );
        }
        break;
    }
    // 做做样子检查一下
    assert(brush);
    m_brushes.push_back(brush);
}

// 添加图元
void LongUI::CUIManager::add_meta(
    const pugi::xml_node node) noexcept {
    LongUI::Meta meta = {
        { 0.f, 0.f, 1.f, 1.f },
        nullptr, BitmapRenderRule::Rule_Scale, 
        D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR
    };
    const char* str = nullptr;
    assert(node && "bad argument");
    // 获取位图
    meta.bitmap = static_cast<ID2D1Bitmap1*>(m_bitmaps[LongUI::AtoI(node.attribute("bitmap").value())]);
    // 获取渲染规则
    if (str = node.attribute("rule").value()) {
        meta.rule = static_cast<BitmapRenderRule>(LongUI::AtoI(str));
    }
    // 获取插值模式
    if (str = node.attribute("interpolation").value()) {
        meta.interpolation = static_cast<uint16_t>(LongUI::AtoI(str));
    }
    // 获取矩形
    UIControl::MakeFloats(node.attribute("rect").value(), &meta.src_rect.left, 4);
    // 推送
    m_metas.push_back(meta);
    // 图标
    m_metaicons.push_back(nullptr);
}


// 添加文本格式
void LongUI::CUIManager::add_textformat(
    const pugi::xml_node node) noexcept {
    register const char* str = nullptr;
    assert(node && "bad argument");
    CUIString fontfamilyname(L"Arial");
    DWRITE_FONT_WEIGHT fontweight = DWRITE_FONT_WEIGHT_NORMAL;
    DWRITE_FONT_STYLE fontstyle = DWRITE_FONT_STYLE_NORMAL;
    DWRITE_FONT_STRETCH fontstretch = DWRITE_FONT_STRETCH_NORMAL;
    float fontsize = 12.f ;
    // 获取字体名称
    UIControl::MakeString(node.attribute("family").value(), fontfamilyname);
    // 获取字体粗细
    if (str = node.attribute("weight").value()) {
        fontweight = static_cast<DWRITE_FONT_WEIGHT>(LongUI::AtoI(str));
    }
    // 获取字体风格
    if (str = node.attribute("style").value()) {
        fontstyle = static_cast<DWRITE_FONT_STYLE>(LongUI::AtoI(str));
    }
    // 获取字体拉伸
    if (str = node.attribute("stretch").value()) {
        fontstretch = static_cast<DWRITE_FONT_STRETCH>(LongUI::AtoI(str));
    }
    // 获取字体大小
    if (str = node.attribute("size").value()) {
        fontsize = LongUI::AtoF(str);
    }
    // 创建基本字体
    IDWriteTextFormat* textformat = nullptr;
    m_pDWriteFactory->CreateTextFormat(
        fontfamilyname.c_str(),
        m_pFontCollection,
        fontweight,
        fontstyle,
        fontstretch,
        fontsize,
        m_szLocaleName,
        &textformat
        );
    // 成功获取则再设置
    if (textformat) {
        // DWRITE_LINE_SPACING_METHOD;
        DWRITE_FLOW_DIRECTION flowdirection = DWRITE_FLOW_DIRECTION_TOP_TO_BOTTOM;
        float tabstop = fontsize * 4.f;
        DWRITE_PARAGRAPH_ALIGNMENT valign = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
        DWRITE_TEXT_ALIGNMENT halign = DWRITE_TEXT_ALIGNMENT_LEADING;
        DWRITE_READING_DIRECTION readingdirection = DWRITE_READING_DIRECTION_LEFT_TO_RIGHT;
        DWRITE_WORD_WRAPPING wordwrapping = DWRITE_WORD_WRAPPING_NO_WRAP;
        // 检查段落排列方向
        if (str = node.attribute("flowdirection").value()) {
            flowdirection = static_cast<DWRITE_FLOW_DIRECTION>(LongUI::AtoI(str));
        }
        // 检查Tab宽度
        if (str = node.attribute("tabstop").value()) {
            tabstop = LongUI::AtoF(str);
        }
        // 检查段落(垂直)对齐
        if (str = node.attribute("valign").value()) {
            valign = static_cast<DWRITE_PARAGRAPH_ALIGNMENT>(LongUI::AtoI(str));
        }
        // 检查文本(水平)对齐
        if (str = node.attribute("halign").value()) {
            halign = static_cast<DWRITE_TEXT_ALIGNMENT>(LongUI::AtoI(str));
        }
        // 检查阅读进行方向
        if (str = node.attribute("readingdirection").value()) {
            readingdirection = static_cast<DWRITE_READING_DIRECTION>(LongUI::AtoI(str));
        }
        // 检查自动换行
        if (str = node.attribute("wordwrapping").value()) {
            wordwrapping = static_cast<DWRITE_WORD_WRAPPING>(LongUI::AtoI(str));
        }
        // 设置段落排列方向
        textformat->SetFlowDirection(flowdirection);
        // 设置Tab宽度
        textformat->SetIncrementalTabStop(tabstop);
        // 设置段落(垂直)对齐
        textformat->SetParagraphAlignment(valign);
        // 设置文本(水平)对齐
        textformat->SetTextAlignment(halign);
        // 设置阅读进行方向
        textformat->SetReadingDirection(readingdirection);
        // 设置自动换行
        textformat->SetWordWrapping(wordwrapping);
    }
    m_textFormats.push_back(textformat);
}

// 格式化文字
/*
control char    C-Type      Infomation                                  StringInlineParamSupported
  
%%               [none]      As '%' Character(like %% in ::printf)                 ---
%a %A      [const wchar_t*] string add(like %S in ::printf)                Yes but no "," char 

%C              [float4*]    new font color range start                            Yes
%c              [uint32_t]   new font color range start, with alpha                Yes
!! color is also a drawing effect

%d %D         [IUnknown*]    new drawing effect range start                 Yes and Extensible

%S %S            [float]     new font size range start                             Yes

%n %N       [const wchar_t*] new font family name range start               Yes but No "," char

%h %H            [enum]      new font stretch range start                          Yes

%y %Y            [enum]      new font style range start                            Yes

%w %W            [enum]      new font weight range start                           Yes

%u %U            [BOOL]      new underline range start                          Yes(0 or 1)

%e %E            [BOOL]      new strikethrough range start                      Yes(0 or 1)

%i %I            [IDIO*]     new inline object range start                  Yes and Extensible

%] %}            [none]      end of the last range                                 ---
//  Unsupported
%f %F   [UNSPT]  [IDFC*]     new font collection range start                       ---
                                IDWriteFontCollection*

%t %T   [UNSPT]  [IDT*]      new inline object range start                         ---
                                IDWriteTypography*

%t %T   [UNSPT] [char_t*]    new locale name range start                           ---

FORMAT IN STRING
the va_list(ap) can be nullptr while string format
include the PARAMETERS,
using %p or %P to mark PARAMETERS start

*/

// 创建格式文本
auto LongUI::CUIManager::FormatTextCore(
    FormatTextConfig& config,
    const wchar_t* format,
    ...) noexcept->IDWriteTextLayout* {
    va_list ap;
    va_start(ap, format);
    return CUIManager::FormatTextCore(config, format, ap);
    
}
/*
 L"He%llo, World"
*/
#define COLOR8TOFLOAT(a) (static_cast<float>(a)/255.f)

// find next param
template<typename T>
const wchar_t*  __fastcall FindNextToken(T* buffer, const wchar_t* stream, size_t token_num) {
    register wchar_t ch;
    while (ch = *stream) {
        ++stream;
        if (ch == L',' && !(--token_num)) {
            break;
        }
        *buffer = static_cast<T>(ch);
        ++buffer;
    }
    *buffer = 0;
    return stream;
}


#define CUIManager_GetNextTokenW(n) param = FindNextToken(param_buffer, param, n)
#define CUIManager_GetNextTokenA(n) param = FindNextToken(reinterpret_cast<char*>(param_buffer), param, n)


// 创建格式文本
// 本函数耗时参考:
// 包含释放数据(::SafeRelease(layout))
// 1. L"%cHello%], world!%p#FFFF0000"
// Debug    : 循环 1000000(一百万)次，耗时8750ms(精确到16ms)
// Release  : 循环 1000000(一百万)次，耗时3484ms(精确到16ms)
// 2. L"%cHello%], world!%cHello%], world!%p#FFFF0000, #FF00FF00"
// Debug    : 循环 1000000(一百万)次，耗时13922ms(精确到16ms)
// Release  : 循环 1000000(一百万)次，耗时 6812ms(精确到16ms)
// 结论: Release版每处理一个字符(包括格式与参数)平均消耗0.12微秒, Debug版加倍
// 假设: 60Hz每帧16ms 拿出8ms处理本函数, 可以处理6万6个字符
//一般论: 不可能每帧调用6万字, 一般可能每帧处理数百字符(忙碌时), 可以忽略不计
auto  LongUI::CUIManager::FormatTextCore(
    FormatTextConfig& config,
    const wchar_t* format, 
    va_list ap) noexcept->IDWriteTextLayout* {
    const wchar_t* param = nullptr;
    // 检查是否带参数
    if (!ap) {
        register auto format_param_tmp = format;
        register wchar_t ch;
        while (ch = *format_param_tmp) {
            if (ch == L'%') {
                ++format_param_tmp;
                ch = *format_param_tmp;
                if (ch == L'p' || ch == L'p') {
                    param = format_param_tmp + 1;
                    break;
                }
            }
            ++format_param_tmp;
        }
        assert(param && "ap set to nullptr, but none param found.");
    }
    // Color
    union ARGBColor {
        struct {
            uint8_t b;
            uint8_t g;
            uint8_t r;
            uint8_t a;
        };
        uint32_t u32;
    };
    // Range Type
    enum class R :size_t { N, W, Y, H, S, U, E, D, I };
    // Range Data
    struct RangeData {
        DWRITE_TEXT_RANGE       range;
        union {
            const wchar_t*      name;       // N
            DWRITE_FONT_WEIGHT  weight;     // W
            DWRITE_FONT_STYLE   style;      // Y
            DWRITE_FONT_STRETCH stretch;    // H
            float               size;       // S
            BOOL                underline;  // U
            BOOL                strikethr;  // E
            IUnknown*           draweffect; // D
            IDWriteInlineObject*inlineobj;  // I
            // ----------------------------
            D2D1_COLOR_F*       color;      // C
            uint32_t            u32;        // c
            ARGBColor           u32color;   // c
        };
        R                       range_type;
    } range_data;
    assert(format && "bad argument");
    IDWriteTextLayout* layout = nullptr;
    register UIColorEffect* tmp_color = nullptr;
    // 缓存字符串长度
    uint32_t string_length = 0;
    // 当前字符
    wchar_t ch;
    // 缓冲区索引
    wchar_t* buffer_index;
    // 参数缓冲区
    wchar_t param_buffer[256];
    // 缓冲区
    wchar_t buffer[LongUIStringBufferLength];
    // 缓冲区
    wchar_t fontname_buffer[LongUIStringBufferLength];
    auto fontname_buffer_index = fontname_buffer;
    // 使用栈
    FixedStack<RangeData, 1024> stack_check, statck_set;
    // 缓存起点
    buffer_index = buffer;
    // 便利
    while (ch = *format) {
        // 为%时, 检查下一字符
        if (ch == L'%' && (++format, ch = *format)) {
            switch (ch)
            {
            case L'%':
                // 添加%
                *buffer_index = L'%';
                ++buffer_index;
                ++string_length;
                break;
            case L'A': case L'a': // [A]dd string
                // 复制字符串
                {
                    register const wchar_t* i;
                    if (ap) {
                        i = va_arg(ap, const wchar_t*);
                    }
                    else {
                        CUIManager_GetNextTokenW(1);
                        i = param_buffer;
                    }
                    for (; *i; ++i) {
                        *buffer_index = *i;
                        ++string_length;
                        ++range_data.name;
                    }
                }
                break;
            case L'C': // [C]olor in float4
                // 浮点数组颜色开始标记: 
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.color = va_arg(ap, D2D1_COLOR_F*);
                }
                range_data.range_type = R::D;
                // 动态创建颜色效果
                tmp_color = UIColorEffect::Create();
                assert(tmp_color && "C");
                // 从范围数据中获取
                if (ap) {
                    tmp_color->color = *range_data.color;
                }
                // 直接设置
                else {
                    CUIManager_GetNextTokenA(4);
                    UIControl::MakeColor(reinterpret_cast<char*>(param_buffer), tmp_color->color);
                }
                range_data.draweffect = tmp_color;
                stack_check.push(range_data);
                break;
            case L'c': // [C]olor in uint32
                // 32位颜色开始标记: 
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.u32 = va_arg(ap, uint32_t);
                }
                range_data.range_type = R::D;
                // 动态创建颜色效果
                tmp_color = UIColorEffect::Create();
                assert(tmp_color && "c");
                if (ap) {
                    tmp_color->color.b = COLOR8TOFLOAT(range_data.u32color.b);
                    tmp_color->color.g = COLOR8TOFLOAT(range_data.u32color.g);
                    tmp_color->color.r = COLOR8TOFLOAT(range_data.u32color.r);
                    tmp_color->color.a = COLOR8TOFLOAT(range_data.u32color.a);
                }
                else {
                    CUIManager_GetNextTokenA(1);
                    UIControl::MakeColor(reinterpret_cast<char*>(param_buffer), tmp_color->color);
                }
                range_data.draweffect = tmp_color;
                stack_check.push(range_data);
                break;
            case 'D': case 'd': // [D]rawing effect
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.draweffect = va_arg(ap, IUnknown*);
                }
                else {
                    CUIManager_GetNextTokenW(1);
                    IUnknown* result = nullptr;
                    if (config.inline_handler) {
                        result = config.inline_handler(0, param_buffer);
                    }
                    range_data.draweffect = result;
                }
                range_data.range_type = R::D;
                stack_check.push(range_data);
                break;
            case 'E': case 'e': // strik[E]through
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.strikethr = va_arg(ap, BOOL);
                }
                else {
                    CUIManager_GetNextTokenA(1);
                    range_data.strikethr = static_cast<BOOL>(
                        LongUI::AtoI(reinterpret_cast<char*>(param_buffer))
                        );
                }
                range_data.range_type = R::E;
                stack_check.push(range_data);
                break;
            case 'H': case 'h': // stretc[H]
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.stretch = va_arg(ap, DWRITE_FONT_STRETCH);
                }
                else {
                    CUIManager_GetNextTokenA(1);
                    range_data.stretch = static_cast<DWRITE_FONT_STRETCH>(
                        LongUI::AtoI(reinterpret_cast<char*>(param_buffer))
                        );
                }
                range_data.range_type = R::H;
                stack_check.push(range_data);
                break;
            case 'I': case 'i': // [I]nline object
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.inlineobj = va_arg(ap, IDWriteInlineObject*);
                }
                else {
                    CUIManager_GetNextTokenW(1);
                    IDWriteInlineObject* result = nullptr;
                    if (config.inline_handler) {
                        result = static_cast<IDWriteInlineObject*>(
                            config.inline_handler(0, param_buffer)
                            );
                    }
                    range_data.inlineobj = result;
                }
                range_data.range_type = R::I;
                stack_check.push(range_data);
                break;
            case 'N': case 'n': // family [N]ame
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.name = va_arg(ap, const wchar_t*);
                }
                else {
                    // 复制字体名称 并去除前后空白
                    register wchar_t now_ch;
                    auto param_buffer_index = param_buffer;
                    wchar_t* last_firststart_while = nullptr;
                    const wchar_t* firststart_notwhile = nullptr;
                    bool nameless = true;
                    while (now_ch = *param_buffer) {
                        *fontname_buffer_index = now_ch;
                        if (nameless && (now_ch == L' ' || now_ch == L'\t')) {
                            last_firststart_while = fontname_buffer_index;
                            nameless = false;
                        }
                        else {
                            nameless = true;
                            if (!firststart_notwhile) {
                                param_buffer_index = fontname_buffer_index;
                            }
                        }
                        ++fontname_buffer_index;
                    }
                    *last_firststart_while = 0;
                    fontname_buffer_index = last_firststart_while + 1;
                    range_data.name = firststart_notwhile;
                }
                range_data.range_type = R::N;
                stack_check.push(range_data);
                break;
            case 'S': case 's': // [S]ize
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.size = va_arg(ap, float);
                }
                else {
                    CUIManager_GetNextTokenA(1);
                    range_data.size = LongUI::AtoF(
                        reinterpret_cast<char*>(param_buffer)
                        );
                }
                range_data.range_type = R::S;
                stack_check.push(range_data);
                break;
            case 'U': case 'u': // [U]nderline
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.underline = va_arg(ap, BOOL);
                }
                else {
                    CUIManager_GetNextTokenA(1);
                    range_data.underline = static_cast<BOOL>(
                        LongUI::AtoI(reinterpret_cast<char*>(param_buffer))
                        );
                }
                range_data.range_type = R::U;
                stack_check.push(range_data);
                break;
            case 'W': case 'w': // [W]eight
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.weight = va_arg(ap, DWRITE_FONT_WEIGHT);
                }
                else {
                    CUIManager_GetNextTokenA(1);
                    range_data.weight = static_cast<DWRITE_FONT_WEIGHT>(
                        LongUI::AtoI(reinterpret_cast<char*>(param_buffer))
                        );
                }
                range_data.range_type = R::W;
                stack_check.push(range_data);
                break;
            case L'Y': case L'y': // st[Y]le
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.style = va_arg(ap, DWRITE_FONT_STYLE);
                }
                else {
                    CUIManager_GetNextTokenA(1);
                    range_data.style = static_cast<DWRITE_FONT_STYLE>(
                        LongUI::AtoI(reinterpret_cast<char*>(param_buffer))
                        );
                }
                range_data.range_type = R::Y;
                stack_check.push(range_data);
                break;
            case L'P': case L'p': // end of main string, then, is the param
                goto force_break;
            case L']': case L'}': // All Range type end
                // 检查栈弹出
                stack_check.pop();
                // 计算长度
                stack_check.top->range.length = string_length - stack_check.top->range.startPosition;
                // 压入设置栈
                statck_set.push(*stack_check.top);
                break;
            }
        }
        // 添加
        else {
            *buffer_index = ch;
            ++buffer_index;
            ++string_length;
        }
        ++format;
    }
force_break:
    // 尾巴0
    *buffer_index = 0;
    // 计算长度
    assert(string_length < lengthof(buffer));
    // 计算需要长度
    config.text_length = string_length;
    register auto string_length_need = static_cast<uint32_t>(static_cast<float>(string_length + 1) * config.progress);
    LongUIClamp(string_length_need, 0, string_length);
    // 修正
    va_end(ap);
    // 创建布局
    if (config.dw_factory && SUCCEEDED(config.dw_factory->CreateTextLayout(
        buffer, string_length_need, config.text_format, config.width, config.height,&layout
        ))) {
        // 创建
        while (!statck_set.empty()) {
            statck_set.pop();
            // 检查进度(progress)范围 释放数据
            if (statck_set.top->range.startPosition 
                + statck_set.top->range.length > string_length_need) {
                if (statck_set.top->range_type == R::D || statck_set.top->range_type == R::I) {
                    ::SafeRelease(statck_set.top->draweffect);
                }
                continue;
            };
            // enum class R :size_t { N, W, Y, H, S, U, E, D, I };
            switch (statck_set.top->range_type)
            {
            case R::N:
                layout->SetFontFamilyName(statck_set.top->name, statck_set.top->range);
                break;
            case R::W:
                layout->SetFontWeight(statck_set.top->weight, statck_set.top->range);
                break;
            case R::Y:
                layout->SetFontStyle(statck_set.top->style, statck_set.top->range);
                break;
            case R::H:
                layout->SetFontStretch(statck_set.top->stretch, statck_set.top->range);
                break;
            case R::S:
                layout->SetFontSize(statck_set.top->size, statck_set.top->range);
                break;
            case R::U:
                layout->SetUnderline(statck_set.top->underline, statck_set.top->range);
                break;
            case R::E:
                layout->SetStrikethrough(statck_set.top->strikethr, statck_set.top->range);
                break;
            case R::D:
                layout->SetDrawingEffect(statck_set.top->draweffect, statck_set.top->range);
                break;
            case R::I:
                layout->SetInlineObject(statck_set.top->inlineobj, statck_set.top->range);
                break;
            }
        }
    }
    return layout;
}


// 从文件读取位图
auto LongUI::CUIManager::LoadBitmapFromFile(
    LongUIRenderTarget *pRenderTarget,
    IWICImagingFactory *pIWICFactory,
    PCWSTR uri,
    UINT destinationWidth,
    UINT destinationHeight,
    ID2D1Bitmap1 **ppBitmap
    ) noexcept -> HRESULT {
    IWICBitmapDecoder *pDecoder = nullptr;
    IWICBitmapFrameDecode *pSource = nullptr;
    IWICStream *pStream = nullptr;
    IWICFormatConverter *pConverter = nullptr;
    IWICBitmapScaler *pScaler = nullptr;

    register HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
        uri,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &pDecoder
        );

    if (SUCCEEDED(hr)) {
        hr = pDecoder->GetFrame(0, &pSource);
    }
    if (SUCCEEDED(hr)) {
        hr = pIWICFactory->CreateFormatConverter(&pConverter);
    }


    if (SUCCEEDED(hr)) {
        if (destinationWidth != 0 || destinationHeight != 0)  {
            UINT originalWidth, originalHeight;
            hr = pSource->GetSize(&originalWidth, &originalHeight);
            if (SUCCEEDED(hr)) {
                if (destinationWidth == 0) {
                    FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
                    destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
                }
                else if (destinationHeight == 0) {
                    FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
                    destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
                }

                hr = pIWICFactory->CreateBitmapScaler(&pScaler);
                if (SUCCEEDED(hr)) {
                    hr = pScaler->Initialize(
                        pSource,
                        destinationWidth,
                        destinationHeight,
                        WICBitmapInterpolationModeCubic
                        );
                }
                if (SUCCEEDED(hr)) {
                    hr = pConverter->Initialize(
                        pScaler,
                        GUID_WICPixelFormat32bppPBGRA,
                        WICBitmapDitherTypeNone,
                        nullptr,
                        0.f,
                        WICBitmapPaletteTypeMedianCut
                        );
                }
            }
        }
        else
        {
            hr = pConverter->Initialize(
                pSource,
                GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone,
                nullptr,
                0.f,
                WICBitmapPaletteTypeMedianCut
                );
        }
    }
    if (SUCCEEDED(hr))
    {
        hr = pRenderTarget->CreateBitmapFromWicBitmap(
            pConverter,
            nullptr,
            ppBitmap
            );
    }

    ::SafeRelease(pDecoder);
    ::SafeRelease(pSource);
    ::SafeRelease(pStream);
    ::SafeRelease(pConverter);
    ::SafeRelease(pScaler);

    return hr;
}


// 添加窗口
void LongUI::CUIManager::AddWindow(UIWindow * wnd) noexcept {
    assert(wnd && "bad argument");
#ifdef _DEBUG
    if (std::find(m_windows.cbegin(), m_windows.cend(), wnd) != m_windows.cend()) {
        assert(!"target window has been added.");
    }
#endif
    try {
        m_windows.push_back(wnd);
    }
    catch (...) {
    }
}

// 移出窗口
void LongUI::CUIManager::RemoveWindow(UIWindow * wnd) noexcept {
    assert(wnd && "bad argument");
#ifdef _DEBUG
    if (std::find(m_windows.cbegin(), m_windows.cend(), wnd) == m_windows.cend()) {
        assert(!"target window not in windows vector");
    }
#endif
    try {
        m_windows.erase(std::find(m_windows.cbegin(), m_windows.cend(), wnd));
    }
    catch (...) {
    }
}

// 是否以管理员权限运行
bool LongUI::CUIManager::IsRunAsAdministrator() noexcept {
    BOOL fIsRunAsAdmin = FALSE;
    PSID pAdministratorsGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    ::AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &pAdministratorsGroup
    );
    // 成功? 清理资源
    if (pAdministratorsGroup) {
        ::CheckTokenMembership(nullptr, pAdministratorsGroup, &fIsRunAsAdmin);
        ::FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = nullptr;
    }
    // 返回结果
    return fIsRunAsAdmin != 0;
}

// 提升权限
bool LongUI::CUIManager::TryElevateUACNow(const wchar_t* parameters, bool exit) noexcept {
    if (!CUIManager::IsRunAsAdministrator()) {
        wchar_t szPath[MAX_PATH];
        // 获取实例句柄
        if (::GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath))) {
            // Launch itself as admin
            SHELLEXECUTEINFO sei = { 0 };
            sei.cbSize = sizeof(sei);
            sei.lpVerb = L"runas";
            sei.lpFile = szPath;
            sei.lpParameters = parameters;
            sei.hwnd = nullptr;
            sei.nShow = SW_NORMAL;
            // 执行
            if (!::ShellExecuteExW(&sei)) {
                DWORD dwError = ::GetLastError();
                assert(dwError == ERROR_CANCELLED && "anyelse?");
                return false;
            }
            else if(exit) {
                // 退出
                UIManager.Exit();
            }
        }
    }
    return true;
}

//#include <valarray>

#ifdef _DEBUG

// 换行刷新重载
auto LongUI::CUIManager::operator<<(const LongUI::EndL) noexcept ->CUIManager& {
    wchar_t chs[3] = { L'\r',L'\n', 0 }; 
    this->Output(m_lastLevel, chs);
    return *this;
}

auto LongUI::CUIManager::operator<<(const DXGI_ADAPTER_DESC& desc) noexcept->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    ::swprintf(
        buffer, LongUIStringBufferLength,
        L"Adapter:   { \n\t Description: %ls\n\t VendorId:0x%08X"
        L"\t\t DeviceId:0x%08X\n\t SubSysId:0x%08X\t\t Revision:0x%08X\n"
        L"\t DedicatedVideoMemory: %.3lfMB\n"
        L"\t DedicatedSystemMemory: %.3lfMB\n"
        L"\t SharedSystemMemory: %.3lfMB\n"
        L"\t AdapterLuid: 0x%08X--%08X\n }",
        desc.Description,
        desc.VendorId,
        desc.DeviceId,
        desc.SubSysId,
        desc.Revision,
        static_cast<double>(desc.DedicatedVideoMemory) / (1024.*1024.),
        static_cast<double>(desc.DedicatedSystemMemory) / (1024.*1024.),
        static_cast<double>(desc.SharedSystemMemory) / (1024.*1024.),
        desc.AdapterLuid.HighPart,
        desc.AdapterLuid.LowPart
        );
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

auto LongUI::CUIManager::operator<<(const RectLTWH_F& rect) noexcept->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    ::swprintf(
        buffer, LongUIStringBufferLength,
        L"RECT_WH(%f, %f, %f, %f)",
        rect.left, rect.top, rect.width, rect.height
        );
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

auto LongUI::CUIManager::operator<<(const D2D1_RECT_F& rect) noexcept->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    ::swprintf(
        buffer, LongUIStringBufferLength,
        L"RECT_RB(%f, %f, %f, %f)",
        rect.left, rect.top, rect.right, rect.bottom
        );
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

auto LongUI::CUIManager::operator<<(const D2D1_POINT_2F& pt) noexcept->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    ::swprintf(
        buffer, LongUIStringBufferLength,
        L"POINT(%f, %f)",
        pt.x, pt.y
        );
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

// 输出UTF-8字符串 并刷新
void LongUI::CUIManager::Output(DebugStringLevel l, const char * s) noexcept {
    wchar_t buffer[LongUIStringBufferLength];
    buffer[LongUI::UTF8toWideChar(s, buffer)] = 0;
    this->Output(m_lastLevel, buffer);
}

// 输出UTF-8字符串
void LongUI::CUIManager::OutputNoFlush(DebugStringLevel l, const char * s) noexcept {
    wchar_t buffer[LongUIStringBufferLength];
    buffer[LongUI::UTF8toWideChar(s, buffer)] = 0;
    this->OutputNoFlush(m_lastLevel, buffer);
}

// 浮点重载
auto LongUI::CUIManager::operator<<(const float f) noexcept ->CUIManager&  {
    wchar_t buffer[LongUIStringBufferLength];
    ::swprintf(buffer, LongUIStringBufferLength, L"%f", f);
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

// 整型重载
auto LongUI::CUIManager::operator<<(const long l) noexcept ->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    ::swprintf(buffer, LongUIStringBufferLength, L"%d", l);
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}


// 调试输出
#define OutputDebug(a, b)\
void LongUI::CUIManager::a(const wchar_t* format, ...) noexcept {\
    wchar_t buffer[LongUIStringBufferLength];\
    va_list argList;\
    va_start(argList, format);\
    auto ret = ::vswprintf(buffer, LongUIStringBufferLength - 1, format, argList);\
    buffer[ret] = 0;\
    va_end(argList);\
    this->Output(b, buffer);\
}

OutputDebug(OutputN, DLevel_None)
OutputDebug(OutputL, DLevel_Log)
OutputDebug(OutputH, DLevel_Hint)
OutputDebug(OutputW, DLevel_Warning)
OutputDebug(OutputE, DLevel_Error)
OutputDebug(OutputF, DLevel_Fatal)

#endif