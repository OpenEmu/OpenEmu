/* sms_ntsc 0.2.3. http://www.slack.net/~ant/ */

/* Common implementation of NTSC filters */

#include <assert.h>
#include <math.h>

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

#define DISABLE_CORRECTION 0

#undef PI
#define PI 3.14159265358979323846f

#ifndef LUMA_CUTOFF
  #define LUMA_CUTOFF 0.20
#endif
#ifndef gamma_size
  #define gamma_size 1
#endif
#ifndef rgb_bits
  #define rgb_bits 8
#endif
#ifndef artifacts_max
  #define artifacts_max (artifacts_mid * 1.5f)
#endif
#ifndef fringing_max
  #define fringing_max (fringing_mid * 2)
#endif
#ifndef STD_HUE_CONDITION
  #define STD_HUE_CONDITION( setup ) 1
#endif

#define ext_decoder_hue     (std_decoder_hue + 15)
#define rgb_unit            (1 << rgb_bits)
#define rgb_offset          (rgb_unit * 2 + 0.5f)

enum { burst_size  = sms_ntsc_entry_size / burst_count };
enum { kernel_half = 16 };
enum { kernel_size = kernel_half * 2 + 1 };

typedef struct init_t
{
  float to_rgb [burst_count * 6];
  float to_float [gamma_size];
  float contrast;
  float brightness;
  float artifacts;
  float fringing;
  float kernel [rescale_out * kernel_size * 2];
} init_t;

#define ROTATE_IQ( i, q, sin_b, cos_b ) {\
  float t;\
  t = i * cos_b - q * sin_b;\
  q = i * sin_b + q * cos_b;\
  i = t;\
}

static void init_filters( init_t* impl, sms_ntsc_setup_t const* setup )
{
#if rescale_out > 1
  float kernels [kernel_size * 2];
#else
  float* const kernels = impl->kernel;
#endif

  /* generate luma (y) filter using sinc kernel */
  {
    /* sinc with rolloff (dsf) */
    float const rolloff = 1 + (float) setup->sharpness * (float) 0.032;
    float const maxh = 32;
    float const pow_a_n = (float) pow( rolloff, maxh );
    float sum;
    int i;
    /* quadratic mapping to reduce negative (blurring) range */
    float to_angle = (float) setup->resolution + 1;
    to_angle = PI / maxh * (float) LUMA_CUTOFF * (to_angle * to_angle + 1);
    
    kernels [kernel_size * 3 / 2] = maxh; /* default center value */
    for ( i = 0; i < kernel_half * 2 + 1; i++ )
    {
      int x = i - kernel_half;
      float angle = x * to_angle;
      /* instability occurs at center point with rolloff very close to 1.0 */
      if ( x || pow_a_n > (float) 1.056 || pow_a_n < (float) 0.981 )
      {
        float rolloff_cos_a = rolloff * (float) cos( angle );
        float num = 1 - rolloff_cos_a -
            pow_a_n * (float) cos( maxh * angle ) +
            pow_a_n * rolloff * (float) cos( (maxh - 1) * angle );
        float den = 1 - rolloff_cos_a - rolloff_cos_a + rolloff * rolloff;
        float dsf = num / den;
        kernels [kernel_size * 3 / 2 - kernel_half + i] = dsf - (float) 0.5;
      }
    }
    
    /* apply blackman window and find sum */
    sum = 0;
    for ( i = 0; i < kernel_half * 2 + 1; i++ )
    {
      float x = PI * 2 / (kernel_half * 2) * i;
      float blackman = 0.42f - 0.5f * (float) cos( x ) + 0.08f * (float) cos( x * 2 );
      sum += (kernels [kernel_size * 3 / 2 - kernel_half + i] *= blackman);
    }
    
    /* normalize kernel */
    sum = 1.0f / sum;
    for ( i = 0; i < kernel_half * 2 + 1; i++ )
    {
      int x = kernel_size * 3 / 2 - kernel_half + i;
      kernels [x] *= sum;
      assert( kernels [x] == kernels [x] ); /* catch numerical instability */
    }
  }

  /* generate chroma (iq) filter using gaussian kernel */
  {
    float const cutoff_factor = -0.03125f;
    float cutoff = (float) setup->bleed;
    int i;
    
    if ( cutoff < 0 )
    {
      /* keep extreme value accessible only near upper end of scale (1.0) */
      cutoff *= cutoff;
      cutoff *= cutoff;
      cutoff *= cutoff;
      cutoff *= -30.0f / 0.65f;
    }
    cutoff = cutoff_factor - 0.65f * cutoff_factor * cutoff;
    
    for ( i = -kernel_half; i <= kernel_half; i++ )
      kernels [kernel_size / 2 + i] = (float) exp( i * i * cutoff );
    
    /* normalize even and odd phases separately */
    for ( i = 0; i < 2; i++ )
    {
      float sum = 0;
      int x;
      for ( x = i; x < kernel_size; x += 2 )
        sum += kernels [x];
      
      sum = 1.0f / sum;
      for ( x = i; x < kernel_size; x += 2 )
      {
        kernels [x] *= sum;
        assert( kernels [x] == kernels [x] ); /* catch numerical instability */
      }
    }
  }
  
  /*
  printf( "luma:\n" );
  for ( i = kernel_size; i < kernel_size * 2; i++ )
    printf( "%f\n", kernels [i] );
  printf( "chroma:\n" );
  for ( i = 0; i < kernel_size; i++ )
    printf( "%f\n", kernels [i] );
  */
  
  /* generate linear rescale kernels */
  #if rescale_out > 1
  {
    float weight = 1.0f;
    float* out = impl->kernel;
    int n = rescale_out;
    do
    {
      float remain = 0;
      int i;
      weight -= 1.0f / rescale_in;
      for ( i = 0; i < kernel_size * 2; i++ )
      {
        float cur = kernels [i];
        float m = cur * weight;
        *out++ = m + remain;
        remain = cur - m;
      }
    }
    while ( --n );
  }
  #endif
}

static float const default_decoder [6] =
  { 0.956f, 0.621f, -0.272f, -0.647f, -1.105f, 1.702f };

static void init( init_t* impl, sms_ntsc_setup_t const* setup )
{
  impl->brightness = (float) setup->brightness * (0.5f * rgb_unit) + rgb_offset;
  impl->contrast   = (float) setup->contrast   * (0.5f * rgb_unit) + rgb_unit;
  #ifdef default_palette_contrast
    if ( !setup->palette )
      impl->contrast *= default_palette_contrast;
  #endif
  
  impl->artifacts = (float) setup->artifacts;
  if ( impl->artifacts > 0 )
    impl->artifacts *= artifacts_max - artifacts_mid;
  impl->artifacts = impl->artifacts * artifacts_mid + artifacts_mid;

  impl->fringing = (float) setup->fringing;
  if ( impl->fringing > 0 )
    impl->fringing *= fringing_max - fringing_mid;
  impl->fringing = impl->fringing * fringing_mid + fringing_mid;
  
  init_filters( impl, setup );
  
  /* generate gamma table */
  if ( gamma_size > 1 )
  {
    float const to_float = 1.0f / (gamma_size - (gamma_size > 1));
    float const gamma = 1.1333f - (float) setup->gamma * 0.5f;
    /* match common PC's 2.2 gamma to TV's 2.65 gamma */
    int i;
    for ( i = 0; i < gamma_size; i++ )
      impl->to_float [i] =
          (float) pow( i * to_float, gamma ) * impl->contrast + impl->brightness;
  }
  
  /* setup decoder matricies */
  {
    float hue = (float) setup->hue * PI + PI / 180 * ext_decoder_hue;
    float sat = (float) setup->saturation + 1;
    float const* decoder = setup->decoder_matrix;
    if ( !decoder )
    {
      decoder = default_decoder;
      if ( STD_HUE_CONDITION( setup ) )
        hue += PI / 180 * (std_decoder_hue - ext_decoder_hue);
    }
    
    {
      float s = (float) sin( hue ) * sat;
      float c = (float) cos( hue ) * sat;
      float* out = impl->to_rgb;
      int n;
      
      n = burst_count;
      do
      {
        float const* in = decoder;
        int n = 3;
        do
        {
          float i = *in++;
          float q = *in++;
          *out++ = i * c - q * s;
          *out++ = i * s + q * c;
        }
        while ( --n );
        if ( burst_count <= 1 )
          break;
        ROTATE_IQ( s, c, 0.866025f, -0.5f ); /* +120 degrees */
      }
      while ( --n );
    }
  }
}

/* kernel generation */

#define RGB_TO_YIQ( r, g, b, y, i ) (\
  (y = (r) * 0.299f + (g) * 0.587f + (b) * 0.114f),\
  (i = (r) * 0.596f - (g) * 0.275f - (b) * 0.321f),\
  ((r) * 0.212f - (g) * 0.523f + (b) * 0.311f)\
)

#define YIQ_TO_RGB( y, i, q, to_rgb, type, r, g ) (\
  r = (type) (y + to_rgb [0] * i + to_rgb [1] * q),\
  g = (type) (y + to_rgb [2] * i + to_rgb [3] * q),\
  (type) (y + to_rgb [4] * i + to_rgb [5] * q)\
)

#define PACK_RGB( r, g, b ) ((r) << 21 | (g) << 11 | (b) << 1)

enum { rgb_kernel_size = burst_size / alignment_count };
enum { rgb_bias = rgb_unit * 2 * sms_ntsc_rgb_builder };

typedef struct pixel_info_t
{
  int offset;
  float negate;
  float kernel [4];
} pixel_info_t;

#if rescale_in > 1
  #define PIXEL_OFFSET_( ntsc, scaled ) \
    (kernel_size / 2 + ntsc + (scaled != 0) + (rescale_out - scaled) % rescale_out + \
        (kernel_size * 2 * scaled))

  #define PIXEL_OFFSET( ntsc, scaled ) \
    PIXEL_OFFSET_( ((ntsc) - (scaled) / rescale_out * rescale_in),\
        (((scaled) + rescale_out * 10) % rescale_out) ),\
    (1.0f - (((ntsc) + 100) & 2))
#else
  #define PIXEL_OFFSET( ntsc, scaled ) \
    (kernel_size / 2 + (ntsc) - (scaled)),\
    (1.0f - (((ntsc) + 100) & 2))
#endif

extern pixel_info_t const sms_ntsc_pixels [alignment_count];

/* Generate pixel at all burst phases and column alignments */
static void gen_kernel( init_t* impl, float y, float i, float q, sms_ntsc_rgb_t* out )
{
  /* generate for each scanline burst phase */
  float const* to_rgb = impl->to_rgb;
  int burst_remain = burst_count;
  y -= rgb_offset;
  do
  {
    /* Encode yiq into *two* composite signals (to allow control over artifacting).
    Convolve these with kernels which: filter respective components, apply
    sharpening, and rescale horizontally. Convert resulting yiq to rgb and pack
    into integer. Based on algorithm by NewRisingSun. */
    pixel_info_t const* pixel = sms_ntsc_pixels;
    int alignment_remain = alignment_count;
    do
    {
      /* negate is -1 when composite starts at odd multiple of 2 */
      float const yy = y * impl->fringing * pixel->negate;
      float const ic0 = (i + yy) * pixel->kernel [0];
      float const qc1 = (q + yy) * pixel->kernel [1];
      float const ic2 = (i - yy) * pixel->kernel [2];
      float const qc3 = (q - yy) * pixel->kernel [3];
      
      float const factor = impl->artifacts * pixel->negate;
      float const ii = i * factor;
      float const yc0 = (y + ii) * pixel->kernel [0];
      float const yc2 = (y - ii) * pixel->kernel [2];
      
      float const qq = q * factor;
      float const yc1 = (y + qq) * pixel->kernel [1];
      float const yc3 = (y - qq) * pixel->kernel [3];
      
      float const* k = &impl->kernel [pixel->offset];
      int n;
      ++pixel;
      for ( n = rgb_kernel_size; n; --n )
      {
        float i = k[0]*ic0 + k[2]*ic2;
        float q = k[1]*qc1 + k[3]*qc3;
        float y = k[kernel_size+0]*yc0 + k[kernel_size+1]*yc1 +
                  k[kernel_size+2]*yc2 + k[kernel_size+3]*yc3 + rgb_offset;
        if ( rescale_out <= 1 )
          k--;
        else if ( k < &impl->kernel [kernel_size * 2 * (rescale_out - 1)] )
          k += kernel_size * 2 - 1;
        else
          k -= kernel_size * 2 * (rescale_out - 1) + 2;
        {
          int r, g, b = YIQ_TO_RGB( y, i, q, to_rgb, int, r, g );
          *out++ = PACK_RGB( r, g, b ) - rgb_bias;
        }
      }
    }
    while ( alignment_count > 1 && --alignment_remain );
    
    if ( burst_count <= 1 )
      break;
    
    to_rgb += 6;
    
    ROTATE_IQ( i, q, -0.866025f, -0.5f ); /* -120 degrees */
  }
  while ( --burst_remain );
}

static void correct_errors( sms_ntsc_rgb_t color, sms_ntsc_rgb_t* out );

#if DISABLE_CORRECTION
  #define CORRECT_ERROR( a ) { out [i] += rgb_bias; }
  #define DISTRIBUTE_ERROR( a, b, c ) { out [i] += rgb_bias; }
#else
  #define CORRECT_ERROR( a ) { out [a] += error; }
  #define DISTRIBUTE_ERROR( a, b, c ) {\
    sms_ntsc_rgb_t fourth = (error + 2 * sms_ntsc_rgb_builder) >> 2;\
    fourth &= (rgb_bias >> 1) - sms_ntsc_rgb_builder;\
    fourth -= rgb_bias >> 2;\
    out [a] += fourth;\
    out [b] += fourth;\
    out [c] += fourth;\
    out [i] += error - (fourth * 3);\
  }
#endif

#define RGB_PALETTE_OUT( rgb, out_ )\
{\
  unsigned char* out = (out_);\
  sms_ntsc_rgb_t clamped = (rgb);\
  SMS_NTSC_CLAMP_( clamped, (8 - rgb_bits) );\
  out [0] = (unsigned char) (clamped >> 21);\
  out [1] = (unsigned char) (clamped >> 11);\
  out [2] = (unsigned char) (clamped >>  1);\
}

/* blitter related */

#ifndef restrict
  #if defined (__GNUC__)
    #define restrict __restrict__
  #elif defined (_MSC_VER) && _MSC_VER > 1300
    #define restrict __restrict
  #else
    /* no support for restricted pointers */
    #define restrict
  #endif
#endif

#include <limits.h>

#if SMS_NTSC_OUT_DEPTH <= 16
  #if USHRT_MAX == 0xFFFF
    typedef unsigned short sms_ntsc_out_t;
  #else
    #error "Need 16-bit int type"
  #endif

#else
  #if UINT_MAX == 0xFFFFFFFF
    typedef unsigned int  sms_ntsc_out_t;
  #elif ULONG_MAX == 0xFFFFFFFF
    typedef unsigned long sms_ntsc_out_t;
  #else
    #error "Need 32-bit int type"
  #endif

#endif
