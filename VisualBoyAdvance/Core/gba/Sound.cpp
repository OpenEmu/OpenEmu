#include <string.h>

#include "Sound.h"

#include "GBA.h"
#include "Globals.h"
#include "../Util.h"
#include "../common/Port.h"

#include "../apu/Gb_Apu.h"
#include "../apu/Multi_Buffer.h"

#include "../common/SoundDriver.h"

#define NR10 0x60
#define NR11 0x62
#define NR12 0x63
#define NR13 0x64
#define NR14 0x65
#define NR21 0x68
#define NR22 0x69
#define NR23 0x6c
#define NR24 0x6d
#define NR30 0x70
#define NR31 0x72
#define NR32 0x73
#define NR33 0x74
#define NR34 0x75
#define NR41 0x78
#define NR42 0x79
#define NR43 0x7c
#define NR44 0x7d
#define NR50 0x80
#define NR51 0x81
#define NR52 0x84

SoundDriver * soundDriver = 0;

extern bool stopState;      // TODO: silence sound when true

int const SOUND_CLOCK_TICKS_ = 167772; // 1/100 second

static u16   soundFinalWave [1600];
long  soundSampleRate    = 44100;
bool  soundInterpolation = true;
bool  soundPaused        = true;
float soundFiltering     = 0.5f;
int   SOUND_CLOCK_TICKS  = SOUND_CLOCK_TICKS_;
int   soundTicks         = SOUND_CLOCK_TICKS_;

static float soundVolume     = 1.0f;
static int soundEnableFlag   = 0x3ff; // emulator channels enabled
static float soundFiltering_ = -1;
static float soundVolume_    = -1;

void interp_rate() { /* empty for now */ }

class Gba_Pcm {
public:
	void init();
	void apply_control( int idx );
	void update( int dac );
	void end_frame( blip_time_t );

private:
	Blip_Buffer* output;
	blip_time_t last_time;
	int last_amp;
	int shift;
};

class Gba_Pcm_Fifo {
public:
	int     which;
	Gba_Pcm pcm;

	void write_control( int data );
	void write_fifo( int data );
	void timer_overflowed( int which_timer );

	// public only so save state routines can access it
	int  readIndex;
	int  count;
	int  writeIndex;
	u8   fifo [32];
	int  dac;
private:

	int  timer;
	bool enabled;
};

static Gba_Pcm_Fifo     pcm [2];
static Gb_Apu*          gb_apu;
static Stereo_Buffer*   stereo_buffer;

static Blip_Synth<blip_best_quality,1> pcm_synth [3]; // 32 kHz, 16 kHz, 8 kHz

static inline blip_time_t blip_time()
{
	return SOUND_CLOCK_TICKS - soundTicks;
}

void Gba_Pcm::init()
{
	output    = 0;
	last_time = 0;
	last_amp  = 0;
	shift     = 0;
}

void Gba_Pcm::apply_control( int idx )
{
	shift = ~ioMem [SGCNT0_H] >> (2 + idx) & 1;

	int ch = 0;
	if ( (soundEnableFlag >> idx & 0x100) && (ioMem [NR52] & 0x80) )
		ch = ioMem [SGCNT0_H+1] >> (idx * 4) & 3;

	Blip_Buffer* out = 0;
	switch ( ch )
	{
	case 1: out = stereo_buffer->right();  break;
	case 2: out = stereo_buffer->left();   break;
	case 3: out = stereo_buffer->center(); break;
	}

	if ( output != out )
	{
		if ( output )
		{
			output->set_modified();
			pcm_synth [0].offset( blip_time(), -last_amp, output );
		}
		last_amp = 0;
		output = out;
	}
}

void Gba_Pcm::end_frame( blip_time_t time )
{
	last_time -= time;
	if ( last_time < -2048 )
		last_time = -2048;

	if ( output )
		output->set_modified();
}

void Gba_Pcm::update( int dac )
{
	if ( output )
	{
		blip_time_t time = blip_time();

		dac = (s8) dac >> shift;
		int delta = dac - last_amp;
		if ( delta )
		{
			last_amp = dac;

			int filter = 0;
			if ( soundInterpolation )
			{
				// base filtering on how long since last sample was output
				int period = time - last_time;

				int idx = (unsigned) period / 512;
				if ( idx >= 3 )
					idx = 3;

				static int const filters [4] = { 0, 0, 1, 2 };
				filter = filters [idx];
			}

			pcm_synth [filter].offset( time, delta, output );
		}
		last_time = time;
	}
}

void Gba_Pcm_Fifo::timer_overflowed( int which_timer )
{
	if ( which_timer == timer && enabled )
	{
		if ( count <= 16 )
		{
			// Need to fill FIFO
			CPUCheckDMA( 3, which ? 4 : 2 );
			if ( count <= 16 )
			{
				// Not filled by DMA, so fill with 16 bytes of silence
				int reg = which ? FIFOB_L : FIFOA_L;
				for ( int n = 4; n--; )
				{
					soundEvent(reg  , (u16)0);
					soundEvent(reg+2, (u16)0);
				}
			}
		}

		// Read next sample from FIFO
		count--;
		dac = fifo [readIndex];
		readIndex = (readIndex + 1) & 31;
		pcm.update( dac );
	}
}

void Gba_Pcm_Fifo::write_control( int data )
{
	enabled = (data & 0x0300) ? true : false;
	timer   = (data & 0x0400) ? 1 : 0;

	if ( data & 0x0800 )
	{
		// Reset
		writeIndex = 0;
		readIndex  = 0;
		count      = 0;
		dac        = 0;
		memset( fifo, 0, sizeof fifo );
	}

	pcm.apply_control( which );
	pcm.update( dac );
}

void Gba_Pcm_Fifo::write_fifo( int data )
{
	fifo [writeIndex  ] = data & 0xFF;
	fifo [writeIndex+1] = data >> 8;
	count += 2;
	writeIndex = (writeIndex + 2) & 31;
}

static void apply_control()
{
	pcm [0].pcm.apply_control( 0 );
	pcm [1].pcm.apply_control( 1 );
}

static int gba_to_gb_sound( int addr )
{
	static const int table [0x40] =
	{
		0xFF10,     0,0xFF11,0xFF12,0xFF13,0xFF14,     0,     0,
		0xFF16,0xFF17,     0,     0,0xFF18,0xFF19,     0,     0,
		0xFF1A,     0,0xFF1B,0xFF1C,0xFF1D,0xFF1E,     0,     0,
		0xFF20,0xFF21,     0,     0,0xFF22,0xFF23,     0,     0,
		0xFF24,0xFF25,     0,     0,0xFF26,     0,     0,     0,
		     0,     0,     0,     0,     0,     0,     0,     0,
		0xFF30,0xFF31,0xFF32,0xFF33,0xFF34,0xFF35,0xFF36,0xFF37,
		0xFF38,0xFF39,0xFF3A,0xFF3B,0xFF3C,0xFF3D,0xFF3E,0xFF3F,
	};
	if ( addr >= 0x60 && addr < 0xA0 )
		return table [addr - 0x60];
	return 0;
}

void soundEvent(u32 address, u8 data)
{
	int gb_addr = gba_to_gb_sound( address );
	if ( gb_addr )
	{
		ioMem[address] = data;
		gb_apu->write_register( blip_time(), gb_addr, data );

		if ( address == NR52 )
			apply_control();
	}

	// TODO: what about byte writes to SGCNT0_H etc.?
}

static void apply_volume( bool apu_only = false )
{
	if ( !apu_only )
		soundVolume_ = soundVolume;

	if ( gb_apu )
	{
		static float const apu_vols [4] = { 0.25, 0.5, 1, 0.25 };
		gb_apu->volume( soundVolume_ * apu_vols [ioMem [SGCNT0_H] & 3] );
	}

	if ( !apu_only )
	{
		for ( int i = 0; i < 3; i++ )
			pcm_synth [i].volume( 0.66 / 256 * soundVolume_ );
	}
}

static void write_SGCNT0_H( int data )
{
	WRITE16LE( &ioMem [SGCNT0_H], data & 0x770F );
	pcm [0].write_control( data      );
	pcm [1].write_control( data >> 4 );
	apply_volume( true );
}

void soundEvent(u32 address, u16 data)
{
	switch ( address )
	{
	case SGCNT0_H:
		write_SGCNT0_H( data );
		break;

	case FIFOA_L:
	case FIFOA_H:
		pcm [0].write_fifo( data );
		WRITE16LE( &ioMem[address], data );
		break;

	case FIFOB_L:
	case FIFOB_H:
		pcm [1].write_fifo( data );
		WRITE16LE( &ioMem[address], data );
		break;

	case 0x88:
		data &= 0xC3FF;
		WRITE16LE( &ioMem[address], data );
		break;

	default:
		soundEvent( address & ~1, (u8) (data     ) ); // even
		soundEvent( address |  1, (u8) (data >> 8) ); // odd
		break;
	}
}

void soundTimerOverflow(int timer)
{
	pcm [0].timer_overflowed( timer );
	pcm [1].timer_overflowed( timer );
}

static void end_frame( blip_time_t time )
{
	pcm [0].pcm.end_frame( time );
	pcm [1].pcm.end_frame( time );

	gb_apu       ->end_frame( time );
	stereo_buffer->end_frame( time );
}

void flush_samples(Multi_Buffer * buffer)
{
	// We want to write the data frame by frame to support legacy audio drivers
	// that don't use the length parameter of the write method.
	// TODO: Update the Win32 audio drivers (DS, OAL, XA2), and flush all the
	// samples at once to help reducing the audio delay on all platforms.
	int soundBufferLen = ( soundSampleRate / 60 ) * 4;

	// soundBufferLen should have a whole number of sample pairs
	assert( soundBufferLen % (2 * sizeof (*soundFinalWave)) == 0 );

	// number of samples in output buffer
	int const out_buf_size = soundBufferLen / sizeof (*soundFinalWave);

	// Keep filling and writing soundFinalWave until it can't be fully filled
	while ( buffer->samples_avail() >= out_buf_size )
	{
		buffer->read_samples( (blip_sample_t*) soundFinalWave, out_buf_size );
		if(soundPaused)
			soundResume();

		soundDriver->write(soundFinalWave, soundBufferLen);
		systemOnWriteDataToSoundBuffer(soundFinalWave, soundBufferLen);
	}
}

static void apply_filtering()
{
	soundFiltering_ = soundFiltering;

	int const base_freq = (int) (32768 - soundFiltering_ * 16384);
	int const nyquist = stereo_buffer->sample_rate() / 2;

	for ( int i = 0; i < 3; i++ )
	{
		int cutoff = base_freq >> i;
		if ( cutoff > nyquist )
			cutoff = nyquist;
		pcm_synth [i].treble_eq( blip_eq_t( 0, 0, stereo_buffer->sample_rate(), cutoff ) );
	}
}

void psoundTickfn()
{
 	if ( gb_apu && stereo_buffer )
	{
		// Run sound hardware to present
		end_frame( SOUND_CLOCK_TICKS );

		flush_samples(stereo_buffer);

		if ( soundFiltering_ != soundFiltering )
			apply_filtering();

		if ( soundVolume_ != soundVolume )
			apply_volume();
	}
}

static void apply_muting()
{
	if ( !stereo_buffer || !ioMem )
		return;

	// PCM
	apply_control();

	if ( gb_apu )
	{
		// APU
		for ( int i = 0; i < 4; i++ )
		{
			if ( soundEnableFlag >> i & 1 )
				gb_apu->set_output( stereo_buffer->center(),
						stereo_buffer->left(), stereo_buffer->right(), i );
			else
				gb_apu->set_output( 0, 0, 0, i );
		}
	}
}

static void reset_apu()
{
	gb_apu->reset( gb_apu->mode_agb, true );

	if ( stereo_buffer )
		stereo_buffer->clear();

	soundTicks = SOUND_CLOCK_TICKS;
}

static void remake_stereo_buffer()
{
	if ( !ioMem )
		return;

	// Clears pointers kept to old stereo_buffer
	pcm [0].pcm.init();
	pcm [1].pcm.init();

	// Stereo_Buffer
	delete stereo_buffer;
	stereo_buffer = 0;

	stereo_buffer = new Stereo_Buffer; // TODO: handle out of memory
	stereo_buffer->set_sample_rate( soundSampleRate ); // TODO: handle out of memory
	stereo_buffer->clock_rate( gb_apu->clock_rate );

	// PCM
	pcm [0].which = 0;
	pcm [1].which = 1;
	apply_filtering();

	// APU
	if ( !gb_apu )
	{
		gb_apu = new Gb_Apu; // TODO: handle out of memory
		reset_apu();
	}

	apply_muting();
	apply_volume();
}

void soundShutdown()
{
	if (soundDriver)
	{
		delete soundDriver;
		soundDriver = 0;
	}

	systemOnSoundShutdown();
}

void soundPause()
{
	soundPaused = true;
	if (soundDriver)
		soundDriver->pause();
}

void soundResume()
{
	soundPaused = false;
	if (soundDriver)
		soundDriver->resume();
}

void soundSetVolume( float volume )
{
	soundVolume = volume;
}

float soundGetVolume()
{
	return soundVolume;
}

void soundSetEnable(int channels)
{
	soundEnableFlag = channels;
	apply_muting();
}

int soundGetEnable()
{
	return (soundEnableFlag & 0x30f);
}

void soundReset()
{
	soundDriver->reset();

	remake_stereo_buffer();
	reset_apu();

	soundPaused = true;
	SOUND_CLOCK_TICKS = SOUND_CLOCK_TICKS_;
	soundTicks        = SOUND_CLOCK_TICKS_;

	soundEvent( NR52, (u8) 0x80 );
}

bool soundInit()
{
	soundDriver = systemSoundInit();
	if ( !soundDriver )
		return false;

	if (!soundDriver->init(soundSampleRate))
		return false;

	soundPaused = true;
	return true;
}

void soundSetThrottle(unsigned short throttle)
{
	if(!soundDriver)
		return;
	soundDriver->setThrottle(throttle);
}

long soundGetSampleRate()
{
	return soundSampleRate;
}

void soundSetSampleRate(long sampleRate)
{
	if ( soundSampleRate != sampleRate )
	{
		if ( systemCanChangeSoundQuality() )
		{
			soundShutdown();
			soundSampleRate      = sampleRate;
			soundInit();
		}
		else
		{
			soundSampleRate      = sampleRate;
		}

		remake_stereo_buffer();
	}
}

static int dummy_state [16];

#define SKIP( type, name ) { dummy_state, sizeof (type) }

#define LOAD( type, name ) { &name, sizeof (type) }

static struct {
	gb_apu_state_t apu;

	// old state
	u8 soundDSAValue;
	int soundDSBValue;
} state;

// Old GBA sound state format
static variable_desc old_gba_state [] =
{
	SKIP( int, soundPaused ),
	SKIP( int, soundPlay ),
	SKIP( int, soundTicks ),
	SKIP( int, SOUND_CLOCK_TICKS ),
	SKIP( int, soundLevel1 ),
	SKIP( int, soundLevel2 ),
	SKIP( int, soundBalance ),
	SKIP( int, soundMasterOn ),
	SKIP( int, soundIndex ),
	SKIP( int, sound1On ),
	SKIP( int, sound1ATL ),
	SKIP( int, sound1Skip ),
	SKIP( int, sound1Index ),
	SKIP( int, sound1Continue ),
	SKIP( int, sound1EnvelopeVolume ),
	SKIP( int, sound1EnvelopeATL ),
	SKIP( int, sound1EnvelopeATLReload ),
	SKIP( int, sound1EnvelopeUpDown ),
	SKIP( int, sound1SweepATL ),
	SKIP( int, sound1SweepATLReload ),
	SKIP( int, sound1SweepSteps ),
	SKIP( int, sound1SweepUpDown ),
	SKIP( int, sound1SweepStep ),
	SKIP( int, sound2On ),
	SKIP( int, sound2ATL ),
	SKIP( int, sound2Skip ),
	SKIP( int, sound2Index ),
	SKIP( int, sound2Continue ),
	SKIP( int, sound2EnvelopeVolume ),
	SKIP( int, sound2EnvelopeATL ),
	SKIP( int, sound2EnvelopeATLReload ),
	SKIP( int, sound2EnvelopeUpDown ),
	SKIP( int, sound3On ),
	SKIP( int, sound3ATL ),
	SKIP( int, sound3Skip ),
	SKIP( int, sound3Index ),
	SKIP( int, sound3Continue ),
	SKIP( int, sound3OutputLevel ),
	SKIP( int, sound4On ),
	SKIP( int, sound4ATL ),
	SKIP( int, sound4Skip ),
	SKIP( int, sound4Index ),
	SKIP( int, sound4Clock ),
	SKIP( int, sound4ShiftRight ),
	SKIP( int, sound4ShiftSkip ),
	SKIP( int, sound4ShiftIndex ),
	SKIP( int, sound4NSteps ),
	SKIP( int, sound4CountDown ),
	SKIP( int, sound4Continue ),
	SKIP( int, sound4EnvelopeVolume ),
	SKIP( int, sound4EnvelopeATL ),
	SKIP( int, sound4EnvelopeATLReload ),
	SKIP( int, sound4EnvelopeUpDown ),
	LOAD( int, soundEnableFlag ),
	SKIP( int, soundControl ),
	LOAD( int, pcm [0].readIndex ),
	LOAD( int, pcm [0].count ),
	LOAD( int, pcm [0].writeIndex ),
	SKIP( u8,  soundDSAEnabled ), // was bool, which was one byte on MS compiler
	SKIP( int, soundDSATimer ),
	LOAD( u8 [32], pcm [0].fifo ),
	LOAD( u8,  state.soundDSAValue ),
	LOAD( int, pcm [1].readIndex ),
	LOAD( int, pcm [1].count ),
	LOAD( int, pcm [1].writeIndex ),
	SKIP( int, soundDSBEnabled ),
	SKIP( int, soundDSBTimer ),
	LOAD( u8 [32], pcm [1].fifo ),
	LOAD( int, state.soundDSBValue ),

	// skipped manually
	//LOAD( int, soundBuffer[0][0], 6*735 },
	//LOAD( int, soundFinalWave[0], 2*735 },
	{ NULL, 0 }
};

variable_desc old_gba_state2 [] =
{
	LOAD( u8 [0x20], state.apu.regs [0x20] ),
	SKIP( int, sound3Bank ),
	SKIP( int, sound3DataSize ),
	SKIP( int, sound3ForcedOutput ),
	{ NULL, 0 }
};

// New state format
static variable_desc gba_state [] =
{
	// PCM
	LOAD( int, pcm [0].readIndex ),
	LOAD( int, pcm [0].count ),
	LOAD( int, pcm [0].writeIndex ),
	LOAD(u8[32],pcm[0].fifo ),
	LOAD( int, pcm [0].dac ),

	SKIP( int [4], room_for_expansion ),

	LOAD( int, pcm [1].readIndex ),
	LOAD( int, pcm [1].count ),
	LOAD( int, pcm [1].writeIndex ),
	LOAD(u8[32],pcm[1].fifo ),
	LOAD( int, pcm [1].dac ),

	SKIP( int [4], room_for_expansion ),

	// APU
	LOAD( u8 [0x40], state.apu.regs ),      // last values written to registers and wave RAM (both banks)
	LOAD( int, state.apu.frame_time ),      // clocks until next frame sequencer action
	LOAD( int, state.apu.frame_phase ),     // next step frame sequencer will run

	LOAD( int, state.apu.sweep_freq ),      // sweep's internal frequency register
	LOAD( int, state.apu.sweep_delay ),     // clocks until next sweep action
	LOAD( int, state.apu.sweep_enabled ),
	LOAD( int, state.apu.sweep_neg ),       // obscure internal flag
	LOAD( int, state.apu.noise_divider ),
	LOAD( int, state.apu.wave_buf ),        // last read byte of wave RAM

	LOAD( int [4], state.apu.delay ),       // clocks until next channel action
	LOAD( int [4], state.apu.length_ctr ),
	LOAD( int [4], state.apu.phase ),       // square/wave phase, noise LFSR
	LOAD( int [4], state.apu.enabled ),     // internal enabled flag

	LOAD( int [3], state.apu.env_delay ),   // clocks until next envelope action
	LOAD( int [3], state.apu.env_volume ),
	LOAD( int [3], state.apu.env_enabled ),

	SKIP( int [13], room_for_expansion ),

	// Emulator
	LOAD( int, soundEnableFlag ),

	SKIP( int [15], room_for_expansion ),

	{ NULL, 0 }
};

// Reads and discards count bytes from in
static void skip_read( gzFile in, int count )
{
	char buf [512];

	while ( count )
	{
		int n = sizeof buf;
		if ( n > count )
			n = count;

		count -= n;
		utilGzRead( in, buf, n );
	}
}

void soundSaveGame( gzFile out )
{
	gb_apu->save_state( &state.apu );

	// Be sure areas for expansion get written as zero
	memset( dummy_state, 0, sizeof dummy_state );

	utilWriteData( out, gba_state );
}

static void soundReadGameOld( gzFile in, int version )
{
	// Read main data
	utilReadData( in, old_gba_state );
	skip_read( in, 6*735 + 2*735 );

	// Copy APU regs
	static int const regs_to_copy [] = {
		NR10, NR11, NR12, NR13, NR14,
		      NR21, NR22, NR23, NR24,
		NR30, NR31, NR32, NR33, NR34,
		      NR41, NR42, NR43, NR44,
		NR50, NR51, NR52, -1
	};

	ioMem [NR52] |= 0x80; // old sound played even when this wasn't set (power on)

	for ( int i = 0; regs_to_copy [i] >= 0; i++ )
		state.apu.regs [gba_to_gb_sound( regs_to_copy [i] ) - 0xFF10] = ioMem [regs_to_copy [i]];

	// Copy wave RAM to both banks
	memcpy( &state.apu.regs [0x20], &ioMem [0x90], 0x10 );
	memcpy( &state.apu.regs [0x30], &ioMem [0x90], 0x10 );

	// Read both banks of wave RAM if available
	if ( version >= SAVE_GAME_VERSION_3 )
		utilReadData( in, old_gba_state2 );

	// Restore PCM
	pcm [0].dac = state.soundDSAValue;
	pcm [1].dac = state.soundDSBValue;

	(void) utilReadInt( in ); // ignore quality
}

#include <stdio.h>

void soundReadGame( gzFile in, int version )
{
	// Prepare APU and default state
	reset_apu();
	gb_apu->save_state( &state.apu );

	if ( version > SAVE_GAME_VERSION_9 )
		utilReadData( in, gba_state );
	else
		soundReadGameOld( in, version );

	gb_apu->load_state( state.apu );
	write_SGCNT0_H( READ16LE( &ioMem [SGCNT0_H] ) & 0x770F );

	apply_muting();
}
