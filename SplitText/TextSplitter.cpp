#include "pch.h"
#include "TextSplitter.h"
#include "SplitText.h"

//--------------------------------------------------------------------

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

//--------------------------------------------------------------------

TextSplitter::TextSplitter(AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp)
{
	m_editp = editp;
	m_fp = fp;
}

BOOL TextSplitter::playVoice(int voice)
{
	if (!voice)
		return FALSE;

	// フォルダ名を取得する。
	TCHAR folderName[MAX_PATH] = {};
	::GetModuleFileName(m_fp->dll_hinst, folderName, MAX_PATH);
	::PathRemoveExtension(folderName);
	MY_TRACE_TSTR(folderName);

	// wav ファイルのパスを取得する。
	TCHAR wavFileName[MAX_PATH] = {};
	::StringCbPrintf(wavFileName, sizeof(wavFileName), _T("%s\\%d.wav"), folderName, voice);
	MY_TRACE_TSTR(wavFileName);

	g_auin.voice(wavFileName);

	return TRUE;
}

BOOL TextSplitter::getTempFileName()
{
	// テンポラリフォルダのパスを取得する。
	char tempPath[MAX_PATH] = {};
	::GetTempPathA(MAX_PATH, tempPath);
	MY_TRACE_STR(tempPath);

//	::StringCbCopy(tempPath, sizeof(tempPath), "F:");

	// カレントプロセスの ID を取得する。ファイル名の重複を防ぐのに使用する。
	DWORD pid = ::GetCurrentProcessId();
	MY_TRACE_INT(pid);

	// テンポラリファイル名を取得する。
	::StringCbPrintfA(m_tempFileName, sizeof(m_tempFileName), "%s\\SplitText%d.exo", tempPath, pid);
	MY_TRACE_STR(m_tempFileName);

	// テンポラリファイル名を取得する。(文字列分割後の exo ファイル)
	::StringCbPrintfA(m_tempFileNameSplit, sizeof(m_tempFileNameSplit), "%s\\SplitText%dSplit.exo", tempPath, pid);
	MY_TRACE_STR(m_tempFileNameSplit);

	return TRUE;
}

BOOL TextSplitter::getTextObjectInfo()
{
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

	playVoice(m_fp->track[Track::Voice]);

	getTempFileName();

	// 現在のシーンを exo ファイルに保存する。
	g_auin.SaveExo(m_tempFileName);

	// ソート済みオブジェクトのインデックスを取得する。
	// このインデックスが exo ファイル内の番号になる。
	m_sortedObjectIndex = getSortedObjectIndex(object);
	MY_TRACE_INT(m_sortedObjectIndex);

	// オブジェクトのセクション名を取得する。
	::StringCbPrintfA(m_objectAppName, sizeof(m_objectAppName), "%d", m_sortedObjectIndex);
	MY_TRACE_STR(m_objectAppName);

	// レイヤーを取得する。
	m_layer = ::GetPrivateProfileIntA(m_objectAppName, "layer", 1, m_tempFileName);
	MY_TRACE_INT(m_layer);

	// 開始フレームを取得する。
	m_start = ::GetPrivateProfileIntA(m_objectAppName, "start", 1, m_tempFileName);
	MY_TRACE_INT(m_start);

	// 終了フレームを取得する。
	m_end = ::GetPrivateProfileIntA(m_objectAppName, "end", 1, m_tempFileName);
	MY_TRACE_INT(m_end);

	// 最初のフィルタのセクション名を取得する。
	::StringCbPrintfA(m_firstFilterAppName, sizeof(m_firstFilterAppName), "%d.0", m_sortedObjectIndex);
	MY_TRACE_STR(m_firstFilterAppName);

	// _name をチェックする。
	char _name[MAX_PATH] = {};
	::GetPrivateProfileStringA(m_firstFilterAppName, "_name", "", _name, _countof(_name), m_tempFileName);
	MY_TRACE_STR(_name);
	if (::lstrcmpA(_name, "テキスト") != 0) return FALSE; // テキストオブジェクトではなかった。

	// テキストオブジェクトの属性を取得する。
	m_size = ::GetPrivateProfileIntA(m_firstFilterAppName, "サイズ", 1, m_tempFileName);
	m_bold = ::GetPrivateProfileIntA(m_firstFilterAppName, "B", FALSE, m_tempFileName);
	m_italic = ::GetPrivateProfileIntA(m_firstFilterAppName, "I", FALSE, m_tempFileName);
	m_align = ::GetPrivateProfileIntA(m_firstFilterAppName, "align", 0, m_tempFileName);
	m_spacing_x = (signed char)(BYTE)::GetPrivateProfileIntA(m_firstFilterAppName, "spacing_x", 0, m_tempFileName);
	m_spacing_y = (signed char)(BYTE)::GetPrivateProfileIntA(m_firstFilterAppName, "spacing_y", 0, m_tempFileName);

	// フォント名を取得する。
	::GetPrivateProfileStringA(m_firstFilterAppName, "font", "", m_fontName, _countof(m_fontName), m_tempFileName);
	MY_TRACE_STR(m_fontName);

	// オブジェクトの x と y を取得する。
	memset(&m_ox, 0, sizeof(m_ox));
	memset(&m_oy, 0, sizeof(m_oy));
	m_drawFilterIndex = -1;

	for (int i = 0; i < ExEdit::Object::MAX_FILTER; i++)
	{
		// セクション名を作成する。
		char appName[MAX_PATH] = {};
		::StringCbPrintfA(appName, sizeof(appName), "%d.%d", m_sortedObjectIndex, i);
		MY_TRACE_STR(appName);

		// _name をチェックする。
		char _name[MAX_PATH] = {};
		::GetPrivateProfileStringA(appName, "_name", "", _name, _countof(_name), m_tempFileName);
		MY_TRACE_STR(_name);

		if (::lstrcmpA(_name, "標準描画") != 0 && ::lstrcmpA(_name, "拡張描画") != 0)
			continue; // このフィルタは標準描画でも拡張描画でもなかった。

		// x と y を取得する。
		getPos(&m_ox, appName, "X");
		getPos(&m_oy, appName, "Y");

		// 描画フィルタのインデックスを保存しておく。
		m_drawFilterIndex = i;

		break;
	}

	return TRUE;
}

BOOL TextSplitter::getPos(Pos* pos, LPCSTR appName, LPCSTR label)
{
	char text[MAX_PATH] = {};
	::GetPrivateProfileStringA(appName, label, "", text, _countof(text), m_tempFileName);
	MY_TRACE_STR(text);

	LPSTR p = text;
	LPSTR sep1 = strchr(p, ',');

	if (!sep1)
	{
		// text が数字のみ。
		pos->m_mode = PosMode::Solo;
		pos->m_pos[Pos::Begin] = atof(p);
		return TRUE;
	}

	LPSTR sep2 = strchr(sep1 + 1, ',');

	if (!sep2)
	{
		// text が数字 2 個のみ。
		pos->m_mode = PosMode::Pair;
		pos->m_pos[Pos::Begin] = atof(p);
		pos->m_pos[Pos::End] = atof(sep1 + 1);
		return TRUE;
	}

	// text が数字 2 個とアニメーションテキスト。

	pos->m_mode = PosMode::Animation;
	pos->m_pos[Pos::Begin] = atof(p);
	pos->m_pos[Pos::End] = atof(sep1 + 1);
	::StringCbPrintf(pos->m_animation, sizeof(pos->m_animation), sep2 + 1);
	return TRUE;
}

BOOL TextSplitter::getText()
{
	// テキストが入っているエディットボックスを取得する。
	HWND edit = ::GetDlgItem(g_auin.GetSettingDialog(), 0x5655);
	MY_TRACE_HEX(edit);

	// エディットボックスからテキストを取得する。
	int length = ::GetWindowTextLengthW(edit);
	m_text.resize(length + 1);
	::GetWindowTextW(edit, m_text.data(), m_text.size());
	m_text.resize(length);

	return TRUE;
}

std::wstring TextSplitter::getLine(const std::wstring& _line)
{
	std::wstring line = _line;
	std::wstring chars = L"\r\n";
 
	for (WCHAR c : chars)
		line.erase(std::remove(line.begin(), line.end(), c), line.end());

	return line;
}

BOOL TextSplitter::getBaseSizeInfo(HDC dc)
{
	// 基準となる文字列矩形を取得する。
	m_baseSize.init(dc, L"A", 1, m_spacing_x, m_spacing_y);

	// 分解後のアイテムの総数。
	m_splitObjectCount = 0;

	// テキスト全体の大きさ。
	m_wholeWidth = 0;
	m_wholeHeight = 0;

	{
		// ここで分解後のアイテムの総数とテキスト全体の大きさを取得する。

		std::wstringstream ss(m_text);
		std::wstring line;
		while (std::getline(ss, line, L'\n'))
		{
			line = getLine(line);

			TextSize lineSize(dc, line.c_str(), line.length(), m_spacing_x, m_spacing_y, TRUE);

			if (lineSize.m_width == 0)
				continue;

			m_wholeWidth = std::max(m_wholeWidth, lineSize.m_width);
			m_wholeHeight += m_baseSize.m_height + m_spacing_y;

			// 行分解モードなら
			if (m_fp->check[Check::SplitToRow])
			{
				m_splitObjectCount++; // 行数をカウントする。
			}
			else
			{
				for (int charIndex = 0; charIndex < (int)line.length(); charIndex++)
				{
					WCHAR ch = line[charIndex];

					m_splitObjectCount++; // 文字数をカウントする。
				}
			}
		}

		m_wholeHeight -= m_spacing_y;

		MY_TRACE_INT(m_splitObjectCount);
		MY_TRACE_INT(m_wholeWidth);
		MY_TRACE_INT(m_wholeHeight);
	}

	// テキスト全体の中心座標。
	m_wholeCenterX = m_wholeWidth / 2;
	m_wholeCenterY = m_wholeHeight / 2;

	m_justificationWidth = m_wholeWidth;
	if (m_fp->track[Track::Justification] != 0)
		m_justificationWidth = m_fp->track[Track::Justification];

	for (int i = 0; i < 2; i++)
	{
		m_justificationX[i] = m_ox.m_pos[i]; // [上]

		switch (m_align)
		{
		case 1:
		case 4:
		case 7:
			{
				// 中央揃え

				m_justificationX[i] -= m_justificationWidth / 2;

				break;
			}
		case 2:
		case 5:
		case 8:
			{
				// 右寄せ

				m_justificationX[i] -= m_justificationWidth;

				break;
			}
		}
	}

	return TRUE;
}

BOOL TextSplitter::writeChar(LPCWSTR text, double x[2], double y[2])
{
	// 16進数文字に変換されたテキストを取得する。
	char hexText[4096 + 1] = {};
	BYTE* hex = (BYTE*)text;
	::StringCbPrintfA(hexText, sizeof(hexText), "%02X%02X", hex[0], hex[1]);
	memset(hexText + 4, '0', 4096 - 4);
	MY_TRACE_STR(hexText);

	// フレームオフセットを取得する。
	int frameOffset = 0;

	// 絶対フレームモードなら
	if (m_fp->check[Check::AbsoluteFrameMode])
	{
		int itemLength = m_end - m_start + 1;
		int range = m_fp->track[Track::Frame] - itemLength;
		frameOffset = ::MulDiv(range, m_splitObjectIndex, m_splitObjectCount - 1);
	}
	// 相対フレームモードなら
	else
	{
		frameOffset = m_splitObjectIndex * m_fp->track[Track::Frame];
	}

	int layer = m_layer + m_splitObjectIndex;

	// 元のオブジェクトを削除しないなら
	if (!m_fp->check[Check::DeleteOriginal])
		layer++; // レイヤーを 1 つ下に下げる。

	::WritePrivateProfileIntA(m_objectAppName, "layer", layer, m_tempFileName);
	::WritePrivateProfileIntA(m_objectAppName, "start", m_start + frameOffset, m_tempFileName);
	::WritePrivateProfileIntA(m_objectAppName, "end", m_end + frameOffset, m_tempFileName);
	writePos(&m_ox, "X", x);
	writePos(&m_oy, "Y", y);
	::WritePrivateProfileStringA(m_firstFilterAppName, "text",  hexText, m_tempFileName);

	char objectAppNameSplit[MAX_PATH] = {};
	::StringCbPrintfA(objectAppNameSplit, sizeof(objectAppNameSplit), "%d", m_splitObjectIndex);
	MY_TRACE_STR(objectAppNameSplit);

	::GetPrivateProfileSectionA(m_objectAppName, m_sectionBuffer.data(), (DWORD)m_sectionBuffer.size(), m_tempFileName);
	if (::GetLastError() == 0)
		::WritePrivateProfileSectionA(objectAppNameSplit, m_sectionBuffer.data(), m_tempFileNameSplit);

	for (int i = 0; i < ExEdit::Object::MAX_FILTER; i++)
	{
		char filterAppName[MAX_PATH] = {};
		::StringCbPrintfA(filterAppName, sizeof(filterAppName), "%d.%d", m_sortedObjectIndex, i);
//		MY_TRACE_STR(filterAppName);

		char filterAppNameSplit[MAX_PATH] = {};
		::StringCbPrintfA(filterAppNameSplit, sizeof(filterAppNameSplit), "%d.%d", m_splitObjectIndex, i);
//		MY_TRACE_STR(filterAppNameSplit);

		::GetPrivateProfileSectionA(filterAppName, m_sectionBuffer.data(), (DWORD)m_sectionBuffer.size(), m_tempFileName);
		if (::GetLastError() == 0)
			::WritePrivateProfileSectionA(filterAppNameSplit, m_sectionBuffer.data(), m_tempFileNameSplit);
	}

	return TRUE;
}

BOOL TextSplitter::writePos(Pos* pos, LPCSTR label, double value[2])
{
	char text[MAX_PATH] = {};

	switch (pos->m_mode)
	{
	case PosMode::Solo:
		{
			::StringCbPrintf(text, sizeof(text), "%.1f", value[0]);
			break;
		}
	case PosMode::Pair:
		{
			::StringCbPrintf(text, sizeof(text), "%.1f,%.1f", value[0], value[1]);
			break;
		}
	case PosMode::Animation:
		{
			::StringCbPrintf(text, sizeof(text), "%.1f,%.1f,%s", value[0], value[1], pos->m_animation);
			break;
		}
	}

	MY_TRACE_STR(text);

	::WritePrivateProfileStringA(m_drawFilterAppName, label, text, m_tempFileName);

	return TRUE;
}

BOOL TextSplitter::splitToChar(HDC dc, const std::wstring& line, double y[2])
{
	TextSize lineSize(dc, line.c_str(), line.length(), m_spacing_x, m_spacing_y, TRUE);

	int extraWidth = m_justificationWidth - lineSize.m_width;

	for (int charIndex = 0; charIndex < (int)line.length(); charIndex++)
	{
		WCHAR ch = line[charIndex];
		MY_TRACE_HEX(ch);

		// 文字列矩形を取得する。
		TextSize textSize(dc, line.c_str(), charIndex, m_spacing_x, m_spacing_y);

		// 文字の矩形を取得する。
		TextSize charSize(dc, line.c_str() + charIndex, 1, m_spacing_x, m_spacing_y, TRUE);

		// x を算出する。

		double x[2] = {};

		for (int i = 0; i < m_ox.getCount(); i++)
		{
			// 行端揃えなら
			if (m_fp->check[Check::Justification])
			{
				x[i] = m_ox.m_pos[i] + textSize.m_width; // 左寄せ

				if (line.length() >= 2)
					x[i] += ::MulDiv(extraWidth, charIndex, line.length() - 1);

				switch (m_align)
				{
				case 1:
				case 4:
				case 7:
					{
						// 中央揃え

						x[i] -= m_justificationWidth / 2;
						x[i] += charSize.m_centerX;

						break;
					}
				case 2:
				case 5:
				case 8:
					{
						// 右寄せ

						x[i] -= m_justificationWidth;
						x[i] += charSize.m_width;

						break;
					}
				}
			}
			// 行端揃えではないなら
			else
			{
				x[i] = m_ox.m_pos[i] + textSize.m_width; // 左寄せ

				switch (m_align)
				{
				case 1:
				case 4:
				case 7:
					{
						// 中央揃え

						x[i] -= lineSize.m_centerX;
						x[i] += charSize.m_centerX;

						break;
					}
				case 2:
				case 5:
				case 8:
					{
						// 右寄せ

						x[i] -= lineSize.m_width;
						x[i] += charSize.m_width;

						break;
					}
				}
			}
		}

		MY_TRACE(_T("line = %d, char = %d, x = (%f, %f), y = (%f, %f)\n"), m_lineIndex, charIndex, x[0], x[1], y[0], y[1]);

		writeChar(&line[charIndex], x, y);

		m_splitObjectIndex++;
	}

	return TRUE;
}

BOOL TextSplitter::splitToRow(HDC dc, const std::wstring& line, double y[2])
{
	TextSize lineSize(dc, line.c_str(), line.length(), m_spacing_x, m_spacing_y, TRUE);

	// x を算出する。

	double x[2] = {};

	for (int i = 0; i < m_ox.getCount(); i++)
		x[i] = m_ox.m_pos[i];

	MY_TRACE(_T("line = %d, x = (%f, %f), y = (%f, %f)\n"), m_lineIndex, x[0], x[1], y[0], y[1]);

	// 16進数文字に変換されたテキストを取得する。
	char hexText[4096 + 1] = {};
	int c = std::min((int)line.length(), 1024);
	for (int i = 0; i < c; i++)
	{
		BYTE* hex = (BYTE*)&line[i];
		::StringCchPrintfA(hexText + i * 4, _countof(hexText) - i * 4, "%02X%02X", hex[0], hex[1]);
	}
	memset(hexText + c * 4, '0', 4096 - c * 4);
	MY_TRACE_STR(hexText);

	// フレームオフセットを取得する。
	int frameOffset = 0;

	// 絶対フレームモードなら
	if (m_fp->check[Check::AbsoluteFrameMode])
	{
		int itemLength = m_end - m_start + 1;
		int range = m_fp->track[Track::Frame] - itemLength;
		frameOffset = ::MulDiv(range, m_splitObjectIndex, m_splitObjectCount - 1);
	}
	// 相対フレームモードなら
	else
	{
		frameOffset = m_splitObjectIndex * m_fp->track[Track::Frame];
	}

	int layer = m_layer + m_splitObjectIndex;

	// 元のオブジェクトを削除しないなら
	if (!m_fp->check[Check::DeleteOriginal])
		layer++; // レイヤーを 1 つ下に下げる。

	::WritePrivateProfileIntA(m_objectAppName, "layer", layer, m_tempFileName);
	::WritePrivateProfileIntA(m_objectAppName, "start", m_start + frameOffset, m_tempFileName);
	::WritePrivateProfileIntA(m_objectAppName, "end", m_end + frameOffset, m_tempFileName);
	writePos(&m_ox, "X", x);
	writePos(&m_oy, "Y", y);
	::WritePrivateProfileStringA(m_firstFilterAppName, "text",  hexText, m_tempFileName);

	char objectAppNameSplit[MAX_PATH] = {};
	::StringCbPrintfA(objectAppNameSplit, sizeof(objectAppNameSplit), "%d", m_splitObjectIndex);
	MY_TRACE_STR(objectAppNameSplit);

	::GetPrivateProfileSectionA(m_objectAppName, m_sectionBuffer.data(), (DWORD)m_sectionBuffer.size(), m_tempFileName);
	if (::GetLastError() == 0)
		::WritePrivateProfileSectionA(objectAppNameSplit, m_sectionBuffer.data(), m_tempFileNameSplit);

	for (int i = 0; i < ExEdit::Object::MAX_FILTER; i++)
	{
		char filterAppName[MAX_PATH] = {};
		::StringCbPrintfA(filterAppName, sizeof(filterAppName), "%d.%d", m_sortedObjectIndex, i);
//		MY_TRACE_STR(filterAppName);

		char filterAppNameSplit[MAX_PATH] = {};
		::StringCbPrintfA(filterAppNameSplit, sizeof(filterAppNameSplit), "%d.%d", m_splitObjectIndex, i);
//		MY_TRACE_STR(filterAppNameSplit);

		::GetPrivateProfileSectionA(filterAppName, m_sectionBuffer.data(), (DWORD)m_sectionBuffer.size(), m_tempFileName);
		if (::GetLastError() == 0)
			::WritePrivateProfileSectionA(filterAppNameSplit, m_sectionBuffer.data(), m_tempFileNameSplit);
	}

	m_splitObjectIndex++;

	return TRUE;
}

//--------------------------------------------------------------------

BOOL TextSplitter::splitText()
{
	if (!getTextObjectInfo())
		return FALSE;

	// セクションをコピーするのに使うバッファ。
	m_sectionBuffer.resize(70000, '\0');

	// [exedit] をコピーする。
	::GetPrivateProfileSectionA("exedit", m_sectionBuffer.data(), (DWORD)m_sectionBuffer.size(), m_tempFileName);
	if (::GetLastError() == 0)
		::WritePrivateProfileSectionA("exedit", m_sectionBuffer.data(), m_tempFileNameSplit);

	// 描画フィルタのセクション名を作成する。
	::StringCbPrintfA(m_drawFilterAppName, sizeof(m_drawFilterAppName), "%d.%d", m_sortedObjectIndex, m_drawFilterIndex);
	MY_TRACE_STR(m_drawFilterAppName);

	getText();

	// 文字列矩形を取得するための HDC を用意する。
	ClientDC dc(m_fp->hwnd);
	GdiObj font = ::CreateFontA(-m_size, 0, 0, 0,
		m_bold ? FW_BOLD : FW_NORMAL, m_italic, FALSE, FALSE,
		DEFAULT_CHARSET, 0, 0, 0, 0, m_fontName);
	GdiObjSelector fontSelector(dc, font);

	::SetTextCharacterExtra(dc, m_spacing_x);

	getBaseSizeInfo(dc);

	m_lineIndex = 0;
	m_splitObjectIndex = 0;

	std::wstringstream ss(m_text);
	std::wstring line;
	while (std::getline(ss, line, L'\n'))
	{
		line = getLine(line);

		MY_TRACE_WSTR(line.c_str());

		// y を算出する。

		double y[2] = {};

		for (int i = 0; i < m_oy.getCount(); i++)
		{
			y[i] = m_oy.m_pos[i] + (m_baseSize.m_height + m_spacing_y) * m_lineIndex; // [上]

			switch (m_align)
			{
			case 3:
			case 4:
			case 5:
				{
					// [中]

					y[i] -= m_wholeCenterY;
					y[i] += m_baseSize.m_centerY;

					break;
				}
			case 6:
			case 7:
			case 8:
				{
					// [下]

					y[i] -= m_wholeHeight;
					y[i] += m_baseSize.m_height;

					break;
				}
			}
		}

		if (m_fp->check[Check::SplitToRow])
			splitToRow(dc, line, y);
		else
			splitToChar(dc, line, y);

		m_lineIndex++;
	}

	if (m_fp->check[Check::DeleteOriginal])
	{
		// 元のオブジェクトを削除する。
		::SendMessage(g_auin.GetExEditWindow(), WM_COMMAND, 0x3E9, 0);
	}

	g_auin.LoadExo(m_tempFileNameSplit, 0, 0, m_fp, m_editp);

	::DeleteFileA(m_tempFileName);
	::DeleteFileA(m_tempFileNameSplit);

	// AviUtl のプレビューウィンドウを再描画する。
	::PostMessage(m_fp->hwnd, AviUtl::FilterPlugin::WindowMessage::Command, 0, 0);

	return TRUE;
}

//--------------------------------------------------------------------
