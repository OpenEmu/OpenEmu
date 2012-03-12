
#include "burnint.h"
#include "nmk004.h"
#include "burn_ym2203.h"
#include "msm6295.h"

UINT8 *NMK004OKIROM0;
UINT8 *NMK004OKIROM1;
UINT8 *NMK004PROGROM;

#define	okim6295_w(x,y,z)		MSM6295Command(x, z)
#define okim6295_r(x,y)			MSM6295ReadStatus(x)

#define ym2203_control_port_w(x, y, z)	BurnYM2203Write(x, 0, z)
#define ym2203_write_port_w(x, y, z)	BurnYM2203Write(x, 1, z)
#define ym2203_status_port_r(x,y)	BurnYM2203Read(x, 0);
#define ym2203_read_port_r(x,y)		BurnYM2203Read(x, 1)
#define device				0
#define	fatalerror			printf
//#define logerror			printf

//--------------------------------------------------------------------------

#define FM_CHANNELS			6
#define PSG_CHANNELS		3
#define EFFECTS_CHANNELS	8

#define FM_FLAG_NEED_INITIALIZATION			(1<<0)
#define FM_FLAG_UNKNOWN2					(1<<1)
#define FM_FLAG_NOTE_IS_PAUSE				(1<<2)
#define FM_FLAG_UNKNOWN3					(1<<3)
#define FM_FLAG_MODULATE_NOTE				(1<<4)
#define FM_FLAG_MUST_SEND_KEYON				(1<<5)
#define FM_FLAG_MUST_SEND_CONFIGURATION		(1<<6)
#define FM_FLAG_ACTIVE						(1<<7)

#define PSG_FLAG_ACTIVE						(1<<0)
#define PSG_FLAG_NOTE_IS_PAUSE				(1<<1)
#define PSG_FLAG_NEED_INITIALIZATION		(1<<2)
#define PSG_FLAG_INITIALIZE_VOLUME			(1<<3)
#define PSG_FLAG_NOTE_IS_NOISE				(1<<5)
#define PSG_FLAG_NOISE_NOT_ENABLED			(1<<6)

#define EFFECTS_FLAG_NEED_INITIALIZATION	(1<<0)
#define EFFECTS_FLAG_ACTIVE					(1<<7)

#define NOTE_PAUSE	0x0c

struct psg_control
{
/* C220      */	UINT8  flags;
/* C221-C222 */	UINT16 note_timer;
/* C223-C224 */	UINT16 note_length;
/* C225      */	UINT8  volume_timer;
/* C227-C228 */	UINT16 current;		// current position in control table
/* C229-C22A */	UINT16 return_address[16];	// return address when control table calls a subtable
				INT32 return_address_depth;
/* C22B-C22C */	UINT16 loop_start;	// first instruction of loop
/* C22D      */	UINT8  loop_times;	// number of times to loop
/* C22E      */	UINT8  volume_shape;
/* C22F      */	UINT8  volume_position;
/* C230      */	UINT8  octave;	// base octave
/* C231      */	UINT8  note;	// note to play
/* C233      */	UINT8  note_period_hi_bits;
};

struct fm_control
{
UINT8 note;
/* C020      */	UINT8  flags;
/* C021      */	UINT8  slot;	// for ym2203 keyon command
/* C022-C039 */	UINT8  voice_params[0x18];	// parameters for the YM2203 to configure sound shape
/* C03A-C03B */	UINT16 f_number;
/* C03C      */	UINT8  self_feedback;
/* C03D      */	UINT8  note_duration_table_select;
/* C03E-C03F */	UINT16 current;	// current position in control table
/* C040-C041 */	UINT16 loop_start;	// first instruction of loop
/* C042      */	UINT8  loop_times;	// number of times to loop
/* C043-C044 */	UINT16 return_address[16];	// return address when control table calls a subtable
				INT32    return_address_depth;
/* C045      */	UINT8  octave;
/* C046-C047 */	UINT16 timer1;
/* C048-C049 */	UINT16 timer2;
/* C04A-C04B */	UINT16 timer1_duration;
/* C04C-C04D */	UINT16 timer2_duration;
/* C04E      */	UINT8  modulation_table_number;
/* C04F-C050 */	UINT16 modulation_timer;
/* C051-C052 */	UINT16 modulation_table;
/* C053-C054 */	UINT16 modulation_table_position;
/* C055-C056 */	UINT16 note_period;
/* C057-C05A */	UINT8  voice_volume[4];	// parameters for the YM2203 to configure sound shape
/* C05C      */ UINT8  must_update_voice_params;
};

struct effects_control
{
/* C1A0      */	UINT8  flags;
/* C1BE-C1BF */	UINT16 current;	// current position in control table
/* C1C0-C1C1 */	UINT16 loop_start;	// first instruction of loop
/* C1C2      */	UINT8  loop_times;	// number of times to loop
/* C1C3-C1C4 */	UINT16 return_address[16];	// return address when control table calls a subtable
				INT32    return_address_depth;
/* C1C6-C1C7 */	UINT16 timer;
/* C1CA-C1CB */	UINT16 timer_duration;
};

static struct
{
	const UINT8 *rom;	// NMK004 data ROM
	UINT8 from_main;	// command from main CPU
	UINT8 to_main;		// answer to main CPU
	INT32 protection_check;

	INT32 ymdevice;
	INT32 oki1device;
	INT32 oki2device;

	/* C001      */	UINT8 last_command;		// last command received
	/* C016      */	UINT8 oki_playing;		// bitmap of active Oki channels
	/* C020-C19F */	struct fm_control fm_control[FM_CHANNELS];
	/* C220-C2DF */	struct psg_control psg_control[PSG_CHANNELS];
	/* C1A0-C21F */	struct effects_control effects_control[EFFECTS_CHANNELS];
} NMK004_state;


#define SAMPLE_TABLE_0		0xefe0
#define SAMPLE_TABLE_1		0xefe2
#define FM_MODULATION_TABLE	0xefe4
#define FM_NOTE_TABLE		0xefe6
#define NOTE_LENGTH_TABLE_1	0xefe8
#define NOTE_LENGTH_TABLE_2	0xefea
#define NOTE_LENGTH_TABLE_3	0xefec
#define PSG_VOLUME_TABLE	0xefee
#define COMMAND_TABLE		0xeff0
#define PSG_NOTE_TABLE		0xeff2

static UINT8 read8(INT32 address)
{
	return NMK004_state.rom[address];
}

static UINT16 read16(INT32 address)
{
	return NMK004_state.rom[address] + 256 * NMK004_state.rom[address+1];
}

/*****************************

          OKI6295

*****************************/

static void oki_play_sample(INT32 sample_no)
{
	UINT16 table_start = (sample_no & 0x80) ? read16(SAMPLE_TABLE_1) : read16(SAMPLE_TABLE_0);
	UINT8 byte1 = read8(table_start + 2 * (sample_no & 0x7f) + 0);
	UINT8 byte2 = read8(table_start + 2 * (sample_no & 0x7f) + 1);
	INT32 chip = (byte1 & 0x80) >> 7;
	const INT32 okidevice = (chip) ? NMK004_state.oki2device : NMK004_state.oki1device;

	if ((byte1 & 0x7f) == 0)
	{
		// stop all channels
		okim6295_w(okidevice, 0, 0x78 );
	}
	else
	{
		INT32 sample = byte1 & 0x7f;
		INT32 ch = byte2 & 0x03;
		INT32 force = (byte2 & 0x80) >> 7;

		if (!force && (NMK004_state.oki_playing & (1 << (ch + 4*chip))))
			return;

		NMK004_state.oki_playing |= 1 << (ch + 4*chip);

		// stop channel
		okim6295_w(okidevice, 0, (0x08 << ch) );

		if (sample != 0)
		{
			UINT8 *rom = (chip == 0) ? NMK004OKIROM0 : NMK004OKIROM1;
			INT32 bank = (byte2 & 0x0c) >> 2;
			INT32 vol = (byte2 & 0x70) >> 4;

			if (bank != 3)
				memcpy(rom + 0x20000,rom + 0x40000 + bank * 0x20000,0x20000);

			okim6295_w(okidevice, 0, 0x80 | sample );
			okim6295_w(okidevice, 0, (0x10 << ch) | vol );
		}
	}
}

static void oki_update_state(void)
{
	NMK004_state.oki_playing = ((okim6295_r(NMK004_state.oki2device, 0) & 0x0f) << 4) | (okim6295_r(NMK004_state.oki1device, 0) & 0x0f);
}

/*****************************

     EFFECTS (OKI6295)

*****************************/

static void effects_update(INT32 channel)
{
	struct effects_control *effects = &NMK004_state.effects_control[channel];

	// advance the timers
	if (effects->timer)
		effects->timer--;


	if (effects->flags & EFFECTS_FLAG_NEED_INITIALIZATION)
	{
		effects->flags = EFFECTS_FLAG_ACTIVE;
		effects->timer = 0;
	}


	if (effects->flags & EFFECTS_FLAG_ACTIVE)
	{
		if (effects->timer == 0)
		{
			UINT8 token;

			do
			{
				token = read8(effects->current++);

				if (token == 0x0ef || (token & 0xf0) == 0xf0)
				{
					switch (token)
					{
						case 0xef:	// play sample
							oki_play_sample(read8(effects->current++));
							break;

						case 0xf6:	// jump
							effects->current = read16(effects->current);
							break;

						case 0xf7:	// begin repeat loop
							effects->loop_times = read8(effects->current++);
							effects->loop_start = effects->current;
							break;

						case 0xf8:	// end repeat loop
							if (--effects->loop_times > 0)
								effects->current = effects->loop_start;
							break;

						case 0xf9:	// call subtable
							effects->return_address[effects->return_address_depth++] = effects->current + 2;
							effects->current = read16(effects->current);
							break;

						case 0xfa:	// return from subtable
							effects->current = effects->return_address[--effects->return_address_depth];
							break;

						case 0xfc:	// ??? (hachamf command 04)
							break;

						case 0xfd:	// ??? (hachamf command 04)
							break;

						case 0xff:	// end
							effects->flags = 0;	// disable channel
							return;

						default:
							fatalerror("effects channel %d unsupported token %02x",channel,token);
					}
				}
			} while (token == 0xef || (token & 0xf0) == 0xf0);

			effects->current--;

			if ((read8(effects->current) & 0x80) == 0)
			{
				if (read8(effects->current++) != 0x0c)
				{
					// this shouldn't happen on the effects channels (but it happens e.g. hachamf command 04)
				}
			}

			// optional note length (otherwise use the same length as the previous one)
			if (read8(effects->current) & 0x80)
			{
				UINT16 table_start = read16(NOTE_LENGTH_TABLE_1);

				effects->timer_duration = read16(table_start + 2 * (read8(effects->current++) & 0x7f));
			}

			effects->timer = effects->timer_duration;
		}
	}
}

/*****************************

        YM2203 - FM

*****************************/

static void fm_update(INT32 channel)
{
	struct fm_control *fm = &NMK004_state.fm_control[channel];

	// advance the timers
	if (fm->timer1)
		fm->timer1--;

	if (fm->timer2)
		fm->timer2--;

	if (fm->modulation_timer)
		fm->modulation_timer--;


	if (fm->flags & FM_FLAG_NEED_INITIALIZATION)
	{
		fm->flags = FM_FLAG_ACTIVE;
		fm->timer1 = 0;
	}


	if (fm->flags & FM_FLAG_ACTIVE)
	{
		if (fm->timer1 == 0)
		{
			UINT8 token;

			do
			{
				token = read8(fm->current++);

				if (token == 0x0ef || (token & 0xf0) == 0xf0)
				{
					INT32 i;

					switch (token)
					{
						case 0xf0:	// slot (for keyon ym2203 command)
							fm->flags |= FM_FLAG_MUST_SEND_CONFIGURATION;
							fm->slot = read8(fm->current++);
							if (channel < 3 || !(NMK004_state.fm_control[channel-3].flags & FM_FLAG_ACTIVE))
							{
								ym2203_control_port_w(NMK004_state.ymdevice, 0, 0x28);	// keyon/off
								ym2203_write_port_w(NMK004_state.ymdevice, 0, channel % 3);
							}
							break;

						case 0xf1:	// sound shape
							fm->flags |= FM_FLAG_MUST_SEND_CONFIGURATION;
							for (i = 0x00; i < 0x04; i++)
								fm->voice_params[i] = read8(fm->current++);
							break;

						case 0xf2:	// sound shape
							fm->flags |= FM_FLAG_MUST_SEND_CONFIGURATION;
							for (i = 0; i < 4; i++)
								fm->voice_volume[i] = read8(fm->current++);
							break;

						case 0xf3:	// sound shape
							fm->flags |= FM_FLAG_MUST_SEND_CONFIGURATION;
							for (i = 0x08; i < 0x18; i++)
								fm->voice_params[i] = read8(fm->current++);
							break;

						case 0xf4:	// set self-feedback
							fm->flags |= FM_FLAG_MUST_SEND_CONFIGURATION;
							fm->self_feedback = read8(fm->current++);
							break;

						case 0xf5:	// select note duration table
							fm->note_duration_table_select = read8(fm->current++);
							break;

						case 0xf6:	// jump
							fm->current = read16(fm->current);
							break;

						case 0xf7:	// begin repeat loop
							fm->loop_times = read8(fm->current++);
							fm->loop_start = fm->current;
							break;

						case 0xf8:	// end repeat loop
							if (--fm->loop_times > 0)
								fm->current = fm->loop_start;
							break;

						case 0xf9:	// call subtable
							fm->return_address[fm->return_address_depth++] = fm->current + 2;
							fm->current = read16(fm->current);
							break;

						case 0xfa:	// return from subtable
							fm->current = fm->return_address[--fm->return_address_depth];
							break;

						case 0xfb:	// set octave
							fm->octave = read8(fm->current++);
							break;

						case 0xfc:	// ???
							fm->flags |=  FM_FLAG_UNKNOWN2;
							fm->flags |=  FM_FLAG_UNKNOWN3;
							break;

						case 0xfd:	// ???
							fm->flags &= ~FM_FLAG_UNKNOWN2;
							break;

						case 0xfe:	// set note modulation
							fm->modulation_table_number = read8(fm->current++);
							if (fm->modulation_table_number == 0)
							{
								fm->flags &= ~FM_FLAG_MODULATE_NOTE;
							}
							else
							{
								UINT16 table_start = read16(FM_MODULATION_TABLE);

								fm->modulation_table = read16(table_start + 2 * (fm->modulation_table_number - 1));
								fm->modulation_timer = read16(fm->modulation_table);
								fm->modulation_table_position = fm->modulation_table + 2;
								fm->flags |= FM_FLAG_MODULATE_NOTE;
							}
							break;

						case 0xff:	// end
							fm->flags = FM_FLAG_MUST_SEND_CONFIGURATION;	// disable channel
							for (i = 0x04; i < 0x08; i++)
								fm->voice_params[i] = 0x7f;
							for (i = 0x14; i < 0x18; i++)
								fm->voice_params[i] = 0x0f;
							return;

						default:
							fatalerror("fm channel %d unsupported token %02x",channel,token);
					}
				}
			} while (token == 0xef || (token & 0xf0) == 0xf0);

			fm->current--;

			if ((read8(fm->current) & 0x80) == 0)
			{
				INT32 note = read8(fm->current++);

				fm->note = note;
				if ((note & 0x0f) == NOTE_PAUSE)
					fm->flags |=  FM_FLAG_NOTE_IS_PAUSE;
				else
				{
					UINT16 table_start = read16(FM_NOTE_TABLE);
					UINT16 period = read16(table_start + 2 * (note & 0x0f));
					UINT8 octave = ((fm->octave << 4) + note) & 0xf0;

					fm->flags &= ~FM_FLAG_NOTE_IS_PAUSE;
					fm->note_period = period | (octave << 7);

					fm->must_update_voice_params = 1;
				}
			}

			// optional note length (otherwise use the same length as the previous one)
			if (read8(fm->current) & 0x80)
			{
				UINT16 table_start;
				UINT8 duration = read8(fm->current++) & 0x7f;

				table_start = read16(NOTE_LENGTH_TABLE_1);
				fm->timer1_duration = read16(table_start + 2 * duration);

				table_start = fm->note_duration_table_select ? read16(NOTE_LENGTH_TABLE_3) : read16(NOTE_LENGTH_TABLE_2);
				fm->timer2_duration = read16(table_start + 2 * duration);
			}

			fm->timer1 = fm->timer1_duration;
			fm->timer2 = fm->timer2_duration;


			if (!(fm->flags & FM_FLAG_NOTE_IS_PAUSE) &&
				((fm->flags & FM_FLAG_UNKNOWN3) || !(fm->flags & FM_FLAG_UNKNOWN2)))
			{
				fm->flags &= ~FM_FLAG_UNKNOWN3;
				fm->flags |=  FM_FLAG_MUST_SEND_KEYON;
				fm->flags |=  FM_FLAG_MUST_SEND_CONFIGURATION;
			}
		}
	}

	if ((fm->flags & FM_FLAG_MODULATE_NOTE) && (fm->flags & FM_FLAG_MUST_SEND_KEYON))
	{
		fm->modulation_timer = read16(fm->modulation_table);
		fm->modulation_table_position = fm->modulation_table + 2;
	}

	if (!(fm->flags & FM_FLAG_MODULATE_NOTE) || (fm->flags & FM_FLAG_MUST_SEND_KEYON) || fm->must_update_voice_params)
	{
		INT32 i;

		fm->must_update_voice_params = 0;

		for (i = 0; i < 4; i++)
			fm->voice_params[0x04 + i] = fm->voice_volume[i];

		fm->f_number = fm->note_period;
	}
	else
	{
		if (fm->modulation_timer == 0)
		{
			INT32 i;
			UINT16 a;

			for (i = 0; i < 4; i++)
				fm->voice_params[0x04 + i] = fm->voice_volume[i];

			fm->modulation_table_position++;
			a = read8(fm->modulation_table_position++);
			if (a & 0x80)	// sign extend
				a |= 0xff00;
			a *= 4;

			fm->f_number = fm->note_period + a;

			fm->modulation_timer = read8(fm->modulation_table_position++);	// modulation_timer is UINT16 but this is just 8-bit

			if (read8(fm->modulation_table_position) == 0x80)	// end of table - repeat
			{
				fm->modulation_table_position = fm->modulation_table + 2;
			}
			else if (read8(fm->modulation_table_position) == 0x88)	// end of table - stop
			{
				fm->flags &= ~FM_FLAG_MODULATE_NOTE;
			}
		}
	}
}

static void fm_voices_update(void)
{
	static const INT32 ym2203_registers[0x18] =
	{
		0x30,0x38,0x34,0x3C,0x40,0x48,0x44,0x4C,0x50,0x58,0x54,0x5C,0x60,0x68,0x64,0x6C,
		0x70,0x78,0x74,0x7C,0x80,0x88,0x84,0x8C
	};
	INT32 channel,i;

	for (channel = 0; channel < 3;channel++)
	{
		struct fm_control *fm1 = &NMK004_state.fm_control[channel];
		struct fm_control *fm2 = &NMK004_state.fm_control[channel + 3];

		if (fm1->flags &  FM_FLAG_MUST_SEND_CONFIGURATION)
		{
			fm1->flags &= ~FM_FLAG_MUST_SEND_CONFIGURATION;

			for (i = 0; i < 0x18; i++)
			{
				ym2203_control_port_w(NMK004_state.ymdevice, 0, ym2203_registers[i] + channel);
				ym2203_write_port_w(NMK004_state.ymdevice, 0, fm1->voice_params[i]);
			}
		}

		if (fm2->flags &  FM_FLAG_MUST_SEND_CONFIGURATION)
		{
			fm2->flags &= ~FM_FLAG_MUST_SEND_CONFIGURATION;

			if (!(fm1->flags & FM_FLAG_ACTIVE))
			{
				for (i = 0; i < 0x18; i++)
				{
					ym2203_control_port_w(NMK004_state.ymdevice, 0, ym2203_registers[i] + channel);
					ym2203_write_port_w(NMK004_state.ymdevice, 0, fm2->voice_params[i]);
				}
			}
		}

		if (fm1->flags & FM_FLAG_ACTIVE)
		{
			ym2203_control_port_w(NMK004_state.ymdevice, 0, 0xb0 + channel);	// self-feedback
			ym2203_write_port_w(NMK004_state.ymdevice, 0, fm1->self_feedback);

			ym2203_control_port_w(NMK004_state.ymdevice, 0, 0xa4 + channel);	// F-number
			ym2203_write_port_w(NMK004_state.ymdevice, 0, fm1->f_number >> 8);

			ym2203_control_port_w(NMK004_state.ymdevice, 0, 0xa0 + channel);	// F-number
			ym2203_write_port_w(NMK004_state.ymdevice, 0, fm1->f_number & 0xff);
		}
		else
		{
			ym2203_control_port_w(NMK004_state.ymdevice, 0, 0xb0 + channel);	// self-feedback
			ym2203_write_port_w(NMK004_state.ymdevice, 0, fm2->self_feedback);

			ym2203_control_port_w(NMK004_state.ymdevice, 0, 0xa4 + channel);	// F-number
			ym2203_write_port_w(NMK004_state.ymdevice, 0, fm2->f_number >> 8);

			ym2203_control_port_w(NMK004_state.ymdevice, 0, 0xa0 + channel);	// F-number
			ym2203_write_port_w(NMK004_state.ymdevice, 0, fm2->f_number & 0xff);
		}

		if (fm1->flags & FM_FLAG_MUST_SEND_KEYON)
		{
			fm1->flags &= ~FM_FLAG_MUST_SEND_KEYON;

			ym2203_control_port_w(NMK004_state.ymdevice, 0, 0x28);	// keyon/off
			ym2203_write_port_w(NMK004_state.ymdevice, 0, fm1->slot | channel);
		}

		if (fm2->flags & FM_FLAG_MUST_SEND_KEYON)
		{
			fm2->flags &= ~FM_FLAG_MUST_SEND_KEYON;

			if (!(fm1->flags & FM_FLAG_ACTIVE))
			{
				ym2203_control_port_w(NMK004_state.ymdevice, 0, 0x28);	// keyon/off
				ym2203_write_port_w(NMK004_state.ymdevice, 0, fm2->slot | channel);
			}
		}
	}
}

/*****************************

        YM2203 - PSG

*****************************/

static void psg_update(INT32 channel)
{
	struct psg_control *psg = &NMK004_state.psg_control[channel];

	// advance the timers
	if (psg->note_timer)
		psg->note_timer--;

	if (psg->volume_timer)
		psg->volume_timer--;

	if (psg->flags & PSG_FLAG_NEED_INITIALIZATION)
	{
		psg->flags &= ~PSG_FLAG_NEED_INITIALIZATION;
		psg->flags |=  PSG_FLAG_ACTIVE;

		if (psg->flags & PSG_FLAG_NOTE_IS_NOISE)
		{
			INT32 enable;

			psg->flags &= ~PSG_FLAG_NOTE_IS_NOISE;
			psg->flags &= ~PSG_FLAG_NOISE_NOT_ENABLED;

			// enable noise, disable tone on this channel
			ym2203_control_port_w(NMK004_state.ymdevice, 0, 0x07);
			enable = ym2203_read_port_r(NMK004_state.ymdevice, 0);
			enable |=  (0x01 << channel);	// disable tone
			enable &= ~(0x08 << channel);	// enable noise
			ym2203_write_port_w(NMK004_state.ymdevice, 0, enable);
		}


		psg->note_timer = 0;
		psg->volume_timer = 0;
		psg->octave = 0;
	}

	if (psg->flags & PSG_FLAG_ACTIVE)
	{
		if (psg->note_timer == 0)
		{
			UINT8 token;

			do
			{
				token = read8(psg->current++);

				if ((token & 0xf0) == 0xf0)
				{
					INT32 enable;

					switch (token)
					{
						case 0xf0:	// noise
							psg->flags |= PSG_FLAG_NOTE_IS_NOISE;
							break;

						case 0xf1:	// note
							psg->flags &= ~PSG_FLAG_NOTE_IS_NOISE;
							psg->flags &= ~PSG_FLAG_NOISE_NOT_ENABLED;

							// enable noise, disable tone on this channel
							ym2203_control_port_w(NMK004_state.ymdevice, 0, 0x07);
							enable = ym2203_read_port_r(NMK004_state.ymdevice, 0);
							enable |=  (0x01 << channel);	// disable tone
							enable &= ~(0x08 << channel);	// enable noise
							ym2203_write_port_w(NMK004_state.ymdevice, 0, enable);
							break;

						case 0xf2:	// set volume shape
						case 0xf3:
						case 0xf4:
						case 0xf5:
							psg->volume_shape = read8(psg->current++);
							break;

						case 0xf6:	// jump
							psg->current = read16(psg->current);
							break;

						case 0xf7:	// begin repeat loop
							psg->loop_times = read8(psg->current++);
							psg->loop_start = psg->current;
							break;

						case 0xf8:	// end repeat loop
							if (--psg->loop_times > 0)
								psg->current = psg->loop_start;
							break;

						case 0xf9:	// call subtable
							psg->return_address[psg->return_address_depth++] = psg->current + 2;
							psg->current = read16(psg->current);
							break;

						case 0xfa:	// return from subtable
							psg->current = psg->return_address[--psg->return_address_depth];
							break;

						case 0xfb:	// set octave
						case 0xfc:
						case 0xfd:
						case 0xfe:
							psg->octave = read8(psg->current++);
							break;

						case 0xff:	// end
							psg->flags = 0;	// disable channel
							psg->volume_shape = 0;

							// mute channel
							ym2203_control_port_w(NMK004_state.ymdevice, 0, 8 + channel);
							ym2203_write_port_w(NMK004_state.ymdevice, 0, 0);
							return;
					}
				}
			} while ((token & 0xf0) == 0xf0);

			// token is the note to play
			psg->note = token;
			if ((psg->note & 0x0f) > NOTE_PAUSE)
			{

			}

			// optional note length (otherwise use the same length as the previous one)
			if (read8(psg->current) & 0x80)
			{
				UINT16 table_start = read16(NOTE_LENGTH_TABLE_1);

				psg->note_length = read16(table_start + 2 * (read8(psg->current++) & 0x7f));
			}

			psg->note_timer = psg->note_length;
			psg->volume_timer = 0;

			psg->flags |= PSG_FLAG_INITIALIZE_VOLUME;

			if (psg->note == NOTE_PAUSE)
				psg->flags |=  PSG_FLAG_NOTE_IS_PAUSE;
			else
			{
				psg->flags &= ~PSG_FLAG_NOTE_IS_PAUSE;

				if (!(psg->flags & PSG_FLAG_NOTE_IS_NOISE))
				{
					UINT16 table_start = read16(PSG_NOTE_TABLE);
					UINT16 period = read16(table_start + 2 * (psg->note & 0x0f));
					UINT8 octave = psg->octave + ((psg->note & 0xf0) >> 4);

					period >>= octave;

					ym2203_control_port_w(NMK004_state.ymdevice, 0, 2 * channel + 1);
					ym2203_write_port_w(NMK004_state.ymdevice, 0, (period & 0x0f00) >> 8);
					ym2203_control_port_w(NMK004_state.ymdevice, 0, 2 * channel + 0);
					ym2203_write_port_w(NMK004_state.ymdevice, 0, (period & 0x00ff));

					psg->note_period_hi_bits = (period & 0x0f00) >> 8;
				}
				else
				{
					if (!(psg->flags & PSG_FLAG_NOISE_NOT_ENABLED))
					{
						INT32 enable;

						psg->flags |= PSG_FLAG_NOISE_NOT_ENABLED;

						// disable noise, enable tone on this channel
						ym2203_control_port_w(NMK004_state.ymdevice, 0, 0x07);
						enable = ym2203_read_port_r(NMK004_state.ymdevice, 0);
						enable &= ~(0x01 << channel);	// enable tone
						enable |=  (0x08 << channel);	// disable noise
						ym2203_write_port_w(NMK004_state.ymdevice, 0, enable);
					}

					ym2203_control_port_w(NMK004_state.ymdevice, 0, 0x06);	// noise period
					ym2203_write_port_w(NMK004_state.ymdevice, 0, psg->note);
					psg->note_period_hi_bits = psg->note;
				}
			}
		}

		if (psg->volume_timer == 0)
		{
			UINT16 table_start = read16(PSG_VOLUME_TABLE);
			UINT16 vol_table_start = read16(table_start + 2 * psg->volume_shape);
			INT32 volume;

			if (psg->flags & PSG_FLAG_INITIALIZE_VOLUME)
			{
				psg->flags &= ~PSG_FLAG_INITIALIZE_VOLUME;
				psg->volume_position = 0;
			}

			volume = read8(vol_table_start + psg->volume_position++);
			psg->volume_timer = read8(vol_table_start + psg->volume_position++);

			if (psg->flags & PSG_FLAG_NOTE_IS_PAUSE)
				volume = 0;

			// set volume
			ym2203_control_port_w(NMK004_state.ymdevice, 0, 8 + channel);
			ym2203_write_port_w(NMK004_state.ymdevice, 0, volume & 0x0f);
		}
	}
}

/*****************************

     Command processing

*****************************/

static void get_command()
{
	static const UINT8 from_main[] =
	{
		0x00,0x22,0x62,0x8c,0xc7,0x00,0x00,0x3f,0x7f,0x89,0xc7,0x00,0x00,0x2b,0x6b
	};
	static const UINT8 to_main[] =
	{
		0x82,0xc7,0x00,0x2c,0x6c,0x00,0x9f,0xc7,0x00,0x29,0x69,0x00,0x8b,0xc7,0x00
	};

	UINT8 cmd = NMK004_state.from_main;

	if (NMK004_state.protection_check < (INT32)sizeof(to_main))
	{
		// startup handshake
		if (cmd == from_main[NMK004_state.protection_check])
		{
//			logerror("advance handshake to %02x\n",to_main[NMK004_state.protection_check]);
			NMK004_state.to_main = to_main[NMK004_state.protection_check++];
		}
	}
	else
	{
		// send command back to main CPU to acknowledge reception
		NMK004_state.to_main = cmd;
	}

	if (NMK004_state.last_command != cmd)
	{
		UINT16 table_start = read16(COMMAND_TABLE);
		UINT16 cmd_table = read16(table_start + 2 * cmd);

		NMK004_state.last_command = cmd;

		if ((cmd_table & 0xff00) == 0)
		{
			oki_play_sample(cmd_table);
		}
		else
		{
			INT32 channel;

			while ((channel = read8(cmd_table++)) != 0xff)
			{
				table_start = read16(cmd_table);
				cmd_table += 2;

				if (channel == 0xef) break;	// bioship bug?

				if (channel < FM_CHANNELS)
				{
					NMK004_state.fm_control[channel].current = table_start;
					NMK004_state.fm_control[channel].return_address_depth = 0;
					NMK004_state.fm_control[channel].flags |= FM_FLAG_NEED_INITIALIZATION;
				}
				else
				{
					channel -= FM_CHANNELS;
					if (channel < PSG_CHANNELS)
					{
						NMK004_state.psg_control[channel].current = table_start;
						NMK004_state.psg_control[channel].return_address_depth = 0;
						NMK004_state.psg_control[channel].flags |= PSG_FLAG_NEED_INITIALIZATION;
					}
					else
					{
						channel -= PSG_CHANNELS;
						if (channel >= EFFECTS_CHANNELS)
						{

						}
						NMK004_state.effects_control[channel].current = table_start;
						NMK004_state.effects_control[channel].return_address_depth = 0;
						NMK004_state.effects_control[channel].flags |= EFFECTS_FLAG_NEED_INITIALIZATION;
					}
				}
			}
		}
	}
}

static void update_music(void)
{
	INT32 channel;

	for (channel = 0; channel < FM_CHANNELS; channel++)
		fm_update(channel);
	fm_voices_update();

	for (channel = 0; channel < PSG_CHANNELS; channel++)
		psg_update(channel);

	for (channel = 0; channel < EFFECTS_CHANNELS; channel++)
		effects_update(channel);
}

void NMK004_irq(INT32 irq)
{
	if (irq)
	{
		INT32 status = ym2203_status_port_r(device,0);

		if (status & 1)	// timer A expired
		{
			oki_update_state();
			get_command();
			update_music();

			// restart timer
			ym2203_control_port_w(device, 0, 0x27);
			ym2203_write_port_w(device, 0, 0x15);
		}
	}
}

void NMK004_init()
{
	static const UINT8 ym2203_init[] =
	{
		0x07,0x38,0x08,0x00,0x09,0x00,0x0A,0x00,0x24,0xB3,0x25,0x00,0x26,0xF9,0x27,0x15,
		0x28,0x00,0x28,0x01,0x28,0x02,0x40,0x00,0x41,0x00,0x42,0x00,0x44,0x00,0x45,0x00,
		0x46,0x00,0x48,0x00,0x49,0x00,0x4A,0x00,0x4C,0x00,0x4D,0x00,0x4E,0x00,0xFF,
	};
	INT32 i;

	memset(&NMK004_state, 0, sizeof(NMK004_state));

	NMK004_state.ymdevice = 0;
	NMK004_state.oki1device = 0;
	NMK004_state.oki2device = 1;
	NMK004_state.rom = NMK004PROGROM;

	ym2203_control_port_w(NMK004_state.ymdevice, 0, 0x2f);

	i = 0;
	while (ym2203_init[i] != 0xff)
	{
		ym2203_control_port_w(NMK004_state.ymdevice, 0, ym2203_init[i++]);
		ym2203_write_port_w(NMK004_state.ymdevice, 0, ym2203_init[i++]);
	}

	NMK004_state.oki_playing = 0;

	oki_play_sample(0);

	NMK004_state.protection_check = 0;
}

void NMK004Write(INT32, INT32 data)
{
	NMK004_state.from_main = data & 0xff;
}

UINT8 NMK004Read()
{
	return NMK004_state.to_main;
}

