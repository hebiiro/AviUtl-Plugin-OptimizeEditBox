#include "pch.h"
#include "OptimizeEditBox.h"

//---------------------------------------------------------------------

IMPLEMENT_HOOK_PROC(BOOL, WINAPI, GetMessageA, (LPMSG msg, HWND hwnd, UINT msgFilterMin, UINT msgFilterMax))
{
#if 1
	BOOL result = ::GetMessageW(msg, hwnd, msgFilterMin, msgFilterMax);
#else
	BOOL result = true_GetMessageA(msg, hwnd, msgFilterMin, msgFilterMax);
#endif
#if 1
	// 親ウィンドウを取得する。
	HWND dlg = ::GetParent(msg->hwnd);
	if (!dlg) return result;

	// ウィンドウがエディットボックスか確認する。
	TCHAR className[MAX_PATH] = {};
	::GetClassName(msg->hwnd, className, MAX_PATH);
//	MY_TRACE_TSTR(className);
	if (::lstrcmpi(className, WC_EDIT) == 0)
	{
/*
		// この処理を実行しても ESC キーでダイアログが非表示になってしまう。
		if (msg->message == WM_KEYDOWN ||
			msg->message == WM_KEYUP ||
			msg->message == WM_CHAR ||
			msg->message == WM_IME_KEYDOWN ||
			msg->message == WM_IME_KEYUP ||
			msg->message == WM_IME_CHAR)
		{
			if (msg->wParam == VK_ESCAPE ||
				msg->wParam == VK_TAB ||
				msg->wParam == VK_RETURN)
			{
				return result;
			}
		}
*/
		// ダイアログメッセージを処理する。
		if (::IsDialogMessageW(dlg, msg))
		{
			// このメッセージはディスパッチしてはならないので WM_NULL に置き換える。
			msg->hwnd = 0;
			msg->message = WM_NULL;
			msg->wParam = 0;
			msg->lParam = 0;
		}
	}
#endif
	return result;
}

IMPLEMENT_HOOK_PROC(BOOL, WINAPI, PeekMessageA, (LPMSG msg, HWND hwnd, UINT msgFilterMin, UINT msgFilterMax, UINT removeMsg))
{
	MY_TRACE(_T("PeekMessageA()\n"));

	return ::PeekMessageW(msg, hwnd, msgFilterMin, msgFilterMax, removeMsg);
//	return true_PeekMessageA(msg, hwnd, msgFilterMin, msgFilterMax, removeMsg);
}

IMPLEMENT_HOOK_PROC(HWND, WINAPI, CreateWindowExA, (DWORD exStyle, LPCSTR className, LPCSTR windowName,
	DWORD style, int x, int y, int w, int h, HWND parent, HMENU menu, HINSTANCE instance, LPVOID param))
{
//	MY_TRACE(_T("CreateWindowExA(%s, %s)\n"), className, windowName);
/*
	if (::lstrcmpiA(className, WC_EDITA) == 0)
	{
		MY_TRACE(_T("エディットボックスを UNICODE で作成します\n"));

		return ::CreateWindowExW(exStyle, (_bstr_t)className, (_bstr_t)windowName, style, x, y, w, h, parent, menu, instance, param);
	}
*/
//	HWND result = ::CreateWindowExW(exStyle, (_bstr_t)className, (_bstr_t)windowName, style, x, y, w, h, parent, menu, instance, param);
	HWND result = true_CreateWindowExA(exStyle, className, windowName, style, x, y, w, h, parent, menu, instance, param);

	if (::lstrcmpiA(className, "ExtendedFilterClass") == 0)
	{
		MY_TRACE(_T("拡張編集をフックします\n"));

		theApp.initExeditHook(result);
	}

	return result;
}

HWND getComboBox(HWND dialog)
{
	for (UINT i = 8200; i >= 8100; i--)
	{
		// ウィンドウハンドルを取得する。
		HWND hwnd = ::GetDlgItem(dialog, i);

		// コンボボックスかどうかクラス名で調べる。
		TCHAR className[MAX_PATH] = {};
		::GetClassName(hwnd, className, MAX_PATH);
		if (::lstrcmpi(className, WC_COMBOBOX) != 0) continue;

		if (::IsWindowVisible(hwnd)) // ウィンドウが可視なら
		{
			// ID - 2 のウィンドウを返す。
			return ::GetDlgItem(dialog, i - 2);
		}
	}

	return 0;
}

IMPLEMENT_HOOK_PROC_NULL(void, CDECL, Exedit_HideControls, ())
{
	MY_TRACE(_T("Exedit_HideControls()\n"));

	true_Exedit_HideControls();
}

IMPLEMENT_HOOK_PROC_NULL(BOOL, CDECL, Exedit_ShowControls, (int objectIndex))
{
	MY_TRACE(_T("Exedit_ShowControls(%d)\n"), objectIndex);

	// ダイアログのハンドルを取得する。
	HWND dialog = theApp.m_exeditObjectDialog;

	// 描画をロックしてからデフォルト処理を行う。
	::SendMessage(dialog, WM_SETREDRAW, FALSE, 0);
	BOOL result = true_Exedit_ShowControls(objectIndex);
	::SendMessage(dialog, WM_SETREDRAW, TRUE, 0);

	// 「アニメーション効果」のコンボボックスにメッセージを送信する。
	HWND combobox = getComboBox(dialog);
	MY_TRACE_HEX(combobox);
	::SendMessage(dialog, WM_CTLCOLOREDIT, 0, (LPARAM)combobox);

	return result;
}

// http://iooiau.net/tips/web20back.html
// 2色のグラデーションを描画する関数です
BOOL TwoColorsGradient(
	HDC hdc,            // 描画先のデバイスコンテキスト・ハンドルです
	const RECT *pRect,  // 描画する範囲の矩形です
	COLORREF Color1,    // 描画する一つ目の色です
	COLORREF Color2,    // 描画する二つ目の色です
	BOOL fHorizontal    // 水平のグラデーションを描画する場合は TRUE にします
)
{
	TRIVERTEX vert[2];
	GRADIENT_RECT rect = {0, 1};

	// 描画範囲と色を設定します
	vert[0].x = pRect->left;
	vert[0].y = pRect->top;
	vert[0].Red   = GetRValue(Color1) << 8;
	vert[0].Green = GetGValue(Color1) << 8;
	vert[0].Blue  = GetBValue(Color1) << 8;
	vert[0].Alpha = 0;
	vert[1].x = pRect->right;
	vert[1].y = pRect->bottom;
	vert[1].Red   = GetRValue(Color2) << 8;
	vert[1].Green = GetGValue(Color2) << 8;
	vert[1].Blue  = GetBValue(Color2) << 8;
	vert[1].Alpha = 0;
	return GradientFill(hdc, vert, 2, &rect, 1,
		fHorizontal ? GRADIENT_FILL_RECT_H : GRADIENT_FILL_RECT_V);
}

void frameRect(HDC dc, LPCRECT rc, COLORREF color, int edgeWidth)
{
	int x = rc->left;
	int y = rc->top;
	int w = rc->right - rc->left;
	int h = rc->bottom - rc->top;

	HBRUSH brush = ::CreateSolidBrush(color);
	HBRUSH oldBrush = (HBRUSH)::SelectObject(dc, brush);
	::PatBlt(dc, x, y, w, edgeWidth, PATCOPY);
	::PatBlt(dc, x, y + h, w, -edgeWidth, PATCOPY);
	::PatBlt(dc, x, y, edgeWidth, h, PATCOPY);
	::PatBlt(dc, x + w, y, -edgeWidth, h, PATCOPY);
	::SelectObject(dc, oldBrush);
	::DeleteObject(brush);
}

IMPLEMENT_HOOK_PROC_NULL(void, CDECL, Exedit_FillGradation, (HDC dc, const RECT *rc, BYTE r, BYTE g, BYTE b, BYTE gr, BYTE gg, BYTE gb, int gs, int ge))
{
	MY_TRACE(_T("Exedit_FillGradation(%d, %d)\n"), gs, ge);

	COLORREF color1 = RGB(r, g, b);
	COLORREF color2 = RGB(gr, gg, gb);
#if 1
	// 大雑把なグラデーション。
	TwoColorsGradient(dc, rc, color1, color2, TRUE);
#else
	// デフォルト処理に近いグラデーション。
	if (gs == 0 && ge == 0)
	{
		TwoColorsGradient(dc, rc, color1, color1, TRUE);
	}
	else
	{
		RECT rc1 = *rc;
		rc1.right = gs;
		TwoColorsGradient(dc, &rc1, color1, color1, TRUE);

		RECT rc2 = *rc;
		rc2.left = gs;
		rc2.right = ge;
		TwoColorsGradient(dc, &rc2, color1, color2, TRUE);

		RECT rc3 = *rc;
		rc3.left = ge;
		TwoColorsGradient(dc, &rc3, color2, color2, TRUE);
	}
#endif
#if 1
	// 枠も描画するならここを使う。
	RECT rcFrame = *rc;
	frameRect(dc, &rcFrame, RGB(0x00, 0x00, 0x00), 1);
	::InflateRect(&rcFrame, -1, -1);
	frameRect(dc, &rcFrame, RGB(0xff, 0xff, 0xff), 1);
#endif
}

IMPLEMENT_HOOK_PROC_NULL(LRESULT, WINAPI, Exedit_ObjectDialog_WndProc, (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam))
{
	return theApp.hook_exedit_wndProc(hwnd, message, wParam, lParam);
}

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
	m_exeditTimelineWindow = 0;
	m_exeditObjectDialog = 0;
	m_timerId = 0;
	m_wParam = 0;
	m_lParam = 0;

	m_optimizeTimeLine = FALSE;
	m_editBoxDelay = 0;
	m_trackBarDelay = 0;
	m_usesUnicodeInput = FALSE;
	m_usesSetRedraw = FALSE;
	m_usesGradientFill = FALSE;
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

BOOL COptimizeEditBoxApp::dllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		{
			MY_TRACE(_T("DLL_PROCESS_ATTACH\n"));

			// DLL インスタンスハンドルを m_instance に格納する。
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
	m_usesSetRedraw		= ::GetPrivateProfileInt(_T("Settings"), _T("usesSetRedraw"),		m_usesSetRedraw, path);
	m_usesGradientFill	= ::GetPrivateProfileInt(_T("Settings"), _T("usesGradientFill"),	m_usesGradientFill, path);

	MY_TRACE_INT(m_optimizeTimeLine);
	MY_TRACE_INT(m_editBoxDelay);
	MY_TRACE_INT(m_trackBarDelay);
	MY_TRACE_INT(m_usesUnicodeInput);
	MY_TRACE_INT(m_usesSetRedraw);
	MY_TRACE_INT(m_usesGradientFill);

	initHook();

	return TRUE;
}

BOOL COptimizeEditBoxApp::exit(FILTER *fp)
{
	MY_TRACE(_T("COptimizeEditBoxApp::exit()\n"));

	termHook();

	return TRUE;
}

BOOL COptimizeEditBoxApp::proc(FILTER *fp, FILTER_PROC_INFO *fpip)
{
	MY_TRACE(_T("COptimizeEditBoxApp::proc()\n"));

	return FALSE;
}

BOOL COptimizeEditBoxApp::exedit_proc(FILTER *fp, FILTER_PROC_INFO *fpip)
{
	MY_TRACE(_T("COptimizeEditBoxApp::exedit_proc()\n"));

	// ここで exedit のフィルタ関数の直前のタイミングで処理を行う。
	// exedit のフィルタ関数の直後だともう一度描画しなければならず、描画処理が 1 つ増えてしまう。

	// フレーム画像を更新する必要があるので
	// タイマーを止めて exedit のテキストオブジェクトにデフォルト処理を実行させる。
	// これによって exedit のフレーム画像更新の準備が整う。
	stopTimer();

	return TRUE;
}

LRESULT COptimizeEditBoxApp::hook_exedit_wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
