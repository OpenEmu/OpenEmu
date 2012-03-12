// ----------------------------------------------------------------------------
// iso/cue/wav support 
/*-----------------------------------------------------------------------------
Modified by: CaptainCPS-X
Updates:
	(10/24/2011)
		- removed libmad and MP3 support
		- added my custom DirectSound library to add WAV support
		- removed most (if not all) references to MP3
		- modified a few other things as needed
------------------------------------------------------------------------------*/
#include "burner.h"
#include "cdsound.h"

#define MAXIMUM_NUMBER_TRACKS (100)

#define CD_FRAMES_MINUTE (60 * 75)
#define CD_FRAMES_SECOND (     75)
#define CD_FRAMES_PREGAP ( 2 * 75)

struct isowavTRACK_DATA { 
	char Control; 
	char TrackNumber; 
	char Address[4]; 
	TCHAR* Filename; 
};

struct isowavCDROM_TOC  { 
	char FirstTrack; 
	char LastTrack; 
	isowavTRACK_DATA TrackData[MAXIMUM_NUMBER_TRACKS]; 
};

static isowavCDROM_TOC* isowavTOC;

static FILE*  isowavFile     = NULL;
static int    isowavTrack    = 0;
static int    isowavLBA      = 0;

// -----------------------------------------------------------------------------

static const char* isowavLBAToMSF(const int LBA)
{
	static char address[4];

	address[0] = 0;
	address[1] = LBA                    / CD_FRAMES_MINUTE;
	address[2] = LBA % CD_FRAMES_MINUTE / CD_FRAMES_SECOND;
	address[3] = LBA % CD_FRAMES_SECOND;

	return address;
}

static int isowavMSFToLBA(const char* address)
{
	int LBA;

	LBA  = address[3];
	LBA += address[2] * CD_FRAMES_SECOND;
	LBA += address[1] * CD_FRAMES_MINUTE;

	return LBA;
}

// -----------------------------------------------------------------------------

static int isowavGetTrackSizes()
{
	// determine the lenght of the .iso / .mp3 files to complete the TOC

	FILE*  h;

	for (int i = isowavTOC->FirstTrack - 1; i < isowavTOC->LastTrack; i++) {

		const char* address;

		if (isowavTOC->TrackData[i].Control & 4) {

			// data track

			h = _tfopen(isowavTOC->TrackData[i].Filename, _T("rb"));
			if (h == NULL) return 1;

			fseek(h, 0, SEEK_END);

			address = isowavLBAToMSF((ftell(h) + 2047) / 2048 + isowavMSFToLBA(isowavTOC->TrackData[i].Address));

			if(h) fclose(h);

		} else {

			// audio track

			h = _tfopen(isowavTOC->TrackData[i].Filename, _T("rb"));
			if (h == NULL)return 1;

			fseek(h, 0, SEEK_END);	

			address = isowavLBAToMSF(((ftell(h) + 2047) / 2048) + isowavMSFToLBA(isowavTOC->TrackData[i].Address));
			if(h) fclose(h);		
		}

		isowavTOC->TrackData[i + 1].Address[0] += 0; // always 0 [?]
		isowavTOC->TrackData[i + 1].Address[1] += address[1]; // M
		isowavTOC->TrackData[i + 1].Address[2] += address[2]; // S
		isowavTOC->TrackData[i + 1].Address[3] += address[3]; // F
	}

	return 0;
}

static int isowavTestISO()
{
	TCHAR fullname[MAX_PATH];
	TCHAR* filename;
	int   length = 0;
	int   offset = 0;
	int   track  = 2;
	FILE* h;

	_tcscpy(fullname, CDEmuImage);
	length = _tcslen(fullname);

	// assume CD-ROM mode1/2048 format

	if (length <= 4 && (_tcscmp(_T(".iso"), fullname + length - 4) || _tcscmp(_T(".bin"), fullname + length - 4))) {
		return 1;
	}

	// create a TOC with only the data track first

	isowavTOC->FirstTrack = 1;
	isowavTOC->LastTrack  = 1;

	isowavTOC->TrackData[0].TrackNumber = 1;

	isowavTOC->TrackData[0].Address[1] = 0;
	isowavTOC->TrackData[0].Address[2] = 2;
	isowavTOC->TrackData[0].Address[3] = 0;

	isowavTOC->TrackData[0].Filename = (TCHAR*)malloc((length + 1) * sizeof(TCHAR));
	if (isowavTOC->TrackData[0].Filename == NULL) {
		return 1;
	}
	_tcscpy(isowavTOC->TrackData[0].Filename, fullname);

	isowavTOC->TrackData[0].Control = 4;

	// if the filename has a number in it, try to find .mp3 tracks

	filename = ExtractFilename(fullname);
	offset = (filename - fullname) + length - 6;
	while (offset >= 0 && fullname[offset] != _T('0') && fullname[offset + 1] != _T('1')) {
		offset--;
	}
	if (offset < 0) {
		return isowavGetTrackSizes();
	}

	_stprintf(fullname + length - 4, _T(".wav"));

	while (1) {
		fullname[offset] = _T('0') + track / 10; fullname[offset + 1] = _T('0') + track % 10;
		
		if ((h = _tfopen(fullname, _T("rb"))) == NULL) {
			break;
		}
		fclose(h);

		isowavTOC->TrackData[track - 1].TrackNumber = track;

		isowavTOC->TrackData[track - 1].Filename = (TCHAR*)malloc((length + 1) * sizeof(TCHAR));
		if (isowavTOC->TrackData[track - 1].Filename == NULL) {
			return 1;
		}
		_tcscpy(isowavTOC->TrackData[track - 1].Filename, fullname);

		isowavTOC->LastTrack = track;

		track++;
	}

	return isowavGetTrackSizes();
}

static int isowavParseCueFile()
{
	TCHAR  szLine[1024];
	TCHAR  szFile[1024];
	TCHAR* s;
	TCHAR* t;
	FILE*  h;
	int    track = 0;
	int    length;

	isowavTOC->FirstTrack = 1;
	isowavTOC->LastTrack  = 1;

	isowavTOC->TrackData[0].Address[1] = 0;
	isowavTOC->TrackData[0].Address[2] = 2;
	isowavTOC->TrackData[0].Address[3] = 0;

	h = _tfopen(CDEmuImage, _T("rt"));
	if (h == NULL) {
		return 1;
	}

	while (1) {
		if (_fgetts(szLine, sizeof(szLine), h) == NULL) {
			break;
		}

		length = _tcslen(szLine);
		// get rid of the linefeed at the end
		while (length && (szLine[length - 1] == _T('\r') || szLine[length - 1] == _T('\n'))) {
			szLine[length - 1] = 0;
			length--;
		}

		s = szLine;

		// file info
		if ((t = LabelCheck(s, _T("FILE"))) != 0) {
			s = t;

			TCHAR* szQuote;

			// read filename
			QuoteRead(&szQuote, NULL, s);

			_sntprintf(szFile, ExtractFilename(CDEmuImage) - CDEmuImage, _T("%s"), CDEmuImage);
			_sntprintf(szFile + (ExtractFilename(CDEmuImage) - CDEmuImage), 1024 - (ExtractFilename(CDEmuImage) - CDEmuImage), _T("/%s"), szQuote);

			continue;
		}

		// track info
		if ((t = LabelCheck(s, _T("TRACK"))) != 0) {
			s = t;

			// track number
			track = _tcstol(s, &t, 10);

			if (track < 1 || track > MAXIMUM_NUMBER_TRACKS) {
				fclose(h);
				return 1;
			}

			if (track < isowavTOC->FirstTrack) {
				isowavTOC->FirstTrack = track;
			}
			if (track > isowavTOC->LastTrack) {
				isowavTOC->LastTrack = track;
			}
			isowavTOC->TrackData[track - 1].TrackNumber = track;

			isowavTOC->TrackData[track - 1].Filename = (TCHAR*)malloc((_tcslen(szFile) + 1) * sizeof(TCHAR));
			if (isowavTOC->TrackData[track - 1].Filename == NULL) {
				fclose(h);
				return 1;
			}
			_tcscpy(isowavTOC->TrackData[track - 1].Filename, szFile);

			s = t;

			// type of track

			if ((t = LabelCheck(s, _T("MODE1/2048"))) != 0) {
				isowavTOC->TrackData[track - 1].Control = 4;

				continue;
			}
			if ((t = LabelCheck(s, _T("AUDIO"))) != 0) {
				isowavTOC->TrackData[track - 1].Control = 0;

				continue;
			}

			fclose(h);
			return 1;
		}

		// pregap
		if ((t = LabelCheck(s, _T("PREGAP"))) != 0) {
			s = t;

			int M, S, F;

			// pregap M
			M = _tcstol(s, &t, 10);
			s = t + 1;
			// pregap S
			S = _tcstol(s, &t, 10);
			s = t + 1;
			// pregap F
			F = _tcstol(s, &t, 10);

			if (M < 0 || M > 100 || S < 0 || S > 59 || F < 0 || F > 74) {
				fclose(h);
				return 1;
			}

			isowavTOC->TrackData[track - 1].Address[1] = M;
			isowavTOC->TrackData[track - 1].Address[2] = S;
			isowavTOC->TrackData[track - 1].Address[3] = F;

			continue;
		}
	}

	fclose(h);

	return isowavGetTrackSizes();
}

// -----------------------------------------------------------------------------

static int isowavExit()
{
	wav_exit();

	if (isowavFile) {
		fclose(isowavFile);
		isowavFile = NULL;
	}

	isowavTrack    = 0;
	isowavLBA      = 0;

	if (isowavTOC) {
		for (int i = 0; i < MAXIMUM_NUMBER_TRACKS; i++) {
			free(isowavTOC->TrackData[i].Filename);
		}

		free(isowavTOC);
		isowavTOC = NULL;
	}

	return 0;
}

static int isowavInit()
{
	wav_exit();

	isowavTOC = (isowavCDROM_TOC*)malloc(sizeof(isowavCDROM_TOC));
	if (isowavTOC == NULL) {
		return 1;
	}
	memset(isowavTOC, 0, sizeof(isowavCDROM_TOC));

	TCHAR* filename = ExtractFilename(CDEmuImage);

	if (_tcslen(filename) < 4) {
		return 1;
	}

	if (_tcscmp(_T(".cue"), filename + _tcslen(filename) - 4) == 0) {
		if (isowavParseCueFile()) {
			dprintf(_T("*** Couldn't parse .cue file\n"));
			isowavExit();

			return 1;
		}
	} else {
		if (isowavTestISO()) {
			dprintf(_T("*** Couldn't find .iso / .bin file\n"));
			isowavExit();

			return 1;
		}
	}

	dprintf(_T("    CD image TOC read\n"));
	
	for (int i = isowavTOC->FirstTrack - 1; i < isowavTOC->LastTrack; i++) {
		dprintf(_T("    track %2i start %02i:%02i:%02i control 0x%02X %s\n"), isowavTOC->TrackData[i].TrackNumber, isowavTOC->TrackData[i].Address[1], isowavTOC->TrackData[i].Address[2], isowavTOC->TrackData[i].Address[3], isowavTOC->TrackData[i].Control, isowavTOC->TrackData[i].Filename);
	}
	dprintf(_T("    total running time %02i:%02i:%02i\n"), isowavTOC->TrackData[(int)isowavTOC->LastTrack].Address[1], isowavTOC->TrackData[(int)isowavTOC->LastTrack].Address[2], isowavTOC->TrackData[(int)isowavTOC->LastTrack].Address[3]);

	CDEmuStatus = idle;
	
	return 0;
}

TCHAR* GetIsoPath() 
{
	if(isowavTOC) {
		return isowavTOC->TrackData[0].Filename;
	}
	return NULL;
}

static int isowavStop()
{
	wav_stop();

	if (isowavFile) {
		fclose(isowavFile);
		isowavFile = NULL;
	}
	CDEmuStatus = idle;
	return 0;
}

static int isowavPlayLBA(int LBA)
{
	isowavLBA = LBA;

	for (isowavTrack = isowavTOC->FirstTrack - 1; isowavTrack < isowavTOC->LastTrack; isowavTrack++) {
		if (isowavLBA < isowavMSFToLBA(isowavTOC->TrackData[isowavTrack + 1].Address)) {
			break;
		}
	}

	if (isowavTrack >= isowavTOC->LastTrack) {
		return 1;
	}

	bprintf(PRINT_IMPORTANT, _T("    playing track %2i - %s\n"), isowavTrack + 1, isowavTOC->TrackData[isowavTrack].Filename);

	isowavFile = _tfopen(isowavTOC->TrackData[isowavTrack].Filename, _T("rb"));
	if (isowavFile == NULL) {
		return 1;
	}

	if(	_tcsstr(isowavTOC->TrackData[isowavTrack].Filename, _T(".wav")) || _tcsstr(isowavTOC->TrackData[isowavTrack].Filename, _T(".WAV"))) {
		// is a wav, no need to keep this file pointer
		if (isowavFile) {
			fclose(isowavFile);
			isowavFile = NULL;
		}
		
		if(wav_open(isowavTOC->TrackData[isowavTrack].Filename)) {
			wav_play();
		} else {
			// error creating the WAV stream
			return 1;
		}
	}

	//dprintf(_T("*** WAV: wBitsPerSample: %d \n"), wav->g_pStreamingSound->m_pWaveFile->m_pwfx->wBitsPerSample);
	//dprintf(_T("*** WAV: nAvgBytesPerSec: %d \n"), wav->g_pStreamingSound->m_pWaveFile->m_pwfx->nAvgBytesPerSec);
	//dprintf(_T("*** WAV: m_dwSize: %d \n"), wav->g_pStreamingSound->m_pWaveFile->m_dwSize);
	//dprintf(_T("*** WAV: nBlockAlign: %d \n"), wav->g_pStreamingSound->m_pWaveFile->m_pwfx->nBlockAlign);

	isowavLBA = isowavMSFToLBA(isowavTOC->TrackData[isowavTrack].Address);
	CDEmuStatus = playing;

	return 0;
}

static int isowavPlay(unsigned char M, unsigned char S, unsigned char F)
{
	const char address[] = { 0, M, S, F };

	return isowavPlayLBA(isowavMSFToLBA(address));
}

static int isowavLoadSector(int LBA, char* pBuffer)
{
	LBA += CD_FRAMES_PREGAP;

	if (LBA != isowavLBA) {

		int track;

		for (track = isowavTOC->FirstTrack - 1; track < isowavTOC->LastTrack; track++) {
			if (LBA < isowavMSFToLBA(isowavTOC->TrackData[track + 1].Address)) {
				break;
			}
		}

		if (isowavFile == NULL || track != isowavTrack) {
			isowavStop();

			isowavTrack = track;

			bprintf(PRINT_IMPORTANT, _T("    reading track %2i - %s\n"), isowavTrack + 1, isowavTOC->TrackData[isowavTrack].Filename);

			isowavFile = _tfopen(isowavTOC->TrackData[isowavTrack].Filename, _T("rb"));
			if (isowavFile == NULL) {
				return 0;
			}
		}

		if (fseek(isowavFile, (LBA - isowavMSFToLBA(isowavTOC->TrackData[isowavTrack].Address)) * 2048, SEEK_SET)) {
			dprintf(_T("*** couldn't seek\n"));

			return 0;
		}

		isowavLBA = (ftell(isowavFile) + 2047) / 2048;

		CDEmuStatus = reading;
	}

	if (fread(pBuffer, 1, 2048, isowavFile) <= 0) {
		dprintf(_T("*** couldn't read from file\n"));

		isowavStop();

		return 0;
	}

	isowavLBA = isowavMSFToLBA(isowavTOC->TrackData[isowavTrack].Address) + (ftell(isowavFile) + 2047) / 2048;

	return isowavLBA - CD_FRAMES_PREGAP;
}

static unsigned char* isowavReadTOC(int track)
{
	static unsigned char TOCEntry[4];

	if (track == -1) {
		TOCEntry[0] = isowavTOC->FirstTrack - 1;
		TOCEntry[1] = isowavTOC->LastTrack;
		TOCEntry[2] = 0;
		TOCEntry[3] = 0;

		return TOCEntry;
	}
	if (track == -2) {
		TOCEntry[0] = isowavTOC->TrackData[(int)isowavTOC->LastTrack].Address[1];
		TOCEntry[1] = isowavTOC->TrackData[(int)isowavTOC->LastTrack].Address[2];
		TOCEntry[2] = isowavTOC->TrackData[(int)isowavTOC->LastTrack].Address[3];

		TOCEntry[3] = 0;

		return TOCEntry;
	}

	if (track >= isowavTOC->FirstTrack - 1 && track <= isowavTOC->LastTrack) {
		TOCEntry[0] = isowavTOC->TrackData[track - 1].Address[1];
		TOCEntry[1] = isowavTOC->TrackData[track - 1].Address[2];
		TOCEntry[2] = isowavTOC->TrackData[track - 1].Address[3];
		TOCEntry[3] = isowavTOC->TrackData[track - 1].Control;
	}

	return TOCEntry;
}

static unsigned char* isowavReadQChannel()
{
	static unsigned char QChannelData[8];

	switch (CDEmuStatus) {
		case reading:
		case playing: {
			const char* AddressAbs = isowavLBAToMSF(isowavLBA);
			const char* AddressRel = isowavLBAToMSF(isowavLBA - isowavMSFToLBA(isowavTOC->TrackData[isowavTrack].Address));
		
			QChannelData[0] = isowavTOC->TrackData[isowavTrack].TrackNumber;
		
			QChannelData[1] = AddressAbs[1];
			QChannelData[2] = AddressAbs[2];
			QChannelData[3] = AddressAbs[3];
		
			QChannelData[4] = AddressRel[1];
			QChannelData[5] = AddressRel[2];
			QChannelData[6] = AddressRel[3];
		
			QChannelData[7] = isowavTOC->TrackData[isowavTrack].Control;

			break;
		}
		case paused: {
			break;
		}
		default: {
			memset(QChannelData, 0, sizeof(QChannelData));
		}
	}
		
	return QChannelData;
}

static int isowavGetSoundBuffer(short* /*buffer*/, int /*samples*/)
{
	// ---------------------------------------------------------------------
	// TODO: 
	// Port the old 'isomp3GetSoundBuffer()' function from 'cd_isomp3.cpp' 
	// to use WAVE stream data, porting that function will fix the 
	// 00:00 progress status on the main NeoGeo CD BIOS menu.
	// ---------------------------------------------------------------------

	return 0;
}

static int isowavGetSettings(InterfaceInfo* /*pInfo*/)
{
	return 0;
}

struct CDEmuDo isowavDo = { isowavExit, isowavInit, isowavStop, isowavPlay, isowavLoadSector, isowavReadTOC, isowavReadQChannel, isowavGetSoundBuffer, isowavGetSettings, _T("cue/iso/wav CD emulation") };
