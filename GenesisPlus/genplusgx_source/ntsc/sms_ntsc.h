/* Sega Master System/Game Gear/TI 99/4A NTSC video filter */

/* sms_ntsc 0.2.3 */
#ifndef SMS_NTSC_H
#define SMS_NTSC_H

#include "sms_ntsc_config.h"

#ifdef __cplusplus
  extern "C" {
#endif

/* Image parameters, ranging from -1.0 to 1.0. Actual internal values shown
in parenthesis and should remain fairly stable in future versions. */
typedef struct sms_ntsc_setup_t
{
  /* Basic parameters */
  double hue;        /* -1 = -180 degrees     +1 = +180 degrees */
  double saturation; /* -1 = grayscale (0.0)  +1 = oversaturated colors (2.0) */
  double contrast;   /* -1 = dark (0.5)       +1 = light (1.5) */
  double brightness; /* -1 = dark (0.5)       +1 = light (1.5) */
  double sharpness;  /* edge contrast enhancement/blurring */
  
  /* Advanced parameters */
  double gamma;      /* -1 = dark (1.5)       +1 = light (0.5) */
  double resolution; /* image resolution */
  double artifacts;  /* artifacts caused by color changes */
  double fringing;   /* color artifacts caused by brightness changes */
  double bleed;      /* color bleed (color resolution reduction) */
  float const* decoder_matrix; /* optional RGB decoder matrix, 6 elements */
  
  unsigned char* palette_out;  /* optional RGB palette out, 3 bytes per color */
} sms_ntsc_setup_t;

/* Video format presets */
extern sms_ntsc_setup_t const sms_ntsc_composite; /* color bleeding + artifacts */
extern sms_ntsc_setup_t const sms_ntsc_svideo;    /* color bleeding only */
extern sms_ntsc_setup_t const sms_ntsc_rgb;       /* crisp image */
extern sms_ntsc_setup_t const sms_ntsc_monochrome;/* desaturated + artifacts */

enum { sms_ntsc_palette_size = 4096 };

/* Initializes and adjusts parameters. Can be called multiple times on the same
sms_ntsc_t object. Can pass NULL for either parameter. */
typedef struct sms_ntsc_t sms_ntsc_t;
void sms_ntsc_init( sms_ntsc_t* ntsc, sms_ntsc_setup_t const* setup );

/* Filters one row of pixels. Input pixel format is set by SMS_NTSC_IN_FORMAT
and output RGB depth is set by SMS_NTSC_OUT_DEPTH. Both default to 16-bit RGB.
In_row_width is the number of pixels to get to the next input row. */
void sms_ntsc_blit( sms_ntsc_t const* ntsc, SMS_NTSC_IN_T const* table, unsigned char* input,
    int in_width, int vline);

/* Number of output pixels written by blitter for given input width. */
#define SMS_NTSC_OUT_WIDTH( in_width ) \
  (((in_width) / sms_ntsc_in_chunk + 1) * sms_ntsc_out_chunk)

/* Number of input pixels that will fit within given output width. Might be
rounded down slightly; use SMS_NTSC_OUT_WIDTH() on result to find rounded
value. */
#define SMS_NTSC_IN_WIDTH( out_width ) \
  (((out_width) / sms_ntsc_out_chunk - 1) * sms_ntsc_in_chunk + 2)


/* Interface for user-defined custom blitters */

enum { sms_ntsc_in_chunk    = 3 }; /* number of input pixels read per chunk */
enum { sms_ntsc_out_chunk   = 7 }; /* number of output pixels generated per chunk */

/* Begins outputting row and starts three pixels. First pixel will be cut off a bit.
Declares variables, so must be before first statement in a block (unless you're using C++). */
#define SMS_NTSC_BEGIN_ROW( ntsc, pixel0, pixel1, pixel2 ) \
  SMS_NTSC_BEGIN_ROW_6_( pixel0, pixel1, pixel2, SMS_NTSC_IN_FORMAT, ntsc )

/* Begins input pixel */
#define SMS_NTSC_COLOR_IN( in_index, ntsc, color_in ) \
  SMS_NTSC_COLOR_IN_( in_index, color_in, SMS_NTSC_IN_FORMAT, ntsc )

/* Generates output pixel */
#define SMS_NTSC_RGB_OUT( x, rgb_out ) {\
  raw_ =\
    kernel0  [x       ] + kernel1  [(x+12)%7+14] + kernel2  [(x+10)%7+28] +\
    kernelx0 [(x+7)%14] + kernelx1 [(x+ 5)%7+21] + kernelx2 [(x+ 3)%7+35];\
  SMS_NTSC_CLAMP_( raw_, 0 );\
  SMS_NTSC_RGB_OUT_( rgb_out, 0 );\
}

/* private */
enum { sms_ntsc_entry_size = 3 * 14 };
typedef unsigned long sms_ntsc_rgb_t;
struct sms_ntsc_t {
  sms_ntsc_rgb_t table [sms_ntsc_palette_size] [sms_ntsc_entry_size];
};

#define SMS_NTSC_BGR12( ntsc, n ) (ntsc)->table [n & 0xFFF]

#define SMS_NTSC_RGB16( ntsc, n ) \
  (sms_ntsc_rgb_t const*) ((char const*) (ntsc)->table +\
  ((n << 10 & 0x7800) | (n & 0x0780) | (n >> 9 & 0x0078)) *\
  (sms_ntsc_entry_size * sizeof (sms_ntsc_rgb_t) / 8))

#define SMS_NTSC_RGB15( ntsc, n ) \
  (sms_ntsc_rgb_t const*) ((char const*) (ntsc)->table +\
  ((n << 9 & 0x3C00) | (n & 0x03C0) | (n >> 9 & 0x003C)) *\
  (sms_ntsc_entry_size * sizeof (sms_ntsc_rgb_t) / 4))

/* common 3->7 ntsc macros */
#define SMS_NTSC_BEGIN_ROW_6_( pixel0, pixel1, pixel2, ENTRY, table ) \
  sms_ntsc_rgb_t raw_;\
  unsigned const sms_ntsc_pixel0_ = (pixel0);\
  sms_ntsc_rgb_t const* kernel0  = ENTRY( table, sms_ntsc_pixel0_ );\
  unsigned const sms_ntsc_pixel1_ = (pixel1);\
  sms_ntsc_rgb_t const* kernel1  = ENTRY( table, sms_ntsc_pixel1_ );\
  unsigned const sms_ntsc_pixel2_ = (pixel2);\
  sms_ntsc_rgb_t const* kernel2  = ENTRY( table, sms_ntsc_pixel2_ );\
  sms_ntsc_rgb_t const* kernelx0;\
  sms_ntsc_rgb_t const* kernelx1 = kernel0;\
  sms_ntsc_rgb_t const* kernelx2 = kernel0


/* common ntsc macros */
#define sms_ntsc_rgb_builder    ((1L << 21) | (1 << 11) | (1 << 1))
#define sms_ntsc_clamp_mask     (sms_ntsc_rgb_builder * 3 / 2)
#define sms_ntsc_clamp_add      (sms_ntsc_rgb_builder * 0x101)
#define SMS_NTSC_CLAMP_( io, shift ) {\
  sms_ntsc_rgb_t sub = (io) >> (9-(shift)) & sms_ntsc_clamp_mask;\
  sms_ntsc_rgb_t clamp = sms_ntsc_clamp_add - sub;\
  io |= clamp;\
  clamp -= sub;\
  io &= clamp;\
}

#define SMS_NTSC_COLOR_IN_( index, color, ENTRY, table ) {\
  unsigned color_;\
  kernelx##index = kernel##index;\
  kernel##index = (color_ = (color), ENTRY( table, color_ ));\
}

/* x is always zero except in snes_ntsc library */
#if SMS_NTSC_OUT_DEPTH == 15
#define SMS_NTSC_RGB_OUT_( rgb_out, x ) {\
    rgb_out = (raw_>>(14-x)& 0x7C00)|(raw_>>(9-x)&0x03E0)|(raw_>>(4-x)&0x001F);\
   }
#elif SMS_NTSC_OUT_DEPTH == 16
#define SMS_NTSC_RGB_OUT_( rgb_out, x) {\
    rgb_out = (raw_>>(13-x)& 0xF800)|(raw_>>(8-x)&0x07E0)|(raw_>>(4-x)&0x001F);\
   }
#endif

#ifdef __cplusplus
  }
#endif

#endif
