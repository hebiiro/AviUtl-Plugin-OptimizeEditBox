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
DECLARE_HOOK_PROC(LRESULT, WINAPI, Exedit_ObjectDialog_WndProc, (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam));

extern COLORREF* g_selectionColor;
extern COLORREF* g_selectionEdgeColor;
extern COLORREF* g_selectionBkColor;

void drawLineLeft(HDC dc, int mx, int my, int lx, int ly, HPEN pen);
void drawLineRight(HDC dc, int mx, int my, int lx, int ly, HPEN pen);
void drawLineTop(HDC dc, int mx, int my, int lx, int ly, HPEN pen);
void drawLineBottom(HDC dc, int mx, int my, int lx, int ly, HPEN pen);

//---------------------------------------------------------------------

class COptimizeEditBoxApp
{
public:

	HINSTANCE m_instance;
	HWND m_exeditTimelineWindow;
	HWND m_exeditObjectDialog;
	UINT_PTR m_timerId;
	WPARAM m_wParam;
	LPARAM m_lParam;

	BOOL m_optimizeTimeLine;
	int m_editBoxDelay;
	int m_trackBarDelay;
	BOOL m_usesUnicodeInput;
	BOOL m_usesSetRedraw;
	BOOL m_usesGradientFill;

	COLORREF m_innerColor;
	int m_innerEdgeWidth;
	int m_innerEdgeHeight;

	COLORREF m_outerColor;
	int m_outerEdgeWidth;
	int m_outerEdgeHeight;

	COLORREF m_selectionColor;
	COLORREF m_selectionEdgeColor;
	COLORREF m_selectionBkColor;

	COLORREF m_layerBorderLeftColor;
	COLORREF m_layerBorderRightColor;
	COLORREF m_layerBorderTopColor;
	COLORREF m_layerBorderBottomColor;

public:

	COptimizeEditBoxApp();
	~COptimizeEditBoxApp();

	BOOL initHook();
	BOOL termHook();
	BOOL initExeditHook(HWND hwnd);

	BOOL dllMain(HINSTANCE instance, DWORD reason, LPVOID reserved);
	BOOL init(FILTER *fp);
	BOOL exit(FILTER *fp);
	BOOL proc(FILTER *fp, FILTER_PROC_INFO *fpip);
	BOOL exedit_proc(FILTER *fp, FILTER_PROC_INFO *fpip);
	LRESULT hook_exedit_wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	void startTimer(WPARAM wParam, LPARAM lParam, int elapse);
	void stopTimer();
	void timerProc(HWND hwnd, UINT message, UINT_PTR id, DWORD time);
	static void CALLBACK _timerProc(HWND hwnd, UINT message, UINT_PTR id, DWORD time);

};

extern COptimizeEditBoxApp theApp;

//---------------------------------------------------------------------
