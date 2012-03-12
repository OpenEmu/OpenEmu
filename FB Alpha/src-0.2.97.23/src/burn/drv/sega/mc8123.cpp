#include "burnint.h"
#include "bitswap.h"

static INT32 decrypt_type0(INT32 val,INT32 param,INT32 swap)
{
	if (swap == 0) val = BITSWAP08(val,7,5,3,1,2,0,6,4);
	if (swap == 1) val = BITSWAP08(val,5,3,7,2,1,0,4,6);
	if (swap == 2) val = BITSWAP08(val,0,3,4,6,7,1,5,2);
	if (swap == 3) val = BITSWAP08(val,0,7,3,2,6,4,1,5);

	if (BIT(param,3) && BIT(val,7))
		val ^= (1<<5)|(1<<3)|(1<<0);

	if (BIT(param,2) && BIT(val,6))
		val ^= (1<<7)|(1<<2)|(1<<1);

	if (BIT(val,6)) val ^= (1<<7);

	if (BIT(param,1) && BIT(val,7))
		val ^= (1<<6);

	if (BIT(val,2)) val ^= (1<<5)|(1<<0);

	val ^= (1<<4)|(1<<3)|(1<<1);

	if (BIT(param,2)) val ^= (1<<5)|(1<<2)|(1<<0);
	if (BIT(param,1)) val ^= (1<<7)|(1<<6);
	if (BIT(param,0)) val ^= (1<<5)|(1<<0);

	if (BIT(param,0)) val = BITSWAP08(val,7,6,5,1,4,3,2,0);

	return val;
}


static INT32 decrypt_type1a(INT32 val,INT32 param,INT32 swap)
{
	if (swap == 0) val = BITSWAP08(val,4,2,6,5,3,7,1,0);
	if (swap == 1) val = BITSWAP08(val,6,0,5,4,3,2,1,7);
	if (swap == 2) val = BITSWAP08(val,2,3,6,1,4,0,7,5);
	if (swap == 3) val = BITSWAP08(val,6,5,1,3,2,7,0,4);

	if (BIT(param,2)) val = BITSWAP08(val,7,6,1,5,3,2,4,0);

	if (BIT(val,1)) val ^= (1<<0);
	if (BIT(val,6)) val ^= (1<<3);
	if (BIT(val,7)) val ^= (1<<6)|(1<<3);
	if (BIT(val,2)) val ^= (1<<6)|(1<<3)|(1<<1);
	if (BIT(val,4)) val ^= (1<<7)|(1<<6)|(1<<2);

	if (BIT(val,7) ^ BIT(val,2))
		val ^= (1<<4);

	val ^= (1<<6)|(1<<3)|(1<<1)|(1<<0);

	if (BIT(param,3)) val ^= (1<<7)|(1<<2);
	if (BIT(param,1)) val ^= (1<<6)|(1<<3);

	if (BIT(param,0)) val = BITSWAP08(val,7,6,1,4,3,2,5,0);

	return val;
}

static INT32 decrypt_type1b(INT32 val,INT32 param,INT32 swap)
{
	if (swap == 0) val = BITSWAP08(val,1,0,3,2,5,6,4,7);
	if (swap == 1) val = BITSWAP08(val,2,0,5,1,7,4,6,3);
	if (swap == 2) val = BITSWAP08(val,6,4,7,2,0,5,1,3);
	if (swap == 3) val = BITSWAP08(val,7,1,3,6,0,2,5,4);

	if (BIT(val,2) && BIT(val,0))
		val ^= (1<<7)|(1<<4);

	if (BIT(val,7)) val ^= (1<<2);
	if (BIT(val,5)) val ^= (1<<7)|(1<<2);
	if (BIT(val,1)) val ^= (1<<5);
	if (BIT(val,6)) val ^= (1<<1);
	if (BIT(val,4)) val ^= (1<<6)|(1<<5);
	if (BIT(val,0)) val ^= (1<<6)|(1<<2)|(1<<1);
	if (BIT(val,3)) val ^= (1<<7)|(1<<6)|(1<<2)|(1<<1)|(1<<0);

	val ^= (1<<6)|(1<<4)|(1<<0);

	if (BIT(param,3)) val ^= (1<<4)|(1<<1);
	if (BIT(param,2)) val ^= (1<<7)|(1<<6)|(1<<3)|(1<<0);
	if (BIT(param,1)) val ^= (1<<4)|(1<<3);
	if (BIT(param,0)) val ^= (1<<6)|(1<<2)|(1<<1)|(1<<0);

	return val;
}

static INT32 decrypt_type2a(INT32 val,INT32 param,INT32 swap)
{
	if (swap == 0) val = BITSWAP08(val,0,1,4,3,5,6,2,7);
	if (swap == 1) val = BITSWAP08(val,6,3,0,5,7,4,1,2);
	if (swap == 2) val = BITSWAP08(val,1,6,4,5,0,3,7,2);
	if (swap == 3) val = BITSWAP08(val,4,6,7,5,2,3,1,0);

	if (BIT(val,3) || (BIT(param,1) && BIT(val,2)))
		val = BITSWAP08(val,6,0,7,4,3,2,1,5);

	if (BIT(val,5)) val ^= (1<<7);
	if (BIT(val,6)) val ^= (1<<5);
	if (BIT(val,0)) val ^= (1<<6);
	if (BIT(val,4)) val ^= (1<<3)|(1<<0);
	if (BIT(val,1)) val ^= (1<<2);

	val ^= (1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<1);

	if (BIT(param,2)) val ^= (1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0);

	if (BIT(param,3))
	{
		if (BIT(param,0))
			val = BITSWAP08(val,7,6,5,3,4,1,2,0);
		else
			val = BITSWAP08(val,7,6,5,1,2,4,3,0);
	}
	else
	{
		if (BIT(param,0))
			val = BITSWAP08(val,7,6,5,2,1,3,4,0);
	}

	return val;
}

static INT32 decrypt_type2b(INT32 val,INT32 param,INT32 swap)
{
	// only 0x20 possible encryptions for this method - all others have 0x40
	// this happens because BIT(param,2) cancels the other three

	if (swap == 0) val = BITSWAP08(val,1,3,4,6,5,7,0,2);
	if (swap == 1) val = BITSWAP08(val,0,1,5,4,7,3,2,6);
	if (swap == 2) val = BITSWAP08(val,3,5,4,1,6,2,0,7);
	if (swap == 3) val = BITSWAP08(val,5,2,3,0,4,7,6,1);

	if (BIT(val,7) && BIT(val,3))
		val ^= (1<<6)|(1<<4)|(1<<0);

	if (BIT(val,7)) val ^= (1<<2);
	if (BIT(val,5)) val ^= (1<<7)|(1<<3);
	if (BIT(val,1)) val ^= (1<<5);
	if (BIT(val,4)) val ^= (1<<7)|(1<<5)|(1<<3)|(1<<1);

	if (BIT(val,7) && BIT(val,5))
		val ^= (1<<4)|(1<<0);

	if (BIT(val,5) && BIT(val,1))
		val ^= (1<<4)|(1<<0);

	if (BIT(val,6)) val ^= (1<<7)|(1<<5);
	if (BIT(val,3)) val ^= (1<<7)|(1<<6)|(1<<5)|(1<<1);
	if (BIT(val,2)) val ^= (1<<3)|(1<<1);

	val ^= (1<<7)|(1<<3)|(1<<2)|(1<<1);

	if (BIT(param,3)) val ^= (1<<6)|(1<<3)|(1<<1);
	if (BIT(param,2)) val ^= (1<<7)|(1<<6)|(1<<5)|(1<<3)|(1<<2)|(1<<1);	// same as the other three combined
	if (BIT(param,1)) val ^= (1<<7);
	if (BIT(param,0)) val ^= (1<<5)|(1<<2);

	return val;
}

static INT32 decrypt_type3a(INT32 val,INT32 param,INT32 swap)
{
	if (swap == 0) val = BITSWAP08(val,5,3,1,7,0,2,6,4);
	if (swap == 1) val = BITSWAP08(val,3,1,2,5,4,7,0,6);
	if (swap == 2) val = BITSWAP08(val,5,6,1,2,7,0,4,3);
	if (swap == 3) val = BITSWAP08(val,5,6,7,0,4,2,1,3);

	if (BIT(val,2)) val ^= (1<<7)|(1<<5)|(1<<4);
	if (BIT(val,3)) val ^= (1<<0);

	if (BIT(param,0)) val = BITSWAP08(val,7,2,5,4,3,1,0,6);

	if (BIT(val,1)) val ^= (1<<6)|(1<<0);
	if (BIT(val,3)) val ^= (1<<4)|(1<<2)|(1<<1);

	if (BIT(param,3)) val ^= (1<<4)|(1<<3);

	if (BIT(val,3)) val = BITSWAP08(val,5,6,7,4,3,2,1,0);

	if (BIT(val,5)) val ^= (1<<2)|(1<<1);

	val ^= (1<<6)|(1<<5)|(1<<4)|(1<<3);

	if (BIT(param,2)) val ^= (1<<7);
	if (BIT(param,1)) val ^= (1<<4);
	if (BIT(param,0)) val ^= (1<<0);

	return val;
}

static INT32 decrypt_type3b(INT32 val,INT32 param,INT32 swap)
{
	if (swap == 0) val = BITSWAP08(val,3,7,5,4,0,6,2,1);
	if (swap == 1) val = BITSWAP08(val,7,5,4,6,1,2,0,3);
	if (swap == 2) val = BITSWAP08(val,7,4,3,0,5,1,6,2);
	if (swap == 3) val = BITSWAP08(val,2,6,4,1,3,7,0,5);

	if (BIT(val,2)) val ^= (1<<7);

	if (BIT(val,7)) val = BITSWAP08(val,7,6,3,4,5,2,1,0);

	if (BIT(param,3)) val ^= (1<<7);

	if (BIT(val,4)) val ^= (1<<6);
	if (BIT(val,1)) val ^= (1<<6)|(1<<4)|(1<<2);

	if (BIT(val,7) && BIT(val,6))
		val ^= (1<<1);

	if (BIT(val,7)) val ^= (1<<1);

	if (BIT(param,3)) val ^= (1<<7);
	if (BIT(param,2)) val ^= (1<<0);

	if (BIT(param,3)) val = BITSWAP08(val,4,6,3,2,5,0,1,7);

	if (BIT(val,4)) val ^= (1<<1);
	if (BIT(val,5)) val ^= (1<<4);
	if (BIT(val,7)) val ^= (1<<2);

	val ^= (1<<5)|(1<<3)|(1<<2);

	if (BIT(param,1)) val ^= (1<<7);
	if (BIT(param,0)) val ^= (1<<3);

	return val;
}

static INT32 decrypt(INT32 val, INT32 key, INT32 opcode)
{
	INT32 type = 0;
	INT32 swap = 0;
	INT32 param = 0;

	key ^= 0xff;

	// no encryption
	if (key == 0x00)
		return val;

	type ^= BIT(key,0) << 0;
	type ^= BIT(key,2) << 0;
	type ^= BIT(key,0) << 1;
	type ^= BIT(key,1) << 1;
	type ^= BIT(key,2) << 1;
	type ^= BIT(key,4) << 1;
	type ^= BIT(key,4) << 2;
	type ^= BIT(key,5) << 2;

	swap ^= BIT(key,0) << 0;
	swap ^= BIT(key,1) << 0;
	swap ^= BIT(key,2) << 1;
	swap ^= BIT(key,3) << 1;

	param ^= BIT(key,0) << 0;
	param ^= BIT(key,0) << 1;
	param ^= BIT(key,2) << 1;
	param ^= BIT(key,3) << 1;
	param ^= BIT(key,0) << 2;
	param ^= BIT(key,1) << 2;
	param ^= BIT(key,6) << 2;
	param ^= BIT(key,1) << 3;
	param ^= BIT(key,6) << 3;
	param ^= BIT(key,7) << 3;

	if (!opcode)
	{
		param ^= 1 << 0;
		type ^= 1 << 0;
	}

	switch (type)
	{
		default:
		case 0: return decrypt_type0(val,param,swap);
		case 1: return decrypt_type0(val,param,swap);
		case 2: return decrypt_type1a(val,param,swap);
		case 3: return decrypt_type1b(val,param,swap);
		case 4: return decrypt_type2a(val,param,swap);
		case 5: return decrypt_type2b(val,param,swap);
		case 6: return decrypt_type3a(val,param,swap);
		case 7: return decrypt_type3b(val,param,swap);
	}
}

static UINT8 mc8123_decrypt(INT32 addr,UINT8 val,const UINT8 *key,INT32 opcode)
{
	INT32 tbl_num;

	/* pick the translation table from bits fd57 of the address */
	tbl_num = (addr & 7) + ((addr & 0x10)>>1) + ((addr & 0x40)>>2) + ((addr & 0x100)>>3) + ((addr & 0xc00)>>4) + ((addr & 0xf000)>>4) ;

	return decrypt(val,key[tbl_num + (opcode ? 0 : 0x1000)],opcode);
}

void mc8123_decrypt_rom(INT32 /*banknum*/, INT32 numbanks, UINT8 *pRom, UINT8 *pFetch, UINT8 *pKey)
{
	UINT8 *decrypted1 = pFetch;
//	UINT8 *decrypted2 = numbanks > 1 ? auto_malloc(0x4000 * numbanks) : decrypted1 + 0x8000;
	UINT8 *rom = pRom;
	UINT8 *key = pKey;
	INT32 A, bank;

	for (A = 0x0000;A < 0x8000;A++)
	{
		UINT8 src = rom[A];

		/* decode the opcodes */
		decrypted1[A] = mc8123_decrypt(A,src,key,1);

		/* decode the data */
		rom[A] = mc8123_decrypt(A,src,key,0);
	}

	for (bank = 0; bank < numbanks; ++bank)
	{
		for (A = 0x8000;A < 0xc000;A++)
		{
			UINT8 src = rom[0x8000 + 0x4000*bank + A];

			/* decode the opcodes */
//			decrypted2[0x4000 * bank + (A-0x8000)] = mc8123_decrypt(A,src,key,1);

			/* decode the data */
			rom[0x8000 + 0x4000*bank + A] = mc8123_decrypt(A,src,key,0);
		}
	}
}
