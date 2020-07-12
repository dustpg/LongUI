// uii
#include <resource/ui_image_res.h>
#include <resource/ui_resource_id.h>
#include <core/ui_manager.h>
// c++
#include <cassert>


// longui namespace
namespace LongUI {
    // remove res info
    namespace impl { void remove_res_only(const char*) noexcept; }
    // private impl
    struct CUISharedResource::Private {
        // AddRef
        static void AddRef(CUISharedResource& sr) noexcept { sr.m_data.ref++; };
        // AddRef
        static void Release(CUISharedResource&) noexcept;
    };
    /// <summary>
    /// Releases the specified .
    /// </summary>
    /// <param name="">The .</param>
    /// <returns></returns>
    void CUISharedResource::Private::Release(CUISharedResource& sr) noexcept {
        assert(sr.m_data.ref);
        // 引用计数归为0
        if (--sr.m_data.ref == 0) {
            // 字符映射表中删除
            impl::remove_res_only(sr.m_data.uri);
            // 释放对象
#ifdef LUI_MULTIPLE_RESOURCE
            switch (sr.m_data.GetType())
            {
            case ResourceType::Type_Image:
                static_cast<CUIImage&>(sr).Destroy();
                break;
            }
#else
            assert(sr.m_data.GetType() == ResourceType::Type_Image);
            static_cast<CUIImage&>(sr).Destroy();
#endif
        }
    }
    /// <summary>
    /// Objects the identifier.
    /// </summary>
    /// <param name="ptr">The PTR.</param>
    /// <returns></returns>
    auto CUIResourceID::ObjectId(CUISharedResource * ptr) noexcept -> uintptr_t {
        return reinterpret_cast<uintptr_t>(ptr);
    }
    /// <summary>
    /// Objects the specified .
    /// </summary>
    /// <param name="id">The identifier.</param>
    /// <returns></returns>
    auto CUIResourceID::Object(uintptr_t id) noexcept -> CUISharedResource * {
        return reinterpret_cast<CUISharedResource*>(id);
    }
    // detail namesapce
    namespace impl {
        // release the res
        void release_res(uintptr_t handle) noexcept {
            CUISharedResource::Private::Release(*CUIResourceID::Object(handle));
        }
        // release the res
        void add_ref(uintptr_t handle) noexcept {
            CUISharedResource::Private::AddRef(*CUIResourceID::Object(handle));
        }
    }
}

/// <summary>
/// Finalizes an instance of the <see cref="CUIResourceID"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIResourceID::~CUIResourceID() noexcept {
    using SRR = CUISharedResource::Private;
    if (m_id) SRR::Release(*CUIResourceID::Object(m_id));
#ifndef NDEBUG
    m_id = uintptr_t(-1);
#endif
}

PCN_NOINLINE
/// <summary>
/// Sets the identifier.
/// </summary>
/// <param name="id">The identifier.</param>
/// <returns></returns>
void LongUI::CUIResourceID::SetId(uintptr_t id) noexcept {
    using SRR = CUISharedResource::Private;
    // 先加
    if (id) SRR::AddRef(*CUIResourceID::Object(id));
    // 后减
    if (m_id) SRR::Release(*CUIResourceID::Object(m_id));
    // 最后赋值
    m_id = id;
}
