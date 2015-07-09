#include "stdafx.h"
#include "included.h"
#include "mruby\class.h"
#include "mruby\data.h"
#include "mruby\variable.h"
#include "mruby\value.h"
#include "mruby\array.h"
#include "mruby\compile.h"
#include "mruby\dump.h"

#pragma comment(lib, "libmruby")

constexpr char* ruby_script = u8R"ruby(
module App
  # 点击测试
  def self.click_button1(arg)
    msg_box "click_button1  " + arg.to_s, "点击测试"
  end
end
)ruby";

template<const wchar_t FileName[128], 
    const wchar_t ControlName[24],
    const char ProcName[32] = "LongUICreateControl">
class LoaderTemplate {
public:
    // 构造函数
    LoaderTemplate() { 
        register LongUI::CreateControlFunction tmp = nullptr;
        if (dll && tmp = reinterpret_cast<decltype(tmp)>(
                    ::GetProcAddress(dll, ProcName)
                    )) {
            UIManager.AddS2CPair(ControlName, tmp);
        }
    }
    // 析构函数
    ~LoaderTemplate() {
        if(dll) 
            ::FreeLibrary(dll); 
        dll = nullptr 
    }
private:
    HMODULE dll = ::LoadLibraryW(FileName);
};




// MRubyScript 构造函数
MRubyScript::MRubyScript(LongUI::CUIManager& manager) noexcept : m_uiManager(manager) {
    // 打开mruby
    m_pMRuby = ::mrb_open();
    if (m_pMRuby && (m_symArgument = mrb_intern_lit(m_pMRuby, "$apparg"))) {
        // 定义全局变量做为参数
        ::mrb_gv_set(m_pMRuby, m_symArgument, mrb_fixnum_value(0));
        // 载入初始化字符串
        ::mrb_load_string(m_pMRuby, ruby_script);
        // 定义API
        this->define_api();
    }
}

// MRubyScript 析构函数
MRubyScript::~MRubyScript() noexcept {
    if (m_pMRuby) {
        ::mrb_close(m_pMRuby);
        m_pMRuby = nullptr;
    }
}

// 申请并填写脚本空间
auto MRubyScript::AllocScript(const char * str) noexcept-> LongUI::UIScript {
    LongUI::UIScript script;
    script.size = ::strlen(str) + 1;
    script.data = ::malloc(script.size);
    ::strcpy(const_cast<char*>(script.script), str);
    return script;
}

// 释放脚本空间
auto MRubyScript::FreeScript(LongUI::UIScript& script) noexcept-> void {
    if (script.data) {
        ::free(script.data);
        script.data = nullptr;
    }
}

// 获取类id
auto MRubyScript::GetClassID(const char* class_name) noexcept -> size_t {

    return 0;
}



// 执行脚本片段
auto MRubyScript::Evaluation(
    const LongUI::UIScript script,
    const LongUI::EventArgument& arg
    ) noexcept -> size_t {
    // 设置参数
    mrb_gv_set(m_pMRuby, m_symArgument, mrb_fixnum_value(static_cast<mrb_int>(arg.event)));
    // 设置窗口
    s_hNowWnd = arg.sender->GetWindow()->GetHwnd();
    // 执行
    ::mrb_load_string(m_pMRuby, script.script);
    return 0;
}



// API.msg_box
auto MRubyScript::MsgBox(mrb_state * mrb, mrb_value self) noexcept-> mrb_value {
    wchar_t buffer1[1024];
    wchar_t buffer2[1024] = L"LongUI Demo";
    // 获取参数
    char* str1 = nullptr;
    char* str2 = nullptr;
    mrb_int type = MB_OK;
    ::mrb_get_args(mrb, "z|zi", &str1, &str2, &type);
    assert(str1);
    buffer1[LongUI::UTF8toWideChar(str1, buffer1)] = 0;
    if (str2) {
        buffer2[LongUI::UTF8toWideChar(str2, buffer2)] = 0;
    }
    return ::mrb_fixnum_value(::MessageBoxW(s_hNowWnd, buffer1, buffer2, type));
}

// 定义接口
bool MRubyScript::define_api() noexcept {
    auto rclass = mrb_module_get_under(m_pMRuby, m_pMRuby->object_class, "App");
    if (rclass) {
        ::mrb_define_module_function(m_pMRuby, rclass, "msg_box", MRubyScript::MsgBox, MRB_ARGS_ARG(1, 2));
        return true;
    }
    return false;
}

// 初始化
HWND MRubyScript::s_hNowWnd = nullptr;







/*
  retrieve arguments from mrb_state.

  mrb_get_args(mrb, format, ...)

  returns number of arguments parsed.

  format specifiers:

    string  mruby type     C type                 note
    ----------------------------------------------------------------------------------------------
    o:      Object         [mrb_value]
    C:      class/module   [mrb_value]
    S:      String         [mrb_value]
    A:      Array          [mrb_value]
    H:      Hash           [mrb_value]
    s:      String         [char*,mrb_int]        Receive two arguments.
    z:      String         [char*]                NUL terminated string.
    a:      Array          [mrb_value*,mrb_int]   Receive two arguments.
    f:      Float          [mrb_float]
    i:      Integer        [mrb_int]
    b:      Boolean        [mrb_bool]
    n:      Symbol         [mrb_sym]
    d:      Data           [void*,mrb_data_type const] 2nd argument will be used to check data type so it won't be modified
    &:      Block          [mrb_value]
    *:      rest argument  [mrb_value*,mrb_int]   Receive the rest of the arguments as an array.
    |:      optional                              Next argument of '|' and later are optional.
    ?:      optional given [mrb_bool]             true if preceding argument (optional) is given.
 */

/*// directly impl
// mrb_get_args(mrb, "*", mrb_value**, int*)
MRB_API mrb_int mrb_get_args_star_directly(mrb_state *mrb, mrb_value** var, mrb_int* pl) {
    mrb_value *sp = mrb->c->stack + 1;
    int argc = mrb->c->ci->argc;
    mrb_bool opt = FALSE;
    mrb_bool given = TRUE;

    if (argc < 0) {
        struct RArray *a = mrb_ary_ptr(mrb->c->stack[1]);

        argc = a->len;
        sp = a->ptr;
    }


            if (argc > i) {
                *pl = argc - i;
                if (*pl > 0) {
                    *var = sp;
                }
                i = argc;
                sp += *pl;
            }
            else {
                *pl = 0;
                *var = NULL;
            }
    return *pl;
}*/