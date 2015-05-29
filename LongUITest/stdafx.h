#pragma once

#ifndef WINVER
#define WINVER 0x0800
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0800
#endif


// Windows Header Files:
#include <windows.h>
#include <Shlobj.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <ShObjIdl.h>
// C++ 
#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <clocale>
#include <string>
#include <cwchar>
#include <vector>
#include <thread>
#include <atomic>
#include <memory>
#include <queue>
#include <mutex>
#include <map>
#include <new>
// C RunTime Header Files
#include <malloc.h>
#include <memory.h>

// RichEdit
#include <Richedit.h>
#include <Textserv.h>

// Basic Lib
#undef DrawText
#include <strsafe.h>
#include <dxgi1_2.h>
#include <d3d11.h>
#include <D3D11SDKLayers.h>
#include <d2d1_1.h>
#include <d2d1_1helper.h>
#include <d2d1effects.h>
#include <d2d1effectauthor.h>
#include <d2d1effecthelpers.h>
#include <dwrite_1.h>
#include <wincodec.h>
// DirectComposition 
#include <dcomp.h>

// pugixml
#include "../3rdparty/pugixml/pugixml.hpp"
#ifndef PUGIXML_HEADER_ONLY
#ifdef _DEBUG
#   pragma comment(lib, "../Debug/pugixml")
#else
#   pragma comment(lib, "../Release/pugixml")
#endif
#endif

// dlmalloc
#define USE_DL_PREFIX
#include "../3rdparty/dlmalloc/dlmalloc.h"
#ifdef _DEBUG
#   pragma comment(lib, "../Debug/dlmalloc")
#else
#   pragma comment(lib, "../Release/dlmalloc")
#endif


#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif


#include "mruby.h"





