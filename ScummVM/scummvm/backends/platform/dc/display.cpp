/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#define RONIN_TIMER_ACCESS

#include <common/scummsys.h>
#include <graphics/surface.h>
#include "dc.h"

#define SCREEN_W 640
#define SCREEN_H 480
#define MOUSE_W 128
#define MOUSE_H 128

#define OVL_W 320
#define OVL_H 200
#define OVL_TXSTRIDE 512

#define TOP_OFFSET (_top_offset+_yscale*_current_shake_pos)

static const struct {
  Graphics::PixelFormat pixelFormat;
  unsigned int textureFormat;
  operator const Graphics::PixelFormat&() const { return pixelFormat; }
} screenFormats[] = {
  /* Note: These are ordered by _increasing_ preference, so that
     CLUT8 appears at index 0.  getSupportedFormats() will return
     them in reversed order. */
  { Graphics::PixelFormat::createFormatCLUT8(), TA_TEXTUREMODE_ARGB1555 },
  { Graphics::PixelFormat(2,4,4,4,4,8,4,0,12), TA_TEXTUREMODE_ARGB4444 },
  { Graphics::PixelFormat(2,5,5,5,1,10,5,0,15), TA_TEXTUREMODE_ARGB1555 },
  { Graphics::PixelFormat(2,5,6,5,0,11,5,0,0), TA_TEXTUREMODE_RGB565 },
};
#define NUM_FORMATS (sizeof(screenFormats)/sizeof(screenFormats[0]))


#define QACR0 (*(volatile unsigned int *)(void *)0xff000038)
#define QACR1 (*(volatile unsigned int *)(void *)0xff00003c)


#define COPYPIXEL(n) do {			\
  unsigned short _tmp = pal[*s++];		\
  d[n] = _tmp|(pal[*s++]<<16);			\
} while (0)

static void texture_memcpy64_pal(void *dest, void *src, int cnt, unsigned short *pal)
{
  unsigned char *s = (unsigned char *)src;
  unsigned int *d = (unsigned int *)(void *)
    (0xe0000000 | (((unsigned long)dest) & 0x03ffffc0));
  QACR0 = ((0xa4000000>>26)<<2)&0x1c;
  QACR1 = ((0xa4000000>>26)<<2)&0x1c;
  while (cnt--) {
    COPYPIXEL(0);
    COPYPIXEL(1);
    COPYPIXEL(2);
    COPYPIXEL(3);
    asm("pref @%0" : : "r" (s+4*16));
    COPYPIXEL(4);
    COPYPIXEL(5);
    COPYPIXEL(6);
    COPYPIXEL(7);
    asm("pref @%0" : : "r" (d));
    d += 8;
    COPYPIXEL(0);
    COPYPIXEL(1);
    COPYPIXEL(2);
    COPYPIXEL(3);
    asm("pref @%0" : : "r" (s+4*16));
    COPYPIXEL(4);
    COPYPIXEL(5);
    COPYPIXEL(6);
    COPYPIXEL(7);
    asm("pref @%0" : : "r" (d));
    d += 8;
  }
}

static void texture_memcpy64(void *dest, void *src, int cnt)
{
  unsigned int *s = (unsigned int *)src;
  unsigned int *d = (unsigned int *)(void *)
    (0xe0000000 | (((unsigned long)dest) & 0x03ffffc0));
  QACR0 = ((0xa4000000>>26)<<2)&0x1c;
  QACR1 = ((0xa4000000>>26)<<2)&0x1c;
  while (cnt--) {
    d[0] = *s++;
    d[1] = *s++;
    d[2] = *s++;
    d[3] = *s++;
    asm("pref @%0" : : "r" (s+16));
    d[4] = *s++;
    d[5] = *s++;
    d[6] = *s++;
    d[7] = *s++;
    asm("pref @%0" : : "r" (d));
    d += 8;
    d[0] = *s++;
    d[1] = *s++;
    d[2] = *s++;
    d[3] = *s++;
    asm("pref @%0" : : "r" (s+16));
    d[4] = *s++;
    d[5] = *s++;
    d[6] = *s++;
    d[7] = *s++;
    asm("pref @%0" : : "r" (d));
    d += 8;
  }
}

void commit_dummy_transpoly()
{
  struct polygon_list mypoly;

  mypoly.cmd =
    TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_TRANSPARENT|TA_CMD_POLYGON_SUBLIST|
    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  mypoly.mode2 =
    TA_POLYMODE2_BLEND_SRC_ALPHA|TA_POLYMODE2_BLEND_DST_INVALPHA|
    TA_POLYMODE2_FOG_DISABLED|TA_POLYMODE2_ENABLE_ALPHA;
  mypoly.texture = 0;
  mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0;
  ta_commit_list(&mypoly);
}


void OSystem_Dreamcast::setPalette(const byte *colors, uint start, uint num)
{
  unsigned short *dst = palette + start;
  if (num>0)
    while ( num-- ) {
      *dst++ = ((colors[0]<<7)&0x7c00)|
	((colors[1]<<2)&0x03e0)|
	((colors[2]>>3)&0x001f);
      colors += 3;
    }
  _screen_dirty = true;
}

void OSystem_Dreamcast::setCursorPalette(const byte *colors, uint start, uint num)
{
  unsigned short *dst = cursor_palette + start;
  if (num>0)
    while ( num-- ) {
      *dst++ = ((colors[0]<<7)&0x7c00)|
	((colors[1]<<2)&0x03e0)|
	((colors[2]>>3)&0x001f);
      colors += 3;
    }
  _enable_cursor_palette = true;
}

void OSystem_Dreamcast::grabPalette(byte *colors, uint start, uint num)
{
  const unsigned short *src = palette + start;
  if (num>0)
    while ( num-- ) {
      unsigned short p = *src++;
      colors[0] = ((p&0x7c00)>>7)|((p&0x7000)>>12);
      colors[1] = ((p&0x03e0)>>2)|((p&0x0380)>>7);
      colors[2] = ((p&0x001f)<<3)|((p&0x001c)>>2);
      colors += 3;
    }
}

Graphics::PixelFormat OSystem_Dreamcast::getScreenFormat() const
{
  return screenFormats[_screenFormat];
}

Common::List<Graphics::PixelFormat> OSystem_Dreamcast::getSupportedFormats() const
{
  Common::List<Graphics::PixelFormat> list;
  unsigned i;
  for (i=0; i<NUM_FORMATS; i++)
    list.push_front(screenFormats[i]);
  return list;
}

void OSystem_Dreamcast::setScaling()
{
  if (_screen_w > 400) {
    _xscale = _yscale = 1.0;
    _top_offset = (SCREEN_H-_screen_h)>>1;
  } else if (_aspect_stretch && _screen_w == 320 && _screen_h == 200) {
    _xscale = SCREEN_W/320.0;
    _yscale = SCREEN_H/200.0;
    _top_offset = 0;
  } else {
    _xscale = _yscale = 2.0;
    _top_offset = (SCREEN_H>>1)-_screen_h;
  }
}

void OSystem_Dreamcast::initSize(uint w, uint h, const Graphics::PixelFormat *format)
{
  assert(w <= SCREEN_W && h <= SCREEN_H);

  int i = 0;
  if (format != NULL)
    for (i=NUM_FORMATS-1; i>0; --i)
      if (*format == screenFormats[i])
	break;
  _screenFormat = i;

  _overlay_visible = false;
  _overlay_fade = 0.0;
  _screen_w = w;
  _screen_h = h;
  _overlay_x = (w-OVL_W)/2;
  _overlay_y = (h-OVL_H)/2;
  if (_overlay_x<0) _overlay_x = 0;
  if (_overlay_y<0) _overlay_y = 0;
  setScaling();
  ta_sync();
  if (!screen)
    screen = new unsigned char[SCREEN_W*SCREEN_H*2];
  if (!overlay)
    overlay = new unsigned short[OVL_W*OVL_H];
  for (i=0; i<NUM_BUFFERS; i++)
    if (!screen_tx[i])
      screen_tx[i] = ta_txalloc(SCREEN_W*SCREEN_H*2);
  for (i=0; i<NUM_BUFFERS; i++)
    if (!mouse_tx[i])
      mouse_tx[i] = ta_txalloc(MOUSE_W*MOUSE_H*2);
  for (i=0; i<NUM_BUFFERS; i++)
    if (!ovl_tx[i])
      ovl_tx[i] = ta_txalloc(OVL_TXSTRIDE*OVL_H*2);
  _screen_buffer = 0;
  _mouse_buffer = 0;
  _overlay_buffer = 0;
  _screen_dirty = true;
  _overlay_dirty = true;
  *(volatile unsigned int *)(0xa05f80e4) = SCREEN_W/32; //stride
  //  dc_reset_screen(0, 0);
  memset(screen, 0, SCREEN_W*SCREEN_H*2);
  memset(overlay, 0, OVL_W*OVL_H*sizeof(unsigned short));

  _devpoll = Timer();
}

void OSystem_Dreamcast::copyRectToScreen(const void *buf, int pitch, int x, int y,
					 int w, int h)
{
  if (w<1 || h<1)
    return;
  if (_screenFormat != 0) {
    x<<=1; w<<=1;
  }
  unsigned char *dst = screen + y*SCREEN_W*2 + x;
  const byte *src = (const byte *)buf;
  do {
    memcpy(dst, src, w);
    dst += SCREEN_W*2;
    src += pitch;
  } while (--h);
  _screen_dirty = true;
}

bool OSystem_Dreamcast::showMouse(bool visible)
{
  bool last = _ms_visible;
  _ms_visible = visible;

  return last;
}

void OSystem_Dreamcast::warpMouse(int x, int y)
{
  _ms_cur_x = x;
  _ms_cur_y = y;
}

void OSystem_Dreamcast::setMouseCursor(const void *buf, uint w, uint h,
				       int hotspot_x, int hotspot_y,
				       uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format)
{
  _ms_cur_w = w;
  _ms_cur_h = h;

  _ms_hotspot_x = hotspot_x;
  _ms_hotspot_y = hotspot_y;

  _ms_keycolor = keycolor;

  int i = 0;
  if (format != NULL)
    for (i=NUM_FORMATS-1; i>0; --i)
      if (*format == screenFormats[i])
	break;
  _mouseFormat = i;

  free(_ms_buf);

  if (_mouseFormat != 0)
    w <<= 1;

  _ms_buf = (byte *)malloc(w * h);
  memcpy(_ms_buf, buf, w * h);
}

void OSystem_Dreamcast::setShakePos(int shake_pos)
{
  _current_shake_pos = shake_pos;
}

void OSystem_Dreamcast::updateScreenTextures(void)
{
  if (_screen_dirty) {

    _screen_buffer++;
    _screen_buffer &= NUM_BUFFERS-1;

    unsigned short *dst = (unsigned short *)screen_tx[_screen_buffer];
    unsigned char *src = screen;

    // while ((*((volatile unsigned int *)(void *)0xa05f810c) & 0x3ff) != 200);
    // *((volatile unsigned int *)(void *)0xa05f8040) = 0xff0000;

    if (_screenFormat == 0)
      for ( int y = 0; y<_screen_h; y++ )
      {
	texture_memcpy64_pal( dst, src, _screen_w>>5, palette );
	src += SCREEN_W*2;
	dst += SCREEN_W;
      }
    else
      for ( int y = 0; y<_screen_h; y++ )
      {
	texture_memcpy64( dst, src, _screen_w>>5 );
	src += SCREEN_W*2;
	dst += SCREEN_W;
      }

    _screen_dirty = false;
  }

  if ( _overlay_visible && _overlay_dirty ) {

    _overlay_buffer++;
    _overlay_buffer &= NUM_BUFFERS-1;

    unsigned short *dst = (unsigned short *)ovl_tx[_overlay_buffer];
    unsigned short *src = overlay;

    for ( int y = 0; y<OVL_H; y++ )
    {
      texture_memcpy64( dst, src, OVL_W>>5 );
      src += OVL_W;
      dst += OVL_TXSTRIDE;
    }

    _overlay_dirty = false;
  }
}

void OSystem_Dreamcast::updateScreenPolygons(void)
{
  struct polygon_list mypoly;
  struct packed_colour_vertex_list myvertex;

  // *((volatile unsigned int *)(void *)0xa05f8040) = 0x00ff00;

  mypoly.cmd =
    TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_OPAQUE|TA_CMD_POLYGON_SUBLIST|
    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR|TA_CMD_POLYGON_TEXTURED;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  mypoly.mode2 =
    TA_POLYMODE2_BLEND_SRC|TA_POLYMODE2_FOG_DISABLED|TA_POLYMODE2_TEXTURE_REPLACE|
    TA_POLYMODE2_U_SIZE_1024|TA_POLYMODE2_V_SIZE_1024;
  mypoly.texture = screenFormats[_screenFormat].textureFormat|
    TA_TEXTUREMODE_NON_TWIDDLED|TA_TEXTUREMODE_STRIDE|
    TA_TEXTUREMODE_ADDRESS(screen_tx[_screen_buffer]);

  mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0;

  ta_begin_frame();
  // *((volatile unsigned int *)(void *)0xa05f8040) = 0x0000ff;
  ta_commit_list(&mypoly);

  myvertex.cmd = TA_CMD_VERTEX;
  myvertex.ocolour = 0;
  myvertex.colour = 0;
  myvertex.z = 0.5;
  myvertex.u = 0.0;
  myvertex.v = 0.0;

  myvertex.x = 0.0;
  myvertex.y = TOP_OFFSET;
  ta_commit_list(&myvertex);

  myvertex.x = _screen_w*_xscale;
  myvertex.u = _screen_w*(1/1024.0);
  ta_commit_list(&myvertex);

  myvertex.x = 0.0;
  myvertex.y += _screen_h*_yscale;
  myvertex.u = 0.0;
  myvertex.v = _screen_h*(1/1024.0);
  ta_commit_list(&myvertex);

  myvertex.x = _screen_w*_xscale;
  myvertex.u = _screen_w*(1/1024.0);
  myvertex.cmd |= TA_CMD_VERTEX_EOS;
  ta_commit_list(&myvertex);

  ta_commit_end();

  if (_overlay_visible) {
    if (_overlay_fade < 1.0)
      _overlay_fade += 0.125;
  } else {
    if (_overlay_fade > 0)
      _overlay_fade -= 0.125;
  }

  if (_overlay_fade > 0.0) {

    mypoly.cmd =
      TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_TRANSPARENT|TA_CMD_POLYGON_SUBLIST|
      TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR|TA_CMD_POLYGON_TEXTURED;
    mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
    mypoly.mode2 =
      TA_POLYMODE2_BLEND_SRC/*_ALPHA*/|TA_POLYMODE2_BLEND_DST_INVALPHA|
      TA_POLYMODE2_ENABLE_ALPHA|
      TA_POLYMODE2_FOG_DISABLED|TA_POLYMODE2_TEXTURE_MODULATE_ALPHA|
      TA_POLYMODE2_U_SIZE_512|TA_POLYMODE2_V_SIZE_512;
    mypoly.texture = TA_TEXTUREMODE_ARGB4444|TA_TEXTUREMODE_NON_TWIDDLED|
      TA_TEXTUREMODE_ADDRESS(ovl_tx[_overlay_buffer]);

    mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0.0;

    ta_commit_list(&mypoly);

    myvertex.cmd = TA_CMD_VERTEX;
    myvertex.ocolour = 0;
    myvertex.colour = 0xffffff|(((int)(255*_overlay_fade))<<24);

    myvertex.z = 0.5;
    myvertex.u = 0.0;
    myvertex.v = 0.0;

    myvertex.x = _overlay_x*_xscale;
    myvertex.y = _overlay_y*_yscale+TOP_OFFSET;
    ta_commit_list(&myvertex);

    myvertex.x += OVL_W*_xscale;
    myvertex.u = OVL_W*(1.0/512.0);
    ta_commit_list(&myvertex);

    myvertex.x = _overlay_x*_xscale;
    myvertex.y += OVL_H*_yscale;
    myvertex.u = 0.0;
    myvertex.v = OVL_H*(1.0/512.0);
    ta_commit_list(&myvertex);

    myvertex.x += OVL_W*_xscale;
    myvertex.u = OVL_W*(1.0/512.0);
    myvertex.cmd |= TA_CMD_VERTEX_EOS;
    ta_commit_list(&myvertex);
  }

  if (_softkbd_on)
    if (_softkbd_motion < 120)
      _softkbd_motion += 10;
    else
      ;
  else
    if (_softkbd_motion > 0)
      _softkbd_motion -= 10;

  if (_softkbd_motion)
    _softkbd.draw(330.0*sin(0.013*_softkbd_motion) - 320.0, 200.0,
		  120-_softkbd_motion);

  // *((volatile unsigned int *)(void *)0xa05f8040) = 0xffff00;
  drawMouse(_ms_cur_x, _ms_cur_y, _ms_cur_w, _ms_cur_h, _ms_buf, _ms_visible);
  // *((volatile unsigned int *)(void *)0xa05f8040) = 0xff00ff;
  ta_commit_frame();

  // *((volatile unsigned int *)(void *)0xa05f8040) = 0x0;

  _last_screen_refresh = Timer();
}

void OSystem_Dreamcast::updateScreen(void)
{
  updateScreenTextures();
  updateScreenPolygons();
}

void OSystem_Dreamcast::maybeRefreshScreen(void)
{
  unsigned int t = Timer();
  if((int)(t-_last_screen_refresh) > USEC_TO_TIMER(30000))
    updateScreenPolygons();
}

void OSystem_Dreamcast::drawMouse(int xdraw, int ydraw, int w, int h,
				  unsigned char *buf, bool visible)
{
  if (!visible || buf == NULL || !w || !h || w>MOUSE_W || h>MOUSE_H) {
    commit_dummy_transpoly();
    return;
  }

  struct polygon_list mypoly;
  struct packed_colour_vertex_list myvertex;

  _mouse_buffer++;
  _mouse_buffer &= NUM_BUFFERS-1;

  unsigned short *dst = (unsigned short *)mouse_tx[_mouse_buffer];
  unsigned int texturemode = screenFormats[_mouseFormat].textureFormat;

  if (_mouseFormat == 0) {
    unsigned short *pal = _enable_cursor_palette? cursor_palette : palette;
    for (int y=0; y<h; y++) {
      int x;
      for (x=0; x<w; x++)
	if (*buf == _ms_keycolor) {
	  *dst++ = 0;
	  buf++;
	} else
	  *dst++ = pal[*buf++]|0x8000;
      dst += MOUSE_W-x;
    }
  } else if(texturemode == TA_TEXTUREMODE_RGB565 &&
	    _ms_keycolor<=0xffff) {
    /* Special handling when doing colorkey on RGB565; we need
       to convert to ARGB1555 to get an alpha channel... */
    texturemode = TA_TEXTUREMODE_ARGB1555;
    unsigned short *bufs = (unsigned short *)buf;
    for (int y=0; y<h; y++) {
      int x;
      for (x=0; x<w; x++)
	if (*bufs == _ms_keycolor) {
	  *dst++ = 0;
	  bufs++;
	} else {
	  unsigned short p = *bufs++;
	  *dst++ = (p&0x1f)|((p&0xffc0)>>1)|0x8000;
	}
      dst += MOUSE_W-x;
    }
  } else {
    unsigned short *bufs = (unsigned short *)buf;
    for (int y=0; y<h; y++) {
      int x;
      for (x=0; x<w; x++)
	if (*bufs == _ms_keycolor) {
	  *dst++ = 0;
	  bufs++;
	} else
	  *dst++ = *bufs++;
      dst += MOUSE_W-x;
    }
  }

  mypoly.cmd =
    TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_TRANSPARENT|TA_CMD_POLYGON_SUBLIST|
    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR|TA_CMD_POLYGON_TEXTURED;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  mypoly.mode2 =
    TA_POLYMODE2_BLEND_SRC_ALPHA|TA_POLYMODE2_BLEND_DST_INVALPHA|
    TA_POLYMODE2_FOG_DISABLED|TA_POLYMODE2_TEXTURE_REPLACE|
    TA_POLYMODE2_U_SIZE_128|TA_POLYMODE2_V_SIZE_128;
  mypoly.texture = texturemode|TA_TEXTUREMODE_NON_TWIDDLED|
    TA_TEXTUREMODE_ADDRESS(mouse_tx[_mouse_buffer]);

  mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0;

  ta_commit_list(&mypoly);

  myvertex.cmd = TA_CMD_VERTEX;
  myvertex.ocolour = 0;
  myvertex.colour = 0xffff00;
  myvertex.z = 0.25;
  myvertex.u = 0.0;
  myvertex.v = 0.0;

  myvertex.x = (xdraw-_ms_hotspot_x)*_xscale;
  myvertex.y = (ydraw-_ms_hotspot_y)*_yscale + TOP_OFFSET;
  ta_commit_list(&myvertex);

  myvertex.x += w*_xscale;
  myvertex.u = w*(1.0/MOUSE_W);
  ta_commit_list(&myvertex);

  myvertex.x -= w*_xscale;
  myvertex.y += h*_yscale;
  myvertex.u = 0.0;
  myvertex.v = h*(1.0/MOUSE_H);
  ta_commit_list(&myvertex);

  myvertex.x += w*_xscale;
  myvertex.u = w*(1.0/MOUSE_W);
  myvertex.cmd |= TA_CMD_VERTEX_EOS;
  ta_commit_list(&myvertex);
}

void OSystem_Dreamcast::mouseToSoftKbd(int x, int y, int &rx, int &ry) const
{
  if (_softkbd_motion) {
    rx = (int)(x*_xscale - (330.0*sin(0.013*_softkbd_motion) - 320.0));
    ry = (int)(y*_yscale + TOP_OFFSET - 200.0);
  } else {
    rx = -1;
    ry = -1;
  }
}


void OSystem_Dreamcast::showOverlay()
{
  _overlay_visible = true;
  clearOverlay();
}

void OSystem_Dreamcast::hideOverlay()
{
  _overlay_visible = false;
}

void OSystem_Dreamcast::clearOverlay()
{
  if (!_overlay_visible)
    return;

  memset(overlay, 0, OVL_W*OVL_H*sizeof(unsigned short));

  _overlay_dirty = true;
}

void OSystem_Dreamcast::grabOverlay(void *buf, int pitch)
{
  int h = OVL_H;
  unsigned short *src = overlay;
  unsigned char *dst = (unsigned char *)buf;
  do {
    memcpy(dst, src, OVL_W*sizeof(int16));
    src += OVL_W;
    dst += pitch;
  } while (--h);
}

void OSystem_Dreamcast::copyRectToOverlay(const void *buf, int pitch,
					  int x, int y, int w, int h)
{
  if (w<1 || h<1)
    return;
  unsigned short *dst = overlay + y*OVL_W + x;
  const unsigned char *src = (const unsigned char *)buf;
  do {
    memcpy(dst, src, w*sizeof(int16));
    dst += OVL_W;
    src += pitch;
  } while (--h);
  _overlay_dirty = true;
}


static const OSystem::GraphicsMode gfxmodes[] = {
  { "default", "640x480 16bpp", 0 },
  { NULL, NULL, 0 }
};

const OSystem::GraphicsMode *OSystem_Dreamcast::getSupportedGraphicsModes() const
{
  return gfxmodes;
}

int OSystem_Dreamcast::getDefaultGraphicsMode() const
{
  return 0;
}

bool OSystem_Dreamcast::setGraphicsMode(int mode)
{
  return mode == 0;
}

int OSystem_Dreamcast::getGraphicsMode() const
{
  return 0;
}

Graphics::Surface *OSystem_Dreamcast::lockScreen()
{
  if (!screen)
    return 0;

  _framebuffer.pixels = screen;
  _framebuffer.w = _screen_w;
  _framebuffer.h = _screen_h;
  _framebuffer.pitch = SCREEN_W*2;
  _framebuffer.format = screenFormats[_screenFormat];

  return &_framebuffer;
}

void OSystem_Dreamcast::unlockScreen()
{
  // Force screen update
  _screen_dirty = true;
}

int16 OSystem_Dreamcast::getOverlayHeight()
{
  return OVL_H;
}

int16 OSystem_Dreamcast::getOverlayWidth()
{
  return OVL_W;
}
