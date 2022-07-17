#include "pch.h"
#include "SplitText.h"

//--------------------------------------------------------------------

BOOL func_init(AviUtl::FilterPlugin* fp)
{
	MY_TRACE(_T("func_init()\n"));

	g_auin.initExEditAddress();

	return TRUE;
}

BOOL func_exit(AviUtl::FilterPlugin* fp)
{
	MY_TRACE(_T("func_exit()\n"));

	return TRUE;
}

BOOL func_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp)
{
//	MY_TRACE(_T("func_WndProc(0x%08X, 0x%08X, 0x%08X)\n"), message, wParam, lParam);

	switch (message)
	{
	case AviUtl::detail::FilterPluginWindowMessage::Init:
		{
			MY_TRACE(_T("func_WndProc(Init, 0x%08X, 0x%08X)\n"), wParam, lParam);

			break;
		}
	case AviUtl::detail::FilterPluginWindowMessage::Exit:
		{
			MY_TRACE(_T("func_WndProc(Exit, 0x%08X, 0x%08X)\n"), wParam, lParam);

			break;
		}
	case AviUtl::detail::FilterPluginWindowMessage::Command:
		{
			MY_TRACE(_T("func_WndProc(Command, 0x%08X, 0x%08X)\n"), wParam, lParam);

			break;
		}
	case WM_COMMAND:
		{
			int index = LOWORD(wParam) - fp->MidFilterButton;
			MY_TRACE_INT(index);

			if (index == CHECK_SPLIT_TEXT) onSplitText(editp, fp);

			break;
		}
	}

	return FALSE;
}

//--------------------------------------------------------------------

LPCSTR check_name[] =
{
	"テキストを分解する",
};

int check_def[] = { -1 };

EXTERN_C AviUtl::FilterPluginDLL* CALLBACK GetFilterTable()
{
	LPCSTR name = "テキスト分解";
	LPCSTR information = "テキスト分解 1.0.1 by 蛇色";

	static AviUtl::FilterPluginDLL filter =
	{
		.flag =
			AviUtl::detail::FilterPluginFlag::AlwaysActive |
			AviUtl::detail::FilterPluginFlag::DispFilter |
//			AviUtl::detail::FilterPluginFlag::WindowThickFrame |
//			AviUtl::detail::FilterPluginFlag::WindowSize |
			AviUtl::detail::FilterPluginFlag::ExInformation,
		.x = 100,
		.y = 100,
		.name = name,
		.check_n = sizeof(check_name) / sizeof(*check_name),
		.check_name = check_name,
		.check_default = check_def,
		.func_init = func_init,
		.func_exit = func_exit,
		.func_WndProc = func_WndProc,
		.information = information,
	};

	return &filter;
}

//--------------------------------------------------------------------
