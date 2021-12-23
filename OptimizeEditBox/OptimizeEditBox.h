#pragma once

class COptimizeEditBoxApp
{
public:

	HINSTANCE m_instance;
	HWND m_exeditWindow;
	UINT_PTR m_timerId;
	WPARAM m_wParam;
	LPARAM m_lParam;

	BOOL m_optimizeTimeLine;
	int m_editBoxDelay;
	int m_trackBarDelay;

	HHOOK m_hook;
	WNDPROC m_orig_exedit_wndProc;

public:

	COptimizeEditBoxApp();
	~COptimizeEditBoxApp();

	BOOL dllMain(HINSTANCE instance, DWORD reason, LPVOID reserved);
	BOOL init(FILTER *fp);
	BOOL exit(FILTER *fp);
	BOOL proc(FILTER *fp, FILTER_PROC_INFO *fpip);
	BOOL exedit_proc(FILTER *fp, FILTER_PROC_INFO *fpip);

	LRESULT hook_exedit_wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK _hook_exedit_wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	void startTimer(WPARAM wParam, LPARAM lParam, int elapse);
	void stopTimer();
	void timerProc(HWND hwnd, UINT message, UINT_PTR id, DWORD time);
	static void CALLBACK _timerProc(HWND hwnd, UINT message, UINT_PTR id, DWORD time);

	void hook();
	void unhook();
	LRESULT cbtProc(int code, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK _cbtProc(int code, WPARAM wParam, LPARAM lParam);

};

extern COptimizeEditBoxApp theApp;
