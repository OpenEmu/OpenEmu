/**********************************************************************
 *
 * Filename:    crc.c
 *
 * Description: Slow and fast implementations of the CRC standards.
 *
 * Notes:       The parameters for each supported CRC standard are
 *              defined in the header file crc.h.  The implementations
 *              here should stand up to further additions to that list.
 *
 *
 * Copyright (c) 2000 by Michael Barr.  This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 **********************************************************************/

#include "engines/wintermute/utils/crc.h"

namespace Wintermute {

/*
 * Derive parameters from the standard-specific parameters in crc.h.
 */
#define WIDTH    (8 * sizeof(crc))
#define TOPBIT   (1 << (WIDTH - 1))

#if (REFLECT_DATA == TRUE)
#undef  REFLECT_DATA
#define REFLECT_DATA(X)         ((unsigned char) reflect((X), 8))
#else
#undef  REFLECT_DATA
#define REFLECT_DATA(X)         (X)
#endif

#if (REFLECT_REMAINDER == TRUE)
#undef  REFLECT_REMAINDER
#define REFLECT_REMAINDER(X)    ((crc) reflect((X), WIDTH))
#else
#undef  REFLECT_REMAINDER
#define REFLECT_REMAINDER(X)    (X)
#endif


/*********************************************************************
 *
 * Function:    reflect()
 *
 * Description: Reorder the bits of a binary sequence, by reflecting
 *              them about the middle position.
 *
 * Notes:       No checking is done that nBits <= 32.
 *
 * Returns:     The reflection of the original data.
 *
 *********************************************************************/
static unsigned long
reflect(unsigned long data, unsigned char nBits) {
	unsigned long  reflection = 0x00000000;
	unsigned char  bit;

	/*
	 * Reflect the data about the center bit.
	 */
	for (bit = 0; bit < nBits; ++bit) {
		/*
		 * If the LSB bit is set, set the reflection of it.
		 */
		if (data & 0x01) {
			reflection |= (1 << ((nBits - 1) - bit));
		}

		data = (data >> 1);
	}

	return (reflection);

}   /* reflect() */


/*********************************************************************
 *
 * Function:    crcSlow()
 *
 * Description: Compute the CRC of a given message.
 *
 * Notes:
 *
 * Returns:     The CRC of the message.
 *
 *********************************************************************/
crc
crcSlow(unsigned char const message[], int nBytes) {
	crc            remainder = INITIAL_REMAINDER;
	int            byte;
	unsigned char  bit;


	/*
	 * Perform modulo-2 division, a byte at a time.
	 */
	for (byte  = 0; byte < nBytes; ++byte) {
		/*
		 * Bring the next byte into the remainder.
		 */
		remainder ^= (REFLECT_DATA(message[byte]) << (WIDTH - 8));

		/*
		 * Perform modulo-2 division, a bit at a time.
		 */
		for (bit = 8; bit > 0; --bit) {
			/*
			 * Try to divide the current data bit.
			 */
			if (remainder & TOPBIT) {
				remainder = (remainder << 1) ^ POLYNOMIAL;
			} else {
				remainder = (remainder << 1);
			}
		}
	}

	/*
	 * The final remainder is the CRC result.
	 */
	return (REFLECT_REMAINDER(remainder) ^ FINAL_XOR_VALUE);

}   /* crcSlow() */


crc  crcTable[256];


/*********************************************************************
 *
 * Function:    crcInit()
 *
 * Description: Populate the partial CRC lookup table.
 *
 * Notes:       This function must be rerun any time the CRC standard
 *              is changed.  If desired, it can be run "offline" and
 *              the table results stored in an embedded system's ROM.
 *
 * Returns:     None defined.
 *
 *********************************************************************/
void
crcInit(void) {
	crc            remainder;
	int            dividend;
	unsigned char  bit;


	/*
	 * Compute the remainder of each possible dividend.
	 */
	for (dividend = 0; dividend < 256; ++dividend) {
		/*
		 * Start with the dividend followed by zeros.
		 */
		remainder = dividend << (WIDTH - 8);

		/*
		 * Perform modulo-2 division, a bit at a time.
		 */
		for (bit = 8; bit > 0; --bit) {
			/*
			 * Try to divide the current data bit.
			 */
			if (remainder & TOPBIT) {
				remainder = (remainder << 1) ^ POLYNOMIAL;
			} else {
				remainder = (remainder << 1);
			}
		}

		/*
		 * Store the result into the table.
		 */
		crcTable[dividend] = remainder;
	}

}   /* crcInit() */


/*********************************************************************
 *
 * Function:    crcFast()
 *
 * Description: Compute the CRC of a given message.
 *
 * Notes:       crcInit() must be called first.
 *
 * Returns:     The CRC of the message.
 *
 *********************************************************************/
crc
crcFast(unsigned char const message[], int nBytes) {
	crc            remainder = INITIAL_REMAINDER;
	unsigned char  data;
	int            byte;


	/*
	 * Divide the message by the polynomial, a byte at a time.
	 */
	for (byte  = 0; byte < nBytes; ++byte) {
		data = (unsigned char)(REFLECT_DATA(message[byte]) ^ (remainder >> (WIDTH - 8)));
		remainder = crcTable[data] ^ (remainder << 8);
	}

	/*
	 * The final remainder is the CRC.
	 */
	return (REFLECT_REMAINDER(remainder) ^ FINAL_XOR_VALUE);

}   /* crcFast() */



crc crc_initialize(void) {
	crcInit();
	return INITIAL_REMAINDER;
}

crc crc_process_byte(unsigned char byteVal, crc remainder) {
	unsigned char  data;
	data = (unsigned char)(REFLECT_DATA(byteVal) ^ (remainder >> (WIDTH - 8)));
	remainder = crcTable[data] ^ (remainder << 8);
	return remainder;
}

crc crc_finalize(crc remainder) {
	return (REFLECT_REMAINDER(remainder) ^ FINAL_XOR_VALUE);
}

} // end of namespace Wintermute
