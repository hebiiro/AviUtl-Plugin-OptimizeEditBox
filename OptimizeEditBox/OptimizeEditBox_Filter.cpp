#include "pch.h"
#include "OptimizeEditBox.h"

//---------------------------------------------------------------------
// exedit �̃t�B���^�֐��̃t�b�N�֐�

using func_proc_type = decltype(FILTER::func_proc);
func_proc_type true_exedit_func_proc = 0;
BOOL hook_exedit_func_proc(void *fp, FILTER_PROC_INFO *fpip)
{
	MY_TRACE(_T("hook_exedit_func_proc() begin\n"));

	theApp.Exedit_func_proc((FILTER*)fp, fpip);

	return true_exedit_func_proc(fp, fpip);
}

//---------------------------------------------------------------------
//		�t�B���^�\���̂̃|�C���^��n���֐�
//---------------------------------------------------------------------
EXTERN_C FILTER_DLL __declspec(dllexport) * __stdcall GetFilterTable(void)
{
	static TCHAR filterName[] = TEXT("�G�f�B�b�g�{�b�N�X�œK��");
	static TCHAR filterInformation[] = TEXT("�G�f�B�b�g�{�b�N�X�œK�� version 6.0.0 by �֐F");

	static FILTER_DLL filter =
	{
//		FILTER_FLAG_NO_CONFIG | // ���̃t���O���w�肷��ƃE�B���h�E���쐬����Ȃ��Ȃ��Ă��܂��B
		FILTER_FLAG_ALWAYS_ACTIVE | // ���̃t���O���Ȃ��Ɓu�t�B���^�v�� ON/OFF ��؂�ւ��邽�߂̍��ڂ��ǉ�����Ă��܂��B
		FILTER_FLAG_DISP_FILTER | // ���̃t���O���Ȃ��Ɓu�ݒ�v�̕��ɃE�B���h�E��\�����邽�߂̍��ڂ��ǉ�����Ă��܂��B
		FILTER_FLAG_WINDOW_THICKFRAME |
		FILTER_FLAG_WINDOW_SIZE |
		FILTER_FLAG_EX_INFORMATION,
		400, 400,
		filterName,
		NULL, NULL, NULL,
		NULL, NULL,
		NULL, NULL, NULL,
		NULL,//func_proc,
		func_init,
		func_exit,
		NULL,
		func_WndProc,
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

	return theApp.func_init(fp);
}

//---------------------------------------------------------------------
//		�I��
//---------------------------------------------------------------------
BOOL func_exit(FILTER *fp)
{
	return theApp.func_exit(fp);
}

//---------------------------------------------------------------------
//		�t�B���^�֐�
//---------------------------------------------------------------------
BOOL func_proc(FILTER *fp, FILTER_PROC_INFO *fpip)
{
	return theApp.func_proc(fp, fpip);
}

//---------------------------------------------------------------------
//		WndProc
//---------------------------------------------------------------------
BOOL func_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, void *editp, FILTER *fp)
{
	// TRUE ��Ԃ��ƑS�̂��ĕ`�悳���
/*
	switch (message)
	{
	case WM_FILTER_INIT:
		{
			MY_TRACE(_T("func_WndProc(WM_FILTER_INIT)\n"));

			// ���̃t�B���^�̃E�B���h�E�n���h����ۑ����Ă����B
			g_filterWindow = fp->hwnd;
			MY_TRACE_HEX(g_filterWindow);
			::SetTimer(g_filterWindow, TIMER_ID_CHECK_UPDATE, 1000, 0);

			// �}�[�N�E�B���h�E���쐬����B
			g_dragSrcWindow = createMarkWindow(RGB(0x00, 0x00, 0xff));
			MY_TRACE_HEX(g_dragSrcWindow);
			g_dragDstWindow = createMarkWindow(RGB(0xff, 0x00, 0x00));
			MY_TRACE_HEX(g_dragDstWindow);

			g_targetMarkWindow = TargetMarkWindowPtr(new TargetMarkWindow());
			g_targetMarkWindow->Create(g_instance);

			break;
		}
	case WM_FILTER_EXIT:
		{
			MY_TRACE(_T("func_WndProc(WM_FILTER_EXIT)\n"));

			// �}�[�N�E�B���h�E���폜����B
			::DestroyWindow(g_dragSrcWindow), g_dragSrcWindow = 0;
			::DestroyWindow(g_dragDstWindow), g_dragDstWindow = 0;

			g_targetMarkWindow->Destroy();
			g_targetMarkWindow = 0;

			break;
		}
	case WM_FILTER_UPDATE:
		{
			MY_TRACE(_T("func_WndProc(WM_FILTER_UPDATE)\n"));

			break;
		}
	case WM_FILTER_CHANGE_EDIT:
		{
			MY_TRACE(_T("func_WndProc(WM_FILTER_CHANGE_EDIT)\n"));

			break;
		}
	case WM_FILTER_COMMAND:
		{
			MY_TRACE(_T("func_WndProc(WM_FILTER_COMMAND)\n"));

			if (wParam == 0 && lParam == 0)
			{
				MY_TRACE(_T("�t���[�����X�V���܂�\n"));
				return TRUE;
			}

			break;
		}
	case WM_TIMER:
		{
			if (wParam == TIMER_ID_CHECK_UPDATE)
			{
				if (g_settingsFile->isFileUpdated())
					loadSettings(g_settingsFile->getFileName());
			}

			break;
		}
	}
*/
	return FALSE;
}

//---------------------------------------------------------------------
//		DllMain
//---------------------------------------------------------------------
BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	return theApp.DllMain(instance, reason, reserved);
}

//---------------------------------------------------------------------
