﻿#pragma once

//--------------------------------------------------------------------

struct Track
{
	static const int32_t Voice = 0;
	static const int32_t Frame = 1;
	static const int32_t Justification = 2;
};

struct Check
{
	static const int32_t SplitText = 0;
	static const int32_t AbsoluteFrameMode = 1;
	static const int32_t SplitToRow = 2;
	static const int32_t Justification = 3;
	static const int32_t DeleteOriginal = 4;
};

//--------------------------------------------------------------------

extern AviUtlInternal g_auin;

//--------------------------------------------------------------------

BOOL onCommand(int commandIndex, AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp);
BOOL onSplitText(AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp);

//--------------------------------------------------------------------
