#ifndef COLOURS_PAL_H_
#define COLOURS_PAL_H_

#include "colours.h"
#include "colours_external.h"

/* PAL palette's current setup. */
extern Colours_setup_t COLOURS_PAL_setup;
/* External PAL palette. */
extern COLOURS_EXTERNAL_t COLOURS_PAL_external;

/* Updates the PAL palette - should be called after changing palette setup
   or loading/unloading an external palette. */
void COLOURS_PAL_Update(int colourtable[256]);

/* Restores default values for PAL-specific colour controls.
   Colours_PAL_Update should be called afterwards to apply changes. */
void COLOURS_PAL_RestoreDefaults(void);

/* Read/write to configuration file. */
int COLOURS_PAL_ReadConfig(char *option, char *ptr);
void COLOURS_PAL_WriteConfig(FILE *fp);

/* PAL Colours initialisation and processing of command-line arguments. */
int COLOURS_PAL_Initialise(int *argc, char *argv[]);

/* Function for getting the PAL-specific color preset. */
Colours_preset_t COLOURS_PAL_GetPreset(void);

#endif /* COLOURS_PAL_H_ */
