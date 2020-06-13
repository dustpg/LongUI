#pragma once

#include <luiconf.h>
#include <cassert>

// ui namespace
namespace LongUI {
    // meta info
    struct MetaControl;
    // meta info const point
    using MetaControlCP = const MetaControl*;
    // control info list
    struct ControlInfoList {
        // control infos end
        MetaControlCP*          end_of_list;
        // control infos
        MetaControlCP           info_list[MAX_CONTROL_TYPE_COUNT];
        // push back
        void Push(LongUI::MetaControlCP cp) noexcept {
            assert(end_of_list < info_list + MAX_CONTROL_TYPE_COUNT);
            *end_of_list = cp; ++end_of_list;
        };
    };
}
