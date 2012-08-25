// Yamaha YMZ280B module

INT32 YMZ280BInit(INT32 nClock, void (*IRQCallback)(INT32));
void YMZ280BSetRoute(INT32 nIndex, double nVolume, INT32 nRouteDir);
void YMZ280BReset();
INT32 YMZ280BScan();
void YMZ280BExit();
INT32 YMZ280BRender(INT16* pSoundBuf, INT32 nSegmenLength);
void YMZ280BWriteRegister(UINT8 nValue);
UINT32 YMZ280BReadStatus();
UINT32 YMZ280BReadRAM();

extern UINT8* YMZ280BROM;

// external memory handlers
extern void (*pYMZ280BRAMWrite)(INT32 offset, INT32 nValue);
extern INT32 (*pYMZ280BRAMRead)(INT32 offset);

extern UINT32 nYMZ280BStatus;
extern UINT32 nYMZ280BRegister;

inline static void YMZ280BSelectRegister(UINT8 nRegister)
{
	nYMZ280BRegister = nRegister;
}

inline static void YMZ280BWrite(INT32 offset, UINT8 nValue)
{
	if (offset & 1) {
		YMZ280BWriteRegister(nValue);
	} else {
		nYMZ280BRegister = nValue;
	}
}

inline static UINT32 YMZ280BRead(INT32 offset)
{
	if (offset & 1) {
		return YMZ280BReadStatus();
	} else {
		return YMZ280BReadRAM();
	}

	return 0;
}

#define BURN_SND_YMZ280B_YMZ280B_ROUTE_1		0
#define BURN_SND_YMZ280B_YMZ280B_ROUTE_2		1

#define YMZ280BSetAllRoutes(v, d)								\
	YMZ280BSetRoute(BURN_SND_YMZ280B_YMZ280B_ROUTE_1, v, d);	\
	YMZ280BSetRoute(BURN_SND_YMZ280B_YMZ280B_ROUTE_2, v, d);
