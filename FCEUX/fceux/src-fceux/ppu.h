void FCEUPPU_Init(void);
void FCEUPPU_Reset(void);
void FCEUPPU_Power(void);
void FCEUPPU_Loop(int skip);

void FCEUPPU_LineUpdate();
void FCEUPPU_SetVideoSystem(int w);

extern void (*PPU_hook)(uint32 A);
extern void (*GameHBIRQHook)(void), (*GameHBIRQHook2)(void);

/* For cart.c and banksw.h, mostly */
extern uint8 NTARAM[0x800],*vnapage[4];
extern uint8 PPUNTARAM;
extern uint8 PPUCHRRAM;

void FCEUPPU_SaveState(void);
void FCEUPPU_LoadState(int version);

#ifdef _MSC_VER
#define FASTCALL __fastcall
#else
#define FASTCALL
#endif

void PPU_ResetHooks();
extern uint8 (FASTCALL *FFCEUX_PPURead)(uint32 A);
extern void (*FFCEUX_PPUWrite)(uint32 A, uint8 V);
extern uint8 FASTCALL FFCEUX_PPURead_Default(uint32 A);
void FFCEUX_PPUWrite_Default(uint32 A, uint8 V);
extern void ppudead_loop(int newppu);

extern int ppudead;
extern int scanline;
extern uint8 PPU[4];

enum PPUPHASE {
	PPUPHASE_VBL, PPUPHASE_BG, PPUPHASE_OBJ
};

extern PPUPHASE ppuphase;
