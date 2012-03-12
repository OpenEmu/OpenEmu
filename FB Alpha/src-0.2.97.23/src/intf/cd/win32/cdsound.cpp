#include "burner.h"
#include "cdsound.h"

WavClass::WavClass()
{
	m_DirectSound		= 0;
	m_primaryBuffer		= 0;
	m_secondaryBuffer1	= 0;
}

WavClass::WavClass(const WavClass&)
{
	//
}

WavClass::~WavClass()
{
	//
}

bool WavClass::Initialize(HWND hwnd, TCHAR* szFile)
{
	bool result;

	// Initialize direct sound and the primary sound buffer.
	result = InitializeDirectSound(hwnd);
	if(!result)
	{
		return false;
	}

	// Load a wave audio file onto a secondary buffer.
	result = LoadWaveFile(szFile, &m_secondaryBuffer1);
	if(!result)
	{
		return false;
	}

	return true;
}

void WavClass::Shutdown()
{
	// Release the secondary buffer.
	ShutdownWaveFile(&m_secondaryBuffer1);

	// Shutdown the Direct Sound API.
	ShutdownDirectSound();

	return;
}

bool WavClass::InitializeDirectSound(HWND hwnd)
{
	HRESULT result;
	DSBUFFERDESC bufferDesc;
	WAVEFORMATEX waveFormat;

	// Initialize the direct sound interface pointer for the default sound device.
	result = _DirectSoundCreate(NULL, &m_DirectSound, NULL);
	if(FAILED(result))
	{
		return false;
	}

	// Set the cooperative level to priority so the format of the primary sound buffer can be modified.
	result = m_DirectSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
	if(FAILED(result))
	{
		return false;
	}
	
	GUID guidNULL;
	memset(&guidNULL,0,sizeof(GUID));

	// Setup the primary buffer description.
	bufferDesc.dwSize			= sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags			= DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes	= 0;
	bufferDesc.dwReserved		= 0;
	bufferDesc.lpwfxFormat		= NULL;
	bufferDesc.guid3DAlgorithm	= guidNULL; //GUID_NULL;

	// Get control of the primary sound buffer on the default sound device.
	result = m_DirectSound->CreateSoundBuffer(&bufferDesc, &m_primaryBuffer, NULL);
	if(FAILED(result))
	{
		return false;
	}

	// Setup the format of the primary sound bufffer.
	// In this case it is a .WAV file recorded at 44,100 samples per second in 16-bit stereo (cd audio format).
	waveFormat.wFormatTag			= WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec		= 44100;
	waveFormat.wBitsPerSample		= 16;
	waveFormat.nChannels			= 2;
	waveFormat.nBlockAlign			= (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec		= waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize				= 0;

	// Set the primary buffer to be the wave format specified.
	result = m_primaryBuffer->SetFormat(&waveFormat);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}

void WavClass::ShutdownDirectSound()
{
	// Release the primary sound buffer pointer.
	if(m_primaryBuffer)
	{
		m_primaryBuffer->Release();
		m_primaryBuffer = 0;
	}

	// Release the direct sound interface pointer.
	if(m_DirectSound)
	{
		m_DirectSound->Release();
		m_DirectSound = 0;
	}

	return;
}

bool WavClass::LoadWaveFile(TCHAR* filename, IDirectSoundBuffer** secondaryBuffer)
{
	FILE* filePtr;
	unsigned int count;
	WaveHeaderType waveFileHeader;
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	HRESULT result;
	IDirectSoundBuffer* tempBuffer;
	unsigned char* waveData;
	unsigned char* bufferPtr;
	unsigned long bufferSize;


	// Open the wave file in binary.
	filePtr = _tfopen(filename, _T("rb"));
	if(!filePtr)
	{
		return false;
	}

	// Read in the wave file header.
	count = fread(&waveFileHeader, sizeof(waveFileHeader), 1, filePtr);
	if(count != 1)
	{
		fclose(filePtr);
		return false;
	}

	// Check that the chunk ID is the RIFF format.
	if((waveFileHeader.chunkId[0] != 'R') || (waveFileHeader.chunkId[1] != 'I') || 
	   (waveFileHeader.chunkId[2] != 'F') || (waveFileHeader.chunkId[3] != 'F'))
	{
		fclose(filePtr);
		return false;
	}

	// Check that the file format is the WAVE format.
	if((waveFileHeader.format[0] != 'W') || (waveFileHeader.format[1] != 'A') ||
	   (waveFileHeader.format[2] != 'V') || (waveFileHeader.format[3] != 'E'))
	{
		fclose(filePtr);
		return false;
	}

	// Check that the sub chunk ID is the fmt format.
	if((waveFileHeader.subChunkId[0] != 'f') || (waveFileHeader.subChunkId[1] != 'm') ||
	   (waveFileHeader.subChunkId[2] != 't') || (waveFileHeader.subChunkId[3] != ' '))
	{
		fclose(filePtr);
		return false;
	}

	// Check that the audio format is WAVE_FORMAT_PCM.
	if(waveFileHeader.audioFormat != WAVE_FORMAT_PCM)
	{
		fclose(filePtr);
		return false;
	}

	// Check that the wave file was recorded in stereo format.
	if(waveFileHeader.numChannels != 2)
	{
		fclose(filePtr);
		return false;
	}

	// Check that the wave file was recorded at a sample rate of 44.1 KHz.
	if(waveFileHeader.sampleRate != 44100)
	{
		fclose(filePtr);
		return false;
	}

	// Ensure that the wave file was recorded in 16 bit format.
	if(waveFileHeader.bitsPerSample != 16)
	{
		fclose(filePtr);
		return false;
	}

	// Check for the data chunk header.
	if((waveFileHeader.dataChunkId[0] != 'd') || (waveFileHeader.dataChunkId[1] != 'a') ||
	   (waveFileHeader.dataChunkId[2] != 't') || (waveFileHeader.dataChunkId[3] != 'a'))
	{
		fclose(filePtr);
		return false;
	}

	// Set the wave format of secondary buffer that this wave file will be loaded onto.
	waveFormat.wFormatTag			= WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec		= 44100;
	waveFormat.wBitsPerSample		= 16;
	waveFormat.nChannels			= 2;
	waveFormat.nBlockAlign			= (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec		= waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize				= 0;

	GUID guidNULL;
	memset(&guidNULL,0,sizeof(GUID));

	// Set the buffer description of the secondary sound buffer that the wave file will be loaded onto.
	bufferDesc.dwSize			= sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags			= DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY;
	bufferDesc.dwBufferBytes	= waveFileHeader.dataSize;
	bufferDesc.dwReserved		= 0;
	bufferDesc.lpwfxFormat		= &waveFormat;
	bufferDesc.guid3DAlgorithm	= guidNULL; //GUID_NULL;

	// Create a temporary sound buffer with the specific buffer settings.
	result = m_DirectSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
	if(FAILED(result))
	{
		fclose(filePtr);
		return false;
	}

	// Test the buffer format against the direct sound 8 interface and create the secondary buffer.
	result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer, (void**)&*secondaryBuffer);
	if(FAILED(result))
	{
		fclose(filePtr);
		return false;
	}

	// Release the temporary buffer.
	tempBuffer->Release();
	tempBuffer = 0;

	// Move to the beginning of the wave data which starts at the end of the data chunk header.
	fseek(filePtr, sizeof(WaveHeaderType), SEEK_SET);

	// Create a temporary buffer to hold the wave file data.
	waveData = new unsigned char[waveFileHeader.dataSize];
	if(!waveData)
	{
		fclose(filePtr);
		return false;
	}

	// Read in the wave file data into the newly created buffer.
	count = fread(waveData, 1, waveFileHeader.dataSize, filePtr);
	if(count != waveFileHeader.dataSize)
	{
		fclose(filePtr);
		return false;
	}

	// Close the file once done reading.
	int error = fclose(filePtr);
	if(error != 0)
	{

		return false;
	}

	// Lock the secondary buffer to write wave data into it.
	result = (*secondaryBuffer)->Lock(0, waveFileHeader.dataSize, (void**)&bufferPtr, (DWORD*)&bufferSize, NULL, 0, 0);
	if(FAILED(result))
	{
		return false;
	}

	// Copy the wave data into the buffer.
	memcpy(bufferPtr, waveData, waveFileHeader.dataSize);

	// Unlock the secondary buffer after the data has been written to it.
	result = (*secondaryBuffer)->Unlock((void*)bufferPtr, bufferSize, NULL, 0);
	if(FAILED(result))
	{
		return false;
	}
	
	// Release the wave data since it was copied into the secondary buffer.
	delete [] waveData;
	waveData = 0;

	return true;
}

void WavClass::ShutdownWaveFile(IDirectSoundBuffer** secondaryBuffer)
{
	// Release the secondary sound buffer.
	if(*secondaryBuffer)
	{
		(*secondaryBuffer)->Release();
		*secondaryBuffer = 0;
	}

	return;
}

#include "cd_interface.h"

WavClass* wav;
extern HWND hScrnWnd;

void wav_exit()
{
	if(wav) {
		wav_stop();
		wav->Shutdown();
		wav = NULL;
	}
}

int wav_open(TCHAR* szFile)
{
	wav_exit();

	if(hScrnWnd) {
		wav = new WavClass;
		wav->Initialize(hScrnWnd, szFile);
	} else {
		return 0;
	}

	return 1;
}

void wav_stop() 
{
	if(!wav) return;

	wav->GetSecondaryBuffer()->Stop();
	wav->GetSecondaryBuffer()->SetCurrentPosition( 0 );
}

void wav_play()
{
	if(!wav) return;

	HRESULT result;

	// Play the contents of the secondary sound buffer.
	IDirectSoundBuffer* m_secondaryBuffer1 = wav->GetSecondaryBuffer();
	if(!m_secondaryBuffer1) {
		//
		return;
	}

	// Set volume of the buffer to 100%.
	result = m_secondaryBuffer1->SetVolume(DSBVOLUME_MAX);
	if(FAILED(result))
	{
		return;
	}

	result = m_secondaryBuffer1->Play(0, 0, DSBPLAY_LOOPING);
	if(FAILED(result)) {
		//
		return;
	}
}

void wav_pause(bool bResume)
{
	if(!wav) return;	

	if(!bResume) 
	{
		DWORD pdwStatus;
		wav->GetSecondaryBuffer()->GetStatus(&pdwStatus);
		if((pdwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)
		{	
			HRESULT result;
			result = wav->GetSecondaryBuffer()->Stop();
			if(FAILED(result)) {
				//
			}
		}

	} else {
		DWORD pdwStatus;
		wav->GetSecondaryBuffer()->GetStatus(&pdwStatus);
		if((pdwStatus & DSBSTATUS_PLAYING) != DSBSTATUS_PLAYING)
		{
			if(CDEmuGetStatus() == playing) 
			{
				HRESULT result;

				// Set volume of the buffer to 100%.
				result = wav->GetSecondaryBuffer()->SetVolume(DSBVOLUME_MAX);
				if(FAILED(result))
				{
					return;
				}

				result = wav->GetSecondaryBuffer()->Play(0, 0, DSBPLAY_LOOPING);
				if(FAILED(result)) {
					//
				}
			}
		}
	}
}
