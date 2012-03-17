#ifndef GENS_ROM_HPP
#define GENS_ROM_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include "emulator/gens.hpp"
#include <stdlib.h>

typedef enum
{
	MD_ROM			= 2,
	MD_ROM_Interleaved	= 3,
	_32X_ROM		= 4,
	_32X_ROM_Interleaved	= 5,
	SegaCD_Image		= 6,
	SegaCD_Image_BIN	= 7,
	SegaCD_32X_Image	= 8,
	SegaCD_32X_Image_BIN	= 9,
} ROMType;


typedef struct _ROM_t
{
	char Console_Name[17];
	char Copyright[17];
	char ROM_Name[49];
	char ROM_Name_W[49];
	char Type[3];
	char Version[13];
	unsigned int Checksum;
	char IO_Support[17];
	unsigned int ROM_Start_Address;
	unsigned int ROM_End_Address;
	unsigned int R_Size;
	char RAM_Info[13];
	unsigned int RAM_Start_Address;
	unsigned int RAM_End_Address;
	char Modem_Info[13];
	char Description[41];
	char Countries[4];
} ROM_t;


extern char Recent_Rom[9][GENS_PATH_MAX];
extern char IPS_Dir[GENS_PATH_MAX];
extern char Rom_Dir[GENS_PATH_MAX];

extern ROM_t* Game;
extern char ROM_Name[512];

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <string>

// New C++ ROM class.
class ROM
{
	public:
		static std::string getNameFromPath(const std::string& fullPath);
		static std::string getDirFromPath(const std::string& fullPath);
		static void updateCDROMName(const char *cdromName);
		
		static int getROM(void);
		static int openROM(const char *Name);
		static ROM_t* loadSegaCD_BIOS(const char *filename);
		
		static ROMType detectFormat(const unsigned char buf[2048]);
		static ROMType detectFormat_fopen(const char* filename);
		
		static ROMType loadROM(const char* filename, ROM_t** retROM);
		static void fixChecksum(void);
		static int applyIPSPatch(void);
		static void freeROM(ROM_t* ROM_MD);
	
	protected:
		static void updateRecentROMList(const char* filename);
		static void updateROMDir(const char *filename);
		static void updateROMName(const char *filename);
		static void deinterleaveSMD(void);
		static void fillROMInfo(void);
		static unsigned short calcChecksum(void);
};
#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif

// Temporary C wrapper functions.
// TODO: Eliminate these.
ROMType detectFormat(const unsigned char buf[2048]);

#ifdef __cplusplus
}
#endif

#endif /* GENS_ROM_HPP */
