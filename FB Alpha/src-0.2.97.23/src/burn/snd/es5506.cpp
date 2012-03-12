/**********************************************************************************************

     Ensoniq ES5505/6 driver
     by Aaron Giles

Ensoniq OTIS - ES5505                                            Ensoniq OTTO - ES5506

  OTIS is a VLSI device designed in a 2 micron double metal        OTTO is a VLSI device designed in a 1.5 micron double metal
   CMOS process. The device is the next generation of audio         CMOS process. The device is the next generation of audio
   technology from ENSONIQ. This new chip achieves a new            technology from ENSONIQ. All calculations in the device are
   level of audio fidelity performance. These improvements          made with at least 18-bit accuracy.
   are achieved through the use of frequency interpolation
   and on board real time digital filters. All calculations       The major features of OTTO are:
   in the device are made with at least 16 bit accuracy.           - 68 pin PLCC package
                                                                   - On chip real time digital filters
 The major features of OTIS are:                                   - Frequency interpolation
  - 48 Pin dual in line package                                    - 32 independent voices
  - On chip real time digital filters                              - Loop start and stop posistions for each voice
  - Frequency interpolation                                        - Bidirectional and reverse looping
  - 32 independent voices (up from 25 in DOCII)                    - 68000 compatibility for asynchronous bus communication
  - Loop start and stop positions for each voice                   - Seperate host and sound memory interface
  - Bidirectional and reverse looping                              - 6 channel stereo serial communication port
  - 68000 compatibility for asynchronous bus communication         - Programmable clocks for defining serial protocol
  - On board pulse width modulation D to A                         - Internal volume multiplication and stereo panning
  - 4 channel stereo serial communication port                     - A to D input for pots and wheels
  - Internal volume multiplication and stereo panning              - Hardware support for envelopes
  - A to D input for pots and wheels                               - Support for dual OTTO systems
  - Up to 10MHz operation                                          - Optional compressed data format for sample data
                                                                   - Up to 16MHz operation
              ______    ______
            _|o     \__/      |_
 A17/D13 - |_|1             48|_| - VSS                                                           A A A A A A
            _|                |_                                                                  2 1 1 1 1 1 A
 A18/D14 - |_|2             47|_| - A16/D12                                                       0 9 8 7 6 5 1
            _|                |_                                                                  / / / / / / 4
 A19/D15 - |_|3             46|_| - A15/D11                                   H H H H H H H V V H D D D D D D /
            _|                |_                                              D D D D D D D S D D 1 1 1 1 1 1 D
      BS - |_|4             45|_| - A14/D10                                   0 1 2 3 4 5 6 S D 7 5 4 3 2 1 0 9
            _|                |_                                             ------------------------------------+
  PWZERO - |_|5             44|_| - A13/D9                                  / 9 8 7 6 5 4 3 2 1 6 6 6 6 6 6 6 6  |
            _|                |_                                           /                    8 7 6 5 4 3 2 1  |
    SER0 - |_|6             43|_| - A12/D8                                |                                      |
            _|       E        |_                                      SER0|10                                  60|A13/D8
    SER1 - |_|7      N      42|_| - A11/D7                            SER1|11                                  59|A12/D7
            _|       S        |_                                      SER2|12                                  58|A11/D6
    SER2 - |_|8      O      41|_| - A10/D6                            SER3|13              ENSONIQ             57|A10/D5
            _|       N        |_                                      SER4|14                                  56|A9/D4
    SER3 - |_|9      I      40|_| - A9/D5                             SER5|15                                  55|A8/D3
            _|       Q        |_                                      WCLK|16                                  54|A7/D2
 SERWCLK - |_|10            39|_| - A8/D4                            LRCLK|17               ES5506             53|A6/D1
            _|                |_                                      BCLK|18                                  52|A5/D0
   SERLR - |_|11            38|_| - A7/D3                             RESB|19                                  51|A4
            _|                |_                                       HA5|20                                  50|A3
 SERBCLK - |_|12     E      37|_| - A6/D2                              HA4|21                OTTO              49|A2
            _|       S        |_                                       HA3|22                                  48|A1
     RLO - |_|13     5      36|_| - A5/D1                              HA2|23                                  47|A0
            _|       5        |_                                       HA1|24                                  46|BS1
     RHI - |_|14     0      35|_| - A4/D0                              HA0|25                                  45|BS0
            _|       5        |_                                    POT_IN|26                                  44|DTACKB
     LLO - |_|15            34|_| - CLKIN                                 |   2 2 2 3 3 3 3 3 3 3 3 3 3 4 4 4 4  |
            _|                |_                                          |   7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3  |
     LHI - |_|16            33|_| - CAS                                   +--------------------------------------+
            _|                |_                                              B E E B E B B D S B B B E K B W W
     POT - |_|17     O      32|_| - AMUX                                      S B L N L S S D S S X S   L Q / /
            _|       T        |_                                              E E R E H M C V V A U A   C R R R
   DTACK - |_|18     I      31|_| - RAS                                       R R D H           R M C     I M
            _|       S        |_                                              _ D                 A
     R/W - |_|19            30|_| - E                                         T
            _|                |_                                              O
      MS - |_|20            29|_| - IRQ                                       P
            _|                |_
      CS - |_|21            28|_| - A3
            _|                |_
     RES - |_|22            27|_| - A2
            _|                |_
     VSS - |_|23            26|_| - A1
            _|                |_
     VDD - |_|24            25|_| - A0
             |________________|

***********************************************************************************************/

//#include "emu.h"
#include "burnint.h"
#include "es5506.h"



/**********************************************************************************************

     CONSTANTS

***********************************************************************************************/

#define LOG_COMMANDS			0
#define RAINE_CHECK				0
#define MAKE_WAVS				0

#if MAKE_WAVS
#include "wavwrite.h"
#endif

#define ES5506_INLINE			static
#define ES5506				1
#define ES5505				2

#define MAX_SAMPLE_CHUNK		10000
#define ULAW_MAXBITS			8

#define CONTROL_BS1				0x8000
#define CONTROL_BS0				0x4000
#define CONTROL_CMPD			0x2000
#define CONTROL_CA2				0x1000
#define CONTROL_CA1				0x0800
#define CONTROL_CA0				0x0400
#define CONTROL_LP4				0x0200
#define CONTROL_LP3				0x0100
#define CONTROL_IRQ				0x0080
#define CONTROL_DIR				0x0040
#define CONTROL_IRQE			0x0020
#define CONTROL_BLE				0x0010
#define CONTROL_LPE				0x0008
#define CONTROL_LEI				0x0004
#define CONTROL_STOP1			0x0002
#define CONTROL_STOP0			0x0001

#define CONTROL_BSMASK			(CONTROL_BS1 | CONTROL_BS0)
#define CONTROL_CAMASK			(CONTROL_CA2 | CONTROL_CA1 | CONTROL_CA0)
#define CONTROL_LPMASK			(CONTROL_LP4 | CONTROL_LP3)
#define CONTROL_LOOPMASK		(CONTROL_BLE | CONTROL_LPE)
#define CONTROL_STOPMASK		(CONTROL_STOP1 | CONTROL_STOP0)



/**********************************************************************************************

     INTERNAL DATA STRUCTURES

***********************************************************************************************/

/* struct describing a single playing voice */
typedef struct _es5506_voice es5506_voice;
struct _es5506_voice
{
	/* external state */
	UINT32		control;				/* control register */
	UINT32		freqcount;				/* frequency count register */
	UINT32		start;					/* start register */
	UINT32		lvol;					/* left volume register */
	UINT32		end;					/* end register */
	UINT32		lvramp;					/* left volume ramp register */
	UINT32		accum;					/* accumulator register */
	UINT32		rvol;					/* right volume register */
	UINT32		rvramp;					/* right volume ramp register */
	UINT32		ecount;					/* envelope count register */
	UINT32		k2;						/* k2 register */
	UINT32		k2ramp;					/* k2 ramp register */
	UINT32		k1;						/* k1 register */
	UINT32		k1ramp;					/* k1 ramp register */
	INT32		o4n1;					/* filter storage O4(n-1) */
	INT32		o3n1;					/* filter storage O3(n-1) */
	INT32		o3n2;					/* filter storage O3(n-2) */
	INT32		o2n1;					/* filter storage O2(n-1) */
	INT32		o2n2;					/* filter storage O2(n-2) */
	INT32		o1n1;					/* filter storage O1(n-1) */
	UINT32		exbank;					/* external address bank */

	/* internal state */
	UINT8		index;					/* index of this voice */
	UINT8		filtcount;				/* filter count */
	UINT32		accum_mask;
};

//typedef struct _es5506_state es5506_state;
struct _es5506_state
{
//	sound_stream *stream;				/* which stream are we using */
	INT32		sample_rate;			/* current sample rate */
	UINT16 *	region_base[4];			/* pointer to the base of the region */
	UINT32		write_latch;			/* currently accumulated data for write */
	UINT32		read_latch;				/* currently accumulated data for read */
	UINT32		master_clock;			/* master clock frequency */
	void		(*irq_callback)(INT32);	/* IRQ callback */
	UINT16		(*port_read)(void);		/* input port read */

	UINT8		current_page;			/* current register page */
	UINT8		active_voices;			/* number of active voices */
	UINT8		mode;					/* MODE register */
	UINT8		wst;					/* W_ST register */
	UINT8		wend;					/* W_END register */
	UINT8		lrend;					/* LR_END register */
	UINT8		irqv;					/* IRQV register */

	es5506_voice voice[32];				/* the 32 voices */

	INT32 *		scratch;

	INT16 *		ulaw_lookup;
	UINT16 *	volume_lookup;
//	device_t *device;

#if MAKE_WAVS
	void *		wavraw;					/* raw waveform */
#endif
};

static struct _es5506_state *chip = NULL;


/**********************************************************************************************

     GLOBAL VARIABLES

***********************************************************************************************/

static FILE *eslog;



/**********************************************************************************************

     update_irq_state -- update the IRQ state

***********************************************************************************************/

static void update_irq_state()
{
	/* ES5505/6 irq line has been set high - inform the host */
	if (chip->irq_callback)
		(*chip->irq_callback)(1); /* IRQB set high */
}

static void update_internal_irq_state()
{
	/*  Host (cpu) has just read the voice interrupt vector (voice IRQ ack).

        Reset the voice vector to show the IRQB line is low (top bit set).
        If we have any stacked interrupts (other voices waiting to be
        processed - with their IRQ bit set) then they will be moved into
        the vector next time the voice is processed.  In emulation
        terms they get updated next time generate_samples() is called.
    */

	chip->irqv=0x80;
	if (chip->irq_callback)
		(*chip->irq_callback)(0); /* IRQB set low */
}

/**********************************************************************************************

     compute_tables -- compute static tables

***********************************************************************************************/

static void compute_tables()
{
	INT32 i;

	/* allocate ulaw lookup table */
	//chip->ulaw_lookup = auto_alloc_array(chip->device->machine, INT16, 1 << ULAW_MAXBITS);
	chip->ulaw_lookup = (INT16*)malloc((1 << ULAW_MAXBITS) * sizeof(INT16));

	/* generate ulaw lookup table */
	for (i = 0; i < (1 << ULAW_MAXBITS); i++)
	{
		UINT16 rawval = (i << (16 - ULAW_MAXBITS)) | (1 << (15 - ULAW_MAXBITS));
		UINT8 exponent = rawval >> 13;
		UINT32 mantissa = (rawval << 3) & 0xffff;

		if (exponent == 0)
			chip->ulaw_lookup[i] = (INT16)mantissa >> 7;
		else
		{
			mantissa = (mantissa >> 1) | (~mantissa & 0x8000);
			chip->ulaw_lookup[i] = (INT16)mantissa >> (7 - exponent);
		}
	}

	/* allocate volume lookup table */
	//chip->volume_lookup = auto_alloc_array(chip->device->machine, UINT16, 4096);
	chip->volume_lookup = (UINT16*)malloc(4096 * sizeof(UINT16));

	/* generate volume lookup table */
	for (i = 0; i < 4096; i++)
	{
		UINT8 exponent = i >> 8;
		UINT32 mantissa = (i & 0xff) | 0x100;

		chip->volume_lookup[i] = (mantissa << 11) >> (20 - exponent);
	}
}



/**********************************************************************************************

     interpolate -- interpolate between two samples

***********************************************************************************************/

#define interpolate(sample1, sample2, accum)								\
		(sample1 * (INT32)(0x800 - (accum & 0x7ff)) +						\
		 sample2 * (INT32)(accum & 0x7ff)) >> 11;



/**********************************************************************************************

     apply_filters -- apply the 4-pole digital filter to the sample

***********************************************************************************************/

#define apply_filters(voice, sample)															\
do																								\
{																								\
	/* pole 1 is always low-pass using K1 */													\
	sample = ((INT32)(voice->k1 >> 2) * (sample - voice->o1n1) / 16384) + voice->o1n1;			\
	voice->o1n1 = sample;																		\
																								\
	/* pole 2 is always low-pass using K1 */													\
	sample = ((INT32)(voice->k1 >> 2) * (sample - voice->o2n1) / 16384) + voice->o2n1;			\
	voice->o2n2 = voice->o2n1;																	\
	voice->o2n1 = sample;																		\
																								\
	/* remaining poles depend on the current filter setting */									\
	switch (voice->control & CONTROL_LPMASK)													\
	{																							\
		case 0:																					\
			/* pole 3 is high-pass using K2 */													\
			sample = sample - voice->o2n2 + ((INT32)(voice->k2 >> 2) * voice->o3n1) / 32768 + voice->o3n1 / 2; \
			voice->o3n2 = voice->o3n1;															\
			voice->o3n1 = sample;																\
																								\
			/* pole 4 is high-pass using K2 */													\
			sample = sample - voice->o3n2 + ((INT32)(voice->k2 >> 2) * voice->o4n1) / 32768 + voice->o4n1 / 2; \
			voice->o4n1 = sample;																\
			break;																				\
																								\
		case CONTROL_LP3:																		\
			/* pole 3 is low-pass using K1 */													\
			sample = ((INT32)(voice->k1 >> 2) * (sample - voice->o3n1) / 16384) + voice->o3n1;	\
			voice->o3n2 = voice->o3n1;															\
			voice->o3n1 = sample;																\
																								\
			/* pole 4 is high-pass using K2 */													\
			sample = sample - voice->o3n2 + ((INT32)(voice->k2 >> 2) * voice->o4n1) / 32768 + voice->o4n1 / 2; \
			voice->o4n1 = sample;																\
			break;																				\
																								\
		case CONTROL_LP4:																		\
			/* pole 3 is low-pass using K2 */													\
			sample = ((INT32)(voice->k2 >> 2) * (sample - voice->o3n1) / 16384) + voice->o3n1;	\
			voice->o3n2 = voice->o3n1;															\
			voice->o3n1 = sample;																\
																								\
			/* pole 4 is low-pass using K2 */													\
			sample = ((INT32)(voice->k2 >> 2) * (sample - voice->o4n1) / 16384) + voice->o4n1;	\
			voice->o4n1 = sample;																\
			break;																				\
																								\
		case CONTROL_LP4 | CONTROL_LP3:															\
			/* pole 3 is low-pass using K1 */													\
			sample = ((INT32)(voice->k1 >> 2) * (sample - voice->o3n1) / 16384) + voice->o3n1;	\
			voice->o3n2 = voice->o3n1;															\
			voice->o3n1 = sample;																\
																								\
			/* pole 4 is low-pass using K2 */													\
			sample = ((INT32)(voice->k2 >> 2) * (sample - voice->o4n1) / 16384) + voice->o4n1;	\
			voice->o4n1 = sample;																\
			break;																				\
	}																							\
} while (0)



/**********************************************************************************************

     update_envelopes -- update the envelopes

***********************************************************************************************/

#define update_envelopes(voice, samples)											\
do																					\
{																					\
	INT32 count = (samples > 1 && samples > (INT32)voice->ecount) ? voice->ecount : samples;	\
																					\
	/* decrement the envelope counter */											\
	voice->ecount -= count;															\
																					\
	/* ramp left volume */															\
	if (voice->lvramp)																\
	{																				\
		voice->lvol += (INT8)voice->lvramp * count;									\
		if ((INT32)voice->lvol < 0) voice->lvol = 0;								\
		else if (voice->lvol > 0xffff) voice->lvol = 0xffff;						\
	}																				\
																					\
	/* ramp right volume */															\
	if (voice->rvramp)																\
	{																				\
		voice->rvol += (INT8)voice->rvramp * count;									\
		if ((INT32)voice->rvol < 0) voice->rvol = 0;								\
		else if (voice->rvol > 0xffff) voice->rvol = 0xffff;						\
	}																				\
																					\
	/* ramp k1 filter constant */													\
	if (voice->k1ramp && ((INT32)voice->k1ramp >= 0 || !(voice->filtcount & 7)))	\
	{																				\
		voice->k1 += (INT8)voice->k1ramp * count;									\
		if ((INT32)voice->k1 < 0) voice->k1 = 0;									\
		else if (voice->k1 > 0xffff) voice->k1 = 0xffff;							\
	}																				\
																					\
	/* ramp k2 filter constant */													\
	if (voice->k2ramp && ((INT32)voice->k2ramp >= 0 || !(voice->filtcount & 7)))	\
	{																				\
		voice->k2 += (INT8)voice->k2ramp * count;									\
		if ((INT32)voice->k2 < 0) voice->k2 = 0;									\
		else if (voice->k2 > 0xffff) voice->k2 = 0xffff;							\
	}																				\
																					\
	/* update the filter constant counter */										\
	voice->filtcount += count;														\
																					\
} while (0)



/**********************************************************************************************

     check_for_end_forward
     check_for_end_reverse -- check for loop end and loop appropriately

***********************************************************************************************/

#define check_for_end_forward(voice, accum)											\
do																					\
{																					\
	/* are we past the end? */														\
	if (accum > voice->end && !(voice->control & CONTROL_LEI))						\
	{																				\
		/* generate interrupt if required */										\
		if (voice->control&CONTROL_IRQE)											\
			voice->control |= CONTROL_IRQ;											\
																					\
		/* handle the different types of looping */									\
		switch (voice->control & CONTROL_LOOPMASK)									\
		{																			\
			/* non-looping */														\
			case 0:																	\
				voice->control |= CONTROL_STOP0;									\
				goto alldone;														\
																					\
			/* uni-directional looping */											\
			case CONTROL_LPE:														\
				accum = (voice->start + (accum - voice->end)) & voice->accum_mask;	\
				break;																\
																					\
			/* trans-wave looping */												\
			case CONTROL_BLE:														\
				accum = (voice->start + (accum - voice->end)) & voice->accum_mask;	\
				voice->control = (voice->control & ~CONTROL_LOOPMASK) | CONTROL_LEI;\
				break;																\
																					\
			/* bi-directional looping */											\
			case CONTROL_LPE | CONTROL_BLE:											\
				accum = (voice->end - (accum - voice->end)) & voice->accum_mask;	\
				voice->control ^= CONTROL_DIR;										\
				goto reverse;														\
		}																			\
	}																				\
} while (0)


#define check_for_end_reverse(voice, accum)											\
do																					\
{																					\
	/* are we past the end? */														\
	if (accum < voice->start && !(voice->control & CONTROL_LEI))					\
	{																				\
		/* generate interrupt if required */										\
		if (voice->control&CONTROL_IRQE)											\
			voice->control |= CONTROL_IRQ;											\
																					\
		/* handle the different types of looping */									\
		switch (voice->control & CONTROL_LOOPMASK)									\
		{																			\
			/* non-looping */														\
			case 0:																	\
				voice->control |= CONTROL_STOP0;									\
				goto alldone;														\
																					\
			/* uni-directional looping */											\
			case CONTROL_LPE:														\
				accum = (voice->end - (voice->start - accum)) & voice->accum_mask;	\
				break;																\
																					\
			/* trans-wave looping */												\
			case CONTROL_BLE:														\
				accum = (voice->end - (voice->start - accum)) & voice->accum_mask;	\
				voice->control = (voice->control & ~CONTROL_LOOPMASK) | CONTROL_LEI;\
				break;																\
																					\
			/* bi-directional looping */											\
			case CONTROL_LPE | CONTROL_BLE:											\
				accum = (voice->start + (voice->start - accum)) & voice->accum_mask;\
				voice->control ^= CONTROL_DIR;										\
				goto reverse;														\
		}																			\
	}																				\
} while (0)



/**********************************************************************************************

     generate_dummy -- generate nothing, just apply envelopes

***********************************************************************************************/

static void generate_dummy(es5506_voice *voice, UINT16 *, INT32 *, INT32 *, INT32 samples)
{
	UINT32 freqcount = voice->freqcount;
	UINT32 accum = voice->accum & voice->accum_mask;

	/* outer loop, in case we switch directions */
	while (samples > 0 && !(voice->control & CONTROL_STOPMASK))
	{
reverse:
		/* two cases: first case is forward direction */
		if (!(voice->control & CONTROL_DIR))
		{
			/* loop while we still have samples to generate */
			while (samples--)
			{
				/* fetch two samples */
				accum = (accum + freqcount) & voice->accum_mask;

				/* update filters/volumes */
				if (voice->ecount != 0)
					update_envelopes(voice, 1);

				/* check for loop end */
				check_for_end_forward(voice, accum);
			}
		}

		/* two cases: second case is backward direction */
		else
		{
			/* loop while we still have samples to generate */
			while (samples--)
			{
				/* fetch two samples */
				accum = (accum - freqcount) & voice->accum_mask;

				/* update filters/volumes */
				if (voice->ecount != 0)
					update_envelopes(voice, 1);

				/* check for loop end */
				check_for_end_reverse(voice, accum);
			}
		}
	}

	/* if we stopped, process any additional envelope */
alldone:
	voice->accum = accum;
	if (samples > 0)
		update_envelopes(voice, samples);
}



/**********************************************************************************************

     generate_ulaw -- general u-law decoding routine

***********************************************************************************************/

static void generate_ulaw(es5506_voice *voice, UINT16 *base, INT32 *lbuffer, INT32 *rbuffer, INT32 samples)
{
	UINT32 freqcount = voice->freqcount;
	UINT32 accum = voice->accum & voice->accum_mask;
	INT32 lvol = chip->volume_lookup[voice->lvol >> 4];
	INT32 rvol = chip->volume_lookup[voice->rvol >> 4];
	
	bprintf(PRINT_NORMAL, _T("ULAW\n"));

	/* pre-add the bank offset */
	base += voice->exbank;

	/* outer loop, in case we switch directions */
	while (samples > 0 && !(voice->control & CONTROL_STOPMASK))
	{
reverse:
		/* two cases: first case is forward direction */
		if (!(voice->control & CONTROL_DIR))
		{
			/* loop while we still have samples to generate */
			while (samples--)
			{
				/* fetch two samples */
				INT32 val1 = base[accum >> 11];
				INT32 val2 = base[((accum + (1 << 11)) & voice->accum_mask) >> 11];

				/* decompress u-law */
				val1 = chip->ulaw_lookup[val1 >> (16 - ULAW_MAXBITS)];
				val2 = chip->ulaw_lookup[val2 >> (16 - ULAW_MAXBITS)];

				/* interpolate */
				val1 = interpolate(val1, val2, accum);
				accum = (accum + freqcount) & voice->accum_mask;

				/* apply filters */
				apply_filters(voice, val1);

				/* update filters/volumes */
				if (voice->ecount != 0)
				{
					update_envelopes(voice, 1);
					lvol = chip->volume_lookup[voice->lvol >> 4];
					rvol = chip->volume_lookup[voice->rvol >> 4];
				}

				/* apply volumes and add */
				*lbuffer++ += (val1 * lvol) >> 11;
				*rbuffer++ += (val1 * rvol) >> 11;

				/* check for loop end */
				check_for_end_forward(voice, accum);
			}
		}

		/* two cases: second case is backward direction */
		else
		{
			/* loop while we still have samples to generate */
			while (samples--)
			{
				/* fetch two samples */
				INT32 val1 = base[accum >> 11];
				INT32 val2 = base[((accum + (1 << 11)) & voice->accum_mask) >> 11];

				/* decompress u-law */
				val1 = chip->ulaw_lookup[val1 >> (16 - ULAW_MAXBITS)];
				val2 = chip->ulaw_lookup[val2 >> (16 - ULAW_MAXBITS)];

				/* interpolate */
				val1 = interpolate(val1, val2, accum);
				accum = (accum - freqcount) & voice->accum_mask;

				/* apply filters */
				apply_filters(voice, val1);

				/* update filters/volumes */
				if (voice->ecount != 0)
				{
					update_envelopes(voice, 1);
					lvol = chip->volume_lookup[voice->lvol >> 4];
					rvol = chip->volume_lookup[voice->rvol >> 4];
				}

				/* apply volumes and add */
				*lbuffer++ += (val1 * lvol) >> 11;
				*rbuffer++ += (val1 * rvol) >> 11;

				/* check for loop end */
				check_for_end_reverse(voice, accum);
			}
		}
	}

	/* if we stopped, process any additional envelope */
alldone:
	voice->accum = accum;
	if (samples > 0)
		update_envelopes(voice, samples);
}



/**********************************************************************************************

     generate_pcm -- general PCM decoding routine

***********************************************************************************************/

static void generate_pcm(es5506_voice *voice, UINT16 *base, INT32 *lbuffer, INT32 *rbuffer, INT32 samples)
{
	UINT32 freqcount = voice->freqcount;
	UINT32 accum = voice->accum & voice->accum_mask;
	INT32 lvol = chip->volume_lookup[voice->lvol >> 4];
	INT32 rvol = chip->volume_lookup[voice->rvol >> 4];
	
//	bprintf(PRINT_NORMAL, _T("PCM, %x, %x, %x\n"), lvol, rvol, voice->control & CONTROL_STOPMASK);

	/* pre-add the bank offset */
	base += voice->exbank;

	/* outer loop, in case we switch directions */
	while (samples > 0 && !(voice->control & CONTROL_STOPMASK))
	{
reverse:
		/* two cases: first case is forward direction */
		if (!(voice->control & CONTROL_DIR))
		{
			/* loop while we still have samples to generate */
			while (samples--)
			{
				/* fetch two samples */
				INT32 val1 = (INT16)base[accum >> 11];
				INT32 val2 = (INT16)base[((accum + (1 << 11)) & voice->accum_mask) >> 11];
				
				/* interpolate */
				val1 = interpolate(val1, val2, accum);
				accum = (accum + freqcount) & voice->accum_mask;

				/* apply filters */
				apply_filters(voice, val1);

				/* update filters/volumes */
				if (voice->ecount != 0)
				{
					update_envelopes(voice, 1);
					lvol = chip->volume_lookup[voice->lvol >> 4];
					rvol = chip->volume_lookup[voice->rvol >> 4];
				}

				/* apply volumes and add */
				*lbuffer++ += (val1 * lvol) >> 11;
				*rbuffer++ += (val1 * rvol) >> 11;

				/* check for loop end */
				check_for_end_forward(voice, accum);
			}
		}

		/* two cases: second case is backward direction */
		else
		{
			/* loop while we still have samples to generate */
			while (samples--)
			{
				/* fetch two samples */
				INT32 val1 = (INT16)base[accum >> 11];
				INT32 val2 = (INT16)base[((accum + (1 << 11)) & voice->accum_mask) >> 11];
				
				bprintf(PRINT_NORMAL, _T("%x, %x\n"), val1, val2);

				/* interpolate */
				val1 = interpolate(val1, val2, accum);
				accum = (accum - freqcount) & voice->accum_mask;

				/* apply filters */
				apply_filters(voice, val1);

				/* update filters/volumes */
				if (voice->ecount != 0)
				{
					update_envelopes(voice, 1);
					lvol = chip->volume_lookup[voice->lvol >> 4];
					rvol = chip->volume_lookup[voice->rvol >> 4];
				}

				/* apply volumes and add */
				*lbuffer++ += (val1 * lvol) >> 11;
				*rbuffer++ += (val1 * rvol) >> 11;

				/* check for loop end */
				check_for_end_reverse(voice, accum);
			}
		}
	}

	/* if we stopped, process any additional envelope */
alldone:
	voice->accum = accum;
	if (samples > 0)
		update_envelopes(voice, samples);
}



/**********************************************************************************************

     generate_samples -- tell each voice to generate samples

***********************************************************************************************/

static void generate_samples(INT32 *left, INT32 *right, INT32 samples)
{
	INT32 v;

	/* skip if nothing to do */
	if (!samples)
		return;

	/* clear out the accumulator */
	memset(left, 0, samples * sizeof(left[0]));
	memset(right, 0, samples * sizeof(right[0]));
	
	/* loop over voices */
	for (v = 0; v <= chip->active_voices; v++)
	{
		es5506_voice *voice = &chip->voice[v];
		UINT16 *base = chip->region_base[voice->control >> 14];

		/* special case: if end == start, stop the voice */
		if (voice->start == voice->end)
			voice->control |= CONTROL_STOP0;

		/* generate from the appropriate source */
		if (!base)
		{
//			logerror("NULL region base %d\n",voice->control >> 14);
			generate_dummy(voice, base, left, right, samples);
		}
		else if (voice->control & 0x2000)
			generate_ulaw(voice, base, left, right, samples);
		else
			generate_pcm(voice, base, left, right, samples);

		/* does this voice have it's IRQ bit raised? */
		if (voice->control&CONTROL_IRQ)
		{
//logerror("IRQ raised on voice %d!!\n",v);
			/* only update voice vector if existing IRQ is acked by host */
			if (chip->irqv&0x80)
			{
				/* latch voice number into vector, and set high bit low */
				chip->irqv=v&0x7f;

				/* take down IRQ bit on voice */
				voice->control&=~CONTROL_IRQ;

				/* inform host of irq */
				update_irq_state();
			}
		}
	}
}



/**********************************************************************************************

     es5506_update -- update the sound chip so that it is in sync with CPU execution

***********************************************************************************************/

//static STREAM_UPDATE( es5506_update )
void ES5506Update(INT16 *pBuffer, INT32 samples)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ES5506Initted) bprintf(PRINT_ERROR, _T("ES5506Update called without init\n"));
#endif

//	es5506_state *chip = (es5506_state *)param;
	INT32 *lsrc = chip->scratch, *rsrc = chip->scratch;
//	short *ldest = &pBuffer[0];
//	short *rdest = &pBuffer[1];

#if MAKE_WAVS
	/* start the logging once we have a sample rate */
	if (chip->sample_rate)
	{
		if (!chip->wavraw)
			chip->wavraw = wav_open("raw.wav", chip->sample_rate, 2);
	}
#endif

	/* loop until all samples are output */
	while (samples)
	{
		INT32 length = (samples > MAX_SAMPLE_CHUNK) ? MAX_SAMPLE_CHUNK : samples;
		INT32 samp;
		
		/* determine left/right source data */
		lsrc = chip->scratch;
		rsrc = chip->scratch + length;
		generate_samples(lsrc, rsrc, length);
		
		/* copy the data */
		for (samp = 0; samp < length; samp++)
		{
//			*ldest++ = lsrc[samp] >> 4;
//			*rdest++ = rsrc[samp] >> 4;
			pBuffer[0] = lsrc[samp] >> 4;
			pBuffer[1] = rsrc[samp] >> 4;
			pBuffer += 2;
//			bprintf(PRINT_NORMAL, _T("%x, %x\n"), lsrc[samp] >> 4, rsrc[samp] >> 4);
		}

#if MAKE_WAVS
		/* log the raw data */
		if (chip->wavraw)
			wav_add_data_32lr(chip->wavraw, lsrc, rsrc, length, 4);
#endif

		/* account for these samples */
		samples -= length;
	}
}


/**********************************************************************************************

     DEVICE_START( es5506 ) -- start emulation of the ES5506

***********************************************************************************************/

static void es5506_start_common(INT32 clock, UINT8* region0, UINT8* region1, UINT8* region2, UINT8* region3, irq_callback callback, INT32 sndtype)
{
	DebugSnd_ES5506Initted = 1;

//	const es5506_interface *intf = (const es5506_interface *)config;
//	es5506_state *chip = get_safe_token(device);
	INT32 j;
	UINT32 accum_mask;

	/* debugging */
	if (LOG_COMMANDS && !eslog)
		eslog = fopen("es.log", "w");

	/* create the stream */
//	chip->stream = device->machine->sound().stream_alloc(*device, 0, 2, device->clock() / (16*32), chip, es5506_update);

	chip = (struct _es5506_state*)malloc(sizeof(_es5506_state));
	memset(chip, 0, sizeof(_es5506_state));

	/* initialize the regions */
	chip->region_base[0] = region0 ? (UINT16 *)region0 : NULL;
	chip->region_base[1] = region1 ? (UINT16 *)region1 : NULL;
	chip->region_base[2] = region2 ? (UINT16 *)region2 : NULL;
	chip->region_base[3] = region3 ? (UINT16 *)region3 : NULL;

	/* initialize the rest of the structure */
	chip->master_clock = clock;
	chip->irq_callback = callback;
	chip->irqv = 0x80;

	/* compute the tables */
	compute_tables();

	/* init the voices */
	accum_mask = (sndtype == ES5506) ? 0xffffffff : 0x7fffffff;
	for (j = 0; j < 32; j++)
	{
		chip->voice[j].index = j;
		chip->voice[j].control = CONTROL_STOPMASK;
		chip->voice[j].lvol = 0xffff;
		chip->voice[j].rvol = 0xffff;
		chip->voice[j].exbank = 0;
		chip->voice[j].accum_mask = accum_mask;
	}

	/* allocate memory */
	chip->scratch = (INT32*)malloc(2 * MAX_SAMPLE_CHUNK * sizeof(INT32));//auto_alloc_array(device->machine, INT32, 2 * MAX_SAMPLE_CHUNK);

	/* register save */
/*	device->save_item(NAME(chip->sample_rate));
	device->save_item(NAME(chip->write_latch));
	device->save_item(NAME(chip->read_latch));

	device->save_item(NAME(chip->current_page));
	device->save_item(NAME(chip->active_voices));
	device->save_item(NAME(chip->mode));
	device->save_item(NAME(chip->wst));
	device->save_item(NAME(chip->wend));
	device->save_item(NAME(chip->lrend));
	device->save_item(NAME(chip->irqv));

	device->save_pointer(NAME(chip->scratch), 2 * MAX_SAMPLE_CHUNK);

	for (j = 0; j < 32; j++)
	{
		device->save_item(NAME(chip->voice[j].control), j);
		device->save_item(NAME(chip->voice[j].freqcount), j);
		device->save_item(NAME(chip->voice[j].start), j);
		device->save_item(NAME(chip->voice[j].lvol), j);
		device->save_item(NAME(chip->voice[j].end), j);
		device->save_item(NAME(chip->voice[j].lvramp), j);
		device->save_item(NAME(chip->voice[j].accum), j);
		device->save_item(NAME(chip->voice[j].rvol), j);
		device->save_item(NAME(chip->voice[j].rvramp), j);
		device->save_item(NAME(chip->voice[j].ecount), j);
		device->save_item(NAME(chip->voice[j].k2), j);
		device->save_item(NAME(chip->voice[j].k2ramp), j);
		device->save_item(NAME(chip->voice[j].k1), j);
		device->save_item(NAME(chip->voice[j].k1ramp), j);
		device->save_item(NAME(chip->voice[j].o4n1), j);
		device->save_item(NAME(chip->voice[j].o3n1), j);
		device->save_item(NAME(chip->voice[j].o3n2), j);
		device->save_item(NAME(chip->voice[j].o2n1), j);
		device->save_item(NAME(chip->voice[j].o2n2), j);
		device->save_item(NAME(chip->voice[j].o1n1), j);
		device->save_item(NAME(chip->voice[j].exbank), j);
		device->save_item(NAME(chip->voice[j].filtcount), j);
	}*/

	/* success */
}


/*static DEVICE_START( es5506 )
{
	es5506_start_common(device, device->baseconfig().static_config(), ES5506);
}*/

void ES5506Init(INT32 clock, UINT8* region0, UINT8* region1, UINT8* region2, UINT8* region3, irq_callback callback)
{
	es5506_start_common(clock, region0, region1, region2, region3, callback, ES5506);
}



/**********************************************************************************************

     DEVICE_STOP( es5506 ) -- stop emulation of the ES5506

***********************************************************************************************/

void ES5506Exit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_ES5506Initted) bprintf(PRINT_ERROR, _T("ES5506Exit called without init\n"));
#endif

	/* debugging */
	if (LOG_COMMANDS && eslog)
	{
		fclose(eslog);
		eslog = NULL;
	}

#if MAKE_WAVS
{
	INT32 i;

	for (i = 0; i < MAX_ES5506; i++)
	{
		if (es5506[i].wavraw)
			wav_close(es5506[i].wavraw);
	}
}
#endif

	DebugSnd_ES5506Initted = 0;
}


void ES5506Reset()
{
#if defined FBA_DEBUG
	if (!DebugSnd_ES5506Initted) bprintf(PRINT_ERROR, _T("ES5506Reset called without init\n"));
#endif
}



/**********************************************************************************************

     es5506_reg_write -- handle a write to the selected ES5506 register

***********************************************************************************************/

ES5506_INLINE void es5506_reg_write_low(es5506_voice *voice, UINT32 offset, UINT32 data)
{
	switch (offset)
	{
		case 0x00/8:	/* CR */
			voice->control = data & 0xffff;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, control=%04x\n", chip->current_page & 0x1f, voice->control);
			break;

		case 0x08/8:	/* FC */
			voice->freqcount = data & 0x1ffff;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, freq count=%08x\n", chip->current_page & 0x1f, voice->freqcount);
			break;

		case 0x10/8:	/* LVOL */
			voice->lvol = data & 0xffff;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, left vol=%04x\n", chip->current_page & 0x1f, voice->lvol);
			break;

		case 0x18/8:	/* LVRAMP */
			voice->lvramp = (data & 0xff00) >> 8;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, left vol ramp=%04x\n", chip->current_page & 0x1f, voice->lvramp);
			break;

		case 0x20/8:	/* RVOL */
			voice->rvol = data & 0xffff;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, right vol=%04x\n", chip->current_page & 0x1f, voice->rvol);
			break;

		case 0x28/8:	/* RVRAMP */
			voice->rvramp = (data & 0xff00) >> 8;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, right vol ramp=%04x\n", chip->current_page & 0x1f, voice->rvramp);
			break;

		case 0x30/8:	/* ECOUNT */
			voice->ecount = data & 0x1ff;
			voice->filtcount = 0;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, envelope count=%04x\n", chip->current_page & 0x1f, voice->ecount);
			break;

		case 0x38/8:	/* K2 */
			voice->k2 = data & 0xffff;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, K2=%04x\n", chip->current_page & 0x1f, voice->k2);
			break;

		case 0x40/8:	/* K2RAMP */
			voice->k2ramp = ((data & 0xff00) >> 8) | ((data & 0x0001) << 31);
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, K2 ramp=%04x\n", chip->current_page & 0x1f, voice->k2ramp);
			break;

		case 0x48/8:	/* K1 */
			voice->k1 = data & 0xffff;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, K1=%04x\n", chip->current_page & 0x1f, voice->k1);
			break;

		case 0x50/8:	/* K1RAMP */
			voice->k1ramp = ((data & 0xff00) >> 8) | ((data & 0x0001) << 31);
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, K1 ramp=%04x\n", chip->current_page & 0x1f, voice->k1ramp);
			break;

		case 0x58/8:	/* ACTV */
		{
			chip->active_voices = data & 0x1f;
			chip->sample_rate = chip->master_clock / (16 * (chip->active_voices + 1));
//			chip->stream->set_sample_rate(chip->sample_rate);

			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "active voices=%d, sample_rate=%d\n", chip->active_voices, chip->sample_rate);
			break;
		}

		case 0x60/8:	/* MODE */
			chip->mode = data & 0x1f;
			break;

		case 0x68/8:	/* PAR - read only */
		case 0x70/8:	/* IRQV - read only */
			break;

		case 0x78/8:	/* PAGE */
			chip->current_page = data & 0x7f;
			break;
	}
}


ES5506_INLINE void es5506_reg_write_high(es5506_voice *voice, UINT32 offset, UINT32 data)
{
	switch (offset)
	{
		case 0x00/8:	/* CR */
			voice->control = data & 0xffff;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, control=%04x\n", chip->current_page & 0x1f, voice->control);
			break;

		case 0x08/8:	/* START */
			voice->start = data & 0xfffff800;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, loop start=%08x\n", chip->current_page & 0x1f, voice->start);
			break;

		case 0x10/8:	/* END */
			voice->end = data & 0xffffff80;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, loop end=%08x\n", chip->current_page & 0x1f, voice->end);
			break;

		case 0x18/8:	/* ACCUM */
			voice->accum = data;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, accum=%08x\n", chip->current_page & 0x1f, voice->accum);
			break;

		case 0x20/8:	/* O4(n-1) */
			voice->o4n1 = (INT32)(data << 14) >> 14;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, O4(n-1)=%05x\n", chip->current_page & 0x1f, voice->o4n1 & 0x3ffff);
			break;

		case 0x28/8:	/* O3(n-1) */
			voice->o3n1 = (INT32)(data << 14) >> 14;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, O3(n-1)=%05x\n", chip->current_page & 0x1f, voice->o3n1 & 0x3ffff);
			break;

		case 0x30/8:	/* O3(n-2) */
			voice->o3n2 = (INT32)(data << 14) >> 14;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, O3(n-2)=%05x\n", chip->current_page & 0x1f, voice->o3n2 & 0x3ffff);
			break;

		case 0x38/8:	/* O2(n-1) */
			voice->o2n1 = (INT32)(data << 14) >> 14;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, O2(n-1)=%05x\n", chip->current_page & 0x1f, voice->o2n1 & 0x3ffff);
			break;

		case 0x40/8:	/* O2(n-2) */
			voice->o2n2 = (INT32)(data << 14) >> 14;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, O2(n-2)=%05x\n", chip->current_page & 0x1f, voice->o2n2 & 0x3ffff);
			break;

		case 0x48/8:	/* O1(n-1) */
			voice->o1n1 = (INT32)(data << 14) >> 14;
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "voice %d, O1(n-1)=%05x\n", chip->current_page & 0x1f, voice->o1n1 & 0x3ffff);
			break;

		case 0x50/8:	/* W_ST */
			chip->wst = data & 0x7f;
			break;

		case 0x58/8:	/* W_END */
			chip->wend = data & 0x7f;
			break;

		case 0x60/8:	/* LR_END */
			chip->lrend = data & 0x7f;
			break;

		case 0x68/8:	/* PAR - read only */
		case 0x70/8:	/* IRQV - read only */
			break;

		case 0x78/8:	/* PAGE */
			chip->current_page = data & 0x7f;
			break;
	}
}

ES5506_INLINE void es5506_reg_write_test(UINT32 offset, UINT32 data)
{
	switch (offset)
	{
		case 0x00/8:	/* CHANNEL 0 LEFT */
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "Channel 0 left test write %08x\n", data);
			break;

		case 0x08/8:	/* CHANNEL 0 RIGHT */
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "Channel 0 right test write %08x\n", data);
			break;

		case 0x10/8:	/* CHANNEL 1 LEFT */
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "Channel 1 left test write %08x\n", data);
			break;

		case 0x18/8:	/* CHANNEL 1 RIGHT */
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "Channel 1 right test write %08x\n", data);
			break;

		case 0x20/8:	/* CHANNEL 2 LEFT */
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "Channel 2 left test write %08x\n", data);
			break;

		case 0x28/8:	/* CHANNEL 2 RIGHT */
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "Channel 2 right test write %08x\n", data);
			break;

		case 0x30/8:	/* CHANNEL 3 LEFT */
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "Channel 3 left test write %08x\n", data);
			break;

		case 0x38/8:	/* CHANNEL 3 RIGHT */
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "Channel 3 right test write %08x\n", data);
			break;

		case 0x40/8:	/* CHANNEL 4 LEFT */
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "Channel 4 left test write %08x\n", data);
			break;

		case 0x48/8:	/* CHANNEL 4 RIGHT */
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "Channel 4 right test write %08x\n", data);
			break;

		case 0x50/8:	/* CHANNEL 5 LEFT */
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "Channel 5 left test write %08x\n", data);
			break;

		case 0x58/8:	/* CHANNEL 6 RIGHT */
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "Channel 5 right test write %08x\n", data);
			break;

		case 0x60/8:	/* EMPTY */
			if (LOG_COMMANDS && eslog)
				fprintf(eslog, "Test write EMPTY %08x\n", data);
			break;

		case 0x68/8:	/* PAR - read only */
		case 0x70/8:	/* IRQV - read only */
			break;

		case 0x78/8:	/* PAGE */
			chip->current_page = data & 0x7f;
			break;
	}
}

//WRITE8_DEVICE_HANDLER( es5506_w )
void ES5506Write(UINT32 offset, UINT8 data)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ES5506Initted) bprintf(PRINT_ERROR, _T("ES5506Write called without init\n"));
#endif

//	es5506_state *chip = get_safe_token(device);
	es5506_voice *voice = &chip->voice[chip->current_page & 0x1f];
	INT32 shift = 8 * (offset & 3);

	/* accumulate the data */
	chip->write_latch = (chip->write_latch & ~(0xff000000 >> shift)) | (data << (24 - shift));

	/* wait for a write to complete */
	if (shift != 24)
		return;

	/* force an update */
//	chip->stream->update();

	/* switch off the page and register */
	if (chip->current_page < 0x20)
		es5506_reg_write_low(voice, offset / 4, chip->write_latch);
	else if (chip->current_page < 0x40)
		es5506_reg_write_high(voice, offset / 4, chip->write_latch);
	else
		es5506_reg_write_test(offset / 4, chip->write_latch);

	/* clear the write latch when done */
	chip->write_latch = 0;
}



/**********************************************************************************************

     es5506_reg_read -- read from the specified ES5506 register

***********************************************************************************************/

ES5506_INLINE UINT32 es5506_reg_read_low(es5506_voice *voice, UINT32 offset)
{
	UINT32 result = 0;

	switch (offset)
	{
		case 0x00/8:	/* CR */
			result = voice->control;
			break;

		case 0x08/8:	/* FC */
			result = voice->freqcount;
			break;

		case 0x10/8:	/* LVOL */
			result = voice->lvol;
			break;

		case 0x18/8:	/* LVRAMP */
			result = voice->lvramp << 8;
			break;

		case 0x20/8:	/* RVOL */
			result = voice->rvol;
			break;

		case 0x28/8:	/* RVRAMP */
			result = voice->rvramp << 8;
			break;

		case 0x30/8:	/* ECOUNT */
			result = voice->ecount;
			break;

		case 0x38/8:	/* K2 */
			result = voice->k2;
			break;

		case 0x40/8:	/* K2RAMP */
			result = (voice->k2ramp << 8) | (voice->k2ramp >> 31);
			break;

		case 0x48/8:	/* K1 */
			result = voice->k1;
			break;

		case 0x50/8:	/* K1RAMP */
			result = (voice->k1ramp << 8) | (voice->k1ramp >> 31);
			break;

		case 0x58/8:	/* ACTV */
			result = chip->active_voices;
			break;

		case 0x60/8:	/* MODE */
			result = chip->mode;
			break;

		case 0x68/8:	/* PAR */
			if (chip->port_read)
				result = (*chip->port_read)();
			break;

		case 0x70/8:	/* IRQV */
			result = chip->irqv;
			update_internal_irq_state();
			break;

		case 0x78/8:	/* PAGE */
			result = chip->current_page;
			break;
	}
	return result;
}


ES5506_INLINE UINT32 es5506_reg_read_high(es5506_voice *voice, UINT32 offset)
{
	UINT32 result = 0;

	switch (offset)
	{
		case 0x00/8:	/* CR */
			result = voice->control;
			break;

		case 0x08/8:	/* START */
			result = voice->start;
			break;

		case 0x10/8:	/* END */
			result = voice->end;
			break;

		case 0x18/8:	/* ACCUM */
			result = voice->accum;
			break;

		case 0x20/8:	/* O4(n-1) */
			result = voice->o4n1 & 0x3ffff;
			break;

		case 0x28/8:	/* O3(n-1) */
			result = voice->o3n1 & 0x3ffff;
			break;

		case 0x30/8:	/* O3(n-2) */
			result = voice->o3n2 & 0x3ffff;
			break;

		case 0x38/8:	/* O2(n-1) */
			result = voice->o2n1 & 0x3ffff;
			break;

		case 0x40/8:	/* O2(n-2) */
			result = voice->o2n2 & 0x3ffff;
			break;

		case 0x48/8:	/* O1(n-1) */
			result = voice->o1n1 & 0x3ffff;
			break;

		case 0x50/8:	/* W_ST */
			result = chip->wst;
			break;

		case 0x58/8:	/* W_END */
			result = chip->wend;
			break;

		case 0x60/8:	/* LR_END */
			result = chip->lrend;
			break;

		case 0x68/8:	/* PAR */
			if (chip->port_read)
				result = (*chip->port_read)();
			break;

		case 0x70/8:	/* IRQV */
			result = chip->irqv;
			update_internal_irq_state();
			break;

		case 0x78/8:	/* PAGE */
			result = chip->current_page;
			break;
	}
	return result;
}

ES5506_INLINE UINT32 es5506_reg_read_test(UINT32 offset)
{
	UINT32 result = 0;

	switch (offset)
	{
		case 0x68/8:	/* PAR */
			if (chip->port_read)
				result = (*chip->port_read)();
			break;

		case 0x70/8:	/* IRQV */
			result = chip->irqv;
			break;

		case 0x78/8:	/* PAGE */
			result = chip->current_page;
			break;
	}
	return result;
}

//READ8_DEVICE_HANDLER( es5506_r )
UINT8 ES5506Read(UINT32 offset)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ES5506Initted) bprintf(PRINT_ERROR, _T("ES5506Read called without init\n"));
#endif

//	es5506_state *chip = get_safe_token(device);
	es5506_voice *voice = &chip->voice[chip->current_page & 0x1f];
	INT32 shift = 8 * (offset & 3);

	/* only read on offset 0 */
	if (shift != 0)
		return chip->read_latch >> (24 - shift);

	if (LOG_COMMANDS && eslog)
		fprintf(eslog, "read from %02x/%02x -> ", chip->current_page, offset / 4 * 8);

	/* force an update */
//	chip->stream->update();

	/* switch off the page and register */
	if (chip->current_page < 0x20)
		chip->read_latch = es5506_reg_read_low(voice, offset / 4);
	else if (chip->current_page < 0x40)
		chip->read_latch = es5506_reg_read_high(voice, offset / 4);
	else
		chip->read_latch = es5506_reg_read_test(offset / 4);

	if (LOG_COMMANDS && eslog)
		fprintf(eslog, "%08x\n", chip->read_latch);

	/* return the high byte */
	return chip->read_latch >> 24;
}



void es5506_voice_bank_w(INT32 voice, INT32 bank)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ES5506Initted) bprintf(PRINT_ERROR, _T("es5506_voice_bank_w called without init\n"));
#endif

//	es5506_state *chip = get_safe_token(device);
	chip->voice[voice].exbank=bank;
}


/**********************************************************************************************

     DEVICE_START( es5505 ) -- start emulation of the ES5505

***********************************************************************************************/

//static DEVICE_START( es5505 )
void ES5505Init(INT32 clock, UINT8* region0, UINT8* region1, irq_callback callback)
{
	es5506_start_common(clock, region0, region1, NULL, NULL, callback, ES5505);
}



/**********************************************************************************************

     DEVICE_STOP( es5505 ) -- stop emulation of the ES5505

***********************************************************************************************/

/*static DEVICE_STOP( es5505 )
{
	DEVICE_STOP_CALL( es5506 );
}


static DEVICE_RESET( es5505 )
{
	DEVICE_RESET_CALL( es5506 );
}*/



/**********************************************************************************************

     es5505_reg_write -- handle a write to the selected ES5505 register

***********************************************************************************************/

ES5506_INLINE void es5505_reg_write_low(es5506_voice *voice, UINT32 offset, UINT16 data)
{
//	running_machine *machine = chip->device->machine;

	if (offset == 0) bprintf(PRINT_NORMAL, _T("low data %x\n"), data);

	switch (offset)
	{
		case 0x00:	/* CR */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
			{
#if RAINE_CHECK
				voice->control &= ~(CONTROL_STOPMASK | CONTROL_LOOPMASK | CONTROL_DIR);
#else
				voice->control &= ~(CONTROL_STOPMASK | CONTROL_BS0 | CONTROL_LOOPMASK | CONTROL_IRQE | CONTROL_DIR | CONTROL_IRQ);
#endif
				voice->control |= (data & (CONTROL_STOPMASK | CONTROL_LOOPMASK | CONTROL_IRQE | CONTROL_DIR | CONTROL_IRQ)) |
								  ((data << 12) & CONTROL_BS0);
			}
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
			{
				voice->control &= ~(CONTROL_CA0 | CONTROL_CA1 | CONTROL_LPMASK);
				voice->control |= ((data >> 2) & CONTROL_LPMASK) |
								  ((data << 2) & (CONTROL_CA0 | CONTROL_CA1));
			}

//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, control=%04x (raw=%04x & %04x)\n", machine->describe_context(), chip->current_page & 0x1f, voice->control, data, mem_mask ^ 0xffff);
			break;

		case 0x01:	/* FC */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
				voice->freqcount = (voice->freqcount & ~0x001fe) | ((data & 0x00ff) << 1);
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
				voice->freqcount = (voice->freqcount & ~0x1fe00) | ((data & 0xff00) << 1);
//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, freq count=%08x\n", machine->describe_context(), chip->current_page & 0x1f, voice->freqcount);
			break;

		case 0x02:	/* STRT (hi) */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
				voice->start = (voice->start & ~0x03fc0000) | ((data & 0x00ff) << 18);
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
				voice->start = (voice->start & ~0x7c000000) | ((data & 0x1f00) << 18);
//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, loop start=%08x\n", machine->describe_context(), chip->current_page & 0x1f, voice->start);
			break;

		case 0x03:	/* STRT (lo) */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
				voice->start = (voice->start & ~0x00000380) | ((data & 0x00e0) << 2);
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
				voice->start = (voice->start & ~0x0003fc00) | ((data & 0xff00) << 2);
//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, loop start=%08x\n", machine->describe_context(), chip->current_page & 0x1f, voice->start);
			break;

		case 0x04:	/* END (hi) */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
				voice->end = (voice->end & ~0x03fc0000) | ((data & 0x00ff) << 18);
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
				voice->end = (voice->end & ~0x7c000000) | ((data & 0x1f00) << 18);
#if RAINE_CHECK
			voice->control |= CONTROL_STOP0;
#endif
//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, loop end=%08x\n", machine->describe_context(), chip->current_page & 0x1f, voice->end);
			break;

		case 0x05:	/* END (lo) */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
				voice->end = (voice->end & ~0x00000380) | ((data & 0x00e0) << 2);
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
				voice->end = (voice->end & ~0x0003fc00) | ((data & 0xff00) << 2);
#if RAINE_CHECK
			voice->control |= CONTROL_STOP0;
#endif
//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, loop end=%08x\n", machine->describe_context(), chip->current_page & 0x1f, voice->end);
			break;

		case 0x06:	/* K2 */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
				voice->k2 = (voice->k2 & ~0x00f0) | (data & 0x00f0);
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
				voice->k2 = (voice->k2 & ~0xff00) | (data & 0xff00);
//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, K2=%04x\n", machine->describe_context(), chip->current_page & 0x1f, voice->k2);
			break;

		case 0x07:	/* K1 */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
				voice->k1 = (voice->k1 & ~0x00f0) | (data & 0x00f0);
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
				voice->k1 = (voice->k1 & ~0xff00) | (data & 0xff00);
//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, K1=%04x\n", machine->describe_context(), chip->current_page & 0x1f, voice->k1);
			break;

		case 0x08:	/* LVOL */
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
				voice->lvol = (voice->lvol & ~0xff00) | (data & 0xff00);
//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, left vol=%04x\n", machine->describe_context(), chip->current_page & 0x1f, voice->lvol);
			break;

		case 0x09:	/* RVOL */
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
				voice->rvol = (voice->rvol & ~0xff00) | (data & 0xff00);
//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, right vol=%04x\n", machine->describe_context(), chip->current_page & 0x1f, voice->rvol);
			break;

		case 0x0a:	/* ACC (hi) */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
				voice->accum = (voice->accum & ~0x03fc0000) | ((data & 0x00ff) << 18);
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
				voice->accum = (voice->accum & ~0x7c000000) | ((data & 0x1f00) << 18);
//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, accum=%08x\n", machine->describe_context(), chip->current_page & 0x1f, voice->accum);
			break;

		case 0x0b:	/* ACC (lo) */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
				voice->accum = (voice->accum & ~0x000003fc) | ((data & 0x00ff) << 2);
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
				voice->accum = (voice->accum & ~0x0003fc00) | ((data & 0xff00) << 2);
//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, accum=%08x\n", machine->describe_context(), chip->current_page & 0x1f, voice->accum);
			break;

		case 0x0c:	/* unused */
			break;

		case 0x0d:	/* ACT */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
			{
				chip->active_voices = data & 0x1f;
				chip->sample_rate = chip->master_clock / (16 * (chip->active_voices + 1));
//				chip->stream->set_sample_rate(chip->sample_rate);

//				if (LOG_COMMANDS && eslog)
//					fprintf(eslog, "active voices=%d, sample_rate=%d\n", chip->active_voices, chip->sample_rate);
			}
			break;

		case 0x0e:	/* IRQV - read only */
			break;

		case 0x0f:	/* PAGE */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
				chip->current_page = data & 0x7f;
			break;
	}
}


ES5506_INLINE void es5505_reg_write_high(es5506_voice *voice, UINT32 offset, UINT16 data)
{
//	running_machine *machine = chip->device->machine;

	if (offset == 0) bprintf(PRINT_NORMAL, _T("high data %x\n"), data);

	switch (offset)
	{
		case 0x00:	/* CR */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
			{
				voice->control &= ~(CONTROL_STOPMASK | CONTROL_BS0 | CONTROL_LOOPMASK | CONTROL_IRQE | CONTROL_DIR | CONTROL_IRQ);
				voice->control |= (data & (CONTROL_STOPMASK | CONTROL_LOOPMASK | CONTROL_IRQE | CONTROL_DIR | CONTROL_IRQ)) |
								  ((data << 12) & CONTROL_BS0);
			}
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
			{
				voice->control &= ~(CONTROL_CA0 | CONTROL_CA1 | CONTROL_LPMASK);
				voice->control |= ((data >> 2) & CONTROL_LPMASK) |
								  ((data << 2) & (CONTROL_CA0 | CONTROL_CA1));
			}
//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, control=%04x (raw=%04x & %04x)\n", machine->describe_context(), chip->current_page & 0x1f, voice->control, data, mem_mask);
			break;

		case 0x01:	/* O4(n-1) */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
				voice->o4n1 = (voice->o4n1 & ~0x00ff) | (data & 0x00ff);
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
				voice->o4n1 = (INT16)((voice->o4n1 & ~0xff00) | (data & 0xff00));
//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, O4(n-1)=%05x\n", machine->describe_context(), chip->current_page & 0x1f, voice->o4n1 & 0x3ffff);
			break;

		case 0x02:	/* O3(n-1) */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
				voice->o3n1 = (voice->o3n1 & ~0x00ff) | (data & 0x00ff);
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
				voice->o3n1 = (INT16)((voice->o3n1 & ~0xff00) | (data & 0xff00));
//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, O3(n-1)=%05x\n", machine->describe_context(), chip->current_page & 0x1f, voice->o3n1 & 0x3ffff);
			break;

		case 0x03:	/* O3(n-2) */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
				voice->o3n2 = (voice->o3n2 & ~0x00ff) | (data & 0x00ff);
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
				voice->o3n2 = (INT16)((voice->o3n2 & ~0xff00) | (data & 0xff00));
//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, O3(n-2)=%05x\n", machine->describe_context(), chip->current_page & 0x1f, voice->o3n2 & 0x3ffff);
			break;

		case 0x04:	/* O2(n-1) */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
				voice->o2n1 = (voice->o2n1 & ~0x00ff) | (data & 0x00ff);
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
				voice->o2n1 = (INT16)((voice->o2n1 & ~0xff00) | (data & 0xff00));
//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, O2(n-1)=%05x\n", machine->describe_context(), chip->current_page & 0x1f, voice->o2n1 & 0x3ffff);
			break;

		case 0x05:	/* O2(n-2) */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
				voice->o2n2 = (voice->o2n2 & ~0x00ff) | (data & 0x00ff);
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
				voice->o2n2 = (INT16)((voice->o2n2 & ~0xff00) | (data & 0xff00));
//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, O2(n-2)=%05x\n", machine->describe_context(), chip->current_page & 0x1f, voice->o2n2 & 0x3ffff);
			break;

		case 0x06:	/* O1(n-1) */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
				voice->o1n1 = (voice->o1n1 & ~0x00ff) | (data & 0x00ff);
//			if (ACCESSING_BITS_8_15)
			if (data & 0xff00)
				voice->o1n1 = (INT16)((voice->o1n1 & ~0xff00) | (data & 0xff00));
//			if (LOG_COMMANDS && eslog)
//				fprintf(eslog, "%s:voice %d, O1(n-1)=%05x (accum=%08x)\n", machine->describe_context(), chip->current_page & 0x1f, voice->o2n1 & 0x3ffff, voice->accum);
			break;

		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:	/* unused */
			break;

		case 0x0d:	/* ACT */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
			{
				chip->active_voices = data & 0x1f;
				chip->sample_rate = chip->master_clock / (16 * (chip->active_voices + 1));
//				chip->stream->set_sample_rate(chip->sample_rate);

//				if (LOG_COMMANDS && eslog)
//					fprintf(eslog, "active voices=%d, sample_rate=%d\n", chip->active_voices, chip->sample_rate);
			}
			break;

		case 0x0e:	/* IRQV - read only */
			break;

		case 0x0f:	/* PAGE */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
				chip->current_page = data & 0x7f;
			break;
	}
}


ES5506_INLINE void es5505_reg_write_test(UINT32 offset, UINT16 data)
{
	switch (offset)
	{
		case 0x00:	/* CH0L */
		case 0x01:	/* CH0R */
		case 0x02:	/* CH1L */
		case 0x03:	/* CH1R */
		case 0x04:	/* CH2L */
		case 0x05:	/* CH2R */
		case 0x06:	/* CH3L */
		case 0x07:	/* CH3R */
			break;

		case 0x08:	/* SERMODE */
			chip->mode = data & 0x0007;
			break;

		case 0x09:	/* PAR */
			break;

		case 0x0d:	/* ACT */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
			{
				chip->active_voices = data & 0x1f;
				chip->sample_rate = chip->master_clock / (16 * (chip->active_voices + 1));
//				chip->stream->set_sample_rate(chip->sample_rate);

//				if (LOG_COMMANDS && eslog)
//					fprintf(eslog, "active voices=%d, sample_rate=%d\n", chip->active_voices, chip->sample_rate);
			}
			break;

		case 0x0e:	/* IRQV - read only */
			break;

		case 0x0f:	/* PAGE */
//			if (ACCESSING_BITS_0_7)
			if (data & 0xff)
				chip->current_page = data & 0x7f;
			break;
	}
}


//WRITE16_DEVICE_HANDLER( es5505_w )
void ES5505Write(UINT32 offset, UINT16 data)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ES5506Initted) bprintf(PRINT_ERROR, _T("ES5505Write called without init\n"));
#endif

//	es5506_state *chip = get_safe_token(device);
	es5506_voice *voice = &chip->voice[chip->current_page & 0x1f];

//  logerror("%s:ES5505 write %02x/%02x = %04x & %04x\n", machine->describe_context(), chip->current_page, offset, data, mem_mask);

	/* force an update */
//	chip->stream->update();

	/* switch off the page and register */
	if (chip->current_page < 0x20)
		es5505_reg_write_low(voice, offset, data);
	else if (chip->current_page < 0x40)
		es5505_reg_write_high(voice, offset, data);
	else
		es5505_reg_write_test(offset, data);
}



/**********************************************************************************************

     es5505_reg_read -- read from the specified ES5505 register

***********************************************************************************************/

ES5506_INLINE UINT16 es5505_reg_read_low(es5506_voice *voice, UINT32 offset)
{
	UINT16 result = 0;

	switch (offset)
	{
		case 0x00:	/* CR */
			result = (voice->control & (CONTROL_STOPMASK | CONTROL_LOOPMASK | CONTROL_IRQE | CONTROL_DIR | CONTROL_IRQ)) |
					 ((voice->control & CONTROL_BS0) >> 12) |
					 ((voice->control & CONTROL_LPMASK) << 2) |
					 ((voice->control & (CONTROL_CA0 | CONTROL_CA1)) >> 2) |
					 0xf000;
			break;

		case 0x01:	/* FC */
			result = voice->freqcount >> 1;
			break;

		case 0x02:	/* STRT (hi) */
			result = voice->start >> 18;
			break;

		case 0x03:	/* STRT (lo) */
			result = voice->start >> 2;
			break;

		case 0x04:	/* END (hi) */
			result = voice->end >> 18;
			break;

		case 0x05:	/* END (lo) */
			result = voice->end >> 2;
			break;

		case 0x06:	/* K2 */
			result = voice->k2;
			break;

		case 0x07:	/* K1 */
			result = voice->k1;
			break;

		case 0x08:	/* LVOL */
			result = voice->lvol;
			break;

		case 0x09:	/* RVOL */
			result = voice->rvol;
			break;

		case 0x0a:	/* ACC (hi) */
			result = voice->accum >> 18;
			break;

		case 0x0b:	/* ACC (lo) */
			result = voice->accum >> 2;
			break;

		case 0x0c:	/* unused */
			break;

		case 0x0d:	/* ACT */
			result = chip->active_voices;
			break;

		case 0x0e:	/* IRQV */
			result = chip->irqv;
			update_internal_irq_state();
			break;

		case 0x0f:	/* PAGE */
			result = chip->current_page;
			break;
	}
	return result;
}


ES5506_INLINE UINT16 es5505_reg_read_high(es5506_voice *voice, UINT32 offset)
{
	UINT16 result = 0;

	switch (offset)
	{
		case 0x00:	/* CR */
			result = (voice->control & (CONTROL_STOPMASK | CONTROL_LOOPMASK | CONTROL_IRQE | CONTROL_DIR | CONTROL_IRQ)) |
					 ((voice->control & CONTROL_BS0) >> 12) |
					 ((voice->control & CONTROL_LPMASK) << 2) |
					 ((voice->control & (CONTROL_CA0 | CONTROL_CA1)) >> 2) |
					 0xf000;
			break;

		case 0x01:	/* O4(n-1) */
			result = voice->o4n1;
			break;

		case 0x02:	/* O3(n-1) */
			result = voice->o3n1;
			break;

		case 0x03:	/* O3(n-2) */
			result = voice->o3n2;
			break;

		case 0x04:	/* O2(n-1) */
			result = voice->o2n1;
			break;

		case 0x05:	/* O2(n-2) */
			result = voice->o2n2;
			break;

		case 0x06:	/* O1(n-1) */
			/* special case for the Taito F3 games: they set the accumulator on a stopped */
			/* voice and assume the filters continue to process the data. They then read */
			/* the O1(n-1) in order to extract raw data from the sound ROMs. Since we don't */
			/* want to waste time filtering stopped channels, we just look for a read from */
			/* this register on a stopped voice, and return the raw sample data at the */
			/* accumulator */
			if ((voice->control & CONTROL_STOPMASK) && chip->region_base[voice->control >> 14]) {
				voice->o1n1 = chip->region_base[voice->control >> 14][voice->exbank + (voice->accum >> 11)];
//				logerror("%02x %08x ==> %08x\n",voice->o1n1,voice->control >> 14,voice->exbank + (voice->accum >> 11));
			}
				result = voice->o1n1;
			break;

		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:	/* unused */
			break;

		case 0x0d:	/* ACT */
			result = chip->active_voices;
			break;

		case 0x0e:	/* IRQV */
			result = chip->irqv;
			update_internal_irq_state();
			break;

		case 0x0f:	/* PAGE */
			result = chip->current_page;
			break;
	}
	return result;
}


ES5506_INLINE UINT16 es5505_reg_read_test(UINT32 offset)
{
	UINT16 result = 0;

	switch (offset)
	{
		case 0x00:	/* CH0L */
		case 0x01:	/* CH0R */
		case 0x02:	/* CH1L */
		case 0x03:	/* CH1R */
		case 0x04:	/* CH2L */
		case 0x05:	/* CH2R */
		case 0x06:	/* CH3L */
		case 0x07:	/* CH3R */
			break;

		case 0x08:	/* SERMODE */
			result = chip->mode;
			break;

		case 0x09:	/* PAR */
			if (chip->port_read)
				result = (*chip->port_read)();
			break;

		case 0x0f:	/* PAGE */
			result = chip->current_page;
			break;
	}
	return result;
}


//READ16_DEVICE_HANDLER( es5505_r )
UINT16 ES5505Read(UINT32 offset)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ES5506Initted) bprintf(PRINT_ERROR, _T("ES5505Read called without init\n"));
#endif

//	es5506_state *chip = get_safe_token(device);
	es5506_voice *voice = &chip->voice[chip->current_page & 0x1f];
	UINT16 result = 0;

	if (LOG_COMMANDS && eslog)
		fprintf(eslog, "read from %02x/%02x -> ", chip->current_page, offset);

	/* force an update */
//	chip->stream->update();

	/* switch off the page and register */
	if (chip->current_page < 0x20)
		result = es5505_reg_read_low(voice, offset);
	else if (chip->current_page < 0x40)
		result = es5505_reg_read_high(voice, offset);
	else
		result = es5505_reg_read_test(offset);

	if (LOG_COMMANDS && eslog)
		fprintf(eslog, "%04x (accum=%08x)\n", result, voice->accum);

	/* return the high byte */
	return result;
}



void es5505_voice_bank_w(INT32 voice, INT32 bank)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ES5506Initted) bprintf(PRINT_ERROR, _T("es5505_voice_bank_w called without init\n"));
#endif

//	es5506_state *chip = get_safe_token(device);
#if RAINE_CHECK
	chip->voice[voice].control = CONTROL_STOPMASK;
#endif
	chip->voice[voice].exbank=bank;
}


#if 0

/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( es5505 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(es5506_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( es5505 );		break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME( es5505 );		break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( es5505 );		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "ES5505");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Ensoniq Wavetable");			break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( es5506 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(es5506_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( es5506 );		break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME( es5506 );		break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( es5506 );		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "ES5506");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Ensoniq Wavetable");			break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(ES5505, es5505);
DEFINE_LEGACY_SOUND_DEVICE(ES5506, es5506);
#endif
