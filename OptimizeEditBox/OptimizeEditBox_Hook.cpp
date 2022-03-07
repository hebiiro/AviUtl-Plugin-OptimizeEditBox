#include "pch.h"
#include "OptimizeEditBox.h"
#include "OptimizeEditBox_Hook.h"

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
		if (theApp.m_usesCtrlA)
		{
			if (msg->message == WM_KEYDOWN &&
				msg->wParam == 'A' &&
				::GetKeyState(VK_CONTROL) < 0)
			{
				MY_TRACE(_T("Ctrl+A キーが押されました\n"));

				Edit_SetSel(msg->hwnd, 0, -1);

				msg->hwnd = 0;
				msg->message = WM_NULL;
				msg->wParam = 0;
				msg->lParam = 0;
				return result;
			}
		}

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

inline bool isAtom(LPCSTR text)
{
	return (DWORD)text <= 0x0000FFFFUL;
}

inline bool isAtom(LPCWSTR text)
{
	return (DWORD)text <= 0x0000FFFFUL;
}

IMPLEMENT_HOOK_PROC(HWND, WINAPI, CreateWindowExA, (DWORD exStyle, LPCSTR className, LPCSTR windowName,
	DWORD style, int x, int y, int w, int h, HWND parent, HMENU menu, HINSTANCE instance, LPVOID param))
{
	if (isAtom(className))
	{
		MY_TRACE(_T("CreateWindowExA(0x%08X, %s)\n"), className, windowName);

		return true_CreateWindowExA(exStyle, className, windowName, style, x, y, w, h, parent, menu, instance, param);
	}

//	MY_TRACE(_T("CreateWindowExA(%s, %s)\n"), className, windowName);
#if 0
	if (::lstrcmpiA(className, WC_EDITA) == 0)
	{
		MY_TRACE(_T("エディットボックスを UNICODE で作成します\n"));

		return ::CreateWindowExW(exStyle, (_bstr_t)className, (_bstr_t)windowName, style, x, y, w, h, parent, menu, instance, param);
	}
#endif
#if 0
	HWND result = ::CreateWindowExW(exStyle, (_bstr_t)className, (_bstr_t)windowName, style, x, y, w, h, parent, menu, instance, param);
#else
	HWND result = true_CreateWindowExA(exStyle, className, windowName, style, x, y, w, h, parent, menu, instance, param);
#endif
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

	CMyClocker clocker(theApp.m_filterWindow, _T("UsesSetRedraw"));

	// ダイアログのハンドルを取得する。
	HWND dialog = theApp.m_exeditObjectDialog;

	// 描画をロックしてからデフォルト処理を行う。
	::SendMessage(dialog, WM_SETREDRAW, FALSE, 0);
	BOOL result = true_Exedit_ShowControls(objectIndex);
	::SendMessage(dialog, WM_SETREDRAW, TRUE, 0);
//	::RedrawWindow(dialog, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);

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

void frameRect(HDC dc, LPCRECT rc, COLORREF color, int edgeWidth, int edgeHeight)
{
	int x = rc->left;
	int y = rc->top;
	int w = rc->right - rc->left;
	int h = rc->bottom - rc->top;

	HBRUSH brush = ::CreateSolidBrush(color);
	HBRUSH oldBrush = (HBRUSH)::SelectObject(dc, brush);
	if (edgeHeight > 0)
	{
		::PatBlt(dc, x, y, w, edgeHeight, PATCOPY);
		::PatBlt(dc, x, y + h, w, -edgeHeight, PATCOPY);
	}
	if (edgeWidth > 0)
	{
		::PatBlt(dc, x, y, edgeWidth, h, PATCOPY);
		::PatBlt(dc, x + w, y, -edgeWidth, h, PATCOPY);
	}
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
	frameRect(dc, &rcFrame, theApp.m_outerColor, theApp.m_outerEdgeWidth, theApp.m_outerEdgeHeight);
	::InflateRect(&rcFrame, -theApp.m_outerEdgeWidth, -theApp.m_outerEdgeHeight);
	frameRect(dc, &rcFrame, theApp.m_innerColor, theApp.m_innerEdgeWidth, theApp.m_innerEdgeHeight);
#endif
}

IMPLEMENT_HOOK_PROC_NULL(LRESULT, WINAPI, Exedit_ObjectDialog_WndProc, (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam))
{
	return theApp.Exedit_ObjectDialog_WndProc(hwnd, message, wParam, lParam);
}

void Exedit_DrawLineLeft(HDC dc, int mx, int my, int lx, int ly, HPEN pen)
{
//	MY_TRACE(_T("Exedit_DrawLineLeft(0x%08X, %d, %d, %d, %d, 0x%08X)\n"), dc, mx, my, lx, ly, pen);

	HBRUSH brush = ::CreateSolidBrush(theApp.m_layerBorderLeftColor);
	HBRUSH oldBrush = (HBRUSH)::SelectObject(dc, brush);
	::PatBlt(dc, mx, my, 1, ly - my, PATCOPY);
	::SelectObject(dc, oldBrush);
	::DeleteObject(brush);
}

void Exedit_DrawLineRight(HDC dc, int mx, int my, int lx, int ly, HPEN pen)
{
//	MY_TRACE(_T("Exedit_DrawLineRight(0x%08X, %d, %d, %d, %d, 0x%08X)\n"), dc, mx, my, lx, ly, pen);

	HBRUSH brush = ::CreateSolidBrush(theApp.m_layerBorderRightColor);
	HBRUSH oldBrush = (HBRUSH)::SelectObject(dc, brush);
	::PatBlt(dc, mx, my, 1, ly - my, PATCOPY);
	::SelectObject(dc, oldBrush);
	::DeleteObject(brush);
}

void Exedit_DrawLineTop(HDC dc, int mx, int my, int lx, int ly, HPEN pen)
{
//	MY_TRACE(_T("Exedit_DrawLineTop(0x%08X, %d, %d, %d, %d, 0x%08X)\n"), dc, mx, my, lx, ly, pen);

	HBRUSH brush = ::CreateSolidBrush(theApp.m_layerBorderTopColor);
	HBRUSH oldBrush = (HBRUSH)::SelectObject(dc, brush);
	::PatBlt(dc, mx, my, lx - mx, 1, PATCOPY);
	::SelectObject(dc, oldBrush);
	::DeleteObject(brush);
}

void Exedit_DrawLineBottom(HDC dc, int mx, int my, int lx, int ly, HPEN pen)
{
//	MY_TRACE(_T("Exedit_DrawLineBottom(0x%08X, %d, %d, %d, %d, 0x%08X)\n"), dc, mx, my, lx, ly, pen);

	HBRUSH brush = ::CreateSolidBrush(theApp.m_layerBorderBottomColor);
	HBRUSH oldBrush = (HBRUSH)::SelectObject(dc, brush);
	::PatBlt(dc, mx, my, lx - mx, 1, PATCOPY);
	::SelectObject(dc, oldBrush);
	::DeleteObject(brush);
}

LRESULT CALLBACK subclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (message)
	{
	case WM_SETFONT:
		{
			MY_TRACE(_T("WM_SETFONT, 0x%08X, 0x%08X\n"), wParam, lParam);

			if (theApp.m_font)
				wParam = (WPARAM)theApp.m_font;

			break;
		}
	}

	return ::DefSubclassProc(hwnd, message, wParam, lParam);
}

HWND WINAPI Exedit_CreateTextEditBox(DWORD exStyle, LPCWSTR className, LPCWSTR windowName,
	DWORD style, int x, int y, int w, int h, HWND parent, HMENU menu, HINSTANCE instance, LPVOID param)
{
	MY_TRACE(_T("Exedit_CreateTextEditBox(%ws, %d, %d)\n"), className, w, h);

	h += theApp.m_addTextEditBoxHeight;

	HWND hwnd = ::CreateWindowExW(exStyle, className, windowName, style, x, y, w, h, parent, menu, instance, param);
	::SetWindowSubclass(hwnd, subclassProc, (UINT_PTR)&theApp, 0);
	return hwnd;
}

HWND WINAPI Exedit_CreateScriptEditBox(DWORD exStyle, LPCWSTR className, LPCWSTR windowName,
	DWORD style, int x, int y, int w, int h, HWND parent, HMENU menu, HINSTANCE instance, LPVOID param)
{
	MY_TRACE(_T("Exedit_CreateScriptEditBox(%ws, %d, %d)\n"), className, w, h);

	h += theApp.m_addScriptEditBoxHeight;

	HWND hwnd = ::CreateWindowExW(exStyle, className, windowName, style, x, y, w, h, parent, menu, instance, param);
	::SetWindowSubclass(hwnd, subclassProc, (UINT_PTR)&theApp, 0);
	return hwnd;
}

//---------------------------------------------------------------------
