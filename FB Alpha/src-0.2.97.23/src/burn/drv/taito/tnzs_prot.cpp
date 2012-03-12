// The New Zealand Story protection module
// Based on MAME version 0.67 mcu simulation

#include "burnint.h"
#include "tnzs_prot.h"

#define	UNLOCK	0x0f
#define LOCK	0x0c

static INT32 mcu_type;

static INT32 mcu_initializing;
static INT32 mcu_coinage_init;
static INT32 mcu_command,mcu_readcredits;
static INT32 mcu_reportcoin;
static UINT8 mcu_coinage[4];
static UINT8 mcu_coinsA;
static UINT8 mcu_coinsB;
static UINT8 mcu_credits;
static UINT8 mcu_coin_lockout;

UINT8 *tnzs_mcu_inputs;

void tnzs_mcu_reset()
{
	mcu_initializing = 3;
	mcu_coinage_init = 0;
	mcu_coinage[0] = 1;
	mcu_coinage[1] = 1;
	mcu_coinage[2] = 1;
	mcu_coinage[3] = 1;
	mcu_coinsA = 0;
	mcu_coinsB = 0;
	mcu_credits = 0;
	mcu_reportcoin = 0;
	mcu_command = 0;
	mcu_coin_lockout = LOCK;
}

void tnzs_mcu_init(INT32 type)
{
	tnzs_mcu_reset();
	mcu_type = type;
}

static void mcu_handle_coins(INT32 coin)
{
	static INT32 insertcoin;

	if (coin & 0x08)
		mcu_reportcoin = coin;
	else if (coin && coin != insertcoin)
	{
		if (coin & 0x01)
		{
			mcu_coinsA++;
			if (mcu_coinsA >= mcu_coinage[0])
			{
				mcu_coinsA -= mcu_coinage[0];
				mcu_credits += mcu_coinage[1];
				if (mcu_credits >= 9)
				{
					mcu_credits = 9;
					mcu_coin_lockout = LOCK;
				}
				else
				{
					mcu_coin_lockout = UNLOCK;
				}
			}
		}
		if (coin & 0x02)
		{
			mcu_coinsB++;
			if (mcu_coinsB >= mcu_coinage[2])
			{
				mcu_coinsB -= mcu_coinage[2];
				mcu_credits += mcu_coinage[3];
				if (mcu_credits >= 9)
				{
					mcu_credits = 9;
					mcu_coin_lockout = LOCK;
				}
				else
				{
					mcu_coin_lockout = UNLOCK;
				}
			}
		}
		if (coin & 0x04)
		{
			mcu_credits++;
		}
		mcu_reportcoin = coin;
	}
	else
	{
		if (mcu_credits < 9)
			mcu_coin_lockout = UNLOCK;
		mcu_reportcoin = 0;
	}
	insertcoin = coin;
}

static UINT8 mcu_arknoid2_r(INT32 offset)
{
	const char *mcu_startup = "\x55\xaa\x5a";

	if (offset == 0)
	{
		if (mcu_initializing)
		{
			mcu_initializing--;
			return mcu_startup[2 - mcu_initializing];
		}

		switch (mcu_command)
		{
			case 0x41:
				return mcu_credits;

			case 0xc1:
				if (mcu_readcredits == 0)
				{
					mcu_readcredits = 1;
					if (mcu_reportcoin & 0x08)
					{
						mcu_initializing = 3;
						return 0xee;
					}
					else return mcu_credits;
				}
				else return tnzs_mcu_inputs[0];

			default:
				return 0xff;
				break;
		}
	}
	else
	{
		if (mcu_reportcoin & 0x08) return 0xe1;
		if (mcu_reportcoin & 0x01) return 0x11;
		if (mcu_reportcoin & 0x02) return 0x21;
		if (mcu_reportcoin & 0x04) return 0x31;
		return 0x01;
	}
}

static void mcu_arknoid2_w(INT32 offset, INT32 data)
{
	if (offset == 0)
	{
		if (mcu_command == 0x41)
		{
			mcu_credits = (mcu_credits + data) & 0xff;
		}
	}
	else
	{
		if (mcu_initializing)
		{
			mcu_coinage[mcu_coinage_init++] = data;
			if (mcu_coinage_init == 4) mcu_coinage_init = 0;
		}

		if (data == 0xc1)
			mcu_readcredits = 0;

		if (data == 0x15)
			mcu_credits = (mcu_credits - 1) & 0xff;

		mcu_command = data;
	}
}

static UINT8 mcu_chukatai_r(INT32 offset)
{
	const char *mcu_startup = "\xa5\x5a\xaa";

	if (offset == 0)
	{
		if (mcu_initializing)
		{
			mcu_initializing--;
			return mcu_startup[2 - mcu_initializing];
		}

		switch (mcu_command)
		{
			case 0x1f:
				return (tnzs_mcu_inputs[2] >> 4) ^ 0x0f;

			case 0x03:
				return tnzs_mcu_inputs[2] & 0x0f;

			case 0x41:
				return mcu_credits;

			case 0x93:
				if (mcu_readcredits == 0)
				{
					mcu_readcredits += 1;
					if (mcu_reportcoin & 0x08)
					{
						mcu_initializing = 3;
						return 0xee;
					}
					else return mcu_credits;
				}

				if (mcu_readcredits == 1)
				{
					mcu_readcredits += 1;
					return tnzs_mcu_inputs[0];
				}

				if (mcu_readcredits == 2)
				{
					return tnzs_mcu_inputs[1];
				}

			default:
				return 0xff;
				break;
		}
	}
	else
	{
		if (mcu_reportcoin & 0x08) return 0xe1;	
		if (mcu_reportcoin & 0x01) return 0x11;
		if (mcu_reportcoin & 0x02) return 0x21;
		if (mcu_reportcoin & 0x04) return 0x31;
		return 0x01;
	}
}

static void mcu_chukatai_w(INT32 offset, INT32 data)
{
	if (offset == 0)
	{
		if (mcu_command == 0x41)
		{
			mcu_credits = (mcu_credits + data) & 0xff;
		}
	}
	else
	{
		if (mcu_initializing)
		{
			mcu_coinage[mcu_coinage_init++] = data;
			if (mcu_coinage_init == 4) mcu_coinage_init = 0;
		}

		if (data == 0x93)
			mcu_readcredits = 0;

		mcu_command = data;
	}
}

static UINT8 mcu_tnzs_r(INT32 offset)
{
	const char *mcu_startup = "\x5a\xa5\x55";

	if (offset == 0)
	{
		if (mcu_initializing)
		{
			mcu_initializing--;
			return mcu_startup[2 - mcu_initializing];
		}

		switch (mcu_command)
		{
			case 0x01:
				return ~tnzs_mcu_inputs[0];

			case 0x02:
				return ~tnzs_mcu_inputs[1];

			case 0x1a:
				if (mcu_type == MCU_DRTOPPEL) return (tnzs_mcu_inputs[2] >> 2) ^ 0x03;
				return tnzs_mcu_inputs[2] >> 4;

			case 0x21:
				return tnzs_mcu_inputs[2] & 0x0f; 

			case 0x41:
				return mcu_credits;

			case 0xa0:
				if (mcu_reportcoin & 0x08)
				{
					mcu_initializing = 3;
					return 0xee;
				}
				else return mcu_credits;

			case 0xa1:
				if (mcu_readcredits == 0)
				{
					mcu_readcredits = 1;
					if (mcu_reportcoin & 0x08)
					{
						mcu_initializing = 3;
						return 0xee;
					}
					else return mcu_credits;
				}
				else
					return ~((tnzs_mcu_inputs[0] & 0xf0) | (tnzs_mcu_inputs[1] & 0x0f));

			default:
				return 0xff;
				break;
		}
	}
	else
	{
		if (mcu_reportcoin & 0x08) return 0xe1;

		if (mcu_type == MCU_TNZS)
		{
			if (mcu_reportcoin & 0x01) return 0x31;
			if (mcu_reportcoin & 0x02) return 0x21;
			if (mcu_reportcoin & 0x04) return 0x11;
		}
		else
		{
			if (mcu_reportcoin & 0x01) return 0x11;
			if (mcu_reportcoin & 0x02) return 0x21;
			if (mcu_reportcoin & 0x04) return 0x31;
		}
		return 0x01;
	}
}

static void mcu_tnzs_w(INT32 offset, INT32 data)
{
	if (offset == 0) {
		if (mcu_command == 0x41) {
			mcu_credits = (mcu_credits + data) & 0xff;
		}
	}
	else
	{
		if (mcu_initializing)
		{
			mcu_coinage[mcu_coinage_init++] = data;
			if (mcu_coinage_init == 4) mcu_coinage_init = 0;
		}

		if (data == 0xa1)
			mcu_readcredits = 0;

		if ((data == 0x09) && (mcu_type == MCU_DRTOPPEL || mcu_type == MCU_PLUMPOP))
			mcu_credits = (mcu_credits - 1) & 0xff;

		if ((data == 0x18) && (mcu_type == MCU_DRTOPPEL || mcu_type == MCU_PLUMPOP))
			mcu_credits = (mcu_credits - 2) & 0xff;

		mcu_command = data;
	}
}

UINT8 tnzs_mcu_read(INT32 offset)
{
	switch (mcu_type)
	{
		case MCU_ARKANOID:
			return mcu_arknoid2_r(offset & 1);
			break;

		case MCU_CHUKATAI:
			return mcu_chukatai_r(offset & 1);
			break;

		case MCU_EXTRMATN:
		case MCU_DRTOPPEL:
		case MCU_TNZS:
		case MCU_PLUMPOP:
			return mcu_tnzs_r(offset & 1);
			break;

		case MCU_NONE_INSECTX:
		case MCU_NONE_KAGEKI:
		case MCU_NONE_JPOPNICS:
		case MCU_NONE:
			return tnzs_mcu_inputs[offset & 1];
			break;
	}

	return 0xff;
}

void tnzs_mcu_write(INT32 offset, INT32 data)
{
	switch (mcu_type)
	{
		case MCU_ARKANOID:
			mcu_arknoid2_w(offset & 1, data);
			break;

		case MCU_CHUKATAI:
			mcu_chukatai_w(offset & 1, data);
			break;

		case MCU_EXTRMATN:
		case MCU_DRTOPPEL:
		case MCU_TNZS:
		case MCU_PLUMPOP:
			mcu_tnzs_w(offset & 1, data);
			break;
	}
}

void tnzs_mcu_interrupt()
{
	INT32 coin;

	switch (mcu_type)
	{
		case MCU_ARKANOID:
			coin = (~tnzs_mcu_inputs[1] >> 4) & 0x0f;
			coin = (coin & 0x08) | ((coin & 0x03) << 1) | ((coin & 0x04) >> 2);
			coin &= mcu_coin_lockout;
			mcu_handle_coins(coin);
			break;

		case MCU_EXTRMATN:
		case MCU_DRTOPPEL:
		case MCU_PLUMPOP:
		case MCU_CHUKATAI:
		case MCU_TNZS:
			coin = (((~tnzs_mcu_inputs[2] & 0x30) >> 4) | ((~tnzs_mcu_inputs[2] & 0x03) << 2));
			coin &= mcu_coin_lockout;
			mcu_handle_coins(coin);
			break;

		case MCU_NONE:
			break;
	}
}

INT32 tnzs_mcu_type()
{
	return mcu_type;
}

void tnzs_mcu_scan()
{
	SCAN_VAR(mcu_initializing);
	SCAN_VAR(mcu_coinage_init);
	SCAN_VAR(mcu_coinage[0]);
	SCAN_VAR(mcu_coinage[1]);
	SCAN_VAR(mcu_coinage[2]);
	SCAN_VAR(mcu_coinage[3]);
	SCAN_VAR(mcu_coinsA);
	SCAN_VAR(mcu_coinsB);
	SCAN_VAR(mcu_credits);
	SCAN_VAR(mcu_reportcoin);
	SCAN_VAR(mcu_command);
	SCAN_VAR(mcu_coin_lockout);
}
