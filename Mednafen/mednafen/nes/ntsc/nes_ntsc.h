
/* NES NTSC composite video to RGB emulator/blitter */

/* nes_ntsc 0.2.0 */

#ifndef NES_NTSC_H
#define NES_NTSC_H

/* Image parameters, ranging from -1.0 to 1.0 */
typedef struct nes_ntsc_setup_t
{
	/* Basic parameters */
	double hue;        /* -1 = -180 degrees, +1 = +180 degrees */
	double saturation; /* -1 = grayscale, +1 = oversaturated colors */
	double contrast;
	double brightness;
	double sharpness;  /* edge contrast enhancement/blurring */
	
	/* Advanced parameters */
	double gamma;
	double resolution; /* image resolution */
	double artifacts;  /* artifacts caused by color changes */
	double fringing;   /* color artifacts caused by brightness changes */
	double bleed;      /* color bleed (color resolution reduction) */
	double hue_warping;/* -1 = expand purple & green, +1 = expand orange & cyan */
	int merge_fields;  /* if 1, merges even and odd fields together to reduce flicker */
	float const* decoder_matrix; /* optional RGB decoder matrix, 6 elements */
	
	unsigned char* palette_out;  /* optional RGB palette out, 3 bytes per color */
} nes_ntsc_setup_t;

/* Video format presets */
extern nes_ntsc_setup_t const nes_ntsc_composite; /* color bleeding + artifacts */
extern nes_ntsc_setup_t const nes_ntsc_svideo;    /* color bleeding only */
extern nes_ntsc_setup_t const nes_ntsc_rgb;       /* crisp image */
extern nes_ntsc_setup_t const nes_ntsc_monochrome;/* desaturated + artifacts */

enum { nes_ntsc_palette_size      = 64 };
enum { nes_ntsc_emph_palette_size = 64 * 8 };

/* Initialize and adjust parameters. Can be called multiple times on the same
nes_ntsc_t object. Caller must allocate memory for nes_ntsc_t. Can pass 0
for either parameter. */
typedef struct nes_ntsc_t nes_ntsc_t;
void nes_ntsc_init( nes_ntsc_t* ntsc, nes_ntsc_setup_t const* setup );

/* Blit one or more rows of pixels. Input pixels are 6-bit palette indicies.
In_row_width is the number of pixels to get to the next input row. Out_pitch
is the number of *bytes* to get to the next output row. Output pixel format
is set by NES_NTSC_OUT_DEPTH (defaults to 16-bit RGB). */
void nes_ntsc_blit( nes_ntsc_t const* ntsc, unsigned char const* nes_in,
		long in_row_width, int burst_phase, int in_width, int in_height,
		void* rgb_out, long out_pitch );

/* Equivalent functions with color emphasis support. Source pixels are
9-bit values with the upper 3 bits specifying the emphasis bits from 0x2001. */
typedef struct nes_ntsc_emph_t nes_ntsc_emph_t;
void nes_ntsc_init_emph( nes_ntsc_emph_t* ntsc, nes_ntsc_setup_t const* setup );
void nes_ntsc_blit_emph( nes_ntsc_emph_t const* ntsc, unsigned short const* nes_in,
		long in_row_width, int burst_phase, int in_width, int in_height,
		void* rgb_out, long out_pitch );

/* Number of output pixels written by blitter for given input width. Width might
be rounded down slightly; use NES_NTSC_IN_WIDTH() on result to find rounded
value. Guaranteed not to round 256 down at all. */
#define NES_NTSC_OUT_WIDTH( in_width ) \
	(((in_width) - 1) / nes_ntsc_in_chunk * nes_ntsc_out_chunk + nes_ntsc_out_chunk)

/* Number of input pixels that will fit within given output width. Might be
rounded down slightly; use NES_NTSC_OUT_WIDTH() on result to find rounded
value. */
#define NES_NTSC_IN_WIDTH( out_width ) \
	((out_width) / nes_ntsc_out_chunk * nes_ntsc_in_chunk - nes_ntsc_in_chunk + 1)


/* Interface for user-defined custom blitters.
Can be used with nes_ntsc_t and nes_ntsc_emph_t */

enum { nes_ntsc_in_chunk    = 3  }; /* number of input pixels read per chunk */
enum { nes_ntsc_out_chunk   = 7  }; /* number of output pixels generated per chunk */
enum { nes_ntsc_black       = 15 }; /* palette index for black */
enum { nes_ntsc_burst_count = 3  }; /* burst phase cycles through 0, 1, and 2 */

/* Begin outputting row and start three pixels. First pixel will be cut off a bit.
Use nes_ntsc_black for unused pixels. Declares variables, so must be before first
statement in a block (unless you're using C++). */
#define NES_NTSC_BEGIN_ROW( ntsc, burst, pixel0, pixel1, pixel2 ) \
	char const* ktable = (char*) (ntsc)->table + burst * (nes_ntsc_burst_size * sizeof (ntsc_rgb_t));\
	ntsc_rgb_t const* kernel0  = NES_NTSC_ENTRY_( pixel0 );\
	ntsc_rgb_t const* kernel1  = NES_NTSC_ENTRY_( pixel1 );\
	ntsc_rgb_t const* kernel2  = NES_NTSC_ENTRY_( pixel2 );\
	ntsc_rgb_t const* kernelx0;\
	ntsc_rgb_t const* kernelx1 = kernel0;\
	ntsc_rgb_t const* kernelx2 = kernel0

/* Begin input pixel */
#define NES_NTSC_COLOR_IN( in_index, color_in ) {\
	kernelx##in_index = kernel##in_index;\
	kernel##in_index = NES_NTSC_ENTRY_( color_in );\
}

/* Generate output pixel. Bits can be 24, 16, 15, or 32 (treated as 24):
24: RRRRRRRR GGGGGGGG BBBBBBBB
16:          RRRRRGGG GGGBBBBB
15:           RRRRRGG GGGBBBBB
 0: xxxRRRRR RRRxxGGG GGGGGxxB BBBBBBBx (raw format; x = junk bits) */
#define NES_NTSC_RGB_OUT( x, rgb_out, bits ) {\
	ntsc_rgb_t raw =\
		kernel0  [x       ] + kernel1  [(x+12)%7+14] + kernel2  [(x+10)%7+28] +\
		kernelx0 [(x+7)%14] + kernelx1 [(x+ 5)%7+21] + kernelx2 [(x+ 3)%7+35];\
	NES_NTSC_CLAMP_( raw );\
	if ( bits == 16 )\
		rgb_out = (raw >> 13  & 0xF800) | (raw >> 8 & 0x07E0) | (raw >> 4 & 0x001F);\
	else if ( bits == 24 || bits == 32 )\
		rgb_out = (raw >> 5 & 0xFF0000) | (raw >> 3 & 0xFF00) | (raw >> 1 & 0xFF);\
	else if ( bits == 15 )\
		rgb_out = (raw >> 14  & 0x7C00) | (raw >> 9 & 0x03E0) | (raw >> 4 & 0x001F);\
	else\
		rgb_out = raw;\
}


/* private */

enum { nes_ntsc_entry_size = 128 };
typedef unsigned long ntsc_rgb_t;
struct nes_ntsc_t {
	ntsc_rgb_t table [nes_ntsc_palette_size * nes_ntsc_entry_size];
};
struct nes_ntsc_emph_t {
	ntsc_rgb_t table [nes_ntsc_emph_palette_size * nes_ntsc_entry_size];
};
enum { nes_ntsc_burst_size = nes_ntsc_entry_size / nes_ntsc_burst_count };

enum { ntsc_rgb_builder = (1L << 21) | (1 << 11) | (1 << 1) };
enum { nes_ntsc_clamp_mask = ntsc_rgb_builder * 3 / 2 };
enum { nes_ntsc_clamp_add  = ntsc_rgb_builder * 0x101 };

#define NES_NTSC_ENTRY_( n ) \
	(ntsc_rgb_t*) (ktable + (n) * (nes_ntsc_entry_size * sizeof (ntsc_rgb_t)))

#define NES_NTSC_CLAMP_( io ) {\
	ntsc_rgb_t sub = io >> 9 & nes_ntsc_clamp_mask;\
	ntsc_rgb_t clamp = nes_ntsc_clamp_add - sub;\
	io |= clamp;\
	clamp -= sub;\
	io &= clamp;\
}

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

#endif

