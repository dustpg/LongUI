#pragma once
#pragma interface

// ui
#include "../core/ui_object.h"
#include "../typecheck/int_by_size.h"
// c++
#include <cstdint>

// ui namespace
namespace LongUI {
    // Shared Resource Object
    class CUISharedResource;
    // resource manager
    class CUIResMgr;
    // resource type
    enum class ResourceType : uhalfptr_t {
        // custom type
        Type_Custom = 0,
        // image type
        Type_Image,
    };
    // Resource Data
    struct ResourceData {
        // shared object
        CUISharedResource*  obj;
        // utf-8 string of uri
        const char*         uri;
        // ref-count
        uhalfptr_t          ref;
        // type of resource
        ResourceType        type;
    };
    // Shared Resource
    class PCN_NOVTABLE CUISharedResource : 
        protected IUIDestroyable,
        protected CUISmallObject {
        // friend class
        friend class CUIResMgr;
    public:
        // render
    public:
        // def ctor
        CUISharedResource() noexcept = default;
        // def dtor
        ~CUISharedResource() noexcept = default;
        // no move ctor
        CUISharedResource(CUISharedResource&&) noexcept = delete;
        // no copy ctor
        CUISharedResource(const CUISharedResource&) noexcept = delete;
    };
}