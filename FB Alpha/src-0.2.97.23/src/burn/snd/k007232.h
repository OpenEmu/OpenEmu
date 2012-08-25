void K007232Update(INT32 chip, INT16* pSoundBuf, INT32 nLength);
UINT8 K007232ReadReg(INT32 chip, INT32 r);
void K007232WriteReg(INT32 chip, INT32 r, INT32 v);
void K007232SetPortWriteHandler(INT32 chip, void (*Handler)(INT32 v));
void K007232Init(INT32 chip, INT32 clock, UINT8 *pPCMData, INT32 PCMDataSize);
void K007232SetRoute(INT32 chip, INT32 nIndex, double nVolume, INT32 nRouteDir);
void K007232Exit();
INT32 K007232Scan(INT32 nAction, INT32 *pnMin);
void K007232SetVolume(INT32 chip, INT32 channel,INT32 volumeA,INT32 volumeB);
void k007232_set_bank( INT32 chip, INT32 chABank, INT32 chBBank );

#define BURN_SND_K007232_ROUTE_1		0
#define BURN_SND_K007232_ROUTE_2		1

#define K007232PCMSetAllRoutes(i, v, d)						\
	K007232SetRoute(i, BURN_SND_K007232_ROUTE_1, v, d);	\
	K007232SetRoute(i, BURN_SND_K007232_ROUTE_2, v, d);
