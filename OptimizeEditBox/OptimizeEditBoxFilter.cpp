#include "pch.h"
#include "OptimizeEditBox.h"

//---------------------------------------------------------------------
//		フィルタ構造体のポインタを渡す関数
//---------------------------------------------------------------------
EXTERN_C FILTER_DLL __declspec(dllexport) * __stdcall GetFilterTable(void)
{
	static TCHAR g_filterName[] = TEXT("エディットボックス最適化");
	static TCHAR g_filterInformation[] = TEXT("エディットボックス最適化 version 2.0.1 by 蛇色");

	static FILTER_DLL filter =
	{
		FILTER_FLAG_NO_CONFIG |
		FILTER_FLAG_ALWAYS_ACTIVE |
		FILTER_FLAG_DISP_FILTER |
		FILTER_FLAG_EX_INFORMATION,
		0, 0,
		g_filterName,
		NULL, NULL, NULL,
		NULL, NULL,
		NULL, NULL, NULL,
		func_proc,
		func_init,
		func_exit,
		NULL,
		NULL,
		NULL, NULL,
		NULL,
		NULL,
		g_filterInformation,
		NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL,
	};

	return &filter;
}

//---------------------------------------------------------------------
//		初期化
//---------------------------------------------------------------------

BOOL func_init(FILTER *fp)
{
	return theApp.init(fp);
}

//---------------------------------------------------------------------
//		終了
//---------------------------------------------------------------------
BOOL func_exit(FILTER *fp)
{
	return theApp.exit(fp);
}

//---------------------------------------------------------------------
//		フィルタ関数
//---------------------------------------------------------------------
BOOL func_proc(FILTER *fp, FILTER_PROC_INFO *fpip)
{
	return theApp.proc(fp, fpip);
}

//---------------------------------------------------------------------
//		DllMain
//---------------------------------------------------------------------
BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	return theApp.dllMain(instance, reason, reserved);
}
