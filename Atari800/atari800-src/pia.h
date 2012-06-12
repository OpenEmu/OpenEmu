#ifndef PIA_H_
#define PIA_H_

#include "atari.h"

#define PIA_OFFSET_PORTA 0x00
#define PIA_OFFSET_PORTB 0x01
#define PIA_OFFSET_PACTL 0x02
#define PIA_OFFSET_PBCTL 0x03

extern UBYTE PIA_PACTL;
extern UBYTE PIA_PBCTL;
extern UBYTE PIA_PORTA;
extern UBYTE PIA_PORTB;
extern UBYTE PIA_PORTA_mask;
extern UBYTE PIA_PORTB_mask;
extern UBYTE PIA_PORT_input[2];

int PIA_Initialise(int *argc, char *argv[]);
void PIA_Reset(void);
UBYTE PIA_GetByte(UWORD addr);
void PIA_PutByte(UWORD addr, UBYTE byte);
void PIA_StateSave(void);
void PIA_StateRead(void);

#endif /* PIA_H_ */
