#pragma once

//--------------------------------------------------------------------

struct Track
{
	static const int32_t Voice = 0;
	static const int32_t Frame = 1;
};

struct Check
{
	static const int32_t SplitText = 0;
	static const int32_t AbsoluteFrameMode = 1;
};

//--------------------------------------------------------------------

extern AviUtlInternal g_auin;

//--------------------------------------------------------------------

BOOL onCommand(int commandIndex, AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp);
BOOL onSplitText(AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp);

//--------------------------------------------------------------------
