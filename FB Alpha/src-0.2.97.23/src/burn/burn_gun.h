#define MAX_GUNS	4

extern INT32 nBurnGunNumPlayers;

extern INT32 BurnGunX[MAX_GUNS];
extern INT32 BurnGunY[MAX_GUNS];

UINT8 BurnGunReturnX(INT32 num);
UINT8 BurnGunReturnY(INT32 num);

extern void BurnGunInit(INT32 nNumPlayers, bool bDrawTargets);
void BurnGunExit();
void BurnGunScan();
extern void BurnGunDrawTarget(INT32 num, INT32 x, INT32 y);
extern void BurnGunMakeInputs(INT32 num, INT16 x, INT16 y);
