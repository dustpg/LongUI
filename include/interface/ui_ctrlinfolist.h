#pragma once

// VECTOR
#include "../container/pod_vector.h"
#include "../core/ui_ctrlmeta.h"

// ui namespace
namespace LongUI {
    // control info list
    struct ControlInfoList : POD::Vector<const MetaControl*> {};
}