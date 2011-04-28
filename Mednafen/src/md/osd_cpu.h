/*******************************************************************************
*																			   *
*	Define size independent data types and operations.						   *
*																			   *
*   The following types must be supported by all platforms:					   *
*																			   *
*	uint8  - Unsigned 8-bit Integer		int8  - Signed 8-bit integer           *
*	uint16 - Unsigned 16-bit Integer	int16 - Signed 16-bit integer          *
*	uint32 - Unsigned 32-bit Integer	int32 - Signed 32-bit integer          *
*	uint64 - Unsigned 64-bit Integer	INT64 - Signed 64-bit integer          *
*																			   *
*																			   *
*   The macro names for the artithmatic operations are composed as follows:    *
*																			   *
*   XXX_R_A_B, where XXX - 3 letter operation code (ADD, SUB, etc.)			   *
*					 R   - The type	of the result							   *
*					 A   - The type of operand 1							   *
*			         B   - The type of operand 2 (if binary operation)		   *
*																			   *
*				     Each type is one of: U8,8,U16,16,U32,32,U64,64			   *
*																			   *
*******************************************************************************/


#ifndef OSD_CPU_H
#define OSD_CPU_H

/* Combine two 32-bit integers into a 64-bit integer */
#define COMBINE_64_32_32(A,B)     ((((uint64)(A))<<32) | (uint32)(B))
#define COMBINE_U64_U32_U32(A,B)  COMBINE_64_32_32(A,B)

/* Return upper 32 bits of a 64-bit integer */
#define HI32_32_64(A)		  (((uint64)(A)) >> 32)
#define HI32_U32_U64(A)		  HI32_32_64(A)

/* Return lower 32 bits of a 64-bit integer */
#define LO32_32_64(A)		  ((A) & 0xffffffff)
#define LO32_U32_U64(A)		  LO32_32_64(A)

#define DIV_64_64_32(A,B)	  ((A)/(B))
#define DIV_U64_U64_U32(A,B)  ((A)/(uint32)(B))

#define MOD_32_64_32(A,B)	  ((A)%(B))
#define MOD_U32_U64_U32(A,B)  ((A)%(uint32)(B))

#define MUL_64_32_32(A,B)	  ((A)*(INT64)(B))
#define MUL_U64_U32_U32(A,B)  ((A)*(uint64)(uint32)(B))


/******************************************************************************
 * Union of uint8, uint16 and uint32 in native endianess of the target
 * This is used to access bytes and words in a machine independent manner.
 * The upper bytes h2 and h3 normally contain zero (16 bit CPU cores)
 * thus PAIR.d can be used to pass arguments to the memory system
 * which expects 'int' really.
 ******************************************************************************/
typedef union {
#ifdef LSB_FIRST
	struct { uint8 l,h,h2,h3; } b;
	struct { uint16 l,h; } w;
#else
	struct { uint8 h3,h2,h,l; } b;
	struct { uint16 h,l; } w;
#endif
	uint32 d;
}	PAIR;

#endif	/* defined OSD_CPU_H */
