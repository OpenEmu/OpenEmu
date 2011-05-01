#include "../mednafen.h"

#if 0

MDFNGI EmulatedDEMO =
{
 "demo",
 "Mednafen Demo/Example Module",
 KnownExtensions,
 MODPRIO_INTERNAL_LOW,
 NULL,                                          // Debugger
 &SNESInputInfo,
 Load,
 NULL,
 NULL,
 NULL,
 CloseGame,
 ToggleLayer,
 "Background\0Sprites\0",
 NULL, //InstallReadPatch,
 NULL, //RemoveReadPatches,
 NULL, //MemRead,
 NULL, //StateAction,
 Emulate,
 SetInput,
 SetSoundRate,
 DoSimpleCommand,
 SNESSettings,
 0,
 0,
 FALSE, // Multires
 320,   // Nominal width
 240,   // Nominal height
 512,  			// Framebuffer width
 256,                   // Framebuffer height
};

#endif
