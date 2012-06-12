#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "config.h"
#include <stdio.h>
#if SUPPORTS_CHANGE_VIDEOMODE
#include "videomode.h"
#endif

/* This include file defines prototypes for platform-specific functions. */

int PLATFORM_Initialise(int *argc, char *argv[]);
int PLATFORM_Exit(int run_monitor);
int PLATFORM_Keyboard(void);
void PLATFORM_DisplayScreen(void);

int PLATFORM_PORT(int num);
int PLATFORM_TRIG(int num);

#ifdef SUPPORTS_PLATFORM_CONFIGINIT
/* This function sets the configuration parameters to default values */
void PLATFORM_ConfigInit(void);
#endif

#ifdef SUPPORTS_PLATFORM_CONFIGURE
/* This procedure processes lines not recognized by RtConfigLoad. */
int PLATFORM_Configure(char *option, char *parameters);
#endif

#ifdef SUPPORTS_PLATFORM_CONFIGSAVE
/* This function saves additional config lines */
void PLATFORM_ConfigSave(FILE *fp);
#endif

#ifdef SUPPORTS_PLATFORM_PALETTEUPDATE
/* This function updates the palette */
/* If the platform does a conversion of colortable when it initialises
 * and the user changes colortable (such as changing from PAL to NTSC)
 * then this function should update the platform palette */
void PLATFORM_PaletteUpdate(void);
#endif

#ifdef SUPPORTS_PLATFORM_SLEEP
/* This function is for those ports that need their own version of sleep */
void PLATFORM_Sleep(double s);
#endif

#ifdef SDL
/* used in UI to show how the keyboard joystick is mapped */
extern int PLATFORM_kbd_joy_0_enabled;
extern int PLATFORM_kbd_joy_1_enabled;
int PLATFORM_GetRawKey(void);
#endif

#ifdef DIRECTX
int PLATFORM_GetKeyName(void);
#endif

#ifdef SYNCHRONIZED_SOUND
/* This function returns a number which is used to adjust the speed
 * of execution to synchronize with the sound output */
double PLATFORM_AdjustSpeed(void);
#endif /* SYNCHRONIZED SOUND */

#if SUPPORTS_CHANGE_VIDEOMODE
/* Returns whether the platform-specific code support the given display mode, MODE,
   with/without stretching and with/without rotation. */
int PLATFORM_SupportsVideomode(VIDEOMODE_MODE_t mode, int stretch, int rotate90);
/* Sets the screen (or window, if WINDOWED is TRUE) to resolution RES and
   selects the display mode, MODE. If ROTATE90 is TRUE, then the display area
   is rotated 90 degrees counter-clockwise. */
void PLATFORM_SetVideoMode(VIDEOMODE_resolution_t const *res, int windowed, VIDEOMODE_MODE_t mode, int rotate90);
/* Returns list of all available resolutions. The result points to a newly
   allocated memory. If no resolutions are found, the result is NULL and no
   memory is allocated. Entries on the list may repeat.*/
VIDEOMODE_resolution_t *PLATFORM_AvailableResolutions(unsigned int *size);
/* Returns the current desktop resolution. Used to compute pixel aspect ratio in windowed modes. */
VIDEOMODE_resolution_t *PLATFORM_DesktopResolution(void);
/* When in windowed mode, returns whether the application window is maximised. */
int PLATFORM_WindowMaximised(void);

#endif /* SUPPORTS_CHANGE_VIDEOMODE */

#endif /* PLATFORM_H_ */
