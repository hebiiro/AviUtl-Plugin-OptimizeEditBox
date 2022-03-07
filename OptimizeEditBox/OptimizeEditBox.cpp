#include "pch.h"
#include "OptimizeEditBox.h"
#include "OptimizeEditBox_Hook.h"

//---------------------------------------------------------------------

COptimizeEditBoxApp theApp;

// デバッグ用コールバック関数。デバッグメッセージを出力する
void ___outputLog(LPCTSTR text, LPCTSTR output)
{
	::OutputDebugString(output);
}

COptimizeEditBoxApp::COptimizeEditBoxApp()
{
	// 初期化。基本 0。
	m_instance = 0;
	m_filterWindow = 0;
	m_exeditTimelineWindow = 0;
	m_exeditObjectDialog = 0;
	m_timerId = 0;
	m_wParam = 0;
	m_lParam = 0;

	m_optimizeTimeLine = FALSE;
	m_editBoxDelay = 0;
	m_trackBarDelay = 0;
	m_usesUnicodeInput = FALSE;
	m_usesCtrlA = FALSE;
	m_usesSetRedraw = FALSE;
	m_usesGradientFill = FALSE;

	m_innerColor = RGB(0xff, 0xff, 0xff);
	m_innerEdgeWidth = 1;
	m_innerEdgeHeight = 1;

	m_outerColor = RGB(0x00, 0x00, 0x00);
	m_outerEdgeWidth = 1;
	m_outerEdgeHeight = 1;

	m_selectionColor = CLR_NONE;
	m_selectionEdgeColor = CLR_NONE;
	m_selectionBkColor = CLR_NONE;

	m_layerBorderLeftColor = RGB(0x99, 0x99, 0x99);
	m_layerBorderRightColor = RGB(0x99, 0x99, 0x99);
	m_layerBorderTopColor = RGB(0x99, 0x99, 0x99);
	m_layerBorderBottomColor = RGB(0x99, 0x99, 0x99);

	m_addTextEditBoxHeight = 0;
	m_addScriptEditBoxHeight = 0;

	m_font = 0;
}

COptimizeEditBoxApp::~COptimizeEditBoxApp()
{
}

BOOL COptimizeEditBoxApp::initHook()
{
	MY_TRACE(_T("COptimizeEditBoxApp::initHook()\n"));

	DetourTransactionBegin();
	DetourUpdateThread(::GetCurrentThread());

	if (m_usesUnicodeInput)
	{
		ATTACH_HOOK_PROC(GetMessageA);
//		ATTACH_HOOK_PROC(PeekMessageA);
	}

	ATTACH_HOOK_PROC(CreateWindowExA);

	if (DetourTransactionCommit() == NO_ERROR)
	{
		MY_TRACE(_T("API フックのインストールに成功しました\n"));

		return TRUE;
	}
	else
	{
		MY_TRACE(_T("API フックのインストールに失敗しました\n"));

		return FALSE;
	}
}

BOOL COptimizeEditBoxApp::termHook()
{
	MY_TRACE(_T("COptimizeEditBoxApp::termHook()\n"));

	DetourTransactionBegin();
	DetourUpdateThread(::GetCurrentThread());

	if (m_usesUnicodeInput)
	{
		DETACH_HOOK_PROC(GetMessageA);
//		DETACH_HOOK_PROC(PeekMessageA);
	}

	DETACH_HOOK_PROC(CreateWindowExA);

	if (DetourTransactionCommit() == NO_ERROR)
	{
		MY_TRACE(_T("API フックのアンインストールに成功しました\n"));

		return TRUE;
	}
	else
	{
		MY_TRACE(_T("API フックのアンインストールに失敗しました\n"));

		return FALSE;
	}

	return TRUE;
}

BOOL COptimizeEditBoxApp::initExeditHook(HWND hwnd)
{
	MY_TRACE(_T("COptimizeEditBoxApp::initExeditHook(0x%08X)\n"), hwnd);

	// 拡張編集のオブジェクトダイアログのハンドルを変数に格納しておく。
	m_exeditObjectDialog = hwnd;
	// 拡張編集のオブジェクトダイアログのウィンドウプロシージャを変数に格納しておく。
	true_Exedit_ObjectDialog_WndProc = (WNDPROC)::GetClassLong(hwnd, GCL_WNDPROC);

	HMODULE exedit_auf = ::GetModuleHandle(_T("exedit.auf"));
	true_Exedit_HideControls = (Type_Exedit_HideControls)((DWORD)exedit_auf + 0x00030500);
	true_Exedit_ShowControls = (Type_Exedit_ShowControls)((DWORD)exedit_auf + 0x000305E0);
	true_Exedit_FillGradation = (Type_Exedit_FillGradation)((DWORD)exedit_auf + 0x00036a70);

	if (m_layerBorderLeftColor != CLR_NONE) hookCall((DWORD)exedit_auf + 0x00038845, Exedit_DrawLineLeft);
	if (m_layerBorderRightColor != CLR_NONE) hookCall((DWORD)exedit_auf + 0x000388AA, Exedit_DrawLineRight);
	if (m_layerBorderTopColor != CLR_NONE) hookCall((DWORD)exedit_auf + 0x00038871, Exedit_DrawLineTop);
	if (m_layerBorderBottomColor != CLR_NONE) hookCall((DWORD)exedit_auf + 0x000388DA, Exedit_DrawLineBottom);

	if (m_selectionColor != CLR_NONE) writeAbsoluteAddress((DWORD)exedit_auf + 0x0003807E, &m_selectionColor);
	if (m_selectionEdgeColor != CLR_NONE) writeAbsoluteAddress((DWORD)exedit_auf + 0x00038076, &m_selectionEdgeColor);
	if (m_selectionBkColor != CLR_NONE) writeAbsoluteAddress((DWORD)exedit_auf + 0x00038087, &m_selectionBkColor);

	if (m_addTextEditBoxHeight)
	{
		hookAbsoluteCall((DWORD)exedit_auf + 0x0008C46E, Exedit_CreateTextEditBox);
		addInt32((DWORD)exedit_auf + 0x0008CC56 + 1, m_addTextEditBoxHeight);
	}

	if (m_addScriptEditBoxHeight)
	{
		hookAbsoluteCall((DWORD)exedit_auf + 0x00087658, Exedit_CreateScriptEditBox);
		addInt32((DWORD)exedit_auf + 0x000876DE + 1, m_addScriptEditBoxHeight);
	}

	DetourTransactionBegin();
	DetourUpdateThread(::GetCurrentThread());

	if (m_usesSetRedraw)
	{
		ATTACH_HOOK_PROC(Exedit_HideControls);
		ATTACH_HOOK_PROC(Exedit_ShowControls);
	}

	if (m_usesGradientFill)
	{
		ATTACH_HOOK_PROC(Exedit_FillGradation);
	}

	ATTACH_HOOK_PROC(Exedit_ObjectDialog_WndProc);

	if (DetourTransactionCommit() == NO_ERROR)
	{
		MY_TRACE(_T("API フックのインストールに成功しました\n"));

		return TRUE;
	}
	else
	{
		MY_TRACE(_T("API フックのインストールに失敗しました\n"));

		return FALSE;
	}
}

BOOL COptimizeEditBoxApp::DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		{
			MY_TRACE(_T("DLL_PROCESS_ATTACH\n"));

			// DLL インスタンスハンドルを m_instance に格納する。
			m_instance = instance;
			MY_TRACE_HEX(m_instance);

			// この DLL の参照カウンタを増やしておく。
			WCHAR moduleFileName[MAX_PATH] = {};
			::GetModuleFileNameW(m_instance, moduleFileName, MAX_PATH);
			::LoadLibraryW(moduleFileName);

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

BOOL COptimizeEditBoxApp::func_init(FILTER *fp)
{
	MY_TRACE(_T("COptimizeEditBoxApp::func_init()\n"));

	m_filterWindow = fp->hwnd;
	MY_TRACE_HEX(m_filterWindow);

	// 拡張編集のタイムラインウィンドウを取得する。
	m_exeditTimelineWindow = auls::Exedit_GetWindow(fp);
	MY_TRACE_HEX(m_exeditTimelineWindow);

	// ini ファイルから設定を読み込む。
	TCHAR path[MAX_PATH];
	::GetModuleFileName(m_instance, path, MAX_PATH);
	::PathRenameExtension(path, _T(".ini"));
	MY_TRACE_TSTR(path);

	m_optimizeTimeLine	= ::GetPrivateProfileInt(_T("Settings"), _T("optimizeTimeLine"),	m_optimizeTimeLine, path);
	m_editBoxDelay		= ::GetPrivateProfileInt(_T("Settings"), _T("editBoxDelay"),		m_editBoxDelay, path);
	m_trackBarDelay		= ::GetPrivateProfileInt(_T("Settings"), _T("trackBarDelay"),		m_trackBarDelay, path);
	m_usesUnicodeInput	= ::GetPrivateProfileInt(_T("Settings"), _T("usesUnicodeInput"),	m_usesUnicodeInput, path);
	m_usesCtrlA			= ::GetPrivateProfileInt(_T("Settings"), _T("usesCtrlA"),			m_usesCtrlA, path);
	m_usesSetRedraw		= ::GetPrivateProfileInt(_T("Settings"), _T("usesSetRedraw"),		m_usesSetRedraw, path);
	m_usesGradientFill	= ::GetPrivateProfileInt(_T("Settings"), _T("usesGradientFill"),	m_usesGradientFill, path);

	MY_TRACE_INT(m_optimizeTimeLine);
	MY_TRACE_INT(m_editBoxDelay);
	MY_TRACE_INT(m_trackBarDelay);
	MY_TRACE_INT(m_usesUnicodeInput);
	MY_TRACE_INT(m_usesSetRedraw);
	MY_TRACE_INT(m_usesGradientFill);

	BYTE innerColorR = (BYTE)::GetPrivateProfileInt(_T("Settings"), _T("innerColorR"), GetRValue(m_innerColor), path);
	BYTE innerColorG = (BYTE)::GetPrivateProfileInt(_T("Settings"), _T("innerColorG"), GetGValue(m_innerColor), path);
	BYTE innerColorB = (BYTE)::GetPrivateProfileInt(_T("Settings"), _T("innerColorB"), GetBValue(m_innerColor), path);
	m_innerColor = RGB(innerColorR, innerColorG, innerColorB);
	m_innerEdgeWidth = ::GetPrivateProfileInt(_T("Settings"), _T("innerEdgeWidth"), m_innerEdgeWidth, path);
	m_innerEdgeHeight = ::GetPrivateProfileInt(_T("Settings"), _T("innerEdgeHeight"), m_innerEdgeHeight, path);

	BYTE outerColorR = (BYTE)::GetPrivateProfileInt(_T("Settings"), _T("outerColorR"), GetRValue(m_outerColor), path);
	BYTE outerColorG = (BYTE)::GetPrivateProfileInt(_T("Settings"), _T("outerColorG"), GetGValue(m_outerColor), path);
	BYTE outerColorB = (BYTE)::GetPrivateProfileInt(_T("Settings"), _T("outerColorB"), GetBValue(m_outerColor), path);
	m_outerColor = RGB(outerColorR, outerColorG, outerColorB);
	m_outerEdgeWidth = ::GetPrivateProfileInt(_T("Settings"), _T("outerEdgeWidth"), m_outerEdgeWidth, path);
	m_outerEdgeHeight = ::GetPrivateProfileInt(_T("Settings"), _T("outerEdgeHeight"), m_outerEdgeHeight, path);

	m_selectionColor = ::GetPrivateProfileInt(_T("Settings"), _T("selectionColor"), m_selectionColor, path);
	m_selectionEdgeColor = ::GetPrivateProfileInt(_T("Settings"), _T("selectionEdgeColor"), m_selectionEdgeColor, path);
	m_selectionBkColor = ::GetPrivateProfileInt(_T("Settings"), _T("selectionBkColor"), m_selectionBkColor, path);

	m_layerBorderLeftColor = ::GetPrivateProfileInt(_T("Settings"), _T("layerBorderLeftColor"), m_layerBorderLeftColor, path);
	m_layerBorderRightColor = ::GetPrivateProfileInt(_T("Settings"), _T("layerBorderRightColor"), m_layerBorderRightColor, path);
	m_layerBorderTopColor = ::GetPrivateProfileInt(_T("Settings"), _T("layerBorderTopColor"), m_layerBorderTopColor, path);
	m_layerBorderBottomColor = ::GetPrivateProfileInt(_T("Settings"), _T("layerBorderBottomColor"), m_layerBorderBottomColor, path);

	m_addTextEditBoxHeight = ::GetPrivateProfileInt(_T("Settings"), _T("addTextEditBoxHeight"), m_addTextEditBoxHeight, path);
	m_addScriptEditBoxHeight = ::GetPrivateProfileInt(_T("Settings"), _T("addScriptEditBoxHeight"), m_addScriptEditBoxHeight, path);

	TCHAR fontName[MAX_PATH] = {};
	::GetPrivateProfileString(_T("Settings"), _T("fontName"), _T(""), fontName, MAX_PATH, path);
	if (::lstrlen(fontName) != 0)
	{
		int fontSize = ::GetPrivateProfileInt(_T("Settings"), _T("fontSize"), 0, path);
		int fontPitch = ::GetPrivateProfileInt(_T("Settings"), _T("fontPitch"), 0, path);
		int dpi = ::GetSystemDpiForProcess(::GetCurrentProcess());
		fontSize = ::MulDiv(fontSize, dpi, 96);
		m_font = ::CreateFont(fontSize, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, fontPitch, fontName);
	}

	MY_TRACE_HEX(m_font);

	initHook();

	return TRUE;
}

BOOL COptimizeEditBoxApp::func_exit(FILTER *fp)
{
	MY_TRACE(_T("COptimizeEditBoxApp::func_exit()\n"));

	::DeleteObject(m_font), m_font = 0;

	termHook();

	return TRUE;
}

BOOL COptimizeEditBoxApp::func_proc(FILTER *fp, FILTER_PROC_INFO *fpip)
{
	MY_TRACE(_T("COptimizeEditBoxApp::func_proc()\n"));

	return FALSE;
}

BOOL COptimizeEditBoxApp::Exedit_func_proc(FILTER *fp, FILTER_PROC_INFO *fpip)
{
	MY_TRACE(_T("COptimizeEditBoxApp::Exedit_func_proc()\n"));

	// ここで exedit のフィルタ関数の直前のタイミングで処理を行う。
	// exedit のフィルタ関数の直後だともう一度描画しなければならず、描画処理が 1 つ増えてしまう。

	// フレーム画像を更新する必要があるので
	// タイマーを止めて exedit のテキストオブジェクトにデフォルト処理を実行させる。
	// これによって exedit のフレーム画像更新の準備が整う。
	stopTimer();

	return TRUE;
}

LRESULT COptimizeEditBoxApp::Exedit_ObjectDialog_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//	MY_TRACE(_T("0x%08X, 0x%08X, 0x%08X)\n"), message, wParam, lParam);

	// WM_COMMAND メッセージが来たかチェックする。
	if (message == WM_COMMAND)
	{
		UINT code = HIWORD(wParam);
		UINT id = LOWORD(wParam);
		HWND sender = (HWND)lParam;

//		MY_TRACE(_T("WM_COMMAND, 0x%04X, 0x%04X, 0x%08X)\n"), code, id, sender);

		// エディットボックス通知メッセージの EN_CHANGE もしくは EN_UPDATE が来たかチェックする。
		// この 2 つをチェックするのは exedit がテキストオブジェクトは EN_CHANGE で処理をしているが、その他のオブジェクトは EN_UPDATE で処理をしているため。
		if (code == EN_CHANGE || code == EN_UPDATE)
		{
//			MY_TRACE(_T("WM_COMMAND, EN_CHANGE || EN_UPDATE, 0x%04X, 0x%04X, 0x%08X)\n"), code, id, sender);

			if (m_optimizeTimeLine)
			{
				// タイムラインウィンドウが左クリックされているかチェックする。
				if (::GetActiveWindow() == m_exeditTimelineWindow && ::GetKeyState(VK_LBUTTON) < 0) 
				{
//					MY_TRACE(_T("エディットボックスメッセージの処理を間引きます\n"));
					return 0;
				}
			}
		}

		// エディットボックス通知メッセージの EN_CHANGE がテキストオブジェクト関係のエディットボックスから送られてきたかチェックする。
		if (code == EN_CHANGE && id >= 0x5600)
		{
//			MY_TRACE(_T("WM_COMMAND, code == EN_CHANGE && id >= 0x5600, 0x%04X, 0x%04X, 0x%08X)\n"), code, id, sender);

			// エディットボックスの処理を遅延させるかチェックする。
			if (m_editBoxDelay > 0)
			{
//				MY_TRACE(_T("タイマーを使ってエディットボックスメッセージの処理を先送りにします\n"));
				startTimer(wParam, lParam, m_editBoxDelay);
				return 0;
			}
		}
	}

	return true_Exedit_ObjectDialog_WndProc(hwnd, message, wParam, lParam);
}

void COptimizeEditBoxApp::startTimer(WPARAM wParam, LPARAM lParam, int elapse)
{
//	MY_TRACE(_T("COptimizeEditBoxApp::startTimer(0x%08X, 0x%08X, %d)\n"), wParam, lParam, elapse);

	if (m_timerId)
	{
//		MY_TRACE(_T("タイマーはすでにセットされています\n"));

		// スレッドタイマーは上書きができないので削除してから作り直す。
		// (ウィンドウタイマーとして実装すれば上書きできるのでいちいち削除しなくてもよい)
		::KillTimer(0, m_timerId), m_timerId = 0;
	}

	// タイマー停止時に実行するコマンド用の変数をメンバ変数に格納しておく。
	m_wParam = wParam;
	m_lParam = lParam;
	// スレッドタイマーをセットする。
	m_timerId = ::SetTimer(0, 0, elapse, _timerProc);
//	MY_TRACE_HEX(m_wParam);
//	MY_TRACE_HEX(m_lParam);
//	MY_TRACE_HEX(m_timerId);
}

void COptimizeEditBoxApp::stopTimer()
{
	MY_TRACE(_T("COptimizeEditBoxApp::stopTimer()\n"));

	if (m_timerId)
	{
		// タイマーを削除する。
		::KillTimer(0, m_timerId), m_timerId = 0;

		// 先送りしていたメッセージのデフォルト処理を実行する。
//		MY_TRACE(_T("デフォルト処理を実行します : WM_COMMAND, 0x%08X, 0x%08X)\n"), m_wParam, m_lParam);
		true_Exedit_ObjectDialog_WndProc(m_exeditObjectDialog, WM_COMMAND, m_wParam, m_lParam);
	}
}

void COptimizeEditBoxApp::timerProc(HWND hwnd, UINT message, UINT_PTR id, DWORD time)
{
	MY_TRACE(_T("COptimizeEditBoxApp::timerProc(0x%08X, 0x%08X)\n"), id, time);

	if (id == m_timerId)
	{
		// このタイマーを止めてデフォルト処理を実行する。
		stopTimer();
	}
}

void CALLBACK COptimizeEditBoxApp::_timerProc(HWND hwnd, UINT message, UINT_PTR id, DWORD time)
{
	theApp.timerProc(hwnd, message, id, time);
}

//---------------------------------------------------------------------
