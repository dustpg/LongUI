#pragma once

#include "../core/ui_basic_type.h"
#include "../util/ui_ostype.h"
#include "ui_graphics_decl.h"

// longui::impl
namespace LongUI { namespace impl {
    // dcomp window buf
    struct dcomp_window_buf { void* buf[3]; };
    // init dcomp support[full support start at win8.1]
    void init_dcomp_support() noexcept;
    // check support for dcomp
    bool check_dcomp_support() noexcept;
    // uninit dcomp support
    void uninit_dcomp_support() noexcept;
    // init
    void init_dcomp(dcomp_window_buf&) noexcept;
    // create data
    auto create_dcomp(dcomp_window_buf&, HWND, I::Swapchan&) noexcept -> Result;
    // release
    void release_dcomp(dcomp_window_buf&) noexcept;
}}