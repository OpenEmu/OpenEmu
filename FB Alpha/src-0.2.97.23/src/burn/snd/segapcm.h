#define   BANK_256    (11)
#define   BANK_512    (12)
#define   BANK_12M    (13)
#define   BANK_MASK7    (0x70<<16)
#define   BANK_MASKF    (0xf0<<16)
#define   BANK_MASKF8   (0xf8<<16)

void SegaPCMUpdate(INT16* pSoundBuf, INT32 nLength);
void SegaPCMInit(INT32 nChip, INT32 clock, INT32 bank, UINT8 *pPCMData, INT32 PCMDataSize);
void SegaPCMSetRoute(INT32 nChip, INT32 nIndex, double nVolume, INT32 nRouteDir);
void SegaPCMExit();
INT32 SegaPCMScan(INT32 nAction,INT32 *pnMin);
UINT8 SegaPCMRead(INT32 nChip, UINT32 Offset);
void SegaPCMWrite(INT32 nChip, UINT32 Offset, UINT8 Data);

#define BURN_SND_SEGAPCM_ROUTE_1		0
#define BURN_SND_SEGAPCM_ROUTE_2		1

#define SegaPCMSetAllRoutes(n, v, d)						\
	SegaPCMSetRoute(n, BURN_SND_SEGAPCM_ROUTE_1, v, d);		\
	SegaPCMSetRoute(n, BURN_SND_SEGAPCM_ROUTE_2, v, d);
