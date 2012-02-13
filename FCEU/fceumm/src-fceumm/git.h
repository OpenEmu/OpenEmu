#ifndef __FCEU_GIT
#define __FCEU_GIT
/* Mmm...git. Almost as funny as "gimp". */
#define GIT_CART  0  /* Cart. */
#define GIT_VSUNI       1  /* VS Unisystem. */
#define GIT_FDS   2  /* Famicom Disk System. */

#define GIV_NTSC  0  /* NTSC emulation. */
#define GIV_PAL    1  /* PAL emulation. */
#define GIV_USER  2  /* What was set by FCEUI_SetVidSys(). */

typedef struct {
  uint8 *name;  /* Game name, UTF8 encoding */

  int type;       /* GIT_* */
  int vidsys;     /* Current emulated video system; GIV_* */
  int input[2];   /* Desired input for emulated input ports 1 and 2; -1
         for unknown desired input. */
  int inputfc;  /* Desired Famicom expansion port device. -1 for unknown
         desired input. */
  int cspecial;  /* Special cart expansion: DIP switches, barcode
         reader, etc.
      */
  uint8 MD5[16];
  int soundrate;  /* For Ogg Vorbis expansion sound wacky support.  0 for default. */
  int soundchan;  /* Number of sound channels. */
} FCEUGI;
#endif
