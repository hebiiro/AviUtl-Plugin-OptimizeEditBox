﻿#include "pch.h"
#include "OptimizeEditBox.h"

COptimizeEditBoxApp theApp;

// MY_TRACE() から呼ばれる。テキストを追加出力したいときに使う。
void ___outputLog(LPCTSTR text, LPCTSTR output)
{
}

// exedit オブジェクト編集ダイアログをサブクラス化するための関数
WNDPROC orig_exedit_wndProc = 0;
LRESULT CALLBACK hook_exedit_wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return theApp.exedit_wndProc(orig_exedit_wndProc, hwnd, message, wParam, lParam);
}

COptimizeEditBoxApp::COptimizeEditBoxApp()
{
	// 初期化。基本 0。
	m_instance = 0;
	m_exeditWindow = 0;
	m_timerId = 0;
	m_wParam = 0;
	m_lParam = 0;
	m_doDefault = false;

	m_optimizeTimeLine = FALSE;
	m_editBoxDelay = 0;
	m_trackBarDelay = 0;

	m_hook = 0;
}

COptimizeEditBoxApp::~COptimizeEditBoxApp()
{
}

BOOL COptimizeEditBoxApp::dllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		{
			MY_TRACE(_T("DLL_PROCESS_ATTACH\n"));

			m_instance = instance;

			MY_TRACE_HEX(m_instance);

			break;
		}
	case DLL_PROCESS_DETACH:
		{
			MY_TRACE(_T("DLL_PROCESS_DETACH\n"));

			break;
		}
	}

	return TRUE;
}

BOOL COptimizeEditBoxApp::init(FILTER *fp)
{
	MY_TRACE(_T("COptimizeEditBoxApp::init()\n"));

	// exedit のオブジェクト編集ダイアログをサブクラス化するタイミングが得られるようにウィンドウズフックを仕掛ける。
	hook();

	// ini ファイルから設定を読み込む。
	TCHAR path[MAX_PATH];
	::GetModuleFileName(m_instance, path, MAX_PATH);
	::PathRenameExtension(path, _T(".ini"));

	MY_TRACE_STR(path);

	m_optimizeTimeLine	= ::GetPrivateProfileInt(_T("Settings"), _T("optimizeTimeLine"),	m_optimizeTimeLine, path);
	m_editBoxDelay		= ::GetPrivateProfileInt(_T("Settings"), _T("editBoxDelay"),		m_editBoxDelay, path);
	m_trackBarDelay		= ::GetPrivateProfileInt(_T("Settings"), _T("trackBarDelay"),		m_trackBarDelay, path);

	MY_TRACE_NUM(m_optimizeTimeLine);
	MY_TRACE_NUM(m_editBoxDelay);
	MY_TRACE_NUM(m_trackBarDelay);

	return TRUE;
}

BOOL COptimizeEditBoxApp::exit(FILTER *fp)
{
	MY_TRACE(_T("COptimizeEditBoxApp::exit()\n"));

	return TRUE;
}

BOOL COptimizeEditBoxApp::proc(FILTER *fp, FILTER_PROC_INFO *fpip)
{
	MY_TRACE(_T("COptimizeEditBoxApp::proc()\n"));

	// 念のためフレーム画像が更新されたタイミングでタイマーを止める。
	stopTimer();

	return TRUE;
}

LRESULT COptimizeEditBoxApp::exedit_wndProc(WNDPROC orig, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("0x%08X, 0x%08X, 0x%08X)\n"), message, wParam, lParam);

	// WM_COMMAND メッセージが来たかチェックする。
	if (message == WM_COMMAND)
	{
		UINT code = HIWORD(wParam);
		UINT id = LOWORD(wParam);
		HWND sender = (HWND)lParam;

		MY_TRACE(_T("WM_COMMAND, 0x%04X, 0x%04X, 0x%08X)\n"), code, id, sender);

		// エディットボックス通知メッセージの EN_CHANGE もしくは EN_UPDATE が来たかチェックする。
		// この 2 つをチェックするのは exedit がテキストオブジェクトは EN_CHANGE で処理をしているが、その他のオブジェクトは EN_UPDATE で処理をしているため。
		if (code == EN_CHANGE || code == EN_UPDATE)
		{
			MY_TRACE(_T("WM_COMMAND, EN_CHANGE || EN_UPDATE, 0x%04X, 0x%04X, 0x%08X)\n"), code, id, sender);

			// 左クリックがされているかチェックする。
			if (m_optimizeTimeLine && ::GetKeyState(VK_LBUTTON) < 0) 
			{
				MY_TRACE(_T("エディットボックスメッセージの処理を間引きます\n"));
				return 0;
			}
		}

		// エディットボックス通知メッセージの EN_CHANGE がテキストオブジェクト関係のエディットボックスから送られてきたかチェックする。
		if (code == EN_CHANGE && id >= 0x5600)
		{
			MY_TRACE(_T("WM_COMMAND, code == EN_CHANGE && id >= 0x5600, 0x%04X, 0x%04X, 0x%08X)\n"), code, id, sender);

			if (m_editBoxDelay > 0)
			{
				// デフォルト処理を行うフラグが立っている場合はフラグを消すだけで何もしない。
				if (m_doDefault)
				{
					MY_TRACE(_T("先送りしていたエディットボックスメッセージをデフォルト処理に送ります\n"));
					m_doDefault = false;
				}
				else
				{
					MY_TRACE(_T("タイマーを使ってエディットボックスメッセージの処理を先送りにします\n"));
					startTimer(wParam, lParam, m_editBoxDelay);
					return 0;
				}
			}
		}
	}

	return ::CallWindowProc(orig, hwnd, message, wParam, lParam);
}

void COptimizeEditBoxApp::timerProc(HWND hwnd, UINT message, UINT_PTR id, DWORD time)
{
	MY_TRACE(_T("COptimizeEditBoxApp::timerProc(0x%08X, 0x%08X)\n"), id, time);

	if (id == m_timerId)
	{
		// このタイマーを止める。
		stopTimer();

		// 先送りしていたメッセージのデフォルト処理を促す。
		m_doDefault = true;

		WPARAM wParam = m_wParam;
		LPARAM lParam = m_lParam;

		MY_TRACE(_T("::PostMessage(WM_COMMAND, 0x%08X, 0x%08X)\n"), wParam, lParam);
		::PostMessage(m_exeditWindow, WM_COMMAND, wParam, lParam);
	}
}

LRESULT COptimizeEditBoxApp::cbtProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0)
	{
		if (code == HCBT_CREATEWND)
		{
			CBT_CREATEWND* cw = (CBT_CREATEWND*)lParam;

			if ((DWORD)cw->lpcs->lpszClass > 0xFFFF &&
				::lstrcmpi(cw->lpcs->lpszClass, _T("ExtendedFilterClass")) == 0)
			{
				// exedit のオブジェクト編集ダイアログをサブクラス化する。
				m_exeditWindow = (HWND)wParam;
				orig_exedit_wndProc = SubclassWindow(m_exeditWindow, hook_exedit_wndProc);
				MY_TRACE_HEX(orig_exedit_wndProc);

				// フックはこれで終わり。
				unhook();
			}
		}
	}

	return ::CallNextHookEx(m_hook, code, wParam, lParam);
}

void COptimizeEditBoxApp::startTimer(WPARAM wParam, LPARAM lParam, int delay)
{
	MY_TRACE(_T("COptimizeEditBoxApp::startTimer(0x%08X, 0x%08X, %d)\n"), wParam, lParam, delay);

	if (m_timerId)
	{
//		MY_TRACE(_T("タイマーはすでにセットされています\n"));

		// スレッドタイマーは上書きができないので削除してから作り直す。
		// (ウィンドウタイマーとして実装すれば上書きできるのでいちいち削除しなくてもよい)
		::KillTimer(0, m_timerId), m_timerId = 0;
	}

	m_wParam = wParam;
	m_lParam = lParam;
	m_timerId = ::SetTimer(0, 0, delay, _timerProc);
	MY_TRACE_HEX(m_wParam);
	MY_TRACE_HEX(m_lParam);
	MY_TRACE_HEX(m_timerId);
}

void COptimizeEditBoxApp::stopTimer()
{
	MY_TRACE(_T("COptimizeEditBoxApp::stopTimer()\n"));

	if (m_timerId)
	{
		// タイマーを削除する。
		::KillTimer(0, m_timerId), m_timerId = 0;
	}
}

void CALLBACK COptimizeEditBoxApp::_timerProc(HWND hwnd, UINT message, UINT_PTR id, DWORD time)
{
	theApp.timerProc(hwnd, message, id, time);
}

void COptimizeEditBoxApp::hook()
{
	MY_TRACE(_T("COptimizeEditBoxApp::hook()\n"));

	// CBT フックを作成する。
	m_hook = ::SetWindowsHookEx(WH_CBT, _cbtProc, NULL, ::GetCurrentThreadId());
}

void COptimizeEditBoxApp::unhook()
{
	MY_TRACE(_T("COptimizeEditBoxApp::unhook()\n"));

	// CBT フックを削除する。
	::UnhookWindowsHookEx(m_hook), m_hook = 0;
}

LRESULT CALLBACK COptimizeEditBoxApp::_cbtProc(int code, WPARAM wParam, LPARAM lParam)
{
	return theApp.cbtProc(code, wParam, lParam);
}