﻿#include "pch.h"
#include "SplitText.h"

//--------------------------------------------------------------------

BOOL func_init(AviUtl::FilterPlugin* fp)
{
	MY_TRACE(_T("func_init()\n"));

	g_auin.initExEditAddress();

	fp->exfunc->add_menu_item(fp, "テキストオブジェクトを分解する",
		fp->hwnd, Check::SplitText, 0, AviUtl::ExFunc::AddMenuItemFlag::None);

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
	case AviUtl::FilterPlugin::WindowMessage::Init:
		{
			MY_TRACE(_T("func_WndProc(Init, 0x%08X, 0x%08X)\n"), wParam, lParam);

			break;
		}
	case AviUtl::FilterPlugin::WindowMessage::Exit:
		{
			MY_TRACE(_T("func_WndProc(Exit, 0x%08X, 0x%08X)\n"), wParam, lParam);

			break;
		}
	case AviUtl::FilterPlugin::WindowMessage::Command:
		{
			MY_TRACE(_T("func_WndProc(Command, 0x%08X, 0x%08X)\n"), wParam, lParam);

			if (wParam == 0 && lParam == 0) return TRUE;

			return onCommand(LOWORD(wParam), editp, fp);
		}
	case WM_COMMAND:
		{
			return onCommand(LOWORD(wParam) - fp->MidFilterButton, editp, fp);
		}
	}

	return FALSE;
}

//--------------------------------------------------------------------

LPCSTR track_name[] =
{
	"ﾎﾞｲｽ",
	"ﾌﾚｰﾑ",
	"幅",
};

int track_def[] = {  1,     0,     0 };
int track_min[] = {  0, -6000,     0 };
int track_max[] = { 10, +6000, 10000 };

LPCSTR check_name[] =
{
	"テキストを分解する",
	"絶対フレームモード",
	"行単位で分解する",
	"行端揃え",
	"元のオブジェクトを削除する",
};

int check_def[] = { -1, 0, 0, 0, 0 };

EXTERN_C AviUtl::FilterPluginDLL* WINAPI GetFilterTable()
{
	LPCSTR name = "テキスト分解";
	LPCSTR information = "テキスト分解 2.0.1 by 蛇色";

	static AviUtl::FilterPluginDLL filter =
	{
		.flag =
			AviUtl::FilterPluginDLL::Flag::AlwaysActive |
			AviUtl::FilterPluginDLL::Flag::DispFilter |
//			AviUtl::FilterPluginDLL::Flag::WindowThickFrame |
//			AviUtl::FilterPluginDLL::Flag::WindowSize |
			AviUtl::FilterPluginDLL::Flag::ExInformation,
		.name = name,
		.track_n = sizeof(track_name) / sizeof(*track_name),
		.track_name = track_name,
		.track_default = track_def,
		.track_s = track_min,
		.track_e = track_max,
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
