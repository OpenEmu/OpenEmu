#ifndef __VB_VIP_H
#define __VB_VIP_H

namespace MDFN_IEN_VB
{

bool VIP_Init(void);
void VIP_Power(void);

void VIP_SetInstantDisplayHack(bool);
void VIP_SetAllowDrawSkip(bool);
void VIP_Set3DMode(uint32 mode, bool reverse, uint32 prescale, uint32 sbs_separation);
void VIP_SetParallaxDisable(bool disabled);
void VIP_SetDefaultColor(uint32 default_color);
void VIP_SetAnaglyphColors(uint32 lcolor, uint32 rcolor);	// R << 16, G << 8, B << 0

v810_timestamp_t MDFN_FASTCALL VIP_Update(const v810_timestamp_t timestamp);
void VIP_ResetTS(void);

void VIP_StartFrame(EmulateSpecStruct *espec);

uint8 VIP_Read8(v810_timestamp_t &timestamp, uint32 A);
uint16 VIP_Read16(v810_timestamp_t &timestamp, uint32 A);


void VIP_Write8(v810_timestamp_t &timestamp, uint32 A, uint8 V);
void VIP_Write16(v810_timestamp_t &timestamp, uint32 A, uint16 V);



int VIP_StateAction(StateMem *sm, int load, int data_only);


enum
{
 VIP_GSREG_IPENDING = 0,	// Current pending interrupt(bits)
 VIP_GSREG_IENABLE,

 VIP_GSREG_DPCTRL,

 VIP_GSREG_BRTA,
 VIP_GSREG_BRTB,
 VIP_GSREG_BRTC,
 VIP_GSREG_REST,
 VIP_GSREG_FRMCYC,
 VIP_GSREG_XPCTRL,

 VIP_GSREG_SPT0,
 VIP_GSREG_SPT1,
 VIP_GSREG_SPT2,
 VIP_GSREG_SPT3,

 VIP_GSREG_GPLT0,
 VIP_GSREG_GPLT1,
 VIP_GSREG_GPLT2,
 VIP_GSREG_GPLT3,

 VIP_GSREG_JPLT0,
 VIP_GSREG_JPLT1,
 VIP_GSREG_JPLT2,
 VIP_GSREG_JPLT3,

 VIP_GSREG_BKCOL,
};

uint32 VIP_GetRegister(const unsigned int id, char *special, const uint32 special_len);
void VIP_SetRegister(const unsigned int id, const uint32 value);


}
#endif
