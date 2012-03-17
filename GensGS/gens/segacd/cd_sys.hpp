#ifndef GENS_CD_SYS_HPP
#define GENS_CD_SYS_HPP

#ifdef __cplusplus
extern "C" {
#endif

#define TRAY_OPEN	0x0500		// TRAY OPEN CDD status
#define NOCD		0x0000		// CD removed CDD status
#define STOPPED		0x0900		// STOPPED CDD status (happen after stop or close tray command)
#define READY		0x0400		// READY CDD status (also used for seeking)
#define FAST_FOW	0x0300		// FAST FORWARD track CDD status
#define FAST_REV	0x10300		// FAST REVERSE track CDD status
#define PLAYING		0x0100		// PLAYING audio track CDD status

#define MSF_FORMAT  1

#define CDROM_		0			// value for CD_Load_System
#define FILE_CUE	1			// value for CD_Load_System
#define FILE_ISO	2			// value for CD_Load_System
#define FILE_ZIP	3			// value for CD_Load_System

//#define DEBUG_CD

#ifdef DEBUG_CD
	extern FILE *debug_SCD_file;
#endif


#define INT_TO_BCDB(c)										\
((c) > 99)?(0x99):((((c) / 10) << 4) + ((c) % 10));

#define INT_TO_BCDW(c)										\
((c) > 99)?(0x0909):((((c) / 10) << 8) + ((c) % 10));

#define BCDB_TO_INT(c)										\
(((c) >> 4) * 10) + ((c) & 0xF);

#define BCDW_TO_INT(c)										\
(((c) >> 8) * 10) + ((c) & 0xF);


typedef struct
{
	unsigned char M;
	unsigned char S;
	unsigned char F;
} _msf;

typedef struct
{
	unsigned char Type;
	unsigned char Num;
	_msf MSF;
} _scd_track;

typedef struct
{
	unsigned char First_Track;
	unsigned char Last_Track;
	_scd_track Tracks[100];
} _scd_toc;

typedef struct {
	unsigned int Status_CDD;
	unsigned int Status_CDC;
	_scd_toc TOC;
	int Cur_LBA;
	unsigned int Cur_Track;
} _scd;


extern int File_Add_Delay;	// GENS Re-Recording [GENS Savestate v7]
extern int CDDA_Enable;

extern int CD_Audio_Buffer_L[8192];
extern int CD_Audio_Buffer_R[8192];
extern int CD_Audio_Buffer_Read_Pos;
extern int CD_Audio_Buffer_Write_Pos;
extern int CD_Audio_Starting;

extern int CD_Present;
extern int CD_Load_System;
extern int CD_Timer_Counter;

extern int CDD_Complete;

extern int track_number;			// Used for the Get_Track_Adr function

extern unsigned int CD_timer_st;	// Used for CD timer
extern unsigned int CD_LBA_st;		// Used for CD timer

extern _scd SCD;


void MSB2DWORD(unsigned int *d, unsigned char *b);
int MSF_to_LBA(_msf *MSF);
void LBA_to_MSF(int lba, _msf *MSF);
unsigned int MSF_to_Track(_msf *MSF);
unsigned int LBA_to_Track(int lba);
void Track_to_MSF(int track, _msf *MSF);
int Track_to_LBA(int track);


void Flush_CD_Command(void);
void Check_CD_Command(void);
void Init_CD_Driver(void);
void End_CD_Driver(void);

// Alias to Check_CD_Command, needed by mem_s68k.asm
void _Check_CD_Command(void);

int Reset_CD(char *buf, const char *iso_name);
void Stop_CD(void);
void Change_CD(void);

int Get_Status_CDD_c0(void);
int Stop_CDD_c1(void);
int Get_Pos_CDD_c20(void);
int Get_Track_Pos_CDD_c21(void);
int Get_Current_Track_CDD_c22(void);
int Get_Total_Length_CDD_c23(void);
int Get_First_Last_Track_CDD_c24(void);
int Get_Track_Adr_CDD_c25(void);
int Play_CDD_c3(void);
int Seek_CDD_c4(void);
int Pause_CDD_c6(void);
int Resume_CDD_c7(void);
int	Fast_Foward_CDD_c8(void);
int	Fast_Rewind_CDD_c9(void);
int CDD_cA(void);
int Close_Tray_CDD_cC(void);
int Open_Tray_CDD_cD(void);

int CDD_Def(void);

void Write_CD_Audio(short *Buf, int rate, int channel, int length);
void Update_CD_Audio(int **Buf, int length);
//void Start_CD_Timer(void);
//void Stop_CD_Timer(void);
//void Update_CD_Timer(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_CD_SYS_H */
