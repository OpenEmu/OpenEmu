#include "sys16.h"
#include "bitswap.h"

struct parameters
{
	INT32 xor1;
	INT32 s7,s6,s5,s4,s3,s2,s1,s0;
};

static UINT8 basetable_fd1089[0x100] =
{
	0x00,0x1c,0x76,0x6a,0x5e,0x42,0x24,0x38,0x4b,0x67,0xad,0x81,0xe9,0xc5,0x03,0x2f,
	0x45,0x69,0xaf,0x83,0xe7,0xcb,0x01,0x2d,0x02,0x1e,0x78,0x64,0x5c,0x40,0x2a,0x36,
	0x32,0x2e,0x44,0x58,0xe4,0xf8,0x9e,0x82,0x29,0x05,0xcf,0xe3,0x93,0xbf,0x79,0x55,
	0x3f,0x13,0xd5,0xf9,0x85,0xa9,0x63,0x4f,0xb8,0xa4,0xc2,0xde,0x6e,0x72,0x18,0x04,
	0x0c,0x10,0x7a,0x66,0xfc,0xe0,0x86,0x9a,0x47,0x6b,0xa1,0x8d,0xbb,0x97,0x51,0x7d,
	0x17,0x3b,0xfd,0xd1,0xeb,0xc7,0x0d,0x21,0xa0,0xbc,0xda,0xc6,0x50,0x4c,0x26,0x3a,
	0x3e,0x22,0x48,0x54,0x46,0x5a,0x3c,0x20,0x25,0x09,0xc3,0xef,0xc1,0xed,0x2b,0x07,
	0x6d,0x41,0x87,0xab,0x89,0xa5,0x6f,0x43,0x1a,0x06,0x60,0x7c,0x62,0x7e,0x14,0x08,
	0x0a,0x16,0x70,0x6c,0xdc,0xc0,0xaa,0xb6,0x4d,0x61,0xa7,0x8b,0xf7,0xdb,0x11,0x3d,
	0x5b,0x77,0xbd,0x91,0xe1,0xcd,0x0b,0x27,0x80,0x9c,0xf6,0xea,0x56,0x4a,0x2c,0x30,
	0xb0,0xac,0xca,0xd6,0xee,0xf2,0x98,0x84,0x37,0x1b,0xdd,0xf1,0x95,0xb9,0x73,0x5f,
	0x39,0x15,0xdf,0xf3,0x9b,0xb7,0x71,0x5d,0xb2,0xae,0xc4,0xd8,0xec,0xf0,0x96,0x8a,
	0xa8,0xb4,0xd2,0xce,0xd0,0xcc,0xa6,0xba,0x1f,0x33,0xf5,0xd9,0xfb,0xd7,0x1d,0x31,
	0x57,0x7b,0xb1,0x9d,0xb3,0x9f,0x59,0x75,0x8c,0x90,0xfa,0xe6,0xf4,0xe8,0x8e,0x92,
	0x12,0x0e,0x68,0x74,0xe2,0xfe,0x94,0x88,0x65,0x49,0x8f,0xa3,0x99,0xb5,0x7f,0x53,
	0x35,0x19,0xd3,0xff,0xc9,0xe5,0x23,0x0f,0xbe,0xa2,0xc8,0xd4,0x4e,0x52,0x34,0x28,
};


/* common to FD1089A and FD1089B */
static const struct parameters addr_params[16] =
{
	{ 0x23, 6,4,5,7,3,0,1,2 },
	{ 0x92, 2,5,3,6,7,1,0,4 },
	{ 0xb8, 6,7,4,2,0,5,1,3 },
	{ 0x74, 5,3,7,1,4,6,0,2 },
	{ 0xcf, 7,4,1,0,6,2,3,5 },
	{ 0xc4, 3,1,6,4,5,0,2,7 },
	{ 0x51, 5,7,2,4,3,1,6,0 },
	{ 0x14, 7,2,0,6,1,3,4,5 },
	{ 0x7f, 3,5,6,0,2,1,7,4 },
	{ 0x03, 2,3,4,0,6,7,5,1 },
	{ 0x96, 3,1,7,5,2,4,6,0 },
	{ 0x30, 7,6,2,3,0,4,5,1 },
	{ 0xe2, 1,0,3,7,4,5,2,6 },
	{ 0x72, 1,6,0,5,7,2,4,3 },
	{ 0xf5, 0,4,1,2,6,5,7,3 },
	{ 0x5b, 0,7,5,3,1,4,2,6 },
};

static UINT8 rearrange_key(UINT8 table, INT32 opcode)
{
	if (opcode == 0) {
		table ^= (1<<4);
		table ^= (1<<5);
		table ^= (1<<6);

		if (BIT(~table,3))
			table ^= (1<<1);

		if (BIT(table,6))
			table ^= (1<<7);

		table = BITSWAP08(table,1,0,6,4,3,5,2,7);

		if (BIT(table,6))
			table = BITSWAP08(table,7,6,2,4,5,3,1,0);
	} else {
		table ^= (1<<2);
		table ^= (1<<3);
		table ^= (1<<4);

		if (BIT(~table,3))
			table ^= (1<<5);

		if (BIT(~table,7))
			table ^= (1<<6);

		table = BITSWAP08(table,5,6,7,4,2,3,1,0);

		if (BIT(table,6))
			table = BITSWAP08(table,7,6,5,3,2,4,1,0);
	}

	if (BIT(table,6)) {
		if (BIT(table,5))
			table ^= (1<<4);
	} else {
		if (BIT(~table,4))
			table ^= (1<<5);
	}

	return table;
}

static INT32 decode_fd1089a(INT32 val,INT32 key,INT32 opcode)
{
	INT32 table;

	static const struct parameters data_params[16] =
	{
		{ 0x55, 6,5,1,0,7,4,2,3 },
		{ 0x94, 7,6,4,2,0,5,1,3 },
		{ 0x8d, 1,4,2,3,0,6,7,5 },
		{ 0x9a, 4,3,5,6,0,2,1,7 },
		{ 0x72, 4,3,7,0,5,6,1,2 },
		{ 0xff, 1,7,2,3,6,4,5,0 },
		{ 0x06, 6,5,3,2,4,1,0,7 },
		{ 0xc5, 3,5,1,4,2,7,0,6 },
		{ 0xec, 4,7,5,1,6,0,2,3 },
		{ 0x89, 3,5,0,6,1,2,7,4 },
		{ 0x5c, 1,3,0,7,5,2,4,6 },
		{ 0x3f, 7,3,0,2,4,6,1,5 },
		{ 0x57, 6,4,7,2,1,5,3,0 },
		{ 0xf7, 6,3,7,0,5,4,2,1 },
		{ 0x3a, 6,1,3,2,7,4,5,0 },
		{ 0xac, 1,6,3,5,0,7,4,2 },
	};
	const struct parameters *p;
	const struct parameters *q;
	INT32 family;

	/* special case - don't decrypt */
	if (key == 0x40)
		return val;

	table = rearrange_key(key, opcode);

	p = &addr_params[table >> 4];
	val = BITSWAP08(val, p->s7,p->s6,p->s5,p->s4,p->s3,p->s2,p->s1,p->s0) ^ p->xor1;

	if (BIT(table,3)) val ^= 0x01;
	if (BIT(table,0)) val ^= 0xb1;
	if (opcode) val ^= 0x34;
	if (opcode == 0)
		if (BIT(table,6))
			val ^= 0x01;

	val = basetable_fd1089[val];

	family = table & 0x07;
	if (opcode == 0)
	{
		if (BIT(~table,6) & BIT(table,2)) family ^= 8;
		if (BIT(table,4)) family ^= 8;
	}
	else
	{
		if (BIT(table,6) & BIT(table,2)) family ^= 8;
		if (BIT(table,5)) family ^= 8;
	}

	if (BIT(table,0))
	{
		if (BIT(val,0)) val ^= 0xc0;
		if (BIT(~val,6) ^ BIT(val,4))
			val = BITSWAP08(val, 7,6,5,4,1,0,2,3);
	}
	else
	{
		if (BIT(~val,6) ^ BIT(val,4))
			val = BITSWAP08(val, 7,6,5,4,0,1,3,2);
	}
	if (BIT(~val,6))
		val = BITSWAP08(val, 7,6,5,4,2,3,0,1);

	q = &data_params[family];

	val ^= q->xor1;
	val = BITSWAP08(val, q->s7,q->s6,q->s5,q->s4,q->s3,q->s2,q->s1,q->s0);

	return val;
}

static INT32 decode_fd1089b(INT32 val,INT32 key,INT32 opcode)
{
	INT32 table;
	INT32 xor1;

	const struct parameters *p;

	/* special case - don't decrypt */
	if (key == 0x40)
		return val;

	table = rearrange_key(key, opcode);

	p = &addr_params[table >> 4];
	val = BITSWAP08(val, p->s7,p->s6,p->s5,p->s4,p->s3,p->s2,p->s1,p->s0) ^ p->xor1;

	if (BIT(table,3)) val ^= 0x01;
	if (BIT(table,0)) val ^= 0xb1;
	if (opcode) val ^= 0x34;
	if (opcode == 0)
		if (BIT(table,6))
			val ^= 0x01;

	val = basetable_fd1089[val];

	xor1 = 0;
	if (opcode == 0)
	{
		if (BIT(~table,6) & BIT(table,2)) xor1 ^= 0x01;
		if (BIT(table,4)) xor1 ^= 0x01;
	}
	else
	{
		if (BIT(table,6) & BIT(table,2)) xor1 ^= 0x01;
		if (BIT(table,5)) xor1 ^= 0x01;
	}

	val ^= xor1;

	if (BIT(table,2))
	{
		val = BITSWAP08(val, 7,6,5,4,1,0,3,2);

		if (BIT(table,0) ^ BIT(table,1))
			val = BITSWAP08(val, 7,6,5,4,0,1,3,2);
	}
	else
	{
		val = BITSWAP08(val, 7,6,5,4,3,2,0,1);

		if (BIT(table,0) ^ BIT(table,1))
			val = BITSWAP08(val, 7,6,5,4,1,0,2,3);
	}

	return val;
}

static UINT16 fd1089_decrypt(/*offs_t*/INT32 addr,UINT16 val,const UINT8 *key,INT32 opcode)
{
	INT32 tbl_num,src;

	/* pick the translation table from bits ff022a of the address */
	tbl_num =	((addr & 0x000002) >> 1) |
				((addr & 0x000008) >> 2) |
				((addr & 0x000020) >> 3) |
				((addr & 0x000200) >> 6) |
				((addr & 0xff0000) >> 12);

	src =	((val & 0x0008) >> 3) |
			((val & 0x0040) >> 5) |
			((val & 0xfc00) >> 8);

	if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_FD1089A_ENC) src = decode_fd1089a(src,key[tbl_num + (1^opcode) * 0x1000],opcode);
	if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_FD1089B_ENC) src = decode_fd1089b(src,key[tbl_num + (1^opcode) * 0x1000],opcode);
	
	src =	((src & 0x01) << 3) |
			((src & 0x02) << 5) |
			((src & 0xfc) << 8);

	return (val & ~0xfc48) | src;
}

static void sys16_decrypt(const UINT8 *key)
{
	UINT16 *rom = (UINT16 *)System16Rom;
	INT32 size = 0x100000;
	INT32 A;
	UINT16 *decrypted = (UINT16 *)System16Code;

	for (A = 0;A < size;A+=2)
	{
		UINT16 src = BURN_ENDIAN_SWAP_INT16(rom[A / 2]);

		/* decode the opcodes */
		decrypted[A/2] = BURN_ENDIAN_SWAP_INT16(fd1089_decrypt(A,src,key,1));
		
		/* decode the data */
		rom[A/2] = BURN_ENDIAN_SWAP_INT16(fd1089_decrypt(A,src,key,0));
	}
}

void FD1089Decrypt()
{
	sys16_decrypt(System16Key);
}
