/**********************************************************************
 *
 * Filename:    crc.h
 *
 * Description: A header file describing the various CRC standards.
 *
 * Notes:
 *
 *
 * Copyright (c) 2000 by Michael Barr.  This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 **********************************************************************/

#ifndef _crc_h
#define _crc_h

#include "common/system.h" // For types.

namespace Wintermute {

#ifndef TRUE
#define FALSE   0
#define TRUE    !FALSE
#endif

/*
 * Select the CRC standard from the list that follows.
 */
#define CRC32

#if defined(CRC_CCITT)

typedef uint16  crc;

#define CRC_NAME            "CRC-CCITT"
#define POLYNOMIAL          0x1021
#define INITIAL_REMAINDER   0xFFFF
#define FINAL_XOR_VALUE     0x0000
#define REFLECT_DATA        FALSE
#define REFLECT_REMAINDER   FALSE
#define CHECK_VALUE         0x29B1

#elif defined(CRC16)

typedef uint16  crc;

#define CRC_NAME            "CRC-16"
#define POLYNOMIAL          0x8005
#define INITIAL_REMAINDER   0x0000
#define FINAL_XOR_VALUE     0x0000
#define REFLECT_DATA        TRUE
#define REFLECT_REMAINDER   TRUE
#define CHECK_VALUE         0xBB3D

#elif defined(CRC32)

typedef uint32  crc;

#define CRC_NAME            "CRC-32"
#define POLYNOMIAL          0x04C11DB7
#define INITIAL_REMAINDER   0xFFFFFFFF
#define FINAL_XOR_VALUE     0xFFFFFFFF
#define REFLECT_DATA        TRUE
#define REFLECT_REMAINDER   TRUE
#define CHECK_VALUE         0xCBF43926

#else

#error "One of CRC_CCITT, CRC16, or CRC32 must be #define'd."

#endif

void  crcInit(void);
crc   crcSlow(unsigned char const message[], int nBytes);
crc   crcFast(unsigned char const message[], int nBytes);

extern "C" crc crc_initialize(void);
extern "C" crc crc_process_byte(unsigned char byteVal, crc remainder);
extern "C" crc crc_finalize(crc remainder);

} // End of namespace Wintermute

#endif /* _crc_h */
