#include "burnint.h"

#ifndef HARDWARE_CAPCOM_CPS3
	#define HARDWARE_CAPCOM_CPS3		HARDWARE_MISC_MISC
#endif

#ifndef HARDWARE_CAPCOM_CPS3_NO_CD
	#define HARDWARE_CAPCOM_CPS3_NO_CD	HARDWARE_CAPCOM_CPS3
#endif

extern UINT8 cps3_reset;
extern UINT8 cps3_palette_change;

extern UINT16 *Cps3CurPal;

extern UINT32 cps3_key1, cps3_key2, cps3_isSpecial;
extern UINT32 cps3_bios_test_hack, cps3_game_test_hack;
extern UINT32 cps3_speedup_ram_address, cps3_speedup_code_address;
extern UINT8 cps3_dip;
extern UINT32 cps3_region_address, cps3_ncd_address;

extern UINT8 Cps3But1[16];
extern UINT8 Cps3But2[16];
extern UINT8 Cps3But3[16];

INT32 cps3Init();
INT32 cps3Exit();
INT32 cps3Frame();
INT32 cps3Scan(INT32 nAction,INT32 *pnMin);

// sound 

UINT8 __fastcall cps3SndReadByte(UINT32 addr);
UINT16 __fastcall cps3SndReadWord(UINT32 addr);
UINT32 __fastcall cps3SndReadLong(UINT32 addr);

void __fastcall cps3SndWriteByte(UINT32 addr, UINT8 data);
void __fastcall cps3SndWriteWord(UINT32 addr, UINT16 data);
void __fastcall cps3SndWriteLong(UINT32 addr, UINT32 data);

INT32 cps3SndInit(UINT8 *);
void cps3SndSetRoute(INT32 nIndex, double nVolume, INT32 nRouteDir);
void cps3SndReset();
void cps3SndExit();
void cps3SndUpdate();

INT32 cps3SndScan(INT32);

#define BURN_SND_CPS3SND_ROUTE_1		0
#define BURN_SND_CPS3SND_ROUTE_2		1

#define cps3SndSetAllRoutes(v, d)						\
	cps3SndSetRoute(BURN_SND_CPS3SND_ROUTE_1, v, d);	\
	cps3SndSetRoute(BURN_SND_CPS3SND_ROUTE_2, v, d);
