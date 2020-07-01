#include <core/ui_manager.h>
#include <control/ui_label.h>
#include <control/ui_button.h>
#include <control/ui_viewport.h>
#include <interface/ui_default_config.h>
extern "C" { 
#include "v7.h" 
}



const auto xul = u8R"xml(
<?xml version="1.0"?>
<window lui:clearcolor="white" title="lui demo">
<script>
<![CDATA[

function rename()
{
    document.getElementById("me").label = "已经设置";
}

]]>
</script>
  <spacer flex="1"/>
  <hbox>
    <spacer flex="1"/>
    <button id="me" label="RENAME THIS" oncommand="rename()"/>
    <spacer flex="1"/>
    <button label="MAX WINDOW" oncommand="document.maxwindow()"/>
    <spacer flex="1"/>
  </hbox>
  <spacer flex="1"/>
</window>
)xml";

using namespace LongUI;
class JavascriptConfig : public CUIDefaultConfigure {
private:
    static auto make_control(struct v7 *v7, UIControl* ctrl) noexcept {
        const auto c = ::v7_mk_object(v7);
        ::v7_set_user_data(v7, c, ctrl);
        const auto setter = ::v7_mk_cfunction([](struct v7 *v7, v7_val_t *res) noexcept {
            const auto obj = v7_get_this(v7);
            const auto data = ::v7_get_user_data(v7, obj);
            assert(data && "bad user data");
            const auto ctrl = reinterpret_cast<UIControl*>(data);
            // Button
            if (const auto btn = uisafe_cast<UIButton>(ctrl)) {
                auto arg0 = ::v7_arg(v7, 0); size_t len = 0;
                const auto str = ::v7_get_string(v7, &arg0, &len);
                btn->SetText(CUIString::FromUtf8({ str , str + len }));
            }
            return V7_OK;
        });
        const auto getter = ::v7_mk_cfunction([](struct v7 *v7, v7_val_t *res) noexcept {
            const auto obj = v7_get_this(v7);
            const auto data = ::v7_get_user_data(v7, obj);
            assert(data && "bad user data");
            const auto ctrl = reinterpret_cast<UIControl*>(data);
            // Button
            if (const auto btn = uisafe_cast<UIButton>(ctrl)) {
                auto& text = btn->RefText();
                const auto text16 = CUIStringU8::FromUtf16(text.view());
                *res = ::v7_mk_string(v7, text16.c_str(), text16.length(), 1);
            }
            return V7_OK;
        });
        v7_val_t getset = ::v7_mk_array(v7);
        ::v7_array_set(v7, getset, 0, getter);
        ::v7_array_set(v7, getset, 1, setter);
        const v7_prop_attr_desc_t prop 
            = V7_PROPERTY_NON_ENUMERABLE 
            | V7_PROPERTY_SETTER 
            | V7_PROPERTY_GETTER
            ;
        ::v7_def(v7, c, "label", 5, prop, getset);
        return c;
    }
    static void init(CUIWindow* data) noexcept {
        if (const auto v7 = ::v7_create()) {
            data->custom_script = v7;
            const auto doc = "document"_sv;
            const auto v7g = ::v7_get_global(v7);
            const auto obj = ::v7_mk_object(v7);
            ::v7_set(v7, v7g, doc.begin(), doc.size(), obj);
            ::v7_set_user_data(v7, obj, data);
            ::v7_set_method(v7, obj, "maxwindow", [](struct v7 *v7, v7_val_t *res) noexcept {
                const auto obj = v7_get_this(v7);
                const auto data = ::v7_get_user_data(v7, obj);
                assert(data && "bad user data");
                const auto window = reinterpret_cast<CUIWindow*>(data);
                window->MaxWindow();
                return V7_OK;
            });
            ::v7_set_method(v7, obj, "getElementById", [](struct v7 *v7, v7_val_t *res) noexcept {
                const auto obj = v7_get_this(v7);
                const auto data = ::v7_get_user_data(v7, obj);
                assert(data && "bad user data");
                const auto window = reinterpret_cast<CUIWindow*>(data);
                auto arg0 = ::v7_arg(v7, 0); size_t len = 0;
                const auto id = ::v7_get_string(v7, &arg0, &len);
                const auto control = window->FindControl({ id, id + len });
                if (control) *res = make_control(v7,control);
                return V7_OK;
            });
            
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
    bool Evaluation(ScriptUI data, const GuiEventArg& arg) noexcept override {
        const auto v7 = ensure(arg.current->GetWindow());
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

