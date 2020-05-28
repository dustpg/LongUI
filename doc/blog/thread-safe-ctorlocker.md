### BETA 性能

实验中的功能, 定义 LUI_USING_CTOR_LOCKER 使用隐藏的一把构造锁。未经全面测试, 这个功能默认关闭.

### 利用 UIControl::ControlMakingBegin/End 创建大量控件


为了保证正确性, 请在创建大量控件时使用``` UIControl::ControlMakingBegin ``` 和 ``` UIControl::ControlMakingEnd ```包裹创建过程, 比如``` UIControl::SetXul ```是这样实现的:

```cpp
void LongUI::UIControl::SetXul(const char* xul) noexcept {
    UIControl::ControlMakingBegin();
    CUIControlControl::MakeXul(*this, xul);
    UIControl::ControlMakingEnd();
}
```

这样做是为了避免控件创建到一半就被渲染线程调用 —— 导致部分控件上一帧创建, 部分控件下一帧创建. 里面调用的就是隐藏的锁——构造锁

### impl::ctor_lock 与 impl::ctor_unlock

前面提到了可能部分控件上一帧创建, 部分控件下一帧创建. 那么如果一个控件被卡在中间怎么处理呢? 这就是 ```impl::ctor_lock``` 与 ```impl::ctor_unlock``` 这两条函数的作用. 

如果**不能接受**一个控件在卡在构造函数, 需要在构造函数写上:

```cpp
MyControl::MyControl(UIControl* parent, const MetaControl& meta) : Super(impl::ctor_lock(parent), meta) {
    // ...
    impl::ctor_unlock();
}
```

目前 ```impl::ctor_lock```与```UIControl::ControlMakingBegin()```基本等价, ```impl::ctor_unlock```与```UIControl::ControlMakingEnd()```等价

具体是: 构造时使用构造锁, 构造独立数据; 离开时候再将独立数据移动至公共区域. 避免**构造(大量)控件阻塞渲染线程**(不只是大量, 还有例如IO操作)

```cpp
void LongUI::CUIControlControl::swap_init_list(CUILocker& locker) noexcept {
    auto& obj = cc();
    // try to lock, won't block the render thread
    if (locker.TryLock()) {
        std::swap(obj.init_list_lock_free, obj.init_list);
        locker.Unlock();
    }
}
```

**注意**, 虽然不会阻塞渲染线程, 但是如果是在消息线程上面创建的话, 自然会阻塞消息线程, 所以有特殊要求的话, 创建一个**专门的线程**来创建控件
