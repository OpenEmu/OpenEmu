#ifndef SCREEN_H_
#define SCREEN_H_

#include <stdio.h>

#include "atari.h"  /* UBYTE */

#ifdef DIRTYRECT
#ifndef CLIENTUPDATE
extern UBYTE *Screen_dirty;
#endif /* CLIENTUPDATE */
#endif /* DIRTYRECT */

extern ULONG *Screen_atari;

/* Dimensions of Screen_atari.
   Screen_atari is Screen_WIDTH * Screen_HEIGHT bytes.
   Each byte is an Atari color code - use Colours_Get[RGB] functions
   to get actual RGB codes.
   You should never display anything outside the middle 336 columns. */
#define Screen_WIDTH  384
#define Screen_HEIGHT 240

#ifdef BITPL_SCR
extern ULONG *Screen_atari_b;
extern ULONG *Screen_atari1;
extern ULONG *Screen_atari2;
#endif

/* The area that can been seen is Screen_visible_x1 <= x < Screen_visible_x2,
   Screen_visible_y1 <= y < Screen_visible_y2.
   Full Atari screen is 336x240. Screen_WIDTH is 384 only because
   the code in antic.c sometimes draws more than 336 bytes in a line.
   Currently Screen_visible variables are used only to place
   disk led and snailmeter in the corners of the screen.
*/
extern int Screen_visible_x1;
extern int Screen_visible_y1;
extern int Screen_visible_x2;
extern int Screen_visible_y2;

extern int Screen_show_atari_speed;
extern int Screen_show_disk_led;
extern int Screen_show_sector_counter;
extern int Screen_show_1200_leds;

int Screen_Initialise(int *argc, char *argv[]);
int Screen_ReadConfig(char *string, char *ptr);
void Screen_WriteConfig(FILE *fp);
void Screen_DrawAtariSpeed(double);
void Screen_DrawDiskLED(void);
void Screen_Draw1200LED(void);
void Screen_FindScreenshotFilename(char *buffer, unsigned bufsize);
int Screen_SaveScreenshot(const char *filename, int interlaced);
void Screen_SaveNextScreenshot(int interlaced);
void Screen_EntireDirty(void);

#endif /* SCREEN_H_ */
