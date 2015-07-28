## LongUI
  
Lightweight Direct2D GUI Library/Framework  
轻量级 Direct2D GUI 库/框架  

## Requirements
  
For this repo, you need:
  
  - OS: 
    - **Windows 8 or later**
  - Tool-chain/Environment A: MSC  
    - Visual Stuidio 2015 - Express for Desktop/Community or higher
    - Windows SDK(10.0.10240.0 RTM)/DirectX SDK(optional)
  - Tool-chain/Environment B(would support in the future): GCC/Clang  
    - Rakefile(ruby)
    - MinGW-W64
    
##Remarks
  
  Windows 7 also supported Direct2D 1.1, but DirectComposition, planned support
  for Win7 without DirectComposition,but MS said that Win7 could upgrade to 
  Win10 for free, so just 'planned', and, maybe Win8.1/Win10 support only in 
  the next year
  
##LongUI Programming Style
  
  - class :  
      - \[C/I/?\] UI ClassName, etc. CUIManager IUIInterface  
      - except: class under LongUI::Component
  - member variable:  
      - public: member_variable, etc. parent, user_data  
      - private/protected: m_typeMemberVariable, etc, m_pWindow
  - method/function:  
      - public: ClassMethod(), etc GetWindow(), Update()
      - private/protected:  class_method(), etc, refresh_this()  
      - except: match STL-Style method: begin(), end(), insert(), but public
  - enum:  see it
```cpp
        enum class PointType : uint16_t {
            Type_None,      // None
            Type_Arrow1,    // Arrow1
            Type_Arrow2,    // Arrow2
            Type_Thumb,     // Thumb
            Type_Shaft,     // Shaft
        };
```
  - auto variable:  
    - DEPENDS ON MY MOOD, etc, int old_x, oldX, uOldX
    
  
## PROJECTS LIST
  
  -  3rdparty/* , 3rdparty library, like dlmalloc
  -  Demos/* , demos
  
           1. helloworld -- how to build a basic LongUI app.
  -  Helper/* , helper projects
  
           1.  ConsoleHelper -- helper for logging for debugging
           2.  ScriptInterfaceGenerator -- helper for binding script interface
  -  LongUI -- core project, designed to be a static-link framework
  -  LongUITest -- my test project, just for me for debugging

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
   
   - LongUI Under MIT License Basically
   - pugixml parser - version 1.5 (MIT License)
   - dlmalloc-2.8.6 (public domain)
   - ~~Scintilla-3.5.3 (License for Scintilla)~~
   - More detail, see License.txt
   