#include "pch.h"
#include "OptimizeEditBox.h"

//---------------------------------------------------------------------
// exedit のフィルタ関数のフック関数

using func_proc_type = decltype(FILTER::func_proc);
func_proc_type true_exedit_func_proc = 0;
BOOL hook_exedit_func_proc(void *fp, FILTER_PROC_INFO *fpip)
{
	MY_TRACE(_T("hook_exedit_func_proc() begin\n"));

	theApp.exedit_proc((FILTER*)fp, fpip);

	return true_exedit_func_proc(fp, fpip);
}

//---------------------------------------------------------------------
//		フィルタ構造体のポインタを渡す関数
//---------------------------------------------------------------------
EXTERN_C FILTER_DLL __declspec(dllexport) * __stdcall GetFilterTable(void)
{
	static TCHAR g_filterName[] = TEXT("エディットボックス最適化");
	static TCHAR g_filterInformation[] = TEXT("エディットボックス最適化 version 2.1.0 by 蛇色");

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
	// exedit フィルタ関数をフックする。
	FILTER* exedit = auls::Exedit_GetFilter(fp);

	MY_TRACE_HEX(exedit);

	if (exedit)
	{
		true_exedit_func_proc = exedit->func_proc;
		exedit->func_proc = hook_exedit_func_proc;
	}

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
