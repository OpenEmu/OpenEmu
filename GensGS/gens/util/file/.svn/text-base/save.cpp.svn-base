/**
 * GENS: Save file handler.
 */

// Turn this on to enable savestate debugging.
#define GENS_DEBUG_SAVESTATE
#include <assert.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "save.hpp"

#include "emulator/g_main.hpp"

// CPU
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "gens_core/cpu/sh2/sh2.h"
#include "gens_core/cpu/z80/z80.h"

// Memory
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_m68k_cd.h"
#include "gens_core/mem/mem_m68k_32x.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/mem/mem_z80.h"
#include "gens_core/io/io.h"
#include "util/file/rom.hpp"

// VDP
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_32x.h"

// Audio
#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/sound/pwm.h"

// SegaCD
#include "segacd/cd_sys.hpp"
#include "gens_core/gfx/gfx_cd.h"
#include "segacd/lc89510.h"
#include "segacd/cd_file.h"

// UI
#include "ui/gens_ui.hpp"

// Byteswapping
#include "gens_core/misc/byteswap.h"

// Needed for SetCurrentDirectory.
#ifdef GENS_OS_WIN32
#include <windows.h>
#endif /* GENS_OS_WIN32 */

#ifdef GENS_MP3
// Gens Rerecording
// fatal_mp3_error indicates an error occurred while reading an MP3 for a Sega CD game.
extern int fatal_mp3_error;	// cdda_mp3.c

// Various MP3 stuff, needed for Gens Rerecording
extern unsigned int Current_OUT_Pos, Current_OUT_Size;	// cdda_mp3.c
#endif /* GENS_MP3 */

extern char preloaded_tracks [100], played_tracks_linear [101]; // added for synchronous MP3 code

int Current_State = 0;
char State_Dir[GENS_PATH_MAX] = "";
char SRAM_Dir[GENS_PATH_MAX] = "";
char BRAM_Dir[GENS_PATH_MAX] = "";
unsigned char State_Buffer[MAX_STATE_FILE_LENGTH];

// C++ includes
using std::string;

#if !defined(GENS_UI_OPENEMU)
/**
 * selectFile(): Select a savestate to load or save.
 * @param save If false, load; if true, save.
 * @param dir ???
 * @return Filename if a savestate was selected, or "" if no file was selected.
 */
string Savestate::selectFile(const bool save, const string& dir)
{
	string filename;
	
	if (save)
		filename = GensUI::saveFile("Save Savestate", "" /*Rom_Dir*/, SavestateFile);
	else
		filename = GensUI::openFile("Load Savestate", "" /*Rom_Dir*/, SavestateFile);
	
	return filename;
}


/**
 * selectCDImage(): Select a CD image to load. (Used for multi-disc SegaCD games.)
 * @param dir ???
 * @return Filename if a CD image was selected, or "" if no file was selected.
 */
string Savestate::selectCDImage(const string& dir)
{
	return GensUI::openFile("Load SegaCD Disc Image", "" /*Rom_Dir*/, CDImage);
}
#endif

/*
int Change_Dir(char *Dest, char *Dir, char *Titre, char *Filter, char *Ext)
{
	int i;
	gchar* filename=NULL;
	static FileSelData fd;
	GtkWidget* widget;
	GList* extensions=NULL;

	widget = init_file_selector(&fd, Titre);
	extensions = g_list_append(extensions,Ext);
	file_selector_add_type(&fd,"", extensions);
	gtk_widget_show(widget);
	filename = fileselection_get_filename(fd.filesel);	

	if (filename)
		{
		i = strlen(Dest) - 1;
		while ((i > 0) && (Dest[i] != GENS_DIR_SEPARATOR_CHR)) i--;
		if (!i) return 0;
		Dest[++i] = 0;
		g_free(filename);
		return 1;
	}
	return 0;
}
*/


/**
 * getStateFile(): Open the current savestate file.
 * @return File handle for the current savestate file.
 */
FILE* Savestate::getStateFile(void)
{
	return fopen(getStateFilename().c_str(), "rb");
}


/**
 * getStateFilename(): Get the filename of the current savestate.
 * @return Filename of the current savestate.
 */
string Savestate::getStateFilename(void)
{
	return string(State_Dir) + string(ROM_Name) + ".gs" + (char)('0' + Current_State);
}


/**
 * loadState(): Load a savestate.
 * @param filename Filename of the savestate.
 * @return 1 if successful; 0 on error.
 */
int Savestate::loadState(const string& filename)
{
	FILE *f;
	unsigned char *buf;
	int len;
	
	ice = 0;
	
	len = GENESIS_STATE_LENGTH;
	if (Genesis_Started);
	else if (SegaCD_Started)
		len += SEGACD_LENGTH_EX;
	else if (_32X_Started)
		len += G32X_LENGTH_EX;
	else
		return 0;
	
	buf = State_Buffer;
	
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif /* GENS_OS_WIN32 */
	
	if (!(f = fopen(filename.c_str(), "rb")))
		return 0;
	
	memset(buf, 0, len);
	if (fread(buf, 1, len, f))
	{
		//z80_Reset (&M_Z80); // Commented out in Gens Rerecording...
		/*
		main68k_reset();
		YM2612ResetChip(0);
		Reset_VDP();
		*/
		
		// Save functions updated from Gens Rerecording
		buf += gsxImportGenesis(buf);
		if (SegaCD_Started)
		{
			gsxImportSegaCD(buf);
			buf += SEGACD_LENGTH_EX;
		}
		if (_32X_Started)
		{
			gsxImport32X(buf);
			buf += G32X_LENGTH_EX;
		}
		
		/*
		// Commented out in Gens Rerecording...
		Flag_Clr_Scr = 1;
		CRam_Flag = 1;
		VRam_Flag = 1;
		*/
		
		sprintf(Str_Tmp, "STATE %d LOADED", Current_State);
		draw->writeText(Str_Tmp, 2000);
	}
	
	fclose(f);
	
	return 1;
}


/**
 * saveState(): Save a savestate.
 * @param filename Filename of the savestate.
 * @return 1 if successful; 0 on error.
 */
int Savestate::saveState(const string& filename)
{
	FILE *f;
	unsigned char *buf;
	int len;
	
	ice = 0;
	
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif /* GENS_OS_WIN32 */
	
	buf = State_Buffer;
	if ((f = fopen(filename.c_str(), "wb")) == NULL)
		return 0;
	
	len = GENESIS_STATE_LENGTH;
	if (Genesis_Started);
	else if (SegaCD_Started)
		len += SEGACD_LENGTH_EX;
	else if (_32X_Started)
		len += G32X_LENGTH_EX;
	else
		return 0;
	
	if (buf == NULL)
		return 0;
	memset(buf, 0, len);
	
	gsxExportGenesis(buf);
	buf += GENESIS_STATE_LENGTH;
	if (SegaCD_Started)
	{
		gsxExportSegaCD(buf);
		buf += SEGACD_LENGTH_EX;
	}
	if (_32X_Started)
	{
		gsxExport32X(buf);
		buf += G32X_LENGTH_EX;
	}
	
	fwrite(State_Buffer, 1, len, f);
	fclose(f);
	
	sprintf(Str_Tmp, "STATE %d SAVED", Current_State);
	draw->writeText(Str_Tmp, 2000);
	
	return 1;
}


// See doc/genecyst_save_file_format.txt for information
// on the Genecyst save file format.


// Version field is initialized in gsxImportGenesis(),
// but is also used in gsxImportSegaCD() and gsxImport32X().
// TODO: Move this to the Savestate class.
static unsigned char m_Version;


// importData and ExportData functions from Gens Rerecording

//Modif N. - added ImportData and ExportData because the import/export code was getting a little hairy without these
// The main advantage to using these, besides less lines of code, is that
// you can replace ImportData with ExportData, without changing anything else in the arguments,
// to go from import code to export code.

// Changed to importData and exportData to follow C++ naming conventions.

inline void Savestate::importData(void* into, const void* data,
				  const unsigned int offset,
				  unsigned int numBytes)
{
	unsigned char* dst = static_cast<unsigned char*>(into);
	const unsigned char* src = static_cast<const unsigned char*>(data) + offset;
	
	while (numBytes--)
		*dst++ = *src++;
}


inline void Savestate::exportData(const void* from, void* data,
				  const unsigned int offset,
				  unsigned int numBytes)
{
	const unsigned char* src = static_cast<const unsigned char*>(from);
	unsigned char* dst = static_cast<unsigned char*>(data) + offset;
	
	while (numBytes--)
	{
#ifdef GENS_DEBUG_SAVESTATE
		assert((*dst == 0 || *dst == 0xFF) && "error: saved over the same byte twice");
#endif /* GENS_DEBUG_SAVESTATE */
		*dst++ = *src++;
	}
}


// versions that auto-increment the offset
inline void Savestate::importDataAuto(void* into, const void* data,
				      unsigned int* pOffset,
				      const unsigned int numBytes)
{
	importData(into, data, *pOffset, numBytes);
	*pOffset += numBytes;
}


inline void Savestate::exportDataAuto(const void* from, void* data,
				      unsigned int *pOffset,
				      const unsigned int numBytes)
{
	exportData(from, data, *pOffset, numBytes);
	*pOffset += numBytes;
}


/**
 * gsxImportGenesis(): Load Genesis data from a GSX savestate.
 * (Portions ported from Gens Rerecording.)
 * @param Data Savestate data.
 * @return Number of bytes read.
 */
int Savestate::gsxImportGenesis(const unsigned char* data)
{
	unsigned char Reg_1[0x200], *src;
	int i;
	
	// Savestate V6 and V7 code from Gens Rerecording.
	
	/*
	// Commented out in Gens Rerecording.
	VDP_Int = 0;
	DMAT_Length = 0;
	*/
	
	// Length of the savestate.
	int len = GENESIS_STATE_LENGTH;
	
	// Get the savestate version.
	m_Version = data[0x50];
	
	// Savestates earlier than Version 6 are shitty.
	if (m_Version < 6)
		len -= 0x10000;
	
	importData(CRam, data, 0x112, 0x80);
	importData(VSRam, data, 0x192, 0x50);
	importData(Ram_Z80, data, 0x474, 0x2000);
	
	// 68000 RAM
	for (i = 0; i < 0x10000; i += 2)
	{
		Ram_68k[i + 0] = data[i + 0x2478 + 1];
		Ram_68k[i + 1] = data[i + 0x2478 + 0];
	}
	
	// VRAM
	for (i = 0; i < 0x10000; i += 2)
	{
		VRam[i + 0] = data[i + 0x12478 + 1];
		VRam[i + 1] = data[i + 0x12478 + 0];
	}
	
	importData(Reg_1, data, 0x1E4, 0x200);
	YM2612_Restore(Reg_1);
	
	// Version 2, 3, and 4 save files
	if ((m_Version >= 2) && (m_Version < 4))
	{
		importData(&Ctrl, data, 0x30, 7 * 4);
		
		Z80_State &= ~6;
		if (data[0x440] & 1)
			Z80_State |= 2;
		if (data[0x444] & 1)
			Z80_State |= 4;
		
		importData(&Bank_Z80, data, 0x448, 4);
		
		importData(&PSG_Save, data, 0x224B8, 8 * 4);
		PSG_Restore_State();
	}
	else if ((m_Version >= 4) || (m_Version == 0))
	{
		// New savestate version compatible with Kega
		Z80_State &= ~6;
		
		if (m_Version == 4)
		{
			M_Z80.IM = data[0x437];
			M_Z80.IFF.b.IFF1 = (data[0x438] & 1) << 2;
			M_Z80.IFF.b.IFF2 = (data[0x438] & 1) << 2;
			
			Z80_State |= (data[0x439] & 1) << 1;
		}
		else
		{
			M_Z80.IM = 1;
			M_Z80.IFF.b.IFF1 = (data[0x436] & 1) << 2;
			M_Z80.IFF.b.IFF2 = (data[0x436] & 1) << 2;
			
			Z80_State |= ((data[0x439] & 1) ^ 1) << 1;
			Z80_State |= ((data[0x438] & 1) ^ 1) << 2;
		}
		
		src = (unsigned char *) &Ctrl;
		for (i = 0; i < 7 * 4; i++)
			*src++ = 0;
		
		Write_VDP_Ctrl(data[0x40] + (data[0x41] << 8));
		Write_VDP_Ctrl(data[0x42] + (data[0x43] << 8));
		
		Ctrl.Flag = data[0x44];
		Ctrl.DMA = (data[0x45] & 1) << 2;
		Ctrl.Access = data[0x46] + (data[0x47] << 8); //Nitsuja added this
		Ctrl.Address = data[0x48] + (data[0x49] << 8);
		
		importData(&Bank_Z80, data, 0x43C, 4);
		
		if (m_Version >= 4)
		{
			for (i = 0; i < 8; i++)
				PSG_Save[i] = data[i * 2 + 0x60] + (data[i * 2 + 0x61] << 8);
			PSG_Restore_State();
		}
	}

	z80_Set_AF (&M_Z80, data[0x404] + (data[0x405] << 8));
	M_Z80.AF.b.FXY = data[0x406]; //Modif N [Gens Rerecording]
	M_Z80.BC.w.BC = data[0x408] + (data[0x409] << 8);
	M_Z80.DE.w.DE = data[0x40C] + (data[0x40D] << 8);
	M_Z80.HL.w.HL = data[0x410] + (data[0x411] << 8);
	M_Z80.IX.w.IX = data[0x414] + (data[0x415] << 8);
	M_Z80.IY.w.IY = data[0x418] + (data[0x419] << 8);
	z80_Set_PC(&M_Z80, data[0x41C] + (data[0x41D] << 8));
	M_Z80.SP.w.SP = data[0x420] + (data[0x421] << 8);
	z80_Set_AF2(&M_Z80, data[0x424] + (data[0x425] << 8));
	M_Z80.BC2.w.BC2 = data[0x428] + (data[0x429] << 8);
	M_Z80.DE2.w.DE2 = data[0x42C] + (data[0x42D] << 8);
	M_Z80.HL2.w.HL2 = data[0x430] + (data[0x431] << 8);
	M_Z80.I = data[0x434] & 0xFF;
	
	// Gens Rerecording: This seems to only be used for movies (e.g. *.giz), so ignore it for now.
	//FrameCount = data[0x22478] + (data[0x22479] << 8) + (data[0x2247A] << 16) + (data[0x2247B] << 24);
	
	main68k_GetContext(&Context_68K);
	
	// VDP registers
	for (i = 0; i < 24; i++)
		Set_VDP_Reg(i, data[0xFA + i]);
	
	// 68000 registers
	importData(&Context_68K.dreg[0], data, 0x80, 8 * 2 * 4);
	importData(&Context_68K.pc, data, 0xC8, 4);
	importData(&Context_68K.sr, data, 0xD0, 2);
	
	if ((m_Version >= 3) || (m_Version == 0))
	{
		if (data[0xD1] & 0x20)
		{
			// Supervisor
			importData(&Context_68K.asp, data, 0xD2, 2);
		}
		else
		{
			// User
			importData(&Context_68K.asp, data, 0xD6, 2);
		}
	}
	
	// NEW AND IMPROVED! GENS v6 and v7 savestate formats are here!
	// Ported from Gens Rerecording.
	unsigned int offset = GENESIS_LENGTH_EX1;
	if (m_Version == 6)
	{
		// Gens v6 savestate
		//Modif N. - saving more stuff (although a couple of these are saved above in a weird way that I don't trust)
		importDataAuto(&Context_68K.dreg, data, &offset, 4*8);
		importDataAuto(&Context_68K.areg, data, &offset, 4*8);
		importDataAuto(&Context_68K.asp, data, &offset, 4);
		importDataAuto(&Context_68K.pc, data, &offset, 4);
		importDataAuto(&Context_68K.odometer, data, &offset, 4);
		importDataAuto(&Context_68K.interrupts, data, &offset, 8);
		importDataAuto(&Context_68K.sr, data, &offset, 2);
		importDataAuto(&Context_68K.contextfiller00, data, &offset, 2);
		
		importDataAuto(&VDP_Reg.H_Int, data, &offset, 4);
		importDataAuto(&VDP_Reg.Set1, data, &offset, 4);
		importDataAuto(&VDP_Reg.Set2, data, &offset, 4);
		importDataAuto(&VDP_Reg.Pat_ScrA_Adr, data, &offset, 4);
		importDataAuto(&VDP_Reg.Pat_ScrA_Adr, data, &offset, 4);
		importDataAuto(&VDP_Reg.Pat_Win_Adr, data, &offset, 4);
		importDataAuto(&VDP_Reg.Pat_ScrB_Adr, data, &offset, 4);
		importDataAuto(&VDP_Reg.Spr_Att_Adr, data, &offset, 4);
		importDataAuto(&VDP_Reg.Reg6, data, &offset, 4);
		importDataAuto(&VDP_Reg.BG_Color, data, &offset, 4);
		importDataAuto(&VDP_Reg.Reg8, data, &offset, 4);
		importDataAuto(&VDP_Reg.Reg9, data, &offset, 4);
		importDataAuto(&VDP_Reg.H_Int, data, &offset, 4);
		importDataAuto(&VDP_Reg.Set3, data, &offset, 4);
		importDataAuto(&VDP_Reg.Set4, data, &offset, 4);
		importDataAuto(&VDP_Reg.H_Scr_Adr, data, &offset, 4);
		importDataAuto(&VDP_Reg.Reg14, data, &offset, 4);
		importDataAuto(&VDP_Reg.Auto_Inc, data, &offset, 4);
		importDataAuto(&VDP_Reg.Scr_Size, data, &offset, 4);
		importDataAuto(&VDP_Reg.Win_H_Pos, data, &offset, 4);
		importDataAuto(&VDP_Reg.Win_V_Pos, data, &offset, 4);
		importDataAuto(&VDP_Reg.DMA_Length_L, data, &offset, 4);
		importDataAuto(&VDP_Reg.DMA_Length_H, data, &offset, 4);
		importDataAuto(&VDP_Reg.DMA_Src_Adr_L, data, &offset, 4);
		importDataAuto(&VDP_Reg.DMA_Src_Adr_M, data, &offset, 4);
		importDataAuto(&VDP_Reg.DMA_Src_Adr_H, data, &offset, 4);
		importDataAuto(&VDP_Reg.DMA_Length, data, &offset, 4);
		importDataAuto(&VDP_Reg.DMA_Address, data, &offset, 4);
		
		importDataAuto(&Controller_1_Counter, data, &offset, 4);
		importDataAuto(&Controller_1_Delay, data, &offset, 4);
		importDataAuto(&Controller_1_State, data, &offset, 4);
		importDataAuto(&Controller_1_COM, data, &offset, 4);
		importDataAuto(&Controller_2_Counter, data, &offset, 4);
		importDataAuto(&Controller_2_Delay, data, &offset, 4);
		importDataAuto(&Controller_2_State, data, &offset, 4);
		importDataAuto(&Controller_2_COM, data, &offset, 4);
		importDataAuto(&Memory_Control_Status, data, &offset, 4);
		importDataAuto(&Cell_Conv_Tab, data, &offset, 4);
		
		importDataAuto(&Controller_1_Type, data, &offset, 4);
		/* TODO: Make this stuff use bitfields.
		         For now, it's disabled, especially since v6 is rare.
		importDataAuto(&Controller_1_Up, data, &offset, 4);
		importDataAuto(&Controller_1_Down, data, &offset, 4);
		importDataAuto(&Controller_1_Left, data, &offset, 4);
		importDataAuto(&Controller_1_Right, data, &offset, 4);
		importDataAuto(&Controller_1_Start, data, &offset, 4);
		importDataAuto(&Controller_1_Mode, data, &offset, 4);
		importDataAuto(&Controller_1_A, data, &offset, 4);
		importDataAuto(&Controller_1_B, data, &offset, 4);
		importDataAuto(&Controller_1_C, data, &offset, 4);
		importDataAuto(&Controller_1_X, data, &offset, 4);
		importDataAuto(&Controller_1_Y, data, &offset, 4);
		importDataAuto(&Controller_1_Z, data, &offset, 4);
		*/
		offset += 12*4;
		
		importDataAuto(&Controller_2_Type, data, &offset, 4);
		/* TODO: Make this stuff use bitfields.
		         For now, it's disabled, especially since v6 is rare.
		importDataAuto(&Controller_2_Up, data, &offset, 4);
		importDataAuto(&Controller_2_Down, data, &offset, 4);
		importDataAuto(&Controller_2_Left, data, &offset, 4);
		importDataAuto(&Controller_2_Right, data, &offset, 4);
		importDataAuto(&Controller_2_Start, data, &offset, 4);
		importDataAuto(&Controller_2_Mode, data, &offset, 4);
		importDataAuto(&Controller_2_A, data, &offset, 4);
		importDataAuto(&Controller_2_B, data, &offset, 4);
		importDataAuto(&Controller_2_C, data, &offset, 4);
		importDataAuto(&Controller_2_X, data, &offset, 4);
		importDataAuto(&Controller_2_Y, data, &offset, 4);
		importDataAuto(&Controller_2_Z, data, &offset, 4);
		*/
		offset += 12*4;
		
		importDataAuto(&DMAT_Length, data, &offset, 4);
		importDataAuto(&DMAT_Type, data, &offset, 4);
		importDataAuto(&DMAT_Tmp, data, &offset, 4);
		importDataAuto(&VDP_Current_Line, data, &offset, 4);
		importDataAuto(&VDP_Num_Vis_Lines, data, &offset, 4);
		importDataAuto(&VDP_Num_Vis_Lines, data, &offset, 4);
		importDataAuto(&Bank_M68K, data, &offset, 4);
		importDataAuto(&S68K_State, data, &offset, 4);
		importDataAuto(&Z80_State, data, &offset, 4);
		importDataAuto(&Last_BUS_REQ_Cnt, data, &offset, 4);
		importDataAuto(&Last_BUS_REQ_St, data, &offset, 4);
		importDataAuto(&Fake_Fetch, data, &offset, 4);
		importDataAuto(&Game_Mode, data, &offset, 4);
		importDataAuto(&CPU_Mode, data, &offset, 4);
		importDataAuto(&CPL_M68K, data, &offset, 4);
		importDataAuto(&CPL_S68K, data, &offset, 4);
		importDataAuto(&CPL_Z80, data, &offset, 4);
		importDataAuto(&Cycles_S68K, data, &offset, 4);
		importDataAuto(&Cycles_M68K, data, &offset, 4);
		importDataAuto(&Cycles_Z80, data, &offset, 4);
		importDataAuto(&VDP_Status, data, &offset, 4);
		importDataAuto(&VDP_Int, data, &offset, 4);
		importDataAuto(&Ctrl.Write, data, &offset, 4);
		importDataAuto(&Ctrl.DMA_Mode, data, &offset, 4);
		importDataAuto(&Ctrl.DMA, data, &offset, 4);
		//importDataAuto(&CRam_Flag, data, &offset, 4); //Causes screen to blank
		//offset+=4;
		
		// TODO: LagCount from Gens Rerecording.
		//LagCount = importNumber_le32(data, &offset);
		offset += 4;
		
		importDataAuto(&VRam_Flag, data, &offset, 4);
		importDataAuto(&VDP_Reg.DMA_Length, data, &offset, 4);
		importDataAuto(&VDP_Reg.Auto_Inc, data, &offset, 4);
		importDataAuto(&VDP_Reg.DMA_Length, data, &offset, 4);
		//importDataAuto(VRam, data, &offset, 65536);
		importDataAuto(CRam, data, &offset, 512);
		//importDataAuto(VSRam, data, &offset, 64);
		importDataAuto(H_Counter_Table, data, &offset, 512 * 2);
		//importDataAuto(Spr_Link, data, &offset, 4*256);
		//extern int DMAT_Tmp, VSRam_Over;
		//importDataAuto(&DMAT_Tmp, data, &offset, 4);
		//importDataAuto(&VSRam_Over, data, &offset, 4);
		importDataAuto(&VDP_Reg.DMA_Length_L, data, &offset, 4);
		importDataAuto(&VDP_Reg.DMA_Length_H, data, &offset, 4);
		importDataAuto(&VDP_Reg.DMA_Src_Adr_L, data, &offset, 4);
		importDataAuto(&VDP_Reg.DMA_Src_Adr_M, data, &offset, 4);
		importDataAuto(&VDP_Reg.DMA_Src_Adr_H, data, &offset, 4);
		importDataAuto(&VDP_Reg.DMA_Length, data, &offset, 4);
		importDataAuto(&VDP_Reg.DMA_Address, data, &offset, 4);
		
#ifdef GENS_DEBUG_SAVESTATE
		assert(offset == GENESIS_V6_STATE_LENGTH);
#endif
	}
	else if (m_Version >= 7)
	{
		// Gens v7 savestate
		unsigned char Reg_2[sizeof(ym2612_)];
		importDataAuto(Reg_2, data, &offset, sizeof(ym2612_)); // some important parts of this weren't saved above
		YM2612_Restore_Full(Reg_2);
		
		importDataAuto(PSG_Save_Full, data, &offset, sizeof(struct _psg)); // some important parts of this weren't saved above
		PSG_Restore_State_Full();
		
		// BUG: The Gens v7 savestate stores M_Z80.BasePC, which is a *real* pointer.
		// Also, it stores M_Z80.PC, which is *relative* to M_Z80.BasePC.
		// Workaround: Save M_Z80.BasePC and M_Z80.PC, and restore them after.
		// The PC is set correctly by the older savestate code above via z80_Set_PC().
		unsigned int oldBasePC = M_Z80.BasePC;
		unsigned int oldPC = M_Z80.PC.d;
		importDataAuto(&M_Z80, data, &offset, 0x5C); // some important parts of this weren't saved above
		M_Z80.PC.d = oldPC;
		M_Z80.BasePC = oldBasePC;
		
		importDataAuto(&M_Z80.RetIC, data, &offset, 4); // not sure about the last two variables, might as well save them too
		importDataAuto(&M_Z80.IntAckC, data, &offset, 4);
		
		importDataAuto(&Context_68K.dreg[0], data, &offset, 86); // some important parts of this weren't saved above
		
		importDataAuto(&Controller_1_State, data, &offset, 448); // apparently necessary (note: 448 == (((char*)&Controller_2D_Z)+sizeof(Controller_2D_Z) - (char*)&Controller_1_State))
		
		// apparently necessary
		importDataAuto(&VDP_Status, data, &offset, 4);
		importDataAuto(&VDP_Int, data, &offset, 4);
		importDataAuto(&VDP_Current_Line, data, &offset, 4);
		importDataAuto(&VDP_Num_Lines, data, &offset, 4);
		importDataAuto(&VDP_Num_Vis_Lines, data, &offset, 4);
		importDataAuto(&DMAT_Length, data, &offset, 4);
		importDataAuto(&DMAT_Type, data, &offset, 4);
		//importDataAuto(&CRam_Flag. data. &offset, 4); //emulator flag which causes Gens not to update its draw palette, but doesn't affect sync state
		
		// TODO: LagCount for Gens Rerecording.
		//importDataAuto(&LagCount, data, &offset, 4);
		offset += 4;
		
		importDataAuto(&VRam_Flag, data, &offset, 4);
		importDataAuto(&CRam, data, &offset, 256 * 2);
		
		// it's probably safer sync-wise to keep SRAM stuff in the savestate
		importDataAuto(&SRAM, data, &offset, sizeof(SRAM));
		importDataAuto(&SRAM_Start, data, &offset, 4);
		importDataAuto(&SRAM_End, data, &offset, 4);
		importDataAuto(&SRAM_ON, data, &offset, 4);
		importDataAuto(&SRAM_Write, data, &offset, 4);
		importDataAuto(&SRAM_Custom, data, &offset, 4);
		
		// this group I'm not sure about, they don't seem to be necessary but I'm keeping them around just in case
		importDataAuto(&Bank_M68K, data, &offset, 4);
		importDataAuto(&S68K_State, data, &offset, 4);
		importDataAuto(&Z80_State, data, &offset, 4);
		importDataAuto(&Last_BUS_REQ_Cnt, data, &offset, 4);
		importDataAuto(&Last_BUS_REQ_St, data, &offset, 4);
		importDataAuto(&Fake_Fetch, data, &offset, 4);
		importDataAuto(&Game_Mode, data, &offset, 4);
		importDataAuto(&CPU_Mode, data, &offset, 4);
		importDataAuto(&CPL_M68K, data, &offset, 4);
		importDataAuto(&CPL_S68K, data, &offset, 4);
		importDataAuto(&CPL_Z80, data, &offset, 4);
		importDataAuto(&Cycles_S68K, data, &offset, 4);
		importDataAuto(&Cycles_M68K, data, &offset, 4);
		importDataAuto(&Cycles_Z80, data, &offset, 4);
		importDataAuto(&Gen_Mode, data, &offset, 4);
		importDataAuto(&Gen_Version, data, &offset, 4);
		importDataAuto(H_Counter_Table, data, &offset, 512 * 2);
		importDataAuto(&VDP_Reg, data, &offset, sizeof(VDP_Reg));
		importDataAuto(&Ctrl, data, &offset, sizeof(Ctrl));
		
		importDataAuto(&Context_68K.cycles_needed, data, &offset, 44);
		
#ifdef GENS_DEBUG_SAVESTATE
		assert(offset == GENESIS_STATE_LENGTH);
#endif		
	}
	
	main68k_SetContext(&Context_68K);
	return len;
}


/**
 * gsxExportGenesis(): Save Genesis data to a savestate.
 * (Portions ported from Gens Rerecording.)
 * @param data Savestate data buffer.
 */
void Savestate::gsxExportGenesis(unsigned char* data)
{
	// This savestate function uses the GENS v7 savestate format.
	// Note about GENS v7 savestate format:
	// - Plain MD savestates will work in older versions of GENS.
	// - Sega CD savestates won't work in older versions, but then again, older versions didn't properly support it.
	// - 32X savestates will *not* work in older versions of GENS. :(
	
	// from Gens Rerecording
	struct S68000CONTEXT Context_68K; // Modif N.: apparently no longer necessary but I'm leaving it here just to be safe: purposely shadows the global Context_68K variable with this local copy to avoid tampering with it while saving
	
	unsigned char Reg_1[0x200], *src;
	int i;
	
	// Be sure to finish DMA before save
	// [from Gens Rerecording] commented out; this may cause the saving to change the current state
	/*
	while (DMAT_Length)
		Update_DMA();
	*/
	
	// Genecyst savestate header
	data[0x00] = 'G';
	data[0x01] = 'S';
	data[0x02] = 'T';
	data[0x03] = 0x40;
	data[0x04] = 0xE0;
	
	data[0x50] = 7;		// Version
	data[0x51] = 0;		// Gens
	
	// Save the PSG state.
	PSG_Save_State ();
	
	// Copy the PSG state into the savestate buffer.
	for (i = 0; i < 8; i++)
	{
		data[0x60 + i * 2] = PSG_Save[i] & 0xFF;
		data[0x61 + i * 2] = (PSG_Save[i] >> 8) & 0xFF;
	}
	
#ifdef GENS_DEBUG_SAVESTATE
	int contextsize1 = main68k_GetContextSize();
	int contextsize2 = sizeof(Context_68K);
	assert(contextsize1 == contextsize2);
#endif
	
	main68k_GetContext(&Context_68K);
	
	// 68000 registers
	exportData(&Context_68K.dreg[0], data, 0x80, 8 * 2 * 4);
	exportData(&Context_68K.pc, data, 0xC8, 4);
	exportData(&Context_68K.sr, data, 0xD0, 2);
	
	if (Context_68K.sr & 0x2000)
	{
		// Supervisor
		exportData(&Context_68K.asp, data, 0xD2, 4);
		exportData(&Context_68K.areg[7], data, 0xD6, 4);
	}
	else
	{
		// User
		exportData(&Context_68K.asp, data, 0xD6, 4);
		exportData(&Context_68K.areg[7], data, 0xD2, 4);
	}
	
	exportData(&Ctrl.Data, data, 0x40, 4);
	
	data[0x44] = Ctrl.Flag;
	data[0x45] = (Ctrl.DMA >> 2) & 1;
	
	// Gens Rerecording
	data[0x46] = Ctrl.Access & 0xFF; //Nitsuja added this
	data[0x47] = (Ctrl.Access >> 8) & 0xFF; //Nitsuja added this
	
	data[0x48] = Ctrl.Address & 0xFF;
	data[0x49] = (Ctrl.Address >> 8) & 0xFF;
	
	VDP_Reg.DMA_Length_L = VDP_Reg.DMA_Length & 0xFF;
	VDP_Reg.DMA_Length_H = (VDP_Reg.DMA_Length >> 8) & 0xFF;
	
	VDP_Reg.DMA_Src_Adr_L = VDP_Reg.DMA_Address & 0xFF;
	VDP_Reg.DMA_Src_Adr_M = (VDP_Reg.DMA_Address >> 8) & 0xFF;
	VDP_Reg.DMA_Src_Adr_H = (VDP_Reg.DMA_Address >> 16) & 0xFF;
	
	VDP_Reg.DMA_Src_Adr_H |= Ctrl.DMA_Mode & 0xC0;
	
	src = (unsigned char *) &(VDP_Reg.Set1);
	for (i = 0; i < 24; i++)
	{
		data[0xFA + i] = *src;
		src += 4;
	}
	
	for (i = 0; i < 0x80; i++)
		data[i + 0x112] = (CRam[i] & 0xFF);
	for (i = 0; i < 0x50; i++)
		data[i + 0x192] = VSRam[i];
	
	YM2612_Save(Reg_1);
	for (i = 0; i < 0x200; i++)
		data[i + 0x1E4] = Reg_1[i];
	
	data[0x404] = (unsigned char) (z80_Get_AF (&M_Z80) & 0xFF);
	data[0x405] = (unsigned char) (z80_Get_AF (&M_Z80) >> 8);
	data[0x406] = (unsigned char) (M_Z80.AF.b.FXY & 0xFF); //Modif N
	data[0x407] = (unsigned char) 0; //Modif N
	data[0x408] = (unsigned char) (M_Z80.BC.w.BC & 0xFF);
	data[0x409] = (unsigned char) (M_Z80.BC.w.BC >> 8);
	data[0x40C] = (unsigned char) (M_Z80.DE.w.DE & 0xFF);
	data[0x40D] = (unsigned char) (M_Z80.DE.w.DE >> 8);
	data[0x410] = (unsigned char) (M_Z80.HL.w.HL & 0xFF);
	data[0x411] = (unsigned char) (M_Z80.HL.w.HL >> 8);
	data[0x414] = (unsigned char) (M_Z80.IX.w.IX & 0xFF);
	data[0x415] = (unsigned char) (M_Z80.IX.w.IX >> 8);
	data[0x418] = (unsigned char) (M_Z80.IY.w.IY & 0xFF);
	data[0x419] = (unsigned char) (M_Z80.IY.w.IY >> 8);
	data[0x41C] = (unsigned char) (z80_Get_PC (&M_Z80) & 0xFF);
	data[0x41D] = (unsigned char) ((z80_Get_PC (&M_Z80) >> 8) & 0xFF);
	data[0x420] = (unsigned char) (M_Z80.SP.w.SP & 0xFF);
	data[0x421] = (unsigned char) (M_Z80.SP.w.SP >> 8);
	data[0x424] = (unsigned char) (z80_Get_AF2 (&M_Z80) & 0xFF);
	data[0x425] = (unsigned char) (z80_Get_AF2 (&M_Z80) >> 8);
	data[0x428] = (unsigned char) (M_Z80.BC2.w.BC2 & 0xFF);
	data[0x429] = (unsigned char) (M_Z80.BC2.w.BC2 >> 8);
	data[0x42C] = (unsigned char) (M_Z80.DE2.w.DE2 & 0xFF);
	data[0x42D] = (unsigned char) (M_Z80.DE2.w.DE2 >> 8);
	data[0x430] = (unsigned char) (M_Z80.HL2.w.HL2 & 0xFF);
	data[0x431] = (unsigned char) (M_Z80.HL2.w.HL2 >> 8);
	data[0x434] = (unsigned char) (M_Z80.I);
	data[0x436] = (unsigned char) (M_Z80.IFF.b.IFF1 >> 2);
	
	data[0x438] = (unsigned char) (((Z80_State & 4) >> 2) ^ 1);
	data[0x439] = (unsigned char) (((Z80_State & 2) >> 1) ^ 1);
	
	exportData(&Bank_Z80, data, 0x43C, 4);
	
	for (i = 0; i < 0x2000; i++)
		data[i + 0x474] = Ram_Z80[i];
	
	// 68000 RAM
	for (i = 0; i < 0x10000; i += 2)
	{
		data[i + 0x2478 + 1] = Ram_68k[i + 0];
		data[i + 0x2478 + 0] = Ram_68k[i + 1];
	}
	
	// VRAM
	for (i = 0; i < 0x10000; i += 2)
	{
		data[i + 0x12478 + 1] = VRam[i + 0];
		data[i + 0x12478 + 0] = VRam[i + 1];
	}
	
	// TODO: This is from Gens Rerecording, and is probably not any use right now.
	/*
	data[0x22478]=unsigned char (FrameCount&0xFF);   //Modif
	data[0x22479]=unsigned char ((FrameCount>>8)&0xFF);   //Modif
	data[0x2247A]=unsigned char ((FrameCount>>16)&0xFF);   //Modif
	data[0x2247B]=unsigned char ((FrameCount>>24)&0xFF);   //Modif
	*/
	
	// everything after this should use this offset variable for ease of extensibility
	unsigned int offset = GENESIS_LENGTH_EX1; // Modif U. - got rid of about 12 KB of 00 bytes.
	
	// GENS v7 Savestate Additions
	// version 7 additions (version 6 additions deleted)
	//Modif N. - saving more stuff (added everything after this)
	
	unsigned char Reg_2[sizeof(ym2612_)];
	YM2612_Save_Full(Reg_2);
	exportDataAuto(Reg_2, data, &offset, sizeof(ym2612_)); // some important parts of this weren't saved above
	
	PSG_Save_State_Full();
	exportDataAuto(PSG_Save_Full, data, &offset, sizeof(struct _psg));  // some important parts of this weren't saved above
	
	exportDataAuto(&M_Z80, data, &offset, 0x5C); // some important parts of this weren't saved above
	
	// BUG: The above exportDataAuto call saves the PC and BasePC registers,
	// which are based on x86 memory locations and not emulated memory.
	// Set them to 0xDEADBEEF in the savestate, in big-endian format
	// so it's readable by a hex editor.
	
	// PC
	data[offset - 0x5C + 0x18] = 0xDE;
	data[offset - 0x5C + 0x19] = 0xAD;
	data[offset - 0x5C + 0x1A] = 0xBE;
	data[offset - 0x5C + 0x1B] = 0xEF;
	
	// BasePC
	data[offset - 0x5C + 0x40] = 0xDE;
	data[offset - 0x5C + 0x41] = 0xAD;
	data[offset - 0x5C + 0x42] = 0xBE;
	data[offset - 0x5C + 0x43] = 0xEF;
	
	exportDataAuto(&M_Z80.RetIC, data, &offset, 4); // not sure about the last two variables, might as well save them too
	exportDataAuto(&M_Z80.IntAckC, data, &offset, 4);
	
	exportDataAuto(&Context_68K.dreg[0], data, &offset, 86); // some important parts of this weren't saved above
	
	exportDataAuto(&Controller_1_State, data, &offset, 448);   // apparently necessary (note: 448 == (((char*)&Controller_2D_Z)+sizeof(Controller_2D_Z) - (char*)&Controller_1_State))
	
	// apparently necessary
	exportDataAuto(&VDP_Status, data, &offset, 4);
	exportDataAuto(&VDP_Int, data, &offset, 4);
	exportDataAuto(&VDP_Current_Line, data, &offset, 4);
	exportDataAuto(&VDP_Num_Lines, data, &offset, 4);
	exportDataAuto(&VDP_Num_Vis_Lines, data, &offset, 4);
	exportDataAuto(&DMAT_Length, data, &offset, 4);
	exportDataAuto(&DMAT_Type, data, &offset, 4);
	//exportDataAuto(&CRam_Flag, data, &offset, 4);
	// TODO: LagCount for GENS ReRecording.
	//exportDataAuto(&LagCount, data, &offset, 4);
	offset += 4;
	exportDataAuto(&VRam_Flag, data, &offset, 4);
	exportDataAuto(&CRam, data, &offset, 256 * 2);
	
	// it's probably safer sync-wise to keep SRAM stuff in the savestate
	exportDataAuto(&SRAM, data, &offset, sizeof(SRAM));
	exportDataAuto(&SRAM_Start, data, &offset, 4);
	exportDataAuto(&SRAM_End, data, &offset, 4);
	exportDataAuto(&SRAM_ON, data, &offset, 4);
	exportDataAuto(&SRAM_Write, data, &offset, 4);
	exportDataAuto(&SRAM_Custom, data, &offset, 4);
	
	// this group I'm not sure about, they don't seem to be necessary but I'm keeping them around just in case
	exportDataAuto(&Bank_M68K, data, &offset, 4);
	exportDataAuto(&S68K_State, data, &offset, 4);
	exportDataAuto(&Z80_State, data, &offset, 4);
	exportDataAuto(&Last_BUS_REQ_Cnt, data, &offset, 4);
	exportDataAuto(&Last_BUS_REQ_St, data, &offset, 4);
	exportDataAuto(&Fake_Fetch, data, &offset, 4);
	exportDataAuto(&Game_Mode, data, &offset, 4);
	exportDataAuto(&CPU_Mode, data, &offset, 4);
	exportDataAuto(&CPL_M68K, data, &offset, 4);
	exportDataAuto(&CPL_S68K, data, &offset, 4);
	exportDataAuto(&CPL_Z80, data, &offset, 4);
	exportDataAuto(&Cycles_S68K, data, &offset, 4);
	exportDataAuto(&Cycles_M68K, data, &offset, 4);
	exportDataAuto(&Cycles_Z80, data, &offset, 4);
	exportDataAuto(&Gen_Mode, data, &offset, 4);
	exportDataAuto(&Gen_Version, data, &offset, 4);
	exportDataAuto(H_Counter_Table, data, &offset, 512 * 2);
	exportDataAuto(&VDP_Reg, data, &offset, sizeof(VDP_Reg));
	exportDataAuto(&Ctrl, data, &offset, sizeof(Ctrl));
	
	exportDataAuto(&Context_68K.cycles_needed, data, &offset, 44);
	
	#ifdef GENS_DEBUG_SAVESTATE
		// assert that the final offset value equals our savestate size, otherwise we screwed up
		// if it fails, that probably means you have to add ((int)offset-(int)desiredoffset) to the last GENESIS_LENGTH_EX define
		assert(offset == GENESIS_STATE_LENGTH);
	#endif	
}


/**
 * gsxImportSegaCD(): Load Sega CD data from a savestate.
 * (Ported from Gens Rerecording.)
 * @param data Savestate data.
 */
void Savestate::gsxImportSegaCD(const unsigned char* data)
{
	struct S68000CONTEXT Context_sub68K;
	unsigned char *src;
	int i, j;
	
	sub68k_GetContext(&Context_sub68K);
	
	//sub68K bit goes here
	importData(&(Context_sub68K.dreg[0]), data, 0x0, 8 * 4);
	importData(&(Context_sub68K.areg[0]), data, 0x20, 8 * 4);
	importData(&(Context_sub68K.pc), data, 0x48, 4);
	importData(&(Context_sub68K.sr), data, 0x50, 2);
	
	if(data[0x51] & 0x20)
	{
		// Supervisor
		importData(&Context_68K.asp, data, 0x52, 4);
	}
	else
	{
		// User
		importData(&Context_68K.asp, data, 0x56, 4);
	}
	
	
	importData(&Context_sub68K.odometer, data, 0x5A, 4);
	importData(Context_sub68K.interrupts, data, 0x60, 8);
	importData(&Ram_Word_State, data, 0x6C, 4);
	
	//here ends sub68k bit
	
	//sub68k_SetContext(&Context_sub68K); // Modif N. -- moved to later
	
	//PCM Chip Load
	importData(&PCM_Chip.Rate, data, 0x100, 4);
	importData(&PCM_Chip.Enable, data, 0x104, 4);
	importData(&PCM_Chip.Cur_Chan, data, 0x108, 4);
	importData(&PCM_Chip.Bank, data, 0x10C, 4);
	
	for (j = 0; j < 8; j++)
	{
		importData(&PCM_Chip.Channel[j].ENV, data, 0x120 + (j * 4 * 11), 4);
		importData(&PCM_Chip.Channel[j].PAN, data, 0x124 + (j * 4 * 11), 4);
		importData(&PCM_Chip.Channel[j].MUL_L, data, 0x128 + (j * 4 * 11), 4);
		importData(&PCM_Chip.Channel[j].MUL_R, data, 0x12C + (j * 4 * 11), 4);
		importData(&PCM_Chip.Channel[j].St_Addr, data, 0x130 + (j * 4 * 11), 4);
		importData(&PCM_Chip.Channel[j].Loop_Addr, data, 0x134 + (j * 4 * 11), 4);
		importData(&PCM_Chip.Channel[j].Addr, data, 0x138 + (j * 4 * 11), 4);
		importData(&PCM_Chip.Channel[j].Step, data, 0x13C + (j * 4 * 11), 4);
		importData(&PCM_Chip.Channel[j].Step_B, data, 0x140 + (j * 4 * 11), 4);
		importData(&PCM_Chip.Channel[j].Enable, data, 0x144 + (j * 4 * 11), 4);
		importData(&PCM_Chip.Channel[j].Data, data, 0x148 + (j * 4 * 11), 4);
	}
	
	//End PCM Chip Load
	
	//Init_RS_GFX(); //purge old GFX data
	//GFX State Load
	importData(&(Rot_Comp.Stamp_Size), data, 0x300, 4);
	importData(&(Rot_Comp.Stamp_Map_Adr), data, 0x304, 4);
	importData(&(Rot_Comp.IB_V_Cell_Size), data, 0x308, 4);
	importData(&(Rot_Comp.IB_Adr), data, 0x30C, 4);
	importData(&(Rot_Comp.IB_Offset), data, 0x310, 4);
	importData(&(Rot_Comp.IB_H_Dot_Size), data, 0x314, 4);
	importData(&(Rot_Comp.IB_V_Dot_Size), data, 0x318, 4);
	importData(&(Rot_Comp.Vector_Adr), data, 0x31C, 4);
	importData(&(Rot_Comp.Rotation_Running), data, 0x320, 4);
	//End GFX State Load
	
	//gate array bit
	importData(&COMM.Flag, data, 0x0500, 4);
	src = (unsigned char *) &COMM.Command;
	for (j = 0; j < 8 * 2; j += 2)
	{
		for (i = 0; i < 2; i++)
		{
			*src++ = data[i + 0x0504 + j];
		}
	}
	src = (unsigned char *) &COMM.Status;
	for (j = 0; j < 8 * 2; j += 2)
	{
		for (i = 0; i < 2; i++)
		{
			*src++ = data[i + 0x0514 + j];
		}
	}
	importData(&Memory_Control_Status, data, 0x0524, 4);
	importData(&Init_Timer_INT3, data, 0x0528, 4);
	importData(&Timer_INT3, data, 0x052C, 4);
	importData(&Timer_Step, data, 0x0530, 4);
	importData(&Int_Mask_S68K, data, 0x0534, 4);
	importData(&Font_COLOR, data, 0x0538, 4);
	importData(&Font_BITS, data, 0x053C, 4);
	importData(&CD_Access_Timer, data, 0x0540, 4);
	importData(&SCD.Status_CDC, data, 0x0544, 4);
	importData(&SCD.Status_CDD, data, 0x0548, 4);
	importData(&SCD.Cur_LBA, data, 0x054C, 4);
	importData(&SCD.Cur_Track, data, 0x0550, 4);
	importData(&S68K_Mem_WP, data, 0x0554, 4);
	importData(&S68K_Mem_PM, data, 0x0558, 4);
	// More goes here when found
	//here ends gate array bit
	
	//Misc Status Flags
	importData(&Ram_Word_State, data, 0xF00, 4); //For determining 1M or 2M
	importData(&LED_Status, data, 0xF08, 4); //So the LED shows up properly
	//Word RAM state
	
	//Prg RAM
	importData(Ram_Prg, data, 0x1000, 0x80000);
	
	//Word RAM
	if (Ram_Word_State >= 2)
		importData(Ram_Word_1M, data, 0x81000, 0x40000); //1M mode
	else
		importData(Ram_Word_2M, data, 0x81000, 0x40000); //2M mode
		//importData(Ram_Word_2M, data, 0x81000, 0x40000); //2M mode
	//Word RAM end
	
	importData(Ram_PCM, data, 0xC1000, 0x10000); //PCM RAM
	
	//CDD & CDC Data
	//CDD
	unsigned int CDD_data[8]; //makes an array for reading CDD unsigned int Data into
	for (j = 0; j < 8; j++)
	{
		importData(&CDD_data[j], data, 0xD1000  + (4 * j), 4);
	}
	for(i = 0; i < 10; i++)
		CDD.Rcv_Status[i] = data[0xD1020 + i];
	for(i = 0; i < 10; i++)
		CDD.Trans_Comm[i] = data[0xD102A + i];
	CDD.Fader = CDD_data[0];
	CDD.Control = CDD_data[1];
	CDD.Cur_Comm = CDD_data[2];
	CDD.Status = CDD_data[3];
	CDD.Minute = CDD_data[4];
	CDD.Seconde = CDD_data[5];
	CDD.Frame = CDD_data[6];
	CDD.Ext = CDD_data[7];
	if (CDD.Status & PLAYING)
		if (IsAsyncAllowed()) // Modif N. -- disabled call to resume in synchronous mode (it's unnecessary there and can cause desyncs)
			FILE_Play_CD_LBA(0); // and replaced Resume_CDD_c7 with a call to preload the (new) current MP3 when a savestate is loaded (mainly for sound quality and camhack stability reasons), or do nothing if it's not an MP3
	//CDD end
	
	//CDC
	importData(&CDC.RS0, data, 0xD1034, 4);
	importData(&CDC.RS1, data, 0xD1038, 4);
	importData(&CDC.Host_Data, data, 0xD103C, 4);
	importData(&CDC.DMA_Adr, data, 0xD1040, 4);
	importData(&CDC.Stop_Watch, data, 0xD1044, 4);
	importData(&CDC.COMIN, data, 0xD1048, 4);
	importData(&CDC.IFSTAT, data, 0xD104C, 4);
	importData(&CDC.DBC.N, data, 0xD1050, 4);
	importData(&CDC.DAC.N, data, 0xD1054, 4);
	importData(&CDC.HEAD.N, data, 0xD1058, 4);
	importData(&CDC.PT.N, data, 0xD105C, 4);
	importData(&CDC.WA.N, data, 0xD1060, 4);
	importData(&CDC.STAT.N, data, 0xD1064, 4);
	importData(&CDC.SBOUT, data, 0xD1068, 4);
	importData(&CDC.IFCTRL, data, 0xD106C, 4);
	importData(&CDC.CTRL.N, data, 0xD1070, 4);
	importData(CDC.Buffer, data, 0xD1074, ((32 * 1024 * 2) + 2352)); //Modif N. - added the *2 because the buffer appears to be that large
	//CDC end
	//CDD & CDC Data end
	
	if (m_Version >= 7)
	{
		//Modif N. - extra stuff added to save/set for synchronization reasons
		// I'm not sure how much of this really needs to be saved, should check it sometime
		unsigned int offset = SEGACD_LENGTH_EX1;
		
		importDataAuto(&File_Add_Delay, data, &offset, 4);
//		importDataAuto(CD_Audio_Buffer_L, data, &offset, 4*8192); // removed, seems to be unnecessary
//		importDataAuto(CD_Audio_Buffer_R, data, &offset, 4*8192); // removed, seems to be unnecessary
		importDataAuto(&CD_Audio_Buffer_Read_Pos, data, &offset, 4);
		importDataAuto(&CD_Audio_Buffer_Write_Pos, data, &offset, 4);
		importDataAuto(&CD_Audio_Starting, data, &offset, 4);
		importDataAuto(&CD_Present, data, &offset, 4);
		importDataAuto(&CD_Load_System, data, &offset, 4);
		importDataAuto(&CD_Timer_Counter, data, &offset, 4);
		importDataAuto(&CDD_Complete, data, &offset, 4);
		importDataAuto(&track_number, data, &offset, 4);
		importDataAuto(&CD_timer_st, data, &offset, 4);
		importDataAuto(&CD_LBA_st, data, &offset, 4);
		importDataAuto(&CDC_Decode_Reg_Read, data, &offset, 4);
		
		importDataAuto(&SCD, data, &offset, sizeof(SCD));
		//importDataAuto(&CDC, data, &offset, sizeof(CDC)); // removed, seems unnecessary/redundant
		importDataAuto(&CDD, data, &offset, sizeof(CDD));
		importDataAuto(&COMM, data, &offset, sizeof(COMM));
		
		importDataAuto(Ram_Backup, data, &offset, sizeof(Ram_Backup));
		importDataAuto(Ram_Backup_Ex, data, &offset, sizeof(Ram_Backup_Ex));
		
		importDataAuto(&Rot_Comp, data, &offset, sizeof(Rot_Comp));
		importDataAuto(&Stamp_Map_Adr, data, &offset, 4);
		importDataAuto(&Buffer_Adr, data, &offset, 4);
		importDataAuto(&Vector_Adr, data, &offset, 4);
		importDataAuto(&Jmp_Adr, data, &offset, 4);
		importDataAuto(&Float_Part, data, &offset, 4);
		importDataAuto(&Draw_Speed, data, &offset, 4);
		importDataAuto(&XS, data, &offset, 4);
		importDataAuto(&YS, data, &offset, 4);
		importDataAuto(&DXS, data, &offset, 4);
		importDataAuto(&DYS, data, &offset, 4);
		importDataAuto(&XD, data, &offset, 4);
		importDataAuto(&YD, data, &offset, 4);
		importDataAuto(&XD_Mul, data, &offset, 4);
		importDataAuto(&H_Dot, data, &offset, 4);
		
		importDataAuto(&Context_sub68K.cycles_needed, data, &offset, 44);
		importDataAuto(&Rom_Data[0x72], data, &offset, 2); 	//Sega CD games can overwrite the low two bytes of the Horizontal Interrupt vector
		
#ifdef GENS_MP3
		importDataAuto(&fatal_mp3_error, data, &offset, 4);
		importDataAuto(&Current_OUT_Pos, data, &offset, 4);
		importDataAuto(&Current_OUT_Size, data, &offset, 4);
#endif /* GENS_MP3 */
		importDataAuto(&Track_Played, data, &offset, 1);
		importDataAuto(played_tracks_linear, data, &offset, 100);
		//importDataAuto(&Current_IN_Pos, data, &offset, 4)? // don't save this; bad things happen
		
#ifdef GENS_DEBUG_SAVESTATE
		assert(offset == SEGACD_LENGTH_EX);
#endif
	}
	
	sub68k_SetContext(&Context_sub68K); // Modif N. -- moved here from earlier in the function
	
	M68K_Set_Prg_Ram();
	MS68K_Set_Word_Ram();	
}


/**
 * gsxExportSegaCD(): Save Sega CD data to a savestate.
 * (Uses Gens v7 format from Gens Rerecording.)
 * @param data Savestate data buffer.
 */
void Savestate::gsxExportSegaCD(unsigned char* data)
{
	struct S68000CONTEXT Context_sub68K;
	unsigned char *src;
	int i,j;
	
	sub68k_GetContext(&Context_sub68K);
	
	//sub68K bit goes here
	exportData(&Context_sub68K.dreg[0], data, 0x0, 8 * 4);
	exportData(&Context_sub68K.areg[0], data, 0x20, 8 * 4);
	exportData(&Context_sub68K.pc, data, 0x48, 4);
	exportData(&Context_sub68K.sr, data, 0x50, 2);
	
	if (Context_sub68K.sr & 0x2000)
	{
		exportData(&Context_sub68K.asp, data, 0x52, 4);
		exportData(&Context_sub68K.areg[7], data, 0x56, 4);
	}
	else
	{
		exportData(&Context_sub68K.asp, data, 0x56, 4);
		exportData(&Context_sub68K.areg[7], data, 0x52, 4);
	}
	
	exportData(&Context_sub68K.odometer, data, 0x5A, 4);
	exportData(Context_sub68K.interrupts, data, 0x60, 8);
	exportData(&Ram_Word_State, data, 0x6C, 4);
	//here ends sub68k bit
	
	//PCM Chip dump
	exportData(&PCM_Chip.Rate, data, 0x100, 4);
	exportData(&PCM_Chip.Enable, data, 0x104, 4);
	exportData(&PCM_Chip.Cur_Chan, data, 0x108, 4);
	exportData(&PCM_Chip.Bank, data, 0x10C, 4);
	
	for (j = 0; j < 8; j++)
	{
		exportData(&PCM_Chip.Channel[j].ENV, data, 0x120 + (j * 4 * 11), 4);
		exportData(&PCM_Chip.Channel[j].PAN, data, 0x124 + (j * 4 * 11), 4);
		exportData(&PCM_Chip.Channel[j].MUL_L, data, 0x128 + (j * 4 * 11), 4);
		exportData(&PCM_Chip.Channel[j].MUL_R, data, 0x12C + (j * 4 * 11), 4);
		exportData(&PCM_Chip.Channel[j].St_Addr, data, 0x130 + (j * 4 * 11), 4);
		exportData(&PCM_Chip.Channel[j].Loop_Addr, data, 0x134 + (j * 4 * 11), 4);
		exportData(&PCM_Chip.Channel[j].Addr, data, 0x138 + (j * 4 * 11), 4);
		exportData(&PCM_Chip.Channel[j].Step, data, 0x13C + (j * 4 * 11), 4);
		exportData(&PCM_Chip.Channel[j].Step_B, data, 0x140 + (j * 4 * 11), 4);
		exportData(&PCM_Chip.Channel[j].Enable, data, 0x144 + (j * 4 * 11), 4);
		exportData(&PCM_Chip.Channel[j].Data, data, 0x148 + (j * 4 * 11), 4);
	}
	//End PCM Chip Dump

	//GFX State Dump
	exportData(&Rot_Comp.Stamp_Size, data, 0x300, 4);
	exportData(&Rot_Comp.Stamp_Map_Adr, data, 0x304, 4);
	exportData(&Rot_Comp.IB_V_Cell_Size, data, 0x308, 4);
	exportData(&Rot_Comp.IB_Adr, data, 0x30C, 4);
	exportData(&Rot_Comp.IB_Offset, data, 0x310, 4);
	exportData(&Rot_Comp.IB_H_Dot_Size, data, 0x314, 4);
	exportData(&Rot_Comp.IB_V_Dot_Size, data, 0x318, 4);
	exportData(&Rot_Comp.Vector_Adr, data, 0x31C, 4);
	exportData(&Rot_Comp.Rotation_Running, data, 0x320, 4);
	//End GFX State Dump

	//gate array bit
	exportData(&COMM.Flag, data, 0x0500, 4);
	src = (unsigned char *) &COMM.Command;
	for (j = 0; j < 8 * 2; j += 2)
	{
		for (i = 0; i < 2; i++)
		{
			data[i + 0x0504 + j] = *src++;
		}
	}
	src = (unsigned char *) &COMM.Status;
	for (j = 0; j < 8 * 2; j += 2)
	{
		for (i = 0; i < 2; i++)
		{
			data[i + 0x0514 + j] = *src++;
		}
	}
	exportData(&Memory_Control_Status, data, 0x0524, 4);
	exportData(&Init_Timer_INT3, data, 0x0528, 4);
	exportData(&Timer_INT3, data, 0x052C, 4);
	exportData(&Timer_Step, data, 0x0530, 4);
	exportData(&Int_Mask_S68K, data, 0x0534, 4);
	exportData(&Font_COLOR, data, 0x0538, 4);
	exportData(&Font_BITS, data, 0x053C, 4);
	exportData(&CD_Access_Timer, data, 0x0540, 4);
	exportData(&SCD.Status_CDC, data, 0x0544, 4);
	exportData(&SCD.Status_CDD, data, 0x0548, 4);
	exportData(&SCD.Cur_LBA, data, 0x054C, 4);
	exportData(&SCD.Cur_Track, data, 0x0550, 4);
	exportData(&S68K_Mem_WP, data, 0x0554, 4);
	exportData(&S68K_Mem_PM, data, 0x0558, 4);
	// More goes here When found
	//here ends gate array bit
	
	//Misc Status Flags
	exportData(&Ram_Word_State, data, 0xF00, 4); //For determining 1M or 2M
	exportData(&LED_Status, data, 0xF08, 4); //So the LED shows up properly
	//Word RAM state
	
	//Prg RAM
	exportData(Ram_Prg, data, 0x1000, 0x80000);
	
	//Word RAM
	if (Ram_Word_State >= 2)
		exportData(Ram_Word_1M, data, 0x81000, 0x40000); //1M mode
	else
		exportData(Ram_Word_2M, data, 0x81000, 0x40000); //2M mode
	//Word RAM end
	
	exportData(Ram_PCM, data, 0xC1000, 0x10000); //PCM RAM
	
	//CDD & CDC Data
	//CDD
	unsigned int CDD_src[8] = {CDD.Fader, CDD.Control, CDD.Cur_Comm, CDD.Status,
				   CDD.Minute, CDD.Seconde, CDD.Frame, CDD.Ext}; // Makes an array for easier loop construction
	for (j = 0; j < 8; j++)
	{
		exportData(&CDD_src[j], data, 0xD1000  + (4 * j), 4);
	}
	for(i = 0; i < 10; i++)
		data[0xD1020 + i] = CDD.Rcv_Status[i];
	for(i = 0; i < 10; i++)
		data[0xD102A + i] = CDD.Trans_Comm[i];
	//CDD end
	
	//CDC
	exportData(&CDC.RS0, data, 0xD1034, 4);
	exportData(&CDC.RS1, data, 0xD1038, 4);
	exportData(&CDC.Host_Data, data, 0xD103C, 4);
	exportData(&CDC.DMA_Adr, data, 0xD1040, 4);
	exportData(&CDC.Stop_Watch, data, 0xD1044, 4);
	exportData(&CDC.COMIN, data, 0xD1048, 4);
	exportData(&CDC.IFSTAT, data, 0xD104C, 4);
	exportData(&CDC.DBC.N, data, 0xD1050, 4);
	exportData(&CDC.DAC.N, data, 0xD1054, 4);
	exportData(&CDC.HEAD.N, data, 0xD1058, 4);
	exportData(&CDC.PT.N, data, 0xD105C, 4);
	exportData(&CDC.WA.N, data, 0xD1060, 4);
	exportData(&CDC.STAT.N, data, 0xD1064, 4);
	exportData(&CDC.SBOUT, data, 0xD1068, 4);
	exportData(&CDC.IFCTRL, data, 0xD106C, 4);
	exportData(&CDC.CTRL.N, data, 0xD1070, 4);
	exportData(CDC.Buffer, data, 0xD1074, ((32 * 1024 * 2) + 2352)); //Modif N. - added the *2 because the buffer appears to be that large
	//CDC end
	//CDD & CDC Data end
	
	//Modif N. - extra stuff added to save/set for synchronization reasons
	// I'm not sure how much of this really needs to be saved, should check it sometime
	
	unsigned int offset = SEGACD_LENGTH_EX1;
	
	exportDataAuto(&File_Add_Delay, data, &offset, 4);
//	exportDataAuto(CD_Audio_Buffer_L, data, &offset, 4*8192); // removed, seems to be unnecessary
//	exportDataAuto(CD_Audio_Buffer_R, data, &offset, 4*8192); // removed, seems to be unnecessary
	exportDataAuto(&CD_Audio_Buffer_Read_Pos, data, &offset, 4);
	exportDataAuto(&CD_Audio_Buffer_Write_Pos, data, &offset, 4);
	exportDataAuto(&CD_Audio_Starting, data, &offset, 4);
	exportDataAuto(&CD_Present, data, &offset, 4);
	exportDataAuto(&CD_Load_System, data, &offset, 4);
	exportDataAuto(&CD_Timer_Counter, data, &offset, 4);
	exportDataAuto(&CDD_Complete, data, &offset, 4);
	exportDataAuto(&track_number, data, &offset, 4);
	exportDataAuto(&CD_timer_st, data, &offset, 4);
	exportDataAuto(&CD_LBA_st, data, &offset, 4);
	exportDataAuto(&CDC_Decode_Reg_Read, data, &offset, 4);
	
	exportDataAuto(&SCD, data, &offset, sizeof(SCD));
//	exportDataAuto(&CDC, data, &offset, sizeof(CDC)); // removed, seems unnecessary/redundant
	exportDataAuto(&CDD, data, &offset, sizeof(CDD));
	exportDataAuto(&COMM, data, &offset, sizeof(COMM));
	
	exportDataAuto(Ram_Backup, data, &offset, sizeof(Ram_Backup));
	exportDataAuto(Ram_Backup_Ex, data, &offset, sizeof(Ram_Backup_Ex));
	
	exportDataAuto(&Rot_Comp, data, &offset, sizeof(Rot_Comp));
	exportDataAuto(&Stamp_Map_Adr, data, &offset, 4);
	exportDataAuto(&Buffer_Adr, data, &offset, 4);
	exportDataAuto(&Vector_Adr, data, &offset, 4);
	exportDataAuto(&Jmp_Adr, data, &offset, 4);
	exportDataAuto(&Float_Part, data, &offset, 4);
	exportDataAuto(&Draw_Speed, data, &offset, 4);
	exportDataAuto(&XS, data, &offset, 4);
	exportDataAuto(&YS, data, &offset, 4);
	exportDataAuto(&DXS, data, &offset, 4);
	exportDataAuto(&DYS, data, &offset, 4);
	exportDataAuto(&XD, data, &offset, 4);
	exportDataAuto(&YD, data, &offset, 4);
	exportDataAuto(&XD_Mul, data, &offset, 4);
	exportDataAuto(&H_Dot, data, &offset, 4);
	
	exportDataAuto(&Context_sub68K.cycles_needed, data, &offset, 44);
	exportDataAuto(&Rom_Data[0x72], data, &offset, 2);	//Sega CD games can overwrite the low two bytes of the Horizontal Interrupt vector
	
#ifdef GENS_MP3
	exportDataAuto(&fatal_mp3_error, data, &offset, 4);
	exportDataAuto(&Current_OUT_Pos, data, &offset, 4);
	exportDataAuto(&Current_OUT_Size, data, &offset, 4);
#else
	int dummy = 0;
	exportDataAuto(&dummy, data, &offset, 4);
	exportDataAuto(&dummy, data, &offset, 4);
	exportDataAuto(&dummy, data, &offset, 4);
#endif /* GENS_MP3 */
	exportDataAuto(&Track_Played, data, &offset, 1);
	exportDataAuto(played_tracks_linear, data, &offset, 100);
	//exportDataAuto(&Current_IN_Pos, data, &offset, 4)? // don't save this; bad things happen
	
#ifdef GENS_DEBUG_SAVESTATE
	assert(offset == SEGACD_LENGTH_EX);
#endif
}


/**
 * gsxImport32X(): Load 32X data from a GSX savestate.
 * (Ported from Gens Rerecording.)
 * @param data Savestate data.
 */
void Savestate::gsxImport32X(const unsigned char* data)
{
	// TODO: Reimplement v5 support.
	
	unsigned int offset = 0;
	int i, contextNum;
	
	for (contextNum = 0; contextNum < 2; contextNum++)
	{
		SH2_CONTEXT* context = (contextNum == 0) ? &M_SH2 : &S_SH2;

		importDataAuto(context->Cache, data, &offset, sizeof(context->Cache));
		importDataAuto(context->R, data, &offset, sizeof(context->R));
		importDataAuto(&context->SR, data, &offset, sizeof(context->SR));
		importDataAuto(&context->INT, data, &offset, sizeof(context->INT));
		importDataAuto(&context->GBR, data, &offset, sizeof(context->GBR));
		importDataAuto(&context->VBR, data, &offset, sizeof(context->VBR));
		importDataAuto(context->INT_QUEUE, data, &offset, sizeof(context->INT_QUEUE));
		importDataAuto(&context->MACH, data, &offset, sizeof(context->MACH));
		importDataAuto(&context->MACL, data, &offset, sizeof(context->MACL));
		importDataAuto(&context->PR, data, &offset, sizeof(context->PR));
		importDataAuto(&context->PC, data, &offset, sizeof(context->PC));
		importDataAuto(&context->Status, data, &offset, sizeof(context->Status));
		importDataAuto(&context->Base_PC, data, &offset, sizeof(context->Base_PC));
		importDataAuto(&context->Fetch_Start, data, &offset, sizeof(context->Fetch_Start));
		importDataAuto(&context->Fetch_End, data, &offset, sizeof(context->Fetch_End));
		importDataAuto(&context->DS_Inst, data, &offset, sizeof(context->DS_Inst));
		importDataAuto(&context->DS_PC, data, &offset, sizeof(context->DS_PC));
		importDataAuto(&context->Odometer, data, &offset, sizeof(context->Odometer));
		importDataAuto(&context->Cycle_TD, data, &offset, sizeof(context->Cycle_TD));
		importDataAuto(&context->Cycle_IO, data, &offset, sizeof(context->Cycle_IO));
		importDataAuto(&context->Cycle_Sup, data, &offset, sizeof(context->Cycle_Sup));
		importDataAuto(context->IO_Reg, data, &offset, sizeof(context->IO_Reg));
		importDataAuto(&context->DVCR, data, &offset, sizeof(context->DVCR));
		importDataAuto(&context->DVSR, data, &offset, sizeof(context->DVSR));
		importDataAuto(&context->DVDNTH, data, &offset, sizeof(context->DVDNTH));
		importDataAuto(&context->DVDNTL, data, &offset, sizeof(context->DVDNTL));
		importDataAuto(&context->DRCR0, data, &offset, sizeof(context->DRCR0));
		importDataAuto(&context->DRCR1, data, &offset, sizeof(context->DRCR1));
		importDataAuto(&context->DREQ0, data, &offset, sizeof(context->DREQ0));
		importDataAuto(&context->DREQ1, data, &offset, sizeof(context->DREQ1));
		importDataAuto(&context->DMAOR, data, &offset, sizeof(context->DMAOR));
		importDataAuto(&context->SAR0, data, &offset, sizeof(context->SAR0));
		importDataAuto(&context->DAR0, data, &offset, sizeof(context->DAR0));
		importDataAuto(&context->TCR0, data, &offset, sizeof(context->TCR0));
		importDataAuto(&context->CHCR0, data, &offset, sizeof(context->CHCR0));
		importDataAuto(&context->SAR1, data, &offset, sizeof(context->SAR1));
		importDataAuto(&context->DAR1, data, &offset, sizeof(context->DAR1));
		importDataAuto(&context->TCR1, data, &offset, sizeof(context->TCR1));
		importDataAuto(&context->CHCR1, data, &offset, sizeof(context->CHCR1));
		importDataAuto(&context->VCRDIV, data, &offset, sizeof(context->VCRDIV));
		importDataAuto(&context->VCRDMA0, data, &offset, sizeof(context->VCRDMA0));
		importDataAuto(&context->VCRDMA1, data, &offset, sizeof(context->VCRDMA1));
		importDataAuto(&context->VCRWDT, data, &offset, sizeof(context->VCRWDT));
		importDataAuto(&context->IPDIV, data, &offset, sizeof(context->IPDIV));
		importDataAuto(&context->IPDMA, data, &offset, sizeof(context->IPDMA));
		importDataAuto(&context->IPWDT, data, &offset, sizeof(context->IPWDT));
		importDataAuto(&context->IPBSC, data, &offset, sizeof(context->IPBSC));
		importDataAuto(&context->BARA, data, &offset, sizeof(context->BARA));
		importDataAuto(&context->BAMRA, data, &offset, sizeof(context->BAMRA));
		importDataAuto(context->WDT_Tab, data, &offset, sizeof(context->WDT_Tab));
		importDataAuto(&context->WDTCNT, data, &offset, sizeof(context->WDTCNT));
		importDataAuto(&context->WDT_Sft, data, &offset, sizeof(context->WDT_Sft));
		importDataAuto(&context->WDTSR, data, &offset, sizeof(context->WDTSR));
		importDataAuto(&context->WDTRST, data, &offset, sizeof(context->WDTRST));
		importDataAuto(context->FRT_Tab, data, &offset, sizeof(context->FRT_Tab));
		importDataAuto(&context->FRTCNT, data, &offset, sizeof(context->FRTCNT));
		importDataAuto(&context->FRTOCRA, data, &offset, sizeof(context->FRTOCRA));
		importDataAuto(&context->FRTOCRB, data, &offset, sizeof(context->FRTOCRB));
		importDataAuto(&context->FRTTIER, data, &offset, sizeof(context->FRTTIER));
		importDataAuto(&context->FRTCSR, data, &offset, sizeof(context->FRTCSR));
		importDataAuto(&context->FRTTCR, data, &offset, sizeof(context->FRTTCR));
		importDataAuto(&context->FRTTOCR, data, &offset, sizeof(context->FRTTOCR));
		importDataAuto(&context->FRTICR, data, &offset, sizeof(context->FRTICR));
		importDataAuto(&context->FRT_Sft, data, &offset, sizeof(context->FRT_Sft));
		importDataAuto(&context->BCR1, data, &offset, sizeof(context->BCR1));
		importDataAuto(&context->FRTCSR, data, &offset, sizeof(context->FRTCSR));
	}

	importDataAuto(_32X_Ram, data, &offset, sizeof(_32X_Ram));
	importDataAuto(_MSH2_Reg, data, &offset, sizeof(_MSH2_Reg));
	importDataAuto(_SSH2_Reg, data, &offset, sizeof(_SSH2_Reg));
	importDataAuto(_SH2_VDP_Reg, data, &offset, sizeof(_SH2_VDP_Reg));
	importDataAuto(_32X_Comm, data, &offset, sizeof(_32X_Comm));
	importDataAuto(&_32X_ADEN, data, &offset, sizeof(_32X_ADEN));
	importDataAuto(&_32X_RES, data, &offset, sizeof(_32X_RES));
	importDataAuto(&_32X_FM, data, &offset, sizeof(_32X_FM));
	importDataAuto(&_32X_RV, data, &offset, sizeof(_32X_RV));
	importDataAuto(&_32X_DREQ_ST, data, &offset, sizeof(_32X_DREQ_ST));
	importDataAuto(&_32X_DREQ_SRC, data, &offset, sizeof(_32X_DREQ_SRC));
	importDataAuto(&_32X_DREQ_DST, data, &offset, sizeof(_32X_DREQ_DST));
	importDataAuto(&_32X_DREQ_LEN, data, &offset, sizeof(_32X_DREQ_LEN));
	importDataAuto(_32X_FIFO_A, data, &offset, sizeof(_32X_FIFO_A));
	importDataAuto(_32X_FIFO_B, data, &offset, sizeof(_32X_FIFO_B));
	importDataAuto(&_32X_FIFO_Block, data, &offset, sizeof(_32X_FIFO_Block));
	importDataAuto(&_32X_FIFO_Read, data, &offset, sizeof(_32X_FIFO_Read));
	importDataAuto(&_32X_FIFO_Write, data, &offset, sizeof(_32X_FIFO_Write));
	importDataAuto(&_32X_MINT, data, &offset, sizeof(_32X_MINT));
	importDataAuto(&_32X_SINT, data, &offset, sizeof(_32X_SINT));
	importDataAuto(&_32X_HIC, data, &offset, sizeof(_32X_HIC));
	importDataAuto(&CPL_SSH2, data, &offset, sizeof(CPL_SSH2));
	importDataAuto(&CPL_MSH2, data, &offset, sizeof(CPL_MSH2));
	importDataAuto(&Cycles_MSH2, data, &offset, sizeof(Cycles_MSH2));
	importDataAuto(&Cycles_SSH2, data, &offset, sizeof(Cycles_SSH2));

	importDataAuto(&_32X_VDP, data, &offset, sizeof(_32X_VDP));
	importDataAuto(_32X_VDP_Ram, data, &offset, sizeof(_32X_VDP_Ram));
	importDataAuto(_32X_VDP_CRam, data, &offset, sizeof(_32X_VDP_CRam));

	importDataAuto(Set_SR_Table, data, &offset, sizeof(Set_SR_Table));
	importDataAuto(&Bank_SH2, data, &offset, sizeof(Bank_SH2));

	importDataAuto(PWM_FIFO_R, data, &offset, sizeof(PWM_FIFO_R));
	importDataAuto(PWM_FIFO_L, data, &offset, sizeof(PWM_FIFO_L));
	importDataAuto(&PWM_RP_R, data, &offset, sizeof(PWM_RP_R));
	importDataAuto(&PWM_WP_R, data, &offset, sizeof(PWM_WP_R));
	importDataAuto(&PWM_RP_L, data, &offset, sizeof(PWM_RP_L));
	importDataAuto(&PWM_WP_L, data, &offset, sizeof(PWM_WP_L));
	importDataAuto(&PWM_Cycles, data, &offset, sizeof(PWM_Cycles));
	importDataAuto(&PWM_Cycle, data, &offset, sizeof(PWM_Cycle));
	importDataAuto(&PWM_Cycle_Cnt, data, &offset, sizeof(PWM_Cycle_Cnt));
	importDataAuto(&PWM_Int, data, &offset, sizeof(PWM_Int));
	importDataAuto(&PWM_Int_Cnt, data, &offset, sizeof(PWM_Int_Cnt));
	importDataAuto(&PWM_Mode, data, &offset, sizeof(PWM_Mode));
	importDataAuto(&PWM_Out_R, data, &offset, sizeof(PWM_Out_R));
	importDataAuto(&PWM_Out_L, data, &offset, sizeof(PWM_Out_L));

	importDataAuto(_32X_Rom, data, &offset, 1024); // just in case some of these bytes are not in fact read-only as was apparently the case with Sega CD games (1024 seems acceptably small)
	importDataAuto(_32X_MSH2_Rom, data, &offset, sizeof(_32X_MSH2_Rom));
	importDataAuto(_32X_SSH2_Rom, data, &offset, sizeof(_32X_SSH2_Rom));

	M68K_32X_Mode();
	_32X_Set_FB();
	M68K_Set_32X_Rom_Bank();

	//Recalculate_Palettes();
	for (i = 0; i < 0x100; i++)
	{
		_32X_VDP_CRam_Ajusted[i] = _32X_Palette_16B[_32X_VDP_CRam[i]];
		_32X_VDP_CRam_Ajusted32[i] = _32X_Palette_32B[_32X_VDP_CRam[i]];
	}

#ifdef GENS_DEBUG_SAVESTATE
	assert(offset == G32X_LENGTH_EX);
#endif
}


/**
 * gsxExport32X(): Save 32X data to a GSX savestate.
 * (Ported from Gens Rerecording.)
 * @param data Savestate data buffer.
 */
void Savestate::gsxExport32X(unsigned char* data)
{
	// NOTE: Gens v7 Savestates with 32X data will *not* work on older versions of Gens!
	unsigned int offset = 0;
	int contextNum;
	
	for (contextNum = 0; contextNum < 2; contextNum++)
	{
		SH2_CONTEXT* context = (contextNum == 0) ? &M_SH2 : &S_SH2;
	
		exportDataAuto(context->Cache, data, &offset, sizeof(context->Cache));
		exportDataAuto(context->R, data, &offset, sizeof(context->R));
		exportDataAuto(&context->SR, data, &offset, sizeof(context->SR));
		exportDataAuto(&context->INT, data, &offset, sizeof(context->INT));
		exportDataAuto(&context->GBR, data, &offset, sizeof(context->GBR));
		exportDataAuto(&context->VBR, data, &offset, sizeof(context->VBR));
		exportDataAuto(context->INT_QUEUE, data, &offset, sizeof(context->INT_QUEUE));
		exportDataAuto(&context->MACH, data, &offset, sizeof(context->MACH));
		exportDataAuto(&context->MACL, data, &offset, sizeof(context->MACL));
		exportDataAuto(&context->PR, data, &offset, sizeof(context->PR));
		exportDataAuto(&context->PC, data, &offset, sizeof(context->PC));
		exportDataAuto(&context->Status, data, &offset, sizeof(context->Status));
		exportDataAuto(&context->Base_PC, data, &offset, sizeof(context->Base_PC));
		exportDataAuto(&context->Fetch_Start, data, &offset, sizeof(context->Fetch_Start));
		exportDataAuto(&context->Fetch_End, data, &offset, sizeof(context->Fetch_End));
		exportDataAuto(&context->DS_Inst, data, &offset, sizeof(context->DS_Inst));
		exportDataAuto(&context->DS_PC, data, &offset, sizeof(context->DS_PC));
		exportDataAuto(&context->Odometer, data, &offset, sizeof(context->Odometer));
		exportDataAuto(&context->Cycle_TD, data, &offset, sizeof(context->Cycle_TD));
		exportDataAuto(&context->Cycle_IO, data, &offset, sizeof(context->Cycle_IO));
		exportDataAuto(&context->Cycle_Sup, data, &offset, sizeof(context->Cycle_Sup));
		exportDataAuto(context->IO_Reg, data, &offset, sizeof(context->IO_Reg));
		exportDataAuto(&context->DVCR, data, &offset, sizeof(context->DVCR));
		exportDataAuto(&context->DVSR, data, &offset, sizeof(context->DVSR));
		exportDataAuto(&context->DVDNTH, data, &offset, sizeof(context->DVDNTH));
		exportDataAuto(&context->DVDNTL, data, &offset, sizeof(context->DVDNTL));
		exportDataAuto(&context->DRCR0, data, &offset, sizeof(context->DRCR0));
		exportDataAuto(&context->DRCR1, data, &offset, sizeof(context->DRCR1));
		exportDataAuto(&context->DREQ0, data, &offset, sizeof(context->DREQ0));
		exportDataAuto(&context->DREQ1, data, &offset, sizeof(context->DREQ1));
		exportDataAuto(&context->DMAOR, data, &offset, sizeof(context->DMAOR));
		exportDataAuto(&context->SAR0, data, &offset, sizeof(context->SAR0));
		exportDataAuto(&context->DAR0, data, &offset, sizeof(context->DAR0));
		exportDataAuto(&context->TCR0, data, &offset, sizeof(context->TCR0));
		exportDataAuto(&context->CHCR0, data, &offset, sizeof(context->CHCR0));
		exportDataAuto(&context->SAR1, data, &offset, sizeof(context->SAR1));
		exportDataAuto(&context->DAR1, data, &offset, sizeof(context->DAR1));
		exportDataAuto(&context->TCR1, data, &offset, sizeof(context->TCR1));
		exportDataAuto(&context->CHCR1, data, &offset, sizeof(context->CHCR1));
		exportDataAuto(&context->VCRDIV, data, &offset, sizeof(context->VCRDIV));
		exportDataAuto(&context->VCRDMA0, data, &offset, sizeof(context->VCRDMA0));
		exportDataAuto(&context->VCRDMA1, data, &offset, sizeof(context->VCRDMA1));
		exportDataAuto(&context->VCRWDT, data, &offset, sizeof(context->VCRWDT));
		exportDataAuto(&context->IPDIV, data, &offset, sizeof(context->IPDIV));
		exportDataAuto(&context->IPDMA, data, &offset, sizeof(context->IPDMA));
		exportDataAuto(&context->IPWDT, data, &offset, sizeof(context->IPWDT));
		exportDataAuto(&context->IPBSC, data, &offset, sizeof(context->IPBSC));
		exportDataAuto(&context->BARA, data, &offset, sizeof(context->BARA));
		exportDataAuto(&context->BAMRA, data, &offset, sizeof(context->BAMRA));
		exportDataAuto(context->WDT_Tab, data, &offset, sizeof(context->WDT_Tab));
		exportDataAuto(&context->WDTCNT, data, &offset, sizeof(context->WDTCNT));
		exportDataAuto(&context->WDT_Sft, data, &offset, sizeof(context->WDT_Sft));
		exportDataAuto(&context->WDTSR, data, &offset, sizeof(context->WDTSR));
		exportDataAuto(&context->WDTRST, data, &offset, sizeof(context->WDTRST));
		exportDataAuto(context->FRT_Tab, data, &offset, sizeof(context->FRT_Tab));
		exportDataAuto(&context->FRTCNT, data, &offset, sizeof(context->FRTCNT));
		exportDataAuto(&context->FRTOCRA, data, &offset, sizeof(context->FRTOCRA));
		exportDataAuto(&context->FRTOCRB, data, &offset, sizeof(context->FRTOCRB));
		exportDataAuto(&context->FRTTIER, data, &offset, sizeof(context->FRTTIER));
		exportDataAuto(&context->FRTCSR, data, &offset, sizeof(context->FRTCSR));
		exportDataAuto(&context->FRTTCR, data, &offset, sizeof(context->FRTTCR));
		exportDataAuto(&context->FRTTOCR, data, &offset, sizeof(context->FRTTOCR));
		exportDataAuto(&context->FRTICR, data, &offset, sizeof(context->FRTICR));
		exportDataAuto(&context->FRT_Sft, data, &offset, sizeof(context->FRT_Sft));
		exportDataAuto(&context->BCR1, data, &offset, sizeof(context->BCR1));
		exportDataAuto(&context->FRTCSR, data, &offset, sizeof(context->FRTCSR));
	}
	
	exportDataAuto(_32X_Ram, data, &offset, sizeof(_32X_Ram));
	exportDataAuto(_MSH2_Reg, data, &offset, sizeof(_MSH2_Reg));
	exportDataAuto(_SSH2_Reg, data, &offset, sizeof(_SSH2_Reg));
	exportDataAuto(_SH2_VDP_Reg, data, &offset, sizeof(_SH2_VDP_Reg));
	exportDataAuto(_32X_Comm, data, &offset, sizeof(_32X_Comm));
	exportDataAuto(&_32X_ADEN, data, &offset, sizeof(_32X_ADEN));
	exportDataAuto(&_32X_RES, data, &offset, sizeof(_32X_RES));
	exportDataAuto(&_32X_FM, data, &offset, sizeof(_32X_FM));
	exportDataAuto(&_32X_RV, data, &offset, sizeof(_32X_RV));
	exportDataAuto(&_32X_DREQ_ST, data, &offset, sizeof(_32X_DREQ_ST));
	exportDataAuto(&_32X_DREQ_SRC, data, &offset, sizeof(_32X_DREQ_SRC));
	exportDataAuto(&_32X_DREQ_DST, data, &offset, sizeof(_32X_DREQ_DST));
	exportDataAuto(&_32X_DREQ_LEN, data, &offset, sizeof(_32X_DREQ_LEN));
	exportDataAuto(_32X_FIFO_A, data, &offset, sizeof(_32X_FIFO_A));
	exportDataAuto(_32X_FIFO_B, data, &offset, sizeof(_32X_FIFO_B));
	exportDataAuto(&_32X_FIFO_Block, data, &offset, sizeof(_32X_FIFO_Block));
	exportDataAuto(&_32X_FIFO_Read, data, &offset, sizeof(_32X_FIFO_Read));
	exportDataAuto(&_32X_FIFO_Write, data, &offset, sizeof(_32X_FIFO_Write));
	exportDataAuto(&_32X_MINT, data, &offset, sizeof(_32X_MINT));
	exportDataAuto(&_32X_SINT, data, &offset, sizeof(_32X_SINT));
	exportDataAuto(&_32X_HIC, data, &offset, sizeof(_32X_HIC));
	exportDataAuto(&CPL_SSH2, data, &offset, sizeof(CPL_SSH2));
	exportDataAuto(&CPL_MSH2, data, &offset, sizeof(CPL_MSH2));
	exportDataAuto(&Cycles_MSH2, data, &offset, sizeof(Cycles_MSH2));
	exportDataAuto(&Cycles_SSH2, data, &offset, sizeof(Cycles_SSH2));
	
	exportDataAuto(&_32X_VDP, data, &offset, sizeof(_32X_VDP));
	exportDataAuto(_32X_VDP_Ram, data, &offset, sizeof(_32X_VDP_Ram));
	exportDataAuto(_32X_VDP_CRam, data, &offset, sizeof(_32X_VDP_CRam));
	
	exportDataAuto(Set_SR_Table, data, &offset, sizeof(Set_SR_Table));
	exportDataAuto(&Bank_SH2, data, &offset, sizeof(Bank_SH2));
	
	exportDataAuto(PWM_FIFO_R, data, &offset, sizeof(PWM_FIFO_R));
	exportDataAuto(PWM_FIFO_L, data, &offset, sizeof(PWM_FIFO_L));
	exportDataAuto(&PWM_RP_R, data, &offset, sizeof(PWM_RP_R));
	exportDataAuto(&PWM_WP_R, data, &offset, sizeof(PWM_WP_R));
	exportDataAuto(&PWM_RP_L, data, &offset, sizeof(PWM_RP_L));
	exportDataAuto(&PWM_WP_L, data, &offset, sizeof(PWM_WP_L));
	exportDataAuto(&PWM_Cycles, data, &offset, sizeof(PWM_Cycles));
	exportDataAuto(&PWM_Cycle, data, &offset, sizeof(PWM_Cycle));
	exportDataAuto(&PWM_Cycle_Cnt, data, &offset, sizeof(PWM_Cycle_Cnt));
	exportDataAuto(&PWM_Int, data, &offset, sizeof(PWM_Int));
	exportDataAuto(&PWM_Int_Cnt, data, &offset, sizeof(PWM_Int_Cnt));
	exportDataAuto(&PWM_Mode, data, &offset, sizeof(PWM_Mode));
	exportDataAuto(&PWM_Out_R, data, &offset, sizeof(PWM_Out_R));
	exportDataAuto(&PWM_Out_L, data, &offset, sizeof(PWM_Out_L));
	
	exportDataAuto(_32X_Rom, data, &offset, 1024); // just in case some of these bytes are not in fact read-only as was apparently the case with Sega CD games (1024 seems acceptably small)
	exportDataAuto(_32X_MSH2_Rom, data, &offset, sizeof(_32X_MSH2_Rom));
	exportDataAuto(_32X_SSH2_Rom, data, &offset, sizeof(_32X_SSH2_Rom));
	
#ifdef GENS_DEBUG_SAVESTATE
	assert(offset == G32X_LENGTH_EX);
#endif
}


/**
 * getSRAMFilename(): Get the filename of the SRAM file.
 * @return Filename of the SRAM file.
 */
inline string Savestate::getSRAMFilename(void)
{
	if (strlen(ROM_Name) == 0)
		return "";
	return string(State_Dir) + string(ROM_Name) + ".srm";
}


/**
 * loadSRAM(): Load the SRAM file.
 * @return 1 on success; 0 on error.
 */
int Savestate::loadSRAM(void)
{
	FILE* SRAM_File = 0;
	
	memset(SRAM, 0, 64 * 1024);
	
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif /* GENS_OS_WIN32 */
	
	string filename = getSRAMFilename();
	if (filename.empty())
		return 0;
	if ((SRAM_File = fopen(filename.c_str(), "rb")) == 0)
		return 0;
	
	fread(SRAM, 64 * 1024, 1, SRAM_File);
	fclose(SRAM_File);
	
	string msg = "SRAM loaded from " + filename;
	draw->writeText(msg, 2000);
	return 1;
}


/**
 * saveSRAM(): Save the SRAM file.
 * @return 1 on success; 0 on error.
 */
int Savestate::saveSRAM(void)
{
	FILE* SRAM_File = 0;
	int size_to_save, i;
	
	i = (64 * 1024) - 1;
	while ((i >= 0) && (SRAM[i] == 0))
		i--;
	
	if (i < 0)
		return 0;
	
	i++;
	
	size_to_save = 1;
	while (i > size_to_save)
		size_to_save <<= 1;
	
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif /* GENS_OS_WIN32 */
	
	string filename = getSRAMFilename();
	if (filename.empty())
		return 0;
	if ((SRAM_File = fopen(filename.c_str(), "wb")) == 0)
		return 0;
	
	fwrite(SRAM, size_to_save, 1, SRAM_File);
	fclose(SRAM_File);
	
	draw->writeText("SRAM saved in " + filename, 2000);
	return 1;
}


/**
 * formatSegaCD_BRAM(): Format the internal SegaCD BRAM.
 * @param buf Pointer to location 0x1FC0 in the internal SegaCD BRAM.
 */
void Savestate::formatSegaCD_BRAM(unsigned char *buf)
{
	// TODO: Format cartridge BRAM.
	const char brmHeader[0x40] =
	{
		0x5F, 0x5F, 0x5F, 0x5F, 0x5F, 0x5F, 0x5F, 0x5F,
		0x5F, 0x5F, 0x5F, 0x00, 0x00, 0x00, 0x00, 0x40,
		0x00, 0x7D, 0x00, 0x7D, 0x00, 0x7D, 0x00, 0x7D,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		
		'S', 'E', 'G', 'A', 0x5F, 'C', 'D', 0x5F, 'R', 'O', 'M', 0x00, 0x01, 0x00, 0x00, 0x00,
		'R', 'A', 'M', 0x5F, 'C', 'A', 'R', 'T', 'R', 'I', 'D', 'G', 'E', 0x5F, 0x5F, 0x5F
	};
	
	memcpy(buf, brmHeader, sizeof(brmHeader));
}


/**
 * formatSegaCD_BackupRAM(): Format the SegaCD backup RAM.
 */
void Savestate::formatSegaCD_BackupRAM(void)
{
	// SegaCD internal BRAM.
	memset(Ram_Backup, 0, 8 * 1024);
	formatSegaCD_BRAM(&Ram_Backup[0x1FC0]);
	
	// SegaCD cartridge memory.
	// TODO: Format the cartridge memory.
	memset(Ram_Backup_Ex, 0, 64 * 1024);
}


/**
 * getSRAMFilename(): Get the filename of the BRAM file.
 * @return Filename of the BRAM file.
 */
inline string Savestate::getBRAMFilename(void)
{
	if (strlen(ROM_Name) == 0)
		return "";
	return string(State_Dir) + string(ROM_Name) + ".brm";
}


/**
 * loadBRAM(): Load the BRAM file.
 * @return 1 on success; 0 on error.
 */
int Savestate::loadBRAM(void)
{
	FILE* BRAM_File = 0;
	
	Savestate::formatSegaCD_BackupRAM();
	
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif /* GENS_OS_WIN32 */
	
	string filename = getBRAMFilename();
	if (filename.empty())
		return 0;
	if ((BRAM_File = fopen(filename.c_str(), "rb")) == 0)
		return 0;
	
	fread(Ram_Backup, 8 * 1024, 1, BRAM_File);
	fread(Ram_Backup_Ex, (8 << BRAM_Ex_Size) * 1024, 1, BRAM_File);
	fclose(BRAM_File);
	
	draw->writeText("BRAM loaded in " + filename, 2000);
	return 1;
}

/**
 * saveBRAM(): Save the BRAM file.
 * @return 1 on success; 0 on error.
 */
int Savestate::saveBRAM(void)
{
	FILE* BRAM_File = 0;
	
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif /* GENS_OS_WIN32 */
	
	string filename = getBRAMFilename();
	if (filename.empty())
		return 0;
	if ((BRAM_File = fopen(filename.c_str(), "wb")) == 0)
		return 0;
	
	fwrite(Ram_Backup, 8 * 1024, 1, BRAM_File);
	fwrite(Ram_Backup_Ex, (8 << BRAM_Ex_Size) * 1024, 1, BRAM_File);
	fclose(BRAM_File);
	
	draw->writeText("BRAM saved in " + filename, 2000);
	return 1;
}
