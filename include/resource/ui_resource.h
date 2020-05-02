#pragma once
//#pragma interface

#include "../luiconf.h"
// ui
#include "../core/ui_node.h"
#include "../core/ui_object.h"
#include "../typecheck/int_by_size.h"
// c++
#include <cstdint>


#ifndef LUI_MULTIPLE_RESOURCE_OVERRIDE
#ifdef LUI_MULTRES_OVERRIDE
#define LUI_MULTRES_OVERRIDE override
#else
#define LUI_MULTRES_OVERRIDE
#endif
#endif

// ui namespace
namespace LongUI {
    // Shared Resource Object
    class CUISharedResource;
    // resource manager
    class CUIResMgr;
    // resource type
    enum class ResourceType : uhalfptr_t {
        // unknown type
        Type_Unknown = 0,
        // image type
        Type_Image,
    };
    // Resource Data
    struct ResourceData {
        // utf-8 string of uri
        const char*         uri;
#ifdef LUI_MULTIPLE_RESOURCE
        // ref-count
        uhalfptr_t          ref;
        // type of resource
        ResourceType        type;
        // get type
        auto GetType() const noexcept { return type; }
#else
        uint32_t            ref;
        // get type
        auto GetType() const noexcept { return ResourceType::Type_Image; }
#endif
    };
    // Shared Resource
#ifdef LUI_MULTIPLE_RESOURCE
    class PCN_NOVTABLE CUISharedResource :
        protected IUIDestroyable {
#else
        class PCN_NOVTABLE CUISharedResource {
#endif
    protected:
        // res-data
        ResourceData                    m_data /*= { 0 }*/;
    public:
        // Private impl
        struct Private;
        // ref data
        auto&RefData() const noexcept { return m_data; }
        // def ctor
        CUISharedResource() noexcept  = default;
        // def dtor
        ~CUISharedResource() noexcept = default;
        // no move ctor
        CUISharedResource(CUISharedResource&&) noexcept = delete;
        // no copy ctor
        CUISharedResource(const CUISharedResource&) noexcept = delete;
    };
}