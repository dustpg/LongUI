#pragma once
// Script

#include "interface.h"

// impl your own script, defaulty, supported mruby and lua
enum class Script : uint32_t {
    MRuby = 0,
    Lua,
};


// code generator
template<Script script>
class Generator {
public:
    // define basic function format
    static auto BasicFunctionFormat() -> const char*;
};


// mruby generator
template<>
class Generator<Script::MRuby> {
public:
    // define basic function format
    static auto BasicFunctionFormat() -> const char* {
        /*static mrb_value [*funcname*](mrb_state * mrb, mrb_value self) {
           [*funcbody*]
        }*/
        return u8R"mruby(static mrb_value %s(mrb_state * mrb, mrb_value self) {
  %s
}
)mruby";
    }
};
