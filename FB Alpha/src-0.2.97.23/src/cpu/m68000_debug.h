enum SekRegister {
	SEK_REG_D0, SEK_REG_D1, SEK_REG_D2, SEK_REG_D3, SEK_REG_D4, SEK_REG_D5, SEK_REG_D6, SEK_REG_D7,
	SEK_REG_A0, SEK_REG_A1, SEK_REG_A2, SEK_REG_A3, SEK_REG_A4, SEK_REG_A5, SEK_REG_A6, SEK_REG_A7,
	SEK_REG_PC,
	SEK_REG_SR,
	SEK_REG_SP, SEK_REG_USP, SEK_REG_ISP, SEK_REG_MSP,
	SEK_REG_VBR,
	SEK_REG_SFC, SEK_REG_DFC,
	SEK_REG_CACR, SEK_REG_CAAR
};

extern void (*SekDbgBreakpointHandlerRead)(UINT32, INT32);
extern void (*SekDbgBreakpointHandlerFetch)(UINT32, INT32);
extern void (*SekDbgBreakpointHandlerWrite)(UINT32, INT32);

void SekDbgDisableBreakpoints();
void SekDbgEnableBreakpoints();
void SekDbgEnableSingleStep();

INT32 SekDbgSetBreakpointDataRead(UINT32 nAddress, INT32 nIdentifier);
INT32 SekDbgSetBreakpointDataWrite(UINT32 nAddress, INT32 nIdentifier);
INT32 SekDbgSetBreakpointFetch(UINT32 nAddress, INT32 nIdentifier);

INT32 SekDbgGetCPUType();
INT32 SekDbgGetPendingIRQ();
UINT32 SekDbgGetRegister(SekRegister nRegister);
bool SekDbgSetRegister(SekRegister nRegister, UINT32 nValue);
