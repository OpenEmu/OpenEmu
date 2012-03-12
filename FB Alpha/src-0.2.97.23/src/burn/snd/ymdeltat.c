/*
**
** File: ymdeltat.c
**
** YAMAHA DELTA-T adpcm sound emulation subroutine
** used by fmopl.c (Y8950) and fm.c (YM2608 and YM2610/B)
**
** Base program is YM2610 emulator by Hiromitsu Shioya.
** Written by Tatsuyuki Satoh
** Improvements by Jarek Burczynski (bujar at mame dot net)
**
**
** History:
**
** 03-08-2003 Jarek Burczynski:
**  - fixed BRDY flag implementation.
**
** 24-07-2003 Jarek Burczynski, Frits Hilderink:
**  - fixed delault value for control2 in YM_DELTAT_ADPCM_Reset
**
** 22-07-2003 Jarek Burczynski, Frits Hilderink:
**  - fixed external memory support
**
** 15-06-2003 Jarek Burczynski:
**  - implemented CPU -> AUDIO ADPCM synthesis (via writes to the ADPCM data reg $08)
**  - implemented support for the Limit address register
**  - supported two bits from the control register 2 ($01): RAM TYPE (x1 bit/x8 bit), ROM/RAM
**  - implemented external memory access (read/write) via the ADPCM data reg reads/writes
**    Thanks go to Frits Hilderink for the example code.
**
** 14-06-2003 Jarek Burczynski:
**  - various fixes to enable proper support for status register flags: BSRDY, PCM BSY, ZERO
**  - modified EOS handling
**
** 05-04-2003 Jarek Burczynski:
**  - implemented partial support for external/processor memory on sample replay
**
** 01-12-2002 Jarek Burczynski:
**  - fixed first missing sound in gigandes thanks to previous fix (interpolator) by ElSemi
**  - renamed/removed some YM_DELTAT struct fields
**
** 28-12-2001 Acho A. Tang
**  - added EOS status report on ADPCM playback.
**
** 05-08-2001 Jarek Burczynski:
**  - now_step is initialized with 0 at the start of play.
**
** 12-06-2001 Jarek Burczynski:
**  - corrected end of sample bug in YM_DELTAT_ADPCM_CALC.
**    Checked on real YM2610 chip - address register is 24 bits wide.
**    Thanks go to Stefan Jokisch (stefan.jokisch@gmx.de) for tracking down the problem.
**
** TO DO:
**		Check size of the address register on the other chips....
**
** Version 0.72
**
** sound chips that have this unit:
** YM2608   OPNA
** YM2610/B OPNB
** Y8950    MSX AUDIO
**
*/

#include "driver.h"
#include "state.h"
#include "ymdeltat.h"

#define YM_DELTAT_DELTA_MAX (24576)
#define YM_DELTAT_DELTA_MIN (127)
#define YM_DELTAT_DELTA_DEF (127)

#define YM_DELTAT_DECODE_RANGE 32768
#define YM_DELTAT_DECODE_MIN (-(YM_DELTAT_DECODE_RANGE))
#define YM_DELTAT_DECODE_MAX ((YM_DELTAT_DECODE_RANGE)-1)


/* Forecast to next Forecast (rate = *8) */
/* 1/8 , 3/8 , 5/8 , 7/8 , 9/8 , 11/8 , 13/8 , 15/8 */
const INT32 ym_deltat_decode_tableB1[16] = {
  1,   3,   5,   7,   9,  11,  13,  15,
  -1,  -3,  -5,  -7,  -9, -11, -13, -15,
};
/* delta to next delta (rate= *64) */
/* 0.9 , 0.9 , 0.9 , 0.9 , 1.2 , 1.6 , 2.0 , 2.4 */
const INT32 ym_deltat_decode_tableB2[16] = {
  57,  57,  57,  57, 77, 102, 128, 153,
  57,  57,  57,  57, 77, 102, 128, 153
};

#if 0
void YM_DELTAT_BRDY_callback(YM_DELTAT *DELTAT)
{
	logerror("BRDY_callback reached (flag set) !\n");

	/* set BRDY bit in status register */
	if(DELTAT->status_set_handler)
		if(DELTAT->status_change_BRDY_bit)
			(DELTAT->status_set_handler)(DELTAT->status_change_which_chip, DELTAT->status_change_BRDY_bit);
}
#endif

UINT8 YM_DELTAT_ADPCM_Read(YM_DELTAT *DELTAT)
{
	UINT8 v = 0;

	/* external memory read */
	if ( (DELTAT->portstate & 0xe0)==0x20 )
	{
		/* two dummy reads */
		if (DELTAT->memread)
		{
			DELTAT->now_addr = DELTAT->start << 1;
			DELTAT->memread--;
			return 0;
		}


		if ( DELTAT->now_addr != (DELTAT->end<<1) )
		{
			v = DELTAT->memory[DELTAT->now_addr>>1];

			/*logerror("YM Delta-T memory read  $%08x, v=$%02x\n", DELTAT->now_addr >> 1, v);*/

			DELTAT->now_addr+=2; /* two nibbles at a time */

			/* reset BRDY bit in status register, which means we are reading the memory now */
			if(DELTAT->status_reset_handler)
				if(DELTAT->status_change_BRDY_bit)
					(DELTAT->status_reset_handler)(DELTAT->status_change_which_chip, DELTAT->status_change_BRDY_bit);

	/* setup a timer that will callback us in 10 master clock cycles for Y8950
	* in the callback set the BRDY flag to 1 , which means we have another data ready.
	* For now, we don't really do this; we simply reset and set the flag in zero time, so that the IRQ will work.
	*/
			/* set BRDY bit in status register */
			if(DELTAT->status_set_handler)
				if(DELTAT->status_change_BRDY_bit)
					(DELTAT->status_set_handler)(DELTAT->status_change_which_chip, DELTAT->status_change_BRDY_bit);
		}
		else
		{
			/* set EOS bit in status register */
			if(DELTAT->status_set_handler)
				if(DELTAT->status_change_EOS_bit)
					(DELTAT->status_set_handler)(DELTAT->status_change_which_chip, DELTAT->status_change_EOS_bit);
		}
	}

	return v;
}


/* 0-DRAM x1, 1-ROM, 2-DRAM x8, 3-ROM (3 is bad setting - not allowed by the manual) */
static UINT8 dram_rightshift[4]={3,0,0,0};

/* DELTA-T ADPCM write register */
void YM_DELTAT_ADPCM_Write(YM_DELTAT *DELTAT,int r,int v)
{
	if(r>=0x10) return;
	DELTAT->reg[r] = v; /* stock data */

	switch( r )
	{
	case 0x00:
/*
START:
	Accessing *external* memory is started when START bit (D7) is set to "1", so
	you must set all conditions needed for recording/playback before starting.
	If you access *CPU-managed* memory, recording/playback starts after
	read/write of ADPCM data register $08.

REC:
	0 = ADPCM synthesis (playback)
	1 = ADPCM analysis (record)

MEMDATA:
	0 = processor (*CPU-managed*) memory (means: using register $08)
	1 = external memory (using start/end/limit registers to access memory: RAM or ROM)


SPOFF:
	controls output pin that should disable the speaker while ADPCM analysis

RESET and REPEAT only work with external memory.


some examples:
value:   START, REC, MEMDAT, REPEAT, SPOFF, x,x,RESET   meaning:
  C8     1      1    0       0       1      0 0 0       Analysis (recording) from AUDIO to CPU (to reg $08), sample rate in PRESCALER register
  E8     1      1    1       0       1      0 0 0       Analysis (recording) from AUDIO to EXT.MEMORY,       sample rate in PRESCALER register
  80     1      0    0       0       0      0 0 0       Synthesis (playing) from CPU (from reg $08) to AUDIO,sample rate in DELTA-N register
  a0     1      0    1       0       0      0 0 0       Synthesis (playing) from EXT.MEMORY to AUDIO,        sample rate in DELTA-N register

  60     0      1    1       0       0      0 0 0       External memory write via ADPCM data register $08
  20     0      0    1       0       0      0 0 0       External memory read via ADPCM data register $08

*/
		/* handle emulation mode */
		if(DELTAT->emulation_mode == YM_DELTAT_EMULATION_MODE_YM2610)
		{
			v |= 0x20;		/*	YM2610 always uses external memory and doesn't even have memory flag bit. */
		}

		DELTAT->portstate = v & (0x80|0x40|0x20|0x10|0x01); /* start, rec, memory mode, repeat flag copy, reset(bit0) */

		if( DELTAT->portstate&0x80 )/* START,REC,MEMDATA,REPEAT,SPOFF,--,--,RESET */
		{
			/* set PCM BUSY bit */
			DELTAT->PCM_BSY = 1;

			/* start ADPCM */
			DELTAT->now_step = 0;
			DELTAT->acc      = 0;
			DELTAT->prev_acc = 0;
			DELTAT->adpcml   = 0;
			DELTAT->adpcmd   = YM_DELTAT_DELTA_DEF;
			DELTAT->now_data = 0;

		}

		if( DELTAT->portstate&0x20 ) /* do we access external memory? */
		{
			DELTAT->now_addr = DELTAT->start << 1;
			DELTAT->memread = 2;	/* two dummy reads needed before accesing external memory via register $08*/

			/* if yes, then let's check if ADPCM memory is mapped and big enough */
			if(DELTAT->memory == 0)
			{
				logerror("YM Delta-T ADPCM rom not mapped\n");
				DELTAT->portstate = 0x00;
				DELTAT->PCM_BSY = 0;
			}
			else
			{
				if( DELTAT->end >= DELTAT->memory_size )	/* Check End in Range */
				{
					logerror("YM Delta-T ADPCM end out of range: $%08x\n", DELTAT->end);
					DELTAT->end = DELTAT->memory_size - 1;
				}
				if( DELTAT->start >= DELTAT->memory_size )	/* Check Start in Range */
				{
					logerror("YM Delta-T ADPCM start out of range: $%08x\n", DELTAT->start);
					DELTAT->portstate = 0x00;
					DELTAT->PCM_BSY = 0;
				}
			}
		}
		else	/* we access CPU memory (ADPCM data register $08) so we only reset now_addr here */
		{
			DELTAT->now_addr = 0;
		}

		if( DELTAT->portstate&0x01 )
		{
			DELTAT->portstate = 0x00;

			/* clear PCM BUSY bit (in status register) */
			DELTAT->PCM_BSY = 0;

			/* set BRDY flag */
			if(DELTAT->status_set_handler)
				if(DELTAT->status_change_BRDY_bit)
					(DELTAT->status_set_handler)(DELTAT->status_change_which_chip, DELTAT->status_change_BRDY_bit);
		}
		break;
	case 0x01:	/* L,R,-,-,SAMPLE,DA/AD,RAMTYPE,ROM */
		/* handle emulation mode */
		if(DELTAT->emulation_mode == YM_DELTAT_EMULATION_MODE_YM2610)
		{
			v |= 0x01;		/*	YM2610 always uses ROM as an external memory and doesn't tave ROM/RAM memory flag bit. */
		}

		DELTAT->pan = &DELTAT->output_pointer[(v>>6)&0x03];
		if ((DELTAT->control2 & 3) != (v & 3))
		{
			/*0-DRAM x1, 1-ROM, 2-DRAM x8, 3-ROM (3 is bad setting - not allowed by the manual) */
			if (DELTAT->DRAMportshift != dram_rightshift[v&3])
			{
				DELTAT->DRAMportshift = dram_rightshift[v&3];

				/* final shift value depends on chip type and memory type selected:
						8 for YM2610 (ROM only),
						5 for ROM for Y8950 and YM2608,
						5 for x8bit DRAMs for Y8950 and YM2608,
						2 for x1bit DRAMs for Y8950 and YM2608.
				*/

				/* refresh addresses */
				DELTAT->start  = (DELTAT->reg[0x3]*0x0100 | DELTAT->reg[0x2]) << (DELTAT->portshift - DELTAT->DRAMportshift);
				DELTAT->end    = (DELTAT->reg[0x5]*0x0100 | DELTAT->reg[0x4]) << (DELTAT->portshift - DELTAT->DRAMportshift);
				DELTAT->end   += (1 << (DELTAT->portshift-DELTAT->DRAMportshift) ) - 1;
				DELTAT->limit  = (DELTAT->reg[0xd]*0x0100 | DELTAT->reg[0xc]) << (DELTAT->portshift - DELTAT->DRAMportshift);
			}
		}
		DELTAT->control2 = v;
		break;
	case 0x02:	/* Start Address L */
	case 0x03:	/* Start Address H */
		DELTAT->start  = (DELTAT->reg[0x3]*0x0100 | DELTAT->reg[0x2]) << (DELTAT->portshift - DELTAT->DRAMportshift);
		/*logerror("DELTAT start: 02=%2x 03=%2x addr=%8x\n",DELTAT->reg[0x2], DELTAT->reg[0x3],DELTAT->start );*/
		break;
	case 0x04:	/* Stop Address L */
	case 0x05:	/* Stop Address H */
		DELTAT->end    = (DELTAT->reg[0x5]*0x0100 | DELTAT->reg[0x4]) << (DELTAT->portshift - DELTAT->DRAMportshift);
		DELTAT->end   += (1 << (DELTAT->portshift-DELTAT->DRAMportshift) ) - 1;
		/*logerror("DELTAT end  : 04=%2x 05=%2x addr=%8x\n",DELTAT->reg[0x4], DELTAT->reg[0x5],DELTAT->end   );*/
		break;
	case 0x06:	/* Prescale L (ADPCM and Record frq) */
	case 0x07:	/* Prescale H */
		break;
	case 0x08:	/* ADPCM data */

/*
some examples:
value:   START, REC, MEMDAT, REPEAT, SPOFF, x,x,RESET   meaning:
  C8     1      1    0       0       1      0 0 0       Analysis (recording) from AUDIO to CPU (to reg $08), sample rate in PRESCALER register
  E8     1      1    1       0       1      0 0 0       Analysis (recording) from AUDIO to EXT.MEMORY,       sample rate in PRESCALER register
  80     1      0    0       0       0      0 0 0       Synthesis (playing) from CPU (from reg $08) to AUDIO,sample rate in DELTA-N register
  a0     1      0    1       0       0      0 0 0       Synthesis (playing) from EXT.MEMORY to AUDIO,        sample rate in DELTA-N register

  60     0      1    1       0       0      0 0 0       External memory write via ADPCM data register $08
  20     0      0    1       0       0      0 0 0       External memory read via ADPCM data register $08

*/

		/* external memory write */
		if ( (DELTAT->portstate & 0xe0)==0x60 )
		{
			if (DELTAT->memread)
			{
				DELTAT->now_addr = DELTAT->start << 1;
				DELTAT->memread = 0;
			}

			/*logerror("YM Delta-T memory write $%08x, v=$%02x\n", DELTAT->now_addr >> 1, v);*/

			if ( DELTAT->now_addr != (DELTAT->end<<1) )
			{
				DELTAT->memory[DELTAT->now_addr>>1] = v;
			 	DELTAT->now_addr+=2; /* two nibbles at a time */

				/* reset BRDY bit in status register, which means we are processing the write */
				if(DELTAT->status_reset_handler)
					if(DELTAT->status_change_BRDY_bit)
						(DELTAT->status_reset_handler)(DELTAT->status_change_which_chip, DELTAT->status_change_BRDY_bit);

	/* setup a timer that will callback us in 10 master clock cycles for Y8950
	* in the callback set the BRDY flag to 1 , which means we have written the data.
	* For now, we don't really do this; we simply reset and set the flag in zero time, so that the IRQ will work.
	*/
				/* set BRDY bit in status register */
				if(DELTAT->status_set_handler)
					if(DELTAT->status_change_BRDY_bit)
						(DELTAT->status_set_handler)(DELTAT->status_change_which_chip, DELTAT->status_change_BRDY_bit);

			}
			else
			{
				/* set EOS bit in status register */
				if(DELTAT->status_set_handler)
					if(DELTAT->status_change_EOS_bit)
						(DELTAT->status_set_handler)(DELTAT->status_change_which_chip, DELTAT->status_change_EOS_bit);
			}

			return;
		}

		/* ADPCM synthesis from CPU */
		if ( (DELTAT->portstate & 0xe0)==0x80 )
		{
			DELTAT->CPU_data = v;

			/* Reset BRDY bit in status register, which means we are full of data */
			if(DELTAT->status_reset_handler)
				if(DELTAT->status_change_BRDY_bit)
					(DELTAT->status_reset_handler)(DELTAT->status_change_which_chip, DELTAT->status_change_BRDY_bit);
			return;
		}

	  break;
	case 0x09:	/* DELTA-N L (ADPCM Playback Prescaler) */
	case 0x0a:	/* DELTA-N H */
		DELTAT->delta  = (DELTAT->reg[0xa]*0x0100 | DELTAT->reg[0x9]);
		DELTAT->step     = (UINT32)( (double)(DELTAT->delta /* *(1<<(YM_DELTAT_SHIFT-16)) */ ) * (DELTAT->freqbase) );
		/*logerror("DELTAT deltan:09=%2x 0a=%2x\n",DELTAT->reg[0x9], DELTAT->reg[0xa]);*/
		break;
	case 0x0b:	/* Output level control (volume, linear) */
		{
			INT32 oldvol = DELTAT->volume;
			DELTAT->volume = (v&0xff) * (DELTAT->output_range/256) / YM_DELTAT_DECODE_RANGE;
/*								v	  *		((1<<16)>>8)		>>	15;
*						thus:	v	  *		(1<<8)				>>	15;
*						thus: output_range must be (1 << (15+8)) at least
*								v     *		((1<<23)>>8)		>>	15;
*								v	  *		(1<<15)				>>	15;
*/
			/*logerror("DELTAT vol = %2x\n",v&0xff);*/
			if( oldvol != 0 )
			{
				DELTAT->adpcml = (int)((double)DELTAT->adpcml / (double)oldvol * (double)DELTAT->volume);
			}
		}
		break;
	case 0x0c:	/* Limit Address L */
	case 0x0d:	/* Limit Address H */
		DELTAT->limit  = (DELTAT->reg[0xd]*0x0100 | DELTAT->reg[0xc]) << (DELTAT->portshift - DELTAT->DRAMportshift);
		/*logerror("DELTAT limit: 0c=%2x 0d=%2x addr=%8x\n",DELTAT->reg[0xc], DELTAT->reg[0xd],DELTAT->limit );*/
		break;
	}
}

void YM_DELTAT_ADPCM_Reset(YM_DELTAT *DELTAT,int pan,int emulation_mode)
{
	DELTAT->now_addr  = 0;
	DELTAT->now_step  = 0;
	DELTAT->step      = 0;
	DELTAT->start     = 0;
	DELTAT->end       = 0;
	DELTAT->limit     = ~0; /* this way YM2610 and Y8950 (both of which don't have limit address reg) will still work */
	DELTAT->volume    = 0;
	DELTAT->pan       = &DELTAT->output_pointer[pan];
	DELTAT->acc       = 0;
	DELTAT->prev_acc  = 0;
	DELTAT->adpcmd    = 127;
	DELTAT->adpcml    = 0;
	DELTAT->emulation_mode = (UINT8)emulation_mode;
	DELTAT->portstate = (emulation_mode == YM_DELTAT_EMULATION_MODE_YM2610) ? 0x20 : 0;
	DELTAT->control2  = (emulation_mode == YM_DELTAT_EMULATION_MODE_YM2610) ? 0x01 : 0;	/* default setting depends on the emulation mode. MSX demo called "facdemo_4" doesn't setup control2 register at all and still works */
	DELTAT->DRAMportshift = dram_rightshift[DELTAT->control2 & 3];

	/* The flag mask register disables the BRDY after the reset, however
	** as soon as the mask is enabled the flag needs to be set. */

	/* set BRDY bit in status register */
	if(DELTAT->status_set_handler)
		if(DELTAT->status_change_BRDY_bit)
			(DELTAT->status_set_handler)(DELTAT->status_change_which_chip, DELTAT->status_change_BRDY_bit);
}

void YM_DELTAT_postload(YM_DELTAT *DELTAT,UINT8 *regs)
{
	int r;

	/* to keep adpcml */
	DELTAT->volume = 0;
	/* update */
	for(r=1;r<16;r++)
		YM_DELTAT_ADPCM_Write(DELTAT,r,regs[r]);
	DELTAT->reg[0] = regs[0];

	/* current rom data */
	if (DELTAT->memory)
		DELTAT->now_data = *(DELTAT->memory + (DELTAT->now_addr>>1) );

}
void YM_DELTAT_savestate(const char *statename,int num,YM_DELTAT *DELTAT)
{
#ifdef _STATE_H
	state_save_register_UINT8 (statename, num, "DeltaT.portstate", &DELTAT->portstate, 1);
	state_save_register_UINT32(statename, num, "DeltaT.address"  , &DELTAT->now_addr , 1);
	state_save_register_UINT32(statename, num, "DeltaT.step"     , &DELTAT->now_step , 1);
	state_save_register_INT32 (statename, num, "DeltaT.acc"      , &DELTAT->acc      , 1);
	state_save_register_INT32 (statename, num, "DeltaT.prev_acc" , &DELTAT->prev_acc , 1);
	state_save_register_INT32 (statename, num, "DeltaT.adpcmd"   , &DELTAT->adpcmd   , 1);
	state_save_register_INT32 (statename, num, "DeltaT.adpcml"   , &DELTAT->adpcml   , 1);
#endif
}


#define YM_DELTAT_Limit(val,max,min)	\
{										\
	if ( val > max ) val = max;			\
	else if ( val < min ) val = min;	\
}

INLINE void YM_DELTAT_synthesis_from_external_memory(YM_DELTAT *DELTAT)
{
	UINT32 step;
	int data;

	DELTAT->now_step += DELTAT->step;
	if ( DELTAT->now_step >= (1<<YM_DELTAT_SHIFT) )
	{
		step = DELTAT->now_step >> YM_DELTAT_SHIFT;
		DELTAT->now_step &= (1<<YM_DELTAT_SHIFT)-1;
		do{

			if ( DELTAT->now_addr == (DELTAT->limit<<1) )
				DELTAT->now_addr = 0;

			if ( DELTAT->now_addr == (DELTAT->end<<1) ) {	/* 12-06-2001 JB: corrected comparison. Was > instead of == */
				if( DELTAT->portstate&0x10 ){
					/* repeat start */
					DELTAT->now_addr = DELTAT->start<<1;
					DELTAT->acc      = 0;
					DELTAT->adpcmd   = YM_DELTAT_DELTA_DEF;
					DELTAT->prev_acc = 0;
				}else{
					/* set EOS bit in status register */
					if(DELTAT->status_set_handler)
						if(DELTAT->status_change_EOS_bit)
							(DELTAT->status_set_handler)(DELTAT->status_change_which_chip, DELTAT->status_change_EOS_bit);

					/* clear PCM BUSY bit (reflected in status register) */
					DELTAT->PCM_BSY = 0;

					DELTAT->portstate = 0;
					DELTAT->adpcml = 0;
					DELTAT->prev_acc = 0;
					return;
				}
			}

			if( DELTAT->now_addr&1 ) data = DELTAT->now_data & 0x0f;
			else
			{
				DELTAT->now_data = *(DELTAT->memory + (DELTAT->now_addr>>1));
				data = DELTAT->now_data >> 4;
			}

			DELTAT->now_addr++;
			/* 12-06-2001 JB: */
			/* YM2610 address register is 24 bits wide.*/
			/* The "+1" is there because we use 1 bit more for nibble calculations.*/
			/* WARNING: */
			/* Side effect: we should take the size of the mapped ROM into account */
			DELTAT->now_addr &= ( (1<<(24+1))-1);

			/* store accumulator value */
			DELTAT->prev_acc = DELTAT->acc;

			/* Forecast to next Forecast */
			DELTAT->acc += (ym_deltat_decode_tableB1[data] * DELTAT->adpcmd / 8);
			YM_DELTAT_Limit(DELTAT->acc,YM_DELTAT_DECODE_MAX, YM_DELTAT_DECODE_MIN);

			/* delta to next delta */
			DELTAT->adpcmd = (DELTAT->adpcmd * ym_deltat_decode_tableB2[data] ) / 64;
			YM_DELTAT_Limit(DELTAT->adpcmd,YM_DELTAT_DELTA_MAX, YM_DELTAT_DELTA_MIN );

			/* ElSemi: Fix interpolator. */
			/*DELTAT->prev_acc = prev_acc + ((DELTAT->acc - prev_acc) / 2 );*/

		}while(--step);

	}

	/* ElSemi: Fix interpolator. */
	DELTAT->adpcml = DELTAT->prev_acc * (int)((1<<YM_DELTAT_SHIFT)-DELTAT->now_step);
	DELTAT->adpcml += (DELTAT->acc * (int)DELTAT->now_step);
	DELTAT->adpcml = (DELTAT->adpcml>>YM_DELTAT_SHIFT) * (int)DELTAT->volume;

	/* output for work of output channels (outd[OPNxxxx])*/
	*(DELTAT->pan) += DELTAT->adpcml;
}



INLINE void YM_DELTAT_synthesis_from_CPU_memory(YM_DELTAT *DELTAT)
{
	UINT32 step;
	int data;

	DELTAT->now_step += DELTAT->step;
	if ( DELTAT->now_step >= (1<<YM_DELTAT_SHIFT) )
	{
		step = DELTAT->now_step >> YM_DELTAT_SHIFT;
		DELTAT->now_step &= (1<<YM_DELTAT_SHIFT)-1;
		do{

			if( DELTAT->now_addr&1 )
			{
				data = DELTAT->now_data & 0x0f;

				DELTAT->now_data = DELTAT->CPU_data;

				/* after we used CPU_data, we set BRDY bit in status register,
				* which means we are ready to accept another byte of data */
				if(DELTAT->status_set_handler)
					if(DELTAT->status_change_BRDY_bit)
						(DELTAT->status_set_handler)(DELTAT->status_change_which_chip, DELTAT->status_change_BRDY_bit);
			}
			else
			{
				data = DELTAT->now_data >> 4;
			}

			DELTAT->now_addr++;

			/* store accumulator value */
			DELTAT->prev_acc = DELTAT->acc;

			/* Forecast to next Forecast */
			DELTAT->acc += (ym_deltat_decode_tableB1[data] * DELTAT->adpcmd / 8);
			YM_DELTAT_Limit(DELTAT->acc,YM_DELTAT_DECODE_MAX, YM_DELTAT_DECODE_MIN);

			/* delta to next delta */
			DELTAT->adpcmd = (DELTAT->adpcmd * ym_deltat_decode_tableB2[data] ) / 64;
			YM_DELTAT_Limit(DELTAT->adpcmd,YM_DELTAT_DELTA_MAX, YM_DELTAT_DELTA_MIN );


		}while(--step);

	}

	/* ElSemi: Fix interpolator. */
	DELTAT->adpcml = DELTAT->prev_acc * (int)((1<<YM_DELTAT_SHIFT)-DELTAT->now_step);
	DELTAT->adpcml += (DELTAT->acc * (int)DELTAT->now_step);
	DELTAT->adpcml = (DELTAT->adpcml>>YM_DELTAT_SHIFT) * (int)DELTAT->volume;

	/* output for work of output channels (outd[OPNxxxx])*/
	*(DELTAT->pan) += DELTAT->adpcml;
}



/* ADPCM B (Delta-T control type) */
void YM_DELTAT_ADPCM_CALC(YM_DELTAT *DELTAT)
{

/*
some examples:
value:   START, REC, MEMDAT, REPEAT, SPOFF, x,x,RESET   meaning:
  80     1      0    0       0       0      0 0 0       Synthesis (playing) from CPU (from reg $08) to AUDIO,sample rate in DELTA-N register
  a0     1      0    1       0       0      0 0 0       Synthesis (playing) from EXT.MEMORY to AUDIO,        sample rate in DELTA-N register
  C8     1      1    0       0       1      0 0 0       Analysis (recording) from AUDIO to CPU (to reg $08), sample rate in PRESCALER register
  E8     1      1    1       0       1      0 0 0       Analysis (recording) from AUDIO to EXT.MEMORY,       sample rate in PRESCALER register

  60     0      1    1       0       0      0 0 0       External memory write via ADPCM data register $08
  20     0      0    1       0       0      0 0 0       External memory read via ADPCM data register $08

*/

	if ( (DELTAT->portstate & 0xe0)==0xa0 )
	{
		YM_DELTAT_synthesis_from_external_memory(DELTAT);
		return;
	}

	if ( (DELTAT->portstate & 0xe0)==0x80 )
	{
		/* ADPCM synthesis from CPU-managed memory (from reg $08) */
		YM_DELTAT_synthesis_from_CPU_memory(DELTAT);	/* change output based on data in ADPCM data reg ($08) */
		return;
	}

//todo: ADPCM analysis
//	if ( (DELTAT->portstate & 0xe0)==0xc0 )
//	if ( (DELTAT->portstate & 0xe0)==0xe0 )

	return;
}

