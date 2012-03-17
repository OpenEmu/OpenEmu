/**
 * Gens: Palette handler.
 */

#ifndef G_PALETTE_H
#define G_PALETTE_H

#ifdef __cplusplus
extern "C" {
#endif

extern int RMax_Level;
extern int GMax_Level;
extern int BMax_Level;
extern int Contrast_Level;
extern int Brightness_Level;
extern int Greyscale;
extern int Invert_Color;

void Recalculate_Palettes(void);

#ifdef __cplusplus
}
#endif

#endif
