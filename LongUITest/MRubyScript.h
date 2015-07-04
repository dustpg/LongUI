#pragma once


// MRuby脚本
class MRubyScript final : public LongUI::IUIScript {
    // 参数
    /*enum ArgumentIndex : uint32_t {
        // 事件类型
        Index_UIEventType = 0,
        // 控件指针
        Index_ControlPointer,
        // 索引大小
        Index_Count,
    };*/
public:
    // 构造函数
    MRubyScript() noexcept;
    // 内联析构函数
    ~MRubyScript() noexcept {}
    // 基本接口
    LONGUI_BASIC_INTERFACE_IMPL;
public:
    // 运行脚本
    virtual auto Evaluation(const LongUI::UIScript, const LongUI::EventArgument&) noexcept->size_t;
    // 获取配置信息
    virtual auto GetConfigInfo() noexcept->LongUI::ScriptConfigInfo { return LongUI::Info_None; };
    // 初始化类
    virtual auto Initialize(LongUI::CUIManager*) noexcept ->bool ;
    // 反初始化
    virtual auto UnInitialize() noexcept->void;
    // 申请并填写脚本空间
    virtual auto AllocScript(const char*) noexcept->LongUI::UIScript ;
    // 释放脚本空间
    virtual auto FreeScript(LongUI::UIScript&) noexcept->void ;
public:
    // 获取类ID
    auto GetClassID(const char*) noexcept->size_t;
private:
    // 定义 API 接口
    bool define_api() noexcept;
private:
    // 当前窗口
    static HWND s_hNowWnd;
    // API.msg_box
    static auto MsgBox(mrb_state *mrb, mrb_value self) noexcept->mrb_value;
private:
    // LongUI 管理器
    LongUI::CUIManager*             m_pUIManager = nullptr;
    // MRuby 状态(虚拟机)
    mrb_state*                      m_pMRuby = nullptr;
    // 全局参数符号
    mrb_sym                         m_symArgument = 0;
};