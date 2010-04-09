#ifndef _PCE_VDC_H
#define _PCE_VDC_H
void VDC_SetPixelFormat(int rshift, int gshift, int bshift);
void VDC_RunFrame(uint32 *pXBuf, MDFN_Rect *LineWidths, int skip);
bool VDC_ToggleLayer(int which);

void VDC_BP_Start(void);

DECLFR(VDC_Read_TestBP);
DECLFR(VDC_Read);

DECLFW(VDC_Write_TestBP);
DECLFW(VDC_Write);

DECLFW(VDC_Write_ST_TestBP);
DECLFW(VDC_Write_ST);

DECLFR(VCE_Read);
DECLFW(VCE_Write);

void VDC_Init(int sgx);
void VDC_Close(void);
void VDC_Reset(void);
void VDC_Power(void);
uint32 VDC_GetRegister(const std::string &name, std::string *special);
void VDC_SetRegister(std::string name, uint32 value);

int VDC_StateAction(StateMem *sm, int load, int data_only);

void VDC_SetGraphicsDecode(int line, int which, int w, int h, int xscroll, int yscroll, int pbn);
uint32 *VDC_GetGraphicsDecodeBuffer(void);

void VDC_GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer);
void VDC_PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, const uint8 *Buffer);

#endif
