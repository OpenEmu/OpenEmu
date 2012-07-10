/* md_ntsc 0.1.2. http://www.slack.net/~ant/ */

/* Modified for use with Genesis Plus GX -- EkeEke */

#include "shared.h"
#include "md_ntsc.h"

/* Copyright (C) 2006 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

md_ntsc_setup_t const md_ntsc_monochrome = { 0,-1, 0, 0,.2,  0, 0,-.2,-.2,-1, 0,  0 };
md_ntsc_setup_t const md_ntsc_composite  = { 0, 0, 0, 0, 0,  0, 0,  0,  0, 0, 0,  0 };
md_ntsc_setup_t const md_ntsc_svideo     = { 0, 0, 0, 0, 0,  0,.2, -1, -1, 0, 0,  0 };
md_ntsc_setup_t const md_ntsc_rgb        = { 0, 0, 0, 0,.2,  0,.7, -1, -1,-1, 0,  0 };

#define alignment_count 2
#define burst_count     1
#define rescale_in      1
#define rescale_out     1

#define artifacts_mid   0.40f
#define fringing_mid    0.30f
#define std_decoder_hue 0

#define gamma_size      8
#define artifacts_max   1.00f
#define LUMA_CUTOFF     0.1974

#include "md_ntsc_impl.h"

/* 2 input pixels -> 4 composite samples */
pixel_info_t const md_ntsc_pixels [alignment_count] = {
  { PIXEL_OFFSET( -4, -9 ), { 0.1f, 0.9f, 0.9f, 0.1f } },
  { PIXEL_OFFSET( -2, -7 ), { 0.1f, 0.9f, 0.9f, 0.1f } },
};

static void correct_errors( md_ntsc_rgb_t color, md_ntsc_rgb_t* out )
{
  unsigned i;
  for ( i = 0; i < rgb_kernel_size / 4; i++ )
  {
    md_ntsc_rgb_t error = color -
        out [i    ] - out [i + 2    +16] - out [i + 4    ] - out [i + 6    +16] -
        out [i + 8] - out [(i+10)%16+16] - out [(i+12)%16] - out [(i+14)%16+16];
    CORRECT_ERROR( i + 6 + 16 );
    /*DISTRIBUTE_ERROR( 2+16, 4, 6+16 );*/
  }
}

void md_ntsc_init( md_ntsc_t* ntsc, md_ntsc_setup_t const* setup )
{
  int entry;
  init_t impl;
  if ( !setup )
    setup = &md_ntsc_composite;
  init( &impl, setup );

  for ( entry = 0; entry < md_ntsc_palette_size; entry++ )
  {
    float bb = impl.to_float [entry >> 6 & 7];
    float gg = impl.to_float [entry >> 3 & 7];
    float rr = impl.to_float [entry      & 7];

    float y, i, q = RGB_TO_YIQ( rr, gg, bb, y, i );

    int r, g, b = YIQ_TO_RGB( y, i, q, impl.to_rgb, int, r, g );
    md_ntsc_rgb_t rgb = PACK_RGB( r, g, b );

    if ( setup->palette_out )
      RGB_PALETTE_OUT( rgb, &setup->palette_out [entry * 3] );

    if ( ntsc )
    {
      gen_kernel( &impl, y, i, q, ntsc->table [entry] );
      correct_errors( rgb, ntsc->table [entry] );
    }
  }
}

#ifndef CUSTOM_BLITTER
void md_ntsc_blit( md_ntsc_t const* ntsc, MD_NTSC_IN_T const* table, unsigned char* input,
                   int in_width, int vline)
{
  int const chunk_count = in_width / md_ntsc_in_chunk - 1;

  /* use palette entry 0 for unused pixels */
  MD_NTSC_IN_T border = table[0];

  MD_NTSC_BEGIN_ROW( ntsc, border,
        MD_NTSC_ADJ_IN( table[*input++] ),
        MD_NTSC_ADJ_IN( table[*input++] ),
        MD_NTSC_ADJ_IN( table[*input++] ) );

  md_ntsc_out_t* restrict line_out  = (md_ntsc_out_t*)(&bitmap.data[(vline * bitmap.pitch)]);

  int n;

  for ( n = chunk_count; n; --n )
  {
    /* order of input and output pixels must not be altered */
    MD_NTSC_COLOR_IN( 0, ntsc, MD_NTSC_ADJ_IN( table[*input++] ) );
    MD_NTSC_RGB_OUT( 0, *line_out++ );
    MD_NTSC_RGB_OUT( 1, *line_out++ );

    MD_NTSC_COLOR_IN( 1, ntsc, MD_NTSC_ADJ_IN( table[*input++] ) );
    MD_NTSC_RGB_OUT( 2, *line_out++ );
    MD_NTSC_RGB_OUT( 3, *line_out++ );

    MD_NTSC_COLOR_IN( 2, ntsc, MD_NTSC_ADJ_IN( table[*input++] ) );
    MD_NTSC_RGB_OUT( 4, *line_out++ );
    MD_NTSC_RGB_OUT( 5, *line_out++ );

    MD_NTSC_COLOR_IN( 3, ntsc, MD_NTSC_ADJ_IN( table[*input++] ) );
    MD_NTSC_RGB_OUT( 6, *line_out++ );
    MD_NTSC_RGB_OUT( 7, *line_out++ );
  }

  /* finish final pixels */
  MD_NTSC_COLOR_IN( 0, ntsc, MD_NTSC_ADJ_IN( table[*input++] ) );
  MD_NTSC_RGB_OUT( 0, *line_out++ );
  MD_NTSC_RGB_OUT( 1, *line_out++ );

  MD_NTSC_COLOR_IN( 1, ntsc, border );
  MD_NTSC_RGB_OUT( 2, *line_out++ );
  MD_NTSC_RGB_OUT( 3, *line_out++ );

  MD_NTSC_COLOR_IN( 2, ntsc, border );
  MD_NTSC_RGB_OUT( 4, *line_out++ );
  MD_NTSC_RGB_OUT( 5, *line_out++ );

  MD_NTSC_COLOR_IN( 3, ntsc, border );
  MD_NTSC_RGB_OUT( 6, *line_out++ );
  MD_NTSC_RGB_OUT( 7, *line_out++ );
}
#endif
