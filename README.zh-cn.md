## LongUI
 C++ 轻量级 Direct2D GUI 库/框架  

## Requirements
  
对于本项目:
  
  - OS: 
    - **Windows 8 或更高**
  - Tool-chain/Environment A: MSC  
    - Visual Stuidio 2015 - Express for Desktop/Community 或更高
    - Windows SDK(10.0.10240.0 RTM)
  - Tool-chain/Environment B(目测未来会支持): GCC/Clang  
    - Rakefile(ruby)
    - MinGW-W64
    
##Remarks
  
  Windows 7 also supported Direct2D 1.1, but DirectComposition, planned support
  for Win7 without DirectComposition,but MS said that Win7 could upgrade to 
  Win10 for free, so just 'planned', and, maybe Win8.1/Win10 support only in 
  the next year
  
##LongUI Programming Style
  
  - **class** :  
      - \[C/I/?\] UI ClassName, 例如. `CUIManager`, `IUIInterface `
      - 例外: 位于LongUI::XXX(Component/Helper....)下面的类
  - **member variable**:  
      - public: member_variable, 例如. `parent`, `user_data`
      - private/protected: m_typeMemberVariable, 例如, `m_pWindow`
  - **method/function**:  
      - public: ClassMethod(), 例如. `GetWindow()`, `Update()`
      - private/protected:  class_method(), 例如. `refresh_this()`  
      - 例外: 匹配 STL 风格方法: `begin()`, `end()`, `insert()`, 却是**public**
  - **enum**:  看!
```cpp
        enum class PointType : uint16_t {
            Type_None,      // None
            Type_Arrow1,    // Arrow1
            Type_Arrow2,    // Arrow2
            Type_Thumb,     // Thumb
            Type_Shaft,     // Shaft
        };
```
  - **auto variable**:  
    - 看当时的心情, 例如  int `old_x`, `oldX`, `uOldX`
    

## 项目列表
  
  - 3rdparty/*, 第三方库, 比如dlmalloc
  - Demos/* ,示例

          1. helloworld -- 创建一个基本的LongUI程序
  - Helper/* , helper项目

          1.  ConsoleHelper -- 日志调试小工具
          2.  ScriptInterfaceGenerator -- 脚本接口绑定小工具
  - LongUI -- 被设计为静态链接的核心项目
  - LongUITest -- 测试项目, 仅仅面向于自己, 用于新特性/调试
  
## License
  
  - LongUI **基本基于** MIT License
  - pugixml parser - version 1.5 (MIT License)
  - dlmalloc-2.8.6 (public domain)
  - ~~Scintilla-3.5.3 (License for Scintilla)~~
  - 更多细节请参考[License.txt](./License.txt)
  
附加需要:
  - 请在您的程序中的"关于"或者相关选项(如果有的话)显示或者链接License信息