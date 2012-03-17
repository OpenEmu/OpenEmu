#ifndef GENS_CD_ASPI_HPP
#define GENS_CD_ASPI_HPP

/* This file should be 100% source compatible according to MSes docs and
 * Adaptecs docs */

#ifdef __cplusplus
extern "C" {
#endif

// Needed for uint_*.
#include <stdint.h>

// ASPI definitions
#include "aspi.h"

// SegaCD system
#include "cd_sys.hpp"

#define STOP_DISC	0
#define START_DISC	1
#define OPEN_TRAY	2
#define CLOSE_TRAY	3

typedef struct
{
	uint8_t rsvd;
	uint8_t ADR;
	uint8_t trackNumber;
	uint8_t rsvd2;
	uint8_t addr[4];
} TOCTRACK;

typedef struct
{
	uint16_t tocLen;
	uint8_t  firstTrack;
	uint8_t  lastTrack;
	TOCTRACK tracks[100];
} TOC, *PTOC, *LPTOC;

extern int cdromSpeed;
extern int Num_CD_Drive;
#if defined(GENS_OS_WIN32)
extern int cdromDeviceID;
#elif defined(GENS_OS_LINUX)
extern char cdromDeviceName[64];
#endif

// If ASPI is initialized, this is set.
extern int ASPI_Initialized;

int ASPI_Init(void);
int ASPI_End(void);

void ASPI_Reset_Drive(char *buf);

#ifdef GENS_OS_WIN32
void ASPI_Scan_Drives(void);
int ASPI_Get_Drive_Info(int dev, unsigned char *Inf);
int ASPI_Set_Timeout(int sec);
#endif

int ASPI_Test_Unit_Ready(int timeout);
#ifdef GENS_OS_WIN32
int ASPI_Set_CD_Speed(int rate, int wait);
#endif
#ifdef GENS_OS_LINUX
void LINUXCD_Select_Speed(void);
#endif
int ASPI_Lock(int flock);
int ASPI_Star_Stop_Unit(int op, int imm, int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *));
int ASPI_Read_TOC(int MSF, int format, int st, int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *));
int ASPI_Stop_Play_Scan(int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *));
int ASPI_Seek(int pos, int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *));
int ASPI_Read_CD_LBA(int adr, int length, unsigned char sector, unsigned char flag, unsigned char sub_chan, int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *));

// Default Callback

int ASPI_Star_Stop_Unit_COMP(SRB_ExecSCSICmd *s);
#ifdef GENS_OS_WIN32
int ASPI_Read_TOC_LBA_COMP(SRB_ExecSCSICmd *s);
int ASPI_Read_TOC_MSF_COMP(SRB_ExecSCSICmd *s);
int ASPI_Mechanism_State_COMP(SRB_ExecSCSICmd *s);
int ASPI_Play_CD_MSF_COMP(SRB_ExecSCSICmd *s);
#endif
int ASPI_Stop_Play_Scan_COMP(SRB_ExecSCSICmd *s);
#ifdef GENS_OS_WIN32
int ASPI_Pause_Resume_COMP(SRB_ExecSCSICmd *s);
#endif
int ASPI_Seek_COMP(SRB_ExecSCSICmd *s);
#ifdef GENS_OS_WIN32
int ASPI_Read_CD_LBA_COMP(SRB_ExecSCSICmd *s);
int ASPI_Read_CD_MSF_COMP(SRB_ExecSCSICmd *s);
#endif

// Customize Callback

int ASPI_Stop_CDD_c1_COMP(SRB_ExecSCSICmd *s);
int ASPI_Fast_Seek_COMP(SRB_ExecSCSICmd *s);
int ASPI_Seek_CDD_c4_COMP(SRB_ExecSCSICmd *s);
int ASPI_Close_Tray_CDD_cC_COMP(SRB_ExecSCSICmd *s);
int ASPI_Open_Tray_CDD_cD_COMP(SRB_ExecSCSICmd *s);


// CDC functions

void ASPI_Flush_Cache_CDC(void);
void ASPI_Read_One_LBA_CDC(void);
#ifdef GENS_OS_WIN32
int ASPI_Read_One_CDC_COMP(SRB_ExecSCSICmd *s);
int ASPI_Read_One_CDC_Cache(void);
#endif
void Wait_Read_Complete(void);


// Specials functions

void Fill_SCD_TOC_from_MSF_TOC(void);
void Fill_SCD_TOC_Zero(void);


#ifdef __cplusplus
}
#endif

#endif /* GENS_CD_ASPI_HPP */
