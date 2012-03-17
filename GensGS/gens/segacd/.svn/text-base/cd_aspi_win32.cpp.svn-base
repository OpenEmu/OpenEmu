#include <cstring>
#include <cstdio>
#include <climits>

#include <windows.h>

#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"
#include "util/file/rom.hpp"
#include "gens_core/cpu/68k/star_68k.h"
#include "lc89510.h"
#include "cd_aspi.hpp"
#include "gens_core/mem/mem_s68k.h"


static HINSTANCE hASPI_DLL = NULL;               // Handle to DLL
DWORD (*Get_ASPI_Info)(void);
DWORD (*Get_ASPI_Version)(void);
DWORD (*Send_ASPI_Command)(LPSRB);
int ASPI_Command_Running;
int DEV_PAR[8][3];
unsigned int Current_LBA;
unsigned char Buf_Stat[256];
SRB_ExecSCSICmd se;
TOC toc;


// External variables
int ASPI_Initialized = 0;	// If ASPI is initialized, this is set.
int cdromSpeed;			// Speed of the CD-ROM drive. (TODO: Is this really necessary?)
int Num_CD_Drive;		// Number of CD-ROM drives detected. (TODO: Is this correct?)
int cdromDeviceID;		// CD-ROM device ID


// for CDC functions

int Sectors_In_Cache = 0;
int Read_Complete = 1;
unsigned char Buf_Read[2366 * 64];
SRB_ExecSCSICmd sread;


// WNASPI32.dll version numbers
static const unsigned int WNASPI32_VERSION_WIN98SE		= 0x00000001;
static const unsigned int WNASPI32_VERSION_ADAPTEC_471a2	= 0x00003C04;
static const unsigned int WNASPI32_VERSION_NERO_2_01_50		= 0xAF010002;


int ASPI_Init(void)
{
	unsigned int ASPI_Status;
	
	// Clear the ASPI variables.
	ASPI_Command_Running = 0;
	Num_CD_Drive = 0;
	Current_LBA = 0;
	
	// Clear the TOC and read buffer.
	memset(&toc, 0x00, sizeof(toc));
	memset(Buf_Read, 0x00, sizeof(Buf_Read));
	
	// Clear the ASPI pointers.
	Get_ASPI_Info = NULL;
	Get_ASPI_Version = NULL;
	Send_ASPI_Command = NULL;
	
	// Attempt to load the ASPI DLL.
	ASPI_Initialized = 0;
	hASPI_DLL = LoadLibrary("wnaspi32.dll");
	if (hASPI_DLL)
	{
		// ASPI loaded.
		Get_ASPI_Info = (DWORD(*)(void))GetProcAddress(hASPI_DLL, "GetASPI32SupportInfo");
		Get_ASPI_Version = (DWORD(*)(void))GetProcAddress(hASPI_DLL, "GetASPI32DLLVersion");
		Send_ASPI_Command = (DWORD(*)(LPSRB lpsrb))GetProcAddress(hASPI_DLL, "SendASPI32Command");
	}
	
	if (!Get_ASPI_Info || !Get_ASPI_Version || !Send_ASPI_Command)
	{
		if (hASPI_DLL)
		{
			FreeLibrary(hASPI_DLL);
			hASPI_DLL = NULL;
		}
		
		Get_ASPI_Info = NULL;
		Get_ASPI_Version = NULL;
		Send_ASPI_Command = NULL;
		
		// MessageBox(NULL, "Error loading WNASPI32.DLL\nCD device will not be supported", "ASPI error", MB_ICONSTOP);
#ifdef DEBUG_CD
		fprintf(debug_SCD_file, "error : can't load WNASPI32.DLL\n\n");
#endif
		return 0;
	}
	
	ASPI_Status = Get_ASPI_Info();
	
	switch (HIBYTE(ASPI_Status))
	{
		case SS_COMP:
			// ASPI is initialized.
			Num_CD_Drive = LOBYTE(ASPI_Status);
			ASPI_Initialized = 1;
#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "ASPI driver initialised : %d device(s) detected\n\n", Num_CD_Drive);
#endif
			break;
		
		case SS_NO_ASPI:
#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "error : no ASPI managers were found\n\n");
#endif
			return 0;
			break;

		case SS_ILLEGAL_MODE:
#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "error : ASPI for Windows does not support this mode\n\n");
#endif
			return 0;
			break;

		case SS_OLD_MANAGER:
#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "error : an ASPI manager which does not support Windows is resident\n\n");
#endif
			return 0;
			break;
		
		default:
#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "error : ASPI for Windows is not initialized\n\n");
#endif
			return 0;
			break;
	}
	
	if (Num_CD_Drive > 8)
		Num_CD_Drive = 8;
	
	ASPI_Scan_Drives();
//	ASPI_Set_Timeout(10);			// crashe sur certaines machines
	
	return Num_CD_Drive;
}


int ASPI_End(void)
{
	int i = 0;

	if (hASPI_DLL)
	{
		while ((ASPI_Command_Running == 1) && (i++ < 3000)) Sleep(1);
		ASPI_Star_Stop_Unit(STOP_DISC, 0, 0, NULL);
		FreeLibrary(hASPI_DLL);

#ifdef DEBUG_CD
		fprintf(debug_SCD_file, "ASPI driver unloaded\n");
#endif
	}

	Get_ASPI_Info = NULL;
	Get_ASPI_Version = NULL;
	Send_ASPI_Command = NULL;
	Num_CD_Drive = 0;

	return 1;
}


void ASPI_Reset_Drive(char *buf)
{
	Read_Complete = 1;

	ASPI_Star_Stop_Unit(STOP_DISC, 0, 0, NULL);
	ASPI_Star_Stop_Unit(CLOSE_TRAY, 0, 0, NULL);
	ASPI_Test_Unit_Ready(7000);
	ASPI_Star_Stop_Unit(START_DISC, 0, 0, NULL);

	// Fill the TOC and set the CD_Present flag
	ASPI_Read_TOC(1, 0, 0, 0, NULL);

	if (CD_Present)
	{
		while (ASPI_Read_CD_LBA(0, 1, 0, 0, 0, 0, NULL) == -1);
	}
	else ASPI_Read_CD_LBA(0, 1, 0, 0, 0, 0, NULL);

	memcpy(buf, &Buf_Read[0x100], 0x200);
}


void ASPI_Scan_Drives(void)
{
	SRB_HaInquiry sh;
	SRB_GDEVBlock sd;
#ifdef DEBUG_CD
	char str[512];
#endif
	int i, j, k, maxTgt;

	for(i = 0; i < 8; i++)
	{
		DEV_PAR[i][0] = 0;
		DEV_PAR[i][1] = 0;
		DEV_PAR[i][2] = 0;
	}

#ifdef DEBUG_CD
		fprintf(debug_SCD_file, "Start Scanning :\n");
#endif

	for(k = 0, i = 0; i < Num_CD_Drive; i++)
	{
		memset(&sh, 0, sizeof(sh));
		sh.SRB_Cmd   = SC_HA_INQUIRY;
		sh.SRB_HaId  = i;

		Send_ASPI_Command((LPSRB) &sh);

		if (sh.SRB_Status != SS_COMP)
		continue;

		maxTgt = (int) sh.HA_Unique[3];

		if ((maxTgt != 8) && (maxTgt != 16)) maxTgt = 8;

#ifdef DEBUG_CD
		fprintf(debug_SCD_file, "HaID = %d, max target = %d\n", i, maxTgt);
#endif

		for(j = 0; j < maxTgt; j++)
		{
			memset(&sd, 0, sizeof(sd));
			sd.SRB_Cmd    = SC_GET_DEV_TYPE; 
			sd.SRB_HaId   = i;
			sd.SRB_Target = j;

			Send_ASPI_Command((LPSRB) &sd);
			if (sd.SRB_Status == SS_COMP)
			{
#ifdef DEBUG_CD
				fprintf(debug_SCD_file, "Found device at %d:%d\n", i, j);
#endif

				if (sd.SRB_DeviceType == DTYPE_CDROM)
				{
#ifdef DEBUG_CD
					fprintf(debug_SCD_file, "Accepted %d -> %d:%d\n", i, sd.SRB_HaId, sd.SRB_Target);
#endif

					DEV_PAR[k][0] = i;
					DEV_PAR[k][1] = j;
					DEV_PAR[k][2] = 0;
					k++;
				}
			}
		}
	}
	
	Num_CD_Drive = k;
	
#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "%d drive(s) validated\n", Num_CD_Drive);
#endif
}


int ASPI_Get_Drive_Info(int dev, unsigned char *Inf)
{
	SRB_ExecSCSICmd s;

	if ((dev + 1) > Num_CD_Drive) return 5;

	memset(&s, 0, sizeof(s));
	memset(Inf, 0, 100);

	s.SRB_Cmd        = SC_EXEC_SCSI_CMD;
	s.SRB_HaId       = DEV_PAR[dev][0];
	s.SRB_Target     = DEV_PAR[dev][1];
	s.SRB_Lun        = DEV_PAR[dev][2];
	s.SRB_Flags      = SRB_DIR_IN;
	s.SRB_BufLen     = 100;
	s.SRB_BufPointer = Inf;
	s.SRB_SenseLen   = SENSE_LEN;
	s.SRB_CDBLen     = 6;
	s.SRB_PostProc   = NULL;
	s.CDBByte[0]     = SCSI_INQUIRY;
	s.CDBByte[4]     = 100;

	Send_ASPI_Command((LPSRB) &s);

	while (s.SRB_Status == SS_PENDING) Sleep(1);

	if (s.SRB_Status == SS_COMP)
	{
		Inf[36] = 0;
		return 0;
	}

	return -1;
}


int ASPI_Set_Timeout(int sec)
{
	struct		// Timeout commande structure
	{
		BYTE SRB_Cmd;
		BYTE SRB_Status;
		BYTE SRB_HaId;
		BYTE SRB_Flags;
		DWORD SRB_Hdr_Rsvd;
		BYTE SRB_Target;
		BYTE SRB_Lun;
		DWORD SRB_Timeout;
	} s;

	if ((cdromDeviceID + 1) > Num_CD_Drive)
		return 5;

	memset(&s, 0, sizeof(s));

	s.SRB_Cmd        = SC_GETSET_TIMEOUTS;
	s.SRB_HaId       = 0xFF;
	s.SRB_Target     = 0xFF;
	s.SRB_Lun        = 0xFF;
	s.SRB_Flags      = SRB_DIR_OUT;
	s.SRB_Timeout    = sec * 2;

	Send_ASPI_Command((LPSRB) &s);

	while (s.SRB_Status == SS_PENDING) Sleep(1);

	if (s.SRB_Status == SS_COMP) return 0;
	else return -1;
}


int ASPI_Test_Unit_Ready(int timeout)
{
	SRB_ExecSCSICmd s;
	int wait_time = 0;

	if ((cdromDeviceID + 1) > Num_CD_Drive)
		return 5;

	while (wait_time < timeout)
	{
		memset(&s, 0, sizeof(s));

		s.SRB_Cmd        = SC_EXEC_SCSI_CMD;
		s.SRB_HaId       = DEV_PAR[cdromDeviceID][0];
		s.SRB_Target     = DEV_PAR[cdromDeviceID][1];
		s.SRB_Lun        = DEV_PAR[cdromDeviceID][2];
		s.SRB_Flags      = SRB_DIR_IN;
		s.SRB_BufLen     = 0;
		s.SRB_BufPointer = NULL;
		s.SRB_SenseLen   = SENSE_LEN;
		s.SRB_CDBLen     = 6;
		s.SRB_PostProc   = NULL;

		s.CDBByte[0]     = SCSI_TST_U_RDY;

		Send_ASPI_Command((LPSRB) &s);

		while (s.SRB_Status == SS_PENDING)
		{
			wait_time++;
			Sleep(1);
		}

		if (s.SRB_Status == SS_COMP)
		{
			if (s.SRB_TargStat == STATUS_GOOD) return 0;
		}

		wait_time += 10;
		Sleep(10);
	}

	if (s.SRB_Status == SS_COMP) return 2;
	else return -1;
}


int ASPI_Set_CD_Speed(int rate, int wait)
{
	SRB_ExecSCSICmd s;

	if ((cdromDeviceID + 1) > Num_CD_Drive)
		return 5;

	memset(&s, 0, sizeof(s));

	s.SRB_Cmd        = SC_EXEC_SCSI_CMD;
	s.SRB_HaId       = DEV_PAR[cdromDeviceID][0];
	s.SRB_Target     = DEV_PAR[cdromDeviceID][1];
	s.SRB_Lun        = DEV_PAR[cdromDeviceID][2];
	s.SRB_Flags      = SRB_DIR_IN;
	s.SRB_BufLen     = 0;
	s.SRB_BufPointer = NULL;
	s.SRB_SenseLen   = SENSE_LEN;
	s.SRB_CDBLen     = 12;
	s.SRB_PostProc   = NULL;

	s.CDBByte[0]     = SCSI_SET_SPEED;

	rate &= 0xFFFF;
	
	s.CDBByte[2]     = rate >> 8;
	s.CDBByte[3]     = rate & 0xFF;

	Send_ASPI_Command((LPSRB) &s);

	if (wait == 0) return 0;
	
	while (s.SRB_Status == SS_PENDING) Sleep(1);

	if (s.SRB_Status == SS_COMP)
	{
		if (s.SRB_TargStat == STATUS_GOOD) return 0;
		else return 2;
	}

	return -1;
}


int ASPI_Lock(int flock)
{
	SRB_ExecSCSICmd s;

	if ((cdromDeviceID + 1) > Num_CD_Drive)
		return 5;

	memset(&s, 0, sizeof(s));

	s.SRB_Cmd        = SC_EXEC_SCSI_CMD;
	s.SRB_HaId       = DEV_PAR[cdromDeviceID][0];
	s.SRB_Target     = DEV_PAR[cdromDeviceID][1];
	s.SRB_Lun        = DEV_PAR[cdromDeviceID][2];
	s.SRB_Flags      = SRB_DIR_IN;
	s.SRB_BufLen     = 0;
	s.SRB_BufPointer = NULL;
	s.SRB_SenseLen   = SENSE_LEN;
	s.SRB_CDBLen     = 6;
	s.SRB_PostProc   = NULL;

	s.CDBByte[0]     = SCSI_MED_REMOVL;
	s.CDBByte[4]     = flock & 1;

	Send_ASPI_Command((LPSRB) &s);

	while (s.SRB_Status == SS_PENDING) Sleep(1);

	return 0;
}


int ASPI_Star_Stop_Unit(int op, int imm, int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *))
{
	SRB_ExecSCSICmd s_loc, *s;

	if ((cdromDeviceID + 1) > Num_CD_Drive)
		return 5;

	if (async && IsAsyncAllowed())
	{
		if (ASPI_Command_Running) return 1;
		ASPI_Command_Running = 1;
		s = &se;
	}
	else s = &s_loc;

	if (PostProc == NULL) PostProc = ASPI_Star_Stop_Unit_COMP;

	memset(s, 0, sizeof(s_loc));

	s->SRB_Cmd        = SC_EXEC_SCSI_CMD;
	s->SRB_HaId       = DEV_PAR[cdromDeviceID][0];
	s->SRB_Target     = DEV_PAR[cdromDeviceID][1];
	s->SRB_Lun        = DEV_PAR[cdromDeviceID][2];
	s->SRB_Flags      = SRB_DIR_IN;
	s->SRB_BufLen     = 0;
	s->SRB_BufPointer = NULL;
	s->SRB_SenseLen   = SENSE_LEN;
	s->SRB_CDBLen     = 6;

	if (async && IsAsyncAllowed())
	{
		s->SRB_Flags |= SRB_POSTING;
		s->SRB_PostProc = PostProc;
	}
	else s->SRB_PostProc = NULL;

	s->CDBByte[0]     = SCSI_START_STP;
	s->CDBByte[1]     = imm & 1;
	s->CDBByte[4]     = op;

	if (async && IsAsyncAllowed())
	{
		CDD_Complete = 0;
		Send_ASPI_Command((LPSRB) s);
		return 0;
	}
	else
	{
		Send_ASPI_Command((LPSRB) s);
		while (s->SRB_Status == SS_PENDING) Sleep(1);
		return PostProc(s);
	}
}


int ASPI_Read_TOC(int MSF, int format, int st, int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *))
{
	SRB_ExecSCSICmd s_loc, *s;

	if ((cdromDeviceID + 1) > Num_CD_Drive)
		return 5;

#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "Read TOC command pass 1 ");
#endif

	if (async && IsAsyncAllowed())
	{
		if (ASPI_Command_Running) return 1;
		ASPI_Command_Running = 1;
		s = &se;
	}
	else s = &s_loc;

#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "pass 2 ");
#endif

	if (PostProc == NULL)
	{
		if (MSF)
			PostProc = ASPI_Read_TOC_MSF_COMP;
		else
			PostProc = ASPI_Read_TOC_LBA_COMP;
	}

	memset(s, 0, sizeof(s_loc));
	memset(&toc, 0, sizeof(toc));

#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "pass 3 ");
#endif

	s->SRB_Cmd        = SC_EXEC_SCSI_CMD;
	s->SRB_HaId       = DEV_PAR[cdromDeviceID][0];
	s->SRB_Target     = DEV_PAR[cdromDeviceID][1];
	s->SRB_Lun        = DEV_PAR[cdromDeviceID][2];
	s->SRB_Flags      = SRB_DIR_IN;
	s->SRB_BufLen     = 0x324;
	s->SRB_BufPointer = (BYTE FAR *) &toc;
	s->SRB_SenseLen   = SENSE_LEN;
	s->SRB_CDBLen     = 10;

	if (async && IsAsyncAllowed())
	{
		s->SRB_Flags |= SRB_POSTING;
		s->SRB_PostProc = PostProc;
	}
	else s->SRB_PostProc = NULL;
	
	s->CDBByte[0]   = SCSI_READ_TOC;
	s->CDBByte[6]   = st;
	s->CDBByte[7]   = 0x03;         /* ofs. 7-8 used for toc len */
	s->CDBByte[8]   = 0x24;         /* TOC buffer length == 0x324 */

	if (MSF) s->CDBByte[1] = 0x02;
	else s->CDBByte[1] = 0x00;

/*	Don't use the format field for the moment	*/
/*	s->CDBByte[2] = format & 7;					*/
    
#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "pass 4 \n");
#endif

	if (async && IsAsyncAllowed())
	{
		CDD_Complete = 0;
		Send_ASPI_Command((LPSRB) s);
		return 0;
	}
	else
	{
		Send_ASPI_Command((LPSRB) s);
		while (s->SRB_Status == SS_PENDING) Sleep(1);
		return PostProc(s);
	}
}


int ASPI_Mechanism_State(int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *))
{
	SRB_ExecSCSICmd s_loc, *s;

	if ((cdromDeviceID + 1) > Num_CD_Drive)
		return 5;

	if (async && IsAsyncAllowed())
	{
		if (ASPI_Command_Running) return 1;
		ASPI_Command_Running = 1;
		s = &se;
	}
	else s = &s_loc;

	if (PostProc == NULL)
		PostProc = ASPI_Mechanism_State_COMP;

	memset(s, 0, sizeof(s_loc));
	memset(Buf_Stat, 0, 0x100);

	s->SRB_Cmd        = SC_EXEC_SCSI_CMD;
	s->SRB_HaId       = DEV_PAR[cdromDeviceID][0];
	s->SRB_Target     = DEV_PAR[cdromDeviceID][1];
	s->SRB_Lun        = DEV_PAR[cdromDeviceID][2];
	s->SRB_Flags      = SRB_DIR_IN;
	s->SRB_BufLen     = 0x100;
	s->SRB_BufPointer = Buf_Stat;
	s->SRB_SenseLen   = SENSE_LEN;
	s->SRB_CDBLen     = 12;

	if (async && IsAsyncAllowed())
	{
		s->SRB_Flags |= SRB_POSTING;
		s->SRB_PostProc = PostProc;
	}
	else s->SRB_PostProc = NULL;

	se.CDBByte[0]   = SCSI_GET_MCH_ST;
	se.CDBByte[8]   = 0x01;           /* buffer length */
	se.CDBByte[9]   = 0x00;           /* buffer length == 0x100 */

	if (async && IsAsyncAllowed())
	{
		CDD_Complete = 0;
		Send_ASPI_Command((LPSRB) s);
		return 0;
	}
	else
	{
		Send_ASPI_Command((LPSRB) s);
		while (s->SRB_Status == SS_PENDING) Sleep(1);
		return PostProc(s);
	}
}


int ASPI_Play_CD_MSF(_msf *start, _msf *end, int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *))
{
	SRB_ExecSCSICmd s_loc, *s;

	if ((cdromDeviceID + 1) > Num_CD_Drive)
		return 5;

	if (async && IsAsyncAllowed())
	{
		if (ASPI_Command_Running) return 1;
		ASPI_Command_Running = 1;
		s = &se;
	}
	else s = &s_loc;

	if (PostProc == NULL) PostProc = ASPI_Play_CD_MSF_COMP;

	memset(s, 0, sizeof(s_loc));

	s->SRB_Cmd        = SC_EXEC_SCSI_CMD;
	s->SRB_HaId       = DEV_PAR[cdromDeviceID][0];
	s->SRB_Target     = DEV_PAR[cdromDeviceID][1];
	s->SRB_Lun        = DEV_PAR[cdromDeviceID][2];
	s->SRB_Flags      = SRB_DIR_IN;
	s->SRB_BufLen     = 0;
	s->SRB_BufPointer = NULL;
	s->SRB_SenseLen   = SENSE_LEN;
	s->SRB_CDBLen     = 10;

	if (async && IsAsyncAllowed())
	{
		s->SRB_Flags |= SRB_POSTING;
		s->SRB_PostProc = PostProc;
	}
	else s->SRB_PostProc = NULL;

	s->CDBByte[0]     = SCSI_PLAYAUDMSF;
	s->CDBByte[3]     = start->M;
	s->CDBByte[4]     = start->S;
	s->CDBByte[5]     = start->F;

	if (end == NULL)
	{
		s->CDBByte[6] = 99;
		s->CDBByte[7] = 00;
		s->CDBByte[8] = 00;
	}
	else if ((end->M == 0) && (end->S == 0) && (end->F == 0))
	{
		s->CDBByte[6] = 99;
		s->CDBByte[7] = 00;
		s->CDBByte[8] = 00;
	}
	else
	{
		s->CDBByte[6] = end->M;
		s->CDBByte[7] = end->S;
		s->CDBByte[8] = end->F;
	}

	if (async && IsAsyncAllowed())
	{
		CDD_Complete = 0;
		Send_ASPI_Command((LPSRB) s);
		return 0;
	}
	else
	{
		Send_ASPI_Command((LPSRB) s);
		while (s->SRB_Status == SS_PENDING) Sleep(1);
		return PostProc(s);
	}
}


int ASPI_Stop_Play_Scan(int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *))
{
	SRB_ExecSCSICmd s_loc, *s;

	if ((cdromDeviceID + 1) > Num_CD_Drive)
		return 5;

	if (async && IsAsyncAllowed())
	{
		if (ASPI_Command_Running) return 1;
		ASPI_Command_Running = 1;
		s = &se;
	}
	else s = &s_loc;

	if (PostProc == NULL) PostProc = ASPI_Stop_Play_Scan_COMP;

	memset(s, 0, sizeof(s_loc));

	s->SRB_Cmd        = SC_EXEC_SCSI_CMD;
	s->SRB_HaId       = DEV_PAR[cdromDeviceID][0];
	s->SRB_Target     = DEV_PAR[cdromDeviceID][1];
	s->SRB_Lun        = DEV_PAR[cdromDeviceID][2];
	s->SRB_Flags      = SRB_DIR_IN;
	s->SRB_BufLen     = 0;
	s->SRB_BufPointer = NULL;
	s->SRB_SenseLen   = SENSE_LEN;
	s->SRB_CDBLen     = 10;

	if (async && IsAsyncAllowed())
	{
		s->SRB_Flags |= SRB_POSTING;
		s->SRB_PostProc = PostProc;
	}
	else s->SRB_PostProc = NULL;

	s->CDBByte[0]     = SCSI_STOP_PL_SC;

	if (async && IsAsyncAllowed())
	{
		CDD_Complete = 0;
		Send_ASPI_Command((LPSRB) s);
		return 0;
	}
	else
	{
		Send_ASPI_Command((LPSRB) s);
		while (s->SRB_Status == SS_PENDING) Sleep(1);
		return PostProc(s);
	}
}


int ASPI_Pause_Resume(int resume, int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *))
{
	SRB_ExecSCSICmd s_loc, *s;

	if ((cdromDeviceID + 1) > Num_CD_Drive)
		return 5;

	if (async && IsAsyncAllowed())
	{
		if (ASPI_Command_Running) return 1;
		ASPI_Command_Running = 1;
		s = &se;
	}
	else s = &s_loc;

	if (PostProc == NULL) PostProc = ASPI_Pause_Resume_COMP;

	memset(s, 0, sizeof(s_loc));

	s->SRB_Cmd        = SC_EXEC_SCSI_CMD;
	s->SRB_HaId       = DEV_PAR[cdromDeviceID][0];
	s->SRB_Target     = DEV_PAR[cdromDeviceID][1];
	s->SRB_Lun        = DEV_PAR[cdromDeviceID][2];
	s->SRB_Flags      = SRB_DIR_IN;
	s->SRB_BufLen     = 0;
	s->SRB_BufPointer = NULL;
	s->SRB_SenseLen   = SENSE_LEN;
	s->SRB_CDBLen     = 10;

	if (async && IsAsyncAllowed())
	{
		s->SRB_Flags |= SRB_POSTING;
		s->SRB_PostProc = PostProc;
	}
	else s->SRB_PostProc = NULL;

	s->CDBByte[0]     = SCSI_PAUSE_RESU;

	if (resume) s->CDBByte[8] = 1;
	else s->CDBByte[8] = 0;

	if (async && IsAsyncAllowed())
	{
		CDD_Complete = 0;
		Send_ASPI_Command((LPSRB) s);
		return 0;
	}
	else
	{
		Send_ASPI_Command((LPSRB) s);
		while (s->SRB_Status == SS_PENDING) Sleep(1);
		return PostProc(s);
	}
}


int ASPI_Seek(int pos, int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *))
{
	SRB_ExecSCSICmd s_loc, *s;

	if ((cdromDeviceID + 1) > Num_CD_Drive)
		return 5;

	if (async && IsAsyncAllowed())
	{
		if (ASPI_Command_Running) return 1;
		ASPI_Command_Running = 1;
		s = &se;
	}
	else s = &s_loc;

	if (PostProc == NULL) PostProc = ASPI_Seek_COMP;

	memset(s, 0, sizeof(s_loc));

	if (pos < 0) pos = 0;

	s->SRB_Cmd        = SC_EXEC_SCSI_CMD;
	s->SRB_HaId       = DEV_PAR[cdromDeviceID][0];
	s->SRB_Target     = DEV_PAR[cdromDeviceID][1];
	s->SRB_Lun        = DEV_PAR[cdromDeviceID][2];
	s->SRB_Flags      = SRB_DIR_IN;
	s->SRB_BufLen     = 0;
	s->SRB_BufPointer = NULL;
	s->SRB_SenseLen   = SENSE_LEN;
	s->SRB_CDBLen     = 10;

	if (async && IsAsyncAllowed())
	{
		s->SRB_Flags |= SRB_POSTING;
		s->SRB_PostProc = PostProc;
	}
	else s->SRB_PostProc = NULL;

	s->CDBByte[0]     = SCSI_SEEK10;
	s->CDBByte[2]   = pos >> 24;
	s->CDBByte[3]   = (pos >> 16) & 0xFF;
	s->CDBByte[4]   = (pos >> 8) & 0xFF;
	s->CDBByte[5]   = pos & 0xFF;
    
	if (async && IsAsyncAllowed())
	{
		CDD_Complete = 0;
		Send_ASPI_Command((LPSRB) s);
		return 0;
	}
	else
	{
		Send_ASPI_Command((LPSRB) s);
		while (s->SRB_Status == SS_PENDING) Sleep(1);
		return PostProc(s);
	}
}


int ASPI_Read_CD_LBA(int adr, int length, unsigned char sector, unsigned char flag, unsigned char sub_chan, int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *))
{
	SRB_ExecSCSICmd s_loc, *s;

	if ((cdromDeviceID + 1) > Num_CD_Drive)
		return 5;

	if (async && IsAsyncAllowed())
	{
		if (ASPI_Command_Running) return 1;
		ASPI_Command_Running = 1;
		s = &se;
	}
	else s = &s_loc;

	if (PostProc == NULL) PostProc = ASPI_Read_CD_LBA_COMP;

	if (length > 64 ) length = 64;
	else if (length <= 0) length = 1;
	if (adr < 0) adr = 0;

	memset(s, 0, sizeof(s_loc));

	s->SRB_Cmd        = SC_EXEC_SCSI_CMD;
	s->SRB_HaId       = DEV_PAR[cdromDeviceID][0];
	s->SRB_Target     = DEV_PAR[cdromDeviceID][1];
	s->SRB_Lun        = DEV_PAR[cdromDeviceID][2];
	s->SRB_Flags      = SRB_DIR_IN;
	s->SRB_BufLen     = length * 2366;
	s->SRB_BufPointer = Buf_Read;
	s->SRB_SenseLen   = SENSE_LEN;
	s->SRB_CDBLen     = 12;

#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "\n\nREAD CD LBA :\n Deb : %d   length : %d\n\n", adr, length);
#endif

	if (async && IsAsyncAllowed())
	{
		s->SRB_Flags |= SRB_POSTING;
		s->SRB_PostProc = PostProc;
	}
	else s->SRB_PostProc = NULL;

	s->CDBByte[0]      = SCSI_READ_LBA;
	s->CDBByte[1]      = sector;

	s->CDBByte[2]      = adr >> 24;
	s->CDBByte[3]      = (adr >> 16) & 0xFF;
	s->CDBByte[4]      = (adr >> 8) & 0xFF;
	s->CDBByte[5]      = adr & 0xFF;

	s->CDBByte[6]      = (length >> 16) & 0xFF;
	s->CDBByte[7]      = (length >> 8) & 0xFF;
	s->CDBByte[8]      = length & 0xFF;

//	s->CDBByte[9]      = flag;
	s->CDBByte[9]      = 0x10;           // this works in almost case 
//	s->CDBByte[9]      = 0xF8;           // retrieve all data and field
	s->CDBByte[10]     = sub_chan;
    
	if (async && IsAsyncAllowed())
	{
		CDD_Complete = 0;
		Send_ASPI_Command((LPSRB) s);
		return 0;
	}
	else
	{
		Send_ASPI_Command((LPSRB) s);
		while (s->SRB_Status == SS_PENDING) Sleep(1);
		return PostProc(s);
	}
}


int ASPI_Read_One_CD_LBA(int adr, unsigned char flag, unsigned char sub_chan, int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *))
{
	SRB_ExecSCSICmd s_loc, *s;

	if ((cdromDeviceID + 1) > Num_CD_Drive)
		return 5;

	if (async && IsAsyncAllowed())
	{
		if (ASPI_Command_Running) return 1;
		ASPI_Command_Running = 1;
		s = &se;
	}
	else s = &s_loc;

	if (PostProc == NULL) PostProc = ASPI_Read_CD_LBA_COMP;

	if (adr < 0) adr = 0;

	memset(s, 0, sizeof(s_loc));

	s->SRB_Cmd        = SC_EXEC_SCSI_CMD;
	s->SRB_HaId       = DEV_PAR[cdromDeviceID][0];
	s->SRB_Target     = DEV_PAR[cdromDeviceID][1];
	s->SRB_Lun        = DEV_PAR[cdromDeviceID][2];
	s->SRB_Flags      = SRB_DIR_IN;
	s->SRB_BufLen     = 2366;
	s->SRB_BufPointer = Buf_Read;
	s->SRB_SenseLen   = SENSE_LEN;
	s->SRB_CDBLen     = 12;

#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "\n\nREAD One CD LBA :\n Deb : %d\n\n", adr);
#endif

	if (async && IsAsyncAllowed())
	{
		s->SRB_Flags |= SRB_POSTING;
		s->SRB_PostProc = PostProc;
	}
	else s->SRB_PostProc = NULL;

	s->CDBByte[0]      = SCSI_READ_LBA;

	s->CDBByte[2]      = adr >> 24;
	s->CDBByte[3]      = (adr >> 16) & 0xFF;
	s->CDBByte[4]      = (adr >> 8) & 0xFF;
	s->CDBByte[5]      = adr & 0xFF;

	s->CDBByte[8]      = 1;

//	s->CDBByte[9]      = flag;
	s->CDBByte[9]      = 0x10;           // this works in almost case 
	s->CDBByte[10]     = sub_chan;
    
	if (async && IsAsyncAllowed())
	{
		CDD_Complete = 0;
		Send_ASPI_Command((LPSRB) s);
		return 0;
	}
	else
	{
		Send_ASPI_Command((LPSRB) s);
		while (s->SRB_Status == SS_PENDING) Sleep(1);
		return PostProc(s);
	}
}


int ASPI_Read_CD_MSF(_msf *start, _msf *end, unsigned char sector, unsigned char flag, unsigned char sub_chan, int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *))
{
	SRB_ExecSCSICmd s_loc, *s;
	_msf MSF_S, MSF_E;

	if ((cdromDeviceID + 1) > Num_CD_Drive)
		return 5;

	if (async && IsAsyncAllowed())
	{
		if (ASPI_Command_Running) return 1;
		ASPI_Command_Running = 1;
		s = &se;
	}
	else s = &s_loc;

	if (PostProc == NULL) PostProc = ASPI_Read_CD_MSF_COMP;

	if (start == NULL) memset(&MSF_S, 0, sizeof(MSF_S));
	else memcpy(&MSF_S, start, sizeof(MSF_S));

	if (end == NULL)
	{
		LBA_to_MSF(MSF_to_LBA(&MSF_S) + 1, &MSF_E);
	}
	else
	{
		memcpy(&MSF_E, end, sizeof(MSF_E));

		if ((MSF_to_LBA(&MSF_E) - MSF_to_LBA(&MSF_S)) > 7)
		{
			LBA_to_MSF(MSF_to_LBA(&MSF_S) + 7, &MSF_E);
		}
		else if ((MSF_to_LBA(&MSF_E) - MSF_to_LBA(&MSF_S)) <= 0)
		{
			LBA_to_MSF(MSF_to_LBA(&MSF_S) + 1, &MSF_E);
		}
	}

#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "\n\nREAD CD MSF :\n Deb : %d:%d:%d   end %d:%d:%d\n\n", MSF_S.M, MSF_S.S, MSF_S.F, MSF_E.M, MSF_E.S, MSF_E.F);
#endif

	memset(s, 0, sizeof(s_loc));

	s->SRB_Cmd        = SC_EXEC_SCSI_CMD;
	s->SRB_HaId       = DEV_PAR[cdromDeviceID][0];
	s->SRB_Target     = DEV_PAR[cdromDeviceID][1];
	s->SRB_Lun        = DEV_PAR[cdromDeviceID][2];
	s->SRB_Flags      = SRB_DIR_IN;
	s->SRB_BufLen     = 8 * 2366;
	s->SRB_BufPointer = Buf_Read;
	s->SRB_SenseLen   = SENSE_LEN;
	s->SRB_CDBLen     = 12;

	if (async && IsAsyncAllowed())
	{
		s->SRB_Flags |= SRB_POSTING;
		s->SRB_PostProc = PostProc;
	}
	else s->SRB_PostProc = NULL;

	s->CDBByte[0]      = SCSI_READ_MSF;
	s->CDBByte[1]      = sector;

	s->CDBByte[3]      = MSF_S.M;
	s->CDBByte[4]      = MSF_S.S;
	s->CDBByte[5]      = MSF_S.F;

	s->CDBByte[6]      = MSF_E.M;
	s->CDBByte[7]      = MSF_E.S;
	s->CDBByte[8]      = MSF_E.F;

//	s->CDBByte[9]      = flag;
	s->CDBByte[9]      = 0x10;           // this works in almost case 
//	s->CDBByte[9]      = 0xF8;           // retrieve all data and field
	s->CDBByte[10]     = sub_chan;
   
	if (async && IsAsyncAllowed())
	{
		CDD_Complete = 0;
		Send_ASPI_Command((LPSRB) s);
		return 0;
	}
	else
	{
		Send_ASPI_Command((LPSRB) s);
		while (s->SRB_Status == SS_PENDING) Sleep(1);
		return PostProc(s);
	}
}


int ASPI_Read_One_CD_MSF(_msf *start, unsigned char flag, unsigned char sub_chan, int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *))
{
	SRB_ExecSCSICmd s_loc, *s;
	_msf MSF_S, MSF_E;

	if ((cdromDeviceID + 1) > Num_CD_Drive)
		return 5;

	if (async && IsAsyncAllowed())
	{
		if (ASPI_Command_Running) return 1;
		ASPI_Command_Running = 1;
		s = &se;
	}
	else s = &s_loc;

	if (PostProc == NULL) PostProc = ASPI_Read_CD_MSF_COMP;

	if (start == NULL)
	{
		MSF_S.M = 0;
		MSF_S.S = 2;
		MSF_S.F = 0;

		MSF_E.M = 0;
		MSF_E.S = 2;
		MSF_E.F = 1;
	}
	else
	{
		MSF_S.M = start->M;
		MSF_S.S = start->S;
		MSF_S.F = start->F;

		MSF_E.M = MSF_S.M;
		MSF_E.S = MSF_S.S;
		MSF_E.F = MSF_S.F + 1;

		if (MSF_E.F > 74)
		{
			MSF_E.F = 0;
			MSF_E.S++;

			if (MSF_E.S > 59)
			{
				MSF_E.S = 0;
				MSF_E.M++;
			}
		}
	}

#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "\n\nREAD One CD MSF :\n Deb : %d:%d:%d\n\n", MSF_S.M, MSF_S.S, MSF_S.F);
#endif

	memset(s, 0, sizeof(s_loc));

	s->SRB_Cmd        = SC_EXEC_SCSI_CMD;
	s->SRB_HaId       = DEV_PAR[cdromDeviceID][0];
	s->SRB_Target     = DEV_PAR[cdromDeviceID][1];
	s->SRB_Lun        = DEV_PAR[cdromDeviceID][2];
	s->SRB_Flags      = SRB_DIR_IN;
	s->SRB_BufLen     = 2366;
	s->SRB_BufPointer = Buf_Read;
	s->SRB_SenseLen   = SENSE_LEN;
	s->SRB_CDBLen     = 12;

	if (async && IsAsyncAllowed())
	{
		s->SRB_Flags |= SRB_POSTING;
		s->SRB_PostProc = PostProc;
	}
	else s->SRB_PostProc = NULL;

	s->CDBByte[0]      = SCSI_READ_MSF;

	s->CDBByte[3]      = MSF_S.M;
	s->CDBByte[4]      = MSF_S.S;
	s->CDBByte[5]      = MSF_S.F;

	s->CDBByte[6]      = MSF_E.M;
	s->CDBByte[7]      = MSF_E.S;
	s->CDBByte[8]      = MSF_E.F;

//	s->CDBByte[9]      = flag;
	s->CDBByte[9]      = 0x10;           // this works in almost case 
	s->CDBByte[10]     = sub_chan;
   
	if (async && IsAsyncAllowed())
	{
		CDD_Complete = 0;
		Send_ASPI_Command((LPSRB) s);
		return 0;
	}
	else
	{
		Send_ASPI_Command((LPSRB) s);
		while (s->SRB_Status == SS_PENDING) Sleep(1);
		return PostProc(s);
	}
}


/*

int ASPI_Get_Position(int async, void (*PostProc) (struct tagSRB32_ExecSCSICmd))
{
	s.SRB_Cmd        = SC_EXEC_SCSI_CMD;
	s.SRB_Flags      = SRB_DIR_IN | SRB_EVENT_NOTIFY;
	s.SRB_HaId       = HaId;
	s.SRB_Target     = TaId;
	s.SRB_Lun        = Lun;
	s.SRB_BufLen     = 0x10;
	s.SRB_BufPointer = (BYTE FAR *) &test;
	s.SRB_SenseLen   = 0x0E;
	s.SRB_CDBLen     = 0x0A;
	s.SRB_PostProc   = (LPVOID) hEvent;
	s.CDBByte[0]     = SCSI_SUBCHANNEL;
	s.CDBByte[1]     = 0x02;
	s.CDBByte[2]     = 64;
	s.CDBByte[3]     = 0x01;
	s.CDBByte[7]     = 0x0;
	s.CDBByte[8]     = 0x10;
              
	dwStatus = SendASPI32Command ((LPSRB) &s);
  
	if (dwStatus == SS_PENDING)
	{
		WaitForSingleObject(hEvent,INFINITE);
	}

	if (s.SRB_Status != SS_COMP)
	{
		//print_message("ASPI: Error getting subcodes");
	}

	curPos[0] = test[13];
	curPos[1] = test[14];
	curPos[2] = test[15];
}



int ASPI_Get_Position(int async, void (*PostProc) (struct tagSRB32_ExecSCSICmd))
{
	SRB_ExecSCSICmd s_loc, *s;

	if ((cdromDeviceID + 1) > Num_CD_Drive)
		return 5;

	if (async && IsAsyncAllowed())
	{
		if (ASPI_Command_Running) return 1;
		ASPI_Command_Running = 1;
		s = &se;
	}
	else s = &s_loc;

	if (PostProc == NULL) PostProc = ASPI_Get_Position_COMP;

	memset(s, 0, sizeof(s_loc));

	s->SRB_Cmd        = SC_EXEC_SCSI_CMD;
	s->SRB_HaId       = DEV_PAR[cdromDeviceID][0];
	s->SRB_Target     = DEV_PAR[cdromDeviceID][1];
	s->SRB_Lun        = DEV_PAR[cdromDeviceID][2];
	s->SRB_Flags      = SRB_DIR_IN;
	s->SRB_BufLen     = 0;
	s->SRB_BufPointer = NULL;
	s->SRB_SenseLen   = SENSE_LEN;
	s->SRB_CDBLen     = 12;

	if (async && IsAsyncAllowed())
	{
		s->SRB_Flags |= SRB_POSTING;
		s->SRB_PostProc = PostProc;
	}
	else s->SRB_PostProc = NULL;

	s->CDBByte[0]   = 0x34;
   
	if (async && IsAsyncAllowed())
	{
		CDD_Complete = 0;
		Send_ASPI_Command((LPSRB) s);
		return 0;
	}
	else
	{
		Send_ASPI_Command((LPSRB) s);
		while (s->SRB_Status == SS_PENDING) Sleep(1);
		return *PostProc((LPSRB) s);
	}
}
*/




/*********************************
 *	Default Callbacks functions  *
 *********************************/


int ASPI_Star_Stop_Unit_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
		if (s->SRB_TargStat == STATUS_GOOD) return 0;
		else return 2;
	}

	return -1;
}


int ASPI_Read_TOC_LBA_COMP(SRB_ExecSCSICmd *s)
{
#ifdef DEBUG_CD
	TOCTRACK *t;
	unsigned char i, numTracks;
	unsigned int Cur;
#endif

	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
#ifdef DEBUG_CD
		fprintf(debug_SCD_file, "Table of Content :\n\n");
		fprintf(debug_SCD_file, "First track: %d\n", toc.firstTrack);
		fprintf(debug_SCD_file, "Last track: %d\n", toc.lastTrack);

		numTracks = toc.lastTrack - toc.firstTrack + 1;

		for(i = 0; i < numTracks; i++)
		{
			t = &(toc.tracks[i]);

			fprintf(debug_SCD_file, "Track %d: ", t->trackNumber);
			if (t->ADR & 0x04) printf("(DATA) ");
			else printf("(AUDIO) ");

			MSB2DWORD(&Cur, t->addr);
			Cur /= 75;

			fprintf(debug_SCD_file, "Length %02d:%02d\n", Cur / 60, Cur % 60);
		}

		fprintf(debug_SCD_file, "\nEnd Table of Content\n\n");
#endif

		return 0;
	}
	else
	{
#ifdef DEBUG_CD
		fprintf(debug_SCD_file, "READ TOC COMP PAS OK :(\n");
#endif
	}

	return -1;
}


int ASPI_Read_TOC_MSF_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
		Fill_SCD_TOC_from_MSF_TOC();
		return 0;
	}
	else
	{
		Fill_SCD_TOC_Zero();
		return -1;
	}
}


int ASPI_Mechanism_State_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;
	char crut[1024], crut1[128];

	if (s->SRB_Status == SS_COMP)
	{
		sprintf(crut, "Success :\nChanger state : %d  Slot : %d\n", (Buf_Stat[0] >> 5) & 3, Buf_Stat[0] & 0x1F);
		sprintf(crut1, "Open : %d  Mecha state : %d\n", (Buf_Stat[1] >> 4) & 1, Buf_Stat[1] & 0xF);
		strcat(crut, crut1);
		sprintf(crut1, "Disc : %d\n", (Buf_Stat[8] >> 7) & 1);
		MessageBox(NULL, crut, "", MB_OK);

		return 0;
	}

	sprintf(crut, "Error :\nChanger state : %d  Slot : %d\n", (Buf_Stat[0] >> 5) & 3, Buf_Stat[0] & 0x1F);
	sprintf(crut1, "Open : %d  Mecha state : %d\n", (Buf_Stat[1] >> 4) & 1, Buf_Stat[1] & 0xF);
	strcat(crut, crut1);
	sprintf(crut1, "Disc : %d\n", (Buf_Stat[8] >> 7) & 1);
	MessageBox(NULL, crut, "", MB_OK);

	return -1;
}


int ASPI_Play_CD_MSF_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
		if (s->SRB_TargStat == STATUS_GOOD) return 0;
		else return 2;
	}

	return -1;
}


int ASPI_Stop_Play_Scan_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
		 return 0;
	}

	return -1;
}


int ASPI_Pause_Resume_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
		if (s->SRB_TargStat == STATUS_GOOD) return 0;
		else return 2;
	}

	return -1;
}


int ASPI_Seek_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
		 return 0;
	}

	return -1;
}


int ASPI_Read_CD_LBA_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
#ifdef DEBUG_CD
		fprintf(debug_SCD_file, "\ngood\n%s\n", &Buf_Read[32]);
#endif
		if (s->SRB_TargStat == STATUS_GOOD) return 0;
		else return 2;
	}

#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "\nerror\n");
#endif

	return -1;
}


int ASPI_Read_CD_MSF_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
		if (s->SRB_TargStat == STATUS_GOOD) return 0;
		else return 2;
	}

	return -1;
}


 
 
/********************************
 *	Custom Callbacks functions  *
 ********************************/


int ASPI_Stop_CDD_c1_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (CD_Present) SCD.Status_CDD = STOPPED;
	else SCD.Status_CDD = NOCD;
	CDD.Status = 0x0000;

	CDD.Control |= 0x0100;			// Data bit set because stopped

	CDD.Minute = 0;
	CDD.Seconde = 0;
	CDD.Frame = 0;
	CDD.Ext = 0;

	CDD_Complete = 1;
	return 0;
}


int ASPI_Total_Length_CDD_c23_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
		CDD.Status &= 0xFF;
//		if (!(CDC.CTRL.B.B0 & 0x80)) CDD.Status |= SCD.Status_CDD;
		CDD.Status |= SCD.Status_CDD;

		Fill_SCD_TOC_from_MSF_TOC();

		CDD.Minute = INT_TO_BCDW(SCD.TOC.Tracks[SCD.TOC.Last_Track - SCD.TOC.First_Track + 1].MSF.M);
		CDD.Seconde = INT_TO_BCDW(SCD.TOC.Tracks[SCD.TOC.Last_Track - SCD.TOC.First_Track + 1].MSF.S);
		CDD.Frame = INT_TO_BCDW(SCD.TOC.Tracks[SCD.TOC.Last_Track - SCD.TOC.First_Track + 1].MSF.F);
		CDD.Ext = 0;

		CDD_Complete = 1;
		return 0;
	}
	else
	{
		SCD.Status_CDD = NOCD;
		CDD.Status = (CDD.Status & 0xFF) | SCD.Status_CDD;

		Fill_SCD_TOC_Zero();

		CDD.Minute = 0;
		CDD.Seconde = 0;
		CDD.Frame = 0;
		CDD.Ext = 0;

		CDD_Complete = 1;
		return -1;
	}
}


int ASPI_First_Last_Track_CDD_c24_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
		CDD.Status &= 0xFF;
//		if (!(CDC.CTRL.B.B0 & 0x80)) CDD.Status |= SCD.Status_CDD;
		CDD.Status |= SCD.Status_CDD;

		Fill_SCD_TOC_from_MSF_TOC();

		CDD.Minute = INT_TO_BCDW(SCD.TOC.First_Track);
		CDD.Seconde = INT_TO_BCDW(SCD.TOC.Last_Track);
		CDD.Frame = 0;
		CDD.Ext = 0;

		CDD_Complete = 1;
		return 0;
	}
	else
	{
		SCD.Status_CDD = NOCD;
		CDD.Status = (CDD.Status & 0xFF) | SCD.Status_CDD;

		Fill_SCD_TOC_Zero();

		CDD.Minute = 0;
		CDD.Seconde = 0;
		CDD.Frame = 0;
		CDD.Ext = 0;

		CDD_Complete = 1;
		return -1;
	}
}


int ASPI_Track_Adr_CDD_c25_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
		CDD.Status &= 0xFF;
//		if (!(CDC.CTRL.B.B0 & 0x80)) CDD.Status |= SCD.Status_CDD;
		CDD.Status |= SCD.Status_CDD;

		Fill_SCD_TOC_from_MSF_TOC();

		if (track_number > SCD.TOC.Last_Track) track_number = SCD.TOC.Last_Track;
		else if (track_number < SCD.TOC.First_Track) track_number = SCD.TOC.First_Track;
				
		CDD.Minute = INT_TO_BCDW(SCD.TOC.Tracks[track_number - SCD.TOC.First_Track].MSF.M);
		CDD.Seconde = INT_TO_BCDW(SCD.TOC.Tracks[track_number - SCD.TOC.First_Track].MSF.S);
		CDD.Frame = INT_TO_BCDW(SCD.TOC.Tracks[track_number - SCD.TOC.First_Track].MSF.F);
		CDD.Ext = track_number % 10;

		if (SCD.TOC.Tracks[0].Type) CDD.Frame |= 0x0800;

		CDD_Complete = 1;
		return 0;
	}
	else
	{
		SCD.Status_CDD = NOCD;
		CDD.Status = (CDD.Status & 0xFF) | SCD.Status_CDD;

		Fill_SCD_TOC_Zero();

		CDD.Minute = 0;
		CDD.Seconde = 0;
		CDD.Frame = 0;
		CDD.Ext = track_number % 10;

		CDD_Complete = 1;
		return -1;
	}
}


int ASPI_Fast_Seek_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	return 0;
}


int ASPI_Play_CDD_c3_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
		SCD.Status_CDD = PLAYING;
		CDD.Status = 0x0102;

		CDD.Control &= 0xFEFF;			// Data bit cleared because audio outputed

		if (SCD.Cur_Track == 100) CDD.Minute = 0x0A02;
		else CDD.Minute = INT_TO_BCDW(SCD.Cur_Track);
		CDD.Seconde = 0;
		CDD.Frame = 0;
		CDD.Ext = 0;

#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "Play Comp : Cur LBA = %d\n", SCD.Cur_LBA);
#endif

		CDD_Complete = 1;
		return 0;
	}
	else
	{
		SCD.Status_CDD = READY;
		CDD.Status = (CDD.Status & 0xFF) | SCD.Status_CDD;

		CDD.Minute = 0;
		CDD.Seconde = 0;
		CDD.Frame = 0;
		CDD.Ext = 0;

		CDD_Complete = 1;
		return -1;
	}
}


int ASPI_Seek_CDD_c4_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	SCD.Status_CDD = READY;
	CDD.Status = 0x0200;

	if (SCD.TOC.Tracks[SCD.Cur_Track - SCD.TOC.First_Track].Type) CDD.Control |= 0x0100;
	else CDD.Control &= 0xFEFF;			// Data bit cleared because audio outputed

	CDD.Minute = 0;
	CDD.Seconde = 0;
	CDD.Frame = 0;
	CDD.Ext = 0;

	CDD_Complete = 1;

	if (s->SRB_Status == SS_COMP)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}


int ASPI_Pause_Play_CDD_c6_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
		SCD.Status_CDD = READY;
		CDD.Status = SCD.Status_CDD;

		CDD.Control |= 0x0100;			// Data bit set because stopped

		CDD.Minute = 0;
		CDD.Seconde = 0;
		CDD.Frame = 0;
		CDD.Ext = 0;

		CDD_Complete = 1;
		return 0;
	}
	else
	{
		CDD.Status = (CDD.Status & 0xFF) | SCD.Status_CDD;

		CDD.Minute = 0;
		CDD.Seconde = 0;
		CDD.Frame = 0;
		CDD.Ext = 0;

		CDD_Complete = 1;
		return -1;
	}
}


int ASPI_Resume_Play_CDD_c7_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
		SCD.Status_CDD = PLAYING;
		CDD.Status = 0x0102;

		if (SCD.TOC.Tracks[SCD.Cur_Track - SCD.TOC.First_Track].Type) CDD.Control |= 0x0100;
		else CDD.Control &= 0xFEFF;			// Data bit cleared because audio outputed

		CDD.Minute = INT_TO_BCDW(SCD.Cur_Track);
		CDD.Seconde = 0;
		CDD.Frame = 0;
		CDD.Ext = 0;

		CDD_Complete = 1;
		return 0;
	}
	else
	{
		CDD.Status = (CDD.Status & 0xFF) | SCD.Status_CDD;

		CDD.Minute = 0;
		CDD.Seconde = 0;
		CDD.Frame = 0;
		CDD.Ext = 0;

		CDD_Complete = 1;
		return -1;
	}
}


int ASPI_Close_Tray_CDD_cC_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
		SCD.Status_CDD = STOPPED;
		CDD.Status = 0x0000;

		CDD.Minute = 0;
		CDD.Seconde = 0;
		CDD.Frame = 0;
		CDD.Ext = 0;

		CDD_Complete = 1;
		return 0;
	}
	else
	{
		CDD.Status = (CDD.Status & 0xFF) | SCD.Status_CDD;

		CDD.Minute = 0;
		CDD.Seconde = 0;
		CDD.Frame = 0;
		CDD.Ext = 0;

		CDD_Complete = 1;
		return -1;
	}
}


int ASPI_Open_Tray_CDD_cD_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
		SCD.Status_CDD = TRAY_OPEN;
		CDD.Status = 0x0E00;

		CD_Present = 0;

		CDD.Minute = 0;
		CDD.Seconde = 0;
		CDD.Frame = 0;
		CDD.Ext = 0;

		CDD_Complete = 1;
		return 0;
	}
	else
	{
		CDD.Status = (CDD.Status & 0xFF) | SCD.Status_CDD;

		CDD.Minute = 0;
		CDD.Seconde = 0;
		CDD.Frame = 0;
		CDD.Ext = 0;

		CDD_Complete = 1;
		return -1;
	}
}




/*
int ASPI_Playing_Pos_LBA_CDD_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
		if (SCD.Status_CDD != PLAYING) SCD.Status_CDD = READY;
		CDD.Status = (CDD.Status & 0xFF) | SCD.Status_CDD;

		SCD.Cur_LBA = Buf_Stat[4] + (Buf_Stat[3] << 8) + (Buf_Stat[2] << 16);

		CDD_Complete = 1;
		return 0;
	}
	else
	{
		CDD.Status = (CDD.Status & 0xFF) | SCD.Status_CDD;
		CDD_Complete = 1;
		return -1;
	}
}
*/

/*
int ASPI_Load_TOC_CDD_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	if (s->SRB_Status == SS_COMP)
	{
		if (s->SRB_TargStat == STATUS_GOOD)
		{
			SCD.Status_CDD = READY;
			CDD.Status = SCD.Status_CDD;

			Fill_SCD_TOC_from_MSF_TOC();

			CDD.Minute = 0;
			CDD.Seconde = 0;
			CDD.Frame = 0;
			CDD.Ext = 0;

			CDD_Complete = 1;
			return 0;
		}
	}

	SCD.Status_CDD = STOPPED;
	CDD.Status = (CDD.Status & 0xFF) | SCD.Status_CDD;

	Fill_SCD_TOC_Zero();

	CDD.Minute = 0;
	CDD.Seconde = 0;
	CDD.Frame = 0;
	CDD.Ext = 0;

	CDD_Complete = 1;
	return -1;
}
*/

int ASPI_CD_Init_CDD_COMP(SRB_ExecSCSICmd *s)
{
	int i = 0;

	ASPI_Command_Running = 0;

	ASPI_Star_Stop_Unit(START_DISC, 0, 0, NULL);
	ASPI_Star_Stop_Unit(START_DISC, 0, 0, NULL);

	return 0;
}


int ASPI_Def_CDD_COMP(SRB_ExecSCSICmd *s)
{
	ASPI_Command_Running = 0;

	SCD.Status_CDD = READY;
	CDD.Status = SCD.Status_CDD;


	CDD.Minute = 0;
	CDD.Seconde = 0;
	CDD.Frame = 0;
	CDD.Ext = 0;

	CDD_Complete = 1;
	return 0;
}



/*******************************
 *        CDC functions        *
 *******************************/


void ASPI_Flush_Cache_CDC(void)
{
	Sectors_In_Cache = 0;
}


void ASPI_Read_One_LBA_CDC(void)
{
	int lba_to_read;
	
	if (SCD.Cur_Track == 100) return;

	if ((Sectors_In_Cache < 7) && (SCD.Cur_LBA >= 0) && (Read_Complete))
	{
		memset(&sread, 0, sizeof(sread));

		sread.SRB_Cmd        = SC_EXEC_SCSI_CMD;
		sread.SRB_HaId       = DEV_PAR[cdromDeviceID][0];
		sread.SRB_Target     = DEV_PAR[cdromDeviceID][1];
		sread.SRB_Lun        = DEV_PAR[cdromDeviceID][2];
		sread.SRB_Flags      = SRB_DIR_IN | SRB_POSTING;

		sread.SRB_PostProc   = ASPI_Read_One_CDC_COMP;

		sread.SRB_BufLen     = 16 * 2352;
		sread.SRB_BufPointer = Buf_Read;
		sread.SRB_SenseLen   = SENSE_LEN;
		sread.SRB_CDBLen     = 12;

#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "\n\nREAD CDC LBA 4 sectors for cache : %d\n\n", SCD.Cur_LBA);
#endif

		lba_to_read = SCD.Cur_LBA + Sectors_In_Cache;

		sread.CDBByte[0]      = SCSI_READ_LBA;

		sread.CDBByte[2]      = lba_to_read >> 24;
		sread.CDBByte[3]      = (lba_to_read >> 16) & 0xFF;
		sread.CDBByte[4]      = (lba_to_read >> 8) & 0xFF;
		sread.CDBByte[5]      = lba_to_read & 0xFF;

		if (LBA_to_Track(lba_to_read + 4) == 100)	// Fin du CD
		{
			sread.CDBByte[8]  = 1;
			SCD.Cur_Track = LBA_to_Track(SCD.Cur_LBA);
		}
		else sread.CDBByte[8] = 4;					// 4 sectors

		sread.CDBByte[9]      = 0x10;				// only user data
    
		Read_Complete = 0;

		Send_ASPI_Command((LPSRB) &sread);
	}

	// directly transfert data from the cache

	ASPI_Read_One_CDC_Cache();
}


int ASPI_Read_One_CDC_COMP(SRB_ExecSCSICmd *s)
{
	int i, lba_read, wr_adr;
	_msf msf_read;

#ifdef DEBUG_CD
	if (CDD.Control & 0x0100)
		fprintf(debug_SCD_file, "\n\nRead CDC 4 data sectors cache COMP :\n");
	else
		fprintf(debug_SCD_file, "\n\nRead CDC 4 audio sectors cache COMP :\n");
#endif

	if (s->SRB_Status == SS_COMP)
	{
		if ((SCD.Status_CDC & 1) == 0)
		{
			Read_Complete = 1;
			return 0;			// if read stopped return...
		}

		wr_adr = (CDC.PT.N + (2352 * (Sectors_In_Cache + 1))) & 0x7FFF;
				
		lba_read = sread.CDBByte[2] << 24;
		lba_read += sread.CDBByte[3] << 16;
		lba_read += sread.CDBByte[4] << 8;
		lba_read += sread.CDBByte[5];

		for(i = 0; i < s->CDBByte[8]; i++)
		{

#ifdef DEBUG_CD
			fprintf(debug_SCD_file, "Sector %d [%d] in CDC.Buf[%d]\n", lba_read, i, wr_adr);
#endif

			LBA_to_MSF(lba_read, &msf_read);

			if (CDD.Control & 0x0100)					// DATA
			{
				memcpy(&CDC.Buffer[wr_adr + 4], &Buf_Read[i << 11], 2048);
				CDC.Buffer[wr_adr + 0] = INT_TO_BCDB(msf_read.M);
				CDC.Buffer[wr_adr + 1] = INT_TO_BCDB(msf_read.S);
				CDC.Buffer[wr_adr + 2] = INT_TO_BCDB(msf_read.F);
				CDC.Buffer[wr_adr + 3] = 1;
			}
			else										// AUDIO
			{
				memcpy(&CDC.Buffer[wr_adr], &Buf_Read[i * 2352], 2352);
			}

			wr_adr = (wr_adr + 2352) & 0x7FFF;
			lba_read++;
			Sectors_In_Cache++;
		}

		Read_Complete = 1;
		return 0;
	}
	else
	{
#ifdef DEBUG_CD
		fprintf(debug_SCD_file, "pas ok\n");
#endif

		Read_Complete = 1;

		if ((SCD.Status_CDC & 1) == 0) return 0;

		ASPI_Read_One_LBA_CDC();				// Retry to read
		return -1;
	}
}


int ASPI_Read_One_CDC_Cache(void)
{
	if ((Sectors_In_Cache <= 0) && (SCD.Cur_LBA >= 0)) return -1;

#ifdef DEBUG_CD
	if (CDD.Control & 0x0100)
		fprintf(debug_SCD_file, "\n\nRead CDC 1 data sector : ");
	else
		fprintf(debug_SCD_file, "\n\nRead CDC 1 audio sector : ");
#endif

	if ((SCD.Status_CDC & 1) == 0)
	{
		Read_Complete = 1;

#ifdef DEBUG_CD
		fprintf(debug_SCD_file, "read complete = %d\n", Read_Complete);
#endif

		return 0;		// if read stopped return...
	}

	// Update CDC stuff

	CDC_Update_Header();

	if (CDD.Control & 0x0100)			// DATA track
	{
		if (CDC.CTRL.B.B0 & 0x80)		// DECEN = decoding enable
		{
			if (CDC.CTRL.B.B0 & 0x04)	// WRRQ : this bit enable write to buffer
			{
				// CAUTION : lookahead bit not implemented

				SCD.Cur_LBA++;

				CDC.WA.N = (CDC.WA.N + 2352) & 0x7FFF;		// add one sector to WA
				CDC.PT.N = (CDC.PT.N + 2352) & 0x7FFF;		// add one sector to PT

				if (SCD.Cur_LBA > 0)
				{
					Sectors_In_Cache--;

#ifdef DEBUG_CD
					fprintf(debug_SCD_file, "\nRead -> WA = %d  Buffer[%d] =\n", CDC.WA.N, CDC.PT.N & 0x3FFF);
					fprintf(debug_SCD_file, "Header 1 = %.2X %.2X %.2X %.2X\n", CDC.HEAD.B.B0, CDC.HEAD.B.B1, CDC.HEAD.B.B2, CDC.HEAD.B.B3);
//					fwrite(Buf_Read, 1, 2048, debug_SCD_file);
//					fprintf(debug_SCD_file, "\nCDC buffer =\n");
//					fwrite(&CDC.Buffer[CDC.PT.N], 1, 2052, debug_SCD_file);
					fprintf(debug_SCD_file, "Header 2 = %.2X %.2X %.2X %.2X --- %.2X %.2X\n\n", CDC.Buffer[(CDC.PT.N + 0) & 0x3FFF], CDC.Buffer[(CDC.PT.N + 1) & 0x3FFF], CDC.Buffer[(CDC.PT.N + 2) & 0x3FFF], CDC.Buffer[(CDC.PT.N + 3) & 0x3FFF], CDC.Buffer[(CDC.PT.N + 4) & 0x3FFF], CDC.Buffer[(CDC.PT.N + 5) & 0x3FFF]);
#endif
				}

			}

			CDC.STAT.B.B0 = 0x80;

			if (CDC.CTRL.B.B0 & 0x10)		// determine form bit form sub header ?
			{
				CDC.STAT.B.B2 = CDC.CTRL.B.B1 & 0x08;
			}
			else
			{
				CDC.STAT.B.B2 = CDC.CTRL.B.B1 & 0x0C;
			}

			if (CDC.CTRL.B.B0 & 0x02) CDC.STAT.B.B3 = 0x20;	// ECC done
			else CDC.STAT.B.B3 = 0x00;	// ECC not done

			if (CDC.IFCTRL & 0x20)
			{
				if (Int_Mask_S68K & 0x20) sub68k_interrupt(5, -1);

#ifdef DEBUG_CD
				fprintf(debug_SCD_file, "CDC - DEC interrupt\n");
#endif

				CDC.IFSTAT &= ~0x20;		// DEC interrupt happen
				CDC_Decode_Reg_Read = 0;	// Reset read after DEC int
			}
		}
	}
	else				// AUDIO track
	{
		SCD.Cur_LBA++;		// Always increment sector if audio

		CDC.WA.N = (CDC.WA.N + 2352) & 0x7FFF;		// add one sector to WA
		CDC.PT.N = (CDC.PT.N + 2352) & 0x7FFF;		// add one sector to PT

		if (SCD.Cur_LBA > 0)
		{
			Sectors_In_Cache--;
			Write_CD_Audio((short *) &CDC.Buffer[CDC.PT.N], 44100, 2, 588);
		}

		if (CDC.CTRL.B.B0 & 0x80)		// DECEN = decoding enable
		{
			CDC.STAT.B.B0 = 0x80;

			if (CDC.CTRL.B.B0 & 0x10)		// determine form bit form sub header ?
			{
				CDC.STAT.B.B2 = CDC.CTRL.B.B1 & 0x08;
			}
			else
			{
				CDC.STAT.B.B2 = CDC.CTRL.B.B1 & 0x0C;
			}

			if (CDC.CTRL.B.B0 & 0x02) CDC.STAT.B.B3 = 0x20;	// ECC done
			else CDC.STAT.B.B3 = 0x00;	// ECC not done

			if (CDC.IFCTRL & 0x20)
			{
				if (Int_Mask_S68K & 0x20) sub68k_interrupt(5, -1);

#ifdef DEBUG_CD
				fprintf(debug_SCD_file, "CDC - DEC interrupt\n");
#endif

				CDC.IFSTAT &= ~0x20;		// DEC interrupt happen
				CDC_Decode_Reg_Read = 0;	// Reset read after DEC int
			}
		}
	}

	return 0;
}


void Wait_Read_Complete(void)
{
	int i = 0;
	
	while ((Read_Complete == 0) && (i++ < 1000)) Sleep(1);

#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "\n******* Wait Read %d ******\n", i);
#endif
}


/*******************************
 *     Specials Functions      *
 *******************************/


void Fill_SCD_TOC_from_MSF_TOC(void)
{
	TOCTRACK *t;
	unsigned char i, numTracks;

	SCD.TOC.First_Track = toc.firstTrack;
	SCD.TOC.Last_Track = toc.lastTrack;

#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "\nSCD TOC : First track = %d    Last track = %d\n", SCD.TOC.First_Track, SCD.TOC.Last_Track);
#endif

	numTracks = toc.lastTrack - toc.firstTrack + 1;

	for(i = 0; i < numTracks; i++)
	{
		t = &(toc.tracks[i]);

		SCD.TOC.Tracks[i].Num = t->trackNumber;
		SCD.TOC.Tracks[i].Type = (t->ADR & 0x04) >> 2;		// DATA flag

		SCD.TOC.Tracks[i].MSF.M = t->addr[1];
		SCD.TOC.Tracks[i].MSF.S = t->addr[2];
		SCD.TOC.Tracks[i].MSF.F = t->addr[3];

#ifdef DEBUG_CD
		fprintf(debug_SCD_file, "Track %i - %02d:%02d:%02d ", SCD.TOC.Tracks[i].Num, SCD.TOC.Tracks[i].MSF.M, SCD.TOC.Tracks[i].MSF.S, SCD.TOC.Tracks[i].MSF.F);
		if (SCD.TOC.Tracks[i].Type) fprintf(debug_SCD_file, "DATA\n");
		else fprintf(debug_SCD_file, "AUDIO\n");
#endif
	}

	SCD.TOC.Tracks[numTracks].Num = toc.tracks[numTracks].trackNumber;
	SCD.TOC.Tracks[numTracks].Type = 0;

	SCD.TOC.Tracks[numTracks].MSF.M = toc.tracks[numTracks].addr[1];
	SCD.TOC.Tracks[numTracks].MSF.S = toc.tracks[numTracks].addr[2];
	SCD.TOC.Tracks[numTracks].MSF.F = toc.tracks[numTracks].addr[3];

#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "End CD - %02d:%02d:%02d\n\n", SCD.TOC.Tracks[numTracks].MSF.M, SCD.TOC.Tracks[numTracks].MSF.S, SCD.TOC.Tracks[numTracks].MSF.F);
#endif

	CD_Present = 1;
}


void Fill_SCD_TOC_Zero(void)
{
	int i;

	SCD.TOC.First_Track = 0;
	SCD.TOC.Last_Track = 0;

#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "\nSCD TOC filled with zero, CD no present\n");
#endif

	for(i = 0; i < 10; i++)
	{
		SCD.TOC.Tracks[i].Num = 0;
		SCD.TOC.Tracks[i].Type = 0;
		SCD.TOC.Tracks[i].MSF.M = 0;
		SCD.TOC.Tracks[i].MSF.S = 0;
		SCD.TOC.Tracks[i].MSF.F = 0;
	}

	CD_Present = 0;
}


