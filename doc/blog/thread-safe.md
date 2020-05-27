### 线程安全

LongUI默认(应该)拥有两条线程: UI线程与渲染线程.

UIControl::Update和Render总是在渲染线程中调用, 这也是设计 UIControl::Update的原因: 在渲染前一次性计算需要渲染数据, 然后渲染. 同时也是UIControl::Render是const的原因: **别™改了**

LongUI拥有两把锁, 数据锁与渲染锁。渲染线程先进行数据刷新, 然后进行数据渲染:

```c
void one_frame() {
    data_lock();
    update();
    data_unlock();
    
    render_lock();
    render();
    render_unlock();
}
```

一般情况如果修改基本数据, 就应该添加数据锁; 修改渲染数据就需要渲染锁. 但是LongUI强烈建议渲染数据应该被隐藏起来: **在```update```时检测渲染数据的副本被修改了, 然后向渲染器提交新的渲染数据**. 这时候只需要无脑地使用数据锁而不用担心渲染锁(除非必须及时修改渲染数据, 窗口级一般会需要)

### 时间胶囊 Time Capsule
时间胶囊是LongUI异步处理的一种办法: **将函数包装放在未来一段时间每帧连续调用直到时间耗尽**. 当然, 这个类一开始设计的时候是设计成放在未来某时刻调用一次, 所以取名为时间胶囊. 后来发现有些东西需要连续调用干脆就合并在里面了, 也**懒**得换名字.

时间胶囊总是在渲染时, **在UI线程**进行调用(默认双线程情况):

```c
void one_frame() {
    data_lock();
    update();
    data_unlock();
    
    signal_time_capsule();
    
    render_lock();
    render();
    render_unlock();

    wait_for_time_capsule();
}
```

可以看出时间胶囊总是**在渲染时处理**, 也就是说在时间胶囊中的函数无需使用数据锁. 但是也不建议使用渲染锁, 因为会强制等待. 虽然是在UI线程, 但是不允许调用堵塞(blocking)函数, 会导致渲染线程一直在``` wait_for_time_capsule() ```里面等待. 

为了线程安全, LongUI强烈建议渲染数据应该被隐藏起来(标记被修改了然后等待下帧 ```update``` 处理).可以参考UILabel::SetText的实现(经过简化):

```cpp

class UILabel {
    // ..........

    // m_text是属于渲染数据, 渲染时其他地方修改会造成线程安全隐患
    CUITextLayout           m_text;
    // m_string就是普通的字符串, 渲染时本身无用所以可以在这时其他线程安全修改
    CUIString               m_string;

    void SetText(CUIString&& s) {
        // ......
        m_string = std::move(s);
        m_bTextChanged = true;
    }

    void Update() override {
        // ......
        if (m_bTextChanged) {
            m_bTextChanged = false;
            m_text.SetText(m_string);
        }
        // ......
    }
    
    void Render() const override {
        // ......
        m_text.Render();
    }
};

```

换句话说时间胶囊中的函数应该是**无锁编程**, 安心地在时间胶囊中调用 ```UILabel::SetText``` 而不用加锁!

### 利用CUIBlockingGuiOpAutoUnlocker调用堵塞函数

LongUI仅仅是一个简单的GUI库, 不是一套完整的解决方案, 不可能像Qt那样封装全部操作系统的函数. 有时可能需要调用一些操作系统GUI函数: 最简单的比如``` MessageBox ```, 稍微复杂点的 ``` GetOpenFileName ``` , 这些都是堵塞型的.

默认情况下为了安全, 所有的GUI操作都是加了锁的. 比如点击按钮弹出``` MessageBox ```, 点击时上了数据锁, 导致整个msgbox都在数据上锁的状态, 所以为了渲染线程能够安全运行就封装了``` CUIBlockingGuiOpAutoUnlocker ```

大概应该这么用:

```cpp
void call() {
    // ......
    int rv_msgbox = 0;
    {
        CUIBlockingGuiOpAutoUnlocker unlocker;
        const auto text = L"ASDDSA";
        const auto capt = L"QWEEWQ";
        rv_msgbox = ::MessageBoxW(window->GetHwnd(), text, capt, MB_YESNO);
    }
    // ......
}
```

或者用上匿名表达式减少``` rv_msgbox ```中间状态(const anywhere):

```cpp
void call() {
    // ......
    const auto rv_msgbox = [window]() {
        CUIBlockingGuiOpAutoUnlocker unlocker;
        const auto text = L"ASDDSA";
        const auto capt = L"QWEEWQ";
        return ::MessageBoxW(window->GetHwnd(), text, capt, MB_YESNO);
    }();
    // ......
}
```

### 利用 UIControl::ControlMakingBegin/End 创建大量控件


为了保证正确性, 请在创建大量控件时使用``` UIControl::ControlMakingBegin ``` 和 ``` UIControl::ControlMakingEnd ```包裹创建过程, 比如``` UIControl::SetXul ```是这样实现的:

```cpp
void LongUI::UIControl::SetXul(const char* xul) noexcept {
    UIControl::ControlMakingBegin();
    CUIControlControl::MakeXul(*this, xul);
    UIControl::ControlMakingEnd();
}
```

这样做是为了避免控件创建到一半就被渲染线程调用 —— 导致部分控件上一帧创建, 部分控件下一帧创建. 如果有其他更好地方法处理线程问题, 这两条函数将会删除(一个空函数壳子), 所以不建议直接调用```DataLock``` (查看LUI_BETA_CTOR_LOCKER)

### impl::ctor_lock 与 impl::ctor_unlock

前面提到了可能部分控件上一帧创建, 部分控件下一帧创建. 那么如果一个控件被卡在中间怎么处理呢? 这就是 ```impl::ctor_lock``` 与 ```impl::ctor_unlock``` 这两条函数的作用. 

如果**不能接受**一个控件在卡构造函数, 需要在构造函数写上:

```cpp
MyControl::MyControl(UIControl* parent, const MetaControl& meta) : Super(impl::ctor_lock(parent), meta) {
    // ...
    impl::ctor_unlock();
}
```

同上, 不建议直接调用```DataLock```. (默认控件中, UIBoxLayout可以接受卡在构造函数, 但是其超类UIScrollArea不能接受.) (以防万一就都加上)

### CUIWindow::~CUIWindow的超级锁

与其他地方不同的是, CUIWindow析构操作会同时上两把锁, 因为Window数据(主窗口设计上)是放在栈上需要即时处理, 析构一个窗口需要保证绝对的安全
