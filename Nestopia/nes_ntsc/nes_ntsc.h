/* NES NTSC video filter */

/* nes_ntsc 0.2.2 */
#ifndef NES_NTSC_H
#define NES_NTSC_H

#include "nes_ntsc_config.h"

#ifdef __cplusplus
	extern "C" {
#endif

/* Image parameters, ranging from -1.0 to 1.0. Actual internal values shown
in parenthesis and should remain fairly stable in future versions. */
typedef struct nes_ntsc_setup_t
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
	int merge_fields;  /* if 1, merges even and odd fields together to reduce flicker */
	float const* decoder_matrix; /* optional RGB decoder matrix, 6 elements */
	
	unsigned char* palette_out;  /* optional RGB palette out, 3 bytes per color */
	
	/* You can replace the standard NES color generation with an RGB palette. The
	first replaces all color generation, while the second replaces only the core
	64-color generation and does standard color emphasis calculations on it. */
	unsigned char const* palette;/* optional 512-entry RGB palette in, 3 bytes per color */
	unsigned char const* base_palette;/* optional 64-entry RGB palette in, 3 bytes per color */
} nes_ntsc_setup_t;

/* Video format presets */
extern nes_ntsc_setup_t const nes_ntsc_composite; /* color bleeding + artifacts */
extern nes_ntsc_setup_t const nes_ntsc_svideo;    /* color bleeding only */
extern nes_ntsc_setup_t const nes_ntsc_rgb;       /* crisp image */
extern nes_ntsc_setup_t const nes_ntsc_monochrome;/* desaturated + artifacts */

#ifdef NES_NTSC_EMPHASIS
	enum { nes_ntsc_palette_size = 64 * 8 };
#else
	enum { nes_ntsc_palette_size = 64 };
#endif

/* Initializes and adjusts parameters. Can be called multiple times on the same
nes_ntsc_t object. Can pass NULL for either parameter. */
typedef struct nes_ntsc_t nes_ntsc_t;
void nes_ntsc_init( nes_ntsc_t* ntsc, nes_ntsc_setup_t const* setup );

/* Filters one or more rows of pixels. Input pixels are 6/9-bit palette indicies.
In_row_width is the number of pixels to get to the next input row. Out_pitch
is the number of *bytes* to get to the next output row. Output pixel format
is set by NES_NTSC_OUT_DEPTH (defaults to 16-bit RGB). */
void nes_ntsc_blit( nes_ntsc_t const* ntsc, NES_NTSC_IN_T const* nes_in,
		long in_row_width, int burst_phase, int in_width, int in_height,
		void* rgb_out, long out_pitch );

/* Number of output pixels written by blitter for given input width. Width might
be rounded down slightly; use NES_NTSC_IN_WIDTH() on result to find rounded
value. Guaranteed not to round 256 down at all. */
#define NES_NTSC_OUT_WIDTH( in_width ) \
	((((in_width) - 1) / nes_ntsc_in_chunk + 1) * nes_ntsc_out_chunk)

/* Number of input pixels that will fit within given output width. Might be
rounded down slightly; use NES_NTSC_OUT_WIDTH() on result to find rounded
value. */
#define NES_NTSC_IN_WIDTH( out_width ) \
	(((out_width) / nes_ntsc_out_chunk - 1) * nes_ntsc_in_chunk + 1)


/* Interface for user-defined custom blitters */

enum { nes_ntsc_in_chunk    = 3  }; /* number of input pixels read per chunk */
enum { nes_ntsc_out_chunk   = 7  }; /* number of output pixels generated per chunk */
enum { nes_ntsc_black       = 15 }; /* palette index for black */
enum { nes_ntsc_burst_count = 3  }; /* burst phase cycles through 0, 1, and 2 */

/* Begins outputting row and starts three pixels. First pixel will be cut off a bit.
Use nes_ntsc_black for unused pixels. Declares variables, so must be before first
statement in a block (unless you're using C++). */
#define NES_NTSC_BEGIN_ROW( ntsc, burst, pixel0, pixel1, pixel2 ) \
	char const* const ktable = \
		(char const*) (ntsc)->table [0] + burst * (nes_ntsc_burst_size * sizeof (nes_ntsc_rgb_t));\
	NES_NTSC_BEGIN_ROW_6_( pixel0, pixel1, pixel2, NES_NTSC_ENTRY_, ktable )

/* Begins input pixel */
#define NES_NTSC_COLOR_IN( in_index, color_in ) \
	NES_NTSC_COLOR_IN_( in_index, color_in, NES_NTSC_ENTRY_, ktable )

/* Generates output pixel. Bits can be 24, 16, 15, 32 (treated as 24), or 0:
24:          RRRRRRRR GGGGGGGG BBBBBBBB (8-8-8 RGB)
16:                   RRRRRGGG GGGBBBBB (5-6-5 RGB)
15:                    RRRRRGG GGGBBBBB (5-5-5 RGB)
 0: xxxRRRRR RRRxxGGG GGGGGxxB BBBBBBBx (native internal format; x = junk bits) */
#define NES_NTSC_RGB_OUT( index, rgb_out, bits ) \
	NES_NTSC_RGB_OUT_14_( index, rgb_out, bits, 0 )


/* private */
enum { nes_ntsc_entry_size = 128 };
typedef unsigned long nes_ntsc_rgb_t;
struct nes_ntsc_t {
	nes_ntsc_rgb_t table [nes_ntsc_palette_size] [nes_ntsc_entry_size];
};
enum { nes_ntsc_burst_size = nes_ntsc_entry_size / nes_ntsc_burst_count };

#define NES_NTSC_ENTRY_( ktable, n ) \
	(nes_ntsc_rgb_t const*) (ktable + (n) * (nes_ntsc_entry_size * sizeof (nes_ntsc_rgb_t)))

/* deprecated */
#define NES_NTSC_RGB24_OUT( x, out ) NES_NTSC_RGB_OUT( x, out, 24 )
#define NES_NTSC_RGB16_OUT( x, out ) NES_NTSC_RGB_OUT( x, out, 16 )
#define NES_NTSC_RGB15_OUT( x, out ) NES_NTSC_RGB_OUT( x, out, 15 )
#define NES_NTSC_RAW_OUT( x, out )   NES_NTSC_RGB_OUT( x, out,  0 )

enum { nes_ntsc_min_in_width  = 256 };
enum { nes_ntsc_min_out_width = NES_NTSC_OUT_WIDTH( nes_ntsc_min_in_width ) };

enum { nes_ntsc_640_in_width  = 271 };
enum { nes_ntsc_640_out_width = NES_NTSC_OUT_WIDTH( nes_ntsc_640_in_width ) };
enum { nes_ntsc_640_overscan_left  = 8 };
enum { nes_ntsc_640_overscan_right = nes_ntsc_640_in_width - 256 - nes_ntsc_640_overscan_left };

enum { nes_ntsc_full_in_width  = 283 };
enum { nes_ntsc_full_out_width = NES_NTSC_OUT_WIDTH( nes_ntsc_full_in_width ) };
enum { nes_ntsc_full_overscan_left  = 16 };
enum { nes_ntsc_full_overscan_right = nes_ntsc_full_in_width - 256 - nes_ntsc_full_overscan_left };

/* common 3->7 ntsc macros */
#define NES_NTSC_BEGIN_ROW_6_( pixel0, pixel1, pixel2, ENTRY, table ) \
	unsigned const nes_ntsc_pixel0_ = (pixel0);\
	nes_ntsc_rgb_t const* kernel0  = ENTRY( table, nes_ntsc_pixel0_ );\
	unsigned const nes_ntsc_pixel1_ = (pixel1);\
	nes_ntsc_rgb_t const* kernel1  = ENTRY( table, nes_ntsc_pixel1_ );\
	unsigned const nes_ntsc_pixel2_ = (pixel2);\
	nes_ntsc_rgb_t const* kernel2  = ENTRY( table, nes_ntsc_pixel2_ );\
	nes_ntsc_rgb_t const* kernelx0;\
	nes_ntsc_rgb_t const* kernelx1 = kernel0;\
	nes_ntsc_rgb_t const* kernelx2 = kernel0

#define NES_NTSC_RGB_OUT_14_( x, rgb_out, bits, shift ) {\
	nes_ntsc_rgb_t raw_ =\
		kernel0  [x       ] + kernel1  [(x+12)%7+14] + kernel2  [(x+10)%7+28] +\
		kernelx0 [(x+7)%14] + kernelx1 [(x+ 5)%7+21] + kernelx2 [(x+ 3)%7+35];\
	NES_NTSC_CLAMP_( raw_, shift );\
	NES_NTSC_RGB_OUT_( rgb_out, bits, shift );\
}

/* common ntsc macros */
#define nes_ntsc_rgb_builder    ((1L << 21) | (1 << 11) | (1 << 1))
#define nes_ntsc_clamp_mask     (nes_ntsc_rgb_builder * 3 / 2)
#define nes_ntsc_clamp_add      (nes_ntsc_rgb_builder * 0x101)
#define NES_NTSC_CLAMP_( io, shift ) {\
	nes_ntsc_rgb_t sub = (io) >> (9-(shift)) & nes_ntsc_clamp_mask;\
	nes_ntsc_rgb_t clamp = nes_ntsc_clamp_add - sub;\
	io |= clamp;\
	clamp -= sub;\
	io &= clamp;\
}

#define NES_NTSC_COLOR_IN_( index, color, ENTRY, table ) {\
	unsigned color_;\
	kernelx##index = kernel##index;\
	kernel##index = (color_ = (color), ENTRY( table, color_ ));\
}

/* x is always zero except in snes_ntsc library */
#define NES_NTSC_RGB_OUT_( rgb_out, bits, x ) {\
	if ( bits == 16 )\
		rgb_out = (raw_>>(13-x)& 0xF800)|(raw_>>(8-x)&0x07E0)|(raw_>>(4-x)&0x001F);\
	if ( bits == 24 || bits == 32 )\
		rgb_out = (raw_>>(5-x)&0xFF0000)|(raw_>>(3-x)&0xFF00)|(raw_>>(1-x)&0xFF);\
	if ( bits == 15 )\
		rgb_out = (raw_>>(14-x)& 0x7C00)|(raw_>>(9-x)&0x03E0)|(raw_>>(4-x)&0x001F);\
	if ( bits == 0 )\
		rgb_out = raw_ << x;\
}

#ifdef __cplusplus
	}
#endif

#endif
