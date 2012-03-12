extern UINT8* NamcoSoundProm;
void NamcoSoundUpdate(INT16* buffer, INT32 length);
void NamcoSoundUpdateStereo(INT16* buffer, INT32 length);
void NamcoSoundWrite(UINT32 offset, UINT8 data);
void NamcoSoundInit(INT32 clock);
void NamcoSoundExit();
void NamcoSoundScan(INT32 nAction,INT32 *pnMin);

void namcos1_custom30_write(INT32 offset, INT32 data);
UINT8 namcos1_custom30_read(INT32 offset);
