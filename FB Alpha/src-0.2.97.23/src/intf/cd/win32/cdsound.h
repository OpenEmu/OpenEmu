#ifndef _WavClass_H_
#define _WavClass_H_

//#include <dsound.h>
#include "dsound_core.h"

class WavClass
{
private:
	struct WaveHeaderType
	{
		char chunkId[4];
		unsigned long chunkSize;
		char format[4];
		char subChunkId[4];
		unsigned long subChunkSize;
		unsigned short audioFormat;
		unsigned short numChannels;
		unsigned long sampleRate;
		unsigned long bytesPerSecond;
		unsigned short blockAlign;
		unsigned short bitsPerSample;
		char dataChunkId[4];
		unsigned long dataSize;
	};

public:
	WavClass();
	WavClass(const WavClass&);
	~WavClass();

	bool Initialize(HWND, TCHAR*);
	void Shutdown();

	bool InitializeDirectSound(HWND);
	void ShutdownDirectSound();

	bool LoadWaveFile(TCHAR*, IDirectSoundBuffer**);
	void ShutdownWaveFile(IDirectSoundBuffer**);

	bool PlayWaveFile();	

	IDirectSound*			GetDirectSound()		{ return m_DirectSound; }
	IDirectSoundBuffer*		GetPrimaryBuffer()		{ return m_primaryBuffer; }
	IDirectSoundBuffer*	GetSecondaryBuffer()	{ return m_secondaryBuffer1; }

private:
	IDirectSound*			m_DirectSound;
	IDirectSoundBuffer*		m_primaryBuffer;
	IDirectSoundBuffer*	m_secondaryBuffer1;
};

void	wav_exit();
int		wav_open(TCHAR* szFile);
void	wav_stop();
void	wav_play();

#endif
