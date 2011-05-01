#ifndef __MDFN_ENDIAN_H
#define __MDFN_ENDIAN_H

#ifdef MSB_FIRST
 #ifdef LSB_FIRST
  #error Only define one of LSB_FIRST and MSB_FIRST
 #endif

 #ifndef le32toh
  #define le32toh(l)      ((((l)>>24) & 0xff) | (((l)>>8) & 0xff00) \
                         | (((l)<<8) & 0xff0000) | (((l)<<24) & 0xff000000))
 #endif
 #ifndef le16toh
  #define le16toh(l)      ((((l)>>8) & 0xff) | (((l)<<8) & 0xff00))
 #endif
#else
 #ifndef le32toh
  #define le32toh(l)      (l)
 #endif
 #ifndef le16toh
  #define le16toh(l)      (l)
 #endif
#endif

#ifndef htole32
#define htole32 le32toh
#endif

#ifndef htole16
#define htole16 le16toh
#endif


int write16le(uint16 b, FILE *fp);
int write32le(uint32 b, FILE *fp);
int read32le(uint32 *Bufo, FILE *fp);

void Endian_A16_Swap(void *src, uint32 nelements);
void Endian_A32_Swap(void *src, uint32 nelements);
void Endian_A64_Swap(void *src, uint32 nelements);

void Endian_A16_NE_to_LE(void *src, uint32 nelements);
void Endian_A32_NE_to_LE(void *src, uint32 nelements);
void Endian_A64_NE_to_LE(void *src, uint32 nelements);

void Endian_A16_LE_to_NE(void *src, uint32 nelements);
void Endian_A16_BE_to_NE(void *src, uint32 nelements);
void Endian_A32_LE_to_NE(void *src, uint32 nelements);
void Endian_A64_LE_to_NE(void *src, uint32 nelements);

void Endian_V_LE_to_NE(void *src, uint32 bytesize);
void Endian_V_NE_to_LE(void *src, uint32 bytesize);

void FlipByteOrder(uint8 *src, uint32 count);

// The following functions can encode/decode to unaligned addresses.

static INLINE void MDFN_en16lsb(uint8 *buf, uint16 morp)
{
 buf[0]=morp;
 buf[1]=morp>>8;
}

static INLINE void MDFN_en24lsb(uint8 *buf, uint32 morp)
{
 buf[0]=morp;
 buf[1]=morp>>8;
 buf[2]=morp>>16;
}


static INLINE void MDFN_en32lsb(uint8 *buf, uint32 morp)
{
 buf[0]=morp;
 buf[1]=morp>>8;
 buf[2]=morp>>16;
 buf[3]=morp>>24;
}

static INLINE void MDFN_en64lsb(uint8 *buf, uint64 morp)
{
 buf[0]=morp >> 0;
 buf[1]=morp >> 8;
 buf[2]=morp >> 16;
 buf[3]=morp >> 24;
 buf[4]=morp >> 32;
 buf[5]=morp >> 40;
 buf[6]=morp >> 48;
 buf[7]=morp >> 56;
}


static INLINE void MDFN_en16msb(uint8 *buf, uint16 morp)
{
 buf[0] = morp >> 8;
 buf[1] = morp;
}

static INLINE void MDFN_en24msb(uint8 *buf, uint32 morp)
{
 buf[0] = morp >> 16;
 buf[1] = morp >> 8;
 buf[2] = morp;
}

static INLINE void MDFN_en32msb(uint8 *buf, uint32 morp)
{
 buf[0] = morp >> 24;
 buf[1] = morp >> 16;
 buf[2] = morp >> 8;
 buf[3] = morp;
}

static INLINE void MDFN_en64msb(uint8 *buf, uint64 morp)
{
 buf[0] = morp >> 56;
 buf[1] = morp >> 48;
 buf[2] = morp >> 40;
 buf[3] = morp >> 32;
 buf[4] = morp >> 24;
 buf[5] = morp >> 16;
 buf[6] = morp >> 8;
 buf[7] = morp >> 0;
}


// Overloaded functions, yay.
static INLINE void MDFN_enlsb(uint16 * buf, uint16 value)
{
 MDFN_en16lsb((uint8 *)buf, value);
}

static INLINE void MDFN_enlsb(uint32 * buf, uint32 value)
{
 MDFN_en32lsb((uint8 *)buf, value);
}

static INLINE void MDFN_enlsb(uint64 * buf, uint64 value)
{
 MDFN_en64lsb((uint8 *)buf, value);
}


static INLINE uint16 MDFN_de16lsb(const uint8 *morp)
{
 return(morp[0] | (morp[1] << 8));
}


static INLINE uint32 MDFN_de24lsb(const uint8 *morp)
{
 return(morp[0]|(morp[1]<<8)|(morp[2]<<16));
}

static INLINE uint32 MDFN_de32lsb(const uint8 *morp)
{
 return(morp[0]|(morp[1]<<8)|(morp[2]<<16)|(morp[3]<<24));
}

static INLINE uint64 MDFN_de64lsb(const uint8 *morp)
{
 uint64 ret = 0;

 ret |= (uint64)morp[0];
 ret |= (uint64)morp[1] << 8;
 ret |= (uint64)morp[2] << 16;
 ret |= (uint64)morp[3] << 24;
 ret |= (uint64)morp[4] << 32;
 ret |= (uint64)morp[5] << 40;
 ret |= (uint64)morp[6] << 48;
 ret |= (uint64)morp[7] << 56;

 return(ret);
}

static INLINE uint16 MDFN_delsb(const uint16 *buf)
{
 return(MDFN_de16lsb((uint8 *)buf));
}

static INLINE uint32 MDFN_delsb(const uint32 *buf)
{
 return(MDFN_de32lsb((uint8 *)buf));
}

static INLINE uint64 MDFN_delsb(const uint64 *buf)
{
 return(MDFN_de64lsb((uint8 *)buf));
}

static INLINE uint16 MDFN_de16msb(const uint8 *morp)
{
 return(morp[1] | (morp[0] << 8));
}

static INLINE uint32 MDFN_de24msb(const uint8 *morp)
{
 return((morp[2]<<0)|(morp[1]<<8)|(morp[0]<<16));
}


static INLINE uint32 MDFN_de32msb(const uint8 *morp)
{
 return(morp[3]|(morp[2]<<8)|(morp[1]<<16)|(morp[0]<<24));
}

#endif
