////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
//
// This file is part of Nestopia.
//
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

#ifndef NST_CPU_H
#error Do not include NstApu.h directly!
#endif

#include "NstSoundRenderer.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Sound
		{
			class Output;
		}

		namespace State
		{
			class Saver;
			class Loader;
		}

		class Cpu;

		class Apu
		{
		public:

			explicit Apu(Cpu&);

			void  Reset(bool);
			void  PowerOff();
			void  ClearBuffers();
			void  UpdateModel();
			void  BeginFrame(Sound::Output*);
			void  EndFrame();
			void  WriteFrameCtrl(uint);
			Cycle Clock();
			void  ClockDMA();

			Result SetSampleRate(dword);
			Result SetSampleBits(uint);
			Result SetSpeed(uint);
			Result SetVolume(uint,uint);
			uint   GetVolume(uint) const;
			void   SetAutoTranspose(bool);
			void   EnableStereo(bool);

			void SaveState(State::Saver&,dword) const;
			void LoadState(State::Loader&);

			class NST_NO_VTABLE Channel
			{
				Apu& apu;

			protected:

				Channel(Apu&);
				~Channel();

				void  Update() const;
				void  Connect(bool);
				dword GetSampleRate() const;
				uint  GetVolume(uint) const;
				void  GetOscillatorClock(Cycle&,uint&) const;
				CpuModel GetModel() const;

			public:

				typedef Sound::Sample Sample;

				enum
				{
					APU_SQUARE1,
					APU_SQUARE2,
					APU_TRIANGLE,
					APU_NOISE,
					APU_DPCM,
					EXT_FDS,
					EXT_MMC5,
					EXT_VRC6,
					EXT_VRC7,
					EXT_N163,
					EXT_S5B
				};

				enum
				{
					OUTPUT_MIN     = -32767,
					OUTPUT_MAX     = +32767,
					OUTPUT_MUL     =  256,
					OUTPUT_DECAY   =  OUTPUT_MUL / 4 - 1,
					DEFAULT_VOLUME =  85
				};

				virtual void Reset() = 0;
				virtual Sample GetSample() = 0;
				virtual Cycle Clock(Cycle,Cycle,Cycle);
				virtual bool UpdateSettings() = 0;

				class LengthCounter
				{
				public:

					LengthCounter();

					void Reset();
					void LoadState(State::Loader&);
					void SaveState(State::Saver&,dword) const;

				private:

					uint enabled;
					uint count;

					static const byte lut[32];

				public:

					uint Disable(uint disable)
					{
						NST_ASSERT( disable <= 1 );
						enabled = disable - 1;
						count &= enabled;
						return enabled;
					}

					void Write(uint data)
					{
						NST_ASSERT( (data >> 3) < sizeof(array(lut)) );
						count = lut[data >> 3] & enabled;
					}

					void Write(uint data,bool frameCounterDelta)
					{
						NST_VERIFY_MSG( frameCounterDelta, "APU $40xx/framecounter conflict" );

						if (frameCounterDelta || !count)
							Write( data );
					}

					uint GetCount() const
					{
						return count;
					}

					bool Clock()
					{
						return count && !--count;
					}
				};

				class Envelope
				{
				public:

					Envelope();

					void Reset();
					void SetOutputVolume(uint);
					void LoadState(State::Loader&);
					void SaveState(State::Saver&,dword) const;

					void Clock();
					void Write(uint);

				private:

					void UpdateOutput();

					dword output;
					uint  outputVolume;
					byte  regs[2];
					byte  count;
					bool  reset;

				public:

					bool Looping() const
					{
						return regs[1] & 0x20U;
					}

					dword Volume() const
					{
						return output;
					}

					void ResetClock()
					{
						reset = true;
					}
				};

				class DcBlocker
				{
				public:

					DcBlocker();

					void Reset();
					Sample Apply(Sample);

				private:

					enum
					{
						POLE = 3 // ~0.9999
					};

					idword prev;
					idword next;
					idword acc;
				};
			};

		private:

			typedef void (NST_FASTCALL Apu::*Updater)(Cycle);

			inline void Update(Cycle);
			void Update();
			void UpdateLatency();
			bool UpdateDelta();

			void Reset(bool,bool);
			void CalculateOscillatorClock(Cycle&,uint&) const;
			void Resync(dword);
			NST_NO_INLINE void ClearBuffers(bool);

			enum
			{
				MAX_CHANNELS            = 11,
				RESET_CYCLES            = 2048,
				STATUS_NO_FRAME_IRQ     = 0x40,
				STATUS_SEQUENCE_5_STEP  = 0x80,
				STATUS_FRAME_IRQ_ENABLE = 0,
				STATUS_BITS             = STATUS_NO_FRAME_IRQ|STATUS_SEQUENCE_5_STEP,
				NLN_VOL                 = 192,
				NLN_SQ_F                = 900,
				NLN_SQ_0                = 9552UL * Channel::OUTPUT_MUL * NLN_VOL * (NLN_SQ_F/100),
				NLN_SQ_1                = 8128UL * Channel::OUTPUT_MUL * NLN_SQ_F,
				NLN_SQ_2                = NLN_SQ_F * 100UL,
				NLN_TND_F               = 500,
				NLN_TND_0               = 16367UL * Channel::OUTPUT_MUL * NLN_VOL * (NLN_TND_F/100),
				NLN_TND_1               = 24329UL * Channel::OUTPUT_MUL * NLN_TND_F,
				NLN_TND_2               = NLN_TND_F * 100UL
			};

			NES_DECL_POKE( 4000 );
			NES_DECL_POKE( 4001 );
			NES_DECL_POKE( 4002 );
			NES_DECL_POKE( 4003 );
			NES_DECL_POKE( 4004 );
			NES_DECL_POKE( 4005 );
			NES_DECL_POKE( 4006 );
			NES_DECL_POKE( 4007 );
			NES_DECL_POKE( 4008 );
			NES_DECL_POKE( 400A );
			NES_DECL_POKE( 400B );
			NES_DECL_POKE( 400C );
			NES_DECL_POKE( 400E );
			NES_DECL_POKE( 400F );
			NES_DECL_POKE( 4010 );
			NES_DECL_POKE( 4011 );
			NES_DECL_POKE( 4012 );
			NES_DECL_POKE( 4013 );
			NES_DECL_POKE( 4015 );
			NES_DECL_PEEK( 4015 );
			NES_DECL_PEEK( 4xxx );

			NST_NO_INLINE Channel::Sample GetSample();

			void NST_FASTCALL SyncOn    (Cycle);
			void NST_FASTCALL SyncOnExt (Cycle);
			void NST_FASTCALL SyncOff   (Cycle);

			NST_NO_INLINE void ClockFrameIRQ(Cycle);
			NST_NO_INLINE void ClockFrameCounter();
			NST_NO_INLINE void ClockDmc(Cycle);
			NST_NO_INLINE void ClockOscillators(bool);

			template<typename T,bool STEREO>
			void FlushSound();

			void UpdateSettings();
			void UpdateSettings(CpuModel);
			void UpdateVolumes();

			struct Cycles
			{
				Cycles();

				void Update(dword,uint,CpuModel);
				void Reset(bool);

				uint fixed;
				Cycle rate;
				Cycle rateCounter;
				Cycle frameCounter;
				Cycle extCounter;
				CpuModel model;
				word frameDivider;
				word frameIrqRepeat;
				Cycle frameIrqClock;
				Cycle dmcClock;

				static const dword frameClocks[2][4];
				static const dword oscillatorClocks[2][2][4];
			};

			class Synchronizer
			{
				uint sync;
				uint duty;
				dword streamed;
				dword rate;

			public:

				Synchronizer();

				void Reset(uint,CpuModel,dword);
				void Resync(uint,CpuModel);
				NST_SINGLE_CALL dword Clock(dword,CpuModel,dword);
			};

			class Oscillator
			{
			protected:

				Oscillator();

				void Reset();
				void UpdateSettings(dword,uint);

				ibool active;
				idword timer;
				Cycle rate;
				Cycle frequency;
				dword amp;
				uint fixed;

			public:

				inline void ClearAmp();
			};

			class Square : public Oscillator
			{
			public:

				void Reset();
				void UpdateSettings(dword,uint,uint);
				void LoadState(State::Loader&);
				void SaveState(State::Saver&,dword) const;

				NST_SINGLE_CALL void WriteReg0(uint);
				NST_SINGLE_CALL void WriteReg1(uint);
				NST_SINGLE_CALL void WriteReg2(uint);
				NST_SINGLE_CALL void WriteReg3(uint,Cycle);
				NST_SINGLE_CALL void Disable(uint);

				dword GetSample();

				NST_SINGLE_CALL void ClockEnvelope();
				NST_SINGLE_CALL void ClockSweep(uint);

				inline uint GetLengthCounter() const;

			private:

				inline bool CanOutput() const;
				void UpdateFrequency();

				enum
				{
					MIN_FRQ               = 0x008,
					MAX_FRQ               = 0x7FF,
					REG0_DUTY_SHIFT       = 6,
					REG1_SWEEP_SHIFT      = 0x07,
					REG1_SWEEP_DECREASE   = 0x08,
					REG1_SWEEP_RATE       = 0x70,
					REG1_SWEEP_RATE_SHIFT = 4,
					REG1_SWEEP_ENABLED    = 0x80,
					REG3_WAVELENGTH_LOW   = 0x00FF,
					REG3_WAVELENGTH_HIGH  = 0x0700
				};

				uint step;
				uint duty;
				Channel::Envelope envelope;
				Channel::LengthCounter lengthCounter;
				bool validFrequency;
				bool sweepReload;
				byte sweepCount;
				byte sweepRate;
				uint sweepIncrease;
				word sweepShift;
				word waveLength;
			};

			class Triangle : public Oscillator
			{
			public:

				Triangle();

				void Reset();
				void UpdateSettings(dword,uint,uint);
				void LoadState(State::Loader&);
				void SaveState(State::Saver&,dword) const;

				NST_SINGLE_CALL void WriteReg0(uint);
				NST_SINGLE_CALL void WriteReg2(uint);
				NST_SINGLE_CALL void WriteReg3(uint,Cycle);
				NST_SINGLE_CALL void Disable(uint);

				NST_SINGLE_CALL dword GetSample();

				NST_SINGLE_CALL void ClockLinearCounter();
				NST_SINGLE_CALL void ClockLengthCounter();

				inline uint GetLengthCounter() const;

			private:

				inline bool CanOutput() const;

				enum
				{
					MIN_FRQ                   = 2 + 1,
					STEP_CHECK                = 0x00, // >= 0x1F is technically correct but will produce clicks/pops
					REG0_LINEAR_COUNTER_LOAD  = 0x7F,
					REG0_LINEAR_COUNTER_START = 0x80,
					REG2_WAVE_LENGTH_LOW      = 0x00FF,
					REG3_WAVE_LENGTH_HIGH     = 0x0700
				};

				enum Status
				{
					STATUS_COUNTING,
					STATUS_RELOAD
				};

				uint step;
				uint outputVolume;
				Status status;
				word waveLength;
				byte linearCtrl;
				byte linearCounter;
				Channel::LengthCounter lengthCounter;
			};

			class Noise : public Oscillator
			{
			public:

				void Reset(CpuModel);
				void UpdateSettings(dword,uint,uint,CpuModel);
				void LoadState(State::Loader&,CpuModel);
				void SaveState(State::Saver&,dword) const;

				NST_SINGLE_CALL void WriteReg0(uint);
				NST_SINGLE_CALL void WriteReg2(uint,CpuModel);
				NST_SINGLE_CALL void WriteReg3(uint,Cycle);
				NST_SINGLE_CALL void Disable(uint);

				NST_SINGLE_CALL dword GetSample();

				NST_SINGLE_CALL void ClockEnvelope();
				NST_SINGLE_CALL void ClockLengthCounter();

				inline uint GetLengthCounter() const;

			private:

				inline bool CanOutput() const;
				uint GetFrequencyIndex() const;

				enum
				{
					REG2_FREQUENCY  = 0x0F,
					REG2_93BIT_MODE = 0x80
				};

				uint bits;
				uint shifter;
				Channel::Envelope envelope;
				Channel::LengthCounter lengthCounter;

				static const word lut[2][16];
			};

			class Dmc
			{
			public:

				Dmc();

				void Reset(CpuModel);
				void UpdateSettings(Cycle&,uint,CpuModel);
				void LoadState(State::Loader&,const Cpu&,CpuModel,Cycle&);
				void SaveState(State::Saver&,dword,const Cpu&,Cycle) const;

				NST_SINGLE_CALL bool WriteReg0(uint,CpuModel);
				NST_SINGLE_CALL void WriteReg1(uint);
				NST_SINGLE_CALL void WriteReg2(uint);
				NST_SINGLE_CALL void WriteReg3(uint);
				NST_SINGLE_CALL void Disable(uint,Cpu&);

				NST_SINGLE_CALL dword GetSample();

				NST_SINGLE_CALL bool ClockDAC();
				NST_SINGLE_CALL void Update();
				NST_SINGLE_CALL void ClockDMA(Cpu&,Cycle&);

				inline void ClearAmp();
				inline uint GetLengthCounter() const;

				static Cycle GetResetFrequency(CpuModel);

			private:

				void DoDMA(Cpu&,Cycle);

				enum
				{
					REG0_FREQUENCY  = 0x0F,
					REG0_LOOP       = 0x40,
					REG0_IRQ_ENABLE = 0x80,
					INP_STEP        = 8
				};

				uint curSample;
				uint linSample;
				uint outputVolume;
				Cycle frequency;

				struct
				{
					uint ctrl;
					word lengthCounter;
					word address;
				}   regs;

				struct
				{
					byte shifter;
					byte dac;
					byte buffer;
					bool active;
				}   out;

				struct
				{
					word lengthCounter;
					word address;
					word buffered;
					word buffer;
				}   dma;

				static const word lut[2][16];
			};

			struct Settings
			{
				Settings();

				dword rate;
				byte bits;
				byte speed;
				bool transpose;
				bool stereo;
				bool audible;
				byte volumes[MAX_CHANNELS];
			};

			uint ctrl;
			Updater updater;
			Cpu& cpu;
			Cycles cycles;
			Synchronizer synchronizer;
			Square square[2];
			Triangle triangle;
			Noise noise;
			Dmc dmc;
			Channel* extChannel;
			Channel::DcBlocker dcBlocker;
			Sound::Output* stream;
			Sound::Buffer buffer;
			Settings settings;

		public:

			dword GetSampleRate() const
			{
				return settings.rate;
			}

			uint GetSampleBits() const
			{
				return settings.bits;
			}

			uint GetSpeed() const
			{
				return settings.speed;
			}

			bool IsAutoTransposing() const
			{
				return settings.transpose;
			}

			bool InStereo() const
			{
				return settings.stereo;
			}

			bool IsAudible() const
			{
				return settings.audible;
			}
		};
	}
}
