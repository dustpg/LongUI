#include "HsvEffect.h"
#include <d3dcompiler.h>
#include <LongUI/luiUiHlper.h>
#include <Core/luiManager.h>

// longui namespace
namespace LongUI {
    // {21686B62-95A8-4799-90FE-5D521EF7B334}
    const GUID CLSID_HsvEffect = { 
        0x21686b62, 0x95a8, 0x4799, { 0x90, 0xfe, 0x5d, 0x52, 0x1e, 0xf7, 0xb3, 0x34 } 
    };
    // {1BDEA04C-EB0E-429F-9D1D-5F149B6BABDC}
    const GUID GUID_HsvEffectVShader = { 
        0x1bdea04c, 0xeb0e, 0x429f, { 0x9d, 0x1d, 0x5f, 0x14, 0x9b, 0x6b, 0xab, 0xdc } 
    };
    // {63D30343-A465-4CE1-8ECC-93613FC02538}
    const GUID GUID_HsvEffectPShader = { 
        0x63d30343, 0xa465, 0x4ce1, { 0x8e, 0xcc, 0x93, 0x61, 0x3f, 0xc0, 0x25, 0x38 } 
    };
    // {6EA0C533-2337-4745-906E-10A4C7BEC979}
    const GUID GUID_HsvEffectVBuffer = { 
        0x6ea0c533, 0x2337, 0x4745, { 0x90, 0x6e, 0x10, 0xa4, 0xc7, 0xbe, 0xc9, 0x79 } 
    };
    // hsv shader
    const char* const HSV_SHADER = u8R"hlsl(
cbuffer CT : register(b0) { float2x1 S2OX, S2OY;};
cbuffer HC : register(b1) { float4 hsvcolor; };
struct VIT { float4 color : COLOR; float4 position : MESH_POSITION; };
struct PIT { 
  float4 position : SV_POSITION; 
  float4 scenepos : SCENE_POSITION;
  float4 color    : COLOR;
};
PIT vsmain(VIT i) {
  PIT o;
  o.scenepos = float4(i.position.xy, 1, 1); 
  o.position.x = (i.position.x * S2OX[0] + S2OX[1]);
  o.position.y = (i.position.y * S2OY[0] + S2OY[1]);
  o.position.z = 1;
  o.position.w = 1;
  o.color = i.color.w == 0 ? hsvcolor : i.color;
  return o;
}
float4 psmain(PIT i) : SV_TARGET { return i.color; }
)hlsl";
    // make vertex
    static void MakeVertex(Effect::Hsv::Vertex vbuffer[]) {
        constexpr float unit = float(Effect::Hsv::UNIT);
        constexpr int step = int(Effect::Hsv::VERTEX_PER_UNIT);
        constexpr int count = int(Effect::Hsv::HSV_COLOR_COUNT);
        constexpr float pi = 3.14159265f;
        constexpr float pi2 = pi * 2.f;
        constexpr float sin60 = 0.8660254f;
        constexpr float cos60 = 0.5f;
        D2D1_COLOR_F colors[count + 1] = {
            { 1.f, 0.f, 0.f, 1.f},
            { 1.f, 1.f, 0.f, 1.f},
            { 0.f, 1.f, 0.f, 1.f},
            { 0.f, 1.f, 1.f, 1.f},
            { 0.f, 0.f, 1.f, 1.f},
            { 1.f, 0.f, 1.f, 1.f},
            { 1.f, 0.f, 0.f, 1.f},
        };
        for (int i = 0; i < count; ++i) {
            // 计算
            const auto j = i + 1;
            const float delta1 = float(i) / float(count) * pi2;
            const float delta2 = float(j) / float(count) * pi2;
            D2D1_POINT_2F lt, lb, rt, rb;
            lb.x = std::sin(delta1) * unit;
            lb.y =-std::cos(delta1) * unit;
            lt.x = lb.x * 2.f;
            lt.y = lb.y * 2.f;
            rb.x = std::sin(delta2) * unit;
            rb.y =-std::cos(delta2) * unit;
            rt.x = rb.x * 2.f;
            rt.y = rb.y * 2.f;
            // 设置
            vbuffer[i*step + 0] = { colors[i], lt };
            vbuffer[i*step + 1] = { colors[i], lb };
            vbuffer[i*step + 2] = { colors[j], rt };
            // 设置
            vbuffer[i*step + 3] = { colors[j], rt };
            vbuffer[i*step + 4] = { colors[i], lb };
            vbuffer[i*step + 5] = { colors[j], rb };
        }
        // 最后一个三角形
        vbuffer[Effect::Hsv::VERTEX_COUNT - 3] = {
            D2D1::ColorF(D2D1::ColorF::Red, 0.f),
            { 0.f, -unit }
        };
        vbuffer[Effect::Hsv::VERTEX_COUNT - 2] = {
            D2D1::ColorF(D2D1::ColorF::Black),
            {-sin60 * unit, cos60 * unit }
        };
        vbuffer[Effect::Hsv::VERTEX_COUNT - 1] = {
            D2D1::ColorF(D2D1::ColorF::White),
            { sin60 * unit, cos60 * unit }
        };
    }
    // CompileHLSL
    auto CompileHLSL(
        const char* str,
        const char* main,
        const char* type,
        ID3DBlob*& shader) noexcept {
        UINT flag = 0;
#if _DEBUG
        flag |= D3DCOMPILE_DEBUG;
#endif
#ifdef _DEBUG
        ID3DBlob* info = nullptr;
#endif
        auto hr = ::D3DCompile2(
            str, std::strlen(str),
            nullptr, nullptr, nullptr, 
            main, type, flag, 0, 0,
            nullptr, 0,
            &shader, 
#ifdef _DEBUG
            &info
#else
            nullptr
#endif
        );
#ifdef _DEBUG
        const char* error = "";
        if (info) error = reinterpret_cast<decltype(str)>(info->GetBufferPointer());
        longui_debug_hr(hr, L"D3DCompileFromFile: " << error);
        LongUI::SafeRelease(info);
#endif
        return hr;
    }
}


/// <summary>
/// HSVs this instance.
/// </summary>
/// <returns></returns>
LongUI::Effect::Hsv::Hsv() noexcept {
    m_cbuffer.color = D2D1::ColorF(D2D1::ColorF::Red);
}


/// <summary>
/// Finalizes an instance of the <see cref=""/> class.
/// </summary>
/// <returns></returns>
LongUI::Effect::Hsv::~Hsv() noexcept {
    LongUI::SafeRelease(m_pVertexBuffer);
    LongUI::SafeRelease(m_pDrawInfo);
}


/// <summary>
/// Registers with the specified factory.
/// </summary>
/// <param name="factory">The factory.</param>
/// <returns></returns>
auto LongUI::Effect::Hsv::Register(ID2D1Factory1* factory) noexcept ->HRESULT {
    assert(factory && "bad argment");
    const WCHAR* pszXml = LR"xml(<?xml version = "1.0" ?>
<Effect>
    <Property name = "DisplayName" type = "string" value = "LongUI HSV Effect" />
    <Property name = "Author" type = "string" value = "dustpg" />
    <Property name = "Category" type = "string" value = "Source" />
    <Property name = "Description" type = "string" value = "HSV" />
    <Inputs/>
    <Property name='Color' type='vector4'>
        <Property name='DisplayName' type='string' value='HSV Picked Color'/>
    </Property>
</Effect>
)xml";
    // 创建
    auto create_this = [](IUnknown** effect) noexcept ->HRESULT {
        assert(effect && "bad argment");
        if (const auto ptr = LongUI::SmallAlloc(sizeof(Effect::Hsv))) {
            ID2D1EffectImpl* obj = new(ptr) Effect::Hsv;
            *effect = obj;
            return S_OK;
        }
        return E_OUTOFMEMORY;
    };
    // 属性绑定
    D2D1_PROPERTY_BINDING bindings[] = {
        LONGUI_VALUE_TYPE_BINDING(Hsv, D2D1_COLOR_F, Color),
    };
    // 注册
    return factory->RegisterEffectFromString(
        LongUI::CLSID_HsvEffect,
        pszXml,
        bindings, lengthof<uint32_t>(bindings),
        create_this
    );
}

/// <summary>
/// Initializes the specified p effect context.
/// 初始化对象 Create 创建后 会调用此方法
/// </summary>
/// <param name="pEffectContext">The p effect context.</param>
/// <param name="pTransformGraph">The p transform graph.</param>
/// <returns></returns>
IFACEMETHODIMP LongUI::Effect::Hsv::Initialize(
    ID2D1EffectContext* pEffectContext,
    ID2D1TransformGraph* pTransformGraph) noexcept {
    // 参数检查
    assert(pEffectContext && pTransformGraph && "bad arguments");
    if (!pEffectContext || !pTransformGraph) return E_INVALIDARG;
    HRESULT hr = S_FALSE;
    // 载入shader文件
    if (SUCCEEDED(hr)) {
        BYTE buf[4] = { 0 };
        ID3DBlob *vs = nullptr;
        // 尝试载入VS
        if (FAILED(pEffectContext->LoadVertexShader(LongUI::GUID_HsvEffectVShader, buf, 0))) {
            // 编译VS
            if (SUCCEEDED(hr)) {
                hr = LongUI::CompileHLSL(HSV_SHADER, "vsmain", "vs_5_0", vs);
            }
            // 载入VS
            if (SUCCEEDED(hr)) {
                hr = pEffectContext->LoadVertexShader(
                    LongUI::GUID_HsvEffectVShader,
                    reinterpret_cast<const BYTE*>(vs->GetBufferPointer()),
                    static_cast<uint32_t>(vs->GetBufferSize())
                );
                longui_debug_hr(hr, L"LoadVertexShader failed");
            }
        }
        // 尝试载入PS
        if (FAILED(pEffectContext->LoadVertexShader(LongUI::GUID_HsvEffectPShader, buf, 0))) {
            ID3DBlob *ps = nullptr;
            // 编译PS
            if (SUCCEEDED(hr)) {
                hr = LongUI::CompileHLSL(HSV_SHADER, "psmain", "ps_5_0", ps);
            }
            // 载入PS
            if (SUCCEEDED(hr)) {
                hr = pEffectContext->LoadPixelShader(
                    LongUI::GUID_HsvEffectPShader,
                    reinterpret_cast<const BYTE*>(ps->GetBufferPointer()),
                    static_cast<uint32_t>(ps->GetBufferSize())
                );
                longui_debug_hr(hr, L"LoadPixelShader failed");
            }
            // 扫尾
            LongUI::SafeRelease(ps);
        }
        // 尝试载入顶点缓存
        if (FAILED(pEffectContext->FindVertexBuffer(&GUID_HsvEffectVBuffer, &m_pVertexBuffer))) {
            Hsv::Vertex vbuffer[VERTEX_COUNT];
            LongUI::MakeVertex(vbuffer);
            D2D1_VERTEX_BUFFER_PROPERTIES vbProp = {0};
            vbProp.byteWidth = sizeof(vbuffer);
            vbProp.data = reinterpret_cast<BYTE*>(vbuffer);
            vbProp.inputCount = 1;
            vbProp.usage =  D2D1_VERTEX_USAGE_STATIC;
            static const D2D1_INPUT_ELEMENT_DESC vertexLayout[] = {
                {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0},
                {"MESH_POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(Vertex::color)},
            };
            D2D1_CUSTOM_VERTEX_BUFFER_PROPERTIES cvbProp = {0};
            cvbProp.elementCount = uint32_t(std::end(vertexLayout) - std::begin(vertexLayout));
            cvbProp.inputElements = vertexLayout;
            cvbProp.stride = sizeof(Vertex);
            // 编译VS
            if (!vs) hr = LongUI::CompileHLSL(HSV_SHADER, "vsmain", "vs_5_0", vs);
            // 创建缓存
            if (SUCCEEDED(hr)) {
                const auto buf = reinterpret_cast<const BYTE*>(vs->GetBufferPointer());
                cvbProp.shaderBufferWithInputSignature = buf;
                cvbProp.shaderBufferSize = static_cast<uint32_t>(vs->GetBufferSize());
                // 创建顶点缓存
                hr = pEffectContext->CreateVertexBuffer(
                    &vbProp,
                    &GUID_HsvEffectVBuffer,
                    &cvbProp,
                    &m_pVertexBuffer
                );
            }
            else {
                assert(!"ERROR, NO VS");
            }
        }
        LongUI::SafeRelease(vs);
    }
    // 连接
    if (SUCCEEDED(hr)) {
        hr = pTransformGraph->SetSingleTransformNode(this);
    }
    longui_debug_hr(hr, L"Initialize failed");
    return hr;
}


/// <summary>
/// Prepares for render.
/// </summary>
/// <param name="changeType">Type of the change.</param>
/// <returns></returns>
IFACEMETHODIMP LongUI::Effect::Hsv::PrepareForRender(D2D1_CHANGE_TYPE changeType) noexcept {
    if (changeType == D2D1_CHANGE_TYPE_NONE) return S_OK;
    // 更新常量缓存
    return m_pDrawInfo->SetVertexShaderConstantBuffer(
        reinterpret_cast<BYTE*>(&m_cbuffer), sizeof(m_cbuffer)
    );
}

/// <summary>
/// Releases this instance.
/// </summary>
/// <returns></returns>
IFACEMETHODIMP_(ULONG) LongUI::Effect::Hsv::Release() noexcept {
    if ((--m_cRef) == 0){
        this->Hsv::~Hsv();
        LongUI::SmallFree(this);
        return 0;
    }
    else {
        return m_cRef;
    }
}

/// <summary>
/// Queries the interface.
/// </summary>
/// <param name="riid">The riid.</param>
/// <param name="ppOutput">The pp output.</param>
/// <returns></returns>
IFACEMETHODIMP LongUI::Effect::Hsv::QueryInterface(REFIID riid, _Outptr_ void** ppOutput) noexcept {
    *ppOutput = nullptr;
    HRESULT hr = S_OK;
    // 获取 ID2D1EffectImpl
    if (riid == LongUI::GetIID<ID2D1EffectImpl>()) {
        *ppOutput = static_cast<ID2D1EffectImpl*>(this);
    }
    // 获取 ID2D1DrawTransform
    else if (riid == LongUI::GetIID<ID2D1DrawTransform>()) {
        *ppOutput = static_cast<ID2D1DrawTransform*>(this);
    }
    // 获取 ID2D1Transform
    else if (riid == LongUI::GetIID<ID2D1Transform>()) {
        *ppOutput = static_cast<ID2D1Transform*>(this);
    }
    // 获取 ID2D1TransformNode
    else if (riid == LongUI::GetIID<ID2D1TransformNode>()) {
        *ppOutput = static_cast<ID2D1TransformNode*>(this);
    }
    // 获取 IUnknown
    else if (riid == LongUI::GetIID<IUnknown>()){
        *ppOutput = this;
    }
    // 没有接口
    else {
        hr = E_NOINTERFACE;
    }
    // 有效
    if (*ppOutput != nullptr) {
        this->AddRef();
    }
    return hr;
}


/// <summary>
/// Sets the draw information.
/// </summary>
/// <param name="drawInfo">The draw information.</param>
/// <returns></returns>
IFACEMETHODIMP LongUI::Effect::Hsv::SetDrawInfo(_In_ ID2D1DrawInfo *drawInfo) noexcept {
    LongUI::SafeRelease(m_pDrawInfo);
    m_pDrawInfo = LongUI::SafeAcquire(drawInfo);
    auto hr = S_OK;
    // 设置像素着色器
    if (SUCCEEDED(hr)) {
        hr = drawInfo->SetPixelShader(GUID_HsvEffectPShader);
    }
    // 设置顶点
    if (SUCCEEDED(hr)) {
        D2D1_VERTEX_RANGE range{ 0, VERTEX_COUNT };
        hr = drawInfo->SetVertexProcessing(
            m_pVertexBuffer,
            D2D1_VERTEX_OPTIONS_NONE,
            nullptr, 
            &range,
            &LongUI::GUID_HsvEffectVShader
        );
    }
    // 修改常量缓存
    if (SUCCEEDED(hr)) {
        hr = drawInfo->SetVertexShaderConstantBuffer(
            reinterpret_cast<BYTE*>(&m_cbuffer), sizeof(m_cbuffer)
        );
    }
    return hr;
}

/// <summary>
/// Maps the invalid rect.
/// </summary>
/// <param name="inputIndex">Index of the input.</param>
/// <param name="invalidInputRect">The invalid input rect.</param>
/// <param name="pInvalidOutputRect">The p invalid output rect.</param>
/// <returns></returns>
IFACEMETHODIMP LongUI::Effect::Hsv::MapInvalidRect(
    UINT32 inputIndex,
    D2D1_RECT_L invalidInputRect,
    _Out_ D2D1_RECT_L* pInvalidOutputRect
) const noexcept {
    return S_OK;
}

/// <summary>
/// Maps the output rect to input rects.
/// </summary>
/// <param name="pOutputRect">The p output rect.</param>
/// <param name="pInputRects">The p input rects.</param>
/// <param name="inputRectCount">The input rect count.</param>
/// <returns></returns>
IFACEMETHODIMP LongUI::Effect::Hsv::MapOutputRectToInputRects(
    _In_ const D2D1_RECT_L* pOutputRect,
    _Out_writes_(inputRectCount) D2D1_RECT_L* pInputRects,
    UINT32 inputRectCount
) const noexcept {
    //if (inputRectCount != 1) return E_INVALIDARG;
    return S_OK;
}

/// <summary>
/// Maps the input rects to output rect.
/// </summary>
/// <param name="pInputRects">The p input rects.</param>
/// <param name="pInputOpaqueSubRects">The p input opaque sub rects.</param>
/// <param name="inputRectCount">The input rect count.</param>
/// <param name="pOutputRect">The p output rect.</param>
/// <param name="pOutputOpaqueSubRect">The p output opaque sub rect.</param>
/// <returns></returns>
IFACEMETHODIMP LongUI::Effect::Hsv::MapInputRectsToOutputRect(
    _In_reads_(inputRectCount) const D2D1_RECT_L* pInputRects,
    _In_reads_(inputRectCount) const D2D1_RECT_L* pInputOpaqueSubRects,
    UINT32 inputRectCount,
    _Out_ D2D1_RECT_L* pOutputRect,
    _Out_ D2D1_RECT_L* pOutputOpaqueSubRect
) noexcept {
    pOutputRect[0] = { LONG_MIN, LONG_MIN, LONG_MAX, LONG_MAX };
    pOutputOpaqueSubRect[0] = D2D1_RECT_L{0};
    return S_OK;
}