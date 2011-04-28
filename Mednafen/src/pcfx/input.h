#ifndef __PCFX_PAD_H
#define __PCFX_PAD_H

enum
{
 FX_SIG_MOUSE = 0xD,
 FX_SIG_TAP = 0xE,
 FX_SIG_PAD = 0xF
};

class PCFX_Input_Device
{
 public:
// PCFX_Input_Device(int which); // "which" is advisory and only should be used in status messages.

 virtual ~PCFX_Input_Device();

 virtual uint32 ReadTransferTime(void);
 virtual uint32 WriteTransferTime(void);

 virtual uint32 Read(void);
 virtual void Write(uint32 data);


 virtual void Power(void);

 virtual void Frame(const void *data);
 virtual int StateAction(StateMem *sm, int load, int data_only, const char *section_name);
};


void FXINPUT_Init(void);
void FXINPUT_SettingChanged(const char *name);


void FXINPUT_SetInput(int port, const char *type, void *ptr);

uint16 FXINPUT_Read16(uint32 A, const v810_timestamp_t timestamp);
uint8 FXINPUT_Read8(uint32 A, const v810_timestamp_t timestamp);

void FXINPUT_Write8(uint32 A, uint8 V, const v810_timestamp_t timestamp);
void FXINPUT_Write16(uint32 A, uint16 V, const v810_timestamp_t timestamp);

void FXINPUT_Frame(void);
int FXINPUT_StateAction(StateMem *sm, int load, int data_only);

v810_timestamp_t FXINPUT_Update(const v810_timestamp_t timestamp);
void FXINPUT_ResetTS(void);

extern InputInfoStruct PCFXInputInfo;

#ifdef WANT_DEBUGGER
bool FXINPUT_GetRegister(const std::string &name, uint32 &value, std::string *special);
#endif

#endif
