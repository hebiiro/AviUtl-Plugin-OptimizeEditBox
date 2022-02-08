#include "pch.h"
#include "OptimizeEditBox.h"

//---------------------------------------------------------------------
// exedit �̃t�B���^�֐��̃t�b�N�֐�

using func_proc_type = decltype(FILTER::func_proc);
func_proc_type true_exedit_func_proc = 0;
BOOL hook_exedit_func_proc(void *fp, FILTER_PROC_INFO *fpip)
{
	MY_TRACE(_T("hook_exedit_func_proc() begin\n"));

	theApp.exedit_proc((FILTER*)fp, fpip);

	return true_exedit_func_proc(fp, fpip);
}

//---------------------------------------------------------------------
//		�t�B���^�\���̂̃|�C���^��n���֐�
//---------------------------------------------------------------------
EXTERN_C FILTER_DLL __declspec(dllexport) * __stdcall GetFilterTable(void)
{
	static TCHAR filterName[] = TEXT("�G�f�B�b�g�{�b�N�X�œK��");
	static TCHAR filterInformation[] = TEXT("�G�f�B�b�g�{�b�N�X�œK�� version 3.0.0 by �֐F");

	static FILTER_DLL filter =
	{
		FILTER_FLAG_NO_CONFIG |
		FILTER_FLAG_ALWAYS_ACTIVE |
		FILTER_FLAG_DISP_FILTER |
		FILTER_FLAG_EX_INFORMATION,
		0, 0,
		filterName,
		NULL, NULL, NULL,
		NULL, NULL,
		NULL, NULL, NULL,
		NULL,//func_proc,
		func_init,
		func_exit,
		NULL,
		NULL,
		NULL, NULL,
		NULL,
		NULL,
		filterInformation,
		NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL,
	};

	return &filter;
}

//---------------------------------------------------------------------
//		������
//---------------------------------------------------------------------

BOOL func_init(FILTER *fp)
{
	// exedit �t�B���^�֐����t�b�N����B
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
//		�I��
//---------------------------------------------------------------------
BOOL func_exit(FILTER *fp)
{
	return theApp.exit(fp);
}

//---------------------------------------------------------------------
//		�t�B���^�֐�
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

//---------------------------------------------------------------------
