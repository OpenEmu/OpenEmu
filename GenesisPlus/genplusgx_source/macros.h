#ifndef _MACROS_H_
#define _MACROS_H_

#ifdef LSB_FIRST

#define READ_BYTE(BASE, ADDR) (BASE)[(ADDR)^1]

#define READ_WORD(BASE, ADDR) (((BASE)[ADDR]<<8) | (BASE)[(ADDR)+1])

#define READ_WORD_LONG(BASE, ADDR) (((BASE)[(ADDR)+1]<<24) |      \
                                    ((BASE)[(ADDR)]<<16) |  \
                                    ((BASE)[(ADDR)+3]<<8) |   \
                                    (BASE)[(ADDR)+2])

#define WRITE_BYTE(BASE, ADDR, VAL) (BASE)[(ADDR)^1] = (VAL)&0xff

#define WRITE_WORD(BASE, ADDR, VAL) (BASE)[ADDR] = ((VAL)>>8) & 0xff; \
                                      (BASE)[(ADDR)+1] = (VAL)&0xff

#define WRITE_WORD_LONG(BASE, ADDR, VAL) (BASE)[(ADDR+1)] = ((VAL)>>24) & 0xff;    \
                                          (BASE)[(ADDR)] = ((VAL)>>16)&0xff;  \
                                          (BASE)[(ADDR+3)] = ((VAL)>>8)&0xff;   \
                                          (BASE)[(ADDR+2)] = (VAL)&0xff

#else

#define READ_BYTE(BASE, ADDR) (BASE)[ADDR]
#define READ_WORD(BASE, ADDR) *(uint16 *)((BASE) + (ADDR))
#define READ_WORD_LONG(BASE, ADDR) *(uint32 *)((BASE) + (ADDR))
#define WRITE_BYTE(BASE, ADDR, VAL) (BASE)[ADDR] = VAL & 0xff
#define WRITE_WORD(BASE, ADDR, VAL) *(uint16 *)((BASE) + (ADDR)) = VAL & 0xffff
#define WRITE_WORD_LONG(BASE, ADDR, VAL) *(uint32 *)((BASE) + (ADDR)) = VAL & 0xffffffff
#endif


#endif /* _MACROS_H_ */
