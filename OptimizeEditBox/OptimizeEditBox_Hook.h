#pragma once

//---------------------------------------------------------------------
// Define and Const

#define DECLARE_INTERNAL_PROC(resultType, callType, procName, args) \
	typedef resultType (callType *Type_##procName) args; \
	Type_##procName procName = NULL

#define DECLARE_HOOK_PROC(resultType, callType, procName, args) \
	typedef resultType (callType *Type_##procName) args; \
	extern Type_##procName true_##procName; \
	resultType callType hook_##procName args

#define IMPLEMENT_HOOK_PROC(resultType, callType, procName, args) \
	Type_##procName true_##procName = procName; \
	resultType callType hook_##procName args

#define IMPLEMENT_HOOK_PROC_NULL(resultType, callType, procName, args) \
	Type_##procName true_##procName = NULL; \
	resultType callType hook_##procName args

#define GET_INTERNAL_PROC(module, procName) \
	procName = (Type_##procName)::GetProcAddress(module, #procName)

#define GET_HOOK_PROC(module, procName) \
	true_##procName = (Type_##procName)::GetProcAddress(module, #procName)

#define ATTACH_HOOK_PROC(name) DetourAttach((PVOID*)&true_##name, hook_##name)
#define DETACH_HOOK_PROC(name) DetourDetach((PVOID*)&true_##name, hook_##name)

//---------------------------------------------------------------------
// Api Hook

DECLARE_HOOK_PROC(BOOL, WINAPI, GetMessageA, (LPMSG msg, HWND hwnd, UINT msgFilterMin, UINT msgFilterMax));
DECLARE_HOOK_PROC(BOOL, WINAPI, PeekMessageA, (LPMSG msg, HWND hwnd, UINT msgFilterMin, UINT msgFilterMax, UINT removeMsg));
DECLARE_HOOK_PROC(HWND, WINAPI, CreateWindowExA, (DWORD exStyle, LPCSTR className, LPCSTR windowName, DWORD style, int x, int y, int w, int h, HWND parent, HMENU menu, HINSTANCE instance, LPVOID param));
DECLARE_HOOK_PROC(void, CDECL, Exedit_HideControls, ());
DECLARE_HOOK_PROC(BOOL, CDECL, Exedit_ShowControls, (int objectIndex));
DECLARE_HOOK_PROC(void, CDECL, Exedit_FillGradation, (HDC dc, const RECT *rc, BYTE r, BYTE g, BYTE b, BYTE gr, BYTE gg, BYTE gb, int gs, int ge));
DECLARE_HOOK_PROC(LRESULT, WINAPI, Exedit_SettingDialog_WndProc, (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam));

void Exedit_DrawLineLeft(HDC dc, int mx, int my, int lx, int ly, HPEN pen);
void Exedit_DrawLineRight(HDC dc, int mx, int my, int lx, int ly, HPEN pen);
void Exedit_DrawLineTop(HDC dc, int mx, int my, int lx, int ly, HPEN pen);
void Exedit_DrawLineBottom(HDC dc, int mx, int my, int lx, int ly, HPEN pen);
void Exedit_DrawLineSeparator(HDC dc, int mx, int my, int lx, int ly, HPEN pen);

HWND WINAPI Exedit_CreateTextEditBox(DWORD exStyle, LPCWSTR className, LPCWSTR windowName, DWORD style, int x, int y, int w, int h, HWND parent, HMENU menu, HINSTANCE instance, LPVOID param);
HWND WINAPI Exedit_CreateScriptEditBox(DWORD exStyle, LPCWSTR className, LPCWSTR windowName, DWORD style, int x, int y, int w, int h, HWND parent, HMENU menu, HINSTANCE instance, LPVOID param);

//---------------------------------------------------------------------
// Function

// CALL を書き換える。
template<class T>
inline void hookCall(DWORD address, T hookProc)
{
	BYTE code[5];
	code[0] = 0xE8; // CALL
	*(DWORD*)&code[1] = (DWORD)hookProc - (address + 5);

	// CALL を書き換える。そのあと命令キャッシュをフラッシュする。
	::WriteProcessMemory(::GetCurrentProcess(), (LPVOID)address, code, sizeof(code), NULL);
	::FlushInstructionCache(::GetCurrentProcess(), (LPVOID)address, sizeof(code));
}

// CALL を書き換える。
template<class T>
inline void hookAbsoluteCall(DWORD address, T& hookProc)
{
	BYTE code[6];
	code[0] = 0xE8; // CALL
	*(DWORD*)&code[1] = (DWORD)hookProc - (address + 5);
	code[5] = 0x90; // NOP

	// CALL を書き換える。そのあと命令キャッシュをフラッシュする。
	::WriteProcessMemory(::GetCurrentProcess(), (LPVOID)address, code, sizeof(code), NULL);
	::FlushInstructionCache(::GetCurrentProcess(), (LPVOID)address, sizeof(code));
}

// 絶対アドレスを書き換える。
template<class T>
inline T writeAbsoluteAddress(DWORD address, T x)
{
	// 絶対アドレスから読み込む。
	T retValue = 0;
	::ReadProcessMemory(::GetCurrentProcess(), (LPVOID)address, &retValue, sizeof(retValue), NULL);
	// 絶対アドレスを書き換える。
	::WriteProcessMemory(::GetCurrentProcess(), (LPVOID)address, &x, sizeof(x), NULL);
	// 命令キャッシュをフラッシュする。
	::FlushInstructionCache(::GetCurrentProcess(), (LPVOID)address, sizeof(x));
	return retValue;
}

// 指定アドレスの値に x を加算する。
inline void addInt32(DWORD address, int value)
{
	// プロセスハンドルを取得する。
	HANDLE process = ::GetCurrentProcess();

	// アドレスの値を取得する。
	int x = 0;
	::ReadProcessMemory(process, (LPVOID)address, &x, sizeof(x), NULL);

	// value を加算する。
	x += value;

	// アドレスの値を書き換える。
	::WriteProcessMemory(process, (LPVOID)address, &x, sizeof(x), NULL);

	// 命令キャッシュをフラッシュする。
	::FlushInstructionCache(process, (LPVOID)address, sizeof(x));
}

//---------------------------------------------------------------------
// Class

class CMyClocker
{
private:
	HWND m_hwnd;
	LPCTSTR m_label;
	DWORD m_start;
public:
	CMyClocker(HWND hwnd, LPCTSTR label)
	{
		m_hwnd = hwnd;
		m_label = label;
		m_start = ::timeGetTime();
	}
	~CMyClocker()
	{
		DWORD end = ::timeGetTime();
		TCHAR text[MAX_PATH];
		::StringCbPrintf(text, sizeof(text), _T("%s => %dms"), m_label, end - m_start);
		::SetWindowText(m_hwnd, text);
		MY_TRACE(_T("%s\n"), text);
	}
};

//---------------------------------------------------------------------
