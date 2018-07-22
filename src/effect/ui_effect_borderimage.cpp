#include <effect/ui_effect_borderimage.h>
#ifndef LUI_DISABLE_STYLE_SUPPORT
#include "../private/ui_private_effect.h"
#include <cassert>

// GUID for border image
extern "C" const GUID GUID_LongUIEffect_BorderImage = {
    0xe940bdb6, 0xeb65, 0x4017,{ 0x9d, 0x67, 0x18, 0xd0, 0x14, 0xd7, 0x3e, 0x95 }
};

// border image shader id
static const GUID s_shader_id ={ 
    0x917698dd, 0x43ca, 0x4a6c,{ 0xaa, 0x68, 0x70, 0x94, 0xf3, 0xa3, 0xf1, 0x60 } 
};


// longui namespace
namespace LongUI {
    // impl namespace
    namespace impl {
        // get border image
        auto get_border_image_shader(unsigned char* & ptr) noexcept->uint32_t;
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
            HRESULT hr = S_FALSE;
            // 载入shader文件
            if (SUCCEEDED(hr)) {
                BYTE buffer[4];
                // 检查是否已经可以
                const auto code = pEffectContext->LoadPixelShader(s_shader_id, buffer, 0);
                // 失败时载入
                if (FAILED(code)) {
                    unsigned char* buffer = nullptr;
                    const auto length = impl::get_border_image_shader(buffer);
                    hr = pEffectContext->LoadPixelShader(s_shader_id, buffer, length);
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
            m_pDrawInfo->SetPixelShader(s_shader_id);
            // 修改了属性/初次加入效果图(effect graph)
            if (changeType == D2D1_CHANGE_TYPE_PROPERTIES ||
                changeType == D2D1_CHANGE_TYPE_GRAPH) {
                // 更新常量缓存
                return m_pDrawInfo->SetPixelShaderConstantBuffer(
                    reinterpret_cast<BYTE*>(&m_cbuffer), sizeof(m_cbuffer)
                );
            }
            return S_OK;
        }
    public:
        // get cbuffer
        void GetCBuffer(Effect::CBufferBorderImage& o) const noexcept {
            o = m_cbuffer;
        }
        // set cbuffer
        void SetCBuffer(const Effect::CBufferBorderImage& i) noexcept {
            m_cbuffer = i; 
            m_szDraw.width = static_cast<long>(i.size.width);
            m_szDraw.height = static_cast<long>(i.size.height);
        }
    public:
        // ctor
        CUIEffectBorderImage() noexcept {
            std::memset(&m_cbuffer, 0, sizeof(m_cbuffer));
        }
        // dtor
        ~CUIEffectBorderImage() noexcept = default;
    protected:
        // cbuffer
        Effect::CBufferBorderImage      m_cbuffer;
    };
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
  <Property name="CBuffer" type="matrix4x4">
    <Property name="DisplayName" type="string" value="cbuffer"/>
  </Property>
</Effect>)xml";
    constexpr int len = 512;
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
    auto create_this = [](IUnknown** effect) noexcept ->HRESULT {
        assert(effect && "bad argment");
        ID2D1EffectImpl* obj = new(std::nothrow) CUIEffectBorderImage;
        *effect = obj;
        return obj ? S_OK : E_OUTOFMEMORY;
    };
    // 短名
    using effect_t = CUIEffectBorderImage;
    using param0_t = Effect::CBufferBorderImage;
    static_assert(sizeof(param0_t) == sizeof(D2D1_MATRIX_4X4_F), "matrix4x4");
    // 属性绑定
    D2D1_PROPERTY_BINDING bindings[] = {
        LUI_D2D1_VALUE_TYPE_BINDING(effect_t, param0_t, CBuffer)
    };
    // 注册
    const auto hr = factory_d2d.RegisterEffectFromString(
        GUID_LongUIEffect_BorderImage,
        xml_buffer,
        bindings, sizeof(bindings)/sizeof(bindings[0]),
        create_this
    );
    return  { hr };
}

#endif // !LUI_DISABLE_STYLE_SUPPORT
