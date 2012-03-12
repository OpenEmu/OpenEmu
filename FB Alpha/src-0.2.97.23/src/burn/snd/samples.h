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

void BurnSampleInit(INT32 nGain, INT32 bAdd);

INT32  BurnSampleScan(INT32 nAction, INT32 *pnMin);

void BurnSampleRender(INT16 *pDest, UINT32 pLen);
void BurnSampleExit();
