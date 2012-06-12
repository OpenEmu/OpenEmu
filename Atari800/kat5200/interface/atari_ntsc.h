
/* Atari GTIA NTSC composite video to RGB emulator/blitter */

#ifndef ATARI_NTSC_H
#define ATARI_NTSC_H

/* Picture parameters, ranging from -1.0 to 1.0 where 0.0 is normal. To easily
clear all fields, make it a static object then set whatever fields you want:
	static snes_ntsc_setup_t setup;
	setup.hue = ... */
typedef struct atari_ntsc_setup_t
{
	float hue;
	float saturation;
	float contrast;
	float brightness;
	float sharpness;
	float burst_phase; /* not in radians; -1.0 = -180 degrees, 1.0 = +180 degrees */
	float hue_warping; /* < 0 expands purple and green, > 0 expands orange and cyan */

	/* Advanced parameters */
	double gamma;
	double resolution; /* image resolution */
	//double artifacts;  /* artifacts caused by color changes */
	double fringing;   /* color artifacts caused by brightness changes */
	double bleed;      /* color bleed (color resolution reduction) */
	int merge_fields;  /* if 1, merges even and odd fields together to reduce flicker */
	float const* decoder_matrix; /* optional RGB decoder matrix, 6 elements */
	
} atari_ntsc_setup_t;

/* Video format presets */
extern atari_ntsc_setup_t const atari_ntsc_rf;        /* color bleeding + artifacts */
extern atari_ntsc_setup_t const atari_ntsc_composite; /* color bleeding + artifacts */
extern atari_ntsc_setup_t const atari_ntsc_svideo;    /* color bleeding only */
extern atari_ntsc_setup_t const atari_ntsc_rgb;       /* crisp image */
extern atari_ntsc_setup_t const atari_ntsc_monochrome;/* desaturated + artifacts */

/* private */
enum { atari_ntsc_entry_size = 56 };
enum { atari_ntsc_color_count = 256 };
typedef unsigned long ntsc_rgb_t;

/* Caller must allocate space for blitter data, which uses 56 KB of memory. */
typedef struct atari_ntsc_t
{
	ntsc_rgb_t table [atari_ntsc_color_count] [atari_ntsc_entry_size];
} atari_ntsc_t;

/* Initialize and adjust parameters. Can be called multiple times on the same
atari_ntsc_t object. */
void atari_ntsc_init( struct atari_ntsc_t*, atari_ntsc_setup_t const* setup );

/* Blit one or more scanlines of Atari 8-bit palette values to 16-bit 5-6-5 RGB output.
For every 7 output pixels, reads approximately 4 source pixels. Use constants below for
definite input and output pixel counts. */
void atari_ntsc_blit( struct atari_ntsc_t const*, unsigned char const* atari_in, long in_pitch,
		int out_width, int out_height, unsigned short* rgb_out, long out_pitch );

/* Useful values to use for output width and number of input pixels read */
enum {
	atari_ntsc_min_out_width  = 570, /* minimum width that doesn't cut off active area */
	atari_ntsc_min_in_width   = 320,
	
	atari_ntsc_full_out_width = 598, /* room for 8-pixel left & right overscan borders */
	atari_ntsc_full_in_width  = 336
};

/* supports 16-bit and 15-bit RGB output */
#ifndef ATARI_NTSC_RGB_BITS
	#define ATARI_NTSC_RGB_BITS 16
#endif

/* Number of output pixels written by low-res blitter for given input width. Width
might be rounded down slightly; use ATARI_NTSC_IN_WIDTH() on result to find rounded
value. Guaranteed not to round 256 down at all. */
#define ATARI_NTSC_OUT_WIDTH( in_width ) \
	(((in_width) - 1) / atari_ntsc_in_chunk * atari_ntsc_out_chunk + atari_ntsc_out_chunk)

/* Number of low-res input pixels that will fit within given output width. Might be
rounded down slightly; use ATARI_NTSC_OUT_WIDTH() on result to find rounded
value. */
#define ATARI_NTSC_IN_WIDTH( out_width ) \
	((out_width) / atari_ntsc_out_chunk * atari_ntsc_in_chunk - atari_ntsc_in_chunk + 1)


/* Interface for user-defined custom blitters */

enum { atari_ntsc_in_chunk    = 4  }; /* number of atari pixels read per chunk */
enum { atari_ntsc_out_chunk   = 7  }; /* number of output pixels generated per chunk */

#endif
