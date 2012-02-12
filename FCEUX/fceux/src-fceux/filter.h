#ifndef __FCEU_FILTER_H
#define __FCEU_FILTER_H

#if SOUND_QUALITY == 1
int32 NeoFilterSound(int32 *in, int32 *out, uint32 inlen, int32 *leftover);
#endif
void MakeFilters(int32 rate);
void SexyFilter(int32 *in, int32 *out, int32 count);

#endif
