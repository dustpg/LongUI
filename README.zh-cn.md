## LongUI
 C++ 轻量级 Direct2D GUI 库/框架  
 
 文档总是英文是最新的, 中文文档一般是一段时间再更上
   

## 项目列表
  
  - 3rdparty/*, 第三方库, 比如dlmalloc
  - Demos/* ,示例  
    - Step by Step  
      1. helloworld -- 如何构建一个基本的LongUI程序
      2. handleevent -- 如何使用回调函数处理基本事件
      3. handleeventex -- 如何使用其他方法处理事件
      4. basiconfigure -- 如何写一个基本的configure 
  - Helper/* , helper项目  
    1.  ConsoleHelper -- 日志调试小工具
    2.  ScriptInterfaceGenerator -- 脚本接口绑定小工具
  - LongUI -- 被设计为静态链接的核心项目
  - TestUI -- 测试项目, 仅仅面向于自己, 用于新特性/调试
  
## License
  
  - LongUI MIT License
  - pugixml parser - version 1.5 (MIT License)
  - dlmalloc-2.8.6 (public domain)
  - ~~Scintilla-3.5.3 (License for Scintilla)~~
  - 更多细节请参考[License.txt](./License.txt)