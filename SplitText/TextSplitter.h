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

	struct PosMode
	{
		static const int Solo = 0;
		static const int Pair = 1;
		static const int Animation = 2;
	};

	struct Pos
	{
		static const int Begin = 0;
		static const int End = 1;

		int m_mode;
		double m_pos[2];
		char m_animation[MAX_PATH];

		int getCount()
		{
			switch (m_mode)
			{
			case PosMode::Solo: return 1;
			default: return 2;
			}
		}
	};

	Pos m_ox = {};
	Pos m_oy = {};
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
	int m_justificationWidth = 0;
	double m_justificationX[2] = {};

	int m_lineIndex = 0;
	int m_splitObjectIndex = 0;

public:

	TextSplitter(AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp);

	BOOL splitText();
	BOOL playVoice(int voice);
	BOOL getTempFileName();
	BOOL getTextObjectInfo();
	BOOL getPos(Pos* pos, LPCSTR appName, LPCSTR label);
	BOOL getText();
	static std::wstring getLine(const std::wstring& _line);
	BOOL getBaseSizeInfo(HDC dc);
	BOOL writeChar(LPCWSTR text, double x[2], double y[2]);
	BOOL writePos(Pos* pos, LPCSTR label, double value[2]);
	BOOL splitToChar(HDC dc, const std::wstring& line, double y[2]);
	BOOL splitToRow(HDC dc, const std::wstring& line, double y[2]);
};

//--------------------------------------------------------------------
