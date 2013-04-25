#ifndef PBI_BB_H_
#define PBI_BB_H_

#include "atari.h"
#include <stdio.h>

extern int PBI_BB_enabled;
void PBI_BB_Menu(void);
void PBI_BB_Frame(void);
int PBI_BB_Initialise(int *argc, char *argv[]);
void PBI_BB_Exit(void);
UBYTE PBI_BB_D1GetByte(UWORD addr, int no_side_effects);
void PBI_BB_D1PutByte(UWORD addr, UBYTE byte);
UBYTE PBI_BB_D6GetByte(UWORD addr, int no_side_effects);
void PBI_BB_D6PutByte(UWORD addr, UBYTE byte);
int PBI_BB_ReadConfig(char *string, char *ptr);
void PBI_BB_WriteConfig(FILE *fp);
void PBI_BB_StateSave(void);
void PBI_BB_StateRead(void);

#endif /* PBI_BB_H_ */
