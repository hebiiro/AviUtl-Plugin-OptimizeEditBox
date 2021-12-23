#pragma once

class COptimizeEditBoxApp
{
public:

	HINSTANCE m_instance;
	HWND m_exeditWindow;
	UINT_PTR m_timerId;
	WPARAM m_wParam;
	LPARAM m_lParam;
	bool m_doDefault;

	BOOL m_optimizeTimeLine;
	int m_editBoxDelay;
	int m_trackBarDelay;

	HHOOK m_hook;

public:

	COptimizeEditBoxApp();
	~COptimizeEditBoxApp();

	BOOL dllMain(HINSTANCE instance, DWORD reason, LPVOID reserved);
	BOOL init(FILTER *fp);
	BOOL exit(FILTER *fp);
	BOOL proc(FILTER *fp, FILTER_PROC_INFO *fpip);
	LRESULT exedit_wndProc(WNDPROC orig, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	void timerProc(HWND hwnd, UINT message, UINT_PTR id, DWORD time);
	LRESULT cbtProc(int code, WPARAM wParam, LPARAM lParam);

	void startTimer(WPARAM wParam, LPARAM lParam, int delay);
	void stopTimer();
	static void CALLBACK _timerProc(HWND hwnd, UINT message, UINT_PTR id, DWORD time);

	void hook();
	void unhook();
	static LRESULT CALLBACK _cbtProc(int code, WPARAM wParam, LPARAM lParam);

};

extern COptimizeEditBoxApp theApp;
