#include <cstring>
#include <effect/ui_effect_backimage.h>
#ifndef LUI_DISABLE_STYLE_SUPPORT
#include "../private/ui_private_effect.h"
#include <cassert>

// GUID for back image
extern "C" const GUID GUID_LongUIEffect_BackImage = {
    0xa175c2e0, 0x5fb9, 0x4837, { 0x9a, 0xb2, 0x6c, 0x49, 0xf5, 0x6b, 0x43, 0xd9 }
};

// longui namespace
namespace LongUI {
    // back image ps id -rd
    static const GUID s_backrd_ps = {
        0xb3ec8ec7, 0x2da, 0x41b9, { 0xbc, 0x93, 0x71, 0x9c, 0x30, 0x7d, 0x31, 0xc1 } 
    };
    // back image ps id - sp
    static const GUID s_backsp_ps = {
        0x5a102419, 0x773, 0x4512, { 0xac, 0x46, 0x29, 0xc, 0xd1, 0x88, 0x11, 0xf5 }
    };

    // impl namespace
    namespace impl {
        // get back-round image ps
        auto get_backrd_image_ps(const unsigned char* & ptr) noexcept->uint32_t;
        // get back-space image ps
        auto get_backsp_image_ps(const unsigned char* & ptr) noexcept->uint32_t;
    }
    // back image effect
    class CUIEffectBackImage final : public CUIBaseEffectD2D {
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
            // 载入ps文件
            if (SUCCEEDED(hr)) {
                // 检查是否已经可以
                const auto code = pEffectContext->LoadPixelShader(s_backrd_ps, lbuffer, 0);
                // 失败时载入
                if (FAILED(code)) {
                    const unsigned char* buffer = nullptr;
                    const auto length = impl::get_backrd_image_ps(buffer);
                    hr = pEffectContext->LoadPixelShader(s_backrd_ps, buffer, length);
                }
            }
            // 载入ps文件
            if (SUCCEEDED(hr)) {
                // 检查是否已经可以
                const auto code = pEffectContext->LoadPixelShader(s_backsp_ps, lbuffer, 0);
                // 失败时载入
                if (FAILED(code)) {
                    const unsigned char* buffer = nullptr;
                    const auto length = impl::get_backsp_image_ps(buffer);
                    hr = pEffectContext->LoadPixelShader(s_backsp_ps, buffer, length);
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
            const auto& id = m_cbuffer.matrix.flag[0] ? s_backsp_ps : s_backrd_ps;
            m_pDrawInfo->SetPixelShader(id);
            // 更新常量缓存
            HRESULT hr = S_OK;
            if (m_bCBufferChanged) {
                m_bCBufferChanged = false;
                const auto ptr = reinterpret_cast<BYTE*>(&m_cbuffer);
                const auto len = sizeof(m_cbuffer);
                hr = m_pDrawInfo->SetPixelShaderConstantBuffer(ptr, len);
            }
            return hr;
        }
    public:
        // get matrix
        void GetMatrix(Effect::BackImageMatrix& o) const noexcept {
            o = m_cbuffer.matrix; 
        }
        // set matrix
        void SetMatrix(const Effect::BackImageMatrix& i) noexcept {
            m_cbuffer.matrix = i; 
            m_szDraw.width = static_cast<long>(i.output.width);
            m_szDraw.height = static_cast<long>(i.output.height);
            m_bCBufferChanged = true;
        }
#if 0
        // set draw
        void SetDraw(uint32_t i) noexcept {
            m_cDraw = static_cast<uint16_t>(i);
            //m_bFillChanged = true;
        }
#endif
    public:
        // ctor
        CUIEffectBackImage() noexcept {
            std::memset(&m_cbuffer, 0, sizeof(m_cbuffer));
        }
        // dtor
        ~CUIEffectBackImage() noexcept = default;
    protected:
        // vertex buffer
        //ID2D1VertexBuffer*              m_pVertexBuffer = nullptr;
        // cbuffer
        struct CBuffer {
            // matrix
            Effect::BackImageMatrix     matrix;
            // cbuffer
        }                               m_cbuffer;
        // cbuffer changed
        bool                            m_bCBufferChanged = false;
        // fill changed
        //bool                            m_bFillChanged = false;

    };
}

/// <summary>
/// Releases the back image.
/// </summary>
/// <returns></returns>
void LongUI::Effect::ReleaseBackImage() noexcept {
}

/// <summary>
/// Registers the back image.
/// </summary>
/// <returns></returns>
auto LongUI::Effect::RegisterBackImage(void* factory) noexcept -> Result {
    auto& factory_d2d = *reinterpret_cast<ID2D1Factory1*>(factory);
    // Xml描述
    const auto xml = u8R"xml(<?xml version="1.0"?>
<Effect>
  <Property name="DisplayName" type="string" value="BackImageEffect"/>
  <Property name="Author" type="string" value="dustpg"/>
  <Property name="Category" type="string" value="Transform"/>
  <Property name="Description" type="string" value="draw back-image"/>
  <Inputs><Input name="Source"/></Inputs>
  <Property name="Matrix" type="blob">
    <Property name="DisplayName" type="string" value="matrix"/>
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
            ID2D1EffectImpl* obj = new(std::nothrow) CUIEffectBackImage;
            *effect = obj;
            return obj ? S_OK : E_OUTOFMEMORY;
        };
    };
    // 短名
    using effect_t = CUIEffectBackImage;
    using param0_t = Effect::BackImageMatrix;
    // 属性绑定
    D2D1_PROPERTY_BINDING bindings[] = {
        LUI_D2D1_VALUE_TYPE_BINDING(effect_t, param0_t, Matrix),
    };
    // 注册
    const auto hr = factory_d2d.RegisterEffectFromString(
        GUID_LongUIEffect_BackImage,
        xml_buffer,
        bindings, sizeof(bindings) / sizeof(bindings[0]),
        CreateHelper::Create
    );
    return  { hr };
}

#endif // !LUI_DISABLE_STYLE_SUPPORT
