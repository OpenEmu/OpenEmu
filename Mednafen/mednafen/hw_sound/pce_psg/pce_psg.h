#ifndef _PCE_PSG_H
#define _PCE_PSG_H

#include <blip/Blip_Buffer.h>
#include <blip/Stereo_Buffer.h>

class PCE_PSG;

struct psg_channel
{
        int32 counter;
        uint16 frequency;       /* Channel frequency */
	uint32 freq_cache;
        uint8 control;          /* Channel enable, DDA, volume */
        uint8 balance;          /* Channel balance */
        uint8 waveform[32];     /* Waveform data */
        uint8 waveform_index;   /* Waveform data index */
        uint8 dda;
        uint8 noisectrl;        /* Noise enable/ctrl (channels 4,5 only) */
	uint32 noise_freq_cache;	// Channel 4,5 only
        int32 noisecount;
        uint32 lfsr;

	//int32 sample_cache[2];

        int32 vl[2];	//vll, vlr;

        int samp_accum;		// The result of adding up all the samples in the waveform buffer(part of an optimization for high-frequency playback).

	int32 blip_prev_samp[2];
	int32 lastts;

	void (PCE_PSG::*UpdateOutput)(const int32 timestamp, psg_channel *ch);
};

// Only CH4 and CH5 have NCTRL and LFSR, but it's here for the other channels for "consistency".
enum
{
 PSG_GSREG_CH0_FREQ = 0x000,
// PSG_GSREG_CH0_COUNTER,
 PSG_GSREG_CH0_CTRL,
 PSG_GSREG_CH0_BALANCE,
 PSG_GSREG_CH0_WINDEX,
 PSG_GSREG_CH0_SCACHE,
 PSG_GSREG_CH0_NCTRL,
 PSG_GSREG_CH0_LFSR,

 PSG_GSREG_CH1_FREQ = 0x100,
// PSG_GSREG_CH1_COUNTER,
 PSG_GSREG_CH1_CTRL,
 PSG_GSREG_CH1_BALANCE,
 PSG_GSREG_CH1_WINDEX,
 PSG_GSREG_CH1_SCACHE,
 PSG_GSREG_CH1_NCTRL,
 PSG_GSREG_CH1_LFSR,

 PSG_GSREG_CH2_FREQ = 0x200,
// PSG_GSREG_CH2_COUNTER,
 PSG_GSREG_CH2_CTRL,
 PSG_GSREG_CH2_BALANCE,
 PSG_GSREG_CH2_WINDEX,
 PSG_GSREG_CH2_SCACHE,
 PSG_GSREG_CH2_NCTRL,
 PSG_GSREG_CH2_LFSR,

 PSG_GSREG_CH3_FREQ = 0x300,
// PSG_GSREG_CH3_COUNTER,
 PSG_GSREG_CH3_CTRL,
 PSG_GSREG_CH3_BALANCE,
 PSG_GSREG_CH3_WINDEX,
 PSG_GSREG_CH3_SCACHE,
 PSG_GSREG_CH3_NCTRL,
 PSG_GSREG_CH3_LFSR,

 PSG_GSREG_CH4_FREQ = 0x400,
// PSG_GSREG_CH4_COUNTER,
 PSG_GSREG_CH4_CTRL,
 PSG_GSREG_CH4_BALANCE,
 PSG_GSREG_CH4_WINDEX,
 PSG_GSREG_CH4_SCACHE,
 PSG_GSREG_CH4_NCTRL,
 PSG_GSREG_CH4_LFSR,

 PSG_GSREG_CH5_FREQ = 0x500,
// PSG_GSREG_CH5_COUNTER,
 PSG_GSREG_CH5_CTRL,
 PSG_GSREG_CH5_BALANCE,
 PSG_GSREG_CH5_WINDEX,
 PSG_GSREG_CH5_SCACHE,
 PSG_GSREG_CH5_NCTRL,
 PSG_GSREG_CH5_LFSR,

 PSG_GSREG_SELECT = 0x1000,
 PSG_GSREG_GBALANCE,
 PSG_GSREG_LFOFREQ,
 PSG_GSREG_LFOCTRL,
 _PSG_GSREG_COUNT
};

class PCE_PSG
{
        public:

	enum
	{
	 REVISION_HUC6280 = 0,
	 REVISION_HUC6280A,
	 REVISION_ENHANCED,
	 _REVISION_COUNT
	};


        PCE_PSG(Blip_Buffer *bb_l, Blip_Buffer *bb_r, int want_revision);
        ~PCE_PSG();

	int StateAction(StateMem *sm, int load, int data_only);

        void Power(const int32 timestamp);
        void Write(int32 timestamp, uint8 A, uint8 V);

	void SetVolume(double new_volume);

	void EndFrame(int32 timestamp);

	// TODO: timestamp
	uint32 GetRegister(const unsigned int id, char *special, const uint32 special_len);
	void SetRegister(const unsigned int id, const uint32 value);

	void PeekWave(const unsigned int ch, uint32 Address, uint32 Length, uint8 *Buffer);
	void PokeWave(const unsigned int ch, uint32 Address, uint32 Length, const uint8 *Buffer);

        private:

	void Update(int32 timestamp);

	void UpdateSubLFO(int32 timestamp);
	void UpdateSubNonLFO(int32 timestamp);

	void RecalcUOFunc(int chnum);
	void UpdateOutput_Off(const int32 timestamp, psg_channel *ch);
	void UpdateOutput_Accum(const int32 timestamp, psg_channel *ch);
        void UpdateOutput_Norm(const int32 timestamp, psg_channel *ch);
        void UpdateOutput_Noise(const int32 timestamp, psg_channel *ch);

//        void UpdateOutput_Norm_IL(const int32 timestamp, psg_channel *ch);
//        void UpdateOutput_Noise_IL(const int32 timestamp, psg_channel *ch);


	int32 GetVL(const int chnum, const int lr);

	void RecalcFreqCache(int chnum);
	void RecalcNoiseFreqCache(int chnum);
	void RunChannel(int chc, int32 timestamp, bool LFO_On);
	double OutputVolume;

        uint8 select;               /* Selected channel (0-5) */
        uint8 globalbalance;        /* Global sound balance */
        uint8 lfofreq;              /* LFO frequency */
        uint8 lfoctrl;              /* LFO control */

        int32 vol_update_counter;
        int32 vol_update_which;
	int32 vol_update_vllatch;
	bool vol_pending;

        psg_channel channel[6];

        int32 lastts;
	int revision;

	bool SoundEnabled;
	Blip_Buffer *sbuf[2];
	Blip_Synth<blip_good_quality, 8192> Synth;

        int32 dbtable_volonly[32];

	int32 dbtable[32][32];
};

#endif
