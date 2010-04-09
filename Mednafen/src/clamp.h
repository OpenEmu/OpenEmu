#ifndef __MDFN_CLAMP_H
#define __MDFN_CLAMP_H

static INLINE int32 clamp_to_u8(int32 i)
{
 if(i & 0xFFFFFF00)
  i = (((~i) >> 30) & 0xFF);

 return(i);
}


#endif
