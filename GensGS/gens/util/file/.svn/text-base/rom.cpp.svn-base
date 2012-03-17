#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <ctype.h>

#include <fcntl.h>

#include <string>
#include <list>
using std::string;
using std::list;

#include "rom.hpp"

#include "emulator/g_main.hpp"
#include "emulator/gens.hpp"
#include "emulator/g_md.hpp"
#include "emulator/g_mcd.hpp"
#include "emulator/g_32x.hpp"
#include "util/file/ggenie.h"
#include "gens_core/cpu/68k/cpu_68k.h"
#include "segacd/cd_sys.hpp"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "util/file/save.hpp"
#include "util/sound/wave.h"
#include "util/sound/gym.hpp"

#include "gens_core/misc/misc.h"
#include "gens_core/misc/byteswap.h"

#include "ui/gens_ui.hpp"
#if !defined(GENS_UI_OPENEMU)
#include "zip_select/zip_select_dialog.hpp"
#include "gens/gens_window.hpp"
#endif

// New file compression handler.
#include "util/file/compress/compressor.hpp"

char Recent_Rom[9][GENS_PATH_MAX];
char Rom_Dir[GENS_PATH_MAX];
char IPS_Dir[GENS_PATH_MAX];

ROM_t* Game = NULL;
char ROM_Name[512];

ROM_t* myROM = NULL;


/**
 * getNameFromPath(): Get the filename part of a pathname.
 * @param fullPath Full pathname.
 * @return Filename part of the pathname.
 */
string ROM::getNameFromPath(const string& fullPath)
{
	size_t pos = fullPath.rfind(GENS_DIR_SEPARATOR_CHR);
	
	if (pos == string::npos)
		return fullPath;
	else if (pos + 1 == fullPath.length())
		return "";
	
	return fullPath.substr(pos + 1);
}


/**
 * getDirFromPath(): Get the directory part of a pathname.
 * @param fullPath Full pathname.
 * @return Directory part of the pathname.
 */
string ROM::getDirFromPath(const string& fullPath)
{
	size_t pos = fullPath.rfind(GENS_DIR_SEPARATOR_CHR);
	
	if (pos == string::npos)
		return "";
	
	return fullPath.substr(0, pos + 1);
}


/**
 * updateRecentROMList(): Update the Recent ROM list with the given ROM filename.
 * @param filename Full pathname to a ROM file.
 */
void ROM::updateRecentROMList(const char* filename)
{
	int i;
	
	for (i = 0; i < 9; i++)
	{
		// Check if the ROM exists in the Recent ROM list.
		// If it does, don't do anything.
		// TODO: If it does, move it up to position 1.
		if (!(strcmp(Recent_Rom[i], filename)))
			return;
	}
	
	// Move all recent ROMs down by one index.
	for (i = 8; i > 0; i--)
		strcpy(Recent_Rom[i], Recent_Rom[i - 1]);
	
	// Add this ROM to the recent ROM list.
	strcpy(Recent_Rom[0], filename);
}


/**
 * Update_Rom_Dir(): Update the Rom_Dir using the path of the specified ROM file.
 * @param filename Full pathname to a ROM file.
 */
void ROM::updateROMDir(const char *filename)
{
	string tmpROMDir = getDirFromPath(filename);
	strncpy(Rom_Dir, tmpROMDir.c_str(), sizeof(Rom_Dir));
	Rom_Dir[sizeof(Rom_Dir) - 1] = 0x00;
}


// FIXME: This function is poorly written.
void ROM::updateROMName(const char *filename)
{
	int length = strlen(filename) - 1;
	
	while ((length >= 0) && (filename[length] != GENS_DIR_SEPARATOR_CHR))
		length--;
	
	length++;
	
	int i = 0;
	while ((filename[length]) && (filename[length] != '.'))
		ROM_Name[i++] = filename[length++];
	
	ROM_Name[i] = 0;
}


/**
 * updateCDROMName(): Update the name of a SegaCD game.
 * @param cdromName Name of the SegaCD game.
 */
void ROM::updateCDROMName(const char *cdromName)
{
	int i, j;
	bool validName = false;
	
	// Copy the CD-ROM name to ROM_Name.
	// NOTE: cdromName is only 32 bytes.
	// TODO: Increase it to 48 bytes.
	memcpy(ROM_Name, cdromName, 32);
	memset(&ROM_Name[32], ' ', 16);
	
	// Check for invalid characters.
	for (i = 0; i < 48; i++)
	{
		if (isalnum(ROM_Name[i]))
		{
			// Valid character.
			validName = true;
			continue;
		}
		
		// Invalid character. Replace it with a space.
		ROM_Name[i] = ' ';
	}
	
	if (!validName)
	{
		// CD-ROM name is invalid. Assume that no disc is inserted.
		ROM_Name[0] = 0x00;
		return;
	}
	
	// Make sure the name is null-terminated.
	ROM_Name[48] = 0x00;
	for (i = 47, j = 48; i >= 0; i--, j--)
	{
		if (ROM_Name[i] != ' ')
			i = -1;
	}
	ROM_Name[j + 1] = 0;
}

// Temporary C wrapper functions.
// TODO: Eliminate this.
ROMType detectFormat(const unsigned char buf[2048])
{
	return ROM::detectFormat(buf);
}


/**
 * Detect_Format(): Detect the format of a given ROM header.
 * @param buf Buffer containing the first 2048 bytes of the ROM file.
 * @return ROMType.
 */
ROMType ROM::detectFormat(const unsigned char buf[2048])
{
	bool interleaved = false;
	
	// SegaCD check
	if (!strncasecmp("SEGADISCSYSTEM", (char*)(&buf[0x00]), 14))
	{
		// SegaCD image, ISO9660 format.
		return SegaCD_Image;
	}
	else if (!strncasecmp("SEGADISCSYSTEM", (char*)(&buf[0x10]), 14))
	{
		// SegaCD image, BIN/CUE format.
		// TODO: Proper BIN/CUE audio support, if it's not done already.
		return SegaCD_Image_BIN;
	}
	
	// Check if this is an interleaved ROM.
	if (strncasecmp("SEGA", (char*)(&buf[0x100]), 4))
	{
		// No "SEGA" text in the header. This might be an interleaved ROM.
		if (((buf[0x08] == 0xAA) && (buf[0x09] == 0xBB) && (buf[0x0A] == 0x06)) ||
		    (!strncasecmp((char*)(&buf[0x280]), "EA", 2)))
		{
			// Interleaved.
			interleaved = true;
		}
	}
	
	// Check if this is a 32X ROM.
	if (interleaved)
	{
		// Interleaved 32X check.
		if (buf[0x0300] == 0xF9)
		{
			if ((!strncasecmp((char*)&buf[0x0282], "3X", 2)) ||
			    (!strncasecmp((char*)&buf[0x0407], "AS", 2)))
			{
				// Interleaved 32X ROM.
				return _32X_ROM_Interleaved;
			}
		}
	}
	else
	{
		// Non-interleaved 32X check.
		if (buf[0x0200] == 0x4E)
		{
			if ((!strncasecmp((char*)(&buf[0x0105]), "32X", 3)) ||
			    (!strncasecmp((char*)(&buf[0x040E]), "MARS", 4)))
			{
				// Non-interleaved 32X ROM.
				return _32X_ROM;
			}
		}
	}
	
	// Assuming this is a Genesis ROM.
	
	if (interleaved)
		return MD_ROM_Interleaved;
	
	return MD_ROM;
}


/**
 * detectFormat_fopen(): Detect the format of a given ROM file.
 * @param filename Filename of the ROM file.
 * @return ROMType.
 */
ROMType ROM::detectFormat_fopen(const char* filename)
{
	Compressor *cmp;
	list<CompressedFile> *files;
	ROMType rtype;
	
	// Open the ROM file using the compressor functions.
	cmp = new Compressor(filename);
	if (!cmp->isFileLoaded())
	{
		// Cannot load the file.
		delete cmp;
		return (ROMType)0;
	}
	
	// Get the file information.
	files = cmp->getFileInfo();
	
	// Check how many files are available.
	if (!files || files->empty())
	{
		// No files.
		delete files;
		delete cmp;
		return (ROMType)0;
	}
	
	// Get the first file in the archive.
	// TODO: Store the compressed filename in ROM history.
	unsigned char detectBuf[2048];
	cmp->getFile(&(*files->begin()), detectBuf, sizeof(detectBuf));
	rtype = detectFormat(detectBuf);
	
	// Return the ROM type.
	delete files;
	delete cmp;
	return rtype;
}


/**
 * deinterleaveSMD(): Deinterleaves an SMD-format ROM.
 */
void ROM::deinterleaveSMD(void)
{
	unsigned char buf[0x4000];
	unsigned char *Src;
	int i, j, Nb_Blocks, ptr;
	
	// SMD interleave format has a 512-byte header at the beginning of the ROM.
	// After the header, the ROM is broken into 16 KB chunks.
	// The first 8 KB of each 16 KB block are the odd bytes.
	// The second 8 KB of each 16 KB block are the even bytes.
	
	// Start at 0x200 bytes (after the SMD header).
	Src = &Rom_Data[0x200];
	
	// Subtract the SMD header length from the ROM size.
	Rom_Size -= 0x200;
	
	// Determine how many 16 KB blocks are in the ROM.
	Nb_Blocks = Rom_Size / 0x4000;
	
	// Deinterleave the blocks.
	for (ptr = 0, i = 0; i < Nb_Blocks; i++, ptr += 0x4000)
	{
		// Copy the current 16 KB block to a temporary buffer.
		memcpy(buf, &Src[ptr], 0x4000);
		
		// Go through both 8 KB sub-blocks at the same time.
		for (j = 0; j < 0x2000; j++)
		{
			// Odd byte, first 8 KB
			Rom_Data[ptr + (j << 1) + 1] = buf[j];
			// Even byte, second 8 KB
			Rom_Data[ptr + (j << 1)] = buf[j + 0x2000];
		}
	}
}


/**
 * fillROMInfo(): Fill in game information from the ROM header.
 */
void ROM::fillROMInfo(void)
{
	// Finally we do the IPS patch here, we can have the translated game name
	applyIPSPatch();
	
	// Copy ROM text.
	// TODO: Use constants for the ROM addresses.
	memcpy(myROM->Console_Name,	&Rom_Data[0x100], 16);
	memcpy(myROM->Copyright,	&Rom_Data[0x110], 16);
	memcpy(myROM->ROM_Name,		&Rom_Data[0x120], 48);
	memcpy(myROM->ROM_Name_W,	&Rom_Data[0x150], 48);
	memcpy(myROM->Type,		&Rom_Data[0x180], 2);
	memcpy(myROM->Version,		&Rom_Data[0x182], 12);
	myROM->Checksum			= be16_to_cpu_from_ptr(&Rom_Data[0x18E]);
	memcpy(myROM->IO_Support,	&Rom_Data[0x190], 16);
	myROM->ROM_Start_Address	= be32_to_cpu_from_ptr(&Rom_Data[0x1A0]);
	myROM->ROM_End_Address		= be32_to_cpu_from_ptr(&Rom_Data[0x1A4]);
	memcpy(myROM->RAM_Info,		&Rom_Data[0x1A8], 12);
	myROM->RAM_Start_Address	= be32_to_cpu_from_ptr(&Rom_Data[0x1B4]);
	myROM->RAM_End_Address		= be32_to_cpu_from_ptr(&Rom_Data[0x1B8]);
	memcpy(myROM->Modem_Info,	&Rom_Data[0x1BC], 12);
	memcpy(myROM->Description,	&Rom_Data[0x1C8], 40);
	memcpy(myROM->Countries,	&Rom_Data[0x1F0], 4);
	
	char tmp[15];
	memcpy(&tmp[0], myROM->Type, 2);
	memcpy(&tmp[2], myROM->Version, 12);
	tmp[14] = 0;
	if (strcmp(tmp, "\107\115\040\060\060\060\060\061\060\065\061\055\060\060") == 0)
		ice = 1;
	
	// Calculate internal ROM size using the ROM header's
	// starting address and ending address.
	myROM->R_Size = myROM->ROM_End_Address - myROM->ROM_Start_Address + 1;
	
	// Null-terminate the strings.
	myROM->Console_Name[16] = 0;
	myROM->Copyright[16] = 0;
	myROM->ROM_Name[48] = 0;
	myROM->ROM_Name_W[48] = 0;
	myROM->Type[2] = 0;
	myROM->Version[12] = 0;
	myROM->IO_Support[12] = 0;
	myROM->RAM_Info[12] = 0;
	myROM->Modem_Info[12] = 0;
	myROM->Description[40] = 0;
	myROM->Countries[3] = 0;
}


/**
 * getROM(): Show the OpenFile dialog and load the selected ROM file.
 * @return Error code from Open_Rom().
 */
int ROM::getROM(void)
{
#if !defined(GENS_UI_OPENEMU)
	string filename;
	
	filename = GensUI::openFile("Open ROM", Rom_Dir, ROMFile);
	if (filename.length() == 0)
	{
		// No ROM selected.
		return 0;
	}
	
	// Open the ROM.
	return openROM(filename.c_str());
#else
	return 0;
#endif
}


/**
 * Open_Rom(): Open the specified ROM file.
 * @param Name Filename of the ROM file.
 * @return Unknown.
 */
int ROM::openROM(const char *Name)
{
	int sys;
	
	/*
	Free_Rom(Game);
	sys = Detect_Format(Name);
	
	if (sys < 1)
		return -1;
	*/
	
	// Close any loaded ROM first.
	freeROM(Game);
	
	sys = loadROM(Name, &Game);
	if (sys <= 0)
		return -1;
	
	updateRecentROMList(Name);
	updateROMDir(Name);
	
	switch (sys)
	{
		default:
		case MD_ROM:
		case MD_ROM_Interleaved:
			if (Game)
				Genesis_Started = Init_Genesis(Game);
			
			return Genesis_Started;
			break;
	
		case _32X_ROM:
		case _32X_ROM_Interleaved:
			if (Game)
				_32X_Started = Init_32X(Game);
			
			return _32X_Started;
			break;
		
		case SegaCD_Image:
		case SegaCD_Image_BIN:
			SegaCD_Started = Init_SegaCD(Name);
			
			return SegaCD_Started;
			break;
		
		case SegaCD_32X_Image:
		case SegaCD_32X_Image_BIN:
			break;
	}
	
	return -1;
}


/**
 * loadSegaCD_BIOS(): Load a SegaCD BIOS ROM image.
 * @param filename Filename of the SegaCD BIOS ROM image.
 * @return Pointer to Rom struct with the ROM information.
 */
ROM_t* ROM::loadSegaCD_BIOS(const char *filename)
{
	FILE *f;
	
	// This basically just checks if the BIOS ROM image can be opened.
	// TODO: Show an error message if it can't be opened.
	if ((f = fopen(filename, "rb")) == 0)
		return 0;
	fclose(f);
	
	// Close any ROM that's currently running.
	freeROM(Game);
	
	// Load the SegaCD BIOS ROM image.
	loadROM(filename, &Game);
	return Game;
}


/**
 * loadROM(): Load a ROM file.
 * @param filename Filename of the ROM file.
 * @param interleaved If non-zero, the ROM is interleaved.
 * @return Pointer to Rom struct with the ROM information.
 */
ROMType ROM::loadROM(const char* filename, ROM_t** retROM)
{
	Compressor *cmp;
	list<CompressedFile> *files;
	CompressedFile* selFile;
	ROMType rtype;
	
	// Set up the compressor.
	cmp = new Compressor(filename, true);
	if (!cmp->isFileLoaded())
	{
		// Error loading the file.
		delete cmp;
		Game = NULL;
		*retROM = NULL;
		return (ROMType)0;
	}
	
	// Get the file information.
	files = cmp->getFileInfo();
	
	// Check how many files are available.
	if (!files || files->empty())
	{
		// No files in the archive.
		// TODO: For 7z, suggest setting the 7z binary filename.
		GensUI::msgBox("No files were detected in this archive.", "No Files Detected");
		
		if (files)
			delete files;
		if (cmp)
			delete cmp;
		
		files = NULL;
		cmp = NULL;
		Game = NULL;
		*retROM = NULL;
		return (ROMType)0;
	}
	else if (files->size() == 1)
	{
		// One file is in the archive. Load it.
		selFile = &(*files->begin());
	}
	else
	{
#if !defined(GENS_UI_OPENEMU)
		// More than one file is in the archive. Load it.
		// TODO: Improve this!
		#if defined(GENS_UI_GTK)
			ZipSelectDialog *zip = new ZipSelectDialog(GTK_WINDOW(gens_window));
		#elif defined(GENS_UI_WIN32)
			ZipSelectDialog *zip = new ZipSelectDialog(Gens_hWnd);
		#else
			#error Can't determine UI.
		#endif
		selFile = zip->getFile(files);
		delete zip;
#endif
	}
	
	if (!selFile)
	{
		// No file was selected and/or Cancel was clicked.
		delete files;
		delete cmp;
		Game = NULL;
		*retROM = NULL;
		return (ROMType)0;
	}
	
	// Determine the ROM type.
	unsigned char detectBuf[2048];
	cmp->getFile(&(*selFile), detectBuf, sizeof(detectBuf));
	rtype = detectFormat(detectBuf);
	if (rtype < MD_ROM ||
	    rtype >= SegaCD_Image)
	{
		// Unknown ROM type, or this is a SegaCD image.
		delete files;
		delete cmp;
		Game = NULL;
		*retROM = NULL;
		return rtype;
	}
	
	// If the ROM is larger than 6MB (+512 bytes for SMD interleaving), don't load it.
	if (selFile->filesize > ((6 * 1024 * 1024) + 512))
	{
		GensUI::msgBox("ROM files larger than 6 MB are not supported.", "ROM File Error");
		delete files;
		delete cmp;
		Game = NULL;
		*retROM = NULL;
		return (ROMType)0;
	}
	
	myROM = (ROM_t*)malloc(sizeof(ROM_t));
	if (!myROM)
	{
		// Memory allocation error
		delete files;
		delete cmp;
		Game = NULL;
		*retROM = NULL;
		return (ROMType)0;
	}
	//fseek(ROM_File, 0, SEEK_SET);
	
	// Clear the ROM buffer and load the ROM.
	memset(Rom_Data, 0, 6 * 1024 * 1024);
	int loadedSize = cmp->getFile(&(*selFile), Rom_Data, selFile->filesize);
	if (loadedSize != selFile->filesize)
	{
		// Incorrect filesize.
		GensUI::msgBox("Error loading the ROM file.", "ROM File Error");
		free(myROM);
		delete files;
		delete cmp;
		myROM = NULL;
		Game = NULL;
		*retROM = NULL;
		return (ROMType)0;
	}
	//fclose(ROM_File);
	
	updateROMName(filename);
	Rom_Size = selFile->filesize;
	
	// Delete the compression objects.
	delete files;
	delete cmp;
	
	// Deinterleave the ROM, if necessary.
	if (rtype == MD_ROM_Interleaved ||
	    rtype == _32X_ROM_Interleaved)
		deinterleaveSMD();
	
	fillROMInfo();
	
	*retROM = myROM;
	return rtype;
}


/**
 * calcChecksum(): Calculates the checksum of the loaded ROM.
 * @return Checksum of the loaded ROM.
 */
unsigned short ROM::calcChecksum(void)
{
	unsigned short checksum = 0;
	unsigned int i;
	
	// A game needs to be loaded in order for this function to work...
	if (!Game)
		return 0;
	
	// Checksum starts at 0x200, past the vector table and ROM header.
	for (i = 0x200; i < Rom_Size; i += 2)
	{
		// Remember, since the MC68000 is little-endian, we can't
		// just cast Rom_Data[i] to an unsigned short directly.
		checksum += (unsigned short)(Rom_Data[i + 0]) +
			    (unsigned short)(Rom_Data[i + 1] << 8);
	}
	
	return checksum;
}


/**
 * fixChecksum(): Fixes the checksum of the loaded ROM.
 */
void ROM::fixChecksum(void)
{
	unsigned short checks;
	
	if (!Game)
		return;
	
	// Get the checksum.
	checks = calcChecksum();
	
	if (Rom_Size)
	{
		// MC68000 checksum.
		// MC68000 is big-endian.
		Rom_Data[0x18E] = checks & 0xFF;
		Rom_Data[0x18F] = checks >> 8;
		
		// SH2 checksum.
		// SH2 is little-endian.
		// TODO: Only do this if the 32X is active.
		_32X_Rom[0x18E] = checks >> 8;;
		_32X_Rom[0x18F] = checks & 0xFF;
	}
}


int ROM::applyIPSPatch(void)
{
	FILE* IPS_File;
	char filename[GENS_PATH_MAX];
	unsigned char buf[16];
	unsigned int adr, len, i;
	
	strcpy(filename, IPS_Dir);
	strcat(filename, ROM_Name);
	strcat(filename, ".ips");
	
	IPS_File = fopen(filename, "rb");
	
	if (!IPS_File)
		return 1;
	
	fseek(IPS_File, 0, SEEK_SET);
	fread(buf, 1, 5, IPS_File);
	buf[5] = 0;
	
	// Check the "magic number".
	if (strcasecmp((char*)buf, "patch"))
	{
		fclose(IPS_File);
		return 2;
	}
	
	fread(buf, 1, 3, IPS_File);
	buf[3] = 0;
	
	while (strcasecmp((char*)buf, "eof"))
	{
		adr = (unsigned int)(buf[2] + (buf[1] << 8) + (buf[0] << 16));
		
		if (fread(buf, 1, 2, IPS_File) == 0)
		{
			fclose(IPS_File);
			return 3;
		}
		
		len = (unsigned int)(buf[1] + (buf[0] << 8));
		for (i = 0; i < len; i++)
		{
			if (fread(buf, 1, 1, IPS_File) == 0)
			{
				fclose(IPS_File);
				return 3;
			}
			
			if (adr < Rom_Size)
				Rom_Data[adr++] = buf[0];
		}
		
		if (fread(buf, 1, 3, IPS_File) == 0)
		{
			fclose(IPS_File);
			return 3;
		}
		
		buf[3] = 0;
	}
	
	fclose(IPS_File);
	return 0;
}


void ROM::freeROM(ROM_t* ROM_MD)
{
	if (!Game)
		return;
	
	// Clear the sound buffer.
	audio->clearSoundBuffer();
	
	if (SegaCD_Started)
		Savestate::saveBRAM();
	
	Savestate::saveSRAM();
	Save_Patch_File();
	
	// Audio dumping.
	if (audio->dumpingWAV())
		audio->stopWAVDump();
	if (GYM_Dumping)
		Stop_GYM_Dump();

	if (SegaCD_Started)
		Stop_CD();
	
	Net_Play = 0;
	Genesis_Started = 0;
	_32X_Started = 0;
	SegaCD_Started = 0;
	
	// Clear the border color palette entries.
	MD_Palette[0] = 0;
	MD_Palette32[0] = 0;
	
	Game = NULL;
	ice = 0;
	
	if (ROM_MD)
	{
		free(ROM_MD);
		ROM_MD = NULL;
	}
	
	if (Intro_Style == 3)
		Init_Genesis_Bios();
#if !defined(GENS_UI_OPENEMU)
	GensUI::setWindowTitle_Idle();
#endif
}
