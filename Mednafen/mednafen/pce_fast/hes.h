namespace PCE_Fast
{

uint8 ReadIBP(unsigned int A);
int PCE_HESLoad(const uint8 *buf, uint32 size);
void HES_Reset(void);
void HES_Draw(MDFN_Surface *surface, MDFN_Rect *DisplayRect, int16 *samples, int32 sampcount);
void HES_Close(void);

};
