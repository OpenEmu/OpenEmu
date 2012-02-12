extern uint8 MMC3_cmd;
extern uint8 EXPREGS[8];
extern uint8 DRegBuf[8];

#undef IRQCount
#undef IRQLatch
#undef IRQa
extern uint8 IRQCount,IRQLatch,IRQa;
extern uint8 IRQReload;

extern void (*pwrap)(uint32 A, uint8 V);
extern void (*cwrap)(uint32 A, uint8 V);
extern void (*mwrap)(uint8 V);

void GenMMC3Power(void);
void GenMMC3Restore(int version);
void MMC3RegReset(void);
void FixMMC3PRG(int V);
void FixMMC3CHR(int V);
DECLFW(MMC3_CMDWrite);
DECLFW(MMC3_IRQWrite);

void GenMMC3_Init(CartInfo *info, int prg, int chr, int wram, int battery);
