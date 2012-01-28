/****************************************************************************
 *  gx_video.c
 *
 *  Genesis Plus GX video & rendering support
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

#include "shared.h"
#include "font.h"
#include "aram.h"
#include "md_ntsc.h"
#include "sms_ntsc.h"

#include <png.h>

#define TEX_WIDTH         720
#define TEX_HEIGHT        576
#define TEX_SIZE          (TEX_WIDTH * TEX_HEIGHT * 2)
#define DEFAULT_FIFO_SIZE 256 * 1024
#define HASPECT           320
#define VASPECT           240

/* libpng wrapper */
typedef struct
{
  u8 *buffer;
  u32 offset;
} png_image;

extern const u8 Crosshair_p1_png[];
extern const u8 Crosshair_p2_png[];

/*** VI ***/
GXRModeObj *vmode;  /* Default Video Mode    */
u8 *texturemem;     /* Texture Data          */
u8 *screenshot;     /* Texture Data          */

/*** 50/60hz flag ***/
u32 gc_pal = 0;

/*** NTSC Filters ***/
sms_ntsc_t *sms_ntsc;
md_ntsc_t *md_ntsc;

/*** GX FIFO ***/
static u8 gp_fifo[DEFAULT_FIFO_SIZE] ATTRIBUTE_ALIGN (32);

/*** GX Textures ***/
static u32 vwidth,vheight;
static gx_texture *crosshair[2];

/*** Framebuffers ***/
static u32 *xfb[2];
static u32 whichfb = 0;

/***************************************************************************************/
/*   Emulation video modes                                                             */
/***************************************************************************************/
static GXRModeObj *rmode;

/* 288 lines progressive (PAL 50Hz) */
static GXRModeObj TV50hz_288p = 
{
  VI_TVMODE_PAL_DS,             // viDisplayMode
  640,                          // fbWidth
  286,                          // efbHeight
  286,                          // xfbHeight
  0,                            // viXOrigin
  (VI_MAX_HEIGHT_PAL - 572)/2,  // viYOrigin
  VI_MAX_WIDTH_PAL,             // viWidth
  572,                          // viHeight
  VI_XFBMODE_SF,                // xFBmode
  GX_FALSE,                     // field_rendering
  GX_FALSE,                     // aa

  // sample points arranged in increasing Y order
  {
    {6,6},{6,6},{6,6},  // pix 0, 3 sample points, 1/12 units, 4 bits each
    {6,6},{6,6},{6,6},  // pix 1
    {6,6},{6,6},{6,6},  // pix 2
    {6,6},{6,6},{6,6}   // pix 3
  },

  // vertical filter[7], 1/64 units, 6 bits each
  {
    0,         // line n-1
    0,         // line n-1
    21,        // line n
    22,        // line n
    21,        // line n
    0,         // line n+1
    0          // line n+1
  }
};

/* 288 lines interlaced (PAL 50Hz) */
static GXRModeObj TV50hz_288i = 
{
  VI_TVMODE_PAL_INT,            // viDisplayMode
  640,                          // fbWidth
  286,                          // efbHeight
  286,                          // xfbHeight
  0,                            // viXOrigin
  (VI_MAX_HEIGHT_PAL - 572)/2,  // viYOrigin
  VI_MAX_WIDTH_PAL,             // viWidth
  572,                          // viHeight
  VI_XFBMODE_SF,                // xFBmode
  GX_TRUE,                      // field_rendering
  GX_FALSE,                     // aa

  // sample points arranged in increasing Y order
  {
    {6,6},{6,6},{6,6},  // pix 0, 3 sample points, 1/12 units, 4 bits each
    {6,6},{6,6},{6,6},  // pix 1
    {6,6},{6,6},{6,6},  // pix 2
    {6,6},{6,6},{6,6}   // pix 3
  },

  // vertical filter[7], 1/64 units, 6 bits each
  {
    0,         // line n-1
    0,         // line n-1
    21,        // line n
    22,        // line n
    21,        // line n
    0,         // line n+1
    0          // line n+1
  }
};

/* 576 lines interlaced (PAL 50Hz, scaled) */
static GXRModeObj TV50hz_576i = 
{
  VI_TVMODE_PAL_INT,  // viDisplayMode
  640,                // fbWidth
  480,                // efbHeight
  VI_MAX_HEIGHT_PAL,  // xfbHeight
  0,                  // viXOrigin
  0,                  // viYOrigin
  VI_MAX_WIDTH_PAL,   // viWidth
  VI_MAX_HEIGHT_PAL,  // viHeight
  VI_XFBMODE_DF,      // xFBmode
  GX_FALSE,           // field_rendering
  GX_FALSE,           // aa

  // sample points arranged in increasing Y order
  {
    {6,6},{6,6},{6,6},  // pix 0, 3 sample points, 1/12 units, 4 bits each
    {6,6},{6,6},{6,6},  // pix 1
    {6,6},{6,6},{6,6},  // pix 2
    {6,6},{6,6},{6,6}   // pix 3
  },

  // vertical filter[7], 1/64 units, 6 bits each
  {
    8,         // line n-1
    8,         // line n-1
    10,        // line n
    12,        // line n
    10,        // line n
    8,         // line n+1
    8          // line n+1
  }
};

/* 240 lines progressive (NTSC or PAL 60Hz) */
static GXRModeObj TV60hz_240p = 
{
  VI_TVMODE_EURGB60_DS, // viDisplayMode
  640,                  // fbWidth
  VI_MAX_HEIGHT_NTSC/2, // efbHeight
  VI_MAX_HEIGHT_NTSC/2, // xfbHeight
  0,                    // viXOrigin
  0,                    // viYOrigin
  VI_MAX_WIDTH_NTSC,    // viWidth
  VI_MAX_HEIGHT_NTSC,   // viHeight
  VI_XFBMODE_SF,        // xFBmode
  GX_FALSE,             // field_rendering
  GX_FALSE,             // aa

  // sample points arranged in increasing Y order
  {
    {6,6},{6,6},{6,6},  // pix 0, 3 sample points, 1/12 units, 4 bits each
    {6,6},{6,6},{6,6},  // pix 1
    {6,6},{6,6},{6,6},  // pix 2
    {6,6},{6,6},{6,6}   // pix 3
  },

  // vertical filter[7], 1/64 units, 6 bits each
  {
    0,         // line n-1
    0,         // line n-1
    21,        // line n
    22,        // line n
    21,        // line n
    0,         // line n+1
    0          // line n+1
  }
};

/* 240 lines interlaced (NTSC or PAL 60Hz) */
static GXRModeObj TV60hz_240i = 
{
    VI_TVMODE_EURGB60_INT,  // viDisplayMode
    640,                    // fbWidth
    VI_MAX_HEIGHT_NTSC/2,   // efbHeight
    VI_MAX_HEIGHT_NTSC/2,   // xfbHeight
    0,                      // viXOrigin
    0,                      // viYOrigin
    VI_MAX_WIDTH_NTSC,      // viWidth
    VI_MAX_HEIGHT_NTSC,     // viHeight
    VI_XFBMODE_SF,          // xFBmode
    GX_TRUE,                // field_rendering
    GX_FALSE,               // aa

  // sample points arranged in increasing Y order
  {
    {3,2},{9,6},{3,10},  // pix 0, 3 sample points, 1/12 units, 4 bits each
    {3,2},{9,6},{3,10},  // pix 1
    {9,2},{3,6},{9,10},  // pix 2
    {9,2},{3,6},{9,10}   // pix 3
  },

  // vertical filter[7], 1/64 units, 6 bits each
  {
    0,         // line n-1
    0,         // line n-1
    21,        // line n
    22,        // line n
    21,        // line n
    0,         // line n+1
    0          // line n+1
  }
};

/* 480 lines interlaced (NTSC or PAL 60Hz) */
static GXRModeObj TV60hz_480i = 
{
  VI_TVMODE_EURGB60_INT,// viDisplayMode
  640,                  // fbWidth
  VI_MAX_HEIGHT_NTSC,   // efbHeight
  VI_MAX_HEIGHT_NTSC,   // xfbHeight
  0,                    // viXOrigin
  0,                    // viYOrigin
  VI_MAX_WIDTH_NTSC,    // viWidth
  VI_MAX_HEIGHT_NTSC,   // viHeight
  VI_XFBMODE_DF,        // xFBmode
  GX_FALSE,             // field_rendering
  GX_FALSE,             // aa

  // sample points arranged in increasing Y order
  {
    {6,6},{6,6},{6,6},  // pix 0, 3 sample points, 1/12 units, 4 bits each
    {6,6},{6,6},{6,6},  // pix 1
    {6,6},{6,6},{6,6},  // pix 2
    {6,6},{6,6},{6,6}   // pix 3
  },

  // vertical filter[7], 1/64 units, 6 bits each
  {
    8,         // line n-1
    8,         // line n-1
    10,        // line n
    12,        // line n
    10,        // line n
    8,         // line n+1
    8          // line n+1
  }
};

/* TV modes pointer table */
static GXRModeObj *tvmodes[6] =
{
   /* 60hz modes */
   &TV60hz_240p,
   &TV60hz_240i,
   &TV60hz_480i,

   /* 50Hz modes */
   &TV50hz_288p,
   &TV50hz_288i,
   &TV50hz_576i   
};


/***************************************************************************************/
/*   GX rendering engine                                                               */
/***************************************************************************************/

typedef struct tagcamera
{
  guVector pos;
  guVector up;
  guVector view;
} camera;

/*** Square Matrix
     This structure controls the size of the image on the screen.
   Think of the output as a -80 x 80 by -60 x 60 graph.
***/
static s16 square[] ATTRIBUTE_ALIGN (32) =
{
  /*
   * X,   Y,  Z
   * Values set are for roughly 4:3 aspect
   */
  -HASPECT,  VASPECT, 0,  // 0
   HASPECT,  VASPECT, 0,  // 1
   HASPECT, -VASPECT, 0,  // 2
  -HASPECT, -VASPECT, 0,  // 3
};

static camera cam = {
  {0.0F, 0.0F, -100.0F},
  {0.0F, -1.0F, 0.0F},
  {0.0F, 0.0F, 0.0F}
};

/* VSYNC callback */
static void vi_callback(u32 cnt)
{
  frameticker++;
}

/* Vertex Rendering */
static inline void draw_vert(u8 pos, f32 s, f32 t)
{
  GX_Position1x8(pos);
  GX_TexCoord2f32(s, t);
}

/* textured quad rendering */
static inline void draw_square(void)
{
  GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
  draw_vert(3, 0.0, 0.0);
  draw_vert(2, 1.0, 0.0);
  draw_vert(1, 1.0, 1.0);
  draw_vert(0, 0.0, 1.0);
  GX_End ();
}

/* Initialize GX */
static void gxStart(void)
{
  /*** Clear out FIFO area ***/
  memset(&gp_fifo, 0, DEFAULT_FIFO_SIZE);

  /*** GX default ***/
  GX_Init(&gp_fifo, DEFAULT_FIFO_SIZE);
  GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
  GX_SetCullMode(GX_CULL_NONE);
  GX_SetClipMode(GX_CLIP_DISABLE);
  GX_SetDispCopyGamma(GX_GM_1_0);
  GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
  GX_SetColorUpdate(GX_TRUE);
  GX_SetAlphaUpdate(GX_FALSE);

  /* Modelview */
  Mtx view;
  memset (&view, 0, sizeof (Mtx));
  guLookAt(view, &cam.pos, &cam.up, &cam.view);
  GX_LoadPosMtxImm(view, GX_PNMTX0);
  GX_Flush();
}

/* Reset GX rendering */
static void gxResetRendering(u8 type)
{
  GX_ClearVtxDesc();

  if (type)
  {
    /* uses direct positionning, alpha blending & color channel (menu rendering) */
    GX_SetBlendMode(GX_BM_BLEND,GX_BL_SRCALPHA,GX_BL_INVSRCALPHA,GX_LO_CLEAR);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_S16, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GX_SetVtxDesc (GX_VA_CLR0, GX_DIRECT);
    /* 
       Color.out = Color.rasterized*Color.texture
       Alpha.out = Alpha.rasterized*Alpha.texture 
    */
    GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GX_SetNumTexGens(1);
    GX_SetNumChans(1);
  }
  else
  {
    /* uses array positionning, no alpha blending, no color channel (video emulation) */
    GX_SetBlendMode(GX_BM_NONE,GX_BL_SRCALPHA,GX_BL_INVSRCALPHA,GX_LO_CLEAR);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GX_SetVtxDesc(GX_VA_POS, GX_INDEX8);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GX_SetArray(GX_VA_POS, square, 3 * sizeof (s16));
    /* 
       Color.out = Color.texture
       Alpha.out = Alpha.texture 
    */
    GX_SetTevOp (GX_TEVSTAGE0, GX_REPLACE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
    GX_SetNumTexGens(1);
    GX_SetNumChans(0);
  }

  GX_Flush();
}

/* Reset GX rendering mode */
static void gxResetMode(GXRModeObj *tvmode)
{
  Mtx44 p;
  f32 yScale = GX_GetYScaleFactor(tvmode->efbHeight, tvmode->xfbHeight);
  u16 xfbHeight = GX_SetDispCopyYScale(yScale);
  u16 xfbWidth  = tvmode->fbWidth;  
  if (xfbWidth & 15)  // xfb width is 16 pixels aligned
    xfbWidth = (xfbWidth & ~15) + 16;

  GX_SetCopyClear((GXColor)BLACK,0x00ffffff);
  GX_SetViewport(0.0F, 0.0F, tvmode->fbWidth, tvmode->efbHeight, 0.0F, 1.0F);
  GX_SetScissor(0, 0, tvmode->fbWidth, tvmode->efbHeight);
  GX_SetDispCopySrc(0, 0, tvmode->fbWidth, tvmode->efbHeight);
  GX_SetDispCopyDst(xfbWidth, xfbHeight);
  GX_SetCopyFilter(tvmode->aa, tvmode->sample_pattern, (tvmode->xfbMode == VI_XFBMODE_SF) ? GX_FALSE : GX_TRUE, tvmode->vfilter);
  GX_SetFieldMode(tvmode->field_rendering, ((tvmode->viHeight == 2 * tvmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
  guOrtho(p, tvmode->efbHeight/2, -(tvmode->efbHeight/2), -(tvmode->fbWidth/2), tvmode->fbWidth/2, 100, 1000);
  GX_LoadProjectionMtx(p, GX_ORTHOGRAPHIC);
  GX_Flush();
}

/* Manage Aspect Ratio */
static void gxSetAspectRatio(int *xscale, int *yscale)
{
  /* original aspect ratio */
  /* the following values have been deducted from comparison with a real 50/60hz Mega Drive */
  if (config.aspect)
  {
    /* vertical borders */
    if (config.overscan & 1)
    {
      /* Genesis outputs 288(PAL) or 243(NTSC) lines */
      /* Wii & Game Cube output 286/574(PAL50) or 240/480 (PAL60 & NTSC) lines  */
      *yscale = vdp_pal + ((gc_pal && !config.render) ? 143 : 120);
    }
    else
    {
      /* overscan is simulated (black) */
      *yscale = bitmap.viewport.h / 2;
      if (vdp_pal && (!gc_pal || config.render))
        *yscale = *yscale * 240 / 288;
      else if (!vdp_pal && gc_pal && !config.render)
        *yscale = *yscale * 288 / 240;
    }

    /* horizontal borders */
    if (config.overscan & 2)
    {
      /* max visible range is ~712 pixels, not 720 */
      *xscale = (reg[12] & 1) ? 356 : 360; 
    }
    else
    {
      /* overscan is simulated (black) */
      *xscale = 327;
    }

    /* 16/9 correction */
    if (config.aspect & 2)
    {
      *xscale = (*xscale * 3) / 4;
    }
  }

  /* manual aspect ratio (default is unscaled raw) */
  else
  {
    /* vertical borders */
    if (config.overscan & 1)
    {
      *yscale = (gc_pal && !config.render) ? (vdp_pal ? (268*144 / bitmap.viewport.h):143) : (vdp_pal ? (224*144 / bitmap.viewport.h):120);
    }
    else
    {
      *yscale = (gc_pal && !config.render) ? 134 : 112;
    }

    /* horizontal borders */
    if (config.overscan & 2)
    {
      *xscale = 348;
    }
    else
    {
      *xscale = 320;
    }

    /* add user scaling */
    *xscale += config.xscale;
    *yscale += config.yscale;
  }
}

/* Reset GX/VI hardware scaler */
static void gxResetScaler(u32 width)
{
  int xscale  = 0;
  int yscale  = 0;
  int offset  = 0;

  /* retrieve screen aspect ratio */
  gxSetAspectRatio(&xscale, &yscale);

  /* default EFB width */
  rmode->fbWidth = 640;

  /* no filtering, disable GX horizontal scaling */
  if (!config.bilinear && !config.ntsc)
  {
    if ((width * 2) <= 640)
      rmode->fbWidth = width * 2;
    else if (width <= 640)
      rmode->fbWidth = width;
  }

  /* configure VI width */
  if ((xscale * 2) > rmode->fbWidth)
  {
    /* max width = 720 pixels */
    if (xscale > 360)
    {
      /* save offset for later */
      offset = ((xscale - 360) * rmode->fbWidth) / rmode->viWidth;

      /* maximal width */
      xscale = 360;
    }

    /* enable VI upscaling */
    rmode->viWidth = xscale * 2;
    rmode->viXOrigin = (720 - (xscale * 2)) / 2;

    /* default GX horizontal scaling */
    xscale = (rmode->fbWidth / 2);

    /* handle additional upscaling */
    if (offset)
    {
      /* no filtering, reduce EFB width to increase VI upscaling */
      if (!config.bilinear && !config.ntsc)
        rmode->fbWidth -= (offset * 2);
      
      /* increase GX horizontal scaling */
      else
        xscale += offset;
    }
  }
  else
  {
    /* VI horizontal scaling is disabled */
    rmode->viWidth = rmode->fbWidth;
    rmode->viXOrigin = (720 - rmode->fbWidth) / 2;
  }

  /* Adjust screen position */
  int xshift = (config.xshift * rmode->fbWidth) / rmode->viWidth;
  int yshift = (config.yshift * rmode->efbHeight) / rmode->viHeight;

  /* Double Resolution modes (480i/576i/480p) */
  if (config.render)
  {
    yscale = yscale * 2;
  }

  /* Set GX scaler (Vertex Position matrix) */
  square[6] = square[3]  = xshift + xscale;
  square[0] = square[9]  = xshift - xscale;
  square[4] = square[1]  = yshift + yscale;
  square[7] = square[10] = yshift - yscale;
  DCFlushRange(square, 32);
  GX_InvVtxCache();
}

static void gxDrawCrosshair(gx_texture *texture, int x, int y)
{
  if (texture->data)
  {
    /* load texture object */
    GXTexObj texObj;
    GX_InitTexObj(&texObj, texture->data, texture->width, texture->height, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GX_InitTexObjLOD(&texObj,GX_LINEAR,GX_LIN_MIP_LIN,0.0,10.0,0.0,GX_FALSE,GX_TRUE,GX_ANISO_4);
    GX_LoadTexObj(&texObj, GX_TEXMAP0);
    GX_InvalidateTexAll();

    /* reset GX rendering */
    gxResetRendering(1);

    /* adjust coordinates */
    int w = (texture->width * rmode->fbWidth) / (rmode->viWidth);
    int h = (texture->height * rmode->efbHeight) / (rmode->viHeight);
    x = ((x * rmode->fbWidth) / bitmap.viewport.w) - w/2 - (rmode->fbWidth/2);
    y = ((y * rmode->efbHeight) / bitmap.viewport.h) - h/2 - (rmode->efbHeight/2);

    /* Draw textured quad */
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    GX_Position2s16(x,y+h);
    GX_Color4u8(0xff,0xff,0xff,0xff);
    GX_TexCoord2f32(0.0, 1.0);
    GX_Position2s16(x+w,y+h);
    GX_Color4u8(0xff,0xff,0xff,0xff);
    GX_TexCoord2f32(1.0, 1.0);
    GX_Position2s16(x+w,y);
    GX_Color4u8(0xff,0xff,0xff,0xff);
    GX_TexCoord2f32(1.0, 0.0);
    GX_Position2s16(x,y);
    GX_Color4u8(0xff,0xff,0xff,0xff);
    GX_TexCoord2f32(0.0, 0.0);
    GX_End ();

    /* restore GX rendering */
    gxResetRendering(0);

    /* restore texture object */
    GXTexObj texobj;
    GX_InitTexObj(&texobj, texturemem, vwidth, vheight, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
    if (!config.bilinear) GX_InitTexObjLOD(&texobj,GX_NEAR,GX_NEAR_MIP_NEAR,0.0,10.0,0.0,GX_FALSE,GX_FALSE,GX_ANISO_1);
    GX_LoadTexObj(&texobj, GX_TEXMAP0);
    GX_InvalidateTexAll();
  }
}


void gxDrawRectangle(s32 x, s32 y, s32 w, s32 h, u8 alpha, GXColor color)
{
  /* GX only use Color channel for rendering */
  GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
  GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);
  GX_Flush();

  /* vertex coordinate */
  x -= (vmode->fbWidth/2);
  y -= (vmode->efbHeight/2);

  /* draw colored quad */
  GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
  GX_Position2s16(x,y+h);
  GX_Color4u8(color.r,color.g,color.b,alpha);
  GX_Position2s16(x+w,y+h);
  GX_Color4u8(color.r,color.g,color.b,alpha);
  GX_Position2s16(x+w,y);
  GX_Color4u8(color.r,color.g,color.b,alpha);
  GX_Position2s16(x,y);
  GX_Color4u8(color.r,color.g,color.b,alpha);
  GX_End ();
  GX_DrawDone();

  /* restore GX rendering */
  GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
  GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
  GX_Flush();
}

void gxDrawTexture(gx_texture *texture, s32 x, s32 y, s32 w, s32 h, u8 alpha)
{
  if (!texture) return;
  if (texture->data)
  {
    /* load texture object */
    GXTexObj texObj;
    GX_InitTexObj(&texObj, texture->data, texture->width, texture->height, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GX_InitTexObjLOD(&texObj,GX_LINEAR,GX_LIN_MIP_LIN,0.0,10.0,0.0,GX_FALSE,GX_TRUE,GX_ANISO_4); /* does this really change anything ? */
    GX_LoadTexObj(&texObj, GX_TEXMAP0);
    GX_InvalidateTexAll();

    /* vertex coordinate */
    x -= (vmode->fbWidth/2);
    y -= (vmode->efbHeight/2);

    /* draw textured quad */
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    GX_Position2s16(x,y+h);
    GX_Color4u8(0xff,0xff,0xff,alpha);
    GX_TexCoord2f32(0.0, 1.0);
    GX_Position2s16(x+w,y+h);
    GX_Color4u8(0xff,0xff,0xff,alpha);
    GX_TexCoord2f32(1.0, 1.0);
    GX_Position2s16(x+w,y);
    GX_Color4u8(0xff,0xff,0xff,alpha);
    GX_TexCoord2f32(1.0, 0.0);
    GX_Position2s16(x,y);
    GX_Color4u8(0xff,0xff,0xff,alpha);
    GX_TexCoord2f32(0.0, 0.0);
    GX_End ();
    GX_DrawDone();
  }
}

void gxDrawTextureRotate(gx_texture *texture, s32 x, s32 y, s32 w, s32 h, f32 angle, u8 alpha)
{
  if (!texture) return;
  if (texture->data)
  {
    /* load texture object */
    GXTexObj texObj;
    GX_InitTexObj(&texObj, texture->data, texture->width, texture->height, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GX_InitTexObjLOD(&texObj,GX_LINEAR,GX_LIN_MIP_LIN,0.0,10.0,0.0,GX_FALSE,GX_TRUE,GX_ANISO_4);
    GX_LoadTexObj(&texObj, GX_TEXMAP0);
    GX_InvalidateTexAll();

    /* vertex coordinate */
    x -= (vmode->fbWidth/2);
    y -= (vmode->efbHeight/2);

    /* Modelview rotation */
    Mtx m,mv;
    guVector axis = (guVector) {0,0,1};
    guLookAt(mv, &cam.pos, &cam.up, &cam.view);
    guMtxRotAxisDeg (m, &axis, angle);
    guMtxTransApply(m,m, x+w/2,y+h/2,0);
    guMtxConcat(mv,m,mv);
    GX_LoadPosMtxImm(mv, GX_PNMTX0);
    GX_Flush();

    /* draw textured quad */
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    GX_Position2s16(-w/2,-h/2);
    GX_Color4u8(0xff,0xff,0xff,alpha);
    GX_TexCoord2f32(0.0, 0.0);
    GX_Position2s16(w/2,-h/2);
    GX_Color4u8(0xff,0xff,0xff,alpha);
    GX_TexCoord2f32(1.0, 0.0);
    GX_Position2s16(w/2,h/2);
    GX_Color4u8(0xff,0xff,0xff,alpha);
    GX_TexCoord2f32(1.0, 1.0);
    GX_Position2s16(-w/2,h/2);
    GX_Color4u8(0xff,0xff,0xff,alpha);
    GX_TexCoord2f32(0.0, 1.0);
    GX_End ();
    GX_DrawDone();

    /* restore default Modelview */
    guLookAt(mv, &cam.pos, &cam.up, &cam.view);
    GX_LoadPosMtxImm(mv, GX_PNMTX0);
    GX_Flush();
  }
}

void gxDrawTextureRepeat(gx_texture *texture, s32 x, s32 y, s32 w, s32 h, u8 alpha)
{
  if (!texture) return;
  if (texture->data)
  {
    /* load texture object */
    GXTexObj texObj;
    GX_InitTexObj(&texObj, texture->data, texture->width, texture->height, GX_TF_RGBA8, GX_REPEAT, GX_REPEAT, GX_FALSE);
    GX_LoadTexObj(&texObj, GX_TEXMAP0);
    GX_InvalidateTexAll();

    /* vertex coordinate */
    x -= (vmode->fbWidth/2);
    y -= (vmode->efbHeight/2);

    /* texture coordinates */
    f32 s = (f32)w / (f32)texture->width;
    f32 t = (f32)h / (f32)texture->height;

    /* draw textured quad */
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    GX_Position2s16(x,y+h);
    GX_Color4u8(0xff,0xff,0xff,alpha);
    GX_TexCoord2f32(0.0, t);
    GX_Position2s16(x+w,y+h);
    GX_Color4u8(0xff,0xff,0xff,alpha);
    GX_TexCoord2f32(s, t);
    GX_Position2s16(x+w,y);
    GX_Color4u8(0xff,0xff,0xff,alpha);
    GX_TexCoord2f32(s, 0.0);
    GX_Position2s16(x,y);
    GX_Color4u8(0xff,0xff,0xff,alpha);
    GX_TexCoord2f32(0.0, 0.0);
    GX_End ();
    GX_DrawDone();
  }
}

void gxDrawScreenshot(u8 alpha)
{
  if (!rmode) return;

  /* get current game screen texture */
  GXTexObj texobj;
  GX_InitTexObj(&texobj, texturemem, vwidth, vheight, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
  GX_LoadTexObj(&texobj, GX_TEXMAP0);
  GX_InvalidateTexAll();

  /* get current aspect ratio */
  int xscale,yscale;
  gxSetAspectRatio(&xscale, &yscale);

  /* adjust horizontal scaling */
  xscale = (xscale * vmode->fbWidth) / vmode->viWidth;

  /* adjust screen position */
  int xshift = (config.xshift * vmode->fbWidth) / vmode->viWidth;
  int yshift = (config.yshift * vmode->efbHeight) / vmode->viHeight;

  /* set vertices position & size */
  s32 x = xshift - xscale;
  s32 y = yshift - (yscale * 2);
  s32 w = xscale * 2;
  s32 h = yscale * 4;

  /* draw textured quad */
  GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
  GX_Position2s16(x,y+h);
  GX_Color4u8(0xff,0xff,0xff,alpha);
  GX_TexCoord2f32(0.0, 1.0);
  GX_Position2s16(x+w,y+h);
  GX_Color4u8(0xff,0xff,0xff,alpha);
  GX_TexCoord2f32(1.0, 1.0);
  GX_Position2s16(x+w,y);
  GX_Color4u8(0xff,0xff,0xff,alpha);
  GX_TexCoord2f32(1.0, 0.0);
  GX_Position2s16(x,y);
  GX_Color4u8(0xff,0xff,0xff,alpha);
  GX_TexCoord2f32(0.0, 0.0);
  GX_End ();
  GX_DrawDone();
}

void gxCopyScreenshot(gx_texture *texture)
{
  /* retrieve gamescreen texture */
  GXTexObj texobj;
  GX_InitTexObj(&texobj, texturemem, vwidth, vheight, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
  GX_LoadTexObj(&texobj, GX_TEXMAP0);
  GX_InvalidateTexAll();

  /* scale texture to EFB width */
  s32 w = bitmap.viewport.x ? 696 : 640;
  s32 h = (bitmap.viewport.h + 2*bitmap.viewport.y) * 2;
  s32 x = -w/2;
  s32 y = -(240+ 2*bitmap.viewport.y);

  /* draw textured quad */
  GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
  GX_Position2s16(x,y+h);
  GX_Color4u8(0xff,0xff,0xff,0xff);
  GX_TexCoord2f32(0.0, 1.0);
  GX_Position2s16(x+w,y+h);
  GX_Color4u8(0xff,0xff,0xff,0xff);
  GX_TexCoord2f32(1.0, 1.0);
  GX_Position2s16(x+w,y);
  GX_Color4u8(0xff,0xff,0xff,0xff);
  GX_TexCoord2f32(1.0, 0.0);
  GX_Position2s16(x,y);
  GX_Color4u8(0xff,0xff,0xff,0xff);
  GX_TexCoord2f32(0.0, 0.0);
  GX_End ();

  /* copy EFB to texture */
  texture->format = GX_TF_RGBA8;
  texture->width = 320;
  texture->height = bitmap.viewport.h;
  texture->data = screenshot;
  GX_SetTexCopySrc(0, 0, texture->width * 2, texture->height * 2);
  GX_SetTexCopyDst(texture->width, texture->height, texture->format, GX_TRUE);
  GX_DrawDone();
  GX_CopyTex(texture->data, GX_TRUE);
  GX_Flush();

  /* wait for copy operation to finish */
  /* GX_PixModeSync is only useful if GX_ command follows */
  /* we use dummy GX commands to stall CPU execution */
  GX_PixModeSync();
  GX_LoadTexObj(&texobj, GX_TEXMAP0);
  GX_InvalidateTexAll();
  GX_Flush();
  DCFlushRange(texture->data, texture->width * texture->height * 4);
}

/* Take Screenshot */
void gxSaveScreenshot(char *filename)
{
  /* capture screenshot into a texture */
  gx_texture texture;
  gxCopyScreenshot(&texture);

  /* open PNG file */
  FILE *f = fopen(filename,"wb");
  if (f)
  {
    /* encode screenshot into PNG file */
    gxTextureWritePNG(&texture,f);
    fclose(f);
  }
}

void gxSetScreen(void)
{
  GX_DrawDone();
  GX_CopyDisp(xfb[whichfb], GX_FALSE);
  GX_Flush();
  VIDEO_SetNextFramebuffer (xfb[whichfb]);
  VIDEO_Flush ();
  VIDEO_WaitVSync ();
}

void gxClearScreen(GXColor color)
{
  whichfb ^= 1;
  GX_SetCopyClear(color,0x00ffffff);
  GX_CopyDisp(xfb[whichfb], GX_TRUE);
  GX_Flush();
}

/***************************************************************************************/
/*   GX Texture <-> LibPNG routines                                                    */
/***************************************************************************************/

/* libpng read callback function */
static void png_read_from_mem (png_structp png_ptr, png_bytep data, png_size_t length)
{
  png_image *image = (png_image *)png_get_io_ptr(png_ptr);

  /* copy data from image buffer */
  memcpy (data, image->buffer + image->offset, length);

  /* advance in the file */
  image->offset += length;
}

/* convert PNG image (from file or data buffer) into RGBA8 texture */
gx_texture *gxTextureOpenPNG(const u8 *png_data, FILE *png_file)
{
  int i;

  /* create a png read struct */
  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
  if (!png_ptr) return NULL;

  /* create a png info struct */
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    png_destroy_read_struct(&png_ptr,NULL,NULL);
    return NULL;
  }

  if (png_data)
  {
    /* init PNG image structure */
    png_image image;
    image.buffer = (u8 *) png_data;
    image.offset = 0;

    /* set callback for the read function */
    png_set_read_fn(png_ptr,(png_voidp *)(&image),png_read_from_mem);
  }
  else if (png_file)
  {
    /* check for valid magic number */
    png_byte magic[8];
    if (fread (magic, 1, 8, png_file) != 8)
    {
      png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
      return NULL;
    }

    if (png_sig_cmp (magic, 0, 8))
    {
      png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
      return NULL;
    }

    /* set IO callback for read function */
    png_init_io (png_ptr, png_file);
    png_set_sig_bytes (png_ptr, 8);
  }
  else
  {
    png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
    return NULL;
  }

  /* read png info */
  png_read_info(png_ptr,info_ptr);

  /* retrieve image information */
  u32 width  = png_get_image_width(png_ptr,info_ptr);
  u32 height = png_get_image_height(png_ptr,info_ptr);
  u32 bit_depth = png_get_bit_depth(png_ptr,info_ptr);
  u32 color_type = png_get_color_type(png_ptr,info_ptr);

  /* ensure PNG file is in the supported format */
  if (png_file)
  {
    /* support for RGBA8 textures ONLY !*/
    if ((color_type != PNG_COLOR_TYPE_RGB_ALPHA) || (bit_depth != 8))
    {
      png_destroy_read_struct(&png_ptr, &info_ptr,NULL);
      return NULL;
    }

    /* 4x4 tiles are required */
    if ((width%4) || (height%4))
    {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      return NULL;
    }
  }

  /* allocate memory to store raw image data */
  u32 stride = width << 2;
  u8 *img_data = memalign (32, stride * height);
  if (!img_data)
  {
    png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
    return NULL;
  }

  /* allocate row pointer data */
  png_bytep *row_pointers = (png_bytep *)memalign (32, sizeof (png_bytep) * height);
  if (!row_pointers)
  {
    free (img_data);
    png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
    return NULL;
  }

  /* store raw image data */
  for (i = 0; i < height; i++)
  {
    row_pointers[i] = img_data + (i * stride);
  }

  /* decode image */
  png_read_image(png_ptr, row_pointers);

  /* finish decompression and release memory */
  png_read_end(png_ptr, NULL);
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  free(row_pointers);

  /* initialize texture */
  gx_texture *texture = (gx_texture *)memalign(32, sizeof(gx_texture));
  if (!texture)
  {
    free (img_data);
    return NULL;
  }

  /* initialize texture data */
  texture->data = memalign(32, stride * height);
  if (!texture->data)
  {
    free (img_data);
    free(texture);
    return NULL;
  }

  memset(texture->data, 0, stride * height);
  texture->width  = width;
  texture->height = height;
  texture->format = GX_TF_RGBA8;

  /* encode to GX_TF_RGBA8 format (4x4 pixels paired titles) */
  u16 *dst_ar = (u16 *)(texture->data);
  u16 *dst_gb = (u16 *)(texture->data + 32);
  u32 *src1 = (u32 *)(img_data);
  u32 *src2 = (u32 *)(img_data + stride);
  u32 *src3 = (u32 *)(img_data + 2*stride);
  u32 *src4 = (u32 *)(img_data + 3*stride);
  u32 pixel,h,w;

  for (h=0; h<height; h+=4)
  {
    for (w=0; w<width; w+=4)
    {
      /* line N (4 pixels) */
      for (i=0; i<4; i++)
      {
        pixel = *src1++;
        *dst_ar++= ((pixel << 8) & 0xff00) | ((pixel >> 24) & 0x00ff);
        *dst_gb++= (pixel >> 8) & 0xffff;
      }

      /* line N + 1 (4 pixels) */
      for (i=0; i<4; i++)
      {
        pixel = *src2++;
        *dst_ar++= ((pixel << 8) & 0xff00) | ((pixel >> 24) & 0x00ff);
        *dst_gb++= (pixel >> 8) & 0xffff;
      }

      /* line N + 2 (4 pixels) */
      for (i=0; i<4; i++)
      {
        pixel = *src3++;
        *dst_ar++= ((pixel << 8) & 0xff00) | ((pixel >> 24) & 0x00ff);
        *dst_gb++= (pixel >> 8) & 0xffff;
      }

      /* line N + 3 (4 pixels) */
      for (i=0; i<4; i++)
      {
        pixel = *src4++;
        *dst_ar++= ((pixel << 8) & 0xff00) | ((pixel >> 24) & 0x00ff);
        *dst_gb++= (pixel >> 8) & 0xffff;
      }

      /* next paired tiles */
      dst_ar += 16;
      dst_gb += 16;
    }

    /* next 4 lines */
    src1 = src4;
    src2 = src1 + width;
    src3 = src2 + width;
    src4 = src3 + width;
  }

  /* release memory */
  free(img_data);

  /* flush texture data from cache */
  DCFlushRange(texture->data, height * stride);

  return texture;
}

/* Write RGBA8 Texture to PNG file */
void gxTextureWritePNG(gx_texture *texture, FILE *png_file)
{
  /* allocate PNG data buffer */
  u8 *img_data = (u8 *)memalign(32, texture->width * texture->height * 4);
  if(!img_data) return;

  /* decode GX_TF_RGBA8 format (4x4 pixels paired titles) */
  u16 *ar = (u16 *)(texture->data);
  u16 *gb = (u16 *)(texture->data + 32);
  u32 *dst1 = (u32 *)(img_data);
  u32 *dst2 = dst1 + texture->width;
  u32 *dst3 = dst2 + texture->width;
  u32 *dst4 = dst3 + texture->width;
  u32 i,h,w,pixel;

  for (h=0; h<texture->height; h+=4)
  {
    for (w=0; w<texture->width; w+=4)
    {
      /* line N (4 pixels) */
      for (i=0; i<4; i++)
      {
        pixel = ((*ar & 0xff) << 24) | (*gb << 8) | ((*ar & 0xff00) >> 8);
        *dst1++ = pixel;
        ar++;
        gb++;
      }

      /* line N + 1 (4 pixels) */
      for (i=0; i<4; i++)
      {
        pixel = ((*ar & 0xff) << 24) | (*gb << 8) | ((*ar & 0xff00) >> 8);
        *dst2++ = pixel;
        ar++;
        gb++;
      }

      /* line N + 2 (4 pixels) */
      for (i=0; i<4; i++)
      {
        pixel = ((*ar & 0xff) << 24) | (*gb << 8) | ((*ar & 0xff00) >> 8);
        *dst3++ = pixel;
        ar++;
        gb++;
      }

      /* line N + 3 (4 pixels) */
      for (i=0; i<4; i++)
      {
        pixel = ((*ar & 0xff) << 24) | (*gb << 8) | ((*ar & 0xff00) >> 8);
        *dst4++ = pixel;
        ar++;
        gb++;
      }

      /* next paired tiles */
      ar += 16;
      gb += 16;
    }

    /* next 4 lines */
    dst1 = dst4;
    dst2 = dst1 + texture->width;
    dst3 = dst2 + texture->width;
    dst4 = dst3 + texture->width;
  }

  /* create a png write struct */
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png_ptr)
  {
    free(img_data);
    return;
  }

  /* create a png info struct */
  png_infop info_ptr = png_create_info_struct (png_ptr);
  if (!info_ptr)
  {
    free(img_data);
    png_destroy_write_struct(&png_ptr, NULL);
    return;
  }

  /* set IO callback for the write function */
  png_init_io(png_ptr, png_file);

  /* set PNG file properties */
  png_set_IHDR(png_ptr, info_ptr, texture->width, texture->height, 8, PNG_COLOR_TYPE_RGB_ALPHA, 
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  /* allocate row pointer data */
  png_bytep *row_pointers = (png_bytep *)memalign (32, sizeof (png_bytep) * texture->height);
  if (!row_pointers)
  {
    free (img_data);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return;
  }

  /* store raw image data */
  for (i = 0; i < texture->height; i++)
  {
    row_pointers[i] = img_data + (i * texture->width * 4);
  }

  /* configure libpng for image data */
  png_set_rows(png_ptr,info_ptr,row_pointers);

  /* write data to PNG file */
  png_write_png(png_ptr,info_ptr,PNG_TRANSFORM_IDENTITY,NULL);

  /* finish compression and release memory */
  png_write_end(png_ptr, NULL);
  free(row_pointers);
  free(img_data);
  png_destroy_write_struct(&png_ptr, &info_ptr);
}


void gxTextureClose(gx_texture **p_texture)
{
  gx_texture *texture = *p_texture;

  if (texture)
  {
    if (texture->data) free(texture->data);
    free(texture);
    *p_texture = NULL;
  }
}


/***************************************************************************************/
/*   VIDEO engine                                                                      */
/***************************************************************************************/

/* Emulation mode -> Menu mode */
void gx_video_Stop(void)
{
  /* unallocate NTSC filters */
  if (sms_ntsc) free(sms_ntsc);
  if (md_ntsc) free(md_ntsc);
  sms_ntsc = NULL;
  md_ntsc = NULL;

  /* lightgun textures */
  gxTextureClose(&crosshair[0]);
  gxTextureClose(&crosshair[1]);

  /* GX menu rendering */
  gxResetRendering(1);
  gxResetMode(vmode);

  /* display game snapshot */
  gxClearScreen((GXColor)BLACK);
  gxDrawScreenshot(0xff);

  /* default VI settings */
  VIDEO_SetPreRetraceCallback(NULL);
  VIDEO_SetPostRetraceCallback(gx_input_UpdateMenu);
#ifdef HW_RVL
  VIDEO_SetTrapFilter(1);
  VIDEO_SetGamma(VI_GM_1_0);
#endif

  /* adjust TV width */
  vmode->viWidth    = config.screen_w;
  vmode->viXOrigin  = (VI_MAX_WIDTH_NTSC - vmode->viWidth)/2;
  VIDEO_Configure(vmode);

  /* wait for VSYNC */
  gxSetScreen();
}

/* Menu mode -> Emulation mode */
void gx_video_Start(void)
{
  /* 50Hz/60Hz mode */
  if ((config.tv_mode == 1) || ((config.tv_mode == 2) && vdp_pal))
  {
    gc_pal = 1;
  }
  else
  {
    gc_pal = 0;
  }

#ifdef HW_RVL
  VIDEO_SetTrapFilter(config.trap);
  VIDEO_SetGamma((int)(config.gamma * 10.0));
#endif

  /* VSYNC callbacks */
  /* in 60hz mode, frame emulation is synchronized with Video Interrupt */
  if (!gc_pal && !vdp_pal)
  {
    VIDEO_SetPreRetraceCallback(vi_callback);
  }
  VIDEO_SetPostRetraceCallback(NULL);
  VIDEO_Flush();

  /* set interlaced or progressive video mode */
  if (config.render == 2)
  {
    tvmodes[2]->viTVMode = VI_TVMODE_NTSC_PROG;
    tvmodes[2]->xfbMode = VI_XFBMODE_SF;
  }
  else if (config.render == 1)
  {
    tvmodes[2]->viTVMode = tvmodes[0]->viTVMode & ~3;
    tvmodes[2]->xfbMode = VI_XFBMODE_DF;
  }

  /* force video update */
  bitmap.viewport.changed = 3;

  /* NTSC filter */
  if (config.ntsc)
  {
    /* allocate filters */
    if (!sms_ntsc)
    {
      sms_ntsc = (sms_ntsc_t *)memalign(32,sizeof(sms_ntsc_t));
    }
    if (!md_ntsc)
    {
      md_ntsc = (md_ntsc_t *)memalign(32,sizeof(md_ntsc_t));
    }

    /* setup filters default configuration */
    switch (config.ntsc)
    {
      case 1:
        sms_ntsc_init(sms_ntsc, &sms_ntsc_composite);
        md_ntsc_init(md_ntsc, &md_ntsc_composite);
        break;
      case 2:
        sms_ntsc_init(sms_ntsc, &sms_ntsc_svideo);
        md_ntsc_init(md_ntsc, &md_ntsc_svideo);
        break;
      case 3:
        sms_ntsc_init(sms_ntsc, &sms_ntsc_rgb);
        md_ntsc_init(md_ntsc, &md_ntsc_rgb);
        break;
    }
  }

  /* lightgun textures */
  if (config.gun_cursor[0] && ((input.system[1] == SYSTEM_MENACER) || (input.system[1] == SYSTEM_JUSTIFIER) || (input.system[0] == SYSTEM_LIGHTPHASER)))
  {
    crosshair[0] = gxTextureOpenPNG(Crosshair_p1_png,0);
  }
  if (config.gun_cursor[1] && ((input.system[1] == SYSTEM_JUSTIFIER) || (input.system[1] == SYSTEM_LIGHTPHASER)))
  {
    crosshair[1] = gxTextureOpenPNG(Crosshair_p2_png,0);
  }

  /* GX emulation rendering */
  gxResetRendering(0);

  /* resynchronize emulation with VSYNC*/
  VIDEO_WaitVSync();
}

/* GX render update */
void gx_video_Update(void)
{
  int update = bitmap.viewport.changed & 1;

  /* check if display has changed during frame */
  if (update)
  {
    /* update texture size */
    vwidth = bitmap.viewport.w + (2 * bitmap.viewport.x);
    vheight = bitmap.viewport.h + (2 * bitmap.viewport.y);

    /* interlaced mode */
    if (config.render && interlaced)
    {
      vheight = vheight << 1;
    }

    /* ntsc filter */
    if (config.ntsc)
    {
      vwidth = (reg[12] & 1) ? MD_NTSC_OUT_WIDTH(vwidth) : SMS_NTSC_OUT_WIDTH(vwidth);
    }

    /* texels size must be multiple of 4 */
    vwidth  = (vwidth  >> 2) << 2;
    vheight = (vheight >> 2) << 2;

    /* initialize texture object */
    GXTexObj texobj;
    GX_InitTexObj(&texobj, texturemem, vwidth, vheight, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);

    /* configure texture filtering */
    if (!config.bilinear)
    {
      GX_InitTexObjLOD(&texobj,GX_NEAR,GX_NEAR_MIP_NEAR,0.0,10.0,0.0,GX_FALSE,GX_FALSE,GX_ANISO_1);
    }

    /* load texture object */
    GX_LoadTexObj(&texobj, GX_TEXMAP0);

    /* update rendering mode */
    if (config.render)
    {
      rmode = tvmodes[gc_pal*3 + 2];
    }
    else
    {
      rmode = tvmodes[gc_pal*3 + interlaced];
    }

    /* update aspect ratio */
    gxResetScaler(vwidth);

    /* update GX rendering mode */
    gxResetMode(rmode);

    /* update VI mode */
    VIDEO_Configure(rmode);
  }

  /* texture is now directly mapped by the line renderer */

  /* force texture cache update */
  DCFlushRange(texturemem, TEX_SIZE);
  GX_InvalidateTexAll();

  /* render textured quad */
  draw_square();

  /* Lightgun # 1 screen mark */
  if (crosshair[0])
  {
    if (input.system[0] == SYSTEM_LIGHTPHASER)
    {
      gxDrawCrosshair(crosshair[0], input.analog[0][0],input.analog[0][1]);
    }
    else
    {
      gxDrawCrosshair(crosshair[0], input.analog[4][0],input.analog[4][1]);
    }
  }

  /* Lightgun # 2 screen mark */
  if (crosshair[1])
  {
    if (input.system[1] == SYSTEM_LIGHTPHASER)
    {
      gxDrawCrosshair(crosshair[1], input.analog[1][0],input.analog[1][1]);
    }
    else
    {
      gxDrawCrosshair(crosshair[1], input.analog[5][0],input.analog[5][1]);
    }
  }

  /* swap XFB */ 
  whichfb ^= 1;

  /* copy EFB to XFB */
  GX_DrawDone();
  GX_CopyDisp(xfb[whichfb], GX_TRUE);
  GX_Flush();

  /* XFB is ready to be displayed */
  VIDEO_SetNextFramebuffer(xfb[whichfb]);
  VIDEO_Flush();

  if (update)
  {
    /* Clear update flags */
    bitmap.viewport.changed &= ~1;

    /* field synchronization */
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE)
    {
      VIDEO_WaitVSync();
    }
    else while (VIDEO_GetNextField() != odd_frame)
    {
      VIDEO_WaitVSync();
    }

    /* audio & video resynchronization */
    audioStarted = 0;
  }
}

/* Initialize VIDEO subsystem */
void gx_video_Init(void)
{
  /*
   * Before doing anything else under libogc,
   * Call VIDEO_Init
   */
  VIDEO_Init();

  /*
   * Before any memory is allocated etc.
   * Rescue any tagged ROM in data 2
   */
  int *romptr = (int *)0x80700000;
  StartARAM();
  cart.romsize = 0;
  if (memcmp((char *)romptr,"GENPLUSR",8) == 0)
  {
    cart.romsize = romptr[2];
    ARAMPut((char *) 0x80700000 + 0x20, (char *) 0x8000, cart.romsize);
  }

  /* Get the current VIDEO mode then :
      - set menu VIDEO mode (480p, 480i or 576i)
      - set emulator rendering TV modes (PAL/MPAL/NTSC/EURGB60)
   */
  vmode = VIDEO_GetPreferredMode(NULL);

  /* Adjust display settings */
  switch (vmode->viTVMode >> 2)
  {
    case VI_PAL:  /* 576 lines (PAL 50Hz) */

      TV60hz_240p.viTVMode = VI_TVMODE_EURGB60_DS;
      TV60hz_240i.viTVMode = VI_TVMODE_EURGB60_INT;
      TV60hz_480i.viTVMode = VI_TVMODE_EURGB60_INT;
      config.tv_mode = 1;

      /* use harwdare vertical scaling to fill screen */
      vmode = &TVPal574IntDfScale;
      break;
    
    case VI_NTSC: /* 480 lines (NTSC 60hz) */
      TV60hz_240p.viTVMode = VI_TVMODE_NTSC_DS;
      TV60hz_240i.viTVMode = VI_TVMODE_NTSC_INT;
      TV60hz_480i.viTVMode = VI_TVMODE_NTSC_INT;
      config.tv_mode = 0;

#ifndef HW_RVL
      /* force 480p on NTSC GameCube if the Component Cable is present */
      if (VIDEO_HaveComponentCable()) vmode = &TVNtsc480Prog;
#endif
      break;

    default:  /* 480 lines (PAL 60Hz) */
      TV60hz_240p.viTVMode = VI_TVMODE(vmode->viTVMode >> 2, VI_NON_INTERLACE);
      TV60hz_240i.viTVMode = VI_TVMODE(vmode->viTVMode >> 2, VI_INTERLACE);
      TV60hz_480i.viTVMode = VI_TVMODE(vmode->viTVMode >> 2, VI_INTERLACE);
      config.tv_mode = 2;
      break;
  }

  /* Configure VI */
  VIDEO_Configure (vmode);

  /* Configure the framebuffers (double-buffering) */
  xfb[0] = (u32 *) MEM_K0_TO_K1((u32 *) SYS_AllocateFramebuffer(&TV50hz_576i));
  xfb[1] = (u32 *) MEM_K0_TO_K1((u32 *) SYS_AllocateFramebuffer(&TV50hz_576i));

  /* Define a console */
  console_init(xfb[0], 20, 64, 640, 574, 574 * 2);

  /* Clear framebuffers to black */
  VIDEO_ClearFrameBuffer(vmode, xfb[0], COLOR_BLACK);
  VIDEO_ClearFrameBuffer(vmode, xfb[1], COLOR_BLACK);

  /* Set the framebuffer to be displayed at next VBlank */
  VIDEO_SetNextFramebuffer(xfb[0]);

  /* Enable Video Interface */
  VIDEO_SetBlack(FALSE);

  /* Update VIDEO settings for next VBlank */
  VIDEO_Flush();

  /* Wait for VBlank */
  VIDEO_WaitVSync();
  VIDEO_WaitVSync();

  /* Initialize GX */
  gxStart();
  gxResetRendering(1);
  gxResetMode(vmode);

  /* initialize FONT */
  if (!FONT_Init())
  {
#ifdef HW_RVL
    DI_Close();
    SYS_ResetSystem(SYS_RESTART,0,0);
#else
    SYS_ResetSystem(SYS_HOTRESET,0,0);
#endif
  }

  /* Initialize textures */
  texturemem = memalign(32, TEX_SIZE);
  screenshot = memalign(32, HASPECT*VASPECT*4);
  if (!texturemem || !screenshot)
  {
    FONT_writeCenter("Failed to allocate textures memory... Rebooting",18,0,640,200,(GXColor)WHITE);
    gxSetScreen();
    sleep(2);
    gx_video_Shutdown();
#ifdef HW_RVL
    DI_Close();
    SYS_ResetSystem(SYS_RESTART,0,0);
#else
    SYS_ResetSystem(SYS_HOTRESET,0,0);
#endif
  }
}

void gx_video_Shutdown(void)
{
  if (texturemem) free(texturemem);
  if (screenshot) free(screenshot);
  FONT_Shutdown();
  VIDEO_ClearFrameBuffer(vmode, xfb[whichfb], COLOR_BLACK);
  VIDEO_Flush();
  VIDEO_WaitVSync();
}
