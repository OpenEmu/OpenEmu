#ifndef XEP80_H_
#define XEP80_H_

#include "config.h"
#include "atari.h"

#define XEP80_WIDTH 256
#define XEP80_HEIGHT 25
#define XEP80_CHAR_WIDTH 7
#define XEP80_MAX_CHAR_HEIGHT 12
#define XEP80_GRAPH_WIDTH 320
#define XEP80_GRAPH_HEIGHT 200
#define XEP80_LINE_LEN 80
#define XEP80_SCRN_WIDTH (XEP80_LINE_LEN * XEP80_CHAR_WIDTH)
#define XEP80_MAX_SCRN_HEIGHT (XEP80_HEIGHT * XEP80_MAX_CHAR_HEIGHT)

/* Current height of the XEP80 screen */
extern int XEP80_scrn_height;
/* Current height of XEP80 characters. */
extern int XEP80_char_height;

/* XEP80 hardware is TV system-independent - it can switch to NTSC or PAL
   (or "60Hz" and "50Hz" modes, as they're called in the docs). In each mode
   number of lines per each char varies as defined below.
   NTSC screenshots:
     http://www.atariage.com/forums/topic/108601-what-does-xep80-display-look-like/page__p__1314304#entry1314304
     http://www.atariage.com/forums/topic/171976-xep80-looks-like-this-on-real-hardware/page__p__2130114#entry2130114
     http://www.atariage.com/forums/topic/124642-atari-8-bit-sc1224-bw-greyscale-monitor-interface/page__view__findpost__p__1508084
   PAL screenshots:
     http://www.atariage.com/forums/topic/134208-atari-8-bit-irc-client-fujichat-03-released/page__p__1617958#entry1617958
     http://www.atari.org.pl/forum/viewtopic.php?pid=142647#p142647 (sign in to see) */
enum {
	XEP80_CHAR_HEIGHT_NTSC = 10,
	XEP80_CHAR_HEIGHT_PAL = 12
};

#define XEP80_ATARI_EOL			0x9b

/* Is XEP80 enabled? Don't change directly, use XEP80_SetEnabled(). */
extern int XEP80_enabled;
int XEP80_SetEnabled(int value);
extern int XEP80_port;

extern UBYTE XEP80_screen_1[XEP80_SCRN_WIDTH*XEP80_MAX_SCRN_HEIGHT];
extern UBYTE XEP80_screen_2[XEP80_SCRN_WIDTH*XEP80_MAX_SCRN_HEIGHT];

UBYTE XEP80_GetBit(void);
void XEP80_PutBit(UBYTE byte);
void XEP80_ChangeColors(void);
void XEP80_StateSave(void);
void XEP80_StateRead(void);
int XEP80_ReadConfig(char *string, char *ptr);
void XEP80_WriteConfig(FILE *fp);
int XEP80_Initialise(int *argc, char *argv[]);

#endif /* XEP80_H_ */
