#include <core/ui_manager.h>
#include <control/ui_label.h>
#include <control/ui_viewport.h>
#include <interface/ui_default_config.h>


extern "C" {
#include "v7.h"
}

void *demo_dlsym(void *handle, const char *name) noexcept;

const auto xul = u8R"xml(
<?xml version="1.0"?>
<window clearcolor="white" title="lui demo">
<script>
<![CDATA[

function test()
{
  print("test");
}

]]>
</script>
  <spacer flex="1"/>
  <hbox>
    <spacer flex="1"/>
    <label value="hellow world!"/>
    <spacer flex="1"/>
  </hbox>
  <spacer flex="1"/>
</window>
)xml";

using namespace LongUI;
class JavascriptConfig : public CUIDefaultConfigure {
private:
    static void init(CUIWindow* data) {
        if (const auto v7 = ::v7_create()) {
            data->custom_script = v7;
            const auto js = u8R"js(
)js";
        }
    };
    PCN_NOINLINE
    static auto ensure(CUIWindow* data) {
        struct v7* v7 = nullptr;
        if (data) {
            if (!data->custom_script) init(data);
            v7 = static_cast<struct v7*>(data->custom_script);
        }
        return v7;
    }
public:
    // run a section script for event
    bool Evaluation(ScriptUI data, UIControl& c) noexcept override {
        const auto v7 = ensure(c.GetWindow());
        if (!v7) return false;
        v7_val_t result = 0;
        const auto code = reinterpret_cast<const char*>(data.script);
        const auto rcode = ::v7_exec(v7, code, &result);
        return true;
    }
    // alloc the script memory and copy into(may be compiled into byte code)
    auto AllocScript(U8View v) noexcept->ScriptUI override {
        const size_t len = v.second - v.first + 1;
        const auto data = static_cast<uint8_t*>(std::malloc(len));
        if (!data) return {};
        std::memcpy(data, v.first, len - 1);
        data[len - 1] = 0;
        return { data , len + 1 };
    }
    // free the script memory
    void FreeScript(ScriptUI data) noexcept override {
        const void* ptr = data.script;
        std::free(const_cast<void*>(ptr));
    }
    // eval script for window init
    void Evaluation(U8View view, CUIWindow& win) noexcept override {
        CUIStringU8 string(view);
        if (const auto v7 = ensure(&win)) {
            v7_val_t result = 0;
            const auto rcode = ::v7_exec(v7, string.c_str(), &result);
            result = 0;
        }
    }
    // finalize window script if has script
    void FinalizeScript(CUIWindow& win) noexcept override {
        const auto v7 = static_cast<struct v7*>(win.custom_script);
        win.custom_script = nullptr;
        ::v7_destroy(v7);
    }
};

int main() {
    JavascriptConfig config;
    if (UIManager.Initialize(&config)) {
        LongUI::UIViewport viewport;
        viewport.SetXul(xul);
        viewport.GetWindow()->ShowWindow();
        viewport.GetWindow()->Exec();
    }
    UIManager.Uninitialize();
    return 0;
}


void *demo_dlsym(void *handle, const char *name) noexcept  {
    //if (strcmp(name, "foo") == 0) return foo;
    return nullptr;
}
