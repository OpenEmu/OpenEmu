/* sms_ntsc 0.2.3. http://www.slack.net/~ant/ */

/* Modified for use with Genesis Plus GX -- EkeEke */

#include "shared.h"
#include "sms_ntsc.h"

/* Copyright (C) 2006-2007 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

sms_ntsc_setup_t const sms_ntsc_monochrome = { 0,-1, 0, 0,.2,  0, .2,-.2,-.2,-1, 0,  0 };
sms_ntsc_setup_t const sms_ntsc_composite  = { 0, 0, 0, 0, 0,  0,.25,  0,  0, 0, 0,  0 };
sms_ntsc_setup_t const sms_ntsc_svideo     = { 0, 0, 0, 0, 0,  0,.25, -1, -1, 0, 0,  0 };
sms_ntsc_setup_t const sms_ntsc_rgb        = { 0, 0, 0, 0,.2,  0,.70, -1, -1,-1, 0,  0 };

#define alignment_count 3
#define burst_count     1
#define rescale_in      8
#define rescale_out     7

#define artifacts_mid   0.4f
#define artifacts_max   1.2f
#define fringing_mid    0.8f
#define std_decoder_hue 0

#define gamma_size      16

#include "sms_ntsc_impl.h"

/* 3 input pixels -> 8 composite samples */
pixel_info_t const sms_ntsc_pixels [alignment_count] = {
  { PIXEL_OFFSET( -4, -9 ), { 1, 1, .6667f, 0 } },
  { PIXEL_OFFSET( -2, -7 ), {       .3333f, 1, 1, .3333f } },
  { PIXEL_OFFSET(  0, -5 ), {                  0, .6667f, 1, 1 } },
};

static void correct_errors( sms_ntsc_rgb_t color, sms_ntsc_rgb_t* out )
{
  unsigned i;
  for ( i = 0; i < rgb_kernel_size / 2; i++ )
  {
    sms_ntsc_rgb_t error = color -
        out [i    ] - out [(i+12)%14+14] - out [(i+10)%14+28] -
        out [i + 7] - out [i + 5    +14] - out [i + 3    +28];
    CORRECT_ERROR( i + 3 + 28 );
  }
}

void sms_ntsc_init( sms_ntsc_t* ntsc, sms_ntsc_setup_t const* setup )
{
  int entry;
  init_t impl;
  if ( !setup )
    setup = &sms_ntsc_composite;
  init( &impl, setup );
  
  for ( entry = 0; entry < sms_ntsc_palette_size; entry++ )
  {
    float bb = impl.to_float [entry >> 8 & 0x0F];
    float gg = impl.to_float [entry >> 4 & 0x0F];
    float rr = impl.to_float [entry      & 0x0F];
    
    float y, i, q = RGB_TO_YIQ( rr, gg, bb, y, i );
    
    int r, g, b = YIQ_TO_RGB( y, i, q, impl.to_rgb, int, r, g );
    sms_ntsc_rgb_t rgb = PACK_RGB( r, g, b );
    
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
void sms_ntsc_blit( sms_ntsc_t const* ntsc, SMS_NTSC_IN_T const* table, unsigned char* input,
                    int in_width, int vline)
{
  int const chunk_count = in_width / sms_ntsc_in_chunk;

  /* handle extra 0, 1, or 2 pixels by placing them at beginning of row */
  int const in_extra = in_width - chunk_count * sms_ntsc_in_chunk;
  unsigned const extra2 = (unsigned) -(in_extra >> 1 & 1); /* (unsigned) -1 = ~0 */
  unsigned const extra1 = (unsigned) -(in_extra & 1) | extra2;

  /* use palette entry 0 for unused pixels */
  SMS_NTSC_IN_T border = table[0];

  SMS_NTSC_BEGIN_ROW( ntsc, border,
      (SMS_NTSC_ADJ_IN( table[input[0]] )) & extra2,
      (SMS_NTSC_ADJ_IN( table[input[extra2 & 1]] )) & extra1 );

  sms_ntsc_out_t* restrict line_out  = (sms_ntsc_out_t*)(&bitmap.data[(vline * bitmap.pitch)]);

  int n;
  input += in_extra;

  for ( n = chunk_count; n; --n )
  {
    /* order of input and output pixels must not be altered */
    SMS_NTSC_COLOR_IN( 0, ntsc, SMS_NTSC_ADJ_IN( table[*input++] ) );
    SMS_NTSC_RGB_OUT( 0, *line_out++ );
    SMS_NTSC_RGB_OUT( 1, *line_out++ );
    
    SMS_NTSC_COLOR_IN( 1, ntsc, SMS_NTSC_ADJ_IN( table[*input++] ) );
    SMS_NTSC_RGB_OUT( 2, *line_out++ );
    SMS_NTSC_RGB_OUT( 3, *line_out++ );
      
    SMS_NTSC_COLOR_IN( 2, ntsc, SMS_NTSC_ADJ_IN( table[*input++] ) );
    SMS_NTSC_RGB_OUT( 4, *line_out++ );
    SMS_NTSC_RGB_OUT( 5, *line_out++ );
    SMS_NTSC_RGB_OUT( 6, *line_out++ );
  }

  /* finish final pixels */
  SMS_NTSC_COLOR_IN( 0, ntsc, border );
  SMS_NTSC_RGB_OUT( 0, *line_out++ );
  SMS_NTSC_RGB_OUT( 1, *line_out++ );

  SMS_NTSC_COLOR_IN( 1, ntsc, border );
  SMS_NTSC_RGB_OUT( 2, *line_out++ );
  SMS_NTSC_RGB_OUT( 3, *line_out++ );

  SMS_NTSC_COLOR_IN( 2, ntsc, border );
  SMS_NTSC_RGB_OUT( 4, *line_out++ );
  SMS_NTSC_RGB_OUT( 5, *line_out++ );
  SMS_NTSC_RGB_OUT( 6, *line_out++ );
}
#endif
