## LongUI
  
Lightweight C++ GUI Library

Current Version : **Alpha-Stage**

## Features in LongUI
  
**Light Weight**
  - main target on LongUI
  - static-link framework, helloworld just 150kb now
  - for this, **no c++ exception** in LongUI, becareful exception in LongUI
  - you should catch exception in *noexcept* callback if want to use

**XML User Interface Language**
  - partially compatible Mozilla's XUL (XML part)
  - partially compatible CSS to get custom look
  - built-in script interface to support your own script-lang
  - more detail, see [XUL](https://developer.mozilla.org/en-US/docs/Mozilla/Tech/XUL)

**High-DPI Support**
  - dynamic dpi monitor(e.g: 2 different dpi monitor) support

**Accessible**
  - support accessible even it's light-weight

**Hardware Acceleration**
  - gpu render based on direct2d now
  - dynamic gpu change support(like surface book)
  - hardware acceleration layered window support(on win8.1 and higher)

**Freedom License**
  - the code is almost all written by myself, and under MIT License
  - no 3rd-party-code, as much as possible
  
## License
  - LongUI under MIT License
  - more detail, see [License.txt](./License.txt) 

---

## LongUI
  
C++轻量级GUI库

当前版本 : **Alpha-Stage**

## LongUI特点

**轻量级**
  - 这是LongUI的主要实现目标
  - 设计为静态库, 目前helloworld仅仅150kb
  - 为此, LongUI**没有使用C++异常**, 请小心使用C++异常
  - 其中, 应该在LongUI的回调中catch掉所有异常

**XML User Interface Language**
  - 部分兼容 Mozilla的XUL (XML部分)
  - 部分兼容 CSS 用来显示自定义的样式
  - 内建的脚本接口用来支持自定义的脚本语言
  - 更多XUL相关内容: [查看这里](https://developer.mozilla.org/en-US/docs/Mozilla/Tech/XUL)

**高DPI支持**
  - 动态支持高DPI显示器(比如两个显示器不同DPI)

**Accessible**
  - 没错! 即便是轻量级还是支持了Accessible!

**硬件加速支持**
  - 目前使用基于Direct2D的GPU渲染
  - 支持动态切换显卡(比如surface book, 微软SB)
  - 支持硬件加速的分层(可以透明)窗口(需要Win8.1以及以上)

**自由的License**
  - 代码几乎全是自己写的, 并且是MIT协议
  - 没有第三方代码(尽量)
  
## License
  - LongUI是基于 MIT License的
  - 更多细节请查看[License.txt](./License.txt) 