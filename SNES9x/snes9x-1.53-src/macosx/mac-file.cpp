/***********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2010  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),

  (c) Copyright 2002 - 2011  zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja

  (c) Copyright 2009 - 2011  BearOso,
                             OV2


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com),
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti

  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code used in 1.39-1.51
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  SPC7110 and RTC C++ emulator code used in 1.52+
  (c) Copyright 2009         byuu,
                             neviksti

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001 - 2006  byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound emulator code used in 1.5-1.51
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  Sound emulator code used in 1.52+
  (c) Copyright 2004 - 2007  Shay Green (gblargg@gmail.com)

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  NTSC filter
  (c) Copyright 2006 - 2007  Shay Green

  GTK+ GUI code
  (c) Copyright 2004 - 2011  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2011  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2011  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com/

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
 ***********************************************************************************/

/***********************************************************************************
  SNES9X for Mac OS (c) Copyright John Stiles

  Snes9x for Mac OS X

  (c) Copyright 2001 - 2011  zones
  (c) Copyright 2002 - 2005  107
  (c) Copyright 2002         PB1400c
  (c) Copyright 2004         Alexander and Sander
  (c) Copyright 2004 - 2005  Steven Seeger
  (c) Copyright 2005         Ryan Vogt
 ***********************************************************************************/


#include "snes9x.h"
#include "memmap.h"
#include "movie.h"
#include "display.h"

#include <libgen.h>

#include "mac-prefix.h"
#include "mac-dialog.h"
#include "mac-os.h"
#include "mac-stringtools.h"
#include "mac-file.h"

static void AddFolderIcon (FSRef *, const char *);
static OSStatus FindSNESFolder (FSRef *, char *, const char *);
static OSStatus FindApplicationSupportFolder (FSRef *, char *, const char *);
static OSStatus FindCustomFolder (FSRef *, char *, const char *);


void CheckSaveFolder (FSRef *cartRef)
{
	OSStatus		err;
	Boolean			r;
	FSCatalogInfo	finfo;
	FSVolumeInfo	vinfo;
	FSRef			ref;
	CFURLRef		burl, purl;
	char			s[PATH_MAX + 1];

	switch (saveInROMFolder)
	{
		case 0: // Snes9x folder
			burl = CFBundleCopyBundleURL(CFBundleGetMainBundle());
			purl = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, burl);
			r    = CFURLGetFSRef(purl, &ref);
			CFRelease(purl);
			CFRelease(burl);
			break;

		case 1: // ROM folder
			err = FSGetCatalogInfo(cartRef, kFSCatInfoNone, NULL, NULL, NULL, &ref);
			break;

		case 2: // Application Support folder
			return;

		case 4: // Custom folder
			if (saveFolderPath == NULL)
			{
				saveInROMFolder = 2;
				return;
			}

			r = CFStringGetCString(saveFolderPath, s, PATH_MAX, kCFStringEncodingUTF8);
			err = FSPathMakeRef((unsigned char *) s, &ref, NULL);
			if (err)
			{
				AppearanceAlert(kAlertCautionAlert, kS9xMacAlertFolderNotFound, kS9xMacAlertFolderNotFoundHint);
				saveInROMFolder = 2;
				return;
			}

			break;
	}

	err = FSGetCatalogInfo(&ref, kFSCatInfoUserPrivs | kFSCatInfoVolume, &finfo, NULL, NULL, NULL);
	if (err == noErr)
	{
		if (finfo.userPrivileges & kioACUserNoMakeChangesMask)
		{
			AppearanceAlert(kAlertCautionAlert, kS9xMacAlertFolderFailToCreate, kS9xMacAlertFolderFailToCreateHint);
			saveInROMFolder = 2;
			return;
		}

		err = FSGetVolumeInfo(finfo.volume, 0, NULL, kFSVolInfoFlags, &vinfo, NULL, NULL);
		if (err == noErr)
		{
			if ((vinfo.flags & kFSVolFlagHardwareLockedMask) || (vinfo.flags & kFSVolFlagSoftwareLockedMask))
			{
				AppearanceAlert(kAlertCautionAlert, kS9xMacAlertFolderFailToCreate, kS9xMacAlertFolderFailToCreateHint);
				saveInROMFolder = 2;
				return;
			}
		}
	}

	if (err)
		saveInROMFolder = 2;
}

static OSStatus FindSNESFolder (FSRef *folderRef, char *folderPath, const char *folderName)
{
	OSStatus	err;
	CFURLRef	burl, purl;
	CFStringRef	fstr;
	FSRef		pref;
	UniChar		buffer[PATH_MAX + 1];
	Boolean		r;

	fstr = CFStringCreateWithCString(kCFAllocatorDefault, folderName, CFStringGetSystemEncoding());
	CFStringGetCharacters(fstr, CFRangeMake(0, CFStringGetLength(fstr)), buffer);

	burl = CFBundleCopyBundleURL(CFBundleGetMainBundle());
	purl = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, burl);
	r    = CFURLGetFSRef(purl, &pref);

	err = FSMakeFSRefUnicode(&pref, CFStringGetLength(fstr), buffer, kTextEncodingUnicodeDefault, folderRef);
	if (err == dirNFErr || err == fnfErr)
	{
		err = FSCreateDirectoryUnicode(&pref, CFStringGetLength(fstr), buffer, kFSCatInfoNone, NULL, folderRef, NULL, NULL);
		if (err == noErr)
			AddFolderIcon(folderRef, folderName);
	}

	if (err == noErr)
		err = FSRefMakePath(folderRef, (unsigned char *) folderPath, PATH_MAX);

	CFRelease(purl);
	CFRelease(burl);
	CFRelease(fstr);

	return (err);
}

static OSStatus FindApplicationSupportFolder (FSRef *folderRef, char *folderPath, const char *folderName)
{
	OSStatus	err;
	FSRef		p2ref, p1ref;
	CFStringRef	fstr;
	UniChar		buffer[PATH_MAX + 1];
	UniChar		s9xfolder[6] = { 'S', 'n', 'e', 's', '9', 'x' },
				oldfolder[6] = { 'S', 'N', 'E', 'S', '9', 'X' };

	err = FSFindFolder(kUserDomain, kApplicationSupportFolderType, kCreateFolder, &p2ref);
	if (err)
		return (err);

	err = FSMakeFSRefUnicode(&p2ref, 6, s9xfolder, kTextEncodingUnicodeDefault, &p1ref);
	if (err == dirNFErr || err == fnfErr)
	{
		err = FSMakeFSRefUnicode(&p2ref, 6, oldfolder, kTextEncodingUnicodeDefault, &p1ref);
		if (err == dirNFErr || err == fnfErr)
			err = FSCreateDirectoryUnicode(&p2ref, 6, s9xfolder, kFSCatInfoNone, NULL, &p1ref, NULL, NULL);
	}

	if (err)
		return (err);

	fstr = CFStringCreateWithCString(kCFAllocatorDefault, folderName, CFStringGetSystemEncoding());
	CFStringGetCharacters(fstr, CFRangeMake(0, CFStringGetLength(fstr)), buffer);

	err = FSMakeFSRefUnicode(&p1ref, CFStringGetLength(fstr), buffer, kTextEncodingUnicodeDefault, folderRef);
	if (err == dirNFErr || err == fnfErr)
	{
		err = FSCreateDirectoryUnicode(&p1ref, CFStringGetLength(fstr), buffer, kFSCatInfoNone, NULL, folderRef, NULL, NULL);
		if (err == noErr)
			AddFolderIcon(folderRef, folderName);
	}

	if (err == noErr)
		err = FSRefMakePath(folderRef, (unsigned char *) folderPath, PATH_MAX);

	CFRelease(fstr);

	return (err);
}

static OSStatus FindCustomFolder (FSRef *folderRef, char *folderPath, const char *folderName)
{
	OSStatus	err;
	CFStringRef	fstr;
	FSRef		pref;
	UniChar		buffer[PATH_MAX + 1];
	char		s[PATH_MAX + 1];

	if (saveFolderPath == NULL)
		return (-1);

	err = CFStringGetCString(saveFolderPath, s, PATH_MAX, kCFStringEncodingUTF8) ? noErr : -1;
	if (err == noErr)
		err = FSPathMakeRef((unsigned char *) s, &pref, NULL);

	if (err)
		return (err);

	fstr = CFStringCreateWithCString(kCFAllocatorDefault, folderName, CFStringGetSystemEncoding());
	CFStringGetCharacters(fstr, CFRangeMake(0, CFStringGetLength(fstr)), buffer);

	err = FSMakeFSRefUnicode(&pref, CFStringGetLength(fstr), buffer, kTextEncodingUnicodeDefault, folderRef);
	if (err == dirNFErr || err == fnfErr)
	{
		err = FSCreateDirectoryUnicode(&pref, CFStringGetLength(fstr), buffer, kFSCatInfoNone, NULL, folderRef, NULL, NULL);
		if (err == noErr)
			AddFolderIcon(folderRef, folderName);
	}

	if (err == noErr)
		err = FSRefMakePath(folderRef, (unsigned char *) folderPath, PATH_MAX);

	CFRelease(fstr);

	return (err);
}

void ChangeTypeAndCreator (const char *path, OSType type, OSType creator)
{
	OSStatus	err;
	FSRef		ref;

	err = FSPathMakeRef((unsigned char *) path, &ref, NULL);
	if (err == noErr)
	{
		FSCatalogInfo	catinfo;

		err = FSGetCatalogInfo(&ref, kFSCatInfoFinderInfo, &catinfo, NULL, NULL, NULL);
		if (err == noErr)
		{
			((FileInfo *) &catinfo.finderInfo)->fileCreator = creator;
			((FileInfo *) &catinfo.finderInfo)->fileType    = type;

			err = FSSetCatalogInfo(&ref, kFSCatInfoFinderInfo, &catinfo);
		}
	}
}

static void AddFolderIcon (FSRef *fref, const char *folderName)
{
	OSStatus			err;
	FSCatalogInfo		fcat, icat;
	FSRef				bref, iref;
	CFStringRef			str;
	CFURLRef			url;
	IconFamilyHandle	family;
	IconRef				icon;
	HFSUniStr255		fork;
	Boolean				r;
	SInt16				resf;
	char				name[64];
	UniChar				iconName[5] = { 'I', 'c', 'o', 'n', '\r' };

	strcpy(name, "folder_");
	strcat(name, folderName);

	str = CFStringCreateWithCString(kCFAllocatorDefault, name, CFStringGetSystemEncoding());
	if (str)
	{
		url = CFBundleCopyResourceURL(CFBundleGetMainBundle(), str, CFSTR("icns"), NULL);
		if (url)
		{
			r = CFURLGetFSRef(url, &bref);
			if (r)
			{
				err = RegisterIconRefFromFSRef('~9X~', 'TEMP', &bref, &icon);
				if (err == noErr)
				{
					err = FSGetResourceForkName(&fork);
					if (err == noErr)
					{
						err = FSCreateResourceFile(fref, 5, iconName, kFSCatInfoNone, NULL, fork.length, fork.unicode, &iref, NULL);
						if (err == noErr)
						{
							err = FSOpenResourceFile(&iref, fork.length, fork.unicode, fsWrPerm, &resf);
							if (err == noErr)
							{
								err = IconRefToIconFamily(icon, kSelectorAllAvailableData, &family);
								if (err == noErr)
								{
									AddResource((Handle) family, 'icns', -16455, "\p");
									WriteResource((Handle) family);
									ReleaseResource((Handle) family);

									err = FSGetCatalogInfo(&iref, kFSCatInfoFinderInfo, &icat, NULL, NULL, NULL);
									((FileInfo *) &icat.finderInfo)->finderFlags |= kIsInvisible;
									((FileInfo *) &icat.finderInfo)->fileCreator = 'MACS';
									((FileInfo *) &icat.finderInfo)->fileType    = 'icon';
									err = FSSetCatalogInfo(&iref, kFSCatInfoFinderInfo, &icat);

									err = FSGetCatalogInfo(fref, kFSCatInfoFinderInfo, &fcat, NULL, NULL, NULL);
									((FolderInfo *) &fcat.finderInfo)->finderFlags |=  kHasCustomIcon;
									((FolderInfo *) &fcat.finderInfo)->finderFlags &= ~kHasBeenInited;
									err = FSSetCatalogInfo(fref, kFSCatInfoFinderInfo, &fcat);
								}

								CloseResFile(resf);
							}
						}
					}

					err = UnregisterIconRef('~9X~', 'TEMP');
				}
			}

			CFRelease(url);
		}

		CFRelease(str);
	}
}

const char * S9xGetFilename (const char *inExt, enum s9x_getdirtype dirtype)
{
	static int	index = 0;
	static char	filePath[4][PATH_MAX + 1];

	OSStatus	err;
	FSRef		ref;
	uint32		type;
	char		folderName[16];
	char		drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];
	const char	*p;

	index++;
	if (index > 3)
		index = 0;

	folderName[0] = filePath[index][0] = 0;

	if (strlen(inExt) < 4)
		return (filePath[index]);

	p = inExt + strlen(inExt) - 4;
	type = ((uint32) p[0] << 24) + ((uint32) p[1] << 16) + ((uint32) p[2] << 8) + (uint32) p[3];

	switch (type)
	{
		case '.srm':
		case '.rtc':
			strcpy(folderName, "SRAMs");
			break;

		case '.frz':
			strcpy(folderName, "Freezes");
			break;

		case '.spc':
			strcpy(folderName, "SPCs");
			break;

		case '.cht':
			strcpy(folderName, "Cheats");
			break;

		case '.ups':
		case '.ips':
			strcpy(folderName, "Patches");
			break;

		case '.png':
			strcpy(folderName, "Screenshots");
			break;

		case '.dat':
		case '.out':
		case '.log':
			strcpy(folderName, "Logs");
			break;

		case '.bio':	// dummy
			strcpy(folderName, "BIOSes");
			break;
	}

	if (folderName[0] && (saveInROMFolder != 1))
	{
		char	s[PATH_MAX + 1];

		s[0] = 0;
		err = -1;

		if (saveInROMFolder == 0)
		{
			err = FindSNESFolder(&ref, s, folderName);
			if (err)
				saveInROMFolder = 2;
		}

		if (saveInROMFolder == 4)
		{
			err = FindCustomFolder(&ref, s, folderName);
			if (err)
				saveInROMFolder = 2;
		
		}

		if (saveInROMFolder == 2)
			err = FindApplicationSupportFolder(&ref, s, folderName);

		if (err == noErr)
		{
			_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
			snprintf(filePath[index], PATH_MAX + 1, "%s%s%s%s", s, MAC_PATH_SEPARATOR, fname, inExt);
		}
		else
		{
			_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
			_makepath(filePath[index], drive, dir, fname, inExt);
		}
	}
	else
	{
	    _splitpath(Memory.ROMFilename, drive, dir, fname, ext);
		_makepath(filePath[index], drive, dir, fname, inExt);
	}

	return (filePath[index]);
}

const char * S9xGetSPCFilename (void)
{
	char	spcExt[16];

	sprintf(spcExt, ".%03d.spc", (int) spcFileCount);

	spcFileCount++;
	if (spcFileCount == 1000)
		spcFileCount = 0;

	return (S9xGetFilename(spcExt, SPC_DIR));
}

const char * S9xGetPNGFilename (void)
{
	char	pngExt[16];

	sprintf(pngExt, ".%03d.png", (int) pngFileCount);

	pngFileCount++;
	if (pngFileCount == 1000)
		pngFileCount = 0;

	return (S9xGetFilename(pngExt, SCREENSHOT_DIR));
}

const char * S9xGetFreezeFilename (int which)
{
	char	frzExt[16];

	sprintf(frzExt, ".%03d.frz", which);

	return (S9xGetFilename(frzExt, SNAPSHOT_DIR));
}

const char * S9xGetFilenameInc (const char *inExt, enum s9x_getdirtype dirtype)
{
	uint32		type;
	const char	*p;

	if (strlen(inExt) < 4)
		return (NULL);

	p = inExt + strlen(inExt) - 4;
	type = ((uint32) p[0] << 24) + ((uint32) p[1] << 16) + ((uint32) p[2] << 8) + (uint32) p[3];

	switch (type)
	{
		case '.spc':
			return (S9xGetSPCFilename());

		case '.png':
			return (S9xGetPNGFilename());
	}

	return (NULL);
}

const char * S9xChooseFilename (bool8 read_only)
{
	return (NULL);
}

const char * S9xChooseMovieFilename (bool8 read_only)
{
	return (NULL);
}

bool8 S9xOpenSnapshotFile (const char *fname, bool8 read_only, STREAM *file)
{
    if (read_only)
    {
		if (0 != (*file = OPEN_STREAM(fname, "rb")))
		    return (true);
    }
    else
    {
		if (0 != (*file = OPEN_STREAM(fname, "wb")))
		    return (true);
    }

    return (false);
}

void S9xCloseSnapshotFile (STREAM file)
{
    CLOSE_STREAM(file);
}

const char * S9xBasename (const char *in)
{
	static char	s[PATH_MAX + 1];

	strncpy(s, in, PATH_MAX + 1);
	s[PATH_MAX] = 0;

	size_t	l = strlen(s);

	for (unsigned int i = 0; i < l; i++)
    {
		if (s[i] < 32 || s[i] >= 127)
			s[i] = '_';
	}

	return (basename(s));
}

const char * S9xGetDirectory (enum s9x_getdirtype dirtype)
{
	static int	index = 0;
	static char	path[4][PATH_MAX + 1];

	char	inExt[16];
	char	drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	index++;
	if (index > 3)
		index = 0;

	switch (dirtype)
	{
		case SNAPSHOT_DIR:		strcpy(inExt, ".frz");	break;
		case SRAM_DIR:			strcpy(inExt, ".srm");	break;
		case SCREENSHOT_DIR:	strcpy(inExt, ".png");	break;
		case SPC_DIR:			strcpy(inExt, ".spc");	break;
		case CHEAT_DIR:			strcpy(inExt, ".cht");	break;
		case BIOS_DIR:			strcpy(inExt, ".bio");	break;
		case LOG_DIR:			strcpy(inExt, ".log");	break;
		default:				strcpy(inExt, ".xxx");	break;
	}

	_splitpath(S9xGetFilename(inExt, dirtype), drive, dir, fname, ext);
	_makepath(path[index], drive, dir, "", "");

	int	l = strlen(path[index]);
	if (l > 1)
		path[index][l - 1] = 0;

	return (path[index]);
}

void _splitpath (const char *path, char *drive, char *dir, char *fname, char *ext)
{
	drive[0] = 0;
	fname[0] = 0;
	ext[0]   = 0;
	dir[0]   = 0;

	int	x;

	x = strlen(path) - 1;
	if (x < 0)
		return;

	while (x && (path[x] != MAC_PATH_SEP_CHAR))
		x--;

	if (x)
	{
		strcpy(dir, path);
		dir[x + 1] = 0;

		strcpy(fname, path + x + 1);
	}
	else
		strcpy(fname, path);

	x = strlen(fname);
	while (x && (fname[x] != '.'))
		x--;

	if (x)
	{
		strcpy(ext, fname + x);
		fname[x] = 0;
	}
}

void _makepath (char *path, const char *drive, const char *dir, const char *fname, const char *ext)
{
	static const char	emp[] = "", dot[] = ".";

	const char	*d, *f, *e, *p;

	d = dir ? dir : emp;
	f = fname ? fname : emp;
	e = ext ? ext : emp;
	p = (e[0] && e[0] != '.') ? dot : emp;

	snprintf(path, PATH_MAX + 1, "%s%s%s%s", d, f, p, e);
}
