/**********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2007  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),
                             zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com)
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti


  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley,
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001-2006    byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight,

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound DSP emulator code is derived from SNEeSe and OpenSPC:
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2007  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
**********************************************************************************/




#include "snes9x.h"
#include "memmap.h"
#include <string.h>



/*
Due recognition and credit are given on Overload's DSP website.
Thank those contributors for their hard work on this chip.


Fixed-point math reminder:

[sign, integer, fraction]
1.15.00 * 1.15.00 = 2.30.00 -> 1.30.00 (DSP) -> 1.31.00 (LSB is '0')
1.15.00 * 1.00.15 = 2.15.15 -> 1.15.15 (DSP) -> 1.15.16 (LSB is '0')
*/

typedef struct
{
  bool8 waiting4command;
  bool8 half_command;
  uint16 command;
  uint32 in_count;
  uint32 in_index;
  uint32 out_count;
  uint32 out_index;
  uint8 parameters[512];
  uint8 output[512];
} SDSP4;

SDSP4 DSP4;

//Todo: get all of this into a struct for easy save stating

// op control
int8 DSP4_Logic;            // controls op flow


// projection format
int16 lcv;                  // loop-control variable
int16 distance;             // z-position into virtual world
int16 raster;               // current raster line
int16 segments;             // number of raster lines drawn

// 1.15.16 or 1.15.0 [sign, integer, fraction]
int32 world_x;              // line of x-projection in world
int32 world_y;              // line of y-projection in world
int32 world_dx;             // projection line x-delta
int32 world_dy;             // projection line y-delta
int16 world_ddx;            // x-delta increment
int16 world_ddy;            // y-delta increment
int32 world_xenv;           // world x-shaping factor
int16 world_yofs;           // world y-vertical scroll

int16 view_x1;              // current viewer-x
int16 view_y1;              // current viewer-y
int16 view_x2;              // future viewer-x
int16 view_y2;              // future viewer-y
int16 view_dx;              // view x-delta factor
int16 view_dy;              // view y-delta factor
int16 view_xofs1;           // current viewer x-vertical scroll
int16 view_yofs1;           // current viewer y-vertical scroll
int16 view_xofs2;           // future viewer x-vertical scroll
int16 view_yofs2;           // future viewer y-vertical scroll
int16 view_yofsenv;         // y-scroll shaping factor
int16 view_turnoff_x;       // road turnoff data
int16 view_turnoff_dx;      // road turnoff delta factor


// drawing area

int16 viewport_cx;          // x-center of viewport window
int16 viewport_cy;          // y-center of render window
int16 viewport_left;        // x-left of viewport
int16 viewport_right;       // x-right of viewport
int16 viewport_top;         // y-top of viewport
int16 viewport_bottom;      // y-bottom of viewport


// sprite structure

int16 sprite_x;             // projected x-pos of sprite
int16 sprite_y;             // projected y-pos of sprite
int16 sprite_attr;          // obj attributes
bool8 sprite_size;          // sprite size: 8x8 or 16x16
int16 sprite_clipy;         // visible line to clip pixels off
int16 sprite_count;


// generic projection variables designed for
// two solid polygons + two polygon sides

int16 poly_clipLf[2][2];    // left clip boundary
int16 poly_clipRt[2][2];    // right clip boundary
int16 poly_ptr[2][2];       // HDMA structure pointers
int16 poly_raster[2][2];    // current raster line below horizon
int16 poly_top[2][2];       // top clip boundary
int16 poly_bottom[2][2];    // bottom clip boundary
int16 poly_cx[2][2];        // center for left/right points
int16 poly_start[2];        // current projection points
int16 poly_plane[2];        // previous z-plane distance


// OAM
int16 OAM_attr[16];         // OAM (size,MSB) data
int16 OAM_index;            // index into OAM table
int16 OAM_bits;             // offset into OAM table

int16 OAM_RowMax;           // maximum number of tiles per 8 aligned pixels (row)
int16 OAM_Row[32];          // current number of tiles per row

//////////////////////////////////////////////////////////////

// input protocol

static int16 DSP4_READ_WORD()
{
  int16 out;

  out = READ_WORD(DSP4.parameters + DSP4.in_index);
  DSP4.in_index += 2;

  return out;
}

static int32 DSP4_READ_DWORD()
{
  int32 out;

  out = READ_DWORD(DSP4.parameters + DSP4.in_index);
  DSP4.in_index += 4;

  return out;
}


//////////////////////////////////////////////////////////////

// output protocol

#define DSP4_CLEAR_OUT() \
{ DSP4.out_count = 0; DSP4.out_index = 0; }

#define DSP4_WRITE_BYTE( d ) \
{ WRITE_WORD( DSP4.output + DSP4.out_count, ( d ) ); DSP4.out_count++; }

#define DSP4_WRITE_WORD( d ) \
{ WRITE_WORD( DSP4.output + DSP4.out_count, ( d ) ); DSP4.out_count += 2; }

#ifndef MSB_FIRST
#define DSP4_WRITE_16_WORD( d ) \
{ memcpy(DSP4.output + DSP4.out_count, ( d ), 32); DSP4.out_count += 32; }
#else
#define DSP4_WRITE_16_WORD( d )                         \
{ for (int p = 0; p < 16; p++) DSP4_WRITE_WORD((d)[p]); }
#endif

#ifdef PRINT_OP
#define DSP4_WRITE_DEBUG( x, d ) \
  WRITE_WORD( nop + x, d );
#endif

#ifdef DEBUG_DSP
#define DSP4_WRITE_DEBUG( x, d ) \
  WRITE_WORD( nop + x, d );
#endif

//////////////////////////////////////////////////////////////

// used to wait for dsp i/o

#define DSP4_WAIT( x ) \
  DSP4.in_index = 0; DSP4_Logic = x; return;

//////////////////////////////////////////////////////////////

// 1.7.8 -> 1.15.16
#define SEX78( a ) ( ( (int32) ( (int16) (a) ) ) << 8 )

// 1.15.0 -> 1.15.16
#define SEX16( a ) ( ( (int32) ( (int16) (a) ) ) << 16 )

#ifdef PRINT_OP
#define U16( a ) ( (uint16) ( a ) )
#endif

#ifdef DEBUG_DSP
#define U16( a ) ( (uint16) ( a ) )
#endif

//////////////////////////////////////////////////////////////

// Attention: This lookup table is not verified
const uint16 div_lut[64] = { 0x0000, 0x8000, 0x4000, 0x2aaa, 0x2000, 0x1999, 0x1555, 0x1249, 0x1000, 0x0e38,
                             0x0ccc, 0x0ba2, 0x0aaa, 0x09d8, 0x0924, 0x0888, 0x0800, 0x0787, 0x071c, 0x06bc,
                             0x0666, 0x0618, 0x05d1, 0x0590, 0x0555, 0x051e, 0x04ec, 0x04bd, 0x0492, 0x0469,
                             0x0444, 0x0421, 0x0400, 0x03e0, 0x03c3, 0x03a8, 0x038e, 0x0375, 0x035e, 0x0348,
                             0x0333, 0x031f, 0x030c, 0x02fa, 0x02e8, 0x02d8, 0x02c8, 0x02b9, 0x02aa, 0x029c,
                             0x028f, 0x0282, 0x0276, 0x026a, 0x025e, 0x0253, 0x0249, 0x023e, 0x0234, 0x022b,
                             0x0222, 0x0219, 0x0210, 0x0208,  };

int16 DSP4_Inverse(int16 value)
{
  // saturate bounds
  if (value < 0)
  {
    value = 0;
  }
  if (value > 63)
  {
    value = 63;
  }

  return div_lut[value];
}

//////////////////////////////////////////////////////////////

// Prototype
void DSP4_OP0B(bool8 *draw, int16 sp_x, int16 sp_y, int16 sp_attr, bool8 size, bool8 stop);

//////////////////////////////////////////////////////////////

// OP00
void DSP4_Multiply(int16 Multiplicand, int16 Multiplier, int32 *Product)
{
  *Product = (Multiplicand * Multiplier << 1) >> 1;
}

//////////////////////////////////////////////////////////////


void DSP4_OP01()
{
  DSP4.waiting4command = FALSE;

  // op flow control
  switch (DSP4_Logic)
  {
    case 1:
      goto resume1; break;
    case 2:
      goto resume2; break;
    case 3:
      goto resume3; break;
  }

  ////////////////////////////////////////////////////
  // process initial inputs

  // sort inputs
  world_y = DSP4_READ_DWORD();
  poly_bottom[0][0] = DSP4_READ_WORD();
  poly_top[0][0] = DSP4_READ_WORD();
  poly_cx[1][0] = DSP4_READ_WORD();
  viewport_bottom = DSP4_READ_WORD();
  world_x = DSP4_READ_DWORD();
  poly_cx[0][0] = DSP4_READ_WORD();
  poly_ptr[0][0] = DSP4_READ_WORD();
  world_yofs = DSP4_READ_WORD();
  world_dy = DSP4_READ_DWORD();
  world_dx = DSP4_READ_DWORD();
  distance = DSP4_READ_WORD();
  DSP4_READ_WORD(); // 0x0000
  world_xenv = DSP4_READ_DWORD();
  world_ddy = DSP4_READ_WORD();
  world_ddx = DSP4_READ_WORD();
  view_yofsenv = DSP4_READ_WORD();

  // initial (x,y,offset) at starting raster line
  view_x1 = (world_x + world_xenv) >> 16;
  view_y1 = world_y >> 16;
  view_xofs1 = world_x >> 16;
  view_yofs1 = world_yofs;
  view_turnoff_x = 0;
  view_turnoff_dx = 0;

  // first raster line
  poly_raster[0][0] = poly_bottom[0][0];

  do
  {
    ////////////////////////////////////////////////////
    // process one iteration of projection

    // perspective projection of world (x,y,scroll) points
    // based on the current projection lines
    view_x2 = ( ( ( world_x + world_xenv ) >> 16 ) * distance >> 15 ) + ( view_turnoff_x * distance >> 15 );
    view_y2 = (world_y >> 16) * distance >> 15;
    view_xofs2 = view_x2;
    view_yofs2 = (world_yofs * distance >> 15) + poly_bottom[0][0] - view_y2;


    // 1. World x-location before transformation
    // 2. Viewer x-position at the next
    // 3. World y-location before perspective projection
    // 4. Viewer y-position below the horizon
    // 5. Number of raster lines drawn in this iteration

    DSP4_CLEAR_OUT();
    DSP4_WRITE_WORD((world_x + world_xenv) >> 16);
    DSP4_WRITE_WORD(view_x2);
    DSP4_WRITE_WORD(world_y >> 16);
    DSP4_WRITE_WORD(view_y2);

    //////////////////////////////////////////////////////

    // SR = 0x00

    // determine # of raster lines used
    segments = poly_raster[0][0] - view_y2;

    // prevent overdraw
    if (view_y2 >= poly_raster[0][0])
      segments = 0;
    else
      poly_raster[0][0] = view_y2;

    // don't draw outside the window
    if (view_y2 < poly_top[0][0])
    {
      segments = 0;

      // flush remaining raster lines
      if (view_y1 >= poly_top[0][0])
        segments = view_y1 - poly_top[0][0];
    }

    // SR = 0x80

    DSP4_WRITE_WORD(segments);

    //////////////////////////////////////////////////////

    // scan next command if no SR check needed
    if (segments)
    {
      int32 px_dx, py_dy;
      int32 x_scroll, y_scroll;

      // SR = 0x00

      // linear interpolation (lerp) between projected points
      px_dx = (view_xofs2 - view_xofs1) * DSP4_Inverse(segments) << 1;
      py_dy = (view_yofs2 - view_yofs1) * DSP4_Inverse(segments) << 1;

      // starting step values
      x_scroll = SEX16(poly_cx[0][0] + view_xofs1);
      y_scroll = SEX16(-viewport_bottom + view_yofs1 + view_yofsenv + poly_cx[1][0] - world_yofs);

      // SR = 0x80

      // rasterize line
      for (lcv = 0; lcv < segments; lcv++)
      {
        // 1. HDMA memory pointer (bg1)
        // 2. vertical scroll offset ($210E)
        // 3. horizontal scroll offset ($210D)

        DSP4_WRITE_WORD(poly_ptr[0][0]);
        DSP4_WRITE_WORD((y_scroll + 0x8000) >> 16);
        DSP4_WRITE_WORD((x_scroll + 0x8000) >> 16);


        // update memory address
        poly_ptr[0][0] -= 4;

        // update screen values
        x_scroll += px_dx;
        y_scroll += py_dy;
      }
    }

    ////////////////////////////////////////////////////
    // Post-update

    // update new viewer (x,y,scroll) to last raster line drawn
    view_x1 = view_x2;
    view_y1 = view_y2;
    view_xofs1 = view_xofs2;
    view_yofs1 = view_yofs2;

    // add deltas for projection lines
    world_dx += SEX78(world_ddx);
    world_dy += SEX78(world_ddy);

    // update projection lines
    world_x += (world_dx + world_xenv);
    world_y += world_dy;

    // update road turnoff position
    view_turnoff_x += view_turnoff_dx;

    ////////////////////////////////////////////////////
    // command check

    // scan next command
    DSP4.in_count = 2;
    DSP4_WAIT(1) resume1 :

    // check for termination
    distance = DSP4_READ_WORD();
    if (distance == -0x8000)
      break;

    // road turnoff
    if( (uint16) distance == 0x8001 )
    {
      DSP4.in_count = 6;
      DSP4_WAIT(2) resume2:

      distance = DSP4_READ_WORD();
      view_turnoff_x = DSP4_READ_WORD();
      view_turnoff_dx = DSP4_READ_WORD();

      // factor in new changes
      view_x1 += ( view_turnoff_x * distance >> 15 );
      view_xofs1 += ( view_turnoff_x * distance >> 15 );

      // update stepping values
      view_turnoff_x += view_turnoff_dx;

      DSP4.in_count = 2;
      DSP4_WAIT(1)
    }

    // already have 2 bytes read
    DSP4.in_count = 6;
    DSP4_WAIT(3) resume3 :

    // inspect inputs
    world_ddy = DSP4_READ_WORD();
    world_ddx = DSP4_READ_WORD();
    view_yofsenv = DSP4_READ_WORD();

    // no envelope here
    world_xenv = 0;
  }
  while (1);

  // terminate op
  DSP4.waiting4command = TRUE;
}

//////////////////////////////////////////////////////////////


void DSP4_OP03()
{
  OAM_RowMax = 33;
  memset(OAM_Row, 0, 64);
}


//////////////////////////////////////////////////////////////


void DSP4_OP05()
{
  OAM_index = 0;
  OAM_bits = 0;
  memset(OAM_attr, 0, 32);
  sprite_count = 0;
}


//////////////////////////////////////////////////////////////

void DSP4_OP06()
{
  DSP4_CLEAR_OUT();
  DSP4_WRITE_16_WORD(OAM_attr);
}

//////////////////////////////////////////////////////////////


void DSP4_OP07()
{
  DSP4.waiting4command = FALSE;

  // op flow control
  switch (DSP4_Logic)
  {
    case 1:
      goto resume1; break;
    case 2:
      goto resume2; break;
  }

  ////////////////////////////////////////////////////
  // sort inputs

  world_y = DSP4_READ_DWORD();
  poly_bottom[0][0] = DSP4_READ_WORD();
  poly_top[0][0] = DSP4_READ_WORD();
  poly_cx[1][0] = DSP4_READ_WORD();
  viewport_bottom = DSP4_READ_WORD();
  world_x = DSP4_READ_DWORD();
  poly_cx[0][0] = DSP4_READ_WORD();
  poly_ptr[0][0] = DSP4_READ_WORD();
  world_yofs = DSP4_READ_WORD();
  distance = DSP4_READ_WORD();
  view_y2 = DSP4_READ_WORD();
  view_dy = DSP4_READ_WORD() * distance >> 15;
  view_x2 = DSP4_READ_WORD();
  view_dx = DSP4_READ_WORD() * distance >> 15;
  view_yofsenv = DSP4_READ_WORD();

  // initial (x,y,offset) at starting raster line
  view_x1 = world_x >> 16;
  view_y1 = world_y >> 16;
  view_xofs1 = view_x1;
  view_yofs1 = world_yofs;

  // first raster line
  poly_raster[0][0] = poly_bottom[0][0];


  do
  {
    ////////////////////////////////////////////////////
    // process one iteration of projection

    // add shaping
    view_x2 += view_dx;
    view_y2 += view_dy;

    // vertical scroll calculation
    view_xofs2 = view_x2;
    view_yofs2 = (world_yofs * distance >> 15) + poly_bottom[0][0] - view_y2;

    // 1. Viewer x-position at the next
    // 2. Viewer y-position below the horizon
    // 3. Number of raster lines drawn in this iteration

    DSP4_CLEAR_OUT();
    DSP4_WRITE_WORD(view_x2);
    DSP4_WRITE_WORD(view_y2);

    //////////////////////////////////////////////////////

    // SR = 0x00

    // determine # of raster lines used
    segments = view_y1 - view_y2;

    // prevent overdraw
    if (view_y2 >= poly_raster[0][0])
      segments = 0;
    else
      poly_raster[0][0] = view_y2;

    // don't draw outside the window
    if (view_y2 < poly_top[0][0])
    {
      segments = 0;

      // flush remaining raster lines
      if (view_y1 >= poly_top[0][0])
        segments = view_y1 - poly_top[0][0];
    }

    // SR = 0x80

    DSP4_WRITE_WORD(segments);

    //////////////////////////////////////////////////////

    // scan next command if no SR check needed
    if (segments)
    {
      int32 px_dx, py_dy;
      int32 x_scroll, y_scroll;

      // SR = 0x00

      // linear interpolation (lerp) between projected points
      px_dx = (view_xofs2 - view_xofs1) * DSP4_Inverse(segments) << 1;
      py_dy = (view_yofs2 - view_yofs1) * DSP4_Inverse(segments) << 1;

      // starting step values
      x_scroll = SEX16(poly_cx[0][0] + view_xofs1);
      y_scroll = SEX16(-viewport_bottom + view_yofs1 + view_yofsenv + poly_cx[1][0] - world_yofs);

      // SR = 0x80

      // rasterize line
      for (lcv = 0; lcv < segments; lcv++)
      {
        // 1. HDMA memory pointer (bg2)
        // 2. vertical scroll offset ($2110)
        // 3. horizontal scroll offset ($210F)

        DSP4_WRITE_WORD(poly_ptr[0][0]);
        DSP4_WRITE_WORD((y_scroll + 0x8000) >> 16);
        DSP4_WRITE_WORD((x_scroll + 0x8000) >> 16);

        // update memory address
        poly_ptr[0][0] -= 4;

        // update screen values
        x_scroll += px_dx;
        y_scroll += py_dy;
      }
    }

    /////////////////////////////////////////////////////
    // Post-update

    // update new viewer (x,y,scroll) to last raster line drawn
    view_x1 = view_x2;
    view_y1 = view_y2;
    view_xofs1 = view_xofs2;
    view_yofs1 = view_yofs2;

    ////////////////////////////////////////////////////
    // command check

    // scan next command
    DSP4.in_count = 2;
    DSP4_WAIT(1) resume1 :

    // check for opcode termination
    distance = DSP4_READ_WORD();
    if (distance == -0x8000)
      break;

    // already have 2 bytes in queue
    DSP4.in_count = 10;
    DSP4_WAIT(2) resume2 :

    // inspect inputs
    view_y2 = DSP4_READ_WORD();
    view_dy = DSP4_READ_WORD() * distance >> 15;
    view_x2 = DSP4_READ_WORD();
    view_dx = DSP4_READ_WORD() * distance >> 15;
    view_yofsenv = DSP4_READ_WORD();
  }
  while (1);

  DSP4.waiting4command = TRUE;
}

//////////////////////////////////////////////////////////////

void DSP4_OP08()
{
  int16 win_left, win_right;
  int16 view_x[2], view_y[2];
  int16 envelope[2][2];

  DSP4.waiting4command = FALSE;

  // op flow control
  switch (DSP4_Logic)
  {
    case 1:
      goto resume1; break;
    case 2:
      goto resume2; break;
  }

  ////////////////////////////////////////////////////
  // process initial inputs for two polygons

  // clip values
  poly_clipRt[0][0] = DSP4_READ_WORD();
  poly_clipRt[0][1] = DSP4_READ_WORD();
  poly_clipRt[1][0] = DSP4_READ_WORD();
  poly_clipRt[1][1] = DSP4_READ_WORD();

  poly_clipLf[0][0] = DSP4_READ_WORD();
  poly_clipLf[0][1] = DSP4_READ_WORD();
  poly_clipLf[1][0] = DSP4_READ_WORD();
  poly_clipLf[1][1] = DSP4_READ_WORD();

  // unknown (constant) (ex. 1P/2P = $00A6, $00A6, $00A6, $00A6)
  DSP4_READ_WORD();
  DSP4_READ_WORD();
  DSP4_READ_WORD();
  DSP4_READ_WORD();

  // unknown (constant) (ex. 1P/2P = $00A5, $00A5, $00A7, $00A7)
  DSP4_READ_WORD();
  DSP4_READ_WORD();
  DSP4_READ_WORD();
  DSP4_READ_WORD();

  // polygon centering (left,right)
  poly_cx[0][0] = DSP4_READ_WORD();
  poly_cx[0][1] = DSP4_READ_WORD();
  poly_cx[1][0] = DSP4_READ_WORD();
  poly_cx[1][1] = DSP4_READ_WORD();

  // HDMA pointer locations
  poly_ptr[0][0] = DSP4_READ_WORD();
  poly_ptr[0][1] = DSP4_READ_WORD();
  poly_ptr[1][0] = DSP4_READ_WORD();
  poly_ptr[1][1] = DSP4_READ_WORD();

  // starting raster line below the horizon
  poly_bottom[0][0] = DSP4_READ_WORD();
  poly_bottom[0][1] = DSP4_READ_WORD();
  poly_bottom[1][0] = DSP4_READ_WORD();
  poly_bottom[1][1] = DSP4_READ_WORD();

  // top boundary line to clip
  poly_top[0][0] = DSP4_READ_WORD();
  poly_top[0][1] = DSP4_READ_WORD();
  poly_top[1][0] = DSP4_READ_WORD();
  poly_top[1][1] = DSP4_READ_WORD();

  // unknown
  // (ex. 1P = $2FC8, $0034, $FF5C, $0035)
  //
  // (ex. 2P = $3178, $0034, $FFCC, $0035)
  // (ex. 2P = $2FC8, $0034, $FFCC, $0035)

  DSP4_READ_WORD();
  DSP4_READ_WORD();
  DSP4_READ_WORD();
  DSP4_READ_WORD();

  // look at guidelines for both polygon shapes
  distance = DSP4_READ_WORD();
  view_x[0] = DSP4_READ_WORD();
  view_y[0] = DSP4_READ_WORD();
  view_x[1] = DSP4_READ_WORD();
  view_y[1] = DSP4_READ_WORD();

  // envelope shaping guidelines (one frame only)
  envelope[0][0] = DSP4_READ_WORD();
  envelope[0][1] = DSP4_READ_WORD();
  envelope[1][0] = DSP4_READ_WORD();
  envelope[1][1] = DSP4_READ_WORD();

  // starting base values to project from
  poly_start[0] = view_x[0];
  poly_start[1] = view_x[1];

  // starting raster lines to begin drawing
  poly_raster[0][0] = view_y[0];
  poly_raster[0][1] = view_y[0];
  poly_raster[1][0] = view_y[1];
  poly_raster[1][1] = view_y[1];

  // starting distances
  poly_plane[0] = distance;
  poly_plane[1] = distance;

  // SR = 0x00

  // re-center coordinates
  win_left = poly_cx[0][0] - view_x[0] + envelope[0][0];
  win_right = poly_cx[0][1] - view_x[0] + envelope[0][1];

  // saturate offscreen data for polygon #1
  if (win_left < poly_clipLf[0][0])
  {
    win_left = poly_clipLf[0][0];
  }
  if (win_left > poly_clipRt[0][0])
  {
    win_left = poly_clipRt[0][0];
  }
  if (win_right < poly_clipLf[0][1])
  {
    win_right = poly_clipLf[0][1];
  }
  if (win_right > poly_clipRt[0][1])
  {
    win_right = poly_clipRt[0][1];
  }

  // SR = 0x80

  // initial output for polygon #1
  DSP4_CLEAR_OUT();
  DSP4_WRITE_BYTE(win_left & 0xff);
  DSP4_WRITE_BYTE(win_right & 0xff);


  do
  {
    int16 polygon;
    ////////////////////////////////////////////////////
    // command check

    // scan next command
    DSP4.in_count = 2;
    DSP4_WAIT(1) resume1 :

    // terminate op
    distance = DSP4_READ_WORD();
    if (distance == -0x8000)
      break;

    // already have 2 bytes in queue
    DSP4.in_count = 16;

    DSP4_WAIT(2) resume2 :

    // look at guidelines for both polygon shapes
    view_x[0] = DSP4_READ_WORD();
    view_y[0] = DSP4_READ_WORD();
    view_x[1] = DSP4_READ_WORD();
    view_y[1] = DSP4_READ_WORD();

    // envelope shaping guidelines (one frame only)
    envelope[0][0] = DSP4_READ_WORD();
    envelope[0][1] = DSP4_READ_WORD();
    envelope[1][0] = DSP4_READ_WORD();
    envelope[1][1] = DSP4_READ_WORD();

    ////////////////////////////////////////////////////
    // projection begins

    // init
    DSP4_CLEAR_OUT();


    //////////////////////////////////////////////
    // solid polygon renderer - 2 shapes

    for (polygon = 0; polygon < 2; polygon++)
    {
      int32 left_inc, right_inc;
      int16 x1_final, x2_final;
      int16 env[2][2];
      int16 poly;

      // SR = 0x00

      // # raster lines to draw
      segments = poly_raster[polygon][0] - view_y[polygon];

      // prevent overdraw
      if (segments > 0)
      {
        // bump drawing cursor
        poly_raster[polygon][0] = view_y[polygon];
        poly_raster[polygon][1] = view_y[polygon];
      }
      else
        segments = 0;

      // don't draw outside the window
      if (view_y[polygon] < poly_top[polygon][0])
      {
        segments = 0;

        // flush remaining raster lines
        if (view_y[polygon] >= poly_top[polygon][0])
          segments = view_y[polygon] - poly_top[polygon][0];
      }

      // SR = 0x80

      // tell user how many raster structures to read in
      DSP4_WRITE_WORD(segments);

      // normal parameters
      poly = polygon;

      /////////////////////////////////////////////////////

      // scan next command if no SR check needed
      if (segments)
      {
        int32 win_left, win_right;

        // road turnoff selection
        if( (uint16) envelope[ polygon ][ 0 ] == (uint16) 0xc001 )
          poly = 1;
        else if( envelope[ polygon ][ 1 ] == 0x3fff )
          poly = 1;

        ///////////////////////////////////////////////
        // left side of polygon

        // perspective correction on additional shaping parameters
        env[0][0] = envelope[polygon][0] * poly_plane[poly] >> 15;
        env[0][1] = envelope[polygon][0] * distance >> 15;

        // project new shapes (left side)
        x1_final = view_x[poly] + env[0][0];
        x2_final = poly_start[poly] + env[0][1];

        // interpolate between projected points with shaping
        left_inc = (x2_final - x1_final) * DSP4_Inverse(segments) << 1;
        if (segments == 1)
          left_inc = -left_inc;

        ///////////////////////////////////////////////
        // right side of polygon

        // perspective correction on additional shaping parameters
        env[1][0] = envelope[polygon][1] * poly_plane[poly] >> 15;;
        env[1][1] = envelope[polygon][1] * distance >> 15;

        // project new shapes (right side)
        x1_final = view_x[poly] + env[1][0];
        x2_final = poly_start[poly] + env[1][1];


        // interpolate between projected points with shaping
        right_inc = (x2_final - x1_final) * DSP4_Inverse(segments) << 1;
        if (segments == 1)
          right_inc = -right_inc;

        ///////////////////////////////////////////////
        // update each point on the line

        win_left = SEX16(poly_cx[polygon][0] - poly_start[poly] + env[0][0]);
        win_right = SEX16(poly_cx[polygon][1] - poly_start[poly] + env[1][0]);

        // update distance drawn into world
        poly_plane[polygon] = distance;

        // rasterize line
        for (lcv = 0; lcv < segments; lcv++)
        {
          int16 x_left, x_right;

          // project new coordinates
          win_left += left_inc;
          win_right += right_inc;

          // grab integer portion, drop fraction (no rounding)
          x_left = win_left >> 16;
          x_right = win_right >> 16;

          // saturate offscreen data
          if (x_left < poly_clipLf[polygon][0])
            x_left = poly_clipLf[polygon][0];
          if (x_left > poly_clipRt[polygon][0])
            x_left = poly_clipRt[polygon][0];
          if (x_right < poly_clipLf[polygon][1])
            x_right = poly_clipLf[polygon][1];
          if (x_right > poly_clipRt[polygon][1])
            x_right = poly_clipRt[polygon][1];

          // 1. HDMA memory pointer
          // 2. Left window position ($2126/$2128)
          // 3. Right window position ($2127/$2129)

          DSP4_WRITE_WORD(poly_ptr[polygon][0]);
          DSP4_WRITE_BYTE(x_left & 0xff);
          DSP4_WRITE_BYTE(x_right & 0xff);


          // update memory pointers
          poly_ptr[polygon][0] -= 4;
          poly_ptr[polygon][1] -= 4;
        } // end rasterize line
      }

      ////////////////////////////////////////////////
      // Post-update

      // new projection spot to continue rasterizing from
      poly_start[polygon] = view_x[poly];
    } // end polygon rasterizer
  }
  while (1);

  // unknown output
  DSP4_CLEAR_OUT();
  DSP4_WRITE_WORD(0);


  DSP4.waiting4command = TRUE;
}

//////////////////////////////////////////////////////////////

void DSP4_OP09()
{
  DSP4.waiting4command = FALSE;

  // op flow control
  switch (DSP4_Logic)
  {
    case 1:
      goto resume1; break;
    case 2:
      goto resume2; break;
    case 3:
      goto resume3; break;
    case 4:
      goto resume4; break;
    case 5:
      goto resume5; break;
    case 6:
      goto resume6; break;
  }

  ////////////////////////////////////////////////////
  // process initial inputs

  // grab screen information
  viewport_cx = DSP4_READ_WORD();
  viewport_cy = DSP4_READ_WORD();
  DSP4_READ_WORD(); // 0x0000
  viewport_left = DSP4_READ_WORD();
  viewport_right = DSP4_READ_WORD();
  viewport_top = DSP4_READ_WORD();
  viewport_bottom = DSP4_READ_WORD();

  // starting raster line below the horizon
  poly_bottom[0][0] = viewport_bottom - viewport_cy;
  poly_raster[0][0] = 0x100;

  do
  {
    ////////////////////////////////////////////////////
    // check for new sprites

    DSP4.in_count = 4;
    DSP4_WAIT(1) resume1 :

    ////////////////////////////////////////////////
    // raster overdraw check

    raster = DSP4_READ_WORD();

    // continue updating the raster line where overdraw begins
    if (raster < poly_raster[0][0])
    {
      sprite_clipy = viewport_bottom - (poly_bottom[0][0] - raster);
      poly_raster[0][0] = raster;
    }

    /////////////////////////////////////////////////
    // identify sprite

    // op termination
    distance = DSP4_READ_WORD();
    if (distance == -0x8000)
      goto terminate;


    // no sprite
    if (distance == 0x0000)
    {
      continue;
    }

    ////////////////////////////////////////////////////
    // process projection information

    // vehicle sprite
    if ((uint16) distance == 0x9000)
    {
      int16 car_left, car_right, car_back;
      int16 impact_left, impact_back;
      int16 world_spx, world_spy;
      int16 view_spx, view_spy;
      uint16 energy;

      // we already have 4 bytes we want
      DSP4.in_count = 14;
      DSP4_WAIT(2) resume2 :

      // filter inputs
      energy = DSP4_READ_WORD();
      impact_back = DSP4_READ_WORD();
      car_back = DSP4_READ_WORD();
      impact_left = DSP4_READ_WORD();
      car_left = DSP4_READ_WORD();
      distance = DSP4_READ_WORD();
      car_right = DSP4_READ_WORD();

      // calculate car's world (x,y) values
      world_spx = car_right - car_left;
      world_spy = car_back;

      // add in collision vector [needs bit-twiddling]
      world_spx -= energy * (impact_left - car_left) >> 16;
      world_spy -= energy * (car_back - impact_back) >> 16;

      // perspective correction for world (x,y)
      view_spx = world_spx * distance >> 15;
      view_spy = world_spy * distance >> 15;

      // convert to screen values
      sprite_x = viewport_cx + view_spx;
      sprite_y = viewport_bottom - (poly_bottom[0][0] - view_spy);

      // make the car's (x)-coordinate available
      DSP4_CLEAR_OUT();
      DSP4_WRITE_WORD(world_spx);

      // grab a few remaining vehicle values
      DSP4.in_count = 4;
      DSP4_WAIT(3) resume3 :

      // add vertical lift factor
      sprite_y += DSP4_READ_WORD();
    }
    // terrain sprite
    else
    {
      int16 world_spx, world_spy;
      int16 view_spx, view_spy;

      // we already have 4 bytes we want
      DSP4.in_count = 10;
      DSP4_WAIT(4) resume4 :

      // sort loop inputs
      poly_cx[0][0] = DSP4_READ_WORD();
      poly_raster[0][1] = DSP4_READ_WORD();
      world_spx = DSP4_READ_WORD();
      world_spy = DSP4_READ_WORD();

      // compute base raster line from the bottom
      segments = poly_bottom[0][0] - raster;

      // perspective correction for world (x,y)
      view_spx = world_spx * distance >> 15;
      view_spy = world_spy * distance >> 15;

      // convert to screen values
      sprite_x = viewport_cx + view_spx - poly_cx[0][0];
      sprite_y = viewport_bottom - segments + view_spy;
    }

    // default sprite size: 16x16
    sprite_size = 1;
    sprite_attr = DSP4_READ_WORD();

    ////////////////////////////////////////////////////
    // convert tile data to SNES OAM format

    do
    {
      uint16 header;

      int16 sp_x, sp_y, sp_attr, sp_dattr;
      int16 sp_dx, sp_dy;
      int16 pixels;

      bool8 draw;

      DSP4.in_count = 2;
      DSP4_WAIT(5) resume5 :

      draw = TRUE;

      // opcode termination
      raster = DSP4_READ_WORD();
      if (raster == -0x8000)
        goto terminate;

      // stop code
      if (raster == 0x0000 && !sprite_size)
        break;

      // toggle sprite size
      if (raster == 0x0000)
      {
        sprite_size = !sprite_size;
        continue;
      }

      // check for valid sprite header
      header = raster;
      header >>= 8;
      if (header != 0x20 &&
          header != 0x2e && //This is for attractor sprite
          header != 0x40 &&
          header != 0x60 &&
          header != 0xa0 &&
          header != 0xc0 &&
          header != 0xe0)
        break;

      // read in rest of sprite data
      DSP4.in_count = 4;
      DSP4_WAIT(6) resume6 :

      draw = TRUE;

      /////////////////////////////////////
      // process tile data

      // sprite deltas
      sp_dattr = raster;
      sp_dy = DSP4_READ_WORD();
      sp_dx = DSP4_READ_WORD();

      // update coordinates to screen space
      sp_x = sprite_x + sp_dx;
      sp_y = sprite_y + sp_dy;

      // update sprite nametable/attribute information
      sp_attr = sprite_attr + sp_dattr;

      // allow partially visibile tiles
      pixels = sprite_size ? 15 : 7;

      DSP4_CLEAR_OUT();

      // transparent tile to clip off parts of a sprite (overdraw)
      if (sprite_clipy - pixels <= sp_y &&
          sp_y <= sprite_clipy &&
          sp_x >= viewport_left - pixels &&
          sp_x <= viewport_right &&
          sprite_clipy >= viewport_top - pixels &&
          sprite_clipy <= viewport_bottom)
      {
        DSP4_OP0B(&draw, sp_x, sprite_clipy, 0x00EE, sprite_size, 0);
      }


      // normal sprite tile
      if (sp_x >= viewport_left - pixels &&
          sp_x <= viewport_right &&
          sp_y >= viewport_top - pixels &&
          sp_y <= viewport_bottom &&
          sp_y <= sprite_clipy)
      {
        DSP4_OP0B(&draw, sp_x, sp_y, sp_attr, sprite_size, 0);
      }


      // no following OAM data
      DSP4_OP0B(&draw, 0, 0x0100, 0, 0, 1);
    }
    while (1);
  }
  while (1);

  terminate : DSP4.waiting4command = TRUE;
}

//////////////////////////////////////////////////////////////

const uint16 OP0A_Values[16] = { 0x0000, 0x0030, 0x0060, 0x0090, 0x00c0, 0x00f0, 0x0120, 0x0150, 0xfe80,
                                 0xfeb0, 0xfee0, 0xff10, 0xff40, 0xff70, 0xffa0, 0xffd0 };

void DSP4_OP0A(int16 n2, int16 *o1, int16 *o2, int16 *o3, int16 *o4)
{
  *o4 = OP0A_Values[(n2 & 0x000f)];
  *o3 = OP0A_Values[(n2 & 0x00f0) >> 4];
  *o2 = OP0A_Values[(n2 & 0x0f00) >> 8];
  *o1 = OP0A_Values[(n2 & 0xf000) >> 12];
}

//////////////////////////////////////////////////////////////

void DSP4_OP0B(bool8 *draw, int16 sp_x, int16 sp_y, int16 sp_attr, bool8 size, bool8 stop)
{
  int16 Row1, Row2;

  // SR = 0x00

  // align to nearest 8-pixel row
  Row1 = (sp_y >> 3) & 0x1f;
  Row2 = (Row1 + 1) & 0x1f;

  // check boundaries
  if (!((sp_y < 0) || ((sp_y & 0x01ff) < 0x00eb)))
  {
    *draw = 0;
  }
  if (size)
  {
    if (OAM_Row[Row1] + 1 >= OAM_RowMax)
      *draw = 0;
    if (OAM_Row[Row2] + 1 >= OAM_RowMax)
      *draw = 0;
  }
  else
  {
    if (OAM_Row[Row1] >= OAM_RowMax)
    {
      *draw = 0;
    }
  }

  // emulator fail-safe (unknown if this really exists)
  if (sprite_count >= 128)
  {
    *draw = 0;
  }

  // SR = 0x80

  if (*draw)
  {
    // Row tiles
    if (size)
    {
      OAM_Row[Row1] += 2;
      OAM_Row[Row2] += 2;
    }
    else
    {
      OAM_Row[Row1]++;
    }

    // yield OAM output
    DSP4_WRITE_WORD(1);

    // pack OAM data: x,y,name,attr
    DSP4_WRITE_BYTE(sp_x & 0xff);
    DSP4_WRITE_BYTE(sp_y & 0xff);
    DSP4_WRITE_WORD(sp_attr);

    sprite_count++;

    // OAM: size,msb data
    // save post-oam table data for future retrieval
    OAM_attr[OAM_index] |= ((sp_x <0 || sp_x> 255) << OAM_bits);
    OAM_bits++;

    OAM_attr[OAM_index] |= (size << OAM_bits);
    OAM_bits++;

    // move to next byte in buffer
    if (OAM_bits == 16)
    {
      OAM_bits = 0;
      OAM_index++;
    }
  }
  else if (stop)
  {
    // yield no OAM output
    DSP4_WRITE_WORD(0);
  }
}

//////////////////////////////////////////////////////////////

void DSP4_OP0D()
{
  DSP4.waiting4command = FALSE;

  // op flow control
  switch (DSP4_Logic)
  {
    case 1:
      goto resume1; break;
    case 2:
      goto resume2; break;
  }

  ////////////////////////////////////////////////////
  // process initial inputs

  // sort inputs
  world_y = DSP4_READ_DWORD();
  poly_bottom[0][0] = DSP4_READ_WORD();
  poly_top[0][0] = DSP4_READ_WORD();
  poly_cx[1][0] = DSP4_READ_WORD();
  viewport_bottom = DSP4_READ_WORD();
  world_x = DSP4_READ_DWORD();
  poly_cx[0][0] = DSP4_READ_WORD();
  poly_ptr[0][0] = DSP4_READ_WORD();
  world_yofs = DSP4_READ_WORD();
  world_dy = DSP4_READ_DWORD();
  world_dx = DSP4_READ_DWORD();
  distance = DSP4_READ_WORD();
  DSP4_READ_WORD(); // 0x0000
  world_xenv = SEX78(DSP4_READ_WORD());
  world_ddy = DSP4_READ_WORD();
  world_ddx = DSP4_READ_WORD();
  view_yofsenv = DSP4_READ_WORD();

  // initial (x,y,offset) at starting raster line
  view_x1 = (world_x + world_xenv) >> 16;
  view_y1 = world_y >> 16;
  view_xofs1 = world_x >> 16;
  view_yofs1 = world_yofs;

  // first raster line
  poly_raster[0][0] = poly_bottom[0][0];


  do
  {
    ////////////////////////////////////////////////////
    // process one iteration of projection

    // perspective projection of world (x,y,scroll) points
    // based on the current projection lines
    view_x2 = ( ( ( world_x + world_xenv ) >> 16 ) * distance >> 15 ) + ( view_turnoff_x * distance >> 15 );
    view_y2 = (world_y >> 16) * distance >> 15;
    view_xofs2 = view_x2;
    view_yofs2 = (world_yofs * distance >> 15) + poly_bottom[0][0] - view_y2;

    // 1. World x-location before transformation
    // 2. Viewer x-position at the current
    // 3. World y-location before perspective projection
    // 4. Viewer y-position below the horizon
    // 5. Number of raster lines drawn in this iteration

    DSP4_CLEAR_OUT();
    DSP4_WRITE_WORD((world_x + world_xenv) >> 16);
    DSP4_WRITE_WORD(view_x2);
    DSP4_WRITE_WORD(world_y >> 16);
    DSP4_WRITE_WORD(view_y2);

    //////////////////////////////////////////////////////////

    // SR = 0x00

    // determine # of raster lines used
    segments = view_y1 - view_y2;

    // prevent overdraw
    if (view_y2 >= poly_raster[0][0])
      segments = 0;
    else
      poly_raster[0][0] = view_y2;

    // don't draw outside the window
    if (view_y2 < poly_top[0][0])
    {
      segments = 0;

      // flush remaining raster lines
      if (view_y1 >= poly_top[0][0])
        segments = view_y1 - poly_top[0][0];
    }

    // SR = 0x80

    DSP4_WRITE_WORD(segments);

    //////////////////////////////////////////////////////////

    // scan next command if no SR check needed
    if (segments)
    {
      int32 px_dx, py_dy;
      int32 x_scroll, y_scroll;

      // SR = 0x00

      // linear interpolation (lerp) between projected points
      px_dx = (view_xofs2 - view_xofs1) * DSP4_Inverse(segments) << 1;
      py_dy = (view_yofs2 - view_yofs1) * DSP4_Inverse(segments) << 1;

      // starting step values
      x_scroll = SEX16(poly_cx[0][0] + view_xofs1);
      y_scroll = SEX16(-viewport_bottom + view_yofs1 + view_yofsenv + poly_cx[1][0] - world_yofs);

      // SR = 0x80

      // rasterize line
      for (lcv = 0; lcv < segments; lcv++)
      {
        // 1. HDMA memory pointer (bg1)
        // 2. vertical scroll offset ($210E)
        // 3. horizontal scroll offset ($210D)

        DSP4_WRITE_WORD(poly_ptr[0][0]);
        DSP4_WRITE_WORD((y_scroll + 0x8000) >> 16);
        DSP4_WRITE_WORD((x_scroll + 0x8000) >> 16);


        // update memory address
        poly_ptr[0][0] -= 4;

        // update screen values
        x_scroll += px_dx;
        y_scroll += py_dy;
      }
    }

    /////////////////////////////////////////////////////
    // Post-update

    // update new viewer (x,y,scroll) to last raster line drawn
    view_x1 = view_x2;
    view_y1 = view_y2;
    view_xofs1 = view_xofs2;
    view_yofs1 = view_yofs2;

    // add deltas for projection lines
    world_dx += SEX78(world_ddx);
    world_dy += SEX78(world_ddy);

    // update projection lines
    world_x += (world_dx + world_xenv);
    world_y += world_dy;

    ////////////////////////////////////////////////////
    // command check

    // scan next command
    DSP4.in_count = 2;
    DSP4_WAIT(1) resume1 :

    // inspect input
    distance = DSP4_READ_WORD();

    // terminate op
    if (distance == -0x8000)
      break;

    // already have 2 bytes in queue
    DSP4.in_count = 6;
    DSP4_WAIT(2) resume2:

    // inspect inputs
    world_ddy = DSP4_READ_WORD();
    world_ddx = DSP4_READ_WORD();
    view_yofsenv = DSP4_READ_WORD();

    // no envelope here
    world_xenv = 0;
  }
  while (1);

  DSP4.waiting4command = TRUE;
}

//////////////////////////////////////////////////////////////


void DSP4_OP0E()
{
  OAM_RowMax = 16;
  memset(OAM_Row, 0, 64);
}


//////////////////////////////////////////////////////////////

void DSP4_OP0F()
{
  DSP4.waiting4command = FALSE;

  // op flow control
  switch (DSP4_Logic)
  {
    case 1:
      goto resume1; break;
    case 2:
      goto resume2; break;
    case 3:
      goto resume3; break;
    case 4:
      goto resume4; break;
  }

  ////////////////////////////////////////////////////
  // process initial inputs

  // sort inputs
  DSP4_READ_WORD(); // 0x0000
  world_y = DSP4_READ_DWORD();
  poly_bottom[0][0] = DSP4_READ_WORD();
  poly_top[0][0] = DSP4_READ_WORD();
  poly_cx[1][0] = DSP4_READ_WORD();
  viewport_bottom = DSP4_READ_WORD();
  world_x = DSP4_READ_DWORD();
  poly_cx[0][0] = DSP4_READ_WORD();
  poly_ptr[0][0] = DSP4_READ_WORD();
  world_yofs = DSP4_READ_WORD();
  world_dy = DSP4_READ_DWORD();
  world_dx = DSP4_READ_DWORD();
  distance = DSP4_READ_WORD();
  DSP4_READ_WORD(); // 0x0000
  world_xenv = DSP4_READ_DWORD();
  world_ddy = DSP4_READ_WORD();
  world_ddx = DSP4_READ_WORD();
  view_yofsenv = DSP4_READ_WORD();

  // initial (x,y,offset) at starting raster line
  view_x1 = (world_x + world_xenv) >> 16;
  view_y1 = world_y >> 16;
  view_xofs1 = world_x >> 16;
  view_yofs1 = world_yofs;
  view_turnoff_x = 0;
  view_turnoff_dx = 0;

  // first raster line
  poly_raster[0][0] = poly_bottom[0][0];


  do
  {
    ////////////////////////////////////////////////////
    // process one iteration of projection

    // perspective projection of world (x,y,scroll) points
    // based on the current projection lines
    view_x2 = ((world_x + world_xenv) >> 16) * distance >> 15;
    view_y2 = (world_y >> 16) * distance >> 15;
    view_xofs2 = view_x2;
    view_yofs2 = (world_yofs * distance >> 15) + poly_bottom[0][0] - view_y2;

    // 1. World x-location before transformation
    // 2. Viewer x-position at the next
    // 3. World y-location before perspective projection
    // 4. Viewer y-position below the horizon
    // 5. Number of raster lines drawn in this iteration

    DSP4_CLEAR_OUT();
    DSP4_WRITE_WORD((world_x + world_xenv) >> 16);
    DSP4_WRITE_WORD(view_x2);
    DSP4_WRITE_WORD(world_y >> 16);
    DSP4_WRITE_WORD(view_y2);

    //////////////////////////////////////////////////////

    // SR = 0x00

    // determine # of raster lines used
    segments = poly_raster[0][0] - view_y2;

    // prevent overdraw
    if (view_y2 >= poly_raster[0][0])
      segments = 0;
    else
      poly_raster[0][0] = view_y2;

    // don't draw outside the window
    if (view_y2 < poly_top[0][0])
    {
      segments = 0;

      // flush remaining raster lines
      if (view_y1 >= poly_top[0][0])
        segments = view_y1 - poly_top[0][0];
    }

    // SR = 0x80

    DSP4_WRITE_WORD(segments);

    //////////////////////////////////////////////////////

    // scan next command if no SR check needed
    if (segments)
    {
      int32 px_dx, py_dy;
      int32 x_scroll, y_scroll;

      for (lcv = 0; lcv < 4; lcv++)
      {
        // grab inputs
        DSP4.in_count = 4;
        DSP4_WAIT(1);
        resume1 :
        for (;;)
        {
          int16 distance;
          int16 color, red, green, blue;

          distance = DSP4_READ_WORD();
          color = DSP4_READ_WORD();

          // U1+B5+G5+R5
          red = color & 0x1f;
          green = (color >> 5) & 0x1f;
          blue = (color >> 10) & 0x1f;

          // dynamic lighting
          red = (red * distance >> 15) & 0x1f;
          green = (green * distance >> 15) & 0x1f;
          blue = (blue * distance >> 15) & 0x1f;
          color = red | (green << 5) | (blue << 10);

          DSP4_CLEAR_OUT();
          DSP4_WRITE_WORD(color);
          break;
        }
      }

      //////////////////////////////////////////////////////

      // SR = 0x00

      // linear interpolation (lerp) between projected points
      px_dx = (view_xofs2 - view_xofs1) * DSP4_Inverse(segments) << 1;
      py_dy = (view_yofs2 - view_yofs1) * DSP4_Inverse(segments) << 1;


      // starting step values
      x_scroll = SEX16(poly_cx[0][0] + view_xofs1);
      y_scroll = SEX16(-viewport_bottom + view_yofs1 + view_yofsenv + poly_cx[1][0] - world_yofs);

      // SR = 0x80

      // rasterize line
      for (lcv = 0; lcv < segments; lcv++)
      {
        // 1. HDMA memory pointer
        // 2. vertical scroll offset ($210E)
        // 3. horizontal scroll offset ($210D)

        DSP4_WRITE_WORD(poly_ptr[0][0]);
        DSP4_WRITE_WORD((y_scroll + 0x8000) >> 16);
        DSP4_WRITE_WORD((x_scroll + 0x8000) >> 16);

        // update memory address
        poly_ptr[0][0] -= 4;

        // update screen values
        x_scroll += px_dx;
        y_scroll += py_dy;
      }
    }

    ////////////////////////////////////////////////////
    // Post-update

    // update new viewer (x,y,scroll) to last raster line drawn
    view_x1 = view_x2;
    view_y1 = view_y2;
    view_xofs1 = view_xofs2;
    view_yofs1 = view_yofs2;

    // add deltas for projection lines
    world_dx += SEX78(world_ddx);
    world_dy += SEX78(world_ddy);

    // update projection lines
    world_x += (world_dx + world_xenv);
    world_y += world_dy;

    // update road turnoff position
    view_turnoff_x += view_turnoff_dx;

    ////////////////////////////////////////////////////
    // command check

    // scan next command
    DSP4.in_count = 2;
    DSP4_WAIT(2) resume2:

    // check for termination
    distance = DSP4_READ_WORD();
    if (distance == -0x8000)
      break;

    // road splice
    if( (uint16) distance == 0x8001 )
    {
      DSP4.in_count = 6;
      DSP4_WAIT(3) resume3:

      distance = DSP4_READ_WORD();
      view_turnoff_x = DSP4_READ_WORD();
      view_turnoff_dx = DSP4_READ_WORD();

      // factor in new changes
      view_x1 += ( view_turnoff_x * distance >> 15 );
      view_xofs1 += ( view_turnoff_x * distance >> 15 );

      // update stepping values
      view_turnoff_x += view_turnoff_dx;

      DSP4.in_count = 2;
      DSP4_WAIT(2)
    }

    // already have 2 bytes in queue
    DSP4.in_count = 6;
    DSP4_WAIT(4) resume4 :

    // inspect inputs
    world_ddy = DSP4_READ_WORD();
    world_ddx = DSP4_READ_WORD();
    view_yofsenv = DSP4_READ_WORD();

    // no envelope here
    world_xenv = 0;
  }
  while (1);

  // terminate op
  DSP4.waiting4command = TRUE;
}

//////////////////////////////////////////////////////////////


void DSP4_OP10()
{
  DSP4.waiting4command = FALSE;

  // op flow control
  switch (DSP4_Logic)
  {
    case 1:
      goto resume1; break;
    case 2:
      goto resume2; break;
    case 3:
      goto resume3; break;
  }

  ////////////////////////////////////////////////////
  // sort inputs

  DSP4_READ_WORD(); // 0x0000
  world_y = DSP4_READ_DWORD();
  poly_bottom[0][0] = DSP4_READ_WORD();
  poly_top[0][0] = DSP4_READ_WORD();
  poly_cx[1][0] = DSP4_READ_WORD();
  viewport_bottom = DSP4_READ_WORD();
  world_x = DSP4_READ_DWORD();
  poly_cx[0][0] = DSP4_READ_WORD();
  poly_ptr[0][0] = DSP4_READ_WORD();
  world_yofs = DSP4_READ_WORD();
  distance = DSP4_READ_WORD();
  view_y2 = DSP4_READ_WORD();
  view_dy = DSP4_READ_WORD() * distance >> 15;
  view_x2 = DSP4_READ_WORD();
  view_dx = DSP4_READ_WORD() * distance >> 15;
  view_yofsenv = DSP4_READ_WORD();

  // initial (x,y,offset) at starting raster line
  view_x1 = world_x >> 16;
  view_y1 = world_y >> 16;
  view_xofs1 = view_x1;
  view_yofs1 = world_yofs;

  // first raster line
  poly_raster[0][0] = poly_bottom[0][0];

  do
  {
    ////////////////////////////////////////////////////
    // process one iteration of projection

    // add shaping
    view_x2 += view_dx;
    view_y2 += view_dy;

    // vertical scroll calculation
    view_xofs2 = view_x2;
    view_yofs2 = (world_yofs * distance >> 15) + poly_bottom[0][0] - view_y2;

    // 1. Viewer x-position at the next
    // 2. Viewer y-position below the horizon
    // 3. Number of raster lines drawn in this iteration

    DSP4_CLEAR_OUT();
    DSP4_WRITE_WORD(view_x2);
    DSP4_WRITE_WORD(view_y2);

    //////////////////////////////////////////////////////

    // SR = 0x00

    // determine # of raster lines used
    segments = view_y1 - view_y2;

    // prevent overdraw
    if (view_y2 >= poly_raster[0][0])
      segments = 0;
    else
      poly_raster[0][0] = view_y2;

    // don't draw outside the window
    if (view_y2 < poly_top[0][0])
    {
      segments = 0;

      // flush remaining raster lines
      if (view_y1 >= poly_top[0][0])
        segments = view_y1 - poly_top[0][0];
    }

    // SR = 0x80

    DSP4_WRITE_WORD(segments);

    //////////////////////////////////////////////////////

    // scan next command if no SR check needed
    if (segments)
    {
      for (lcv = 0; lcv < 4; lcv++)
      {
        // grab inputs
        DSP4.in_count = 4;
        DSP4_WAIT(1);
        resume1 :
        for (;;)
        {
          int16 distance;
          int16 color, red, green, blue;

          distance = DSP4_READ_WORD();
          color = DSP4_READ_WORD();

          // U1+B5+G5+R5
          red = color & 0x1f;
          green = (color >> 5) & 0x1f;
          blue = (color >> 10) & 0x1f;

          // dynamic lighting
          red = (red * distance >> 15) & 0x1f;
          green = (green * distance >> 15) & 0x1f;
          blue = (blue * distance >> 15) & 0x1f;
          color = red | (green << 5) | (blue << 10);

          DSP4_CLEAR_OUT();
          DSP4_WRITE_WORD(color);
          break;
        }
      }
    }

    //////////////////////////////////////////////////////

    // scan next command if no SR check needed
    if (segments)
    {
      int32 px_dx, py_dy;
      int32 x_scroll, y_scroll;

      // SR = 0x00

      // linear interpolation (lerp) between projected points
      px_dx = (view_xofs2 - view_xofs1) * DSP4_Inverse(segments) << 1;
      py_dy = (view_yofs2 - view_yofs1) * DSP4_Inverse(segments) << 1;

      // starting step values
      x_scroll = SEX16(poly_cx[0][0] + view_xofs1);
      y_scroll = SEX16(-viewport_bottom + view_yofs1 + view_yofsenv + poly_cx[1][0] - world_yofs);

      // SR = 0x80

      // rasterize line
      for (lcv = 0; lcv < segments; lcv++)
      {
        // 1. HDMA memory pointer (bg2)
        // 2. vertical scroll offset ($2110)
        // 3. horizontal scroll offset ($210F)

        DSP4_WRITE_WORD(poly_ptr[0][0]);
        DSP4_WRITE_WORD((y_scroll + 0x8000) >> 16);
        DSP4_WRITE_WORD((x_scroll + 0x8000) >> 16);

        // update memory address
        poly_ptr[0][0] -= 4;

        // update screen values
        x_scroll += px_dx;
        y_scroll += py_dy;
      }
    }

    /////////////////////////////////////////////////////
    // Post-update

    // update new viewer (x,y,scroll) to last raster line drawn
    view_x1 = view_x2;
    view_y1 = view_y2;
    view_xofs1 = view_xofs2;
    view_yofs1 = view_yofs2;

    ////////////////////////////////////////////////////
    // command check

    // scan next command
    DSP4.in_count = 2;
    DSP4_WAIT(2) resume2 :

    // check for opcode termination
    distance = DSP4_READ_WORD();
    if (distance == -0x8000)
      break;

    // already have 2 bytes in queue
    DSP4.in_count = 10;
    DSP4_WAIT(3) resume3 :


    // inspect inputs
    view_y2 = DSP4_READ_WORD();
    view_dy = DSP4_READ_WORD() * distance >> 15;
    view_x2 = DSP4_READ_WORD();
    view_dx = DSP4_READ_WORD() * distance >> 15;
  }
  while (1);

  DSP4.waiting4command = TRUE;
}

//////////////////////////////////////////////////////////////

void DSP4_OP11(int16 A, int16 B, int16 C, int16 D, int16 *M)
{
  // 0x155 = 341 = Horizontal Width of the Screen
  *M = ((A * 0x0155 >> 2) & 0xf000) |
       ((B * 0x0155 >> 6) & 0x0f00) |
       ((C * 0x0155 >> 10) & 0x00f0) |
       ((D * 0x0155 >> 14) & 0x000f);
}





/////////////////////////////////////////////////////////////
//Processing Code
/////////////////////////////////////////////////////////////
uint8 dsp4_byte;
uint16 dsp4_address;

void InitDSP4()
{
  memset(&DSP4, 0, sizeof(DSP4));
  DSP4.waiting4command = TRUE;
}

void DSP4_SetByte()
{
  // clear pending read
  if (DSP4.out_index < DSP4.out_count)
  {
    DSP4.out_index++;
    return;
  }

  if (DSP4.waiting4command)
  {
    if (DSP4.half_command)
    {
      DSP4.command |= (dsp4_byte << 8);
      DSP4.in_index = 0;
      DSP4.waiting4command = FALSE;
      DSP4.half_command = FALSE;
      DSP4.out_count = 0;
      DSP4.out_index = 0;

      DSP4_Logic = 0;


      switch (DSP4.command)
      {
        case 0x0000:
          DSP4.in_count = 4; break;
        case 0x0001:
          DSP4.in_count = 44; break;
        case 0x0003:
          DSP4.in_count = 0; break;
        case 0x0005:
          DSP4.in_count = 0; break;
        case 0x0006:
          DSP4.in_count = 0; break;
        case 0x0007:
          DSP4.in_count = 34; break;
        case 0x0008:
          DSP4.in_count = 90; break;
        case 0x0009:
          DSP4.in_count = 14; break;
        case 0x000a:
          DSP4.in_count = 6; break;
        case 0x000b:
          DSP4.in_count = 6; break;
        case 0x000d:
          DSP4.in_count = 42; break;
        case 0x000e:
          DSP4.in_count = 0; break;
        case 0x000f:
          DSP4.in_count = 46; break;
        case 0x0010:
          DSP4.in_count = 36; break;
        case 0x0011:
          DSP4.in_count = 8; break;
        default:
          DSP4.waiting4command = TRUE;
          break;
      }
    }
    else
    {
      DSP4.command = dsp4_byte;
      DSP4.half_command = TRUE;
    }
  }
  else
  {
    DSP4.parameters[DSP4.in_index] = dsp4_byte;
    DSP4.in_index++;
  }

  if (!DSP4.waiting4command && DSP4.in_count == DSP4.in_index)
  {
    // Actually execute the command
    DSP4.waiting4command = TRUE;
    DSP4.out_index = 0;
    DSP4.in_index = 0;

    switch (DSP4.command)
    {
        // 16-bit multiplication
      case 0x0000:
      {
        int16 multiplier, multiplicand;
        int32 product;

        multiplier = DSP4_READ_WORD();
        multiplicand = DSP4_READ_WORD();

        DSP4_Multiply(multiplicand, multiplier, &product);

        DSP4_CLEAR_OUT();
        DSP4_WRITE_WORD(product);
        DSP4_WRITE_WORD(product >> 16);
      }
      break;

      // single-player track projection
      case 0x0001:
        DSP4_OP01(); break;

      // single-player selection
      case 0x0003:
        DSP4_OP03(); break;

      // clear OAM
      case 0x0005:
        DSP4_OP05(); break;

      // transfer OAM
      case 0x0006:
        DSP4_OP06(); break;

      // single-player track turnoff projection
      case 0x0007:
        DSP4_OP07(); break;

      // solid polygon projection
      case 0x0008:
        DSP4_OP08(); break;

      // sprite projection
      case 0x0009:
        DSP4_OP09(); break;

      // unknown
      case 0x000A:
      {
        //int16 in1a = DSP4_READ_WORD();
        int16 in2a = DSP4_READ_WORD();
        //int16 in3a = DSP4_READ_WORD();
        int16 out1a, out2a, out3a, out4a;

        DSP4_OP0A(in2a, &out2a, &out1a, &out4a, &out3a);

        DSP4_CLEAR_OUT();
        DSP4_WRITE_WORD(out1a);
        DSP4_WRITE_WORD(out2a);
        DSP4_WRITE_WORD(out3a);
        DSP4_WRITE_WORD(out4a);
      }
      break;

      // set OAM
      case 0x000B:
      {
        int16 sp_x = DSP4_READ_WORD();
        int16 sp_y = DSP4_READ_WORD();
        int16 sp_attr = DSP4_READ_WORD();
        bool8 draw = 1;

        DSP4_CLEAR_OUT();

        DSP4_OP0B(&draw, sp_x, sp_y, sp_attr, 0, 1);
      }
      break;

      // multi-player track projection
      case 0x000D:
        DSP4_OP0D(); break;

      // multi-player selection
      case 0x000E:
        DSP4_OP0E(); break;

      // single-player track projection with lighting
      case 0x000F:
        DSP4_OP0F(); break;

      // single-player track turnoff projection with lighting
      case 0x0010:
        DSP4_OP10(); break;

      // unknown: horizontal mapping command
      case 0x0011:
      {
        int16 a, b, c, d, m;


        d = DSP4_READ_WORD();
        c = DSP4_READ_WORD();
        b = DSP4_READ_WORD();
        a = DSP4_READ_WORD();

        DSP4_OP11(a, b, c, d, &m);

        DSP4_CLEAR_OUT();
        DSP4_WRITE_WORD(m);

        break;
      }

      default:
        break;
    }
  }
}

void DSP4_GetByte()
{
  if (DSP4.out_count)
  {
    dsp4_byte = (uint8) DSP4.output[DSP4.out_index&0x1FF];
    DSP4.out_index++;
    if (DSP4.out_count == DSP4.out_index)
      DSP4.out_count = 0;
  }
  else
  {
    dsp4_byte = 0xff;
  }
}
