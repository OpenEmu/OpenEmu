#ifndef FILTER_NTSC_H_
#define FILTER_NTSC_H_

#include <stdio.h>
#include "atari_ntsc/atari_ntsc.h"

/* Limits for the adjustable values. */
#define FILTER_NTSC_SHARPNESS_MIN -1.0
#define FILTER_NTSC_SHARPNESS_MAX 1.0
#define FILTER_NTSC_RESOLUTION_MIN -1.0
#define FILTER_NTSC_RESOLUTION_MAX 1.0
#define FILTER_NTSC_ARTIFACTS_MIN -1.0
#define FILTER_NTSC_ARTIFACTS_MAX 1.0
#define FILTER_NTSC_FRINGING_MIN -1.0
#define FILTER_NTSC_FRINGING_MAX 1.0
#define FILTER_NTSC_BLEED_MIN -1.0
#define FILTER_NTSC_BLEED_MAX 1.0
#define FILTER_NTSC_BURST_PHASE_MIN -1.0
#define FILTER_NTSC_BURST_PHASE_MAX 1.0

/* Contains controls used to adjust the palette in the NTSC filter. */
extern atari_ntsc_setup_t FILTER_NTSC_setup;
/* Pointer to the NTSC filter structure. Initialise it by setting it to value
   returned by FILTER_NTSC_New(). */
extern atari_ntsc_t *FILTER_NTSC_emu;

/* Allocates memory for a new NTSC filter. */
atari_ntsc_t *FILTER_NTSC_New(void);
/* Frees memory used by an NTSC filter, FILTER. */
void FILTER_NTSC_Delete(atari_ntsc_t *filter);
/* Reinitialises an NTSC filter, FILTER. Should be called after changing
   palette setup or loading/unloading an external palette. */
void FILTER_NTSC_Update(atari_ntsc_t *filter);
/* Restores default values for NTSC-filter-specific colour controls.
   FILTER_NTSC_Update should be called afterwards to apply changes. */
void FILTER_NTSC_RestoreDefaults(void);

/* Set/get one of the available preset adjustments: Composite, S-Video, RGB,
   Monochrome. */
enum {
	FILTER_NTSC_PRESET_COMPOSITE,
	FILTER_NTSC_PRESET_SVIDEO,
	FILTER_NTSC_PRESET_RGB,
	FILTER_NTSC_PRESET_MONOCHROME,
	FILTER_NTSC_PRESET_CUSTOM,
	/* Number of "normal" (not including CUSTOM) values in enumerator */
	FILTER_NTSC_PRESET_SIZE = FILTER_NTSC_PRESET_CUSTOM
};
/* FILTER_NTSC_Update should be called afterwards these functions to apply changes. */
void FILTER_NTSC_SetPreset(int preset);
int FILTER_NTSC_GetPreset(void);
void FILTER_NTSC_NextPreset(void);

/* Initialise variables before loading from config file. */
void FILTER_NTSC_PreInitialise(void);

/* Read/write to configuration file. */
int FILTER_NTSC_ReadConfig(char *option, char *ptr);
void FILTER_NTSC_WriteConfig(FILE *fp);

/* NTSC filter initialisation and processing of command-line arguments. */
int FILTER_NTSC_Initialise(int *argc, char *argv[]);

#endif /* FILTER_NTSC_H_ */
