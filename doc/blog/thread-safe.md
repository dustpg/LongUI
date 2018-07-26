### 线程安全

LongUI默认(应该)拥有两条线程: UI线程与渲染线程.

UIControl::Update和Render总是在渲染线程中调用, 这也是设计 UIControl::Update的原因: 在渲染前一次性计算需要渲染数据, 然后渲染. 同时也是UIControl::Render是const的原因: 别™改了

LongUI拥有两把锁, 数据锁与渲染锁。渲染线程先进行数据刷新, 然后进行数据渲染:

```c
void one_frame() {
    data_lock();
    data_update();
    data_unlock();
    
    render_lock();
    render();
    render_unlock();
}
```

一般情况如果修改基本数据, 就应该添加数据锁; 修改渲染数据就需要渲染锁. 但是LongUI强烈建议渲染数据应该被隐藏起来: 在data_update时检测渲染数据的副本被修改了, 然后向渲染器提交新的渲染数据. 这时候只需要无脑地使用数据锁而不用担心渲染锁(除非必须及时修改渲染数据, 窗口级一般会需要)

#### 时间胶囊 Time Capsule
时间胶囊是LongUI异步处理的一种办法: 将函数包装放在未来一段时间每帧连续调用直到时间耗尽.

时间胶囊总是在渲染时, 在UI线程进行调用(默认双线程情况):

```c
void one_frame() {
    data_lock();
    data_update();
    data_unlock();
    
    signal_time_capsule();
    
    render_lock();
    render();
    render_unlock();

    wait_for_time_capsule();
}
```

可以看出时间胶囊总是在渲染时处理, 也就是说在时间胶囊中的函数无需使用数据锁. 但是也不建议使用渲染锁, 因为会强制等待, 为了线程安全,LongUI强烈建议渲染数据应该被隐藏起来(标记被修改了然后等待下帧data_update处理).可以参考UILabel::SetText的实现(经过简化):

```cpp

class UILabel {
    // ..........

    // m_text是属于渲染数据, 渲染时其他地方修改会造成线程安全隐患
    CUITextLayout           m_text;
    // m_string就是普通的字符串, 渲染时本身无用所以可以在这时其他线程安全修改
    CUIString               m_string;

    void SetText(CUIString&& s) {
        // ......
        m_bTextChanged = true;
    }

    void Update() override {
        // ......
        if (m_bTextChanged) {
            m_bTextChanged = false;
            this->on_text_changed();
        }
        // ......
    }
    
    void Render() const override {
        // ......
        m_text.Render();
    }
};

```

换句话说时间胶囊中的函数应该是无锁编程!

