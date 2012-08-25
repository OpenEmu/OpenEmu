void RF5C68PCMUpdate(INT16* pSoundBuf, INT32 length);
void RF5C68PCMReset();
void RF5C68PCMInit(INT32 clock);
void RF5C68PCMSetRoute(INT32 nIndex, double nVolume, INT32 nRouteDir);
void RF5C68PCMExit();
void RF5C68PCMScan(INT32 nAction);
void RF5C68PCMRegWrite(UINT8 offset, UINT8 data);
UINT8 RF5C68PCMRead(UINT16 offset);
void RF5C68PCMWrite(UINT16 offset, UINT8 data);

#define BURN_SND_RF5C68PCM_ROUTE_1		0
#define BURN_SND_RF5C68PCM_ROUTE_2		1

#define RF5C68PCMSetAllRoutes(v, d)					\
	RF5C68PCMSetRoute(BURN_SND_RF5C68PCM_ROUTE_1, v, d);	\
	RF5C68PCMSetRoute(BURN_SND_RF5C68PCM_ROUTE_2, v, d);
