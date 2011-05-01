
#ifndef _MACROS_H_
#define _MACROS_H_

static inline uint8 READ_BYTE_MSB(const uint8 *base, const unsigned int addr)
{
 return(base[addr + 0]);
}

static inline uint16 READ_WORD_MSB(const uint8 *base, const unsigned int addr)
{
 return((base[addr + 0] << 8) | (base[addr + 1] << 0));
}

static inline uint32 READ_32_MSB(const uint8 *base, const unsigned int addr, const bool is_aligned = 0)
{
 #ifdef MSB_FIRST
 if(is_aligned)
  return(*(uint32 *)&base[addr]);
 else
 #endif
  return((base[addr + 0] << 24) | (base[addr + 1] << 16) | (base[addr + 2] << 8) | (base[addr + 3] << 0));
}

static inline void WRITE_BYTE_MSB(uint8 *base, const unsigned int addr, const uint8 value)
{
 base[addr + 0] = value;
}

static inline void WRITE_WORD_MSB(uint8 *base, const unsigned int addr, const uint16 value)
{
 base[addr + 0] = value >> 8;
 base[addr + 1] = value;
}

static inline uint8 READ_BYTE_LSB(const uint8 *base, const unsigned int addr)
{
 return(base[addr + 0]);
}

static inline uint16 READ_WORD_LSB(const uint8 *base, const unsigned int addr)
{
 return((base[addr + 0] << 0) | (base[addr + 1] << 8));
}

static inline uint32 READ_32_LSB(const uint8 *base, const unsigned int addr, const bool is_aligned = 0)
{
 #ifdef LSB_FIRST
 if(is_aligned)
  return(*(uint32 *)&base[addr]);
 else
 #endif
  return((base[addr + 0] << 0) | (base[addr + 1] << 8) | (base[addr + 2] << 16) | (base[addr + 3] << 24));
}

static inline void WRITE_BYTE_LSB(uint8 *base, const unsigned int addr, const uint8 value)
{
 base[addr + 0] = value;
}

static inline void WRITE_WORD_LSB(uint8 *base, const unsigned int addr, const uint16 value)
{
 base[addr + 0] = value >> 0;
 base[addr + 1] = value >> 8;
}

#endif /* _MACROS_H_ */

