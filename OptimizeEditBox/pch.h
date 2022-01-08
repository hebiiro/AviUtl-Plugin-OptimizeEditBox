#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <tchar.h>
#include <strsafe.h>

typedef const BYTE* LPCBYTE;
#include "aulslib/exedit.h"
#include "Common/MyTracer.h"
