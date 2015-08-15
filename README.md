## LongUI
  
Lightweight Direct2D GUI Library/Framework in C++  
[中文说明文档](./README.zh-cn.md)

## Requirements
  
For this repo, you need:
  
  - OS: 
    - **Windows 8 or later**
  - Tool-chain/Environment A: MSC  
    - Visual Stuidio 2015 - Express for Desktop/Community or higher
    - Windows SDK(10.0.10240.0 RTM)
  - Tool-chain/Environment B(would support in the future): GCC/Clang  
    - Rakefile(ruby)
    - MinGW-W64

##Known Issues  
  in (KnownIssues.md)[./KnownIssues.md]
    
##Remarks
  
  Windows 7 also supported Direct2D 1.1, but DirectComposition, planned support
  for Win7 without DirectComposition,but MS said that Win7 could upgrade to 
  Win10 for free, so just 'planned', and, maybe Win8.1/Win10 support only in 
  the next year
  
##LongUI Programming Style
  
  - **class** :  
      - \[C/I/?\] UI ClassName, e.g. `CUIManager`, `IUIInterface`
      - except: class under LongUI::XXX(Component/Helper....)
  - **member variable**:  
      - public: member_variable, e.g. `parent`, `user_data`
      - private/protected: m_typeMemberVariable, e.g, `m_pWindow`
  - **method/function**:  
      - public: ClassMethod(), e.g. `GetWindow()`, `Update()`
      - private/protected:  class_method(), e.g. `refresh_this()`
      - except: match STL-Style method: `begin()`, `end()`, `insert()`, 
      but **public**
  - **enum**:  see it
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
    - DEPENDS ON MY MOOD, e.g. int `old_x`, `oldX`, `uOldX`
    
  
## PROJECTS LIST
  
  -  3rdparty/* , 3rdparty library, like dlmalloc
  -  Demos/* , demos
  
           1. helloworld -- how to build a basic LongUI app.
           2. handleevent -- how to handle LongUI event
  -  Helper/* , helper projects
  
           1.  ConsoleHelper -- helper for logging for debugging
           2.  ScriptInterfaceGenerator -- helper for binding script interface
  -  LongUI -- core project, designed to be a static-link framework
  -  TestUI -- my test project, just for me for debugging
  
## License

  - LongUI under MIT License **basically**
  - pugixml parser - version 1.5 (MIT License)
  - dlmalloc-2.8.6 (public domain)
  - ~~Scintilla-3.5.3 (License for Scintilla)~~
  - More detail, see [License.txt](./License.txt)
  
  p.s.
  - Please put the license/URL in "About" or other option(if exist) in your
   longui app