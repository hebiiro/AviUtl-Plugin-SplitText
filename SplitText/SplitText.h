#pragma once

//--------------------------------------------------------------------

const int32_t TRACK_VOICE = 0;
const int32_t CHECK_SPLIT_TEXT = 0;

//--------------------------------------------------------------------

extern AviUtlInternal g_auin;

//--------------------------------------------------------------------

BOOL onCommand(int commandIndex, AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp);
BOOL onSplitText(AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp);

//--------------------------------------------------------------------
