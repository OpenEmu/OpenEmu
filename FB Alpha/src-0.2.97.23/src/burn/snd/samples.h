#define SAMPLE_IGNORE		0x01 // don't ever play this sample
#define SAMPLE_AUTOLOOP		0x02 // start the looping on start
#define SAMPLE_NOLOOP		0x04 // don't allow this to loop

void BurnSamplePlay(INT32 sample);
void BurnSamplePause(INT32 sample);
void BurnSampleResume(INT32 sample);
void BurnSampleStop(INT32 sample);

void BurnSampleSetLoop(INT32 sample, bool dothis);

INT32  BurnSampleGetStatus(INT32 sample);

INT32  BurnSampleGetPosition(INT32 sample);
void BurnSampleSetPosition(INT32 sample, UINT32 position);

void BurnSampleReset();

void BurnSampleInit(INT32 bAdd);
void BurnSampleSetRoute(INT32 sample, INT32 nIndex, double nVolume, INT32 nRouteDir);
void BurnSampleSetRouteAllSamples(INT32 nIndex, double nVolume, INT32 nRouteDir);

INT32  BurnSampleScan(INT32 nAction, INT32 *pnMin);

void BurnSampleRender(INT16 *pDest, UINT32 pLen);
void BurnSampleExit();

#define BURN_SND_SAMPLE_ROUTE_1			0
#define BURN_SND_SAMPLE_ROUTE_2			1

#define BurnSampleSetAllRoutes(i, v, d)						\
	BurnSampleSetRoute(i, BURN_SND_SAMPLE_ROUTE_1, v, d);	\
	BurnSampleSetRoute(i, BURN_SND_SAMPLE_ROUTE_2, v, d);

#define BurnSampleSetAllRoutesAllSamples(v, d)						\
	BurnSampleSetRouteAllSamples(BURN_SND_SAMPLE_ROUTE_1, v, d);	\
	BurnSampleSetRouteAllSamples(BURN_SND_SAMPLE_ROUTE_2, v, d);
