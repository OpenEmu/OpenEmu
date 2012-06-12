#ifndef COLOURS_NTSC_H_
#define COLOURS_NTSC_H_

#include "config.h"
#include "colours.h"
#include "colours_external.h"

#ifndef M_PI
# define M_PI 3.141592653589793
#endif

/* Contains NTSC-specific controls for palette adjustment. */
typedef struct COLOURS_NTSC_setup_t {
	double hue; /* Like TV tint control */
	/* Delay between phases of two consecutive chromas, in nanoseconds;
	normally 21. Corresponds to the R38 potentiometer on the bottom of
	Atari computers. */
	double color_delay; 
} COLOURS_NTSC_setup_t;

/* Limits for the adjustable values. */
#define COLOURS_NTSC_HUE_MIN -1.0
#define COLOURS_NTSC_HUE_MAX 1.0
#define COLOURS_NTSC_DELAY_MIN 16
#define COLOURS_NTSC_DELAY_MAX 26

/* NTSC palette's current setup - generic controls. */
extern Colours_setup_t COLOURS_NTSC_setup;
/* NTSC palette's current setup - NTSC-specific controls. */
extern COLOURS_NTSC_setup_t COLOURS_NTSC_specific_setup;
/* External NTSC palette. */
extern COLOURS_EXTERNAL_t COLOURS_NTSC_external;

/* Updates the NTSC palette - should be called after changing palette setup
   or loading/unloading an external palette. */
void COLOURS_NTSC_Update(int colourtable[256]);
/* Restores default values for NTSC-specific colour controls.
   Colours_NTSC_Update should be called afterwards to apply changes. */
void COLOURS_NTSC_RestoreDefaults(void);

/* Writes the NTSC palette (internal or external) as YIQ triplets in
   YIQ_TABLE. START_ANGLE defines the phase shift of chroma #1. This function
   is to be used exclusively by the NTSC Filter module. */
void COLOURS_NTSC_GetYIQ(double yiq_table[768], const double start_angle);

/* Initialise variables before loading from config file. */
void COLOURS_NTSC_PreInitialise(void);

/* Read/write to configuration file. */
int COLOURS_NTSC_ReadConfig(char *option, char *ptr);
void COLOURS_NTSC_WriteConfig(FILE *fp);

/* NTSC Colours initialisation and processing of command-line arguments. */
int COLOURS_NTSC_Initialise(int *argc, char *argv[]);

/* Functions for setting and getting the NTSC color preset. */
void COLOURS_NTSC_SetPreset(Colours_preset_t preset);
Colours_preset_t COLOURS_NTSC_GetPreset(void);

#endif /* COLOURS_NTSC_H_ */
