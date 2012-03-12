#define BITSWAP32(n, 														\
				  bit31, bit30, bit29, bit28, bit27, bit26, bit25, bit24,	\
				  bit23, bit22, bit21, bit20, bit19, bit18, bit17, bit16,	\
				  bit15, bit14, bit13, bit12, bit11, bit10, bit09, bit08,	\
				  bit07, bit06, bit05, bit04, bit03, bit02, bit01, bit00)	\
		(((((n) >> (bit31)) & 1) << 31) | 									\
		 ((((n) >> (bit30)) & 1) << 30) | 									\
		 ((((n) >> (bit29)) & 1) << 29) | 									\
		 ((((n) >> (bit28)) & 1) << 28) | 									\
		 ((((n) >> (bit27)) & 1) << 27) | 									\
		 ((((n) >> (bit26)) & 1) << 26) | 									\
		 ((((n) >> (bit25)) & 1) << 25) | 									\
		 ((((n) >> (bit24)) & 1) << 24) | 									\
		 ((((n) >> (bit23)) & 1) << 23) | 									\
		 ((((n) >> (bit22)) & 1) << 22) | 									\
		 ((((n) >> (bit21)) & 1) << 21) | 									\
		 ((((n) >> (bit20)) & 1) << 20) | 									\
		 ((((n) >> (bit19)) & 1) << 19) | 									\
		 ((((n) >> (bit18)) & 1) << 18) | 									\
		 ((((n) >> (bit17)) & 1) << 17) | 									\
		 ((((n) >> (bit16)) & 1) << 16) | 									\
		 ((((n) >> (bit15)) & 1) << 15) | 									\
		 ((((n) >> (bit14)) & 1) << 14) | 									\
		 ((((n) >> (bit13)) & 1) << 13) | 									\
		 ((((n) >> (bit12)) & 1) << 12) | 									\
		 ((((n) >> (bit11)) & 1) << 11) | 									\
		 ((((n) >> (bit10)) & 1) << 10) | 									\
		 ((((n) >> (bit09)) & 1) <<  9) | 									\
		 ((((n) >> (bit08)) & 1) <<  8) | 									\
		 ((((n) >> (bit07)) & 1) <<  7) | 									\
		 ((((n) >> (bit06)) & 1) <<  6) | 									\
		 ((((n) >> (bit05)) & 1) <<  5) | 									\
		 ((((n) >> (bit04)) & 1) <<  4) | 									\
		 ((((n) >> (bit03)) & 1) <<  3) | 									\
		 ((((n) >> (bit02)) & 1) <<  2) | 									\
		 ((((n) >> (bit01)) & 1) <<  1) | 									\
		 ((((n) >> (bit00)) & 1) <<  0))

#define BITSWAP24(n, 														\
				  bit23, bit22, bit21, bit20, bit19, bit18, bit17, bit16,	\
				  bit15, bit14, bit13, bit12, bit11, bit10, bit09, bit08,	\
				  bit07, bit06, bit05, bit04, bit03, bit02, bit01, bit00)	\
		(((((n) >> (bit23)) & 1) << 23) | 									\
		 ((((n) >> (bit22)) & 1) << 22) | 									\
		 ((((n) >> (bit21)) & 1) << 21) | 									\
		 ((((n) >> (bit20)) & 1) << 20) | 									\
		 ((((n) >> (bit19)) & 1) << 19) | 									\
		 ((((n) >> (bit18)) & 1) << 18) | 									\
		 ((((n) >> (bit17)) & 1) << 17) | 									\
		 ((((n) >> (bit16)) & 1) << 16) | 									\
		 ((((n) >> (bit15)) & 1) << 15) | 									\
		 ((((n) >> (bit14)) & 1) << 14) | 									\
		 ((((n) >> (bit13)) & 1) << 13) | 									\
		 ((((n) >> (bit12)) & 1) << 12) | 									\
		 ((((n) >> (bit11)) & 1) << 11) | 									\
		 ((((n) >> (bit10)) & 1) << 10) | 									\
		 ((((n) >> (bit09)) & 1) <<  9) | 									\
		 ((((n) >> (bit08)) & 1) <<  8) | 									\
		 ((((n) >> (bit07)) & 1) <<  7) | 									\
		 ((((n) >> (bit06)) & 1) <<  6) | 									\
		 ((((n) >> (bit05)) & 1) <<  5) | 									\
		 ((((n) >> (bit04)) & 1) <<  4) | 									\
		 ((((n) >> (bit03)) & 1) <<  3) | 									\
		 ((((n) >> (bit02)) & 1) <<  2) | 									\
		 ((((n) >> (bit01)) & 1) <<  1) | 									\
		 ((((n) >> (bit00)) & 1) <<  0))

#define BITSWAP16(n, 														\
				  bit15, bit14, bit13, bit12, bit11, bit10, bit09, bit08,	\
				  bit07, bit06, bit05, bit04, bit03, bit02, bit01, bit00)	\
		(((((n) >> (bit15)) & 1) << 15) | 									\
		 ((((n) >> (bit14)) & 1) << 14) | 									\
		 ((((n) >> (bit13)) & 1) << 13) | 									\
		 ((((n) >> (bit12)) & 1) << 12) | 									\
		 ((((n) >> (bit11)) & 1) << 11) | 									\
		 ((((n) >> (bit10)) & 1) << 10) | 									\
		 ((((n) >> (bit09)) & 1) <<  9) | 									\
		 ((((n) >> (bit08)) & 1) <<  8) | 									\
		 ((((n) >> (bit07)) & 1) <<  7) | 									\
		 ((((n) >> (bit06)) & 1) <<  6) | 									\
		 ((((n) >> (bit05)) & 1) <<  5) | 									\
		 ((((n) >> (bit04)) & 1) <<  4) | 									\
		 ((((n) >> (bit03)) & 1) <<  3) | 									\
		 ((((n) >> (bit02)) & 1) <<  2) | 									\
		 ((((n) >> (bit01)) & 1) <<  1) | 									\
		 ((((n) >> (bit00)) & 1) <<  0))

#define BITSWAP08(n, 														\
				  bit07, bit06, bit05, bit04, bit03, bit02, bit01, bit00)	\
		(((((n) >> (bit07)) & 1) <<  7) | 									\
		 ((((n) >> (bit06)) & 1) <<  6) | 									\
		 ((((n) >> (bit05)) & 1) <<  5) | 									\
		 ((((n) >> (bit04)) & 1) <<  4) | 									\
		 ((((n) >> (bit03)) & 1) <<  3) | 									\
		 ((((n) >> (bit02)) & 1) <<  2) | 									\
		 ((((n) >> (bit01)) & 1) <<  1) | 									\
		 ((((n) >> (bit00)) & 1) <<  0))

#define BIT(x,n) 		(((x)>>(n))&1)

/* ----- macros for accessing bytes and words within larger chunks ----- */
#ifdef LSB_FIRST
	#define BYTE_XOR_BE(a)  	((a) ^ 1)				/* read/write a byte to a 16-bit space */
	#define BYTE_XOR_LE(a)  	(a)
	#define BYTE4_XOR_BE(a) 	((a) ^ 3)				/* read/write a byte to a 32-bit space */
	#define BYTE4_XOR_LE(a) 	(a)
	#define WORD_XOR_BE(a)  	((a) ^ 2)				/* read/write a word to a 32-bit space */
	#define WORD_XOR_LE(a)  	(a)
#else
	#define BYTE_XOR_BE(a)  	(a)
	#define BYTE_XOR_LE(a)  	((a) ^ 1)				/* read/write a byte to a 16-bit space */
	#define BYTE4_XOR_BE(a) 	(a)
	#define BYTE4_XOR_LE(a) 	((a) ^ 3)				/* read/write a byte to a 32-bit space */
	#define WORD_XOR_BE(a)  	(a)
	#define WORD_XOR_LE(a)  	((a) ^ 2)				/* read/write a word to a 32-bit space */
#endif
