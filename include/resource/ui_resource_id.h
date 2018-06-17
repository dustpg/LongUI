#pragma once

// ui
#include "ui_resource.h"

// c++
#include <cstdint>

// ui namespace
namespace LongUI {
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
        // get id
        auto GetId() const noexcept { return m_id; }
        // set a new id
        void SetId(uint32_t id) noexcept;
        // get resource data
        auto GetResoureceData() const noexcept -> const ResourceData&;
        // get resource uri
        auto GetResoureceUri() const noexcept { return GetResoureceData().uri; }
        // get resource obj
        auto GetResoureceObj() const noexcept { return GetResoureceData().obj; }
        // get resource type
        auto GetResoureceType() const noexcept { return GetResoureceData().type; }
    private:
        // id
        uint32_t            m_id = 0;
    };
}