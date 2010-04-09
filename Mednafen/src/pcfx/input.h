#ifndef __PCFX_PAD_H
#define __PCFX_PAD_H

void FXINPUT_Init(void);
void FXINPUT_SettingChanged(const char *name);


void FXINPUT_SetInput(int port, const char *type, void *ptr);

uint16 FXINPUT_Read16(uint32 A);
uint8 FXINPUT_Read8(uint32 A);

void FXINPUT_Write8(uint32 A, uint8 V);
void FXINPUT_Write16(uint32 A, uint16 V);

void FXINPUT_Frame(void);
int FXINPUT_StateAction(StateMem *sm, int load, int data_only);

void FXINPUT_Update(void);
void FXINPUT_ResetTS(void);

extern InputInfoStruct PCFXInputInfo;

#ifdef WANT_DEBUGGER
bool FXINPUT_GetRegister(const std::string &name, uint32 &value, std::string *special);
#endif

#endif
