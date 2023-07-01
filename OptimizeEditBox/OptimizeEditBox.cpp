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

	m_usesUnicodeInput = FALSE;
	m_usesCtrlA = FALSE;
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

	m_layerBorderLeftColor = CLR_NONE;
	m_layerBorderRightColor = CLR_NONE;
	m_layerBorderTopColor = CLR_NONE;
	m_layerBorderBottomColor = CLR_NONE;
	m_layerSeparatorColor = CLR_NONE;

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

	DWORD exedit_auf = (DWORD)::GetModuleHandle(_T("exedit.auf"));

	true_Exedit_FillGradation = (Type_Exedit_FillGradation)(exedit_auf + 0x00036a70);

	if (m_layerBorderLeftColor != CLR_NONE) hookCall(exedit_auf + 0x00038845, Exedit_DrawLineLeft);
	if (m_layerBorderRightColor != CLR_NONE) hookCall(exedit_auf + 0x000388AA, Exedit_DrawLineRight);
	if (m_layerBorderTopColor != CLR_NONE) hookCall(exedit_auf + 0x00038871, Exedit_DrawLineTop);
	if (m_layerBorderBottomColor != CLR_NONE) hookCall(exedit_auf + 0x000388DA, Exedit_DrawLineBottom);
	if (m_layerSeparatorColor != CLR_NONE) hookCall(exedit_auf + 0x00037A1F, Exedit_DrawLineSeparator);

	if (m_selectionColor != CLR_NONE) writeAbsoluteAddress(exedit_auf + 0x0003807E, &m_selectionColor);
	if (m_selectionEdgeColor != CLR_NONE) writeAbsoluteAddress(exedit_auf + 0x00038076, &m_selectionEdgeColor);
	if (m_selectionBkColor != CLR_NONE) writeAbsoluteAddress(exedit_auf + 0x00038087, &m_selectionBkColor);

	if (m_addTextEditBoxHeight || m_font != nullptr)
	{
		hookAbsoluteCall(exedit_auf + 0x0008C46E, Exedit_CreateTextEditBox);
		addInt32(exedit_auf + 0x0008CC56 + 1, m_addTextEditBoxHeight);
	}

	if (m_addScriptEditBoxHeight != 0 || m_font != nullptr)
	{
		hookAbsoluteCall(exedit_auf + 0x00087658, Exedit_CreateScriptEditBox);
		addInt32(exedit_auf + 0x000876DE + 1, m_addScriptEditBoxHeight);
	}

	DetourTransactionBegin();
	DetourUpdateThread(::GetCurrentThread());

	if (m_usesUnicodeInput)
	{
		ATTACH_HOOK_PROC(GetMessageA);
//		ATTACH_HOOK_PROC(PeekMessageA);
	}

	if (m_usesGradientFill)
	{
		ATTACH_HOOK_PROC(Exedit_FillGradation);
	}

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

	if (m_usesGradientFill)
	{
		DETACH_HOOK_PROC(Exedit_FillGradation);
	}

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

	// ini ファイルから設定を読み込む。
	TCHAR path[MAX_PATH];
	::GetModuleFileName(m_instance, path, MAX_PATH);
	::PathRenameExtension(path, _T(".ini"));
	MY_TRACE_TSTR(path);

	m_usesUnicodeInput	= ::GetPrivateProfileInt(_T("Settings"), _T("usesUnicodeInput"),	m_usesUnicodeInput, path);
	m_usesCtrlA			= ::GetPrivateProfileInt(_T("Settings"), _T("usesCtrlA"),			m_usesCtrlA, path);
	m_usesGradientFill	= ::GetPrivateProfileInt(_T("Settings"), _T("usesGradientFill"),	m_usesGradientFill, path);

	MY_TRACE_INT(m_usesUnicodeInput);
	MY_TRACE_INT(m_usesCtrlA);
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
	m_layerSeparatorColor = ::GetPrivateProfileInt(_T("Settings"), _T("layerSeparatorColor"), m_layerSeparatorColor, path);

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

//---------------------------------------------------------------------
