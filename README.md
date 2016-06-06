## LongUI
  
Lightweight Direct2D GUI Library/Framework in C++  
[中文说明文档](./README.zh-cn.md)  
Current Version : **Alpha-Stage**

## Features in LongUI
  
**Friendly with High-DPI**
  - No GDI/GDI+, LongUI could zoom a container into any size
  
**XML Layout**
  - Easy way to create a window through xml
  
**Light Weight**
  - static-link framework.
  - The `helloworld` in 200+kb only(/MD in MSC).

[Known Issues](./KnownIssues.md)  
[Document](./Document/)  
[DemosShow](./DemosShow.md)  

## Requirements
  
For this repo, you need:
  
  - OS: 
    - **Windows 10 or later**
  - Tool-chain/Environment A: MSC  
    - Visual Stuidio 2015 - Express for Desktop/Community or higher
    - Windows SDK(10.0.10586.0)
  - Tool-chain/Environment B(would support in the future): GCC/Clang  
    - Rakefile(ruby)
    - MinGW-W64
  
  
## Remarks
  
Win10 supported only now
  
## PROJECTS LIST
  -  3rdparty/* , 3rdparty library, like dlmalloc
  -  Demos/* , demos  
    - more detail in [DemosShow.md](./DemosShow.md)
  -  Helper/* , helper projects  
    1. ConsoleHelper -- helper for logging for debugging
    2. ScriptInterfaceGenerator -- helper for binding script interface
  -  LongUI -- core project, designed to be a static-link framework
  -  TestUI -- my test project, just for me for debugging
  
## License
  - LongUI under MIT License
  - pugixml parser - version 1.5 (MIT License)
  - dlmalloc-2.8.6 (public domain)
  - ~~Scintilla-3.5.3 (License for Scintilla)~~
  - More detail, see [License.txt](./License.txt)