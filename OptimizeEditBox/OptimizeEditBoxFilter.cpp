#include "pch.h"
#include "OptimizeEditBox.h"

//---------------------------------------------------------------------
//		�t�B���^�\���̂̃|�C���^��n���֐�
//---------------------------------------------------------------------
EXTERN_C FILTER_DLL __declspec(dllexport) * __stdcall GetFilterTable(void)
{
	static TCHAR g_filterName[] = TEXT("�G�f�B�b�g�{�b�N�X�œK��");
	static TCHAR g_filterInformation[] = TEXT("�G�f�B�b�g�{�b�N�X�œK�� version 2.0.1 by �֐F");

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
//		������
//---------------------------------------------------------------------

BOOL func_init(FILTER *fp)
{
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
