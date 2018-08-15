#include <cstring>
#include <effect/ui_effect_borderimage.h>
#ifndef LUI_DISABLE_STYLE_SUPPORT
#include "../private/ui_private_effect.h"
#include <cassert>

// GUID for border image
extern "C" const GUID GUID_LongUIEffect_BorderImage = {
    0xe940bdb6, 0xeb65, 0x4017,{ 0x9d, 0x67, 0x18, 0xd0, 0x14, 0xd7, 0x3e, 0x95 }
};


/*

1 2 3
4 9 5
6 7 8

[0, 0]          [1, 0] [1, 0]          [1, 0] [1, 0]          [1, 0]
[0, 0] -------- [0, 0] [0, 0] -------- [1, 0] [1, 0] -------- [1, 0]
[0, 0]          [0, 0] [0, 0]          [0, 0] [0, 0]          [1, 0]
  |00             01|    |01            05|     |05            15|
  |       1         |    |       2        |     |       3        |
  |02             03|    |03            07|     |07            17|
[0, 1]          [1, 1] [1, 1]          [1, 1] [1, 1]          [1, 1]
[0, 0] -------- [0, 0] [0, 0] -------- [1, 0] [1, 0] -------- [1, 0]
[0, 0]          [0, 0] [0, 0]          [0, 0] [0, 0]          [1, 0]

[0, 1]          [1, 1]
[0, 0] -------- [0, 0] 
[0, 0]          [0, 0] 
  |02             03|     03   +4    07      07     +16    17
  |       4         |           9                    5
  |0A             0B|     0B   +4    0F      0F     +16    1F
[0, 1]          [1, 1] 
         +32
  |0A             0B|     0B   +4    0F      0F     +16    1F
  |       4         |           9                    5
  |2A             2B|     2B   +4    2F      2F     +16    3F

*/

// longui namespace
namespace LongUI {
    // common data
    static ID2D1VertexBuffer*   s_pCommonVertexBuffer;
    // Vertex data
    struct BorderImageVertex {
        uint8_t   pos0_x;
        uint8_t   pos0_y;
        uint8_t   pos1_x;
        uint8_t   pos1_y;
        uint8_t   pos2_x;
        uint8_t   pos2_y;
        uint8_t   tex0_x;
        uint8_t   tex0_y;
        uint16_t  id;
        uint16_t  id2;
    };
    // make data
    static constexpr BorderImageVertex mk(unsigned i, uint16_t id) {
        return {
            (i & (1 << 0)) ? uint8_t(255) : uint8_t(0),
            (i & (1 << 1)) ? uint8_t(255) : uint8_t(0),
            (i & (1 << 2)) ? uint8_t(255) : uint8_t(0),
            (i & (1 << 3)) ? uint8_t(255) : uint8_t(0),
            (i & (1 << 4)) ? uint8_t(255) : uint8_t(0),
            (i & (1 << 5)) ? uint8_t(255) : uint8_t(0),
            (i & (1 << 6)) ? uint8_t(255) : uint8_t(0),
            (i & (1 << 7)) ? uint8_t(255) : uint8_t(0),
            uint16_t(id % 3), uint16_t(id / 3)
        };
    }
    // data 
    static const BorderImageVertex BIVB[] = {
        mk(0x41, 0), mk(0x00, 0), mk(0xc3, 0), mk(0xc3, 0), mk(0x00, 0), mk(0x82, 0),
        mk(0x45, 1), mk(0x01, 1), mk(0xc7, 1), mk(0xc7, 1), mk(0x01, 1), mk(0x83, 1),
        mk(0x55, 2), mk(0x05, 2), mk(0xd7, 2), mk(0xd7, 2), mk(0x05, 2), mk(0x87, 2),
        mk(0x43, 3), mk(0x02, 3), mk(0xcb, 3), mk(0xcb, 3), mk(0x02, 3), mk(0x8a, 3),

        mk(0x57, 5), mk(0x07, 5), mk(0xdf, 5), mk(0xdf, 5), mk(0x07, 5), mk(0x8f, 5),
        mk(0x4b, 6), mk(0x0a, 6), mk(0xeb, 6), mk(0xeb, 6), mk(0x0a, 6), mk(0xaa, 6),
        mk(0x4f, 7), mk(0x0b, 7), mk(0xef, 7), mk(0xef, 7), mk(0x0b, 7), mk(0xab, 7),
        mk(0x5f, 8), mk(0x0f, 8), mk(0xff, 8), mk(0xff, 8), mk(0x0f, 8), mk(0xaf, 8),

        mk(0x47, 4), mk(0x03, 4), mk(0xcf, 4), mk(0xcf, 4), mk(0x03, 4), mk(0x8b, 4),
    };
    // border image vs id
    static const GUID s_resource_vs = {
        0x917698dd, 0x43ca, 0x4a6c,{ 0xaa, 0x68, 0x70, 0x94, 0xf3, 0xa3, 0xf1, 0x60 }
    };
    // border image ps id
    static const GUID s_resource_ps = {
        0x917698dd, 0x43ca, 0x4a6c,{ 0xaa, 0x68, 0x70, 0x94, 0xf3, 0xa3, 0xf1, 0x61 }
    };
    // impl namespace
    namespace impl {
        // get border image vs
        auto get_border_image_vs(const unsigned char* & ptr) noexcept->uint32_t;
        // get border image vs
        auto get_border_image_ps(const unsigned char* & ptr) noexcept->uint32_t;
    }
    // border image effect
    class CUIEffectBorderImage final : public CUIBaseEffectD2D {
    public:
        // RELEASE
        IFACEMETHODIMP_(ULONG) Release() noexcept override final {
            const auto code = --m_cRefCount;
            if (!code) delete this;
            return code;
        }
        // INITIALIZE
        IFACEMETHODIMP Initialize(
            ID2D1EffectContext* pEffectContext,
            ID2D1TransformGraph* pTransformGraph) noexcept override final {
            // 参数检查
            assert(pEffectContext && pTransformGraph && "bad arguments");
            if (!pEffectContext || !pTransformGraph) return E_INVALIDARG;
            HRESULT hr = S_OK;
            BYTE lbuffer[4];
            const unsigned char* vs_buf = nullptr;
            uint32_t vs_len = 0;
            // 载入vs文件
            if (SUCCEEDED(hr)) {
                // 检查是否已经可以
                const auto code = pEffectContext->LoadVertexShader(s_resource_vs, lbuffer, 0);
                // 失败时载入
                if (FAILED(code)) {
                    vs_len = impl::get_border_image_vs(vs_buf);
                    hr = pEffectContext->LoadVertexShader(s_resource_vs, vs_buf, vs_len);
                }
            }
            // 载入顶点数据文件
            if (SUCCEEDED(hr)) {
                // 没有共有数据?
                if (!s_pCommonVertexBuffer) {
                    D2D1_VERTEX_BUFFER_PROPERTIES props = {};
                    D2D1_CUSTOM_VERTEX_BUFFER_PROPERTIES props2 = {};
                    D2D1_INPUT_ELEMENT_DESC descs[] = {
                        { "POSITION", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(BorderImageVertex, pos0_x) },
                        { "POSITION", 1, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(BorderImageVertex, pos2_x) },
                        { "INDEX",    0, DXGI_FORMAT_R16G16_UINT   , 0, offsetof(BorderImageVertex, id) },
                    };
                    //
                    props.inputCount = sizeof(BIVB) / sizeof(BIVB[0]);
                    props.usage = D2D1_VERTEX_USAGE_STATIC;
                    props.data = reinterpret_cast<const BYTE*>(BIVB);
                    props.byteWidth = sizeof(BIVB);
                    //
                    props2.shaderBufferWithInputSignature = vs_buf;
                    props2.shaderBufferSize = vs_len;
                    props2.inputElements = descs;
                    props2.elementCount = sizeof(descs) / sizeof(descs[0]);
                    props2.stride = sizeof(BorderImageVertex);
                    //
                    hr = pEffectContext->CreateVertexBuffer(
                        &props,
                        nullptr,
                        &props2,
                        &s_pCommonVertexBuffer
                    );
                }
            }
            // 载入ps文件
            if (SUCCEEDED(hr)) {
                // 检查是否已经可以
                const auto code = pEffectContext->LoadPixelShader(s_resource_ps, lbuffer, 0);
                // 失败时载入
                if (FAILED(code)) {
                    const unsigned char* buffer = nullptr;
                    const auto length = impl::get_border_image_ps(buffer);
                    hr = pEffectContext->LoadPixelShader(s_resource_ps, buffer, length);
                }
            }
            // 连接
            if (SUCCEEDED(hr)) {
                hr = pTransformGraph->SetSingleTransformNode(this);
            }
            return hr;
        }
        // PREPAREFORRENDER
        IFACEMETHODIMP PrepareForRender(D2D1_CHANGE_TYPE changeType) noexcept override final {
            if (changeType == D2D1_CHANGE_TYPE_NONE) return S_OK;
            // 设置像素着色器
            m_pDrawInfo->SetPixelShader(s_resource_ps);
            // 根据Fill设置顶点范围
            D2D1_VERTEX_RANGE range = { 0, m_cDraw };
            // 设置顶点处理
            m_pDrawInfo->SetVertexProcessing(
                s_pCommonVertexBuffer,
                D2D1_VERTEX_OPTIONS_NONE,
                nullptr,
                &range,
                &s_resource_vs
            );
            // 更新常量缓存
            HRESULT hr = S_OK;
            if (m_bCBufferChanged) {
                m_bCBufferChanged = false;
                const auto ptr = reinterpret_cast<BYTE*>(&m_cbuffer);
                const auto len = sizeof(m_cbuffer);
                hr = m_pDrawInfo->SetVertexShaderConstantBuffer(ptr, len);
            }
            return hr;
        }
    public:
        // get matrix
        void GetMatrix(Effect::BorderImageMatrix& o) const noexcept {
            o = m_cbuffer.matrix; }
        // get draw
        void GetDraw(uint32_t& o) const noexcept {
            o = static_cast<uint32_t>(m_cDraw);  }
        // set matrix
        void SetMatrix(const Effect::BorderImageMatrix& i) noexcept {
            m_cbuffer.matrix = i; 
            m_szDraw.width = static_cast<long>(i.zone3.left);
            m_szDraw.height = static_cast<long>(i.zone3.top);
            m_bCBufferChanged = true;
        }
        // set draw
        void SetDraw(uint32_t i) noexcept {
            m_cDraw = static_cast<uint16_t>(i);
            //m_bFillChanged = true;
        }
    public:
        // ctor
        CUIEffectBorderImage() noexcept {
            std::memset(&m_cbuffer, 0, sizeof(m_cbuffer));
        }
        // dtor
        ~CUIEffectBorderImage() noexcept = default;
    protected:
        // vertex buffer
        //ID2D1VertexBuffer*              m_pVertexBuffer = nullptr;
        // cbuffer
        struct CBuffer {
            // zone matrix
            Effect::BorderImageMatrix   matrix;
            // cbuffer
        }                               m_cbuffer;
        // fill?
        uint16_t                        m_cDraw = Effect::VERTEX_FULLCOUNT;
        // cbuffer changed
        bool                            m_bCBufferChanged = false;
        // fill changed
        //bool                            m_bFillChanged = false;

    };
}

/// <summary>
/// Releases the border image.
/// </summary>
/// <returns></returns>
void LongUI::Effect::ReleaseBorderImage() noexcept {
    LongUI::SafeRelease(s_pCommonVertexBuffer);
}

/// <summary>
/// Registers the border image.
/// </summary>
/// <returns></returns>
auto LongUI::Effect::RegisterBorderImage(void* factory) noexcept -> Result {
    auto& factory_d2d = *reinterpret_cast<ID2D1Factory1*>(factory);
    // Xml描述
    const auto xml = u8R"xml(<?xml version="1.0"?>
<Effect>
  <Property name="DisplayName" type="string" value="BorderImageEffect"/>
  <Property name="Author" type="string" value="dustpg"/>
  <Property name="Category" type="string" value="Transform"/>
  <Property name="Description" type="string" value="draw border-image"/>
  <Inputs><Input name="Source"/></Inputs>
  <Property name="Matrix" type="matrix5x4">
    <Property name="DisplayName" type="string" value="matrix"/>
  </Property>
  <Property name="Draw" type="uint32">
    <Property name="DisplayName" type="string" value="draw"/>
  </Property>
</Effect>)xml";
    constexpr int len = 720;
    assert(std::strlen(xml) < len);
    wchar_t xml_buffer[len];
    {
        auto ritr = xml; auto witr = xml_buffer;
        while (const auto ch = *ritr) {
            *witr = *ritr;
            ++ritr; ++witr;
        }
        *witr = 0;
    }
    // 创建
    struct CreateHelper {
        static HRESULT WINAPI Create(IUnknown** effect) noexcept {
            assert(effect && "bad argment");
            ID2D1EffectImpl* obj = new(std::nothrow) CUIEffectBorderImage;
            *effect = obj;
            return obj ? S_OK : E_OUTOFMEMORY;
        };
    };
    // 短名
    using effect_t = CUIEffectBorderImage;
    using param0_t = Effect::BorderImageMatrix;
    using param1_t = uint32_t;
    static_assert(sizeof(param0_t) == sizeof(D2D1_MATRIX_5X4_F), "matrix5x4");
    // 属性绑定
    D2D1_PROPERTY_BINDING bindings[] = {
        LUI_D2D1_VALUE_TYPE_BINDING(effect_t, param0_t, Matrix),
        LUI_D2D1_VALUE_TYPE_BINDING(effect_t, param1_t, Draw),
    };
    // 注册
    const auto hr = factory_d2d.RegisterEffectFromString(
        GUID_LongUIEffect_BorderImage,
        xml_buffer,
        //bindings, sizeof(bindings) / sizeof(bindings[0]),
        nullptr, 0,
        CreateHelper::Create
    );
    return  { hr };
}

#endif // !LUI_DISABLE_STYLE_SUPPORT
