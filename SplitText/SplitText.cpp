#include "pch.h"
#include "SplitText.h"

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

BOOL WritePrivateProfileIntA(LPCSTR appName, LPCSTR keyName, int value, LPCSTR fileName)
{
	char text[MAX_PATH] = {}; _itoa_s(value, text, 10);
	return ::WritePrivateProfileStringA(appName, keyName, text, fileName);
}

int getSortedObjectIndex(ExEdit::Object* object)
{
	int c = g_auin.GetCurrentSceneObjectCount();

	for (int i = 0; i < c; i++)
	{
		if (object == g_auin.GetSortedObject(i))
			return i;
	}

	return -1;
}

struct TextSize // 改行を含まない文字列のサイズを保持する。
{
	int m_width = 0;
	int m_height = 0;
	int m_centerX = 0;
	int m_centerY = 0;

	TextSize(HDC dc, LPCWSTR _text, int c, int spacing_x, int spacing_y, BOOL overhang = FALSE)
	{
		std::wstring text(_text, c);

		if (text.empty()) return;
		if (text.back() == L'\r') text.pop_back();
		if (text.empty()) return;

		SIZE size = {};
		::GetTextExtentPoint32W(dc, text.c_str(), text.length(), &size);

		m_width = size.cx;
		m_height = size.cy;

		if (overhang)
		{
			ABC abc = {};
			::GetCharABCWidthsW(dc, text.back(), text.back(), &abc);
			m_width -= abc.abcC;
		}

		m_centerX = m_width / 2;
		m_centerY = m_height / 2;
	}
};


BOOL onSplitText(AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp)
{
	MY_TRACE(_T("onSplitText()\n"));

	// カレントオブジェクトのインデックスを取得する。
	int objectIndex = g_auin.GetCurrentObjectIndex();
	MY_TRACE_INT(objectIndex);
	if (objectIndex < 0) return FALSE;

	// カレントオブジェクトを取得する。
	ExEdit::Object* object = g_auin.GetObjectA(objectIndex);
	MY_TRACE_HEX(object);
	if (!object) return FALSE;

	// オブジェクトの中の最初のフィルタを取得する。
	ExEdit::Filter* firstFilter = g_auin.GetFilter(object, 0);
	MY_TRACE_HEX(firstFilter);
	if (!firstFilter) return FALSE;

	// 最初のフィルタがテキストではないなら
	if (::lstrcmpA(firstFilter->name, "テキスト") != 0)
		return FALSE; // 選択アイテムがテキストオブジェクトではなかった。

	{
		// ボイスの番号を取得する。
		int voice = fp->track[Track::Voice];

		if (voice)
		{
			// フォルダ名を取得する。
			TCHAR folderName[MAX_PATH] = {};
			::GetModuleFileName(fp->dll_hinst, folderName, MAX_PATH);
			::PathRemoveExtension(folderName);
			MY_TRACE_TSTR(folderName);

			// wav ファイルのパスを取得する。
			TCHAR wavFileName[MAX_PATH] = {};
			::StringCbPrintf(wavFileName, sizeof(wavFileName), _T("%s\\%d.wav"), folderName, voice);
			MY_TRACE_TSTR(wavFileName);

			// ファイルが存在するなら
			if (::GetFileAttributes(wavFileName) != INVALID_FILE_ATTRIBUTES)
			{
				// wav ファイルを再生する。
				::PlaySound(wavFileName, 0, SND_FILENAME | SND_ASYNC);
			}
		}
	}

	// テンポラリフォルダのパスを取得する。
	char tempPath[MAX_PATH] = {};
	::GetTempPathA(MAX_PATH, tempPath);
	MY_TRACE_STR(tempPath);

	// カレントプロセスの ID を取得する。ファイル名の重複を防ぐのに使用する。
	DWORD pid = ::GetCurrentProcessId();
	MY_TRACE_INT(pid);

	// テンポラリファイル名を取得する。
	char tempFileName[MAX_PATH] = {};
	::StringCbPrintfA(tempFileName, sizeof(tempFileName), "%s\\SplitText%d.exo", tempPath, pid);
	MY_TRACE_STR(tempFileName);

	// 現在のシーンを exo ファイルに保存する。
	g_auin.SaveExo(tempFileName);

	// ソート済みオブジェクトのインデックスを取得する。
	// このインデックスが exo ファイル内の番号になる。
	int sortedObjectIndex = getSortedObjectIndex(object);
	MY_TRACE_INT(sortedObjectIndex);

	// オブジェクトのセクション名を取得する。
	char objectAppName[MAX_PATH] = {};
	::StringCbPrintfA(objectAppName, sizeof(objectAppName), "%d", sortedObjectIndex);
	MY_TRACE_STR(objectAppName);

	// レイヤーを取得する。
	int layer = ::GetPrivateProfileIntA(objectAppName, "layer", 1, tempFileName);
	MY_TRACE_INT(layer);

	// 開始フレームを取得する。
	int start = ::GetPrivateProfileIntA(objectAppName, "start", 1, tempFileName);
	MY_TRACE_INT(start);

	// 終了フレームを取得する。
	int end = ::GetPrivateProfileIntA(objectAppName, "end", 1, tempFileName);
	MY_TRACE_INT(end);

	// 最初のフィルタのセクション名を取得する。
	char firstFilterAppName[MAX_PATH] = {};
	::StringCbPrintfA(firstFilterAppName, sizeof(firstFilterAppName), "%d.0", sortedObjectIndex);
	MY_TRACE_STR(firstFilterAppName);

	// _name をチェックする。
	char _name[MAX_PATH] = {};
	::GetPrivateProfileStringA(firstFilterAppName, "_name", "", _name, _countof(_name), tempFileName);
	MY_TRACE_STR(_name);
	if (::lstrcmpA(_name, "テキスト") != 0) return FALSE; // テキストオブジェクトではなかった。

	// テキストオブジェクトの属性を取得する。
	int size = ::GetPrivateProfileIntA(firstFilterAppName, "サイズ", 1, tempFileName);
	BOOL bold = ::GetPrivateProfileIntA(firstFilterAppName, "B", FALSE, tempFileName);
	BOOL italic = ::GetPrivateProfileIntA(firstFilterAppName, "I", FALSE, tempFileName);
	int align = ::GetPrivateProfileIntA(firstFilterAppName, "align", 0, tempFileName);
	int spacing_x = (signed char)(BYTE)::GetPrivateProfileIntA(firstFilterAppName, "spacing_x", 0, tempFileName);
	int spacing_y = (signed char)(BYTE)::GetPrivateProfileIntA(firstFilterAppName, "spacing_y", 0, tempFileName);

	// フォント名を取得する。
	char fontName[MAX_PATH] = {};
	::GetPrivateProfileStringA(firstFilterAppName, "font", "", fontName, _countof(fontName), tempFileName);
	MY_TRACE_STR(fontName);

	// テキストはエディットボックスから取得するのでここでは取得しない。
//	char hexText[4096 + 1] = {};
//	::GetPrivateProfileStringA(firstFilterAppName, "text", "", hexText, _countof(hexText), tempFileName);

	// オブジェクトの x と y を取得する。
	int ox = 0;
	int oy = 0;
	int drawFilterIndex = -1;

	for (int i = 0; i < ExEdit::Object::MAX_FILTER; i++)
	{
		// セクション名を作成する。
		char appName[MAX_PATH] = {};
		::StringCbPrintfA(appName, sizeof(appName), "%d.%d", sortedObjectIndex, i);
		MY_TRACE_STR(appName);

		// _name をチェックする。
		char _name[MAX_PATH] = {};
		::GetPrivateProfileStringA(appName, "_name", "", _name, _countof(_name), tempFileName);
		MY_TRACE_STR(_name);

		if (::lstrcmpA(_name, "標準描画") != 0 && ::lstrcmpA(_name, "拡張描画") != 0)
			continue; // このフィルタは標準描画でも拡張描画でもなかった。

		// x と y を取得する。
		ox = ::GetPrivateProfileIntA(appName, "X", 0, tempFileName);
		oy = ::GetPrivateProfileIntA(appName, "Y", 0, tempFileName);

		// 描画フィルタのインデックスを保存しておく。
		drawFilterIndex = i;
	}

	// テンポラリファイル名を取得する。(文字列分割後の exo ファイル)
	char tempFileNameSplit[MAX_PATH] = {};
	::StringCbPrintfA(tempFileNameSplit, sizeof(tempFileNameSplit), "%s\\SplitText%dSplit.exo", tempPath, pid);
	MY_TRACE_STR(tempFileNameSplit);

	// セクションをコピーするのに使うバッファ。
	std::vector<char> sectionBuffer(70000, '\0');

	// [exedit] をコピーする。
	::GetPrivateProfileSectionA("exedit", sectionBuffer.data(), (DWORD)sectionBuffer.size(), tempFileName);
	if (::GetLastError() == 0)
		::WritePrivateProfileSectionA("exedit", sectionBuffer.data(), tempFileNameSplit);

	// 描画フィルタのセクション名を作成する。
	char drawFilterAppName[MAX_PATH] = {};
	::StringCbPrintfA(drawFilterAppName, sizeof(drawFilterAppName), "%d.%d", sortedObjectIndex, drawFilterIndex);
	MY_TRACE_STR(drawFilterAppName);

/*
0000100C Edit		X
0000100D Edit		Y
00001004 Edit		サイズ
00005654 ComboBox	フォント名
00005657 ComboBox	coord
0000044D Button		Bold
0000044D Button		Italic
00005658 Edit		字間
00005659 Edit		行間
00005655 Edit		テキスト
*/

	// テキストが入っているエディットボックスを取得する。
	HWND edit = ::GetDlgItem(g_auin.GetSettingDialog(), 0x5655);
	MY_TRACE_HEX(edit);

	int lineIndex = 0;
	int splitObjectIndex = 0;

	// エディットボックスからテキストを取得する。
	std::wstring text;
	{
		int length = ::GetWindowTextLengthW(edit);
		text.resize(length + 1);
		::GetWindowTextW(edit, text.data(), text.size());
		text.resize(length);
	}

	// 文字列矩形を取得するための HDC を用意する。
	ClientDC dc(fp->hwnd);
	GdiObj font = ::CreateFontA(-size, 0, 0, 0,
		bold ? FW_BOLD : FW_NORMAL, italic, FALSE, FALSE,
		DEFAULT_CHARSET, 0, 0, 0, 0, fontName);
	GdiObjSelector fontSelector(dc, font);

	::SetTextCharacterExtra(dc, spacing_x);

//	TEXTMETRICW tm = {};
//	::GetTextMetricsW(dc, &tm);

	// 基準となる文字列矩形を取得する。
	TextSize baseSize(dc, L"A", 1, spacing_x, spacing_y);

	// 分解後のアイテムの総数。
	int splitObjectCount = 0;

	// テキスト全体の大きさ。
	int wholeWidth = 0;
	int wholeHeight = 0;

	{
		// ここで分解後のアイテムの総数とテキスト全体の大きさを取得する。

		std::wstringstream ss(text);
		std::wstring line;
		while (std::getline(ss, line, L'\n'))
		{
			TextSize lineSize(dc, line.c_str(), line.length(), spacing_x, spacing_y, TRUE);

			if (lineSize.m_width == 0)
				continue;

			wholeWidth = std::max(wholeWidth, lineSize.m_width);
			wholeHeight += baseSize.m_height + spacing_y;

			for (int charIndex = 0; charIndex < (int)line.length(); charIndex++)
			{
				WCHAR ch = line[charIndex];

				if (ch == L'\r' || ch == L'\n')
					continue; // 改行文字は除外する。

				splitObjectCount++;
			}
		}

		wholeHeight -= spacing_y;

		MY_TRACE_INT(splitObjectCount);
		MY_TRACE_INT(wholeWidth);
		MY_TRACE_INT(wholeHeight);
	}

	// テキスト全体の中心座標。
	int wholeCenterX = wholeWidth / 2;
	int wholeCenterY = wholeHeight / 2;

	std::wstringstream ss(text);
	std::wstring line;
	while (std::getline(ss, line, L'\n'))
	{
		MY_TRACE_WSTR(line.c_str());

		// y を決定する。
		int y = oy + (baseSize.m_height + spacing_y) * lineIndex; // [上]

		switch (align)
		{
		case 3:
		case 4:
		case 5:
			{
				// [中]

				y -= wholeCenterY;
				y += baseSize.m_centerY;

				break;
			}
		case 6:
		case 7:
		case 8:
			{
				// [下]

				y -= wholeHeight;
				y += baseSize.m_height;

				break;
			}
		}

		TextSize lineSize(dc, line.c_str(), line.length(), spacing_x, spacing_y, TRUE);

		for (int charIndex = 0; charIndex < (int)line.length(); charIndex++)
		{
			WCHAR ch = line[charIndex];
			MY_TRACE_HEX(ch);

			if (ch == L'\r' || ch == L'\n')
				continue; // 改行文字は除外する。

			// 文字列矩形を取得する。
			TextSize textSize(dc, line.c_str(), charIndex, spacing_x, spacing_y);

			// 文字の矩形を取得する。
			TextSize charSize(dc, line.c_str() + charIndex, 1, spacing_x, spacing_y, TRUE);

			// x を決定する。
			int x = ox + textSize.m_width; // 左寄せ

			switch (align)
			{
			case 1:
			case 4:
			case 7:
				{
					// 中央揃え

					x -= lineSize.m_centerX;
					x += charSize.m_centerX;

					break;
				}
			case 2:
			case 5:
			case 8:
				{
					// 右寄せ

					x -= lineSize.m_width;
					x += charSize.m_width;

					break;
				}
			}

			MY_TRACE(_T("line = %d, char = %d, x = %d, y = %d\n"), lineIndex, charIndex, x, y);

			// 16進数文字に変換されたテキストを取得する。
			BYTE* hex = (BYTE*)&line[charIndex];
			char hexText[4096 + 1] = {};
			::StringCbPrintfA(hexText, sizeof(hexText), "%02X%02X", hex[0], hex[1]);
			memset(hexText + 4, '0', 4096 - 4);
			MY_TRACE_STR(hexText);

			// フレームオフセットを取得する。
			int frameOffset = 0;

			// 絶対フレームモードなら
			if (fp->check[Check::AbsoluteFrameMode])
			{
				int itemLength = end - start + 1;
				int range = fp->track[Track::Frame] - itemLength;
				frameOffset = ::MulDiv(range, splitObjectIndex, splitObjectCount - 1);
			}
			// 相対フレームモードなら
			else
			{
				frameOffset = splitObjectIndex * fp->track[Track::Frame];
			}

			::WritePrivateProfileIntA(objectAppName, "layer", layer + splitObjectIndex + 1, tempFileName);
			::WritePrivateProfileIntA(objectAppName, "start", start + frameOffset, tempFileName);
			::WritePrivateProfileIntA(objectAppName, "end", end + frameOffset, tempFileName);
			::WritePrivateProfileIntA(drawFilterAppName, "X", x, tempFileName);
			::WritePrivateProfileIntA(drawFilterAppName, "Y", y, tempFileName);
			::WritePrivateProfileStringA(firstFilterAppName, "text",  hexText, tempFileName);

			char objectAppNameSplit[MAX_PATH] = {};
			::StringCbPrintfA(objectAppNameSplit, sizeof(objectAppNameSplit), "%d", splitObjectIndex);
			MY_TRACE_STR(objectAppNameSplit);

			::GetPrivateProfileSectionA(objectAppName, sectionBuffer.data(), (DWORD)sectionBuffer.size(), tempFileName);
			if (::GetLastError() == 0)
				::WritePrivateProfileSectionA(objectAppNameSplit, sectionBuffer.data(), tempFileNameSplit);

			for (int i = 0; i < ExEdit::Object::MAX_FILTER; i++)
			{
				char filterAppName[MAX_PATH] = {};
				::StringCbPrintfA(filterAppName, sizeof(filterAppName), "%d.%d", sortedObjectIndex, i);
//				MY_TRACE_STR(filterAppName);

				char filterAppNameSplit[MAX_PATH] = {};
				::StringCbPrintfA(filterAppNameSplit, sizeof(filterAppNameSplit), "%d.%d", splitObjectIndex, i);
//				MY_TRACE_STR(filterAppNameSplit);

				::GetPrivateProfileSectionA(filterAppName, sectionBuffer.data(), (DWORD)sectionBuffer.size(), tempFileName);
				if (::GetLastError() == 0)
					::WritePrivateProfileSectionA(filterAppNameSplit, sectionBuffer.data(), tempFileNameSplit);
			}

			splitObjectIndex++;
		}

		lineIndex++;
	}

	g_auin.LoadExo(tempFileNameSplit, 0, 0, fp, editp);

	::DeleteFileA(tempFileName);
	::DeleteFileA(tempFileNameSplit);

	return TRUE;
}

//--------------------------------------------------------------------
