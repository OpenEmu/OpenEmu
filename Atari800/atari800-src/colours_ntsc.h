#ifndef COLOURS_NTSC_H_
#define COLOURS_NTSC_H_

#include "config.h"
#include "colours.h"
#include "colours_external.h"

#ifndef M_PI
# define M_PI 3.141592653589793
#endif

/* NTSC palette's current setup - generic controls. */
extern Colours_setup_t COLOURS_NTSC_setup;
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

/* Read/write to configuration file. */
int COLOURS_NTSC_ReadConfig(char *option, char *ptr);
void COLOURS_NTSC_WriteConfig(FILE *fp);

/* NTSC Colours initialisation and processing of command-line arguments. */
int COLOURS_NTSC_Initialise(int *argc, char *argv[]);

/* Function for getting the NTSC-specific color preset. */
Colours_preset_t COLOURS_NTSC_GetPreset(void);

#endif /* COLOURS_NTSC_H_ */
