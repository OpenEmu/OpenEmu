#ifndef CD_INTERFACE_H_
#define CD_INTERFACE_H_

// ----------------------------------------------------------------------------
// CD emulation module

enum CDEmuStatusValue { idle = 0, reading, playing, paused, seeking, fastforward, fastreverse };

extern TCHAR CDEmuImage[MAX_PATH];

INT32 CDEmuInit();
INT32 CDEmuExit();
INT32 CDEmuStop();
INT32 CDEmuPlay(UINT8 M, UINT8 S, UINT8 F);
INT32 CDEmuLoadSector(INT32 LBA, char* pBuffer);
UINT8* CDEmuReadTOC(INT32 track);
UINT8* CDEmuReadQChannel();
INT32 CDEmuGetSoundBuffer(INT16* buffer, INT32 samples);

static inline CDEmuStatusValue CDEmuGetStatus()
{
	extern CDEmuStatusValue CDEmuStatus;

	return CDEmuStatus;
}

static inline void CDEmuStartRead()
{
	extern CDEmuStatusValue CDEmuStatus;

	CDEmuStatus = seeking;
}

static inline void CDEmuPause()
{
	extern CDEmuStatusValue CDEmuStatus;

	CDEmuStatus = paused;
}

#endif /*CD_INTERFACE_H_*/
