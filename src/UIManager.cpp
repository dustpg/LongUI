#include "LongUI.h"

#define LONGUI_D3D_DEBUG
//#define LONGUI_RENDER_IN_STD_THREAD

// CUIManager 初始化
auto LongUI::CUIManager::Initialize(IUIConfigure* config) noexcept ->HRESULT {
    // 检查GUID
#if defined(_DEBUG) && defined(_MSC_VER)
#define CHECK_GUID(x)  assert(LongUI::IID_##x == __uuidof(x) && "bad guid")
    CHECK_GUID(IDWriteTextRenderer);
    CHECK_GUID(IDWriteInlineObject);
    CHECK_GUID(IDWriteFactory1);
    CHECK_GUID(IDWriteFontCollection);
    CHECK_GUID(IDWriteFontFileEnumerator);
    CHECK_GUID(IDWriteFontCollectionLoader);
    CHECK_GUID(IDXGISwapChain2);
#undef CHECK_GUID
#endif
    m_szLocaleName[0] = L'\0';
    m_vDelayCleanup.reserve(100);
    std::memset(m_apWindows, 0, sizeof(m_apWindows));
    // 开始计时
    m_uiTimer.Start();
    this->RefreshDisplayFrequency();
    // 检查
    if (!config) {
#ifdef LONGUI_WITH_DEFAULT_CONFIG
        config = &m_config;
#else
        return E_INVALIDARG;
#endif
    }
    // 获取信息
    force_cast(this->configure) = config;
    // 获取flag
    this->flag = this->configure->GetConfigureFlag();
    // 获取资源加载器
    config->CreateInterface(LongUI_IID_PV_ARGS(m_pResourceLoader));
    // 获取脚本
    config->CreateInterface(LongUI_IID_PV_ARGS(force_cast(this->script)));
    // 本地字符集名称
    config->GetLocaleName(m_szLocaleName);
    // 初始化其他
    std::memset(m_apTextRenderer, 0, sizeof(m_apTextRenderer));
    std::memset(m_apSystemBrushes, 0, sizeof(m_apSystemBrushes));
    // 获取实例句柄
    //auto hInstance = ::GetModuleHandleW(nullptr);
    HRESULT hr = S_OK;
    // 位图缓存
    if (SUCCEEDED(hr)) {
        m_pBitmap0Buffer = reinterpret_cast<uint8_t*>(LongUI::NormalAlloc(
            sizeof(RGBQUAD) * LongUIDefaultBitmapSize * LongUIDefaultBitmapSize)
            );
        // 内存不足
        if (!m_pBitmap0Buffer) {
            hr = E_OUTOFMEMORY;
        }
    }
    // 加载控件模板
    if (SUCCEEDED(hr)) {
        m_cCountCtrlTemplate = 1;
        hr = this->load_control_template_string(this->configure->GetTemplateString());
        longui_debug_hr(hr, L"load_control_template_string faild");
    }
    // 资源数据缓存
    if (SUCCEEDED(hr)) {
        // 获取缓存数据
        auto get_buffer_length = [this]() {
            size_t buffer_length =
                sizeof(void*) * m_cCountBmp +
                sizeof(void*) * m_cCountBrs +
                sizeof(void*) * m_cCountTf +
                sizeof(pugi::xml_node) * m_cCountCtrlTemplate +
                (sizeof(HICON) + sizeof(LongUI::Meta)) * m_cCountMt;
            return buffer_length;
        };
        m_cCountBmp = m_cCountBrs = m_cCountTf = m_cCountMt = 1;
        // 检查资源缓存
        if (!m_pResourceBuffer) {
            // 查询资源数量
            if (m_pResourceLoader) {
                m_cCountBmp += static_cast<decltype(m_cCountBmp)>(m_pResourceLoader->GetResourceCount(IUIResourceLoader::Type_Bitmap));
                m_cCountBrs += static_cast<decltype(m_cCountBrs)>(m_pResourceLoader->GetResourceCount(IUIResourceLoader::Type_Brush));
                m_cCountTf += static_cast<decltype(m_cCountTf)>(m_pResourceLoader->GetResourceCount(IUIResourceLoader::Type_TextFormat));
                m_cCountMt += static_cast<decltype(m_cCountMt)>(m_pResourceLoader->GetResourceCount(IUIResourceLoader::Type_Meta));
            }
            // 申请内存
            m_pResourceBuffer = LongUI::NormalAlloc(get_buffer_length());
        }
        // 修改资源
        if (m_pResourceBuffer) {
            std::memset(m_pResourceBuffer, 0, get_buffer_length());
            m_ppBitmaps = reinterpret_cast<decltype(m_ppBitmaps)>(m_pResourceBuffer);
            m_ppBrushes = reinterpret_cast<decltype(m_ppBrushes)>(m_ppBitmaps + m_cCountBmp);
            m_ppTextFormats = reinterpret_cast<decltype(m_ppTextFormats)>(m_ppBrushes + m_cCountBrs);
            m_pMetasBuffer = reinterpret_cast<decltype(m_pMetasBuffer)>(m_ppTextFormats + m_cCountTf);
            m_phMetaIcon = reinterpret_cast<decltype(m_phMetaIcon)>(m_pMetasBuffer + m_cCountMt);
            m_pTemplateNodes = reinterpret_cast<decltype(m_pTemplateNodes)>(m_phMetaIcon + m_cCountMt);
            // 初始化
            for (auto itr = m_pTemplateNodes; itr < m_pTemplateNodes + m_cCountCtrlTemplate; ++itr) {
                LongUI::CreateObject(*itr);
            }
        }
        // 内存不足
        else {
            hr = E_OUTOFMEMORY;
        }
    }
    // 设置控件模板
    if (SUCCEEDED(hr)) {
        hr = this->set_control_template_string();
        longui_debug_hr(hr, L"set_control_template_string faild");
    }
    // 创建D2D工厂
    if (SUCCEEDED(hr)) {
        D2D1_FACTORY_OPTIONS options = { D2D1_DEBUG_LEVEL_NONE };
#ifdef _DEBUG
        options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
        hr = LongUI::Dll::D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            IID_ID2D1Factory4,
            &options,
            reinterpret_cast<void**>(&m_pd2dFactory)
            );
        longui_debug_hr(hr, L"D2D1CreateFactory faild");
    }
    // 创建TSF线程管理器
    if (SUCCEEDED(hr)) {
        hr = ::CoCreateInstance(
            CLSID_TF_ThreadMgr,
            nullptr,
            CLSCTX_INPROC_SERVER,
            LongUI_IID_PV_ARGS(m_pTsfThreadManager)
            );
        longui_debug_hr(hr, L"CoCreateInstance CLSID_TF_ThreadMgr faild");
    }
    // 激活客户ID
    if (SUCCEEDED(hr)) {
        hr = m_pTsfThreadManager->Activate(&m_idTsfClient);
        longui_debug_hr(hr, L"m_pTsfThreadManager->Activate faild");
    }
    // 创建 DirectWrite 工厂.
    if (SUCCEEDED(hr)) {
        hr = LongUI::Dll::DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_ISOLATED,
            LongUI_IID_PV_ARGS_Ex(m_pDWriteFactory)
            );
        longui_debug_hr(hr, L"DWriteCreateFactory faild");
    }
    // 创建帮助器
    if (SUCCEEDED(hr)) {
        hr = ::CoCreateInstance(
            CLSID_DragDropHelper,
            nullptr,
            CLSCTX_INPROC_SERVER,
            LongUI_IID_PV_ARGS(m_pDropTargetHelper)
            );
        longui_debug_hr(hr, L"CoCreateInstance CLSID_DragDropHelper faild");
    }
    // 创建字体集
    if (SUCCEEDED(hr)) {
        // 获取脚本
        config->CreateInterface(LongUI_IID_PV_ARGS(m_pFontCollection));
        // 失败获取系统字体集
        if (!m_pFontCollection) {
            hr = m_pDWriteFactory->GetSystemFontCollection(&m_pFontCollection);
            longui_debug_hr(hr, L"m_pDWriteFactory->GetSystemFontCollection faild");
        }
    }
#ifdef _DEBUG
    // 枚举字体
    if (SUCCEEDED(hr) && (this->flag & IUIConfigure::Flag_DbgOutputFontFamily)) {
        auto count = m_pFontCollection->GetFontFamilyCount();
        UIManager << DL_Log << "Font found: " << long(count) << L"\r\n";
        // 遍历所有字体
        for (auto i = 0u; i < count; ++i) {
            IDWriteFontFamily* family = nullptr;
            // 获取字体信息
            if (SUCCEEDED(m_pFontCollection->GetFontFamily(i, &family))) {
                IDWriteLocalizedStrings* string = nullptr;
                // 获取字体名称
                if (SUCCEEDED(family->GetFamilyNames(&string))) {
                    wchar_t buffer[LongUIStringBufferLength];
                    auto tc = string->GetCount();
                    UIManager << DLevel_Log << Formated(L"%4d[%d]: ", int(i), int(tc));
                    // 遍历所有字体名称
#if 0
                    for (auto j = 0u; j < 1u; j++) {
                        string->GetLocaleName(j, buffer, LongUIStringBufferLength);
                        UIManager << DLevel_Log << buffer << " => ";
                        // 有些语言在我的机器上显示不了(比如韩语), 会出现bug略过不少东西, 就显示第一个了
                        string->GetString(j, buffer, LongUIStringBufferLength);
                        UIManager << DLevel_Log << buffer << "; ";
                    }
#else
                    // 显示第一个
                    string->GetLocaleName(0, buffer, LongUIStringBufferLength);
                    UIManager << DLevel_Log << buffer << " => ";
                    string->GetString(0, buffer, LongUIStringBufferLength);
                    UIManager << DLevel_Log << buffer << ";\r\n";
#endif
                }
                LongUI::SafeRelease(string);
            }
            LongUI::SafeRelease(family);
        }
        // 刷新
        UIManager << DL_Log << LongUI::endl;
    }
#endif
    // 注册渲染器
    if (SUCCEEDED(hr)) {
        // 普通渲染器
        if (this->RegisterTextRenderer(&m_normalTRenderer, "normal") != Type_NormalTextRenderer) {
            assert(!"Type_NormalTextRenderer");
            hr = E_FAIL;
        }
    }
    // 添加控件
    if (SUCCEEDED(hr)) {
        // 添加默认控件创建函数
        this->RegisterControlClass(CreateNullControl, "Null");
        this->RegisterControlClass(UIText::CreateControl, "Text");
        this->RegisterControlClass(UIList::CreateControl, "List");
        this->RegisterControlClass(UIPage::CreateControl, "Page");
        this->RegisterControlClass(UISlider::CreateControl, "Slider");
        this->RegisterControlClass(UIButton::CreateControl, "Button");
        this->RegisterControlClass(UISingle::CreateControl, "Single");
        this->RegisterControlClass(UIListLine::CreateControl, "ListLine");
        this->RegisterControlClass(UICheckBox::CreateControl, "CheckBox");
        this->RegisterControlClass(UIComboBox::CreateControl, "ComboBox");
        this->RegisterControlClass(UIRichEdit::CreateControl, "RichEdit");
        this->RegisterControlClass(UIEditBasic::CreateControl, "Edit");
        this->RegisterControlClass(UIListHeader::CreateControl, "ListHeader");
        this->RegisterControlClass(UIScrollBarA::CreateControl, "ScrollBarA");
        this->RegisterControlClass(UIScrollBarB::CreateControl, "ScrollBarB");
        this->RegisterControlClass(UIRadioButton::CreateControl, "RadioButton");
        this->RegisterControlClass(UIFloatLayout::CreateControl, "FloatLayout");
        this->RegisterControlClass(UIVerticalLayout::CreateControl, "VerticalLayout");
        this->RegisterControlClass(UIHorizontalLayout::CreateControl, "HorizontalLayout");
        // 添加自定义控件
        config->RegisterSome();
    }
    // 创建资源
    if (SUCCEEDED(hr)) {
        hr = this->RecreateResources();
        longui_debug_hr(hr, L"RecreateResources faild");
    }
    // 初始化事件
    if (SUCCEEDED(hr)) {
        this->do_creating_event(LongUI::CreateEventType::Type_Initialize);
        longui_debug_hr(hr, L"do_creating_event(init) faild");
    }
    // 检查错误
    else {
        this->ShowError(hr);
    }
    // 检查当前路径
#ifdef _DEBUG
    wchar_t buffer[MAX_PATH * 4]; buffer[0] = 0;
    ::GetCurrentDirectoryW(lengthof<uint32_t>(buffer), buffer);
    UIManager << DL_Log << L" Current Directory: " << buffer << LongUI::endl;
#endif
    return hr;
}


// CUIManager  反初始化
void LongUI::CUIManager::Uninitialize() noexcept {
    // 反初始化事件
    this->do_creating_event(LongUI::CreateEventType::Type_Uninitialize);
    // 释放文本渲染器
    for (auto& renderer : m_apTextRenderer) {
        LongUI::SafeRelease(renderer);
    }
    // 释放公共设备无关资源
    {
        // 释放文本格式
        for (auto itr = m_ppTextFormats; itr != m_ppTextFormats + m_cCountTf; ++itr) {
            LongUI::SafeRelease(*itr);
        }
        // 摧毁META图标
        for (auto itr = m_phMetaIcon; itr != m_phMetaIcon + m_cCountMt; ++itr) {
            if (*itr) {
                ::DestroyIcon(*itr);
                *itr = nullptr;
            }
        }
        // 控件模板
        for (auto itr = m_pTemplateNodes; itr != m_pTemplateNodes + m_cCountCtrlTemplate; ++itr) {
            LongUI::DestoryObject(*itr);
        }
    }
    // 释放资源
    LongUI::SafeRelease(m_pFontCollection);
    LongUI::SafeRelease(m_pDWriteFactory);
    LongUI::SafeRelease(m_pDropTargetHelper);
    LongUI::SafeRelease(m_pd2dFactory);
    LongUI::SafeRelease(m_pTsfThreadManager);
    // 释放脚本
    LongUI::SafeRelease(force_cast(script));
    // 释放读取器
    LongUI::SafeRelease(m_pResourceLoader);
    // 释放设备相关资源
    this->discard_resources();
    // 释放内存
    if (m_pBitmap0Buffer) {
        LongUI::NormalFree(m_pBitmap0Buffer);
        m_pBitmap0Buffer = nullptr;
    }
    // 释放资源缓存
    if (m_pResourceBuffer) {
        LongUI::NormalFree(m_pResourceBuffer);
        m_pResourceBuffer = nullptr;
    }
    m_cCountMt = m_cCountTf = m_cCountBmp = m_cCountBrs = 0;
    // 清理
    m_hashStr2CreateFunc.Clear();
#ifdef _DEBUG
    long time = ::timeGetTime() - m_dbgExitTime;
    UIManager << DL_Log
        << L" took " << time << L"ms to exit here"
        << LongUI::endl;
#endif // _DEBUG
    // 释放配置
    LongUI::SafeRelease(force_cast(this->configure));
}

// 创建事件
void LongUI::CUIManager::do_creating_event(CreateEventType type) noexcept {
    // 类型断言
    assert(type < LongUI::TypeGreater_CreateControl_ReinterpretParentPointer);
    assert(type > Type_CreateControl_NullParentPointer);
    // 遍历hash表
    m_hashStr2CreateFunc.ForEach([type](StringTable::Unit* unit) noexcept {
        assert(unit);
        auto func = reinterpret_cast<CreateControlFunction>(unit->value);
        func(type, pugi::xml_node(nullptr));
    });
}

// CUIManager 创建控件树
void LongUI::CUIManager::MakeControlTree(UIContainer* root, pugi::xml_node node) noexcept {
    // 断言
    assert(root && node && "bad argument");
    // 队列 -- 顺序遍历树
    EzContainer::FixedCirQueue<pugi::xml_node, LongUIMaxControlInited> xml_queue;
    EzContainer::FixedCirQueue<UIContainer*, LongUIMaxControlInited> parents_queue;
    UIControl* control = nullptr;
    UIContainer* parent = root;
    // 遍历算法: 1.压入所有子结点 2.依次弹出 3.重复1
    bool noskip = true;
    while (true) {
        // 压入/入队 所有子结点
        node = node.first_child();
        while (noskip && node) {
            xml_queue.Push(node);
            parents_queue.Push(parent);
            node = node.next_sibling();
        }
        noskip = true;
        // 为空则退出
        if (xml_queue.IsEmpty()) break;
        // 弹出/出队 第一个结点
        node = xml_queue.Front();  xml_queue.Pop();
        parent = parents_queue.Front(); parents_queue.Pop();
        assert(node && "bad xml node");
        // 根据名称创建控件
        if (!(control = this->CreateControl(parent, node, nullptr))) {
            // 错误
            parent = nullptr;
            UIManager << DL_Error
                << L" control class not found: "
                << node.name()
                << L".or OOM"
                << LongUI::endl;
            continue;
        }
        // 添加子结点
        parent->Push(control);
        // 去除引用
        control->Release();
        // 设置结点为下次父结点
        parent = static_cast<UIContainer*>(control);
        // 不是容器的话直接跳过
        noskip = !!(parent->flags & Flag_UIContainer);
    }
}

// 获取创建控件函数指针
auto LongUI::CUIManager::GetCreateFunc(const char* clname) noexcept -> CreateControlFunction {
    // 检查 !white_space(clname[0]) && 
    assert(clname && clname[0] && "bad argment");
    // 查找
    auto result = m_hashStr2CreateFunc.Find(clname);
    // 检查
    assert(result && "404 not found");
    // 返回
    return reinterpret_cast<CreateControlFunction>(*result);
}

// 创建文本格式
auto LongUI::CUIManager::CreateTextFormat(
    WCHAR const * fontFamilyName, 
    DWRITE_FONT_WEIGHT fontWeight, 
    DWRITE_FONT_STYLE fontStyle,
    DWRITE_FONT_STRETCH fontStretch, 
    FLOAT fontSize, 
    IDWriteTextFormat ** textFormat) noexcept -> HRESULT {
    auto hr = S_OK;
#ifdef _DEBUG
    // 检查字体名称
    UINT32 index = 0; BOOL exist = FALSE;
    hr = m_pFontCollection->FindFamilyName(fontFamilyName, &index, &exist);
    if (SUCCEEDED(hr)) {
        // 字体不存在, 则给予警告
        if (!exist) {
            UIManager << DL_Hint
                << Formated(L"font family(%ls) not found", fontFamilyName)
                << LongUI::endl;
            int bk; bk = 9;
        }
    }
    longui_debug_hr(hr, L"m_pFontCollection->FindFamilyName failed");
#endif
    // 创建文本格式
    hr = m_pDWriteFactory->CreateTextFormat(
        fontFamilyName,
        m_pFontCollection,
        fontWeight,
        fontStyle,
        fontStretch,
        fontSize,
        m_szLocaleName,
        textFormat
        );
    // 检查错误
    longui_debug_hr(hr, Formated(L"CreateTextFormat(%ls) faild", fontFamilyName));
    return hr;
}


#ifdef _DEBUG
void LongUI::CUIManager::Exit() noexcept {
    m_dbgExitTime = ::timeGetTime();
    UIManager << DL_Log << L" CALLED" << LongUI::endl;
    this->exit();
}
#endif

#ifdef LONGUI_RENDER_IN_STD_THREAD
#include <thread>
#else
#include <process.h>
#endif

// 消息循环
void LongUI::CUIManager::Run() noexcept {
    // 开始!
    m_dwWaitVSStartTime = ::timeGetTime();
    // 渲染线程函数
    auto render_thread_func = [](void*) noexcept ->unsigned {
        UIWindow* windows[LongUIMaxWindow];
        HANDLE waitvs[LongUIMaxWindow];
        // 不退出?
        while (!UIManager.m_exitFlag) {
            uint32_t vslen = 0;
            uint32_t wndlen = 0;
            {
                // 数据锁
                CUIDataAutoLocker locker;
#ifdef _DEBUG
                ++UIManager.frame_id;
#endif
                // 延迟清理
                UIManager.cleanup_delay_cleanup_chain();
                // 有窗口
                wndlen = UIManager.m_cCountWindow;
                // 复制数据
                for (auto i = 0u; i < wndlen; ++i) {
                    windows[i] = UIManager.m_apWindows[i];
                }
                // 更新计时器
                UIManager.m_fDeltaTime = UIManager.m_uiTimer.Delta_s<float>();
                UIManager.m_uiTimer.MovStartEnd();
                // 刷新窗口
                for (auto i = 0u; i < wndlen; ++i) {
                    // 刷新
                    windows[i]->Update();
                    // 下一帧
                    windows[i]->NextFrame();
                    // 获取等待垂直同步事件
                    waitvs[vslen] = windows[i]->GetVSyncEvent();
                    // 刷新?
                    if (!windows[i]->IsRendered()) {
                        windows[i] = nullptr;
                    }
                    // 有效
                    else if (waitvs[vslen]) {
                        ++vslen;
                    }
                }
            }
            {
                // 渲染锁
                CUIDxgiAutoLocker locker;
                // 渲染窗口
                for (auto i = 0u; i < wndlen; ++i) {
                    if (windows[i]) {
                        windows[i]->RenderWindow();
                    }
                }
            }
            // 等待垂直同步
            UIManager.WaitVS(waitvs, vslen);
        }
        return 0;
    };
    // 需要std::thread?
#ifdef LONGUI_RENDER_IN_STD_THREAD
    std::thread thread(render_thread_func, nullptr);
#else
    auto thread = reinterpret_cast<HANDLE>(
        ::_beginthreadex(nullptr, 0, render_thread_func, nullptr, 0, nullptr)
        );
    assert(thread && "failed to create thread");
#endif
    // 消息响应
    MSG msg;
    while (::GetMessageW(&msg, nullptr, 0, 0)) {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }
    // 退出
    m_exitFlag = true;
    // 等待线程
#ifdef LONGUI_RENDER_IN_STD_THREAD
    try { if (thread.joinable()) { thread.join(); } }
    catch (...) { }
#else
    if (thread) {
        ::WaitForSingleObject(thread, INFINITE);
        ::CloseHandle(thread);
        thread = nullptr;
    }
#endif
    // 再次清理
    this->cleanup_delay_cleanup_chain();
    // 尝试强行关闭
    /*if (m_cCountWindow) {
        UIWindow* windows[LongUIMaxWindow];
        std::memcpy(windows, m_apWindows, sizeof(m_apWindows));
        auto count = m_cCountWindow;
        // 清理窗口
        for (auto i = 0u; i < count; ++i) {
            windows[count - i - 1]->cleanup();
        }
    }*/
    assert(!m_cCountWindow && "bad");
    m_cCountWindow = 0;
}

// 等待垂直同步
void LongUI::CUIManager::WaitVS(HANDLE events[], uint32_t length) noexcept {
    // 一直刷新
    if (this->flag & IUIConfigure::Flag_RenderInAnytime) 
        return static_cast<void>(::WaitForMultipleObjects(length, events, TRUE, INFINITE));
    // 保留刷新时间点
    auto end_time_of_sleep = m_dwWaitVSStartTime + 
        ((++m_dwWaitVSCount) * 1000ui32) / static_cast<uint16_t>(m_dDisplayFrequency);
    // 等待事件
    if (length) ::WaitForMultipleObjects(length, events, TRUE, INFINITE);
    // 保证等待
    while (::timeGetTime() < end_time_of_sleep) ::Sleep(1);
}


// 利用模板ID创建控件
auto LongUI::CUIManager::CreateControl(UIContainer* cp, size_t templateid, CreateControlFunction function) noexcept ->UIControl* {
    // 检查参数
    assert(function && "function must be specified");
    //assert(templateid && "template id must be specified");
    // 检查模板ID
    assert(templateid < m_cCountCtrlTemplate && "out of range");
    // 越界检查
    if (templateid >= m_cCountCtrlTemplate) templateid = 0;
    // 创建
    return function(cp->GetCET(), m_pTemplateNodes[templateid]);
}

// 利用现有资源创建控件
auto LongUI::CUIManager::create_control(UIContainer* cp, CreateControlFunction function, pugi::xml_node node, size_t tid) noexcept -> UIControl * {
    // TODO: NODE
    assert(node && "call another method if no xml-node");
    // 检查参数 function
    if (!function) {
        function = this->GetCreateFunc(node.name());
    }
    // 结点有效并且没有指定模板ID则尝试获取
    if (!tid) {
        tid = static_cast<decltype(tid)>(LongUI::AtoI(
            node.attribute(LongUI::XMLAttribute::TemplateID).value())
            );
    }
    // 利用id查找模板控件
    if (tid) {
        assert(tid < m_cCountCtrlTemplate && "out of range");
        if (tid >= m_cCountCtrlTemplate) tid = 0;
        // 结点有效->添加属性
        {
            auto attribute = m_pTemplateNodes[tid].first_attribute();
            // 遍历属性
            while (attribute) {
                // 添加属性
                auto name = attribute.name();
                if (!node.attribute(name)) {
                    node.insert_attribute_after(name, node.last_attribute()).set_value(attribute.value());
                }
                // 推进
                attribute = attribute.next_attribute();
            }
        }
    }
    // 检查
    assert(function && "bad idea");
    if (!function) return nullptr;
    auto ctrl = function(cp->GetCET(), node);
    return ctrl;
}

// 创建UI窗口
auto LongUI::CUIManager::create_ui_window(
    pugi::xml_node node,
    UIWindow* parent,
    callback_for_creating_window func,
    void* buf) noexcept -> UIWindow* {
    assert(func && node && "bad argument");
    // 创建窗口
    auto window = func(node, parent, buf);
    // 查错
    assert(window && "OOM or some error");
    // 成功
    if (window) {
#ifdef _DEBUG
        //::Sleep(5000);
        CUITimerH dbg_timer; dbg_timer.Start();
        UIManager << DL_Log << window << LongUI::endl;
#endif
        // 重建资源
        auto hr = window->Recreate();
        ShowHR(hr);
#ifdef _DEBUG
        //::Sleep(5000);
        auto time = dbg_timer.Delta_ms<double>();
        UIManager << DL_Log
            << Formated(L" took %.3lfms for recreate.", time)
            << LongUI::endl;
        dbg_timer.MovStartEnd();
#endif
        // 创建控件树
        this->MakeControlTree(window, node);
        // 完成创建
#ifdef _DEBUG
        time = dbg_timer.Delta_ms<double>();
        UIManager << DL_Log
            << Formated(L" took %.3lfms for making.", time)
            << LongUI::endl;
        dbg_timer.MovStartEnd();
#endif
        // 发送消息
        window->DoLongUIEvent(Event::Event_TreeBulidingFinished);
#ifdef _DEBUG
        time = dbg_timer.Delta_ms<double>();
        UIManager << DL_Log
            << Formated(L" took %.3lfms for sending finished event.", time)
            << LongUI::endl;
        //::Sleep(5000);
#endif
            // 返回 
    }
    return window;
}

// 消息转换
void LongUI::CUIManager::WindowsMsgToMouseEvent(MouseEventArgument& arg, 
    UINT message, WPARAM wParam, LPARAM lParam) noexcept {
    // 范围检查
    assert((message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) || 
        message == WM_MOUSELEAVE || message == WM_MOUSEHOVER);
    // 获取X
    auto get_x = [lParam]() { return float(int16_t(LOWORD(lParam))); };
    // 获取Y
    auto get_y = [lParam]() { return float(int16_t(HIWORD(lParam))); };
    // 设置
    arg.sys.wParam = wParam;
    arg.sys.lParam = lParam;
    arg.pt.x = get_x();
    arg.pt.y = get_y();
    arg.last = nullptr;
    MouseEvent me;
    // 检查信息
    switch (message)
    {
    case WM_MOUSEMOVE:
        me = MouseEvent::Event_MouseMove;
        break;
    case WM_LBUTTONDOWN:
        me = MouseEvent::Event_LButtonDown;
        break;
    case WM_LBUTTONUP:
        me = MouseEvent::Event_LButtonUp;
        break;
    case WM_RBUTTONDOWN:
        me = MouseEvent::Event_RButtonDown;
        break;
    case WM_RBUTTONUP:
        me = MouseEvent::Event_RButtonUp;
        break;
    case WM_MBUTTONDOWN:
        me = MouseEvent::Event_MButtonDown;
        break;
    case WM_MBUTTONUP:
        me = MouseEvent::Event_MButtonUp;
        break;
    case WM_MOUSEWHEEL:
        me = MouseEvent::Event_MouseWheelV;
        break;
    case WM_MOUSEHWHEEL:
        me = MouseEvent::Event_MouseWheelH;
        break;
    case WM_MOUSEHOVER:
        me = MouseEvent::Event_MouseHover;
        break;
    case WM_MOUSELEAVE:
        me = MouseEvent::Event_MouseLeave;
        break;
    default:
        me = MouseEvent::Event_None;
        break;
    }
    arg.event = me;
}



// 获取主题颜色
auto LongUI::CUIManager::GetThemeColor(D2D1_COLOR_F& colorf) noexcept -> HRESULT {
    union { DWORD color; uint8_t argb[4]; };
    color = DWORD(-1); auto hr = S_OK; DWORD buffer_size = sizeof(DWORD);
    // 获取Colorization颜色
    ::RegGetValueA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\DWM", "ColorizationColor",
        RRF_RT_DWORD, nullptr, &color, &buffer_size
        );
    DWORD balance = 50; buffer_size = sizeof(DWORD);
    // 获取Colorization混合标准
    ::RegGetValueA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\DWM", "ColorizationColorBalance",
        RRF_RT_DWORD, nullptr, &balance, &buffer_size
        );
    {
        // 混合通道
        auto blend_channel = [](float ch1, float ch2, float prec) {
            auto data = ch1 + (ch2 - ch1) * prec;
            return data > 1.f ? 1.f : (data < 0.f ? 0.f : data);
        };
        colorf.a = 1.f; auto prec = 1.f - float(balance) / 100.f;
        constexpr float basegrey = float(217) / 255.f;
        colorf.r = blend_channel(float(argb[2]) / 255.f, basegrey, prec);
        colorf.g = blend_channel(float(argb[1]) / 255.f, basegrey, prec);
        colorf.b = blend_channel(float(argb[0]) / 255.f, basegrey, prec);
    }
    return hr;
}

// CUIManager 构造函数
LongUI::CUIManager::CUIManager() noexcept : m_config(*this) {

}

// CUIManager 析构函数
LongUI::CUIManager::~CUIManager() noexcept {
    this->discard_resources();
}

// 获取控件 wchar_t指针
auto LongUI::CUIManager::RegisterControlClass(
    CreateControlFunction func, const char* clname) noexcept ->HRESULT {
    if (!clname || !(*clname)) {
        assert(!"bad argument");
        return S_FALSE;
    }
    // 插入
    auto result = m_hashStr2CreateFunc.Insert(m_oStringAllocator.CopyString(clname), func);
    // 插入失败的原因只有一个->OOM
    return result ? S_OK : E_OUTOFMEMORY;
}

// 获取控件 wchar_t指针
void LongUI::CUIManager::UnregisterControlClass(const char* clname) noexcept {
    if (!clname || !(*clname)) {
        assert(!"bad argument");
        return;
    }
    // 移除
    if (m_hashStr2CreateFunc.Find(clname)) {
        m_hashStr2CreateFunc.Remove(clname);
    }
#ifdef _DEBUG
    else {
        UIManager << DL_Error
            << L"class name # "
            << clname
            << L" # not found"
            << LongUI::endl;
        assert(!"class not found");
    }
#endif
}

// 显示错误代码
void LongUI::CUIManager::ShowError(HRESULT hr, const wchar_t* str_b) noexcept {
    wchar_t buffer[LongUIStringBufferLength];
    // 格式化
    if (!::FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,  hr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
        buffer, LongUIStringBufferLength,
        nullptr)) {
        // 处理
        std::swprintf(
            buffer, LongUIStringBufferLength,
            L"Error! HRESULT Code: 0x%08X", hr
            );
    }
    // 错误
    this->ShowError(buffer, str_b);
}

// 注册文本渲染器
auto LongUI::CUIManager::RegisterTextRenderer(
    XUIBasicTextRenderer* renderer, const char name[LongUITextRendererNameMaxLength]
    ) noexcept -> int32_t {
    assert(m_uTextRenderCount < lengthof(m_apTextRenderer) && "buffer too small");
    assert(!white_space(name[0]) && "name cannot begin with white space");
    // 满了
    if (m_uTextRenderCount == lengthof(m_apTextRenderer)) {
        return -1;
    }
    const auto count = m_uTextRenderCount;
    assert((std::strlen(name) + 1) < LongUITextRendererNameMaxLength && "buffer too small");
    std::strcpy(m_aszTextRendererName[count].name, name);
    m_apTextRenderer[count] = LongUI::SafeAcquire(renderer);
    ++m_uTextRenderCount;
    return count;
}

// 创建0索引资源
auto LongUI::CUIManager::create_indexzero_resources() noexcept ->HRESULT {
    assert(m_pResourceBuffer && "bad alloc");
    HRESULT hr = S_OK;
    // 索引0位图: 可MAP位图
    if (SUCCEEDED(hr)) {
        assert(m_ppBitmaps[LongUIDefaultBitmapIndex] == nullptr && "bad action");
        hr = m_pd2dDeviceContext->CreateBitmap(
            D2D1::SizeU(LongUIDefaultBitmapSize, LongUIDefaultBitmapSize),
            nullptr, LongUIDefaultBitmapSize * 4,
            D2D1::BitmapProperties1(
                static_cast<D2D1_BITMAP_OPTIONS>(LongUIDefaultBitmapOptions),
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
                ),
            m_ppBitmaps + LongUIDefaultBitmapIndex
            );
        longui_debug_hr(hr, L"_pd2dDeviceContext->CreateBitmap failed");
    }
    // 索引0笔刷: 全控件共享用前写纯色笔刷
    if (SUCCEEDED(hr)) {
        assert(m_ppBrushes[LongUICommonSolidColorBrushIndex] == nullptr && "bad action");
        ID2D1SolidColorBrush* brush = nullptr;
        D2D1_COLOR_F color = D2D1::ColorF(D2D1::ColorF::Black);
        hr = m_pd2dDeviceContext->CreateSolidColorBrush(&color, nullptr, &brush);
        m_ppBrushes[LongUICommonSolidColorBrushIndex] = LongUI::SafeAcquire(brush);
        LongUI::SafeRelease(brush);
        longui_debug_hr(hr, L"_pd2dDeviceContext->CreateSolidColorBrush failed");
    }
    // 索引0文本格式: 默认格式, 属于设备无关资源，仅需要重建一次
    if (SUCCEEDED(hr) && !(m_ppTextFormats[LongUIDefaultTextFormatIndex])) {
        hr = this->CreateTextFormat(
            LongUIDefaultTextFontName,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            LongUIDefaultTextFontSize,
            //12.f,
            m_ppTextFormats + LongUIDefaultTextFormatIndex
            );
        longui_debug_hr(hr, L"this->CreateTextFormat failed");
    }
    // 设置
    if (SUCCEEDED(hr)) {
        m_ppTextFormats[LongUIDefaultTextFormatIndex]->SetParagraphAlignment(
            DWRITE_PARAGRAPH_ALIGNMENT(LongUIDefaultTextVAlign)
            );
        m_ppTextFormats[LongUIDefaultTextFormatIndex]->SetTextAlignment(
            DWRITE_TEXT_ALIGNMENT(LongUIDefaultTextHAlign)
            );
    }
    // 索引0图元: 暂无
    if (SUCCEEDED(hr)) {

    }
    // 索引0 xml节点
    if (SUCCEEDED(hr) && !(m_pTemplateNodes[0])) {
        m_pTemplateNodes[0] = m_docTemplate.append_child("Control");
    }
    return hr;
}

// 清理延迟清理链
void LongUI::CUIManager::cleanup_delay_cleanup_chain() noexcept {
#if 0
    for (auto ctrl : m_vDelayCleanup) {
        ctrl->Release();
    }
#else
    for (uint32_t i = 1; i <= m_vDelayCleanup.size(); ++i) {
        uint32_t index = m_vDelayCleanup.size() - i;
        m_vDelayCleanup[index]->Release();
    }
#endif
    m_vDelayCleanup.clear();
}

// 载入模板字符串
auto LongUI::CUIManager::load_control_template_string(const char* str) noexcept ->HRESULT {
    // 检查参数
    if (str && *str) {
        // 载入字符串
        auto code = m_docTemplate.load_string(str);
        if (code.status) {
            assert(!"load error");
            ::MessageBoxA(nullptr, code.description(), "<LongUI::CUIManager::load_control_template_string>: Failed to Parse/Load XML", MB_ICONERROR);
            return E_FAIL;
        }
        // 获取子结点数量
        auto get_children_count = [](pugi::xml_node node) {
            node = node.first_child();
            auto count = 0ui16;
            while (node) { node = node.next_sibling(); ++count; }
            return count;
        };
        m_cCountCtrlTemplate = 1 + get_children_count(m_docTemplate.root().first_child());
        // 解析
        return S_OK;
    }
    else {
        return S_FALSE;
    }
}


// 设置模板字符串
auto LongUI::CUIManager::set_control_template_string() noexcept ->HRESULT {
    // 有效情况
    if (m_cCountCtrlTemplate > 1) {
        auto itr = m_pTemplateNodes + 1;
        auto node = m_docTemplate.root().first_child().first_child();
        // 写入索引
        while (node) {
            *itr = node;
            node = node.next_sibling();
            ++itr;
        }
    }
    return S_OK;
}


// UIManager 创建设备相关资源
auto LongUI::CUIManager::create_device_resources() noexcept ->HRESULT {
    // 重新获取flag
    this->flag = this->configure->GetConfigureFlag();
    // 待用适配器
    IDXGIAdapter1* adapter = nullptr;
    // 枚举显示适配器
    if (!(this->flag & IUIConfigure::Flag_RenderByCPU)) {
        IDXGIFactory1* dxgifactory = nullptr;
        // 创建一个临时工程
        if (SUCCEEDED(LongUI::Dll::CreateDXGIFactory1(
            IID_IDXGIFactory1, reinterpret_cast<void**>(&dxgifactory)
            ))) {
            uint32_t adnum = 0;
            IDXGIAdapter1* apAdapters[LongUIMaxAdaptersSize];
            DXGI_ADAPTER_DESC1 descs[LongUIMaxAdaptersSize];
            // 枚举适配器
            for (adnum = 0; adnum < lengthof(apAdapters); ++adnum) {
                if (dxgifactory->EnumAdapters1(adnum, apAdapters + adnum) == DXGI_ERROR_NOT_FOUND) {
                    break;
                }
                apAdapters[adnum]->GetDesc1(descs + adnum);
            }
            // 选择适配器
            auto index = this->configure->ChooseAdapter(descs, adnum);
            if (index < adnum) {
                adapter = LongUI::SafeAcquire(apAdapters[index]);
            }
            // 释放适配器
            for (size_t i = 0; i < adnum; ++i) {
                LongUI::SafeRelease(apAdapters[i]);
            }
        }
        LongUI::SafeRelease(dxgifactory);
    }
    // 创建设备资源
    HRESULT hr /*= m_docResource.Error() ? E_FAIL :*/ S_OK;
    // 创建 D3D11设备与设备上下文 
    if (SUCCEEDED(hr)) {
        // D3D11 创建flag 
        // 一定要有D3D11_CREATE_DEVICE_BGRA_SUPPORT
        // 否则创建D2D设备上下文会失败
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG) && defined(LONGUI_D3D_DEBUG)
        // Debug状态 有D3D DebugLayer就可以取消注释
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
        auto tmpflag = D3D11_CREATE_DEVICE_FLAG(creationFlags);
        tmpflag = D3D11_CREATE_DEVICE_FLAG(0);
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
        // 根据情况检查驱动类型
        D3D_DRIVER_TYPE dtype = (this->flag & IUIConfigure::Flag_RenderByCPU) ? D3D_DRIVER_TYPE_WARP :
            (adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE);
        // 创建设备
        hr = LongUI::Dll::D3D11CreateDevice(
            // 设置为渲染
            adapter,
            // 驱动类型
            dtype,
            // 没有软件接口
            nullptr,
            // 创建flag
            creationFlags,
            // 欲使用的特性等级列表
            featureLevels,
            // 特性等级列表长度
            static_cast<UINT>(lengthof(featureLevels)),
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
            UIManager << DL_Warning 
                << L"create d3d11 device failed, now, try to create in warp mode" 
                << LongUI::endl;
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
                static_cast<UINT>(lengthof(featureLevels)),
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
        // 再次检查错误
        if (FAILED(hr)) {
            UIManager << DL_Error
                << L" create d3d11-device in warp modd, but failed."
                << LongUI::endl;
            this->ShowError(hr);
        }
    }
    LongUI::SafeRelease(adapter);
    // 创建 ID3D11Debug对象
#if defined(_DEBUG) && defined(LONGUI_D3D_DEBUG)
    if (SUCCEEDED(hr)) {
        hr = m_pd3dDevice->QueryInterface(LongUI_IID_PV_ARGS(m_pd3dDebug));
        longui_debug_hr(hr, L"m_pd3dDevice->QueryInterface(m_pd3dDebug) faild");
    }
#endif
    // 创建 IDXGIDevice
    if (SUCCEEDED(hr)) {
        hr = m_pd3dDevice->QueryInterface(LongUI_IID_PV_ARGS(m_pDxgiDevice));
        longui_debug_hr(hr, L"m_pd3dDevice->QueryInterface(m_pd3dDebug) faild");
    }
    // 创建 D2D设备
    if (SUCCEEDED(hr)) {
        hr = m_pd2dFactory->CreateDevice(m_pDxgiDevice, &m_pd2dDevice);
        longui_debug_hr(hr, L"m_pd2dFactory->CreateDevice faild");
    }
    // 创建 D2D设备上下文
    if (SUCCEEDED(hr)) {
        hr = m_pd2dDevice->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            &m_pd2dDeviceContext
            );
        longui_debug_hr(hr, L"m_pd2dDevice->CreateDeviceContext faild");
    }
    // 获取 Dxgi适配器 可以获取该适配器信息
    if (SUCCEEDED(hr)) {
        // 顺带使用像素作为单位
        m_pd2dDeviceContext->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
        // 获取
        hr = m_pDxgiDevice->GetAdapter(&m_pDxgiAdapter);
        longui_debug_hr(hr, L"m_pDxgiDevice->GetAdapter faild");
    }
#ifdef _DEBUG
    // 输出显卡信息
    if (SUCCEEDED(hr)) {
        DXGI_ADAPTER_DESC desc = { 0 }; 
        m_pDxgiAdapter->GetDesc(&desc);
        UIManager << DL_Log << desc << LongUI::endl;
    }
#endif
    // 获取 Dxgi工厂
    if (SUCCEEDED(hr)) {
        hr = m_pDxgiAdapter->GetParent(LongUI_IID_PV_ARGS(m_pDxgiFactory));
    }
#ifdef LONGUI_WITH_MMFVIDEO
    UINT token = 0;
    // 多线程
    if (SUCCEEDED(hr)) {
        ID3D10Multithread* mt = nullptr;
        hr = m_pd3dDevice->QueryInterface(LongUI_IID_PV_ARGS(mt));
        longui_debug_hr(hr, L"m_pd3dDevice->QueryInterface ID3D10Multithread faild");
        // 保护
        if (SUCCEEDED(hr)) {
            mt->SetMultithreadProtected(TRUE);
        }
        LongUI::SafeRelease(mt);
    }
    // 设置 MF
    if (SUCCEEDED(hr)) {
        hr = ::MFStartup(MF_VERSION);
        longui_debug_hr(hr, L"MFStartup faild");
    }
    // 创建 MF Dxgi 设备管理器
    if (SUCCEEDED(hr)) {
        hr = ::MFCreateDXGIDeviceManager(&token, &m_pMFDXGIManager);
        longui_debug_hr(hr, L"MFCreateDXGIDeviceManager faild");
    }
    // 重置设备
    if (SUCCEEDED(hr)) {
        hr = m_pMFDXGIManager->ResetDevice(m_pd3dDevice, token);
        longui_debug_hr(hr, L"m_pMFDXGIManager->ResetDevice faild");
    }
    // 创建 MF媒体类工厂
    if (SUCCEEDED(hr)) {
        hr = ::CoCreateInstance(
            CLSID_MFMediaEngineClassFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            LongUI_IID_PV_ARGS(m_pMediaEngineFactory)
            );
        longui_debug_hr(hr, L"CoCreateInstance CLSID_MFMediaEngineClassFactory faild");
    }
#endif
    // 创建系统笔刷
    if (SUCCEEDED(hr)) {
        hr = this->create_system_brushes();
        longui_debug_hr(hr, L"create_system_brushes faild");
    }
    // 创建资源描述资源
    if (SUCCEEDED(hr)) {
        hr = this->create_indexzero_resources();
        longui_debug_hr(hr, L"create_indexzero_resources faild");
    }
    // 事件
    if (SUCCEEDED(hr)) {
        this->do_creating_event(LongUI::CreateEventType::Type_Recreate);
        longui_debug_hr(hr, L"do_creating_event(recreate) faild");
    }
    // 设置文本渲染器数据
    if (SUCCEEDED(hr)) {
        for (uint32_t i = 0u; i < m_uTextRenderCount; ++i) {
            m_apTextRenderer[i]->SetNewTarget(m_pd2dDeviceContext);
            m_apTextRenderer[i]->SetNewBrush(
                static_cast<ID2D1SolidColorBrush*>(m_ppBrushes[LongUICommonSolidColorBrushIndex])
                );
        }
    }
    // 重建所有窗口
    for (auto itr = m_apWindows; itr < m_apWindows + m_cCountWindow; ++itr) {
        auto wnd = *itr;
        if (SUCCEEDED(hr)) {
            hr = wnd->Recreate();
            longui_debug_hr(hr, wnd << L"wnd->Recreate");
        }
    }
    // 断言 HR
    ShowHR(hr);
    return hr;
}


// 创建系统笔刷
auto LongUI::CUIManager::create_system_brushes() noexcept -> HRESULT {
    HRESULT hr = S_OK;
    /*
    焦点: 0x3399FF 矩形描边, 并且内边有虚线矩形
        0. 禁用: 0xBF灰度 矩形描边; 中心 0xCC灰色
        1. 普通: 0xAC灰度 矩形描边; 中心 从上到下0xF0灰色到0xE5灰色渐变
        2. 移上: 0x7EB4EA 矩形描边; 中心 从上到下0xECF4FC到0xDCECFC渐变
        3. 按下: 0x569DE5 矩形描边; 中心 从上到下0xDAECFC到0xC4E0FC渐变
    */
    // 禁用
    if (SUCCEEDED(hr)) {
        hr = m_pd2dDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(0xCCCCCC),
            reinterpret_cast<ID2D1SolidColorBrush**>(m_apSystemBrushes + State_Disabled)
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
                stops, lengthof<uint32_t>(stops), &collection
                );
        }
        // 创建笔刷
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    D2D1::Point2F(), D2D1::Point2F(0.f, 1.f)
                    ),
                collection,
                reinterpret_cast<ID2D1LinearGradientBrush**>(m_apSystemBrushes + State_Normal)
                );
        }
        LongUI::SafeRelease(collection);
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
                stops, lengthof<uint32_t>(stops), &collection
                );
        }
        // 创建笔刷
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    D2D1::Point2F(), D2D1::Point2F(0.f, 1.f)
                    ),
                collection,
                reinterpret_cast<ID2D1LinearGradientBrush**>(m_apSystemBrushes + State_Hover)
                );
        }
        LongUI::SafeRelease(collection);
    }
    // 按下
    if (SUCCEEDED(hr)) {
        ID2D1GradientStopCollection* collection = nullptr;
        D2D1_GRADIENT_STOP stops[] = {
            { 0.f, D2D1::ColorF(0xDAECFC) } ,
            { 1.f, D2D1::ColorF(0xC4E0FC) } ,
        };
        // 渐变关键点集
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateGradientStopCollection(
                stops, lengthof<uint32_t>(stops), &collection
                );
        }
        // 创建笔刷
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    D2D1::Point2F(), D2D1::Point2F(0.f, 1.f)
                    ),
                collection,
                reinterpret_cast<ID2D1LinearGradientBrush**>(m_apSystemBrushes + State_Pushed)
                );
        }
        LongUI::SafeRelease(collection);
    }
    return hr;
}

#if defined(_DEBUG) && defined(_MSC_VER)
extern ID3D11Debug*    g_pd3dDebug_longui;
#endif

// UIManager 丢弃
void LongUI::CUIManager::discard_resources() noexcept {
    // 释放系统笔刷
    for (auto& brush : m_apSystemBrushes) {
        LongUI::SafeRelease(brush);
    }
    // 释放公共设备相关资源
    {
        // 释放 位图
        for (auto itr = m_ppBitmaps; itr != m_ppBitmaps + m_cCountBmp; ++itr) {
            LongUI::SafeRelease(*itr);
        }
        // 释放 笔刷
        for (auto itr = m_ppBrushes; itr != m_ppBrushes + m_cCountBrs; ++itr) {
            LongUI::SafeRelease(*itr);
        }
        // META
        for (auto itr = m_pMetasBuffer; itr != m_pMetasBuffer + m_cCountMt; ++itr) {
            LongUI::DestoryObject(*itr);
            itr->bitmap = nullptr;
        }
    }
    // 清除
    if (m_pd2dDevice) {
        m_pd2dDevice->ClearResources();
    }
    // 释放 设备
    LongUI::SafeRelease(m_pDxgiFactory);
    LongUI::SafeRelease(m_pd2dDeviceContext);
    LongUI::SafeRelease(m_pd2dDevice);
    LongUI::SafeRelease(m_pDxgiAdapter);
    LongUI::SafeRelease(m_pDxgiDevice);
    LongUI::SafeRelease(m_pd3dDevice);
    LongUI::SafeRelease(m_pd3dDeviceContext);
#ifdef LONGUI_WITH_MMFVIDEO
    LongUI::SafeRelease(m_pMFDXGIManager);
    LongUI::SafeRelease(m_pMediaEngineFactory);
    ::MFShutdown();
#endif
#ifdef _DEBUG
#ifdef _MSC_VER
    __try {
        if (m_pd3dDebug) {
            LongUI::SafeRelease(g_pd3dDebug_longui);
            g_pd3dDebug_longui = m_pd3dDebug;
            m_pd3dDebug = nullptr; 
        }
    }
    __finally {
        m_pd3dDebug = nullptr;
    }
#else
    if (m_pd3dDebug) {
        m_pd3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
    }
    LongUI::SafeRelease(m_pd3dDebug);
#endif
    this;
#endif
}

// 获取位图
auto LongUI::CUIManager::GetBitmap(size_t index) noexcept ->ID2D1Bitmap1* {
    // 越界
    if (index >= m_cCountBmp) {
        UIManager << DL_Warning 
            << L"[index @ " << long(index)
            << L"]is out of range \t\tNow set to 0" 
            << LongUI::endl;
        index = 0;
    }
    auto bitmap = m_ppBitmaps[index];
    // 没有数据则载入
    if (!bitmap) {
        // 没有数据并且没有资源加载器则?
        assert(m_pResourceLoader);
        // 载入资源
        m_ppBitmaps[index] = static_cast<ID2D1Bitmap1*>(
            m_pResourceLoader->GetResourcePointer(m_pResourceLoader->Type_Bitmap, index - 1)
            );
        bitmap = m_ppBitmaps[index];
    }
    // 再没有数据则报错
    if (!bitmap) {
        UIManager << DL_Error << L"index @ " << long(index) << L"bitmap is null" << LongUI::endl;
    }
    return LongUI::SafeAcquire(bitmap);
}

// 获取笔刷
auto LongUI::CUIManager::GetBrush(size_t index) noexcept -> ID2D1Brush* {
    // 越界
    if (index >= m_cCountBrs) {
        UIManager << DL_Warning
            << L"[index @ " << long(index)
            << L"]is out of range \t\tNow set to 0"
            << LongUI::endl;
        index = 0;
    }
    auto brush = m_ppBrushes[index];
    // 没有数据则载入
    if (!brush) {
        // 没有数据并且没有资源加载器则?
        assert(m_pResourceLoader);
        // 载入资源
        m_ppBrushes[index] = static_cast<ID2D1Brush*>(
            m_pResourceLoader->GetResourcePointer(m_pResourceLoader->Type_Brush, index - 1)
            );
        brush = m_ppBrushes[index];
    }
    // 再没有数据则报错
    if (!brush) {
        UIManager << DL_Error << L"index @ " << long(index) << L"brush is null" << LongUI::endl;
    }
    return LongUI::SafeAcquire(brush);
}

// CUIManager 获取文本格式
auto LongUI::CUIManager::GetTextFormat(size_t index) noexcept ->IDWriteTextFormat* {
    // 越界
    if (index >= m_cCountTf) {
        UIManager << DL_Warning
            << L"[index @ " << long(index)
            << L"]is out of range \t\tNow set to 0"
            << LongUI::endl;
        index = 0;
    }
    auto format = m_ppTextFormats[index];
    // 没有数据则载入
    if (!format) {
        // 没有数据并且没有资源加载器则?
        assert(m_pResourceLoader);
        // 载入资源
        m_ppTextFormats[index] = static_cast<IDWriteTextFormat*>(
            m_pResourceLoader->GetResourcePointer(m_pResourceLoader->Type_TextFormat, index - 1)
            );
        format = m_ppTextFormats[index];
    }
    // 再没有数据则报错
    if (!format) {
        UIManager << DL_Error << L"index @ " << long(index) << L"text format is null" << LongUI::endl;
    }
    return LongUI::SafeAcquire(format);
}

// 利用名称获取
auto LongUI::CUIManager::GetTextRenderer(const char* name) const noexcept -> XUIBasicTextRenderer* {
    int index = 0;
    if (name && name[0]) {
        // 跳过空白
        while (white_space(*name)) ++name;
        // 检查数字
        if (*name >= '0' && *name <= '9') {
            index = LongUI::AtoI(name);
        }
        // 线性查找
        else {
            for (int i = 0; i < int(m_uTextRenderCount); ++i) {
                if (!std::strcmp(m_aszTextRendererName[i].name, name)) {
                    index = i;
                    break;
                }
            }
        }
    }
    return this->GetTextRenderer(index);
}

// 获取图元
void LongUI::CUIManager::GetMeta(size_t index, LongUI::Meta& meta) noexcept {
    // 越界
    if (index >= m_cCountMt) {
        UIManager << DL_Warning
            << L"[index @ " << long(index)
            << L"]is out of range \t\tNow set to 0"
            << LongUI::endl;
        index = 0;
        std::memset(&meta, 0, sizeof(meta));
        return;
    }
    meta = m_pMetasBuffer[index];
    // 没有位图数据则载入
    if (!meta.bitmap) {
        // 没有数据并且没有资源加载器则?
        assert(m_pResourceLoader);
        DeviceIndependentMeta meta_raw;
        ::std::memset(&meta_raw, 0, sizeof(meta_raw));
        // 载入资源
        m_pResourceLoader->GetMeta(index - 1, meta_raw);
        meta.interpolation = meta_raw.interpolation;
        meta.src_rect = meta_raw.src_rect;
        meta.rule = meta_raw.rule;
        meta.bitmap = this->GetBitmap(meta_raw.bitmap_index);
        // 减少计数
        if (meta.bitmap) {
            meta.bitmap->Release();
        }
    }
    // 再没有数据则报错
    if (!meta.bitmap) {
        UIManager << DL_Error << L"index @ " << long(index) << L"meta is null" << LongUI::endl;
    }
}

// 获取Meta的图标句柄
auto LongUI::CUIManager::GetMetaHICON(size_t index) noexcept -> HICON {
    // 越界
    if (index >= m_cCountMt) {
        UIManager << DL_Warning
            << L"[index @ " << long(index)
            << L"]is out of range \t\tNow set to 0"
            << LongUI::endl;
        index = 0;
    }
    // 有就直接返回
    if (m_phMetaIcon[index]) return m_phMetaIcon[index];
    LongUI::Meta meta; this->GetMeta(index, meta);
    assert(meta.bitmap);
    ID2D1Bitmap1* bitmap = this->GetBitmap(LongUIDefaultBitmapIndex);
    D2D1_RECT_U src_rect = {
        static_cast<uint32_t>(meta.src_rect.left),
        static_cast<uint32_t>(meta.src_rect.top),
        static_cast<uint32_t>(meta.src_rect.right),
        static_cast<uint32_t>(meta.src_rect.bottom)
    };
    HRESULT hr = S_OK;
    // 大小保证
    if (SUCCEEDED(hr)) {
        if (src_rect.right - src_rect.left > LongUIDefaultBitmapSize ||
            src_rect.bottom - src_rect.top > LongUIDefaultBitmapSize) {
            assert(!"width/height is too large");
            hr = E_FAIL;
        }
    }
    // 检查错误
    if (SUCCEEDED(hr)) {
        if (!(bitmap && meta.bitmap)) {
            hr = E_POINTER;
        }
    }
    // 复制数据
    if (SUCCEEDED(hr)) {
        hr = bitmap->CopyFromBitmap(nullptr, meta.bitmap, &src_rect);
        longui_debug_hr(hr, L"bitmap->CopyFromBitmap failed");
    }
    // 映射数据
    if (SUCCEEDED(hr)) {
        D2D1_MAPPED_RECT mapped_rect = {
            LongUIDefaultBitmapSize * sizeof(RGBQUAD) ,
            m_pBitmap0Buffer
        };
        hr = bitmap->Map(D2D1_MAP_OPTIONS_READ, &mapped_rect);
        longui_debug_hr(hr, L"bitmap->Map failed");
    }
    // 取消映射
    if (SUCCEEDED(hr)) {
        hr = bitmap->Unmap();
        longui_debug_hr(hr, L"bitmap->Unmap failed");
    }
    // 转换数据
    HICON hAlphaIcon = nullptr;
    if (SUCCEEDED(hr)) {
        auto meta_width = src_rect.right - src_rect.left;
        auto meta_height = src_rect.bottom - src_rect.top;
#if 1
        BITMAPV5HEADER bi; std::memset(&bi, 0, sizeof(BITMAPV5HEADER));
        bi.bV5Size = sizeof(BITMAPV5HEADER);
        bi.bV5Width = meta_width;
        bi.bV5Height = meta_height;
        bi.bV5Planes = 1;
        bi.bV5BitCount = 32;
        bi.bV5Compression = BI_BITFIELDS;
        // 掩码填写
        bi.bV5RedMask = 0x00FF0000;
        bi.bV5GreenMask = 0x0000FF00;
        bi.bV5BlueMask = 0x000000FF;
        bi.bV5AlphaMask = 0xFF000000;
        HDC hdc = ::GetDC(nullptr);
        uint8_t* pTargetBuffer = nullptr;
        // 创建带Alpha通道DIB
        auto hBitmap = ::CreateDIBSection(
            hdc, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS,
            reinterpret_cast<void **>(&pTargetBuffer), nullptr,
            (DWORD)0
            );
        // 错误
        if (!hBitmap) {
            hr = LongUI::WinCode2HRESULT(::GetLastError());
            longui_debug_hr(hr, L"CreateDIBSection failed");
        }
        // 成功
        else {
            auto hMemDC = ::CreateCompatibleDC(hdc);
            ::ReleaseDC(nullptr, hdc);
            // 写入数据
            auto hOldBitmap = static_cast<HBITMAP*>(::SelectObject(hMemDC, hBitmap));
            ::PatBlt(hMemDC, 0, 0, meta_width, meta_height, WHITENESS);
            ::SelectObject(hMemDC, hOldBitmap);
            ::DeleteDC(hMemDC);
            // 创建掩码位图
            HBITMAP hMonoBitmap = ::CreateBitmap(meta_width, meta_height, 1, 1, nullptr);
            // 输入
            auto lpdwPixel = reinterpret_cast<DWORD*>(pTargetBuffer);
            for (auto y = 0u; y < meta_height; ++y) {
                auto src_buffer = m_pBitmap0Buffer + LongUIDefaultBitmapSize * sizeof(RGBQUAD) * y;
                for (auto x = 0u; x < meta_width; ++x) {
                    *lpdwPixel = *src_buffer;
                    src_buffer++;
                    lpdwPixel++;
                }
            }
            // 填写
            ICONINFO ii;
            ii.fIcon = TRUE; ii.xHotspot = 0; ii.yHotspot = 0;
            ii.hbmMask = hMonoBitmap; ii.hbmColor = hBitmap;
            // 创建图标
            hAlphaIcon = ::CreateIconIndirect(&ii);
            ::DeleteObject(hBitmap);
            ::DeleteObject(hMonoBitmap);
#else
        assert(!"CreateIcon just AND & XOR, no alpha channel");
#endif
        }
    }
    ShowHR(hr);
    LongUI::SafeRelease(bitmap);
    return m_phMetaIcon[index] = hAlphaIcon;
}



// 添加窗口
void LongUI::CUIManager::RegisterWindow(UIWindow * wnd) noexcept {
    assert(wnd && "bad argument");
    // 检查剩余空间
    if (m_cCountWindow >= LongUIMaxWindow) {
        assert(!"ABORT! OUT OF SPACE! m_cCountWindow >= LongUIMaxWindow");
        return;
    }
    // 检查是否已经存在
#ifdef _DEBUG
    {
        auto endwindow = m_apWindows + m_cCountWindow;
        if (std::find(m_apWindows, endwindow, wnd) != endwindow) {
            assert(!"target window has been registered.");
        }
    }
#endif
    // 添加窗口
    m_apWindows[m_cCountWindow] = wnd; ++m_cCountWindow;
}

// 刷新屏幕刷新率
void LongUI::CUIManager::RefreshDisplayFrequency() noexcept {
    // 获取屏幕刷新率
    DEVMODEW mode; std::memset(&mode, 0, sizeof(mode));
    ::EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &mode);
    m_dDisplayFrequency = static_cast<uint16_t>(mode.dmDisplayFrequency);
    // 稍微检查
    if (!m_dDisplayFrequency) {
        UIManager << DL_Error
            << L"EnumDisplaySettingsW failed: got zero for DEVMODEW::dmDisplayFrequency"
            << L", now assume as 60Hz"
            << LongUI::endl;
        m_dDisplayFrequency = 60;
    }
}

// 移出窗口
void LongUI::CUIManager::RemoveWindow(UIWindow* wnd, bool cleanup) noexcept {
    assert(m_cCountWindow); assert(wnd && "bad argument");
    // 清理?
    if (cleanup) {
        wnd->cleanup();
#ifdef _DEBUG
        // 现在已经不再数组中了, 不过需要检查一下
        auto endwindow = m_apWindows + m_cCountWindow;
        if (std::find(m_apWindows, endwindow, wnd) != endwindow) {
            assert(!"remove window failed!");
        }
#endif
        return;
    }
    // 检查时是不是在本数组中
#ifdef _DEBUG
    {
        auto endwindow = m_apWindows + m_cCountWindow;
        if (std::find(m_apWindows, endwindow, wnd) == endwindow) {
            assert(!"target window not in windows array!");
            return;
        }
    }
#endif
    // 正式移除
    {
        auto endwindow = m_apWindows + m_cCountWindow;
        auto itr = std::find(m_apWindows, endwindow, wnd);
        if (itr != endwindow) {
            std::memmove(itr, itr + 1, sizeof(void*));
            --m_cCountWindow;
            m_apWindows[m_cCountWindow] = nullptr;
        }
    }
    // 检查时是不是在本数组中
#ifdef _DEBUG
    {
        auto endwindow = m_apWindows + m_cCountWindow;
        if (std::find(m_apWindows, endwindow, wnd) != endwindow) {
            assert(!"target window in windows array!");
            return;
        }
    }
#endif
}

/*/ 是否以管理员权限运行
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
            SHELLEXECUTEINFOW sei = { 0 };
            sei.cbSize = sizeof(sei);
            sei.lpVerb = L"runas";
            sei.lpFile = szPath;
            sei.lpParameters = parameters;
            sei.hwnd = nullptr;
            sei.nShow = SW_NORMAL;
            // 执行
            if (!::ShellExecuteExW(&sei)) {
#ifdef _DEBUG
                DWORD dwError = ::GetLastError();
                assert(dwError == ERROR_CANCELLED && "anyelse?");
#endif
                return false;
            }
            else if(exit) {
                // 退出
                UIManager.Exit();
            }
        }
    }
    return true;
}*/

#ifdef _DEBUG

// 传递可视化东西
auto LongUI::Formated(const wchar_t* format, ...) noexcept -> const wchar_t* {
    static thread_local wchar_t buffer[LongUIStringBufferLength];
    va_list ap;
    va_start(ap, format);
    std::vswprintf(buffer, LongUIStringBufferLength, format, ap);
    va_end(ap);
    return buffer;
}

// 传递可视化东西
auto LongUI::Interfmt(const wchar_t* format, ...) noexcept -> const wchar_t* {
    static thread_local wchar_t buffer[LongUIStringBufferLength];
    va_list ap;
    va_start(ap, format);
    std::vswprintf(buffer, LongUIStringBufferLength, format, ap);
    va_end(ap);
    return buffer;
}

// 换行刷新重载
auto LongUI::CUIManager::operator<<(const LongUI::EndL) noexcept ->CUIManager& {
    wchar_t chs[3] = { L'\r',L'\n', 0 }; 
    this->Output(m_lastLevel, chs);
    return *this;
}

auto LongUI::CUIManager::operator<<(const DXGI_ADAPTER_DESC& desc) noexcept ->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    std::swprintf(
        buffer, LongUIStringBufferLength,
        L"Adapter:   { \r\n\t Description: %ls\r\n\t VendorId: 0x%08X"
        L"\t\t DeviceId: 0x%08X\r\n\t SubSysId: 0x%08X\t\t Revision: 0x%08X\r\n"
        L"\t DedicatedVideoMemory: %.3lfMB\r\n"
        L"\t DedicatedSystemMemory: %.3lfMB\r\n"
        L"\t SharedSystemMemory: %.3lfMB\r\n"
        L"\t AdapterLuid: 0x%08X%08X\r\n }",
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

auto LongUI::CUIManager::operator<<(const RectLTWH_F& rect) noexcept ->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    std::swprintf(
        buffer, LongUIStringBufferLength,
        L"RECT_WH(%7.2f, %7.2f, %7.2f, %7.2f)",
        rect.left, rect.top, rect.width, rect.height
        );
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

auto LongUI::CUIManager::operator<<(const D2D1_MATRIX_3X2_F& matrix) noexcept ->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    std::swprintf(
        buffer, LongUIStringBufferLength,
        L"MATRIX (%7.2f, %7.2f, %7.2f, %7.2f, %7.2f, %7.2f)",
        matrix._11, matrix._12, 
        matrix._21, matrix._22, 
        matrix._31, matrix._32
        );
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

auto LongUI::CUIManager::operator<<(const D2D1_RECT_F& rect) noexcept ->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    std::swprintf(
        buffer, LongUIStringBufferLength,
        L"RECT_RB(%7.2f, %7.2f, %7.2f, %7.2f)",
        rect.left, rect.top, rect.right, rect.bottom
        );
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

auto LongUI::CUIManager::operator<<(const D2D1_POINT_2F& pt) noexcept ->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    std::swprintf(
        buffer, LongUIStringBufferLength,
        L"POINT(%7.2f, %7.2f)",
        pt.x, pt.y
        );
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

// 输出UTF-8字符串 并刷新
void LongUI::CUIManager::Output(DebugStringLevel l, const char* s) noexcept {
    wchar_t buffer[LongUIStringBufferLength];
    buffer[LongUI::UTF8toWideChar(s, buffer)] = 0;
    this->Output(l, buffer);
}

// 输出UTF-8字符串
void LongUI::CUIManager::OutputNoFlush(DebugStringLevel l, const char* s) noexcept {
    wchar_t buffer[LongUIStringBufferLength];
    buffer[LongUI::UTF8toWideChar(s, buffer)] = 0;
    this->OutputNoFlush(l, buffer);
}

// 浮点重载
auto LongUI::CUIManager::operator<<(const float f) noexcept ->CUIManager&  {
    wchar_t buffer[LongUIStringBufferLength];
    std::swprintf(buffer, LongUIStringBufferLength, L"%.2f", f);
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

// 指针
auto LongUI::CUIManager::operator<<(const void* ctrl) noexcept ->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    std::swprintf(buffer, LongUIStringBufferLength, L"[0x%p] ", ctrl);
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

// 控件
auto LongUI::CUIManager::operator<<(const UIControl* ctrl) noexcept ->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    if (ctrl) {
        std::swprintf(
            buffer, LongUIStringBufferLength,
            L"[0x%p{%S}%ls] ",
            ctrl,
            ctrl->name.c_str(),
            ctrl->GetControlClassName(false)
            );
    }
    else {
        std::swprintf(buffer, LongUIStringBufferLength, L"[null] ");
    }
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

// 控件
auto LongUI::CUIManager::operator<<(const ControlVector& ctrls) noexcept ->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    int index = 0;
    for (auto ctrl : ctrls) {
        std::swprintf(
            buffer, lengthof(buffer),
            L"\r\n\t\t[%4d][0x%p{%S}%ls] ",
            index,
            ctrl,
            ctrl->name.c_str(),
            ctrl->GetControlClassName(false)
            );
        this->OutputNoFlush(m_lastLevel, buffer);
        ++index;
    }
    return *this;
}

// 整型重载
auto LongUI::CUIManager::operator<<(const long l) noexcept ->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    std::swprintf(buffer, LongUIStringBufferLength, L"%ld", l);
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

// 整型重载
auto LongUI::CUIManager::operator<<(const bool b) noexcept ->CUIManager& {
    this->OutputNoFlush(m_lastLevel, b ? "true" : "false");
    return *this;
}

#endif