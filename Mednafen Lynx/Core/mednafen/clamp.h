#ifndef __MDFN_CLAMP_H
#define __MDFN_CLAMP_H

static INLINE int32 clamp_to_u8(int32 i)
{
 if(i & 0xFFFFFF00)
  i = (((~i) >> 30) & 0xFF);

 return(i);
}


static INLINE int32 clamp_to_u16(int32 i)
{
 if(i & 0xFFFF0000)
  i = (((~i) >> 31) & 0xFFFF);

 return(i);
}

template<typename T, typename U, typename V> static INLINE void clamp(T *val, U minimum, V maximum)
{
 if(*val < minimum)
 {
  //printf("Warning: clamping to minimum(%d)\n", (int)minimum);
  *val = minimum;
 }
 if(*val > maximum)
 {
  //printf("Warning: clamping to maximum(%d)\n", (int)maximum);
  *val = maximum;
 }
}

#endif
