#include "../nes.h"
#include "../input.h"
#include "../ppu/ppu.h"
#include "../x6502.h"
#include "../ppu/palette.h"
#include "../../endian.h"

void MDFN_DrawCursor(MDFN_Surface *surface, int xc, int yc);
void MDFN_DrawGunSight(MDFN_Surface *surface, int xc, int yc);
