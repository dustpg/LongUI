#include <core/ui_manager.h>
#include <control/ui_label.h>
#include <control/ui_viewport.h>
#include <interface/ui_default_config.h>
#include "mjs.h"


using namespace LongUI;
class JavascriptConfig : public CUIDefaultConfigure {
private:
    static auto ensure(CUIWindow* data) {
        struct mjs* mjs = nullptr;
        if (data) {
            if (!data->custom_script) data->custom_script = ::mjs_create();
            mjs = static_cast<struct mjs*>(data->custom_script);
        }
        return mjs;
    }
public:
    // run a section script for event
    bool Evaluation(ScriptUI data, UIControl& c) noexcept override {
        const auto mjs = ensure(c.GetWindow());
        if (!mjs) return false;
        mjs_exec(mjs, reinterpret_cast<const char*>(data.script), nullptr);
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
        CUIStringU8 string; string = view;
        if (const auto mjs = ensure(&win))
            mjs_exec(mjs, string.c_str(), nullptr);
    }
    // finalize window script if has script
    void FinalizeScript(CUIWindow& win) noexcept override {
        const auto mjs = static_cast<struct mjs*>(win.custom_script);
        win.custom_script = nullptr;
        mjs_destroy(mjs);
    }
};

int main() {
    JavascriptConfig config;
    if (UIManager.Initialize(&config)) {
        LongUI::UIViewport viewport;
        LongUI::UILabel label{ &viewport };

        label.SetText(u"Hello world!");
        viewport.GetWindow()->ShowWindow();
        viewport.GetWindow()->Exec();
    }
    UIManager.Uninitialize();
    return 0;
}


void *my_dlsym(void *handle, const char *name) noexcept  {
    //if (strcmp(name, "foo") == 0) return foo;
    return nullptr;
}
