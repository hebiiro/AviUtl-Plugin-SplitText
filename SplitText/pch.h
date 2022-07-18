#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#pragma comment(lib, "msimg32.lib")
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")
#include <comdef.h>

#include <tchar.h>
#include <strsafe.h>

#include <algorithm>
#include <numeric>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#include "AviUtl/aviutl_exedit_sdk/aviutl.hpp"
#include "AviUtl/aviutl_exedit_sdk/exedit.hpp"
#include "Common/Tracer.h"
#include "Common/Gdi.h"
#include "Common/Hook.h"
#include "Common/AviUtlInternal.h"
#include "Detours.4.0.1/detours.h"
#pragma comment(lib, "Detours.4.0.1/detours.lib")
