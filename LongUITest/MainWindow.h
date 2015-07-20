#pragma once



// test window 测试窗口
class MainWindow final : public LongUI::UIWindow {
    // super class -- 父类申明
    typedef LongUI::UIWindow Super;
public:
    // Render 渲染!
    //virtual HRESULT Render() noexcept override;
    // do event 事件处理
    virtual bool DoEvent(const LongUI::EventArgument&) noexcept override;
    // recreate 重建
    //virtual HRESULT Recreate(LongUIRenderTarget*) noexcept override;
    // close this control 关闭控件
    virtual void Cleanup() noexcept override;
public:
    // 构造函数
    MainWindow(pugi::xml_node , LongUI::UIWindow* );
    // 析构函数
    ~MainWindow();
    // delete this method , we donn't need it 删除复制构造
    MainWindow(const MainWindow&) = delete;
private:
    // UAC 按钮按下
    bool OnUACButtonOn(UIControl* sender);
private:
    // your own data
    void*           m_pNameless = nullptr;
    // your own data
    void*           m_pNameless1 = nullptr;
    size_t          m_u1 = 123;
    size_t          m_u2 = 456;
    size_t          m_u3 = 789;
};