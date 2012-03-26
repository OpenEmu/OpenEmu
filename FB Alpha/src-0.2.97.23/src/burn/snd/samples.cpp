// FB Alpha sample player module

#include "burnint.h"
//#include "direct.h"
#include "samples.h"

#define SAMPLE_DIRECTORY	szAppSamplesPath

#define get_long()	((ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | (ptr[0] << 0))
#define get_short()	((ptr[1] << 8) | (ptr[0] << 0))

static INT32 bAddToStream = 0;
static INT32 nTotalSamples = 0;
static INT32 nSampleSetGain = 100;

struct sample_format
{
	UINT8 *data;
	UINT32 length;
	UINT32 position;
	UINT8 playing;
	UINT8 loop;
	UINT8 flags;
};

static struct sample_format *samples		= NULL; // store samples
static struct sample_format *sample_ptr		= NULL; // generic pointer for sample

static void make_raw(UINT8 *src, UINT32 len)
{
	UINT8 *ptr = src;

	if (ptr[0] != 'R' || ptr[1] != 'I' || ptr[2] != 'F' || ptr[3] != 'F') return;
	ptr += 4; // skip RIFF

	UINT32 length = get_long();	ptr += 4; // total length of file
	if (len < length) length = len - 8;		  // first 8 bytes (RIFF + Len)

	/* "WAVEfmt " */			ptr += 8; // WAVEfmt + 1 space
	UINT32 length2 = get_long();	ptr += 4; // Wavefmt length
/*	unsigned short format = get_short();  */ptr += 2; // format?
	UINT16 channels = get_short();	ptr += 2; // channels
	UINT32 sample_rate = get_long();	ptr += 4; // sample rate
/*	unsigned int speed = get_long();      */ptr += 4; // speed - should equal (bits * channels * sample_rate)
/*	unsigned short align = get_short();   */ptr += 2; // block align	should be ((bits / 8) * channels)
	UINT16 bits = get_short() / 8;	ptr += 2; // bits per sample	(0010)
	ptr += length2 - 16;				  // get past the wave format chunk

	// are we in the 'data' chunk? if not, skip this chunk.
	if (ptr[0] != 'd' || ptr[1] != 'a' || ptr[2] != 't' || ptr[3] != 'a') {
		ptr += 4; // skip tag

		UINT32 length3 = get_long(); ptr += 4;
		ptr += length3;
	}

	/* "data" */				ptr += 4; // "data"
	UINT32 data_length = get_long();	ptr += 4; // should be up to the data...

	if ((len - (ptr - src)) < data_length) data_length = len - (ptr - src);

	UINT32 converted_len = (UINT32)((float)(data_length * (nBurnSoundRate * 1.00000 / sample_rate) / (bits * channels)));
	if (converted_len == 0) return; 

	sample_ptr->data = (UINT8*)malloc(converted_len * 4);

	// up/down sample everything and convert to raw 16 bit stereo
	{
		INT16 *data = (INT16*)sample_ptr->data;
		INT16 *poin = (INT16*)ptr;
		UINT8 *poib = ptr;
	
		for (UINT32 i = 0; i < converted_len; i++)
		{
			UINT32 x = (UINT32)((float)(i * (sample_rate * 1.00000 / nBurnSoundRate)));

			if (bits == 2) {						//  signed 16 bit, stereo & mono
				data[i * 2 + 0] = poin[x * channels + 0             ];
				data[i * 2 + 1] = poin[x * channels + (channels / 2)];
			}

			if (bits == 1) {						// unsigned 8 bit, stereo & mono
				data[i * 2 + 0] = (poib[x * channels + 0             ] - 128) << 8;
				data[i * 2 + 1] = (poib[x * channels + (channels / 2)] - 128) << 8;
			}
		}

		// now go through and set the gain
		for (UINT32 i = 0; i < converted_len * 2; i++)
		{
			INT32 d = (data[i] * nSampleSetGain) / 100;
			if (d >  0x7fff) d =  0x7fff;
			if (d < -0x7fff) d = -0x7fff;
			data[i] = (INT16)d;
		}
	}

	sample_ptr->length = converted_len;
	sample_ptr->playing = 0;
	sample_ptr->position = 0;
}

void BurnSamplePlay(INT32 sample)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SamplesInitted) bprintf(PRINT_ERROR, _T("BurnSamplePlay called without init\n"));
#endif

	if (sample >= nTotalSamples) return;

	sample_ptr = &samples[sample];

	if (sample_ptr->flags & SAMPLE_IGNORE) return;

	sample_ptr->playing = 1;
	sample_ptr->position = 0;
}

void BurnSamplePause(INT32 sample)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SamplesInitted) bprintf(PRINT_ERROR, _T("BurnSamplePause called without init\n"));
#endif

	if (sample >= nTotalSamples) return;

	sample_ptr = &samples[sample];
	sample_ptr->playing = 0;
}

void BurnSampleResume(INT32 sample)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SamplesInitted) bprintf(PRINT_ERROR, _T("BurnSampleResume called without init\n"));
#endif

	if (sample >= nTotalSamples) return;

	sample_ptr = &samples[sample];
	sample_ptr->playing = 1;
}

void BurnSampleStop(INT32 sample)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SamplesInitted) bprintf(PRINT_ERROR, _T("BurnSampleStop called without init\n"));
#endif

	if (sample >= nTotalSamples) return;

	sample_ptr = &samples[sample];
	sample_ptr->playing = 0;
	sample_ptr->position = 0;
}

void BurnSampleSetLoop(INT32 sample, bool dothis)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SamplesInitted) bprintf(PRINT_ERROR, _T("BurnSampleSetLoop called without init\n"));
#endif

	if (sample >= nTotalSamples) return;

	sample_ptr = &samples[sample];

	if (sample_ptr->flags & SAMPLE_NOLOOP) return;

	sample_ptr->loop = (dothis ? 1 : 0);
}

INT32 BurnSampleGetStatus(INT32 sample)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SamplesInitted) bprintf(PRINT_ERROR, _T("BurnSampleGetStatus called without init\n"));
#endif

	if (sample >= nTotalSamples) return -1;

	sample_ptr = &samples[sample];
	return (sample_ptr->playing);
}

INT32 BurnSampleGetPosition(INT32 sample)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SamplesInitted) bprintf(PRINT_ERROR, _T("BurnSampleGetPosition called without init\n"));
#endif

	if (sample >= nTotalSamples) return -1;

	sample_ptr = &samples[sample];
	return (sample_ptr->position);
}

void BurnSampleSetPosition(INT32 sample, UINT32 position)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SamplesInitted) bprintf(PRINT_ERROR, _T("BurnSampleSetPosition called without init\n"));
#endif

	if (sample >= nTotalSamples) return;

	sample_ptr = &samples[sample];
	sample_ptr->position = position;
}

void BurnSampleReset()
{
#if defined FBA_DEBUG
	if (!DebugSnd_SamplesInitted) bprintf(PRINT_ERROR, _T("BurnSampleReset called without init\n"));
#endif

	for (INT32 i = 0; i < nTotalSamples; i++) {
		BurnSampleStop(i);

		if (sample_ptr->flags & SAMPLE_AUTOLOOP) {
			BurnSampleSetLoop(i, true);
			BurnSamplePlay(i);
		}
	}
}

INT32 __cdecl ZipLoadOneFile(char* arcName, const char* fileName, void** Dest, INT32* pnWrote);
char* TCHARToANSI(const TCHAR* pszInString, char* pszOutString, INT32 nOutSize);
#define _TtoA(a)	TCHARToANSI(a, NULL, 0)

void BurnSampleInit(INT32 nGain /*volume percentage!*/, INT32 bAdd /*add sample to stream?*/)
{
	DebugSnd_SamplesInitted = 1;
	
	if (nBurnSoundRate == 0) {
		nTotalSamples = 0;
		return;
	}

	INT32 length;
	char path[256];
	char setname[128];
	void *destination = NULL;
	char szTempPath[MAX_PATH];
	sprintf(szTempPath, _TtoA(SAMPLE_DIRECTORY));

	// test to see if file exists
	INT32 nEnableSamples = 0;

	if (BurnDrvGetTextA(DRV_SAMPLENAME) == NULL) { // called with no samples
		nTotalSamples = 0;
		return;
	}

	strcpy(setname, BurnDrvGetTextA(DRV_SAMPLENAME));
	sprintf(path, "%s%s.zip", szTempPath, setname);
	
	FILE *test = fopen(path, "rb");
	if (test) 
	{
		nEnableSamples = 1;
		fclose(test);
	}
	
#ifdef INCLUDE_7Z_SUPPORT
	sprintf(path, "%s%s.7z", szTempPath, setname);
	
	test = fopen(path, "rb");
	if (test)
	{	
		nEnableSamples = 1;
		fclose(test);
	}
#endif
	
	if (!nEnableSamples) return;

	bAddToStream = bAdd;
	nSampleSetGain = nGain;
	nTotalSamples = 0;

	struct BurnSampleInfo si;
	INT32 nSampleOffset = -1;
	do {
		BurnDrvGetSampleInfo(&si, ++nSampleOffset);
		if (si.nFlags) nTotalSamples++;
	} while (si.nFlags);
	
	samples = (sample_format*)malloc(sizeof(sample_format) * nTotalSamples);
	memset (samples, 0, sizeof(sample_format) * nTotalSamples);

	for (INT32 i = 0; i < nTotalSamples; i++) {
		BurnDrvGetSampleInfo(&si, i);
		char *szSampleName = NULL;
		BurnDrvGetSampleName(&szSampleName, i, 0);
		sample_ptr = &samples[i];

		if (si.nFlags == 0) break;

		sprintf (path, "%s%s", szTempPath, setname);

		destination = NULL;
		length = 0;
		ZipLoadOneFile((char*)path, (const char*)szSampleName, &destination, &length);
		
		if (length) {
			make_raw((UINT8*)destination, length);

			sample_ptr->flags = si.nFlags;
		} else {
			sample_ptr->flags = SAMPLE_IGNORE;
		}

		if (destination) {
			free (destination);
			destination = NULL;
		}		
	}
}

void BurnSampleExit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_SamplesInitted) bprintf(PRINT_ERROR, _T("BurnSampleExit called without init\n"));
#endif

	for (INT32 i = 0; i < nTotalSamples; i++) {
		sample_ptr = &samples[i];
		if (sample_ptr->data != NULL) {
			free (sample_ptr->data);
			sample_ptr->data = NULL;
		}
	}

	if (samples) {
		free (samples);
		samples = NULL;
	}

	sample_ptr = NULL;
	nTotalSamples = 0;
	bAddToStream = 0;
	nSampleSetGain = 100;
	
	DebugSnd_SamplesInitted = 0;
}

void BurnSampleRender(INT16 *pDest, UINT32 pLen)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SamplesInitted) bprintf(PRINT_ERROR, _T("BurnSampleRender called without init\n"));
#endif

	if (pBurnSoundOut == NULL) return;

	INT32 nFirstSample = 0;
	UINT32 *dest = (UINT32*)pDest;

	for (INT32 i = 0; i < nTotalSamples; i++)
	{
		sample_ptr = &samples[i];
		if (sample_ptr->playing == 0) continue;

		INT32 playlen = pLen;
		INT32 loop = sample_ptr->loop;
		INT32 length = sample_ptr->length;
		INT32 position = sample_ptr->position;

		UINT32 *data = (UINT32*)sample_ptr->data;

		if (loop) {
			if (bAddToStream == 0 && nFirstSample == 0) {
				for (INT32 j = 0; j < playlen; j++, position++) {
					dest[j] = data[position % length];
				}
			} else {
				position *= 2;
				length *= 2;
				INT16 *dst = (INT16*)dest;
				INT16 *dat = (INT16*)data;

				for (INT32 j = 0; j < playlen; j++, position+=2, dst += 2) {
					INT32 t0 = dst[0] + dat[(position + 0) % length];
					INT32 t1 = dst[1] + dat[(position + 1) % length];

					if (t0 > 0x7fff) t0 = 0x7fff;
					if (t1 > 0x7fff) t1 = 0x7fff;
					if (t0 < -0x7fff) t0 = -0x7fff;
					if (t1 < -0x7fff) t1 = -0x7fff;
					dst[0] = t0;
					dst[1] = t1;
				}
			}
		} else {
			length = length - position;

			if (length <= 0) {
				if (loop == 0) {
					sample_ptr->playing = 0;
					continue;
				}
			}

			data += position;
			if (playlen > length) playlen = length;

			if (bAddToStream == 0 && nFirstSample == 0) {
				for (INT32 j = 0; j < playlen; j++) {
					dest[j] = data[j];
				}
			} else {
				INT16 *dst = (INT16*)dest;
				INT16 *dat = (INT16*)data;

				for (INT32 j = 0; j < playlen; j++, dst +=2, dat+=2) {
					INT32 t0 = dst[0] + dat[0];
					INT32 t1 = dst[1] + dat[1];

					if (t0 > 0x7fff) t0 = 0x7fff;
					if (t1 > 0x7fff) t1 = 0x7fff;
					if (t0 < -0x7fff) t0 = -0x7fff;
					if (t1 < -0x7fff) t1 = -0x7fff;
					dst[0] = t0;
					dst[1] = t1;
				}
			}
		}

		sample_ptr->position += playlen;
		nFirstSample++;
	}
}

INT32 BurnSampleScan(INT32 nAction, INT32 *pnMin)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SamplesInitted) bprintf(PRINT_ERROR, _T("BurnSampleScan called without init\n"));
#endif

	if (pnMin != NULL) {
		*pnMin = 0x029707;
	}

	if (nAction & ACB_DRIVER_DATA) {
		for (INT32 i = 0; i < nTotalSamples; i++) {
			sample_ptr = &samples[i];
			SCAN_VAR(sample_ptr->playing);
			SCAN_VAR(sample_ptr->loop);
			SCAN_VAR(sample_ptr->position);
		}
	}

	return 0;
}
