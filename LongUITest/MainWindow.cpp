#include "stdafx.h"
#include "included.h"


// 构造函数
MainWindow::MainWindow(pugi::xml_node node, LongUI::UIWindow* p) :Super(node, p) {

}


// 析构函数
MainWindow::~MainWindow(){

}

// UAC 按钮按下
bool MainWindow::OnUACButtonOn(UIControl * sender) {
    // 获取UAC权限
    register auto re = LongUI::CUIManager::TryElevateUACNow();
    return re;
}

// 关闭窗口
void MainWindow::Close() noexcept {
    // 析构对象
    //operator delete(this, this);
    this->~MainWindow();
    UIManager.Exit();
}

// 事件
bool MainWindow::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // if sender is valid, it's some longui events need window to handle
    // 如果sender有效, 说明需要处理longui消息
    if (arg.sender){
        bool done = false;
        switch (arg.event)
        {
        case LongUI::Event::Event_ButtoClicked:
            UIManager << DL_Hint << L"Button Clicked!@" << arg.sender->GetNameStr() << LongUI::endl;
            done = true;
            break;
        //case LongUI::Event::Event_FinishedTreeBuliding:
            //this->SetEventCallBackT(L"uac", LongUI::Event::Event_ButtoClicked, &MainWindow::OnUACButtonOn);
        }

        if(done) return true;
        // 其他消息由父类处理
    }
    // if null,it's a system message, you can handle msg that you interested in, 
    // or, send it to super class, super class will handle it
    // 否则, 证明为系统消息, 你可以处理一些感兴趣的消息,
    // 或者, 交给父类处理
    return Super::DoEvent(arg);
}