#pragma once

//---------------------------------------------------------------------

class COptimizeEditBoxApp
{
public:

	HINSTANCE m_instance;

	BOOL m_usesUnicodeInput;
	BOOL m_usesCtrlA;
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

public:

	COptimizeEditBoxApp();
	~COptimizeEditBoxApp();

	BOOL initHook();
	BOOL termHook();

	BOOL DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved);
	BOOL func_init(FILTER *fp);
	BOOL func_exit(FILTER *fp);
	BOOL func_proc(FILTER *fp, FILTER_PROC_INFO *fpip);
};

extern COptimizeEditBoxApp theApp;

//---------------------------------------------------------------------
