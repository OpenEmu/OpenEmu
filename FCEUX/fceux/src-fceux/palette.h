#ifndef __FCEU_PALETTE_H
#define __FCEU_PALETTE_H
#include <stdint.h>
typedef struct {
	uint8_t r,g,b;
} pal;

extern pal *palo;
void FCEU_ResetPalette(void);

void FCEU_ResetPalette(void);
void FCEU_LoadGamePalette(void);

#endif
