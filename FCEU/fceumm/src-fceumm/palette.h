#ifndef _FCEU_PALETTE_H
#define _FCEU_PALETTE_H

#include "types.h"

typedef struct {
  uint8 r,g,b;
} pal;

extern pal *palo;

#ifdef __cplusplus
extern "C" {
#endif

void FCEU_ResetPalette(void);
void FCEU_LoadGamePalette(void);

#ifdef __cplusplus
}
#endif

#endif
