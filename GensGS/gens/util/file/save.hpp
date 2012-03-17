#ifndef GENS_SAVE_HPP
#define GENS_SAVE_HPP

#include "emulator/gens.hpp"
#include <stdio.h>

#define GENESIS_LENGTH_EX1		0x2247C
#define GENESIS_LENGTH_EX2		0x11ED2
#define GENESIS_STATE_LENGTH		(GENESIS_LENGTH_EX1 + GENESIS_LENGTH_EX2)
#define GENESIS_V6_LENGTH_EX2		0x7EC
#define GENESIS_V6_STATE_LENGTH		(GENESIS_LENGTH_EX1 + GENESIS_V6_LENGTH_EX2)
#define SEGACD_LENGTH_EX1		0xE19A4
#define SEGACD_LENGTH_EX2		0x1238B
#define SEGACD_LENGTH_EX		(SEGACD_LENGTH_EX1 + SEGACD_LENGTH_EX2)
#define G32X_LENGTH_EX			0x849BF
#define MAX_STATE_FILE_LENGTH		(GENESIS_STATE_LENGTH + SEGACD_LENGTH_EX + G32X_LENGTH_EX)

/*
// Old GENS v5 savestate offsets.
#define GENESIS_STATE_FILE_LENGTH    0x22478
#define GENESIS_STATE_FILE_LENGTH_EX 0x25550
#define SEGACD_STATE_FILE_LENGTH     (0x22500 + 0xE0000)
#define G32X_STATE_FILE_LENGTH       (0x22500 + 0x82A00)
#define SCD32X_STATE_FILE_LENGTH     (0x22500 + 0xE0000 + 0x82A00)
#define MAX_STATE_FILE_LENGTH        SCD32X_STATE_FILE_LENGTH
*/

#ifdef __cplusplus
extern "C" {
#endif

extern char State_Dir[GENS_PATH_MAX];
extern char SRAM_Dir[GENS_PATH_MAX];
extern char BRAM_Dir[GENS_PATH_MAX];

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

// C++ includes
#include <string>

class Savestate
{
	public:
		static std::string selectFile(const bool save = false, const std::string& dir = "");
		static std::string selectCDImage(const std::string& dir);
		
		static FILE* getStateFile(void);
		static std::string getStateFilename(void);
		
		static int loadState(const std::string& filename);
		static int saveState(const std::string& filename);
		
		static int loadSRAM(void);
		static int saveSRAM(void);
		
		static void formatSegaCD_BackupRAM(void);
		
		static int loadBRAM(void);
		static int saveBRAM(void);
		
	protected:
		// ImportData / ExportData functions from Gens Rerecording
		
		static void importData(void* into, const void* data, 
				       const unsigned int offset,
				       unsigned int numBytes);
		
		static void exportData(const void* from, void* data,
				       const unsigned int offset,
				       unsigned int numBytes);
		
		static void importDataAuto(void* into, const void* data,
					   unsigned int* pOffset,
					   const unsigned int numBytes);
		
		static void exportDataAuto(const void* from, void* data,
					   unsigned int *pOffset,
					   const unsigned int numBytes);
		
		static int gsxImportGenesis(const unsigned char* data);
		static void gsxExportGenesis(unsigned char* data);
		static void gsxImportSegaCD(const unsigned char* data);
		static void gsxExportSegaCD(unsigned char* data);
		static void gsxImport32X(const unsigned char* data);
		static void gsxExport32X(unsigned char* data);
		
		static std::string getSRAMFilename(void);
		
		static void formatSegaCD_BRAM(unsigned char *buf);
		
		static std::string getBRAMFilename(void);
};

//int Change_Dir(char *Dest, char *Dir, char *Titre, char *Filter, char *Ext);

#endif /* __cplusplus */

#endif /* GENS_SAVE_H */
