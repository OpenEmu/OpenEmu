#define SETA_NUM_CHANNELS	16
#define SETA_NUM_BANKS 		(0x100000 / 0x20000)

#define FREQ_BASE_BITS		8					// Frequency fixed decimal shift bits
#define ENV_BASE_BITS		 16					// wave form envelope fixed decimal shift bits
#define	VOL_BASE	(2 * 32 * 256 / 30)			// Volume base

struct x1_010_info
{
	INT32	rate;									// Output sampling rate (Hz)
	//sound_stream *	stream;					// Stream handle
	INT32	address;								// address eor data
	INT32	sound_enable;							// sound output enable/disable
	UINT8 reg[0x2000];					// X1-010 Register & wave form area
	UINT8 HI_WORD_BUF[0x2000];			// X1-010 16bit access ram check avoidance work
	UINT32 smp_offset[SETA_NUM_CHANNELS];
	UINT32 env_offset[SETA_NUM_CHANNELS];
	UINT32 base_clock;
	UINT32 sound_banks[SETA_NUM_BANKS];
	double gain[2];
	INT32 output_dir[2];
};

typedef struct {
	UINT8	status;
	UINT8	volume;					// volume / wave form no.
	UINT8	frequency;			// frequency / pitch lo
	UINT8	pitch_hi;				// reserved / pitch hi
	UINT8	start;					// start address / envelope time
	UINT8	end;						// end address / envelope no.
	UINT8	reserve[2];
} X1_010_CHANNEL;

extern UINT8 *X1010SNDROM;

extern struct x1_010_info * x1_010_chip;

void x1010_sound_bank_w(UINT32 offset, UINT16 data);
UINT8 x1010_sound_read(UINT32 offset);
UINT16 x1010_sound_read_word(UINT32 offset);
void x1010_sound_update();
void x1010_sound_init(UINT32 base_clock, INT32 address);
void x1010_set_route(INT32 nIndex, double nVolume, INT32 nRouteDir);
void x1010_scan(INT32 nAction,INT32 *pnMin);
void x1010_exit();

#define BURN_SND_X1010_ROUTE_1		0
#define BURN_SND_X1010_ROUTE_2		1

#define x1010_set_all_routes(v, d)						\
	x1010_set_route(BURN_SND_X1010_ROUTE_1, v, d);		\
	x1010_set_route(BURN_SND_X1010_ROUTE_2, v, d);
