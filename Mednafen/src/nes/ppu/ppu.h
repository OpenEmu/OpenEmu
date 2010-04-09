#ifndef __NES_PPU_H
#define __NES_PPU_H

void MDFNPPU_Init(void);
void MDFNPPU_Reset(void);
void MDFNPPU_Power(void);
int MDFNPPU_Loop(int skip);

void MDFNPPU_LineUpdate();

extern void (*PPU_hook)(uint32 A);
extern void (*GameHBIRQHook)(void), (*GameHBIRQHook2)(void);

/* For cart.c and banksw.h, mostly */
extern uint8 NTARAM[0x800],*vnapage[4];
extern uint8 PPUNTARAM;
extern uint8 PPUCHRRAM;

void MDFNPPU_SaveState(void);
void MDFNPPU_LoadState(int version);

extern int scanline;

void MDFNNES_SetPixelFormat(int r, int g, int b);
int MDFNPPU_StateAction(StateMem *sm, int load, int data_only);
bool MDFNNES_ToggleLayer(int which);
uint32 NESPPU_GetRegister(const std::string &name);
void NESPPU_SetRegister(const std::string &name, uint32 value);


void NESPPU_SetGraphicsDecode(int line, int which, int w, int h, int xscroll, int yscroll, int pbn);
uint32 *NESPPU_GetGraphicsDecodeBuffer(void);
void NESPPU_SettingChanged(const char *name);

void NESPPU_GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer);
void NESPPU_PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer);

#endif
