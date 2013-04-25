#ifndef COLOURS_H_
#define COLOURS_H_

#include "colours_external.h"

extern int Colours_table[256];

typedef enum {
	COLOURS_PRESET_STANDARD,
	COLOURS_PRESET_DEEPBLACK,
	COLOURS_PRESET_VIBRANT,
	COLOURS_PRESET_CUSTOM,
	/* Number of "normal" (not including CUSTOM) values in enumerator */
	COLOURS_PRESET_SIZE = COLOURS_PRESET_CUSTOM
} Colours_preset_t;

/* Contains controls for palette adjustment. These controls are available for
   NTSC and PAL palettes. */
typedef struct Colours_setup_t {
	double hue; /* TV tint control */
	double saturation;
	double contrast;
	double brightness;
	double gamma;
	/* Delay between phases of two consecutive chromas, in degrees.
	   Corresponds to the color adjustment potentiometer on the bottom of
	   Atari computers. */
	double color_delay;
	int black_level; /* 0..255. ITU-R Recommendation BT.601 advises it to be 16. */
	int white_level; /* 0..255. ITU-R Recommendation BT.601 advises it to be 235. */
} Colours_setup_t;

/* Limits for the adjustable values. */
#define COLOURS_HUE_MIN -1.0
#define COLOURS_HUE_MAX 1.0
#define COLOURS_SATURATION_MIN -1.0
#define COLOURS_SATURATION_MAX 1.0
#define COLOURS_CONTRAST_MIN -2.0
#define COLOURS_CONTRAST_MAX 2.0
#define COLOURS_BRIGHTNESS_MIN -2.0
#define COLOURS_BRIGHTNESS_MAX 2.0
#define COLOURS_GAMMA_MIN -1.0
#define COLOURS_GAMMA_MAX 1.0
#define COLOURS_DELAY_MIN 10
#define COLOURS_DELAY_MAX 50

/* Pointer to the current palette setup. Depending on the current TV system,
   it points to the NTSC setup, or the PAL setup. (See COLOURS_NTSC_setup and
   COLOURS_PAL_setup.) */
extern Colours_setup_t *Colours_setup;

#define Colours_GetR(x) ((UBYTE) (Colours_table[x] >> 16))
#define Colours_GetG(x) ((UBYTE) (Colours_table[x] >> 8))
#define Colours_GetB(x) ((UBYTE) Colours_table[x])
/* Packs R, G, B into palette COLORTABLE_PTR for colour number I. */
void Colours_SetRGB(int i, int r, int g, int b, int *colortable_ptr);

/* Called when the TV system changes, it updates the current palette
   accordingly. */
void Colours_SetVideoSystem(int mode);

/* Updates the current palette - should be called after changing palette setup
   or loading/unloading an external palette. */
void Colours_Update(void);
/* Restores default setup for the current palette (NTSC or PAL one).
   Colours_Update should be called afterwards to apply changes. */
void Colours_RestoreDefaults(void);
/* Save the current colours, including adjustments, to a palette file.
   Returns TRUE on success or FALSE on error. */
int Colours_Save(const char *filename);

/* Pointer to an externally-loaded palette. Depending on the current TV
   system, it points to the external NTSC or PAL palette - they can be loaded
   independently. (See COLOURS_NTSC_external and COLOURS_PAL_external.) */
extern COLOURS_EXTERNAL_t *Colours_external;

/* Initialise variables before loading from config file. */
void Colours_PreInitialise(void);

/* Read/write to configuration file. */
int Colours_ReadConfig(char *option, char *ptr);
void Colours_WriteConfig(FILE *fp);

/* Colours initialisation and processing of command-line arguments. */
int Colours_Initialise(int *argc, char *argv[]);

/* Functions for setting and getting the color preset. PRESET cannot equal
   COLOURS_PRESET_CUSTOM. */
void Colours_SetPreset(Colours_preset_t preset);
Colours_preset_t Colours_GetPreset(void);

#endif /* COLOURS_H_ */
