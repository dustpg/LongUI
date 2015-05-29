#pragma once

#include "../longui/longui.h"
#include "MRubyScript.h"

#ifdef _DEBUG
#   pragma comment(lib, "../Debug/longui")
#else
#   pragma comment(lib, "../Release/longui")
#endif

#include "MainWindow.h"