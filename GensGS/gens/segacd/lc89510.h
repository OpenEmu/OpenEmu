#ifndef GENS_LC89510_H
#define GENS_LC89510_H

#ifdef __cplusplus
extern "C" {
#endif

extern struct
{
	unsigned int RS0;
	unsigned int RS1;
	unsigned int Host_Data;
	unsigned int DMA_Adr;
	unsigned int Stop_Watch;
	unsigned int COMIN;
	unsigned int IFSTAT;
	union
	{
		struct
		{
			unsigned char L;
			unsigned char H;
			unsigned short unused;
		} B;
		int N;
	} DBC;
	union
	{
		struct
		{
			unsigned char L;
			unsigned char H;
			unsigned short unused;
		} B;
		int N;
	} DAC;
	union
	{
		struct
		{
			unsigned char B0;
			unsigned char B1;
			unsigned char B2;
			unsigned char B3;
		} B;
		unsigned int N;
	} HEAD;
	union
	{
		struct
		{
			unsigned char L;
			unsigned char H;
			unsigned short unused;
		} B;
		int N;
	} PT;
	union
	{
		struct
		{
			unsigned char L;
			unsigned char H;
			unsigned short unused;
		} B;
		int N;
	} WA;
	union
	{
		struct
		{
			unsigned char B0;
			unsigned char B1;
			unsigned char B2;
			unsigned char B3;
		} B;
		unsigned int N;
	} STAT;
	unsigned int SBOUT;
	unsigned int IFCTRL;
	union
	{
		struct
		{
			unsigned char B0;
			unsigned char B1;
			unsigned char B2;
			unsigned char B3;
		} B;
		unsigned int N;
	} CTRL;
	unsigned char Buffer[(32 * 1024 * 2) + 2352];
} CDC;

extern struct
{
	unsigned int Fader;
	unsigned int Control;
	unsigned int Cur_Comm;
	unsigned char Rcv_Status[10];
	unsigned char Trans_Comm[10];
	unsigned int Status;
	unsigned int Minute;
	unsigned int Seconde;
	unsigned int Frame;
	unsigned int Ext;
} CDD;


extern int CDC_Decode_Reg_Read;


void LC89510_Reset(void);
void Update_CDC_TRansfert(void);
void CDC_Update_Header(void);
unsigned char CDC_Read_Reg(void);
void CDC_Write_Reg(unsigned char Data);

void CDD_Processing(void);
void CDD_Import_Command(void);

unsigned char SCD_Read_Byte(unsigned int Adr);
unsigned short SCD_Read_Word(unsigned int Adr);

#ifdef __cplusplus
}
#endif

#endif /* GENS_LC89510_H */
