/**
 * Gens: Option adjustment functions.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "g_main.hpp"
#include "options.hpp"
#include "gens.hpp"
#include "g_md.hpp"
#include "g_mcd.hpp"
#include "g_32x.hpp"
#include "segacd/cd_sys.hpp"
#include "macros/math_m.h"

#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_m68k_cd.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/mem/mem_sh2.h"

#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/sound/pwm.h"

#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_32x.h"

#include "gens_core/misc/misc.h"
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "util/file/save.hpp"

#include "ui/gens_ui.hpp"
#include "gens/gens_window_sync.hpp"


/**
 * spriteLimit(): Get the current sprite limit setting.
 * @return Sprite limit setting.
 */
bool Options::spriteLimit(void)
{
	return Sprite_Over;
}

/**
 * setSpriteLimit(): Enable/Disable the sprite limit.
 * @param newSpriteLimit True to enable; False to disable.
 */
void Options::setSpriteLimit(const bool newSpriteLimit)
{
	Sprite_Over = newSpriteLimit;
	
	if (Sprite_Over)
		MESSAGE_L("Sprite Limit Enabled", "Sprite Limit Enabled", 1000);
	else
		MESSAGE_L("Sprite Limit Disabled", "Sprite Limit Disabled", 1000);
}


/**
 * saveSlot(): Get the current savestate slot.
 * @return Current savestate slot.
 */
int Options::saveSlot(void)
{
	return Current_State;
}

/**
 * setSaveSlot(): Set the current savestate slot.
 * @param newSaveSlot Savestate slot.
 */
void Options::setSaveSlot(const int newSaveSlot)
{
	// Make sure the slot number is in bounds.
	if (newSaveSlot < 0 || newSaveSlot > 9)
		return;
	
	// Set the new savestate slot number.
	Current_State = newSaveSlot;
	
	// TODO: Change this to just check if the file exists.
	FILE *f;
	if ((f = Savestate::getStateFile()))
	{
		fclose(f);
		MESSAGE_NUM_L("SLOT %d [OCCUPIED]", "SLOT %d [OCCUPIED]", Current_State, 1500);
	}
	else
	{
		MESSAGE_NUM_L("SLOT %d [EMPTY]", "SLOT %d [EMPTY]", Current_State, 1500);
	}
}


/**
 * frameSkip(): Get the frame skip setting.
 * @return Frame skip setting. (-1 == Auto)
 */
int Options::frameSkip(void)
{
	return Frame_Skip;
}

/**
 * setFrameSkip(): Set the frame skip setting.
 * @param newFrameSkip Frames to skip. (-1 == Auto)
 */
void Options::setFrameSkip(const int newFrameSkip)
{
	// Make sure the frame skip setting is in bounds.
	if (newFrameSkip < -1 || newFrameSkip > 8)
		return;
	
	Frame_Skip = newFrameSkip;
	
	if (Frame_Skip != -1)
		MESSAGE_NUM_L("Frame skip set to %d", "Frame skip set to %d", Frame_Skip, 1500);
	else
		MESSAGE_L("Frame skip set to Auto", "Frame skip set to Auto", 1500);
}

#ifdef GENS_DEBUGGER

/**
 * debugMode(): Get the current debugging mode.
 * @return Current debugging mode.
 */
DEBUG_MODE Options::debugMode(void)
{
	return (DEBUG_MODE)Debug;
}

/**
 * setDebugMode(): Set the current debugging mode.
 * @param newDebugMode New debug mode.
 */
void Options::setDebugMode(const DEBUG_MODE newDebugMode)
{
	if (!Game)
		return;
	
	Flag_Clr_Scr = 1;
	audio->clearSoundBuffer();
	
	if ((int)newDebugMode == Debug)
		Debug = 0;
	else
		Debug = (int)newDebugMode;
}

#endif
/**
 * country(): Get the current country code.
 * @return Current country code.
 */
int Options::country(void)
{
	return Country;
}

/**
 * setCountry(): Set the current country code.
 * @param newCountry New country code.
 */
void Options::setCountry(const int newCountry)
{
	unsigned char Reg_1[0x200];
	
	Flag_Clr_Scr = 1;
	
	Country = newCountry;
	switch (Country)
	{
		default:
		case -1:
			// Auto-detect.
			if (Genesis_Started || _32X_Started)
				Detect_Country_Genesis();
			else if (SegaCD_Started)
				Detect_Country_SegaCD();
			break;
		
		case 0:
			// Japan (NTSC)
			Game_Mode = 0;
			CPU_Mode = 0;
			break;
		
		case 1:
			// USA (NTSC)
			Game_Mode = 1;
			CPU_Mode = 0;
			break;
		
		case 2:
			// Europe (PAL)
			Game_Mode = 1;
			CPU_Mode = 1;
			break;
		
		case 3:
			// Japan (PAL)
			Game_Mode = 0;
			CPU_Mode = 1;
			break;
	}
	
	// TODO: Combine this with gens.cpp:Set_Clock_Freq().
	if (CPU_Mode)
	{
		CPL_Z80 = roundDouble((((double)CLOCK_PAL / 15.0) / 50.0) / 312.0);
		CPL_M68K = roundDouble((((double)CLOCK_PAL / 7.0) / 50.0) / 312.0);
		CPL_MSH2 = roundDouble(((((((double)CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) *
					(double)MSH2_Speed) / 100.0);
		CPL_SSH2 = roundDouble(((((((double)CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) *
					(double)SSH2_Speed) / 100.0);
		
		VDP_Num_Lines = 312;
		VDP_Status |= 0x0001;
		_32X_VDP.Mode &= ~0x8000;
		
		CD_Access_Timer = 2080;
		Timer_Step = 136752;
	}
	else
	{
		CPL_Z80 = roundDouble((((double)CLOCK_NTSC / 15.0) / 60.0) / 262.0);
		CPL_M68K = roundDouble((((double)CLOCK_NTSC / 7.0) / 60.0) / 262.0);
		CPL_MSH2 = roundDouble(((((((double)CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) *
					(double)MSH2_Speed) / 100.0);
		CPL_SSH2 = roundDouble(((((((double) CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) *
					(double)SSH2_Speed) / 100.0);
		
		VDP_Num_Lines = 262;
		VDP_Status &= 0xFFFE;
		_32X_VDP.Mode |= 0x8000;
		
		CD_Access_Timer = 2096;
		Timer_Step = 135708;
	}
	
	if (audio->enabled())
	{
		PSG_Save_State();
		YM2612_Save(Reg_1);
		
		audio->endSound();
		audio->setEnabled(false);
		
		if (CPU_Mode)
		{
			YM2612_Init(CLOCK_PAL / 7, audio->soundRate(), YM2612_Improv);
			PSG_Init(CLOCK_PAL / 15, audio->soundRate());
		}
		else
		{
			YM2612_Init(CLOCK_NTSC / 7, audio->soundRate(), YM2612_Improv);
			PSG_Init(CLOCK_NTSC / 15, audio->soundRate());
		}
		
		if (SegaCD_Started)
			Set_Rate_PCM(audio->soundRate());
		
		YM2612_Restore (Reg_1);
		PSG_Restore_State();
		
		if (!audio->initSound())
			return;
		
		audio->setEnabled(true);
		audio->playSound();
	}
	
	if (Game_Mode)
	{
		if (CPU_Mode)
			MESSAGE_L("Europe system (50 FPS)", "Europe system (50 FPS)", 1500);
		else
			MESSAGE_L("USA system (60 FPS)", "USA system (60 FPS)", 1500);
	}
	else
	{
		if (CPU_Mode)
			MESSAGE_L("Japan system (50 FPS)", "Japan system (50 FPS)", 1500);
		else
			MESSAGE_L("Japan system (60 FPS)", "Japan system (60 FPS)", 1500);
	}
	
	setGameName();
	return;
}


/** Sound **/


/**
 * soundEnable(): Get the sound enable setting.
 * @return Sound enable setting.
 */
bool Options::soundEnable(void)
{
	return audio->enabled();
}

/**
 * setSoundEnable(): Enable or disable sound.
 * @param newSoundEnable New sound enable setting.
 * @return 1 on success; 0 on error.
 */
int Options::setSoundEnable(const bool newSoundEnable)
{
	audio->setEnabled(newSoundEnable);
	
	if (!audio->enabled())
	{
		audio->endSound();
		YM2612_Enable = 0;
		PSG_Enable = 0;
		DAC_Enable = 0;
		PCM_Enable = 0;
		PWM_Enable = 0;
		CDDA_Enable = 0;
		MESSAGE_L("Sound Disabled", "Sound Disabled", 1500);
	}
	else
	{
		if (!audio->initSound())
		{
			// Error initializing sound.
			audio->setEnabled(false);
			YM2612_Enable = 0;
			PSG_Enable = 0;
			DAC_Enable = 0;
			PCM_Enable = 0;
			PWM_Enable = 0;
			CDDA_Enable = 0;
			return 0;
		}
		
		audio->playSound();
		
		// Make sure Z80 sound emulation is enabled.
		if (!(Z80_State & 1))
			setSoundZ80(true);
		
		YM2612_Enable = 1;
		PSG_Enable = 1;
		DAC_Enable = 1;
		PCM_Enable = 1;
		PWM_Enable = 1;
		CDDA_Enable = 1;
		
		MESSAGE_L("Sound Enabled", "Sound Enabled", 1500);
	}
	
	return 1;
}


/**
 * soundStereo(): Get the stereo sound setting.
 * @return Stereo sound setting.
 */
bool Options::soundStereo(void)
{
	return audio->stereo();
}

/**
 * setSoundStereo(): Enable or disable stereo sound.
 * @param newSoundStereo New stereo sound enable setting.
 */
void Options::setSoundStereo(const bool newSoundStereo)
{
	// TODO: Move most of this code to the Audio class.
	unsigned char Reg_1[0x200];
	
	audio->setStereo(newSoundStereo);
	
	if (!audio->stereo())
		MESSAGE_L("Mono sound", "Mono sound", 1000);
	else
		MESSAGE_L("Stereo sound", "Stereo sound", 1000);
	
	if (!audio->enabled())
	{
		// Sound isn't enabled, so nothing needs to be changed.
		return;
	}
	
	// Save the current sound state.
	// TODO: Use full save instead of partial save?
	PSG_Save_State();
	YM2612_Save(Reg_1);
	
	// Temporarily disable sound.
	audio->endSound();
	audio->setEnabled(false);
	
	// Reinitialize the sound processors.
	if (CPU_Mode)
	{
		YM2612_Init(CLOCK_PAL / 7, audio->soundRate(), YM2612_Improv);
		PSG_Init(CLOCK_PAL / 15, audio->soundRate());
	}
	else
	{
		YM2612_Init(CLOCK_NTSC / 7, audio->soundRate(), YM2612_Improv);
		PSG_Init(CLOCK_NTSC / 15, audio->soundRate());
	}
	
	if (SegaCD_Started)
		Set_Rate_PCM(audio->soundRate());
	
	// Restore the sound state.
	YM2612_Restore(Reg_1);
	PSG_Restore_State();
	
	// Attempt to re-enable sound.
	if (!audio->initSound())
		return;
	
	// Sound enabled.
	audio->setEnabled(true);
	audio->playSound();
	return;
}


/**
 * soundZ80(): Get the Z80 sound emulation setting.
 * @return Z80 sound emulation setting.
 */
bool Options::soundZ80(void)
{
	return (Z80_State & 1);
}

/**
 * setSoundZ80(): Enable or disable Z80 sound emulation.
 * @param newZ80 New Z80 sound enable setting.
 */
void Options::setSoundZ80(const bool newSoundZ80)
{
	if (newSoundZ80)
	{
		Z80_State |= 1;
		MESSAGE_L("Z80 Enabled", "Z80 Enabled", 1000);
	}
	else
	{
		Z80_State &= ~1;
		MESSAGE_L("Z80 Disabled", "Z80 Disabled", 1000);
	}
}


/**
 * soundYM2612(): Get the YM2612 sound emulation setting.
 * @return YM2612 sound emulation setting.
 */
bool Options::soundYM2612(void)
{
	return YM2612_Enable;
}

/**
 * setSoundYM2612(): Enable or disable YM2612 sound emulation.
 * @param newSoundYM2612 New YM2612 sound enable setting.
 */
void Options::setSoundYM2612(const bool newSoundYM2612)
{
	YM2612_Enable = newSoundYM2612;
	
	if (YM2612_Enable)
		MESSAGE_L("YM2612 Enabled", "YM2612 Enabled", 1000);
	else
		MESSAGE_L("YM2612 Disabled", "YM2612 Disabled", 1000);
}


/**
 * soundYM2612_Improved(): Get the improved YM2612 sound emulation setting.
 * @return Improved YM2612 sound emulation setting.
 */
bool Options::soundYM2612_Improved(void)
{
	return YM2612_Improv;
}

/**
 * setSoundYM2612_Improved(): Enable or disable improved YM2612 sound emulation.
 * @param newSoundYM2612_Improved New improved YM2612 sound enable setting.
 */
void Options::setSoundYM2612_Improved(const bool newSoundYM2612_Improved)
{
	unsigned char Reg_1[0x200];
	
	YM2612_Improv = newSoundYM2612_Improved;
	
	if (YM2612_Improv)
	{
		MESSAGE_L("High Quality YM2612 emulation",
			  "High Quality YM2612 emulation", 1000);
	}
	else
	{
		MESSAGE_L("Normal YM2612 emulation",
			  "Normal YM2612 emulation", 1000);
	}
	
	// Save the YM2612 registers.
	// TODO: Use full save instead of partial save?
	YM2612_Save(Reg_1);
	
	if (CPU_Mode)
		YM2612_Init(CLOCK_PAL / 7, audio->soundRate(), YM2612_Improv);
	else
		YM2612_Init(CLOCK_NTSC / 7, audio->soundRate(), YM2612_Improv);
	
	// Restore the YM2612 registers.
	YM2612_Restore(Reg_1);
}


/**
 * soundDAC(): Get the DAC sound emulation setting.
 * @return DAC sound emulation setting.
 */
bool Options::soundDAC(void)
{
	return DAC_Enable;
}

/**
 * setSoundDAC(): Enable or disable DAC sound emulation.
 * @param newSoundDAC New DAC sound emulation setting.
 */
void Options::setSoundDAC(const bool newSoundDAC)
{
	DAC_Enable = newSoundDAC;
	
	if (DAC_Enable)
		MESSAGE_L("DAC Enabled", "DAC Enabled", 1000);
	else
		MESSAGE_L("DAC Disabled", "DAC Disabled", 1000);
}


/**
 * soundDAC_Improved(): Get the improved DAC sound emulation setting.
 * @return Improved DAC sound emulation setting.
 */
bool Options::soundDAC_Improved(void)
{
	return DAC_Improv;
}

/**
 * setSoundDAC_Improved(): Enable or disable improved DAC sound emulation.
 * @param newDACImproved New improved DAC sound emulation setting.
 */
void Options::setSoundDAC_Improved(const bool newSoundDAC_Improved)
{
	DAC_Improv = newSoundDAC_Improved;
	
	if (DAC_Improv)
	{
		MESSAGE_L("Improved DAC sound (voices)",
			  "Improved DAC sound (voices)", 1000);
	}
	else
	{
		MESSAGE_L("Normal DAC sound",
			  "Normal DAC sound", 1000);
	}
}


/**
 * soundPSG(): Get the PSG sound emulation setting.
 * @return PSG sound emulation setting.
 */
bool Options::soundPSG(void)
{
	return PSG_Enable;
}

/**
 * setSoundPSG(): Enable or disable PSG sound emulation.
 * @param newSoundPSG New PSG sound emulation setting.
 */
void Options::setSoundPSG(const bool newSoundPSG)
{
	PSG_Enable = newSoundPSG;
	
	if (PSG_Enable)
		MESSAGE_L("PSG Enabled", "PSG Enabled", 1000);
	else
		MESSAGE_L("PSG Disabled", "PSG Disabled", 1000);
}


/**
 * soundPSG_Sine(): Get the sine wave PSG sound emulation setting.
 * @return Sine wave PSG sound emulation setting.
 */
bool Options::soundPSG_Sine(void)
{
	return PSG_Improv;
}

/**
 * setSoundPSG_Sine(): Enable or disable sine wave PSG sound emulation.
 * @param newSoundPSG_Sine New sine wave PSG sound emulation setting.
 */
void Options::setSoundPSG_Sine(const bool newSoundPSG_Sine)
{
	PSG_Improv = newSoundPSG_Sine;
	
	if (PSG_Improv)
		MESSAGE_L("Sine wave PSG sound", "Sine wave PSG sound", 1000);
	else
		MESSAGE_L("Normal PSG sound", "Normal PSG sound", 1000);
}


/**
 * soundPCM(): Get the PCM sound emulation setting.
 * @return PCM sound emulation setting.
 */
bool Options::soundPCM(void)
{
	return PCM_Enable;
}

/**
 * Change_PCM(): Enable or disable PCM sound emulation.
 * @param newSoundPCM New PCM sound emulation setting.
 */
void Options::setSoundPCM(const bool newSoundPCM)
{
	PCM_Enable = newSoundPCM;
	
	if (PCM_Enable)
		MESSAGE_L("PCM Sound Enabled", "PCM Sound Enabled", 1000);
	else
		MESSAGE_L("PCM Sound Disabled", "PCM Sound Disabled", 1000);
}


/**
 * soundPWM(): Get the PWM sound emulation setting.
 * @return PWM sound emulation setting.
 */
bool Options::soundPWM(void)
{
	return PWM_Enable;
}

/**
 * setSoundPWM(): Enable or disable PWM sound emulation.
 * @param newPWM New PWM sound emulation setting.
 */
void Options::setSoundPWM(const bool newSoundPWM)
{
	PWM_Enable = newSoundPWM;
	
	if (PWM_Enable)
		MESSAGE_L("PWM Sound Enabled", "PWM Sound Enabled", 1000);
	else
		MESSAGE_L("PWM Sound Disabled", "PWM Sound Disabled", 1000);
}


/**
 * soundCDDA(): Get the CDDA (CD Audio) sound emulation setting.
 * @return CDDA sound emulation setting.
 */
bool Options::soundCDDA(void)
{
	return CDDA_Enable;
}

/**
 * setSoundCDDA(): Enable or disable CDDA (CD Audio).
 * @param newCDDA New CDDA setting.
 * @return 1 on success.
 */
void Options::setSoundCDDA(const bool newSoundCDDA)
{
	CDDA_Enable = newSoundCDDA;
	
	if (CDDA_Enable)
		MESSAGE_L("CD Audio Enabled", "CD Audio Enabled", 1000);
	else
		MESSAGE_L("CD Audio Disabled", "CD Audio Disabled", 1000);
}


/**
 * soundSampleRate(): Get the current sample rate ID.
 * @return Current sample rate ID. (TODO: Make an enum containing the rate IDs.)
 */
int Options::soundSampleRate(void)
{
	switch (audio->soundRate())
	{
		case 11025:
			return 0;
		case 22050:
			return 1;
		case 44100:
			return 2;
		default:
			return -1;
	}
	
	return -1;
}

/**
 * setSoundSampleRate(): Change the sample rate.
 * @param Rate Rate ID. (TODO: Make an enum containing the rate IDs.)
 */
void Options::setSoundSampleRate(const int newRate)
{
	// Make sure the rate ID is valid.
	assert(newRate >= 0 && newRate <= 2);
	
	switch (newRate)
	{
		case 0:
			audio->setSoundRate(11025);
			break;
		case 1:
			audio->setSoundRate(22050);
			break;
		case 2:
			audio->setSoundRate(44100);
			break;
	}
	MESSAGE_NUM_L("Sound rate set to %d Hz", "Sound rate set to %d Hz", audio->soundRate(), 2500);
	
	// If sound isn't enabled, we're done.
	if (!audio->enabled())
		return;
	
	// Sound's enabled. Reinitialize it.
	unsigned char Reg_1[0x200];
	
	// Save the sound registers.
	// TODO: Use a full save instead of a partial save?
	PSG_Save_State();
	YM2612_Save(Reg_1);
	
	// Stop sound.
	audio->endSound();
	audio->setEnabled(false);
	
	// Reinitialize the sound processors.
	if (CPU_Mode)
	{
		YM2612_Init(CLOCK_PAL / 7, audio->soundRate(), YM2612_Improv);
		PSG_Init(CLOCK_PAL / 15, audio->soundRate());
	}
	else
	{
		YM2612_Init(CLOCK_NTSC / 7, audio->soundRate(), YM2612_Improv);
		PSG_Init(CLOCK_NTSC / 15, audio->soundRate());
	}

	if (SegaCD_Started)
		Set_Rate_PCM(audio->soundRate());
	
	// Restore the sound registers
	YM2612_Restore(Reg_1);
	PSG_Restore_State();
	
	// Attempt to reinitialize sound.
	if (!audio->initSound())
		return;
	
	// Sound is reinitialized.
	audio->setEnabled(true);
	audio->playSound();
	
	return;
}


/** SegaCD **/


/**
 * segaCD_SRAMSize(): Get the SegaCD SRAM Size ID.
 * @return SegaCD SRAM Size ID. (TODO: Make this an enum?)
 */
int segaCD_SRAMSize(void)
{
	if (BRAM_Ex_State & 0x100)
		return BRAM_Ex_Size;
	else
		return -1;
}

/**
 * setSegaCD_SRAMSize(): Change the Sega CD SRAM size.
 * @param num SegaCD SRAM Size ID. (TODO: Make this an enum?)
 */
void Options::setSegaCD_SRAMSize(const int num)
{
	if (num < -1 || num > 3)
		return;
	
	if (num == -1)
	{
		BRAM_Ex_State &= 1;
		MESSAGE_L("SegaCD SRAM cart removed", "SegaCD SRAM cart removed", 1500);
	}
	else
	{
		char bsize[256];
		
		BRAM_Ex_State |= 0x100;
		BRAM_Ex_Size = num;
		
		sprintf(bsize, "SegaCD SRAM cart plugged in (%d KB)", 8 << num);
		MESSAGE_L(bsize, bsize, 1500);
	}
}


/**
 * segaCD_PerfectSync(): Get SegaCD Perfect Sync.
 * @return SegaCD Perfect Sync.
 */
bool Options::segaCD_PerfectSync(void)
{
	return SegaCD_Accurate;
}

/**
 * setSegaCD_PerfectSync(): Set SegaCD Perfect Sync.
 * @param newPerfectSync True to enable; False to disable.
 */
void Options::setSegaCD_PerfectSync(const bool newSegaCD_PerfectSync)
{
	SegaCD_Accurate = newSegaCD_PerfectSync;
	
	if (!SegaCD_Accurate)
	{
		// SegaCD Perfect Sync disabled.
		if (SegaCD_Started)
		{
			Update_Frame = Do_SegaCD_Frame;
			Update_Frame_Fast = Do_SegaCD_Frame_No_VDP;
		}
	
		MESSAGE_L("SegaCD normal mode", "SegaCD normal mode", 1500);
	}
	else
	{
		// SegaCD Perfect Sync enabled.
		
		if (SegaCD_Started)
		{
			Update_Frame = Do_SegaCD_Frame_Cycle_Accurate;
			Update_Frame_Fast = Do_SegaCD_Frame_No_VDP_Cycle_Accurate;
		}
		
		MESSAGE_L("SegaCD Perfect Sync mode (slower)",
			  "SegaCD Perfect Sync mode (slower)", 1500);
	}
}


/** Graphics **/

/**
 * fastBlur(): Get the Fast Blur setting.
 * @return True if Fast Blur is enabled; False if Fast Blur is disabled.
 */
bool Options::fastBlur(void)
{
	return draw->fastBlur();
}

/**
 * setFastBlur() Set the Fast Blur setting.
 * @param newFastBlur True to enable Fast Blur; False to disable Fast Blur.
 */
void Options::setFastBlur(const bool newFastBlur)
{
	Flag_Clr_Scr = 1;
	draw->setFastBlur(newFastBlur);
	
	if (draw->fastBlur())
		MESSAGE_L("Fast Blur Enabled", "Fast Blur Enabled", 1000);
	else
		MESSAGE_L("Fast Blur Disabled", "Fast Blur Disabled", 1000);
}


/**
 * stretch(): Get the current Stretch setting.
 * @return Current Stretch setting.
 */
uint8_t Options::stretch(void)
{
	return draw->stretch();
}

/**
 * setStretch(): Set the Stretch setting.
 * @param newStretch New Stretch setting.
 */
void Options::setStretch(const uint8_t newStretch)
{
	if (newStretch > VDraw::STRETCH_FULL)
	{
		// TODO: Throw an exception.
		return;
	}
	
	Flag_Clr_Scr = 1;
	draw->setStretch(newStretch);
	
	switch (draw->stretch())
	{
		case VDraw::STRETCH_NONE:
			MESSAGE_L("Correct ratio mode", "Correct ratio mode", 1000);
			break;
		case VDraw::STRETCH_H:
			MESSAGE_L("Horizontal stretched mode", "Horizontal stretched mode", 1000);
			break;
		case VDraw::STRETCH_V:
			MESSAGE_L("Vertical stretched mode", "Vertical stretched mode", 1000);
			break;
		case VDraw::STRETCH_FULL:
			MESSAGE_L("Full stretched mode", "Full stretched mode", 1000);
			break;
	}
}


/**
 * vsync(): Get the VSync setting.
 * @return True if VSync is enabled; False if VSync is disabled.
 */
bool Options::vsync(void)
{
	int *p_vsync;
	
	p_vsync = (draw->fullScreen() ? &Video.VSync_FS : &Video.VSync_W);
	return *p_vsync;
}

/**
 * setVSync(): Set the VSync setting.
 * @param vsync True to enable VSync; False to disable VSync.
 */
void Options::setVSync(const bool newVSync)
{
	int *p_vsync;
	
	p_vsync = (draw->fullScreen() ? &Video.VSync_FS : &Video.VSync_W);
	*p_vsync = (newVSync == 1 ? 1 : 0);
	
	if (*p_vsync)
		MESSAGE_L("Vertical Sync Enabled", "Vertical Sync Enabled", 1000);
	else
		MESSAGE_L("Vertical Sync Disabled", "Vertical Sync Disabled", 1000);
	
	// Update VSync.
	draw->updateVSync();
}


#ifdef GENS_OPENGL

#include "video/v_draw_sdl.hpp"
#include "video/v_draw_sdl_gl.hpp"

/**
 * openGL(): Get the OpenGL setting.
 * @return OpenGL setting.
 */
bool Options::openGL(void)
{
	return Video.OpenGL;
}

/**
 * setOpenGL(): Change the OpenGL setting.
 * @param newOpenGL True to enable OpenGL; False to disable OpenGL.
 */
void Options::setOpenGL(const bool newOpenGL)
{
	// End the current drawing function.
	draw->End_Video();
	
	Video.OpenGL = newOpenGL;
	VDraw *newDraw;
	if (Video.OpenGL)
	{
		newDraw = new VDraw_SDL_GL(draw);
		newDraw->Init_Video();
		delete draw;
		draw = newDraw;
		MESSAGE_L("Selected OpenGL Renderer", "Selected OpenGL Renderer", 1500);
	}
	else
	{
		newDraw = new VDraw_SDL(draw);
		newDraw->Init_Video();
		delete draw;
		draw = newDraw;
		MESSAGE_L("Selected SDL Renderer", "Selected SDL Renderer", 1500);
	}
}


/**
 * setOpenGL_Resolution(): Set the OpenGL resolution.
 * @param w Width.
 * @param h Height.
 */
void Options::setOpenGL_Resolution(int w, int h)
{
	// TODO: Move this to VDraw_GL.
	
	if (Video.Width_GL == w && Video.Height_GL == h)
		return;
	
	// OpenGL resolution has changed.
	Video.Width_GL = w;
	Video.Height_GL = h;
	
	// Print the resolution information.
	MESSAGE_NUM_2L("Selected %dx%d resolution",
		       "Selected %dx%d resolution", w, h, 1500);
	
	// If OpenGL mode isn't enabled, don't do anything.
	if (!Video.OpenGL)
		return;
	
	// OpenGL mode is currently enabled. Change the resolution.
	int rendMode = (draw->fullScreen() ? Video.Render_FS : Video.Render_W);
	draw->setRender(rendMode, true);
	
	// Synchronize the Graphics Menu.
	Sync_Gens_Window_GraphicsMenu();
}
#endif /* GENS_OPENGL */


#ifdef GENS_OS_WIN32
/**
 * swRender(): Get the software rendering setting. (Win32 only)
 * @return Software rendering setting.
 */
bool Options::swRender(void)
{
	return draw->swRender();
}

/**
 * setSwRender(): Force software rendering in fullscreen mode. (Win32 only)
 * @param newSwRender New software rendering setting.
 */
void Options::setSwRender(const bool newSwRender)
{
	// TODO: Specify software blit in the parameter.
	
	Flag_Clr_Scr = 1;
	
	draw->setSwRender(newSwRender);
	
	if (draw->swRender())
		MESSAGE_L("Force software blit for Full-Screen",
			  "Force software blit for Full-Screen", 1000);
	else
		MESSAGE_L("Enable hardware blit for Full-Screen",
			  "Enable hardware blit for Full-Screen", 1000);
}
#endif /* GENS_OS_WIN32 */


/**
 * systemReset(): Reset the active system.
 */
void Options::systemReset(void)
{
	/*
	if (Check_If_Kaillera_Running())
	return 0;
	*/
	
	if (Genesis_Started)
	{
		Reset_Genesis();
		MESSAGE_L("Genesis reset", "Genesis reset", 1500);
	}
	else if (SegaCD_Started)
	{
		Reset_SegaCD();
		MESSAGE_L("SegaCD reset", "SegaCD reset", 1500);
	}
	else if (_32X_Started)
	{
		Reset_32X();
		MESSAGE_L("32X reset", "32X reset", 1500);
	}
}


/**
 * setGameName(): Sets the title of the window to the system name and the game name.
 */
void Options::setGameName(void)
{
#if !defined(GENS_UI_OPENEMU)
	string systemName;
	string gameName;
	string emptyGameName;
	
	if (Genesis_Started)
	{
		if ((CPU_Mode == 1) || (Game_Mode == 0))
			systemName = "Mega Drive";
		else
			systemName = "Genesis";
		gameName = Game->ROM_Name_W;
	}
	else if (_32X_Started)
	{
		if (CPU_Mode == 1)
			systemName = "32X (PAL)";
		else
			systemName = "32X (NTSC)";
		gameName = Game->ROM_Name_W;
	}
	else if (SegaCD_Started)
	{
		if ((CPU_Mode == 1) || (Game_Mode == 0))
			systemName = "MegaCD";
		else
			systemName = "SegaCD";
		gameName = ROM_Name;
		emptyGameName = "No Disc";
	}
	else
	{
		GensUI::setWindowTitle_Idle();
		return;
	}

	GensUI::setWindowTitle_Game(systemName, gameName, emptyGameName);
#endif
}
