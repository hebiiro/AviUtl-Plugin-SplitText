#include "pch.h"
#include "SplitText.h"
#include "TextSplitter.h"

//--------------------------------------------------------------------

// デバッグ用コールバック関数。デバッグメッセージを出力する。
void ___outputLog(LPCTSTR text, LPCTSTR output)
{
	::OutputDebugString(output);
}

//--------------------------------------------------------------------

AviUtlInternal g_auin;

//--------------------------------------------------------------------

BOOL onCommand(int commandIndex, AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp)
{
	MY_TRACE(_T("onCommand(%d)\n"), commandIndex);

	if (commandIndex == Check::SplitText) onSplitText(editp, fp);

	return FALSE;
}

BOOL onSplitText(AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp)
{
	MY_TRACE(_T("onSplitText()\n"));

	TextSplitter splitter(editp, fp);

	return splitter.splitText();
}

//--------------------------------------------------------------------
