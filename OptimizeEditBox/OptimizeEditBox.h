#pragma once

//---------------------------------------------------------------------

class COptimizeEditBoxApp
{
public:

	HINSTANCE m_instance;
	HWND m_filterWindow;
	UINT_PTR m_timerId;
	WPARAM m_wParam;
	LPARAM m_lParam;

	BOOL m_optimizeTimeLine;
	int m_editBoxDelay;
	int m_trackBarDelay;
	BOOL m_usesUnicodeInput;
	BOOL m_usesCtrlA;
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
	COLORREF m_layerSeparatorColor;

	int m_addTextEditBoxHeight;
	int m_addScriptEditBoxHeight;

	HFONT m_font;

	HWND* m_exeditWindow;
	HWND* m_settingDialog;

public:

	COptimizeEditBoxApp();
	~COptimizeEditBoxApp();

	BOOL initHook();
	BOOL termHook();

	BOOL DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved);
	BOOL func_init(FILTER *fp);
	BOOL func_exit(FILTER *fp);
	BOOL func_proc(FILTER *fp, FILTER_PROC_INFO *fpip);
	BOOL func_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, void* editp, FILTER* fp);
	BOOL Exedit_func_proc(FILTER *fp, FILTER_PROC_INFO *fpip);
	LRESULT Exedit_SettingDialog_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	void startTimer(WPARAM wParam, LPARAM lParam, int elapse);
	void stopTimer();
	void timerProc(HWND hwnd, UINT message, UINT_PTR id, DWORD time);
	static void CALLBACK _timerProc(HWND hwnd, UINT message, UINT_PTR id, DWORD time);
};

extern COptimizeEditBoxApp theApp;

//---------------------------------------------------------------------
