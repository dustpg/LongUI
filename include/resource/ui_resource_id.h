#pragma once

// ui
#include "../luiconf.h"
#include "../core/ui_object.h"
#include "ui_resource.h"

// c++
#include <cstdint>

// ui namespace
namespace LongUI {
    // Shared Resource
    class CUISharedResource;
    // Shared Resource ID
    class CUIResourceID : public CUINoMo {
    public:
        // copy
        CUIResourceID(const CUIResourceID&) noexcept = delete;
        // ctor
        CUIResourceID() noexcept {}
        // dtor
        ~CUIResourceID() noexcept;
    public:
        // object to id
        static auto ObjectId(CUISharedResource* ptr) noexcept->uintptr_t;
        // id to object 
        static auto Object(uintptr_t) noexcept->CUISharedResource*;
    public:
        // get id
        auto GetId() const noexcept { return m_id; }
        // set a new id
        void SetId(uintptr_t id) noexcept;
        // ref resource
        auto&RefResource() noexcept { return *this->Object(m_id); }
        // ref resource
        const auto&RefResource() const noexcept { return *this->Object(m_id); }
    private:
        // id
        uintptr_t            m_id = 0;
    };
}