/******************************************************************************

CPS-2 Encryption

All credit goes to Andreas Naive for breaking the encryption algorithm.
Code by Nicola Salmoria.
Thanks to Charles MacDonald and Razoola for extracting the data from the hardware.


The encryption only affects opcodes, not data.

It consists of two 4-round Feistel networks (FN) and involves both
the 16-bit opcode and the low 16 bits of the address.

Let be:

E = 16-bit ciphertext
A = 16-bit address
K = 64-bit key
D = 16-bit plaintext
y = FN1(x,k) = function describing the first Feistel network (x,y = 16 bit, k = 64 bit)
y = FN2(x,k) = function describing the second Feistel network (x,y = 16 bit, k = 64 bit)
y = EX(x) = fixed function that expands the 16-bit x to the 64-bit y

Then the cipher can be described as:

D = FN2( E, K XOR EX( FN1(A, K ) ) )


Each round of the Feistel networks consists of four substitution boxes. The boxes
have 6 inputs and 2 outputs. Usually the input is the XOR of a data bit and a key
bit, however in some cases only the key is used.

(TODO-notes about accuracy of s-boxes)

The s-boxes were chosen in order to use an empty key (all FF) for the dead board.


Also, the hardware has different watchdog opcodes and address range (see below)
which are stored in the battery backed RAM. There doesn't appear to be any relation
between those and the 64-bit encryption key, so they probably use an additional
64 bits of battery-backed RAM.



First FN:

 B(0 1 3 5 8 9 11 12)        A(10 4 6 7 2 13 15 14)
         L0                             R0
         |                              |
        XOR<-----------[F1]<------------|
         |                              |
         R1                             L1
         |                              |
         |------------>[F2]----------->XOR
         |                              |
         L2                             R2
         |                              |
        XOR<-----------[F3]<------------|
         |                              |
         R3                             L3
         |                              |
         |------------>[F4]----------->XOR
         |                              |
         L4                             R4
  (10 4 6 7 2 13 15 14)       (0 1 3 5 8 9 11 12)


Second FN:

 B(3 5 9 10 8 15 12 11)      A(6 0 2 13 1 4 14 7)
         L0                             R0
         |                              |
        XOR<-----------[F1]<------------|
         |                              |
         R1                             L1
         |                              |
         |------------>[F2]----------->XOR
         |                              |
         L2                             R2
         |                              |
        XOR<-----------[F3]<------------|
         |                              |
         R3                             L3
         |                              |
         |------------>[F4]----------->XOR
         |                              |
         L4                             R4
  (6 0 2 13 1 4 14 7)         (3 5 9 10 8 15 12 11)

******************************************************************************

Some Encryption notes.
----------------------

Address range.

The encryption does _not_ cover the entire address space. The range covered
differs per game.


Encryption Watchdog.

The CPS2 system has a watchdog system that will disable the decryption
of data if the watchdog isn't triggered at least once every few seconds.
The trigger varies from game to game (some games do use the same) and is
basically a 68000 opcode/s instruction. The instruction is the same for
all regions of the game. The watchdog instructions are listed alongside
the decryption keys.

*******************************************************************************/

#if 0
#include "driver.h"
#include "cpu/m68000/m68kmame.h"
#include "ui.h"
#include "includes/cps1.h"
#endif

#if 1
#include "cps.h"
#include "bitswap.h"

#define BIT(x,n) (((x)>>(n))&1)
#define BITSWAP8(a, b, c, d, e, f, g, h, i) BITSWAP08(a, b, c, d, e, f, g, h, i)
#endif


/******************************************************************************/

static const INT32 fn1_groupA[8] = { 10, 4, 6, 7, 2, 13, 15, 14 };
static const INT32 fn1_groupB[8] = {  0, 1, 3, 5, 8,  9, 11, 12 };

static const INT32 fn2_groupA[8] = { 6, 0, 2, 13, 1,  4, 14,  7 };
static const INT32 fn2_groupB[8] = { 3, 5, 9, 10, 8, 15, 12, 11 };

/******************************************************************************/

// The order of the input and output bits in the s-boxes is arbitrary.
// Each s-box can be XORed with an arbitrary vale in range 0-3 (but the same value
// must be used for the corresponding output bits in f1 and f3 or in f2 and f4)

struct sbox
{
	const UINT8 table[64];
	const INT32 inputs[6];		// positions of the inputs bits, -1 means no input except from key
	const INT32 outputs[2];		// positions of the output bits
};

// the above struct better defines how the hardware works, however
// to speed up the decryption at run time we convert it to the
// following one
struct optimised_sbox
{
	UINT8 input_lookup[256];
	UINT8 output[64];
};


static const struct sbox fn1_r1_boxes[4] =
{
	{	// subkey bits  0- 5
		{
			0,2,2,0,1,0,1,1,3,2,0,3,0,3,1,2,1,1,1,2,1,3,2,2,2,3,3,2,1,1,1,2,
			2,2,0,0,3,1,3,1,1,1,3,0,0,1,0,0,1,2,2,1,2,3,2,2,2,3,1,3,2,0,1,3,
		},
		{ 3, 4, 5, 6, -1, -1 },
		{ 3, 6 }
	},
	{	// subkey bits  6-11
		{
			3,0,2,2,2,1,1,1,1,2,1,0,0,0,2,3,2,3,1,3,0,0,0,2,1,2,2,3,0,3,3,3,
			0,1,3,2,3,3,3,1,1,1,1,2,0,1,2,1,3,2,3,1,1,3,2,2,2,3,1,3,2,3,0,0,
		},
		{ 0, 1, 2, 4, 7, -1 },
		{ 2, 7 }
	},
	{	// subkey bits 12-17
		{
			3,0,3,1,1,0,2,2,3,1,2,0,3,3,2,3,0,1,0,1,2,3,0,2,0,2,0,1,0,0,1,0,
			2,3,1,2,1,0,2,0,2,1,0,1,0,2,1,0,3,1,2,3,1,3,1,1,1,2,0,2,2,0,0,0,
		},
		{ 0, 1, 2, 3, 6, 7 },
		{ 0, 1 }
	},
	{	// subkey bits 18-23
		{
			3,2,0,3,0,2,2,1,1,2,3,2,1,3,2,1,2,2,1,3,3,2,1,0,1,0,1,3,0,0,0,2,
			2,1,0,1,0,1,0,1,3,1,1,2,2,3,2,0,3,3,2,0,2,1,3,3,0,0,3,0,1,1,3,3,
		},
		{ 0, 1, 3, 5, 6, 7 },
		{ 4, 5 }
	},
};

static const struct sbox fn1_r2_boxes[4] =
{
	{	// subkey bits 24-29
		{
			3,3,2,0,3,0,3,1,0,3,0,1,0,2,1,3,1,3,0,3,3,1,3,3,3,2,3,2,2,3,1,2,
			0,2,2,1,0,1,2,0,3,3,0,1,3,2,1,2,3,0,1,3,0,1,2,2,1,2,1,2,0,1,3,0,
		},
		{ 0, 1, 2, 3, 6, -1 },
		{ 1, 6 }
	},
	{	// subkey bits 30-35
		{
			1,2,3,2,1,3,0,1,1,0,2,0,0,2,3,2,3,3,0,1,2,2,1,0,1,0,1,2,3,2,1,3,
			2,2,2,0,1,0,2,3,2,1,2,1,2,1,0,3,0,1,2,3,1,2,1,3,2,0,3,2,3,0,2,0,
		},
		{ 2, 4, 5, 6, 7, -1 },
		{ 5, 7 }
	},
	{	// subkey bits 36-41
		{
			0,1,0,2,1,1,0,1,0,2,2,2,1,3,0,0,1,1,3,1,2,2,2,3,1,0,3,3,3,2,2,2,
			1,1,3,0,3,1,3,0,1,3,3,2,1,1,0,0,1,2,2,2,1,1,1,2,2,0,0,3,2,3,1,3,
		},
		{ 1, 2, 3, 4, 5, 7 },
		{ 0, 3 }
	},
	{	// subkey bits 42-47
		{
			2,1,0,3,3,3,2,0,1,2,1,1,1,0,3,1,1,3,3,0,1,2,1,0,0,0,3,0,3,0,3,0,
			1,3,3,3,0,3,2,0,2,1,2,2,2,1,1,3,0,1,0,1,0,1,1,1,1,3,1,0,1,2,3,3,
		},
		{ 0, 1, 3, 4, 6, 7 },
		{ 2, 4 }
	},
};

static const struct sbox fn1_r3_boxes[4] =
{
	{	// subkey bits 48-53
		{
			0,0,0,3,3,1,1,0,2,0,2,0,0,0,3,2,0,1,2,3,2,2,1,0,3,0,0,0,0,0,2,3,
			3,0,0,1,1,2,3,3,0,1,3,2,0,1,3,3,2,0,0,1,0,2,0,0,0,3,1,3,3,3,3,3,
		},
		{ 0, 1, 5, 6, 7, -1 },
		{ 0, 5 }
	},
	{	// subkey bits 54-59
		{
			2,3,2,3,0,2,3,0,2,2,3,0,3,2,0,2,1,0,2,3,1,1,1,0,0,1,0,2,1,2,2,1,
			3,0,2,1,2,3,3,0,3,2,3,1,0,2,1,0,1,2,2,3,0,2,1,3,1,3,0,2,1,1,1,3,
		},
		{ 2, 3, 4, 6, 7, -1 },
		{ 6, 7 }
	},
	{	// subkey bits 60-65
		{
			3,0,2,1,1,3,1,2,2,1,2,2,2,0,0,1,2,3,1,0,2,0,0,2,3,1,2,0,0,0,3,0,
			2,1,1,2,0,0,1,2,3,1,1,2,0,1,3,0,3,1,1,0,0,2,3,0,0,0,0,3,2,0,0,0,
		},
		{ 0, 2, 3, 4, 5, 6 },
		{ 1, 4 }
	},
	{	// subkey bits 66-71
		{
			0,1,0,0,2,1,3,2,3,3,2,1,0,1,1,1,1,1,0,3,3,1,1,0,0,2,2,1,0,3,3,2,
			1,3,3,0,3,0,2,1,1,2,3,2,2,2,1,0,0,3,3,3,2,2,3,1,0,2,3,0,3,1,1,0,
		},
		{ 0, 1, 2, 3, 5, 7 },
		{ 2, 3 }
	},
};

static const struct sbox fn1_r4_boxes[4] =
{
	{	// subkey bits 72-77
		{
			1,1,1,1,1,0,1,3,3,2,3,0,1,2,0,2,3,3,0,1,2,1,2,3,0,3,2,3,2,0,1,2,
			0,1,0,3,2,1,3,2,3,1,2,3,2,0,1,2,2,0,0,0,2,1,3,0,3,1,3,0,1,3,3,0,
		},
		{ 1, 2, 3, 4, 5, 7 },
		{ 0, 4 }
	},
	{	// subkey bits 78-83
		{
			3,0,0,0,0,1,0,2,3,3,1,3,0,3,1,2,2,2,3,1,0,0,2,0,1,0,2,2,3,3,0,0,
			1,1,3,0,2,3,0,3,0,3,0,2,0,2,0,1,0,3,0,1,3,1,1,0,0,1,3,3,2,2,1,0,
		},
		{ 0, 1, 2, 3, 5, 6 },
		{ 1, 3 }
	},
	{	// subkey bits 84-89
		{
			0,1,1,2,0,1,3,1,2,0,3,2,0,0,3,0,3,0,1,2,2,3,3,2,3,2,0,1,0,0,1,0,
			3,0,2,3,0,2,2,2,1,1,0,2,2,0,0,1,2,1,1,1,2,3,0,3,1,2,3,3,1,1,3,0,
		},
		{ 0, 2, 4, 5, 6, 7 },
		{ 2, 6 }
	},
	{	// subkey bits 90-95
		{
			0,1,2,2,0,1,0,3,2,2,1,1,3,2,0,2,0,1,3,3,0,2,2,3,3,2,0,0,2,1,3,3,
			1,1,1,3,1,2,1,1,0,3,3,2,3,2,3,0,3,1,0,0,3,0,0,0,2,2,2,1,2,3,0,0,
		},
		{ 0, 1, 3, 4, 6, 7 },
		{ 5, 7 }
	},
};

/******************************************************************************/

static const struct sbox fn2_r1_boxes[4] =
{
	{	// subkey bits  0- 5
		{
			2,0,2,0,3,0,0,3,1,1,0,1,3,2,0,1,2,0,1,2,0,2,0,2,2,2,3,0,2,1,3,0,
			0,1,0,1,2,2,3,3,0,3,0,2,3,0,1,2,1,1,0,2,0,3,1,1,2,2,1,3,1,1,3,1,
		},
		{ 0, 3, 4, 5, 7, -1 },
		{ 6, 7 }
	},
	{	// subkey bits  6-11
		{
			1,1,0,3,0,2,0,1,3,0,2,0,1,1,0,0,1,3,2,2,0,2,2,2,2,0,1,3,3,3,1,1,
			1,3,1,3,2,2,2,2,2,2,0,1,0,1,1,2,3,1,1,2,0,3,3,3,2,2,3,1,1,1,3,0,
		},
		{ 1, 2, 3, 4, 6, -1 },
		{ 3, 5 }
	},
	{	// subkey bits 12-17
		{
			1,0,2,2,3,3,3,3,1,2,2,1,0,1,2,1,1,2,3,1,2,0,0,1,2,3,1,2,0,0,0,2,
			2,0,1,1,0,0,2,0,0,0,2,3,2,3,0,1,3,0,0,0,2,3,2,0,1,3,2,1,3,1,1,3,
		},
		{ 1, 2, 4, 5, 6, 7 },
		{ 1, 4 }
	},
	{	// subkey bits 18-23
		{
			1,3,3,0,3,2,3,1,3,2,1,1,3,3,2,1,2,3,0,3,1,0,0,2,3,0,0,0,3,3,0,1,
			2,3,0,0,0,1,2,1,3,0,0,1,0,2,2,2,3,3,1,2,1,3,0,0,0,3,0,1,3,2,2,0,
		},
		{ 0, 2, 3, 5, 6, 7 },
		{ 0, 2 }
	},
};

static const struct sbox fn2_r2_boxes[4] =
{
	{	// subkey bits 24-29
		{
			3,1,3,0,3,0,3,1,3,0,0,1,1,3,0,3,1,1,0,1,2,3,2,3,3,1,2,2,2,0,2,3,
			2,2,2,1,1,3,3,0,3,1,2,1,1,1,0,2,0,3,3,0,0,2,0,0,1,1,2,1,2,1,1,0,
		},
		{ 0, 2, 4, 6, -1, -1 },
		{ 4, 6 }
	},
	{	// subkey bits 30-35
		{
			0,3,0,3,3,2,1,2,3,1,1,1,2,0,2,3,0,3,1,2,2,1,3,3,3,2,1,2,2,0,1,0,
			2,3,0,1,2,0,1,1,2,0,2,1,2,0,2,3,3,1,0,2,3,3,0,3,1,1,3,0,0,1,2,0,
		},
		{ 1, 3, 4, 5, 6, 7 },
		{ 0, 3 }
	},
	{	// subkey bits 36-41
		{
			0,0,2,1,3,2,1,0,1,2,2,2,1,1,0,3,1,2,2,3,2,1,1,0,3,0,0,1,1,2,3,1,
			3,3,2,2,1,0,1,1,1,2,0,1,2,3,0,3,3,0,3,2,2,0,2,2,1,2,3,2,1,0,2,1,
		},
		{ 0, 1, 3, 4, 5, 7 },
		{ 1, 7 }
	},
	{	// subkey bits 42-47
		{
			0,2,1,2,0,2,2,0,1,3,2,0,3,2,3,0,3,3,2,3,1,2,3,1,2,2,0,0,2,2,1,2,
			2,3,3,3,1,1,0,0,0,3,2,0,3,2,3,1,1,1,1,0,1,0,1,3,0,0,1,2,2,3,2,0,
		},
		{ 1, 2, 3, 5, 6, 7 },
		{ 2, 5 }
	},
};

static const struct sbox fn2_r3_boxes[4] =
{
	{	// subkey bits 48-53
		{
			2,1,2,1,2,3,1,3,2,2,1,3,3,0,0,1,0,2,0,3,3,1,0,0,1,1,0,2,3,2,1,2,
			1,1,2,1,1,3,2,2,0,2,2,3,3,3,2,0,0,0,0,0,3,3,3,0,1,2,1,0,2,3,3,1,
		},
		{ 2, 3, 4, 6, -1, -1 },
		{ 3, 5 }
	},
	{	// subkey bits 54-59
		{
			3,2,3,3,1,0,3,0,2,0,1,1,1,0,3,0,3,1,3,1,0,1,2,3,2,2,3,2,0,1,1,2,
			3,0,0,2,1,0,0,2,2,0,1,0,0,2,0,0,1,3,1,3,2,0,3,3,1,0,2,2,2,3,0,0,
		},
		{ 0, 1, 3, 5, 7, -1 },
		{ 0, 2 }
	},
	{	// subkey bits 60-65
		{
			2,2,1,0,2,3,3,0,0,0,1,3,1,2,3,2,2,3,1,3,0,3,0,3,3,2,2,1,0,0,0,2,
			1,2,2,2,0,0,1,2,0,1,3,0,2,3,2,1,3,2,2,2,3,1,3,0,2,0,2,1,0,3,3,1,
		},
		{ 0, 1, 2, 3, 5, 7 },
		{ 1, 6 }
	},
	{	// subkey bits 66-71
		{
			1,2,3,2,0,2,1,3,3,1,0,1,1,2,2,0,0,1,1,1,2,1,1,2,0,1,3,3,1,1,1,2,
			3,3,1,0,2,1,1,1,2,1,0,0,2,2,3,2,3,2,2,0,2,2,3,3,0,2,3,0,2,2,1,1,
		},
		{ 0, 2, 4, 5, 6, 7 },
		{ 4, 7 }
	},
};

static const struct sbox fn2_r4_boxes[4] =
{
	{	// subkey bits 72-77
		{
			2,0,1,1,2,1,3,3,1,1,1,2,0,1,0,2,0,1,2,0,2,3,0,2,3,3,2,2,3,2,0,1,
			3,0,2,0,2,3,1,3,2,0,0,1,1,2,3,1,1,1,0,1,2,0,3,3,1,1,1,3,3,1,1,0,
		},
		{ 0, 1, 3, 6, 7, -1 },
		{ 0, 3 }
	},
	{	// subkey bits 78-83
		{
			1,2,2,1,0,3,3,1,0,2,2,2,1,0,1,0,1,1,0,1,0,2,1,0,2,1,0,2,3,2,3,3,
			2,2,1,2,2,3,1,3,3,3,0,1,0,1,3,0,0,0,1,2,0,3,3,2,3,2,1,3,2,1,0,2,
		},
		{ 0, 1, 2, 4, 5, 6 },
		{ 4, 7 }
	},
	{	// subkey bits 84-89
		{
			2,3,2,1,3,2,3,0,0,2,1,1,0,0,3,2,3,1,0,1,2,2,2,1,3,2,2,1,0,2,1,2,
			0,3,1,0,0,3,1,1,3,3,2,0,1,0,1,3,0,0,1,2,1,2,3,2,1,0,0,3,2,1,1,3,
		},
		{ 0, 2, 3, 4, 5, 7 },
		{ 1, 2 }
	},
	{	// subkey bits 90-95
		{
			2,0,0,3,2,2,2,1,3,3,1,1,2,0,0,3,1,0,3,2,1,0,2,0,3,2,2,3,2,0,3,0,
			1,3,0,2,2,1,3,3,0,1,0,3,1,1,3,2,0,3,0,2,3,2,1,3,2,3,0,0,1,3,2,1,
		},
		{ 2, 3, 4, 5, 6, 7 },
		{ 5, 6 }
	},
};

/******************************************************************************/


static UINT8 fn(UINT8 in, const struct optimised_sbox *sboxes, UINT32 key)
{
	const struct optimised_sbox *sbox1 = &sboxes[0];
	const struct optimised_sbox *sbox2 = &sboxes[1];
	const struct optimised_sbox *sbox3 = &sboxes[2];
	const struct optimised_sbox *sbox4 = &sboxes[3];

	return
		sbox1->output[sbox1->input_lookup[in] ^ ((key >>  0) & 0x3f)] |
		sbox2->output[sbox2->input_lookup[in] ^ ((key >>  6) & 0x3f)] |
		sbox3->output[sbox3->input_lookup[in] ^ ((key >> 12) & 0x3f)] |
		sbox4->output[sbox4->input_lookup[in] ^ ((key >> 18) & 0x3f)];
}



// srckey is the 64-bit master key (2x32 bits)
// dstkey will contain the 96-bit key for the 1st FN (4x24 bits)
static void expand_1st_key(UINT32 *dstkey, const UINT32 *srckey)
{
	static const INT32 bits[96] =
	{
		33, 58, 49, 36,  0, 31,
		22, 30,  3, 16,  5, 53,
		10, 41, 23, 19, 27, 39,
		43,  6, 34, 12, 61, 21,
		48, 13, 32, 35,  6, 42,
		43, 14, 21, 41, 52, 25,
		18, 47, 46, 37, 57, 53,
		20,  8, 55, 54, 59, 60,
		27, 33, 35, 18,  8, 15,
		63,  1, 50, 44, 16, 46,
		 5,  4, 45, 51, 38, 25,
		13, 11, 62, 29, 48,  2,
		59, 61, 62, 56, 51, 57,
		54,  9, 24, 63, 22,  7,
		26, 42, 45, 40, 23, 14,
		 2, 31, 52, 28, 44, 17,
	};
	INT32 i;

	dstkey[0] = 0;
	dstkey[1] = 0;
	dstkey[2] = 0;
	dstkey[3] = 0;

	for (i = 0; i < 96; ++i)
		dstkey[i / 24] |= BIT(srckey[bits[i] / 32], bits[i] % 32) << (i % 24);
}


// srckey is the 64-bit master key (2x32 bits) XORed with the subkey
// dstkey will contain the 96-bit key for the 2nd FN (4x24 bits)
static void expand_2nd_key(UINT32 *dstkey, const UINT32 *srckey)
{
	static const INT32 bits[96] =
	{
		34,  9, 32, 24, 44, 54,
		38, 61, 47, 13, 28,  7,
		29, 58, 18,  1, 20, 60,
		15,  6, 11, 43, 39, 19,
		63, 23, 16, 62, 54, 40,
		31,  3, 56, 61, 17, 25,
		47, 38, 55, 57,  5,  4,
		15, 42, 22,  7,  2, 19,
		46, 37, 29, 39, 12, 30,
		49, 57, 31, 41, 26, 27,
		24, 36, 11, 63, 33, 16,
		56, 62, 48, 60, 59, 32,
		12, 30, 53, 48, 10,  0,
		50, 35,  3, 59, 14, 49,
		51, 45, 44,  2, 21, 33,
		55, 52, 23, 28,  8, 26,
	};
	INT32 i;

	dstkey[0] = 0;
	dstkey[1] = 0;
	dstkey[2] = 0;
	dstkey[3] = 0;

	for (i = 0; i < 96; ++i)
		dstkey[i / 24] |= BIT(srckey[bits[i] / 32], bits[i] % 32) << (i % 24);
}



// seed is the 16-bit seed generated by the first FN
// subkey will contain the 64-bit key to be XORed with the master key
// for the 2nd FN (2x32 bits)
static void expand_subkey(UINT32* subkey, UINT16 seed)
{
	// Note that each row of the table is a permutation of the seed bits.
	static const INT32 bits[64] =
	{
		 5, 10, 14,  9,  4,  0, 15,  6,  1,  8,  3,  2, 12,  7, 13, 11,
		 5, 12,  7,  2, 13, 11,  9, 14,  4,  1,  6, 10,  8,  0, 15,  3,
		 4, 10,  2,  0,  6,  9, 12,  1, 11,  7, 15,  8, 13,  5, 14,  3,
		14, 11, 12,  7,  4,  5,  2, 10,  1, 15,  0,  9,  8,  6, 13,  3,
	};
	INT32 i;

	subkey[0] = 0;
	subkey[1] = 0;

	for (i = 0; i < 64; ++i)
		subkey[i / 32] |= BIT(seed, bits[i]) << (i % 32);
}



static UINT16 feistel(UINT16 val, const INT32 *bitsA, const INT32 *bitsB,
		const struct optimised_sbox* boxes1, const struct optimised_sbox* boxes2, const struct optimised_sbox* boxes3, const struct optimised_sbox* boxes4,
		UINT32 key1, UINT32 key2, UINT32 key3, UINT32 key4)
{
	UINT8 l = BITSWAP8(val, bitsB[7],bitsB[6],bitsB[5],bitsB[4],bitsB[3],bitsB[2],bitsB[1],bitsB[0]);
	UINT8 r = BITSWAP8(val, bitsA[7],bitsA[6],bitsA[5],bitsA[4],bitsA[3],bitsA[2],bitsA[1],bitsA[0]);

	l ^= fn(r, boxes1, key1);
	r ^= fn(l, boxes2, key2);
	l ^= fn(r, boxes3, key3);
	r ^= fn(l, boxes4, key4);

	return
		(BIT(l, 0) << bitsA[0]) |
		(BIT(l, 1) << bitsA[1]) |
		(BIT(l, 2) << bitsA[2]) |
		(BIT(l, 3) << bitsA[3]) |
		(BIT(l, 4) << bitsA[4]) |
		(BIT(l, 5) << bitsA[5]) |
		(BIT(l, 6) << bitsA[6]) |
		(BIT(l, 7) << bitsA[7]) |
		(BIT(r, 0) << bitsB[0]) |
		(BIT(r, 1) << bitsB[1]) |
		(BIT(r, 2) << bitsB[2]) |
		(BIT(r, 3) << bitsB[3]) |
		(BIT(r, 4) << bitsB[4]) |
		(BIT(r, 5) << bitsB[5]) |
		(BIT(r, 6) << bitsB[6]) |
		(BIT(r, 7) << bitsB[7]);
}



static INT32 extract_inputs(UINT32 val, const INT32 *inputs)
{
	INT32 i;
	INT32 res = 0;

	for (i = 0; i < 6; ++i)
	{
		if (inputs[i] != -1)
			res |= BIT(val, inputs[i]) << i;
	}

	return res;
}



static void optimise_sboxes(struct optimised_sbox* out, const struct sbox* in)
{
	INT32 box;

	for (box = 0; box < 4; ++box)
	{
		INT32 i;

		// precalculate the input lookup
		for (i = 0; i < 256; ++i)
		{
			out[box].input_lookup[i] = extract_inputs(i, in[box].inputs);
		}

		// precalculate the output masks
		for (i = 0; i < 64; ++i)
		{
			INT32 o = in[box].table[i];

			out[box].output[i] = 0;
			if (o & 1)
				out[box].output[i] |= 1 << in[box].outputs[0];
			if (o & 2)
				out[box].output[i] |= 1 << in[box].outputs[1];
		}
	}
}

static void cps2_decrypt(const UINT32 *master_key, UINT32 upper_limit)
{
#if 0
	UINT16 *rom = (UINT16 *)memory_region(REGION_CPU1);
	INT32 length = memory_region_length(REGION_CPU1);
	UINT16 *dec = auto_malloc(length);
	INT32 i;
#endif

#if 1
	UINT16 *rom = (UINT16 *)CpsRom;
	UINT32 length = upper_limit;
	CpsCode = (UINT8*)BurnMalloc(length);
	UINT16 *dec = (UINT16*)CpsCode;
	UINT32 i;
#endif

	UINT32 key1[4];
	struct optimised_sbox sboxes1[4*4];
	struct optimised_sbox sboxes2[4*4];

	optimise_sboxes(&sboxes1[0*4], fn1_r1_boxes);
	optimise_sboxes(&sboxes1[1*4], fn1_r2_boxes);
	optimise_sboxes(&sboxes1[2*4], fn1_r3_boxes);
	optimise_sboxes(&sboxes1[3*4], fn1_r4_boxes);
	optimise_sboxes(&sboxes2[0*4], fn2_r1_boxes);
	optimise_sboxes(&sboxes2[1*4], fn2_r2_boxes);
	optimise_sboxes(&sboxes2[2*4], fn2_r3_boxes);
	optimise_sboxes(&sboxes2[3*4], fn2_r4_boxes);
	

	// expand master key to 1st FN 96-bit key
	expand_1st_key(key1, master_key);

	// add extra bits for s-boxes with less than 6 inputs
	key1[0] ^= BIT(key1[0], 1) <<  4;
	key1[0] ^= BIT(key1[0], 2) <<  5;
	key1[0] ^= BIT(key1[0], 8) << 11;
	key1[1] ^= BIT(key1[1], 0) <<  5;
	key1[1] ^= BIT(key1[1], 8) << 11;
	key1[2] ^= BIT(key1[2], 1) <<  5;
	key1[2] ^= BIT(key1[2], 8) << 11;
	
	for (i = 0; i < 0x10000; ++i)
	{
#if 0
		INT32 a;
#endif

#if 1
		UINT32 a;
#endif

		UINT16 seed;
		UINT32 subkey[2];
		UINT32 key2[4];

		if ((i & 0xff) == 0)
		{
#if 0
			char loadingMessage[256]; // for displaying with UI
			sprintf(loadingMessage, "Decrypting %d%%", i*100/0x10000);
			ui_set_startup_text(loadingMessage,FALSE);
#endif

#if 1
			TCHAR loadingMessage[256]; // for displaying with UI 
			_stprintf(loadingMessage, _T("Decrypting 68000 ROMs with key %d %s"), (i*100/0x10000), _T("%")); 
			BurnUpdateProgress(0.0, loadingMessage, 0); 
#endif
		}


		// pass the address through FN1
		seed = feistel(i, fn1_groupA, fn1_groupB,
				&sboxes1[0*4], &sboxes1[1*4], &sboxes1[2*4], &sboxes1[3*4],
				key1[0], key1[1], key1[2], key1[3]);


		// expand the result to 64-bit
		expand_subkey(subkey, seed);

		// XOR with the master key
		subkey[0] ^= master_key[0];
		subkey[1] ^= master_key[1];

		// expand key to 2nd FN 96-bit key
		expand_2nd_key(key2, subkey);

		// add extra bits for s-boxes with less than 6 inputs
		key2[0] ^= BIT(key2[0], 0) <<  5;
		key2[0] ^= BIT(key2[0], 6) << 11;
		key2[1] ^= BIT(key2[1], 0) <<  5;
		key2[1] ^= BIT(key2[1], 1) <<  4;
		key2[2] ^= BIT(key2[2], 2) <<  5;
		key2[2] ^= BIT(key2[2], 3) <<  4;
		key2[2] ^= BIT(key2[2], 7) << 11;
		key2[3] ^= BIT(key2[3], 1) <<  5;

		
		// decrypt the opcodes
		for (a = i; a < length/2 && a < upper_limit/2; a += 0x10000)
		{
			dec[a] = BURN_ENDIAN_SWAP_INT16(feistel(BURN_ENDIAN_SWAP_INT16(rom[a]), fn2_groupA, fn2_groupB,
				&sboxes2[0*4], &sboxes2[1*4], &sboxes2[2*4], &sboxes2[3*4],
				key2[0], key2[1], key2[2], key2[3]));
		}
		// copy the unencrypted part (not really needed)
		while (a < length/2)
		{
			dec[a] = rom[a];
			a += 0x10000;
		}
	}
#if 0
	memory_set_decrypted_region(0, 0x000000, length - 1, dec);
	m68k_set_encrypted_opcode_range(0,0,length);
#endif
}







struct game_keys
{
	const char *name;             /* game driver name */
	const UINT32 keys[2];
	UINT32 upper_limit;
};


/*
(1) On a dead board, the only encrypted range is actually FF0000-FFFFFF.
It doesn't start from 0, and it's the upper half of a 128kB bank.
*/

static const struct game_keys keys_table[] =
{
	// name                 key               upper                  watchdog
	{ "dead",       { 0xffffffff,0xffffffff }, 0        },	// ffff ffff ffff
	{ "ssf2",       { 0x23456789,0xabcdef01 }, 0x400000 },	// 0838 0007 2000  btst    #7,$2000
	{ "ssf2u",      { 0x12345678,0x9abcdef0 }, 0x400000 },	// 0838 0007 2000  btst    #7,$2000
	{ "ssf2a",      { 0x3456789a,0xbcdef012 }, 0x400000 },	// 0838 0007 2000  btst    #7,$2000
	{ "ssf2ar1",    { 0x3456789a,0xbcdef012 }, 0x400000 },	// 0838 0007 2000  btst    #7,$2000
	{ "ssf2j",      { 0x01234567,0x89abcdef }, 0x400000 },	// 0838 0007 2000  btst    #7,$2000
	{ "ssf2jr1",    { 0x01234567,0x89abcdef }, 0x400000 },	// 0838 0007 2000  btst    #7,$2000
	{ "ssf2jr2",    { 0x01234567,0x89abcdef }, 0x400000 },	// 0838 0007 2000  btst    #7,$2000
	{ "ssf2h",      { 0x56789abc,0xdef01234 }, 0x400000 },	// 0838 0007 2000  btst    #7,$2000
	{ "ssf2tb",     { 0x89abcdef,0x01234567 }, 0x400000 },	// 0838 0007 2000  btst    #7,$2000
	{ "ssf2tbr1",   { 0x89abcdef,0x01234567 }, 0x400000 },	// 0838 0007 2000  btst    #7,$2000
	{ "ssf2tbj",    { 0x6789abcd,0xef012345 }, 0x400000 },	// 0838 0007 2000  btst    #7,$2000
	{ "ddtod",      { 0x4767fe08,0x14ca35d9 }, 0x180000 },	// 0C78 1019 4000  cmpi.w  #$1019,$4000
	{ "ddtodr1",    { 0x4767fe08,0x14ca35d9 }, 0x180000 },	// 0C78 1019 4000  cmpi.w  #$1019,$4000
	{ "ddtodu",     { 0xeca19c3d,0x24736bf0 }, 0x180000 },	// 0C78 1019 4000  cmpi.w  #$1019,$4000
	{ "ddtodur1",   { 0xeca19c3d,0x24736bf0 }, 0x180000 },	// 0C78 1019 4000  cmpi.w  #$1019,$4000
	{ "ddtodj",     { 0x4510e79c,0xf36b8a2d }, 0x180000 },	// 0C78 1019 4000  cmpi.w  #$1019,$4000
	{ "ddtodjr1",   { 0x4510e79c,0xf36b8a2d }, 0x180000 },	// 0C78 1019 4000  cmpi.w  #$1019,$4000
	{ "ddtodjr2",   { 0x4510e79c,0xf36b8a2d }, 0x180000 },	// 0C78 1019 4000  cmpi.w  #$1019,$4000
	{ "ddtoda",     { 0xdecac105,0x19710411 }, 0x180000 },	// 0C78 1019 4000  cmpi.w  #$1019,$4000
	{ "ddtodh",     { 0x19691019,0xe825dde0 }, 0x180000 },	// 0C78 1019 4000  cmpi.w  #$1019,$4000
	{ "ddtodhr1",   { 0x19691019,0xe825dde0 }, 0x180000 },	// 0C78 1019 4000  cmpi.w  #$1019,$4000
	{ "ddtodhr2",   { 0x19691019,0xe825dde0 }, 0x180000 },	// 0C78 1019 4000  cmpi.w  #$1019,$4000
	{ "ecofghtr",   { 0x931031dc,0xba987654 }, 0x200000 },	// 0838 0003 7345  btst    #3,$7345
	{ "ecofghtru",  { 0x931031ed,0xcba98765 }, 0x200000 },	// 0838 0003 7345  btst    #3,$7345
	{ "ecofghtru1", { 0x931031ed,0xcba98765 }, 0x200000 },	// 0838 0003 7345  btst    #3,$7345
	{ "uecology",   { 0x931031fe,0xdcba9876 }, 0x200000 },	// 0838 0003 7345  btst    #3,$7345
	{ "ecofghtra",  { 0x931031ba,0x98765432 }, 0x200000 },	// 0838 0003 7345  btst    #3,$7345
	{ "ecofghtrh",  { 0x931031cb,0xa9876543 }, 0x200000 },	// 0838 0003 7345  btst    #3,$7345
	{ "ssf2t",      { 0x944e8302,0x56d3143c }, 0x400000 },	// 0838 0007 2000  btst    #7,$2000
	{ "ssf2ta",     { 0x94c4d002,0x664a1471 }, 0x400000 },	// 0838 0007 2000  btst    #7,$2000
	{ "ssf2tu",     { 0x94fa8902,0x4c77143f }, 0x400000 },	// 0838 0007 2000  btst    #7,$2000
	{ "ssf2tur1",   { 0x94fa8902,0x4c77143f }, 0x400000 },	// 0838 0007 2000  btst    #7,$2000
	{ "ssf2xj",     { 0x942a5702,0x05ac140e }, 0x400000 },	// 0838 0007 2000  btst    #7,$2000
	{ "ssf2xjr",    { 0x943c2b02,0x7acd1422 }, 0x400000 },  // 0838 0007 2000 btst #7,$2000 // curious, not the usual Japan key on the rent version
	{ "xmcota",     { 0x3bc6eda4,0x97f80251 }, 0x100000 },	// 0C80 1972 0301  cmpi.l  #$19720301,D0
	{ "xmcotau",    { 0x32a57ecd,0x98016f4b }, 0x100000 },	// 0C80 1972 0301  cmpi.l  #$19720301,D0
	{ "xmcotah",    { 0xf5e8dc34,0xa096b217 }, 0x100000 },	// 0C80 1972 0301  cmpi.l  #$19720301,D0
	{ "xmcotahr1",  { 0xf5e8dc34,0xa096b217 }, 0x100000 },	// 0C80 1972 0301  cmpi.l  #$19720301,D0
	{ "xmcotaj",    { 0x46027315,0xaf8bcd9e }, 0x100000 },	// 0C80 1972 0301  cmpi.l  #$19720301,D0
	{ "xmcotaj1",   { 0x46027315,0xaf8bcd9e }, 0x100000 },	// 0C80 1972 0301  cmpi.l  #$19720301,D0
	{ "xmcotaj2",   { 0x46027315,0xaf8bcd9e }, 0x100000 },	// 0C80 1972 0301  cmpi.l  #$19720301,D0
	{ "xmcotaj3",   { 0x46027315,0xaf8bcd9e }, 0x100000 },	// 0C80 1972 0301  cmpi.l  #$19720301,D0
	{ "xmcotajr",   { 0x46027315,0xaf8bcd9e }, 0x100000 },	// 0C80 1972 0301  cmpi.l  #$19720301,D0
	{ "xmcotaa",    { 0x0795a4e2,0xdb3f861c }, 0x100000 },	// 0C80 1972 0301  cmpi.l  #$19720301,D0
	{ "xmcotaar1",  { 0x0795a4e2,0xdb3f861c }, 0x100000 },	// 0C80 1972 0301  cmpi.l  #$19720301,D0
	{ "armwar",     { 0x9e9d4c0b,0x8a39081f }, 0x100000 },	// 3039 0080 4020  move.w  $00804020,D0
	{ "armwarr1",   { 0x9e9d4c0b,0x8a39081f }, 0x100000 },	// 3039 0080 4020  move.w  $00804020,D0
	{ "armwaru",    { 0xd4c0b8a3,0x9081f9e9 }, 0x100000 },	// 3039 0080 4020  move.w  $00804020,D0
	{ "armwaru1",   { 0xd4c0b8a3,0x9081f9e9 }, 0x100000 },	// 3039 0080 4020  move.w  $00804020,D0
	{ "pgear",      { 0x9d4c0b8a,0x39081f9e }, 0x100000 },	// 3039 0080 4020  move.w  $00804020,D0
	{ "pgearr1",    { 0x9d4c0b8a,0x39081f9e }, 0x100000 },	// 3039 0080 4020  move.w  $00804020,D0
	{ "armwara",    { 0x1f9e9d4c,0x0b8a3908 }, 0x100000 },	// 3039 0080 4020  move.w  $00804020,D0
	{ "avsp",       { 0x15208f79,0x4ade6cb3 }, 0x100000 },	// 0C80 1234 5678  cmpi.l  #$12345678,D0
	{ "avspu",      { 0xb4f61089,0xccf75a23 }, 0x100000 },	// 0C80 1234 5678  cmpi.l  #$12345678,D0
	{ "avspj",      { 0xe9dcb8fa,0x51372064 }, 0x100000 },	// 0C80 1234 5678  cmpi.l  #$12345678,D0
	{ "avspa",      { 0xc168f3bd,0x2e4a5970 }, 0x100000 },	// 0C80 1234 5678  cmpi.l  #$12345678,D0
	{ "avsph",      { 0x712b690a,0x43cd8e5f }, 0x100000 },	// 0C80 1234 5678  cmpi.l  #$12345678,D0
	{ "dstlk",      { 0x13d8a7a8,0x0008b090 }, 0x100000 },	// 0838 0000 6160  btst    #0,$6160
	{ "dstlku",     { 0x1e80ebf0,0x10227119 }, 0x100000 },	// 0838 0000 6160  btst    #0,$6160
	{ "dstlkur1",   { 0x1e80ebf0,0x10227119 }, 0x100000 },	// 0838 0000 6160  btst    #0,$6160
	{ "dstlka",     { 0x205d8398,0x06221971 }, 0x100000 },	// 0838 0000 6160  btst    #0,$6160
	{ "dstlkh",     { 0x22463efe,0x011169aa }, 0x100000 },	// 0838 0000 6160  btst    #0,$6160
	{ "vampj",      { 0xefcb0804,0x026819ae }, 0x100000 },	// 0838 0000 6160  btst    #0,$6160
	{ "vampja",     { 0xefcb0804,0x026819ae }, 0x100000 },	// 0838 0000 6160  btst    #0,$6160
	{ "vampjr1",    { 0xefcb0804,0x026819ae }, 0x100000 },	// 0838 0000 6160  btst    #0,$6160
	{ "ringdest",   { 0x19940727,0x17444903 }, 0x180000 },	// 3039 0080 4020  move.w  $00804020,D0
	{ "smbomb",     { 0x19940209,0x17031403 }, 0x180000 },	// 3039 0080 4020  move.w  $00804020,D0
	{ "smbombr1",   { 0x19940209,0x17031403 }, 0x180000 },	// 3039 0080 4020  move.w  $00804020,D0
	{ "ringdesta",  { 0x19940727,0x17452103 }, 0x180000 },	// 3039 0080 4020  move.w  $00804020,D0
	{ "cybots",     { 0x45425943,0x05090901 }, 0x100000 },	// 0C38 00FF 0C38  cmpi.b  #$FF,$0C38
	{ "cybotsu",    { 0x43050909,0x01554259 }, 0x100000 },	// 0C38 00FF 0C38  cmpi.b  #$FF,$0C38
	{ "cybotsj",    { 0x05090901,0x4a425943 }, 0x100000 },	// 0C38 00FF 0C38  cmpi.b  #$FF,$0C38
	{ "msh",        { 0x1a11ee26,0xe7955d17 }, 0x100000 },	// 0C81 1966 0419  cmpi.l  #$19660419,D1
	{ "mshu",       { 0x8705a24e,0x4a17319b }, 0x100000 },	// 0C81 1966 0419  cmpi.l  #$19660419,D1
	{ "mshj",       { 0x05e88219,0x31ad2142 }, 0x100000 },	// 0C81 1966 0419  cmpi.l  #$19660419,D1
	{ "mshjr1",     { 0x05e88219,0x31ad2142 }, 0x100000 },	// 0C81 1966 0419  cmpi.l  #$19660419,D1
	{ "msha",       { 0x457aeb01,0x3897c53d }, 0x100000 },	// 0C81 1966 0419  cmpi.l  #$19660419,D1
	{ "mshh",       { 0xfc4c5a50,0xb59cc190 }, 0x100000 },	// 0C81 1966 0419  cmpi.l  #$19660419,D1
	{ "mshb",       { 0x7a152416,0xad27f8e6 }, 0x100000 },	// 0C81 1966 0419  cmpi.l  #$19660419,D1
	{ "nwarr",      { 0x1019d145,0x03f05a05 }, 0x180000 },	// 0838 0000 6160  btst    #0,$6160
	{ "nwarru",     { 0x104a7d0c,0x3f1b7a1e }, 0x180000 },	// 0838 0000 6160  btst    #0,$6160
	{ "nwarrh",     { 0xc4961b01,0x2a946020 }, 0x180000 },	// 0838 0000 6160  btst    #0,$6160
	{ "nwarrb",     { 0x17c67109,0xb7362a20 }, 0x180000 },	// 0838 0000 6160  btst    #0,$6160
	{ "nwarra",     { 0x4e940d0c,0x39b861a4 }, 0x180000 },	// 0838 0000 6160  btst    #0,$6160
	{ "vhuntj",     { 0x1135b2c3,0xa4e9d7f2 }, 0x180000 },	// 0838 0000 6160  btst    #0,$6160
	{ "vhuntjr1s",  { 0x1135b2c3,0xa4e9d7f2 }, 0x180000 },  // 0838 0000 6160 btst #0,$6160
	{ "vhuntjr1",   { 0x1135b2c3,0xa4e9d7f2 }, 0x180000 },	// 0838 0000 6160  btst    #0,$6160
	{ "vhuntjr2",   { 0x1135b2c3,0xa4e9d7f2 }, 0x180000 },	// 0838 0000 6160  btst    #0,$6160
	{ "sfa",        { 0x0f895d6e,0xc4273a1b }, 0x080000 },	// 0C80 0564 2194  cmpi.l  #$05642194,D0
	{ "sfar1",      { 0x0f895d6e,0xc4273a1b }, 0x080000 },	// 0C80 0564 2194  cmpi.l  #$05642194,D0
	{ "sfar2",      { 0x0f895d6e,0xc4273a1b }, 0x080000 },	// 0C80 0564 2194  cmpi.l  #$05642194,D0
	{ "sfar3",      { 0x0f895d6e,0xc4273a1b }, 0x080000 },	// 0C80 0564 2194  cmpi.l  #$05642194,D0
	{ "sfau",       { 0x25bead36,0x97cf4018 }, 0x080000 },	// 0C80 0564 2194  cmpi.l  #$05642194,D0
	{ "sfza",       { 0xe43dc508,0x621b9a7f }, 0x080000 },	// 0C80 0564 2194  cmpi.l  #$05642194,D0
	{ "sfzj",       { 0x8db3167a,0xc29e0f45 }, 0x080000 },	// 0C80 0564 2194  cmpi.l  #$05642194,D0
	{ "sfzjr1",     { 0x8db3167a,0xc29e0f45 }, 0x080000 },	// 0C80 0564 2194  cmpi.l  #$05642194,D0
	{ "sfzjr2",     { 0x8db3167a,0xc29e0f45 }, 0x080000 },	// 0C80 0564 2194  cmpi.l  #$05642194,D0
	{ "sfzh",       { 0x876b0e39,0x5ca24fd1 }, 0x080000 },	// 0C80 0564 2194  cmpi.l  #$05642194,D0
	{ "sfzhr1",     { 0x876b0e39,0x5ca24fd1 }, 0x080000 },	// 0C80 0564 2194  cmpi.l  #$05642194,D0
	{ "sfzb",       { 0xef415bd3,0x7a92c680 }, 0x080000 },	// 0C80 0564 2194  cmpi.l  #$05642194,D0
	{ "sfzbr1",     { 0xef415bd3,0x7a92c680 }, 0x080000 },	// 0C80 0564 2194  cmpi.l  #$05642194,D0
	{ "mmancp2u",   { 0x054893fa,0x94642525 }, 0x100000 },    // 0C80 0564 2194  cmpi.l  #$05642194,D0
	{ "rmancp2j",   { 0x07215501,0x37fa32d0 }, 0x100000 },	// 0C80 0564 2194  cmpi.l  #$05642194,D0
	{ "19xx",       { 0x0e07181f,0x5fd0f080 }, 0x200000 },	// 0C81 0095 1101  cmpi.l  #$00951101,D1
	{ "19xxa",      { 0xcce74cf5,0xb7da3711 }, 0x200000 },	// 0C81 0095 1101  cmpi.l  #$00951101,D1
	{ "19xxj",      { 0x00115df8,0x000ff87e }, 0x200000 },	// 0C81 0095 1101  cmpi.l  #$00951101,D1
	{ "19xxjr1",    { 0x00115df8,0x000ff87e }, 0x200000 },	// 0C81 0095 1101  cmpi.l  #$00951101,D1
	{ "19xxh",      { 0x5d49bafa,0xf7216c9f }, 0x200000 },	// 0C81 0095 1101  cmpi.l  #$00951101,D1
	{ "19xxb",      { 0xe5f9476a,0x2dfb623f }, 0x200000 },	// 0C81 0095 1101  cmpi.l  #$00951101,D1
	{ "ddsom",      { 0x87889abc,0xd81f5f63 }, 0x100000 },	// 0C81 1966 0419  cmpi.l  #$19660419,D1
	{ "ddsomr1",    { 0x87889abc,0xd81f5f63 }, 0x100000 },	// 0C81 1966 0419  cmpi.l  #$19660419,D1
	{ "ddsomr2",    { 0x87889abc,0xd81f5f63 }, 0x100000 },	// 0C81 1966 0419  cmpi.l  #$19660419,D1
	{ "ddsomr3",    { 0x87889abc,0xd81f5f63 }, 0x100000 },	// 0C81 1966 0419  cmpi.l  #$19660419,D1
	{ "ddsomu",     { 0x489f0526,0x1bcd3e7a }, 0x100000 },	// 0C81 1966 0419  cmpi.l  #$19660419,D1
	{ "ddsomur1",   { 0x489f0526,0x1bcd3e7a }, 0x100000 },	// 0C81 1966 0419  cmpi.l  #$19660419,D1
	{ "ddsomj",     { 0xae92fa94,0x315a9045 }, 0x100000 },	// 0C81 1966 0419  cmpi.l  #$19660419,D1
	{ "ddsomjr1",   { 0xae92fa94,0x315a9045 }, 0x100000 },	// 0C81 1966 0419  cmpi.l  #$19660419,D1
	{ "ddsoma",     { 0x8719abcd,0xef028345 }, 0x100000 },	// 0C81 1966 0419  cmpi.l  #$19660419,D1
	{ "ddsomh",     { 0x42134245,0x120de607 }, 0x100000 },	// 0C81 1966 0419  cmpi.l  #$19660419,D1
	{ "ddsomb",     { 0x7149a782,0xf3a5bfce }, 0x100000 },	// 0C81 1966 0419  cmpi.l  #$19660419,D1
	{ "megaman2",   { 0x50501cac,0xed346550 }, 0x100000 },	// 0C80 0164 7101  cmpi.l  #$01647101,D0
	{ "megaman2a",  { 0x3f148a2b,0xd6790a15 }, 0x100000 },	// 0C80 0164 7101  cmpi.l  #$01647101,D0
	{ "rockman2j",  { 0x319eca73,0x10551270 }, 0x100000 },	// 0C80 0164 7101  cmpi.l  #$01647101,D0
	{ "megaman2h"  ,{ 0x765573ca,0x250210d0 }, 0x100000 },	// 0C80 0164 7101  cmpi.l  #$01647101,D0
	{ "qndream",    { 0x5804ea73,0xf66b0798 }, 0x080000 },	// 0C81 1973 0827  cmpi.l  #$19730827,D1
	{ "sfa2",       { 0xfc4acf9c,0x3bfbe1f9 }, 0x100000 },	// 0C80 3039 9783  cmpi.l  #$30399783,D0
	{ "sfa2u",      { 0x1bbf3d96,0x8af4614a }, 0x100000 },	// 0C80 3039 9783  cmpi.l  #$30399783,D0
	{ "sfa2ur1",    { 0x1bbf3d96,0x8af4614a }, 0x100000 },	// 0C80 3039 9783  cmpi.l  #$30399783,D0
	{ "sfz2j",      { 0x83f47e99,0xda772111 }, 0x100000 },	// 0C80 3039 9783  cmpi.l  #$30399783,D0
	{ "sfz2a",      { 0xafc2e8f4,0x43789487 }, 0x100000 },	// 0C80 3039 9783  cmpi.l  #$30399783,D0
	{ "sfz2b",      { 0xac134599,0x61f8bb2e }, 0x100000 },	// 0C80 3039 9783  cmpi.l  #$30399783,D0
	{ "sfz2br1",    { 0xac134599,0x61f8bb2e }, 0x100000 },	// 0C80 3039 9783  cmpi.l  #$30399783,D0
	{ "sfz2h",      { 0xf98a2d42,0x597b089f }, 0x100000 },	// 0C80 3039 9783  cmpi.l  #$30399783,D0
	{ "sfz2n",      { 0xe32bf89c,0xa57b46dc }, 0x100000 },	// 0C80 3039 9783  cmpi.l  #$30399783,D0
	{ "sfz2al",     { 0xf172c0d0,0x040621a6 }, 0x100000 },	// 0C80 8E73 9110  cmpi.l  #$8E739110,D0
	{ "sfz2alj",    { 0x99450c88,0xa00a2c4d }, 0x100000 },	// 0C80 8E73 9110  cmpi.l  #$8E739110,D0
	{ "sfz2alh",    { 0x95f15b7c,0x200c08c6 }, 0x100000 },	// 0C80 8E73 9110  cmpi.l  #$8E739110,D0
	{ "sfz2alb",    { 0x73cd4a28,0xff83af1c }, 0x100000 },	// 0C80 8E73 9110  cmpi.l  #$8E739110,D0
	{ "spf2t",      { 0x706a8750,0x7d0fc185 }, 0x040000 },	// 0C80 3039 9819  cmpi.l  #$30399819,D0
	{ "spf2xj",     { 0xb12c835a,0xe90976ff }, 0x040000 },	// 0C80 3039 9819  cmpi.l  #$30399819,D0
	{ "spf2ta",     { 0x9c48e1ab,0xd60f34fb }, 0x040000 },	// 0C80 3039 9819  cmpi.l  #$30399819,D0
	{ "spf2th",     { 0x51ed8cab,0x228f85b6 }, 0x040000 },	// 0C80 3039 9819  cmpi.l  #$30399819,D0
	{ "xmvsf",      { 0xbdcf8519,0x3fb2acea }, 0x100000 },	// 0C81 1972 0327  cmpi.l  #$19720327,D1
	{ "xmvsfr1",    { 0xbdcf8519,0x3fb2acea }, 0x100000 },	// 0C81 1972 0327  cmpi.l  #$19720327,D1
	{ "xmvsfu",     { 0x4fcb03d2,0xf8653bc1 }, 0x100000 },	// 0C81 1972 0327  cmpi.l  #$19720327,D1
	{ "xmvsfur1",   { 0x4fcb03d2,0xf8653bc1 }, 0x100000 },	// 0C81 1972 0327  cmpi.l  #$19720327,D1
	{ "xmvsfj",     { 0x38df93bc,0x210373ac }, 0x100000 },	// 0C81 1972 0327  cmpi.l  #$19720327,D1
	{ "xmvsfjr1",   { 0x38df93bc,0x210373ac }, 0x100000 },	// 0C81 1972 0327  cmpi.l  #$19720327,D1
	{ "xmvsfjr2",   { 0x38df93bc,0x210373ac }, 0x100000 },	// 0C81 1972 0327  cmpi.l  #$19720327,D1
	{ "xmvsfa",     { 0x7438fc3e,0x19abed90 }, 0x100000 },	// 0C81 1972 0327  cmpi.l  #$19720327,D1
	{ "xmvsfar1",   { 0x7438fc3e,0x19abed90 }, 0x100000 },	// 0C81 1972 0327  cmpi.l  #$19720327,D1
	{ "xmvsfar2",   { 0x7438fc3e,0x19abed90 }, 0x100000 },	// 0C81 1972 0327  cmpi.l  #$19720327,D1
	{ "xmvsfh",     { 0x835fb2d0,0x42fa9137 }, 0x100000 },	// 0C81 1972 0327  cmpi.l  #$19720327,D1
	{ "xmvsfb",     { 0x8ead9e4a,0xb02184f0 }, 0x100000 },	// 0C81 1972 0327  cmpi.l  #$19720327,D1
	{ "batcir",     { 0xd195e597,0x3cbce2b5 }, 0x200000 },	// 0C81 0097 0131  cmpi.l  #$00970131,D1
	{ "batcira",    { 0x1e5d80cb,0x98882ec7 }, 0x200000 },	// 0C81 0097 0131  cmpi.l  #$00970131,D1
	{ "batcirj",    { 0x00ff4dd8,0x000008e8 }, 0x200000 },	// 0C81 0097 0131  cmpi.l  #$00970131,D1
	{ "csclub",     { 0x662e9fa0,0x4210e7c1 }, 0x200000 },	// 0C81 0097 0310  cmpi.l  #$00970310,D1
	{ "csclub1",    { 0x662e9fa0,0x4210e7c1 }, 0x200000 },	// 0C81 0097 0310  cmpi.l  #$00970310,D1
	{ "cscluba",    { 0x1366de2a,0x9ab42937 }, 0x200000 },	// 0C81 0097 0310  cmpi.l  #$00970310,D1
	{ "csclubj",    { 0x4a2d0be5,0x56c013c0 }, 0x200000 },	// 0C81 0097 0310  cmpi.l  #$00970310,D1
	{ "csclubjr",   { 0x4a2d0be5,0x56c013c0 }, 0x200000 },	// 0C81 0097 0310  cmpi.l  #$00970310,D1
	{ "csclubh",    { 0xf014a8a7,0x2e7794d0 }, 0x200000 },	// 0C81 0097 0310  cmpi.l  #$00970310,D1
	{ "mshvsf",     { 0x1384ae60,0x9cd725bf }, 0x100000 },	// 0C81 1972 1027  cmpi.l  #$19721027,D1
	{ "mshvsfu",    { 0xa36d4971,0xcef51b28 }, 0x100000 },	// 0C81 1972 1027  cmpi.l  #$19721027,D1
	{ "mshvsfu1",   { 0xa36d4971,0xcef51b28 }, 0x100000 },	// 0C81 1972 1027  cmpi.l  #$19721027,D1
	{ "mshvsfj",    { 0x5dc391f8,0xa627e0b4 }, 0x100000 },	// 0C81 1972 1027  cmpi.l  #$19721027,D1
	{ "mshvsfj1",   { 0x5dc391f8,0xa627e0b4 }, 0x100000 },	// 0C81 1972 1027  cmpi.l  #$19721027,D1
	{ "mshvsfj2",   { 0x5dc391f8,0xa627e0b4 }, 0x100000 },	// 0C81 1972 1027  cmpi.l  #$19721027,D1
	{ "mshvsfh",    { 0x7e916fc4,0x03ab852d }, 0x100000 },	// 0C81 1972 1027  cmpi.l  #$19721027,D1
	{ "mshvsfa",    { 0x52e3fa61,0x0c497bd8 }, 0x100000 },	// 0C81 1972 1027  cmpi.l  #$19721027,D1
	{ "mshvsfa1",   { 0x52e3fa61,0x0c497bd8 }, 0x100000 },	// 0C81 1972 1027  cmpi.l  #$19721027,D1
	{ "mshvsfb",    { 0xda68c749,0x5bf3e201 }, 0x100000 },	// 0C81 1972 1027  cmpi.l  #$19721027,D1
	{ "mshvsfb1",   { 0xda68c749,0x5bf3e201 }, 0x100000 },	// 0C81 1972 1027  cmpi.l  #$19721027,D1
	{ "sgemf",      { 0x84234976,0x5e0fbb7e }, 0x080000 },	// 0C80 1F74 0D12  cmpi.l  #$1F740D12,D0
	{ "pfghtj",     { 0x97d2ebc0,0x308f94d7 }, 0x080000 },	// 0C80 1F74 0D12  cmpi.l  #$1F740D12,D0
	{ "sgemfa",     { 0x090b412a,0xc47ee993 }, 0x080000 },	// 0C80 1F74 0D12  cmpi.l  #$1F740D12,D0
	{ "sgemfh",     { 0x8163a71b,0x7c8fd224 }, 0x080000 },	// 0C80 1F74 0D12  cmpi.l  #$1F740D12,D0
	{ "vhunt2",     { 0x36c1eba3,0x26b10f18 }, 0x100000 },	// 0C80 0692 0760  cmpi.l  #$06920760,D0
	{ "vhunt2r1",   { 0x36c1eba3,0x26b10f18 }, 0x100000 },	// 0C80 0692 0760  cmpi.l  #$06920760,D0
	{ "vsav",       { 0xe0cd5881,0x71babb70 }, 0x100000 },	// 0C80 726A 4BAF  cmpi.l  #$726A4BAF,D0
	{ "vsavu",      { 0xa62ea0ee,0x573e03e6 }, 0x100000 },	// 0C80 726A 4BAF  cmpi.l  #$726A4BAF,D0
	{ "vsavj",      { 0xfa8f4e33,0xa4b881b9 }, 0x100000 },	// 0C80 726A 4BAF  cmpi.l  #$726A4BAF,D0
	{ "vsava",      { 0x47ee9930,0x90b412ac }, 0x100000 },	// 0C80 726A 4BAF  cmpi.l  #$726A4BAF,D0
	{ "vsavh",      { 0xb2d37c8d,0xd3b7aadd }, 0x100000 },	// 0C80 726A 4BAF  cmpi.l  #$726A4BAF,D0
	{ "vsav2",      { 0xd681e4f4,0x60371edf }, 0x100000 },	// 0C80 0692 0760  cmpi.l  #$06920760,D0
	{ "mvsc",       { 0x48025ade,0x1c697b27 }, 0x100000 },	// 0C81 1972 0121  cmpi.l  #$19720121,D1
	{ "mvscr1",     { 0x48025ade,0x1c697b27 }, 0x100000 },	// 0C81 1972 0121  cmpi.l  #$19720121,D1
	{ "mvscu",      { 0x692dc41b,0x7ef1c805 }, 0x100000 },	// 0C81 1972 0121  cmpi.l  #$19720121,D1
	{ "mvscur1",    { 0x692dc41b,0x7ef1c805 }, 0x100000 },	// 0C81 1972 0121  cmpi.l  #$19720121,D1
	{ "mvscj",      { 0xafc16138,0x97123eb0 }, 0x100000 },	// 0C81 1972 0121  cmpi.l  #$19720121,D1
	{ "mvscjr1",    { 0xafc16138,0x97123eb0 }, 0x100000 },	// 0C81 1972 0121  cmpi.l  #$19720121,D1
	{ "mvsca",      { 0xf248aec6,0x7905cd17 }, 0x100000 },	// 0C81 1972 0121  cmpi.l  #$19720121,D1
	{ "mvscar1",    { 0xf248aec6,0x7905cd17 }, 0x100000 },	// 0C81 1972 0121  cmpi.l  #$19720121,D1
	{ "mvsch",      { 0x9d5c7a23,0xe56b18ef }, 0x100000 },	// 0C81 1972 0121  cmpi.l  #$19720121,D1
	{ "mvscb",      { 0x0874d6eb,0x51c2b798 }, 0x100000 },	// 0C81 1972 0121  cmpi.l  #$19720121,D1
	{ "sfa3",       { 0x6abfc8e0,0x2780ddc1 }, 0x100000 },	// 0C80 1C62 F5A8  cmpi.l  #$1C62F5A8,D0
	{ "sfa3u",      { 0xe7bbf0e5,0x67943248 }, 0x100000 },	// 0C80 1C62 F5A8  cmpi.l  #$1C62F5A8,D0
	{ "sfa3ur1",    { 0xe7bbf0e5,0x67943248 }, 0x100000 },	// 0C80 1C62 F5A8  cmpi.l  #$1C62F5A8,D0
	{ "sfa3h",      { 0x8422df8c,0x7b17a361 }, 0x100000 },	// 0C80 1C62 F5A8  cmpi.l  #$1C62F5A8,D0
	{ "sfa3hr1",    { 0x8422df8c,0x7b17a361 }, 0x100000 },	// 0C80 1C62 F5A8  cmpi.l  #$1C62F5A8,D0
	{ "sfa3b",      { 0xd421c0b2,0x8116d296 }, 0x100000 },	// 0C80 1C62 F5A8  cmpi.l  #$1C62F5A8,D0
	{ "sfz3j",      { 0x7d49f803,0x0cbe2d79 }, 0x100000 },	// 0C80 1C62 F5A8  cmpi.l  #$1C62F5A8,D0
	{ "sfz3jr1",    { 0x7d49f803,0x0cbe2d79 }, 0x100000 },	// 0C80 1C62 F5A8  cmpi.l  #$1C62F5A8,D0
	{ "sfz3jr2",    { 0x7d49f803,0x0cbe2d79 }, 0x100000 },	// 0C80 1C62 F5A8  cmpi.l  #$1C62F5A8,D0
	{ "sfz3a",      { 0x990b9301,0xa4e42c7e }, 0x100000 },	// 0C80 1C62 F5A8  cmpi.l  #$1C62F5A8,D0
	{ "sfz3ar1",    { 0x990b9301,0xa4e42c7e }, 0x100000 },	// 0C80 1C62 F5A8  cmpi.l  #$1C62F5A8,D0
	{ "jyangoku",   { 0x6ca42ae6,0x92f63f59 }, 0        },	// 0C80 3652 1573  cmpi.l  #$36521573,D0
	{ "hsf2",       { 0x5a369ddd,0xfea3189c }, 0x100000 },	// 0838 0007 2000  btst    #7,$2000
	{ "hsf2a",      { 0xb8ed3630,0xaae30a3d }, 0x100000 },	// 0838 0007 2000  btst    #7,$2000
	{ "hsf2j",      { 0x65d82fe0,0xdbb83e47 }, 0x100000 },	// 0838 0007 2000  btst    #7,$2000
	{ "gigawing",   { 0xe52de290,0x1b60d780 }, 0x100000 },	// 0C81 1972 1027  cmpi.l  #$19721027,D1
	{ "gigawingj",  { 0x1126196a,0xbef50895 }, 0x100000 },	// 0C81 1972 1027  cmpi.l  #$19721027,D1
	{ "gigawinga",  { 0x3506a85a,0x66b1b768 }, 0x100000 },	// 0C81 1972 1027  cmpi.l  #$19721027,D1
	{ "gigawingh",  { 0xcea74211,0x400da385 }, 0x100000 },	// 0C81 1972 1027  cmpi.l  #$19721027,D1
	{ "gigawingb",  { 0x0fe745b0,0x96ef7f9d }, 0x100000 },	// 0C81 1972 1027  cmpi.l  #$19721027,D1
	{ "mmatrix",    { 0xac9ebd79,0x410467df }, 0x180000 },	// B6C0 B447 BACF  cmpa.w  D0,A3   cmp.w   D7,D2   cmpa.w  A7,A5
	{ "mmatrixj",   { 0x4df81e95,0x72ed9823 }, 0x180000 },	// B6C0 B447 BACF  cmpa.w  D0,A3   cmp.w   D7,D2   cmpa.w  A7,A5
	{ "mpang",      { 0x95f741c6,0xe547a21b }, 0x100000 },	// 0C84 347D 89A3  cmpi.l  #$347D89A3,D4
	{ "mpangr1",    { 0x95f741c6,0xe547a21b }, 0x100000 },	// 0C84 347D 89A3  cmpi.l  #$347D89A3,D4
	{ "mpangu",     { 0x95f741c6,0xe547a21b }, 0x100000 },	// 0C84 347D 89A3  cmpi.l  #$347D89A3,D4
	{ "mpangj",     { 0x95f741c6,0xe547a21b }, 0x100000 },	// 0C84 347D 89A3  cmpi.l  #$347D89A3,D4
	{ "pzloop2",    { 0xa054f812,0xc40d36b4 }, 0x400000 },	// 0C82 9A73 15F1  cmpi.l  #$9A7315F1,D2
	{ "pzloop2j",   { 0xa054f812,0xc40d36b4 }, 0x400000 },	// 0C82 9A73 15F1  cmpi.l  #$9A7315F1,D2
	{ "pzloop2jr1", { 0xa054f812,0xc40d36b4 }, 0x400000 },	// 0C82 9A73 15F1  cmpi.l  #$9A7315F1,D2
	{ "choko",      { 0xd3fb12c6,0x7f8e17b5 }, 0x400000 },	// 0C86 4D17 5B3C  cmpi.l  #$4D175B3C,D6
	{ "dimahoo",    { 0x0ddb8e40,0x2817fd2b }, 0x080000 },	// BE4C B244 B6C5  cmp.w   A4,D7   cmp.w   D4,D1   cmpa.w  D5,A3
	{ "dimahoou",   { 0x6575af59,0xb0fea691 }, 0x080000 },	// BE4C B244 B6C5  cmp.w   A4,D7   cmp.w   D4,D1   cmpa.w  D5,A3
	{ "gmahou",     { 0x97f7be58,0x6121eb62 }, 0x080000 },	// BE4C B244 B6C5  cmp.w   A4,D7   cmp.w   D4,D1   cmpa.w  D5,A3
	{ "1944",       { 0x1d3e724c,0x8b59fc7a }, 0x080000 },	// 0C86 7B5D 94F1  cmpi.l  #$7B5D94F1,D6
	{ "1944j",      { 0x23d79c3a,0xe18b2746 }, 0x080000 },	// 0C86 7B5D 94F1  cmpi.l  #$7B5D94F1,D6
	{ "progear",    { 0x639ad8c6,0xef130df3 }, 0x400000 },	// 0C81 63A1 B8D3  cmpi.l  #$63A1B8D3,D1
	{ "progearj",   { 0x9f7edc56,0x39fb47be }, 0x400000 },	// 0C81 63A1 B8D3  cmpi.l  #$63A1B8D3,D1
	{ "progeara",   { 0x658ab128,0xfddc9b5e }, 0x400000 },	// 0C81 63A1 B8D3  cmpi.l  #$63A1B8D3,D1

#if 0
	{ 0 }	// end of table
#endif

#if 1
	{ 0,          { 0,         0          }, 0 }	// end of table
#endif
};



#if 1
void cps2_decrypt_game_data()
{
	const char *gamename = BurnDrvGetTextA(DRV_NAME);
	const struct game_keys *k = &keys_table[0];

	while (k->name)
	{
		if (strcmp(k->name, gamename) == 0)
		{
			break;
		}
		++k;
	}

	if (k->name)
	{
		nCpsCodeLen = k->upper_limit ? k->upper_limit : nCpsRomLen;
		
		// we have a proper key so use it to decrypt
		cps2_decrypt(k->keys, nCpsCodeLen);
	}
	else
	{
		// we don't have a proper key so use the XOR tables if available
		if (CpsCode) {
			UINT16 *rom = (UINT16*)CpsRom;
			UINT16 *xor1 = (UINT16*)CpsCode;
			INT32 length = nCpsCodeLen;
			INT32 i;

			if (xor1)
			{
				for (i = 0; i < length/2; i++)
					xor1[i] ^= rom[i];
			}
		}
	}
}
#endif

#if 0
DRIVER_INIT( cps2 )
{
	const char *gamename = machine->gamedrv->name;
	const struct game_keys *k = &keys_table[0];

	while (k->name)
	{
		if (strcmp(k->name, gamename) == 0)
		{
			// we have a proper key so use it to decrypt
			cps2_decrypt(k->keys, k->upper_limit ? k->upper_limit : 0x400000);

			break;
		}
		++k;
	}

	init_cps2_video(machine);
}
#endif
