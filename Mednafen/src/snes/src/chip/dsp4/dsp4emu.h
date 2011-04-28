//DSP-4 emulator code
//Copyright (c) 2004-2006 Dreamer Nom, John Weidman, Kris Bleakley, Nach, z80 gaiden

#ifndef DSP4EMU_H
#define DSP4EMU_H

#undef TRUE
#undef FALSE
#define TRUE  true
#define FALSE false

struct DSP4_t
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
};

extern struct DSP4_t DSP4;

struct DSP4_vars_t
{
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
};

extern struct DSP4_vars_t DSP4_vars;

#endif
