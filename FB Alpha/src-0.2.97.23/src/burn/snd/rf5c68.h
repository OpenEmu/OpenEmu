void RF5C68PCMUpdate(INT16* pSoundBuf, INT32 length);
void RF5C68PCMReset();
void RF5C68PCMInit(INT32 clock);
void RF5C68PCMExit();
void RF5C68PCMScan(INT32 nAction);
void RF5C68PCMRegWrite(UINT8 offset, UINT8 data);
UINT8 RF5C68PCMRead(UINT16 offset);
void RF5C68PCMWrite(UINT16 offset, UINT8 data);
