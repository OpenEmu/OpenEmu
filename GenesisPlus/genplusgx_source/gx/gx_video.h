/****************************************************************************
 *  gx_video.c
 *
 *  Genesis Plus GX video support
 *
 *  Softdev (2006)
 *  Eke-Eke (2007,2008,2009)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ***************************************************************************/

#ifndef _GC_VIDEO_H_
#define _GC_VIDEO_H_

/* EFB colors */
#define BLACK       {0x00,0x00,0x00,0xff}
#define DARK_GREY   {0x22,0x22,0x22,0xff}
#define LIGHT_BLUE  {0xb8,0xc7,0xda,0xff}
#define SKY_BLUE    {0x99,0xcc,0xff,0xff}
#define LIGHT_GREEN {0xa9,0xc7,0xc6,0xff}
#define WHITE       {0xff,0xff,0xff,0xff}

/* image texture */
typedef struct
{
  u8 *data;
  u16 width;
  u16 height;
  u8 format;
} gx_texture;

/* Global variables */
extern GXRModeObj *vmode;
extern u8 *texturemem;
extern u32 gc_pal;


/* GX rendering */
extern void gxDrawRectangle(s32 x, s32 y, s32 w, s32 h, u8 alpha, GXColor color);
extern void gxDrawTexture(gx_texture *texture, s32 x, s32 y, s32 w, s32 h, u8 alpha);
extern void gxDrawTextureRepeat(gx_texture *texture, s32 x, s32 y, s32 w, s32 h, u8 alpha);
extern void gxDrawTextureRotate(gx_texture *texture, s32 x, s32 y, s32 w, s32 h, f32 angle, u8 alpha);
extern void gxDrawScreenshot(u8 alpha);
extern void gxCopyScreenshot(gx_texture *texture);
extern void gxSaveScreenshot(char *filename);
extern void gxClearScreen(GXColor color);
extern void gxSetScreen(void);

/* PNG textures */
extern gx_texture *gxTextureOpenPNG(const u8 *png_data, FILE *png_file);
extern void gxTextureWritePNG(gx_texture *p_texture, FILE *png_file);
extern void gxTextureClose(gx_texture **p_texture);

/* GX video engine */
extern void gx_video_Init(void);
extern void gx_video_Shutdown(void);
extern void gx_video_Start(void);
extern void gx_video_Stop(void);
extern void gx_video_Update(void);

#endif
