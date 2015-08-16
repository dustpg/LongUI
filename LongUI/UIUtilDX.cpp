#include "LongUI.h"


// 创建LongUI的字体集: 本函数会进行I/O, 所以程序开始调用一次即可
auto LongUI::DX::CreateFontCollection(
    IDWriteFactory* factory, const wchar_t * filename, const wchar_t * folder)
    noexcept -> IDWriteFontCollection* {
    // 字体文件枚举
    class LongUIFontFileEnumerator final : public Helper::ComStatic<
        Helper::QiList<IDWriteFontFileEnumerator >> {
    public:
        // 获取当前字体文件
        HRESULT STDMETHODCALLTYPE GetCurrentFontFile(IDWriteFontFile **ppFontFile) noexcept override {
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
    class LongUIFontCollectionLoader final : public Helper::ComStatic<
        Helper::QiList<IDWriteFontCollectionLoader >> {
    public:
        // 创建枚举器
        HRESULT STDMETHODCALLTYPE CreateEnumeratorFromKey(
            IDWriteFactory *pFactory,
            const void *collectionKey,
            UINT32 collectionKeySize,
            IDWriteFontFileEnumerator **ppFontFileEnumerator
            ) noexcept override {
            UNREFERENCED_PARAMETER(collectionKeySize);
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
        WIN32_FIND_DATAW fileinfo;
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
                buffer, 
                static_cast<uint32_t>(reinterpret_cast<uint8_t*>(index) - reinterpret_cast<uint8_t*>(buffer)),
                &collection
                );
            factory->UnregisterFontCollectionLoader(&loader);
        }
        delete[] buffer;
    }
    return collection;
}

// 从 文本格式创建几何
auto LongUI::DX::CreateTextPathGeometry(
    IN const char32_t* utf32_string,
    IN uint32_t string_length,
    IN IDWriteTextFormat* format,
    IN ID2D1Factory* factory,
    IN OUT OPTIONAL IDWriteFontFace** _fontface,
    OUT ID2D1PathGeometry** geometry
    ) noexcept -> HRESULT {
    // 参数检查
    assert(utf32_string && string_length && format && geometry && factory && "bad arugments");
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
            // 不存在也算错误
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
auto LongUI::DX::CreateMeshFromGeometry(ID2D1Geometry* geometry, ID2D1Mesh** mesh) noexcept -> HRESULT {
    UNREFERENCED_PARAMETER(geometry);
    UNREFERENCED_PARAMETER(mesh);
    return E_NOTIMPL;
}



// 转换为Core-Mode格式
auto LongUI::DX::XMLToCoreFormat(const char* xml, wchar_t* core) noexcept -> bool {
    if (!xml || !core) return false;
    wchar_t buffer[LongUIStringBufferLength];
    *buffer = 0;
    return true;
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
auto __cdecl LongUI::DX::FormatTextCore(const FormatTextConfig& config, const wchar_t* format, ...) noexcept->IDWriteTextLayout* {
    va_list ap;
    va_start(ap, format);
    return DX::FormatTextCore(config, format, ap);
}

// find next param
template<typename T>
auto __fastcall FindNextToken(T* buffer, const wchar_t* stream, size_t token_num) {
    register wchar_t ch;
    while ((ch = *stream)) {
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


#define DXHelper_GetNextTokenW(n) param = FindNextToken(param_buffer, param, n)
#define DXHelper_GetNextTokenA(n) param = FindNextToken(reinterpret_cast<char*>(param_buffer), param, n)


// 创建格式文本
// 效率本函数耗时参考:
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

auto  LongUI::DX::FormatTextCore( const FormatTextConfig& config, 
    const wchar_t* format, va_list ap) noexcept->IDWriteTextLayout* {
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
    // Range Type
    enum class R : size_t { N, W, Y, H, S, U, E, D, I };
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
        };
        R                       range_type;
    } range_data;
    ::memset(&range_data, 0, sizeof(range_data));
    assert(format && "bad argument");
    IDWriteTextLayout* layout = nullptr;
    register CUIColorEffect* tmp_color = nullptr;
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
    LongUI::EzContainer::FixedStack<RangeData, 1024> stack_check, statck_set;
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
                    DXHelper_GetNextTokenW(1);
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
                tmp_color = CUIColorEffect::Create();
                assert(tmp_color && "C");
                // 从范围数据中获取
                if (ap) {
                    tmp_color->color = *range_data.color;
                }
                // 直接设置
                else {
                    DXHelper_GetNextTokenA(4);
                    Helper::MakeColor(reinterpret_cast<char*>(param_buffer), tmp_color->color);
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
                tmp_color = CUIColorEffect::Create();
                assert(tmp_color && "c");
                if (ap) {
                    LongUI::UnpackTheColorARGB(range_data.u32, tmp_color->color);
                }
                else {
                    DXHelper_GetNextTokenA(1);
                    Helper::MakeColor(reinterpret_cast<char*>(param_buffer), tmp_color->color);
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
                    DXHelper_GetNextTokenW(1);
                    IUnknown* result = nullptr;
                    assert(!"noimpl");
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
                    DXHelper_GetNextTokenA(1);
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
                    DXHelper_GetNextTokenA(1);
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
                    DXHelper_GetNextTokenW(1);
                    IDWriteInlineObject* result = nullptr;
                    assert(!"noimpl");
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
                    DXHelper_GetNextTokenA(1);
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
                    DXHelper_GetNextTokenA(1);
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
                    DXHelper_GetNextTokenA(1);
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
                    DXHelper_GetNextTokenA(1);
                    range_data.style = static_cast<DWRITE_FONT_STYLE>(
                        LongUI::AtoI(reinterpret_cast<char*>(param_buffer))
                        );
                }
                range_data.range_type = R::Y;
                stack_check.push(range_data);
                break;
            case L'P': case L'p': // end of main string, then, is the param
                goto force_break;
            case L']': case L'}': // end of all range type
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
    // clamp it
    if (string_length_need < 0) string_length_need = 0;
    else if (string_length_need > string_length) string_length_need = string_length;
    // 修正
    va_end(ap);
    // 创建布局
    if (config.dw_factory && SUCCEEDED(config.dw_factory->CreateTextLayout(
        buffer, string_length_need, config.text_format, config.width, config.height, &layout
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

// 保存图片
auto LongUI::DX::SaveAsImageFile(
    ID2D1Bitmap1* bitmap, IWICImagingFactory* factory, 
    const wchar_t* file_name, const GUID* container_format
    ) noexcept -> HRESULT {
    // 参数检查
    assert(bitmap && factory && file_name && file_name[0]);
    if (!(bitmap && factory && file_name && file_name[0])) {
        return E_INVALIDARG;
    }
    // 初始化
    D2D1_MAPPED_RECT rect = { 0 };
    HRESULT hr = S_OK;
    ID2D1Bitmap1* readable_bitmap = nullptr;
    auto bitmap_size = bitmap->GetPixelSize();
    auto dxgi_format = bitmap->GetPixelFormat();
    // 创建CPU可读位图
    if (SUCCEEDED(hr)) {
        // CPU 可读?
        if (bitmap->GetOptions() & D2D1_BITMAP_OPTIONS_CPU_READ) {
            readable_bitmap = ::SafeAcquire(bitmap);
        }
        else {
            D2D1_BITMAP_PROPERTIES1 prop;
            prop.pixelFormat = dxgi_format;
            bitmap->GetDpi(&prop.dpiX, &prop.dpiY);
            prop.colorContext = nullptr;
            prop.bitmapOptions = D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
            // 创建位图
            hr = UIManager_RenderTaget->CreateBitmap(
                bitmap_size,
                nullptr,
                0,
                &prop,
                &readable_bitmap
                );
        }
    }
    // 复制位图
    if (SUCCEEDED(hr)) {
        hr = readable_bitmap->CopyFromBitmap(nullptr, bitmap, nullptr);
    }
    // 映射数据
    if (SUCCEEDED(hr)) {
        hr = readable_bitmap->Map(D2D1_MAP_OPTIONS_READ, &rect);
    }
    // 处理
    if (SUCCEEDED(hr)) {
        SaveAsImageFileProperties prop;
        prop.bits = rect.bits;
        prop.factory = factory;
        prop.data_format = LongUI::DX::DXGIToWIC(dxgi_format.format);
        prop.container_format = container_format;
        prop.width = bitmap_size.width;
        prop.height = bitmap_size.height;
        prop.pitch = rect.pitch;
        auto hr1 = LongUI::DX::SaveAsImageFile(prop, file_name);
        auto hr2 = readable_bitmap->Unmap();
        // 检查错误
        if (SUCCEEDED(hr1)) {
            hr = SUCCEEDED(hr2) ? S_OK : hr2;
        }
        else {
            hr = hr1;
        }
    }
    // 扫尾处理
    ::SafeRelease(readable_bitmap);
    // 返回结果
    return hr;
}

#include <Wincodec.h>

// longui namespace
namespace LongUI {
    // dx namespace
    namespace DX {
        // 保存数据为图片格式
        auto SaveAsImageFile(const SaveAsImageFileProperties& prop, const wchar_t* file_name) noexcept -> HRESULT {
            // 参数检查
            assert(prop.bits && prop.factory && file_name && file_name[0]);
            if (!(prop.bits && prop.factory && file_name && file_name[0])) {
                return E_INVALIDARG;
            }
            DXGI_FORMAT;
            // 初始化
            HRESULT hr = S_OK;
            IWICBitmapEncoder *pEncoder = nullptr;
            IWICBitmapFrameEncode *pFrameEncode = nullptr;
            IWICStream *pStream = nullptr;
            IWICBitmap *pWICBitmap = nullptr;
            // 创建WIC位图
            if (SUCCEEDED(hr)) {
                hr = prop.factory->CreateBitmapFromMemory(
                    prop.width,
                    prop.height,
                    prop.data_format ? *prop.data_format : GUID_WICPixelFormat32bppBGRA,
                    prop.pitch,
                    prop.pitch * prop.height,
                    prop.bits,
                    &pWICBitmap
                    );
            }
            // 创建流
            if (SUCCEEDED(hr)) {
                hr = prop.factory->CreateStream(&pStream);
            }
            // 从文件初始化
            if (SUCCEEDED(hr)) {
                hr = pStream->InitializeFromFilename(file_name, GENERIC_WRITE);
            }
            // 创建编码器
            if (SUCCEEDED(hr)) {
                hr = prop.factory->CreateEncoder(
                    prop.container_format ? *prop.container_format : GUID_ContainerFormatPng,
                    nullptr,
                    &pEncoder
                    );
            }
            // 初始化编码器
            if (SUCCEEDED(hr)) {
                hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
            }
            // 创建新的一帧
            if (SUCCEEDED(hr)) {
                hr = pEncoder->CreateNewFrame(&pFrameEncode, nullptr);
            }
            // 初始化帧编码器
            if (SUCCEEDED(hr)) {
                hr = pFrameEncode->Initialize(nullptr);
            }
            // 设置大小
            if (SUCCEEDED(hr)) {
                hr = pFrameEncode->SetSize(prop.width, prop.height);
            }
            // 设置格式
            WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
            if (SUCCEEDED(hr)) {
                hr = pFrameEncode->SetPixelFormat(&format);
            }
            // 写入源数据
            if (SUCCEEDED(hr)) {
                hr = pFrameEncode->WriteSource(pWICBitmap, nullptr);
            }
            // 提交帧编码器
            if (SUCCEEDED(hr)) {
                hr = pFrameEncode->Commit();
            }
            // 提交编码
            if (SUCCEEDED(hr)) {
                hr = pEncoder->Commit();
            }
            // 扫尾处理
            ::SafeRelease(pWICBitmap);
            ::SafeRelease(pStream);
            ::SafeRelease(pFrameEncode);
            ::SafeRelease(pEncoder);
            // 返回结果
            return hr;
        }
        // WIC GUID <--> DXGI_FORMAT
        struct WICTranslate { GUID wic; DXGI_FORMAT format; };
        // data
        static const WICTranslate s_WICFormats[] =  {
            { GUID_WICPixelFormat128bppRGBAFloat,       DXGI_FORMAT_R32G32B32A32_FLOAT },

            { GUID_WICPixelFormat64bppRGBAHalf,         DXGI_FORMAT_R16G16B16A16_FLOAT },
            { GUID_WICPixelFormat64bppRGBA,             DXGI_FORMAT_R16G16B16A16_UNORM },

            { GUID_WICPixelFormat32bppRGBA,             DXGI_FORMAT_R8G8B8A8_UNORM },
            { GUID_WICPixelFormat32bppBGRA,             DXGI_FORMAT_B8G8R8A8_UNORM },
            { GUID_WICPixelFormat32bppBGR,              DXGI_FORMAT_B8G8R8X8_UNORM },

            { GUID_WICPixelFormat32bppRGBA1010102XR,    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM },
            { GUID_WICPixelFormat32bppRGBA1010102,      DXGI_FORMAT_R10G10B10A2_UNORM },
            { GUID_WICPixelFormat32bppRGBE,             DXGI_FORMAT_R9G9B9E5_SHAREDEXP },

            { GUID_WICPixelFormat16bppBGRA5551,         DXGI_FORMAT_B5G5R5A1_UNORM },
            { GUID_WICPixelFormat16bppBGR565,           DXGI_FORMAT_B5G6R5_UNORM },

            { GUID_WICPixelFormat32bppGrayFloat,        DXGI_FORMAT_R32_FLOAT },
            { GUID_WICPixelFormat16bppGrayHalf,         DXGI_FORMAT_R16_FLOAT },
            { GUID_WICPixelFormat16bppGray,             DXGI_FORMAT_R16_UNORM },
            { GUID_WICPixelFormat8bppGray,              DXGI_FORMAT_R8_UNORM },

            { GUID_WICPixelFormat8bppAlpha,             DXGI_FORMAT_A8_UNORM },

            { GUID_WICPixelFormatBlackWhite,            DXGI_FORMAT_R1_UNORM },

            { GUID_WICPixelFormat96bppRGBFloat,         DXGI_FORMAT_R32G32B32_FLOAT },
        };
        // DXGI格式转换为 WIC GUID 格式
        auto DXGIToWIC(DXGI_FORMAT format) noexcept ->const GUID*{
            const GUID* outformat = nullptr;
            // 检查
            for (const auto& data : s_WICFormats) {
                if (data.format == format) {
                    outformat = &data.wic;
                    break;
                }
            }
            // 特殊
            if (!outformat) {
                switch (format)
                {
                case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
                    outformat = &GUID_WICPixelFormat32bppRGBA;
                    break;
                case DXGI_FORMAT_D32_FLOAT:
                    outformat = &GUID_WICPixelFormat32bppGrayFloat;
                    break;
                case DXGI_FORMAT_D16_UNORM:
                    outformat = &GUID_WICPixelFormat16bppGray;
                    break;
                case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
                    outformat = &GUID_WICPixelFormat32bppBGRA;
                    break;
                case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
                    outformat = &GUID_WICPixelFormat32bppBGR;
                    break;
                }
            }
            // 返回数据
            return outformat;
        }
    }
}

