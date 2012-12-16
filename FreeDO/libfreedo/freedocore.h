/*
  www.freedo.org
The first and only working 3DO multiplayer emulator.

The FreeDO licensed under modified GNU LGPL, with following notes:

*   The owners and original authors of the FreeDO have full right to develop closed source derivative work.
*   Any non-commercial uses of the FreeDO sources or any knowledge obtained by studying or reverse engineering
    of the sources, or any other material published by FreeDO have to be accompanied with full credits.
*   Any commercial uses of FreeDO sources or any knowledge obtained by studying or reverse engineering of the sources,
    or any other material published by FreeDO is strictly forbidden without owners approval.

The above notes are taking precedence over GNU LGPL in conflicting situations.

Project authors:

Alexander Troosh
Maxim Grishin
Allen Wright
John Sammons
Felix Lazarev
*/

#ifndef __3DO_SYSTEM_HEADER_DEFINITION
#define __3DO_SYSTEM_HEADER_DEFINITION

//------------------------------------------------------------------------------
#pragma pack(push,1)

struct VDLLine
//VDLP Line - one VDLP line per patent
{
        unsigned short line[320*4];//,line2[320*2*16];
        unsigned char xCLUTB[32];
	unsigned char xCLUTG[32];
	unsigned char xCLUTR[32];
        unsigned int xOUTCONTROLL;
        unsigned int xCLUTDMA;
	unsigned int xBACKGROUND;
};
struct VDLFrame
{
        VDLLine lines[240*4];
        unsigned int srcw,srch;
};

#pragma pack(pop)

#define EXT_READ_ROMS           1
#define EXT_READ_NVRAM          2
#define EXT_WRITE_NVRAM         3
#define EXT_SWAPFRAME           5       //frame swap (in mutlithreaded) or frame draw(single treaded)
#define EXT_PUSH_SAMPLE         6       //sends sample to the buffer
#define EXT_GET_PBUSLEN         7
#define EXT_GETP_PBUSDATA       8
#define EXT_KPRINT              9
#define EXT_DEBUG_PRINT         10
#define EXT_FRAMETRIGGER_MT     12      //multitasking
#define EXT_READ2048            14      //for XBUS Plugin
#define EXT_GET_DISC_SIZE       15
#define EXT_ON_SECTOR           16
typedef void* (*_ext_Interface)(int, void*);

#define FDP_FREEDOCORE_VERSION  0
#define FDP_INIT                1    //set ext_interface
#define FDP_DESTROY             2
#define FDP_DO_EXECFRAME        3       //execute 1/60 of second
#define FDP_DO_FRAME_MT         4      //multitasking
#define FDP_DO_EXECFRAME_MT     5      //multitasking
#define FDP_DO_LOAD             6       //load state from buffer, returns !NULL if everything went smooth
#define FDP_GET_SAVE_SIZE       7       //return size of savestatemachine
#define FDP_DO_SAVE             8       //save state to buffer
#define FDP_GETP_NVRAM          9       //returns ptr to NVRAM 32K
#define FDP_GETP_RAMS           10       //returns ptr to RAM 3M
#define FDP_GETP_ROMS           11       //returns ptr to ROM 2M
#define FDP_GETP_PROFILE        12       //returns profile pointer, sizeof = 3M/4
#define FDP_BUGTEMPORALFIX      13
#define FDP_SET_ARMCLOCK        14
#define FDP_SET_TEXQUALITY      15
#define FDP_GETP_WRCOUNT        16

#ifdef __MSVC__

#ifdef FREEDOCORE_EXPORTS
#define FREEDOCORE_API __declspec(dllexport)
#else
#define FREEDOCORE_API __declspec(dllimport)
#endif


#else

#define FREEDOCORE_API

#endif

extern "C"
{
        FREEDOCORE_API void* _freedo_Interface(int procedure, void *datum=0);
};

#ifdef __MSVC__
#ifndef FREEDOCORE_EXPORTS
#pragma comment(lib, "freedocore.lib")
#endif
#endif
//------------------------------------------------------------------------------


#endif
