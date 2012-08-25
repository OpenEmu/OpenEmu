void K053260Reset(INT32 chip);
void K053260Update(INT32 chip, INT16 *pBuf, INT32 length);
void K053260Init(INT32 chip, INT32 clock, UINT8 *rom, INT32 nLen);
void K053260SetRoute(INT32 chip, INT32 nIndex, double nVolume, INT32 nRouteDir);
void K053260Exit();
void K053260Write(INT32 chip, INT32 offset, UINT8 data);
UINT8 K053260Read(INT32 chip, INT32 offset);

INT32 K053260Scan(INT32 nAction);

#define BURN_SND_K053260_ROUTE_1		0
#define BURN_SND_K053260_ROUTE_2		1

#define K053260PCMSetAllRoutes(i, v, d)						\
	K053260SetRoute(i, BURN_SND_K053260_ROUTE_1, v, d);	\
	K053260SetRoute(i, BURN_SND_K053260_ROUTE_2, v, d);
