#ifndef __PCE_INPUT_H
#define __PCE_INPUT_H

namespace MDFN_IEN_PCE
{

class PCE_Input_Device
{
 public:
// PCE_Input_Device(int which);	// "which" is advisory and only should be used in status messages.
 virtual ~PCE_Input_Device();
 virtual void AdjustTS(int32 delta);
 virtual void Write(int32 timestamp, bool old_SEL, bool new_SEL, bool old_CLR, bool new_CLR);
 virtual uint8 Read(int32 timestamp);
 virtual void Power(int32 timestamp);
 virtual void Update(const void *data);
 virtual int StateAction(StateMem *sm, int load, int data_only, const char *section_name);
};

void PCEINPUT_Power(int32 timestamp);
void PCEINPUT_Init(void);
void PCEINPUT_SettingChanged(const char *name);
void PCEINPUT_SetInput(int port, const char *type, void *ptr);
uint8 INPUT_Read(int32 timestamp, unsigned int A);
void INPUT_Write(int32 timestamp, unsigned int A, uint8 V);
void INPUT_Frame(void);
int INPUT_StateAction(StateMem *sm, int load, int data_only);
extern InputInfoStruct PCEInputInfo;
void INPUT_FixTS(int32 timestamp);

uint16 INPUT_HESHack(void);

};

#endif
