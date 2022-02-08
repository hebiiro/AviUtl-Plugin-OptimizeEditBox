#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#pragma comment(lib, "msimg32.lib")
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <commctrl.h>
#include <comdef.h>

#include <tchar.h>
#include <strsafe.h>

#include <map>

typedef const BYTE* LPCBYTE;
#include "../AviUtl/aulslib/exedit.h"
#include "../Common/MyTracer.h"
#include "../Detours.4.0.1/detours.h"
#pragma comment(lib, "../Detours.4.0.1/detours.lib")
