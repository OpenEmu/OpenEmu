#ifndef __MDFN_VIDEO_TEXT_H
#define __MDFN_VIDEO_TEXT_H

enum
{
 // If the order of these constants is changed, you must also update the array of FontDescriptor
 // in text.cpp.
 MDFN_FONT_9x18_18x18 = 0,
 MDFN_FONT_5x7,
 MDFN_FONT_4x5,
 MDFN_FONT_6x13_12x13,

 #ifdef WANT_INTERNAL_CJK
 MDFN_FONT_12x13,
 MDFN_FONT_18x18,
 #endif

 _MDFN_FONT_COUNT
};

uint32 GetTextPixLength(const UTF8 *msg, uint32 which_font = MDFN_FONT_9x18_18x18);
uint32 GetTextPixLength(const UTF32 *msg, uint32 which_font = MDFN_FONT_9x18_18x18);

uint32 DrawTextTrans(uint32 *dest, int pitch, uint32 width, const UTF8 *textmsg, uint32 fgcolor, int centered, uint32 which_font = MDFN_FONT_9x18_18x18);
uint32 DrawTextTrans(uint32 *dest, int pitch, uint32 width, const UTF32 *textmsg, uint32 fgcolor, int centered, uint32 which_font = MDFN_FONT_9x18_18x18);


uint32 DrawTextTransShadow(uint32 *dest, int pitch, uint32 width, const UTF8 *textmsg, uint32 fgcolor, uint32 shadcolor,int centered, uint32 which_font = MDFN_FONT_9x18_18x18);
uint32 DrawTextTransShadow(uint32 *dest, int pitch, uint32 width, const std::string &textmsg, uint32 fgcolor, uint32 shadcolor,int centered, uint32 which_font = MDFN_FONT_9x18_18x18);

#endif
