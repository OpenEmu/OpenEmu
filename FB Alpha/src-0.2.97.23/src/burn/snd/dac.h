void DACUpdate(INT16* Buffer, INT32 Length);
void DACWrite(INT32 Chip, UINT8 Data);
void DACSignedWrite(INT32 Chip, UINT8 Data);
void DACInit(INT32 Num, UINT32 Clock, INT32 bAdd, INT32 (*pSyncCB)());
void DACSetVolShift(INT32 Num, INT32 nShift);
void DACReset();
void DACExit();
INT32 DACScan(INT32 nAction,INT32 *pnMin);
