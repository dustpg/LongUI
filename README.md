## LongUI
  
Lightweight Direct2D GUI Library/Framework

## Requirements
  
For this repo, you need:
  
  - OS: 
    - **Windows 8 or later**
  - Tool-chain/Environment A:
    - Visual Stuidio 2015 - Express for Desktop/Community or higher
    - Windows SDK/DirectX SDK
  - Tool-chain/Environment B(would support in the future):
    - Rakefile(ruby)
    - MinGW-W64
    
##Remarks
  
  Windows 7 also supported Direct2D 1.1, but DirectComposition,  
  planned support for Win7 without DirectComposition,but MS   
  said that Win7 could upgrade to Win10 for free, so just 'planned'
  
##LongUI Programming Style
  
  class : \[C/I/?\] UI ClassName, etc. CUIManager IUIInterface  
  member variable:  
    public: member_variable, etc. parent, user_data  
    private/protect: m_typeMemberVariable, etc, m_pWindow
  method/function:  
    public: ClassMethod() except to match STL-style  
      etc GetWindow(), Update(), begin()
    private/protect:  class_method(), etc, refresh_this()  
  enum:  see it
```cpp
        enum class PointType : uint16_t {
            Type_None,      // None
            Type_Arrow1,    // Arrow1
            Type_Arrow2,    // Arrow2
            Type_Thumb,     // Thumb
            Type_Shaft,     // Shaft
        };
```
  auto variable:  
    DEPENDS ON MY MOOD, etc, int old_x, oldX, uOldX
    
  
## PROJECTS LIST
  
  -  3rdparty/* , 3rdparty library, like dlmalloc
  -  Demos/* , demos
  
           1. helloworld -- how to build a basic LongUI app.
  -  Helper/* , helper projects
  
           1.  ConsoleHelper -- helper for logging for debugging
           2.  ScriptInterfaceGenerator -- helper for binding srcpt
  -  LongUI -- core project, designed to be a static-link framework
  -  LongUITest -- my test project, just for me for debugging,  maybe you cannot build it

## 项目列表
  
  - 3rdparty/*, 第三方库, 比如dlmalloc
  - Demos/* ,示例
  
          1. helloworld -- 创建一个基本的LongUI程序
  - Helper/* , helper项目
  
          1.  ConsoleHelper -- 调试日志helper
          2.  ScriptInterfaceGenerator -- 脚本绑定helper
  - LongUI -- 被设计为静态链接的核心项目
  - LongUITest -- 测试项目, 仅仅面向于自己, 用于新特性/调试, 或许你并不能成功生成