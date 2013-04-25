#ifndef XEP80_FONTS_H_
#define XEP80_FONTS_H_

#include "config.h"
#include "atari.h"
#include "xep80.h"

#define XEP80_FONTS_NUM_FONT_SETS   3
#define XEP80_FONTS_NUM_FONTS       8
#define XEP80_FONTS_CHAR_COUNT    256

#define XEP80_FONTS_REV_FONT_BIT        0x1
#define XEP80_FONTS_UNDER_FONT_BIT      0x2
#define XEP80_FONTS_BLK_FONT_BIT        0x4

#define XEP80_FONTS_UNDER_ROW     9

extern UBYTE XEP80_FONTS_atari_fonts[XEP80_FONTS_NUM_FONT_SETS][XEP80_FONTS_NUM_FONTS][XEP80_FONTS_CHAR_COUNT][XEP80_MAX_CHAR_HEIGHT][XEP80_CHAR_WIDTH];
extern UBYTE XEP80_FONTS_oncolor;
extern UBYTE XEP80_FONTS_offcolor;

extern int XEP80_FONTS_inited;

int XEP80_FONTS_InitFonts(char const *charset_filename);

#endif /* XEP80_FONTS_H_ */
