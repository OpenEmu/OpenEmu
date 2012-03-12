#define SEGA_MD_ROM_LOAD_NORMAL										0x10
#define SEGA_MD_ROM_LOAD16_WORD_SWAP								0x20
#define SEGA_MD_ROM_LOAD16_BYTE										0x30
#define SEGA_MD_ROM_LOAD16_WORD_SWAP_CONTINUE_040000_100000			0x40
#define SEGA_MD_ROM_OFFS_000000										0x01
#define SEGA_MD_ROM_OFFS_000001										0x02
#define SEGA_MD_ROM_OFFS_020000										0x03
#define SEGA_MD_ROM_OFFS_080000										0x04
#define SEGA_MD_ROM_OFFS_100000										0x05
#define SEGA_MD_ROM_OFFS_100001										0x06
#define SEGA_MD_ROM_OFFS_200000										0x07

extern UINT8 MegadriveReset;
extern UINT8 bMegadriveRecalcPalette;
extern UINT8 MegadriveJoy1[12];
extern UINT8 MegadriveJoy2[12];
extern UINT8 MegadriveJoy3[12];
extern UINT8 MegadriveJoy4[12];
extern UINT8 MegadriveDIP[2];
extern UINT16 *MegadriveCurPal;

INT32 MegadriveInit();
INT32 MegadriveExit();
INT32 MegadriveFrame();
INT32 MegadriveScan(INT32 nAction, INT32 *pnMin);
