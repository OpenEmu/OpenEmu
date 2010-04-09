#ifndef _PALETTE_DRIVER_H

typedef struct {
        uint8 r,g,b;
} MDFNPalStruct;

MDFNPalStruct *MDFNI_GetDefaultPalette(void);

#define _PALETTE_DRIVER_H
#endif
