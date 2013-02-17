/****************************************************************************
 *  gx_video.c
 *
 *  Genesis Plus GX video support
 *
 *  Copyright Eke-Eke (2007-2012), based on original work from Softdev (2006)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************/

#ifndef _GC_VIDEO_H_
#define _GC_VIDEO_H_

/* EFB colors */
#define BLACK       {0x00,0x00,0x00,0xff}
#define DARK_GREY   {0x22,0x22,0x22,0xff}
#define LIGHT_BLUE  {0xb8,0xc7,0xda,0xff}
#define SKY_BLUE    {0x99,0xcc,0xff,0xff}
#define LIGHT_GREEN {0xa9,0xc7,0xc6,0xff}
#define WHITE       {0xff,0xff,0xff,0xff}

/* Directly fill a RGB565 texture */
/* One tile is 32 byte = 4x4 pixels */
/* Tiles are stored continuously in texture memory */
#define CUSTOM_BLITTER(line, width, table, in)  \
  width >>= 2;  \
  u16 *out = (u16 *) (texturemem + (((width << 5) * (line >> 2)) + ((line & 3) << 3))); \
  do  \
  { \
    *out++ = table[*in++];  \
    *out++ = table[*in++];  \
    *out++ = table[*in++];  \
    *out++ = table[*in++];  \
    out += 12;  \
  } \
  while (--width);

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
extern int gx_video_Update(void);

#endif
