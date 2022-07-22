#pragma once

//--------------------------------------------------------------------

struct TextSize // 改行を含まない文字列のサイズを保持する。
{
	int m_width = 0;
	int m_height = 0;
	int m_centerX = 0;
	int m_centerY = 0;

	TextSize()
	{
	}

	TextSize(HDC dc, LPCWSTR _text, int c, int spacing_x, int spacing_y, BOOL overhang = FALSE)
	{
		init(dc, _text, c, spacing_x, spacing_y, overhang);
	}

	BOOL init(HDC dc, LPCWSTR _text, int c, int spacing_x, int spacing_y, BOOL overhang = FALSE)
	{
		std::wstring text(_text, c);

		if (text.empty()) return FALSE;
		if (text.back() == L'\r') text.pop_back();
		if (text.empty()) return FALSE;

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

		return TRUE;
	}
};

//--------------------------------------------------------------------

class TextSplitter
{
private:

	AviUtl::EditHandle* m_editp = 0;
	AviUtl::FilterPlugin* m_fp = 0;

	char m_tempFileName[MAX_PATH] = {};
	char m_tempFileNameSplit[MAX_PATH] = {};

	int m_sortedObjectIndex = -1;

	char m_objectAppName[MAX_PATH] = {};
	char m_firstFilterAppName[MAX_PATH] = {};

	int m_layer = 0;
	int m_start = 0;
	int m_end = 0;

	int m_size = 0;
	BOOL m_bold = FALSE;
	BOOL m_italic = FALSE;
	int m_align = 0;
	int m_spacing_x = 0;
	int m_spacing_y = 0;
	char m_fontName[MAX_PATH] = {};

	int m_ox = 0;
	int m_oy = 0;
	int m_drawFilterIndex = -1;

	std::vector<char> m_sectionBuffer;
	char m_drawFilterAppName[MAX_PATH] = {};

	std::wstring m_text;
	TextSize m_baseSize;
	int m_splitObjectCount = 0;
	int m_wholeWidth = 0;
	int m_wholeHeight = 0;
	int m_wholeCenterX = 0;
	int m_wholeCenterY = 0;

	int m_lineIndex = 0;
	int m_splitObjectIndex = 0;

public:

	TextSplitter(AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp);

	BOOL splitText();
	BOOL playVoice(int voice);
	BOOL getTempFileName();
	BOOL getTextObjectInfo();
	BOOL getText();
	BOOL getBaseSizeInfo(HDC dc);
	BOOL splitToChar(HDC dc, const std::wstring& line, int y);
	BOOL splitToRow(HDC dc, const std::wstring& line, int y);
};

//--------------------------------------------------------------------
