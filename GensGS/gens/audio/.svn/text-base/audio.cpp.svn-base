/**
 * Gens: Audio base class.
 */


#include <stdio.h>
#include <string.h>

#include "audio.hpp"
#include "emulator/gens.hpp"
#include "emulator/g_main.hpp"


// TODO: Should these be kept out of the Audio class?
int Seg_L[882], Seg_R[882];
unsigned int Sound_Extrapol[312][2];


Audio::Audio()
{
	// Initialize variables.
	m_soundInitialized = false;
	Seg_To_Buffer = 8; // for frame skip
	m_soundRate = 44100;
	Sound_Segs = 8;
	Bytes_Per_Unit = 4;
	
	m_enabled = true;
	m_stereo = true;
	m_soundIsPlaying = false;
	m_dumpingWAV = false;
	m_playingGYM = false;
}


Audio::~Audio()
{
}


/**
 * writeSoundStereo(): Write a stereo sound segment.
 * @param dest Destination buffer.
 * @param length Length of the sound segment. (TODO: Unused?)
 */
void Audio::writeSoundStereo(short *dest, int length)
{
	int i, out_L, out_R;

	for (i = 0; i < m_segLength; i++)
	{
		// Left channel
		out_L = Seg_L[i];
		Seg_L[i] = 0;
		
		if (out_L < -0x7FFF)
			*dest++ = -0x7FFF;
		else if (out_L > 0x7FFF)
			*dest++ = 0x7FFF;
		else
			*dest++ = static_cast<short>(out_L);
		
		// Right channel
		out_R = Seg_R[i];
		Seg_R[i] = 0;
		
		if (out_R < -0x7FFF)
			*dest++ = -0x7FFF;
		else if (out_R > 0x7FFF)
			*dest++ = 0x7FFF;
		else
			*dest++ = static_cast<short>(out_R);
	}
}


/**
 * dumpSoundStereo(): Dump a stereo sound segment.
 * @param dest Destination buffer.
 * @param length Length of the sound segment. (TODO: Unused?)
 */
void Audio::dumpSoundStereo(short *dest, int length)
{
	int i, out_L, out_R;
	
	for (i = 0; i < m_segLength; i++)
	{
		// Left channel
		out_L = Seg_L[i];
		
		if (out_L < -0x7FFF)
			*dest++ = -0x7FFF;
		else if (out_L > 0x7FFF)
			*dest++ = 0x7FFF;
		else
			*dest++ = static_cast<short>(out_L);
		
		// Right channel
		out_R = Seg_R[i];
		
		if (out_R < -0x7FFF)
			*dest++ = -0x7FFF;
		else if (out_R > 0x7FFF)
			*dest++ = 0x7FFF;
		else
			*dest++ = static_cast<short>(out_R);
	}
}


/**
 * writeSoundMono(): Write a mono sound segment.
 * @param dest Destination buffer.
 * @param length Length of the sound segment. (TODO: Unused?)
 */
void Audio::writeSoundMono(short *dest, int length)
{
	int i, out;
	
	for (i = 0; i < m_segLength; i++)
	{
		out = Seg_L[i] + Seg_R[i];
		Seg_L[i] = Seg_R[i] = 0;
		
		if (out < -0xFFFF)
			*dest++ = -0x7FFF;
		else if (out > 0xFFFF)
			*dest++ = 0x7FFF;
		else
			*dest++ = static_cast<short>(out >> 1);
	}
}


/**
 * dumpSoundMono(): Dump a mono sound segment.
 * @param dest Destination buffer.
 * @param length Length of the sound segment. (TODO: Unused?)
 */
void Audio::dumpSoundMono(short *dest, int length)
{
	int i, out;
	
	for (i = 0; i < m_segLength; i++)
	{
		out = Seg_L[i] + Seg_R[i];
		
		if (out < -0xFFFF)
			*dest++ = -0x7FFF;
		else if (out > 0xFFFF)
			*dest++ = 0x7FFF;
		else
			*dest++ = static_cast<short>(out >> 1);
	}
}


// TODO: Move the WAV functions to util/file/wav.c(pp).


int Audio::startWAVDump(void)
{
	STUB;
#if 0
	char Name[1024] = "";
	
	if (!Sound_Is_Playing || !Game)
		return 0;
	
	if (WAV_Dumping)
	{
		draw->writeText("WAV sound is already dumping", 1000);
		return 0;
	}
	
	strcpy(Name, Dump_Dir);
	strcat(Name, Rom_Name);
	
	if (WaveCreateFile(Name, &MMIOOut, &MainWfx, &CkOut, &CkRIFF))
	{
		draw->writeText("Error in WAV dumping", 1000);
		return 0;
	}
	
	if (WaveStartDataWrite(&MMIOOut, &CkOut, &MMIOInfoOut))
	{
		draw->writeText("Error in WAV dumping", 1000);
		return 0;
	}
	
	draw->writeText("Starting to dump WAV sound", 1000);
	WAV_Dumping = 1;
#endif
	return 1;
}


int Audio::updateWAVDump(void)
{
	STUB;
#if 0
	unsigned char Buf_Tmp[882 * 4 + 16];
	unsigned int length, Writted;
	
	if (!WAV_Dumping)
		return 0;
	
	Write_Sound_Buffer(Buf_Tmp);
	
	length = Seg_Length << 1;
	
	if (Sound_Stereo)
		length *= 2;
	
	if (WaveWriteFile(MMIOOut, length, &Buf_Tmp[0], &CkOut, &Writted, &MMIOInfoOut))
	{
		draw->writeText("Error in WAV dumping", 1000);
		return 0;
	}
#endif
	return 1;
}


int Audio::stopWAVDump(void)
{
	STUB;
#if 0
	if (!WAV_Dumping)
	{
		draw->writeText("Already stopped", 1000);
		return 0;
	}
	
	if (WaveCloseWriteFile(&MMIOOut, &CkOut, &CkRIFF, &MMIOInfoOut, 0))
		return 0;
	
	draw->writeText("WAV dump stopped", 1000);
	WAV_Dumping = 0;
#endif
	return 1;
}


// Properties


int Audio::soundRate(void)
{
	return m_soundRate;
}
void Audio::setSoundRate(const int newSoundRate)
{
	m_soundRate = newSoundRate;
	// TODO: Adjust the audio subsystem to use the new rate.
}


bool Audio::dumpingWAV(void)
{
	return m_dumpingWAV;
}
void Audio::setDumpingWAV(const bool newDumpingWAV)
{
	m_dumpingWAV = newDumpingWAV;
	// TODO: WAV code.
}


bool Audio::playingGYM(void)
{
	return m_playingGYM;
}
void Audio::setPlayingGYM(const bool newPlayingGYM)
{
	m_playingGYM = newPlayingGYM;
	// TODO: GYM code.
}


bool Audio::enabled(void)
{
	return m_enabled;
}
void Audio::setEnabled(const bool newEnabled)
{
	m_enabled = newEnabled;
	// TODO: Enabled code.
}


bool Audio::stereo(void)
{
	return m_stereo;
}
void Audio::setStereo(const bool newStereo)
{
	m_stereo = newStereo;
	// TODO: Stereo code.
}


int Audio::segLength(void)
{
	return m_segLength;
}


bool Audio::soundInitialized(void)
{
	return m_soundInitialized;
}
