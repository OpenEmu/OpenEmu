
#define io_M3SD_c
#include "io_m3sd.h"
#ifdef SUPPORT_M3SD
//M3-SD interface SD card.

#define DMA3SAD      *(volatile u32*)0x040000D4
#define DMA3DAD      *(volatile u32*)0x040000D8
#define DMA3CNT      *(volatile u32*)0x040000DC
#define DMA3CR       *(volatile u16*)0x040000DE

//SD dir control bit cmddir=bit0 clken=bit1
//output
#define SDDIR			(*(volatile u16*)0x8800000)

//SD send get control bit send=bit0 get=bit1
//output
#define SDCON			(*(volatile u16*)0x9800000)

//SD output data obyte[7:0]=AD[7:0]
//output
#define SDODA			(*(volatile u16*)0x9000000)

//SD input data AD[7:0]=ibyte[7:0]
//input
#define SDIDA			(*(volatile u16*)0x9000000)

//readsector data1
#define SDIDA1			(*(volatile u16*)0x9200000)

//readsector data2
#define SDIDA2			(*(volatile u16*)0x9400000)

//readsector data3
#define SDIDA3			(*(volatile u16*)0x9600000)

//SD stutas cmdneg=bit0 cmdpos=bit1 issend=bit2 isget=bit3
//input
#define SDSTA			(*(volatile u16*)0x9800000)

#define M3_REG_STS		*(vu16*)(0x09800000)	// Status of the CF Card / Device control
#define M3_DATA			(vu16*)(0x08800000)		// Pointer to buffer of CF data transered from card

// CF Card status
#define CF_STS_INSERTED1		0x20
#define CF_STS_INSERTED2		0x30
#define CF_STS_INSERTED3		0x22
#define CF_STS_INSERTED4		0x32
#define isM3ins(sta)	((sta==CF_STS_INSERTED1) || (sta==CF_STS_INSERTED2) || (sta==CF_STS_INSERTED3) || (sta==CF_STS_INSERTED4))


#define CARD_TIMEOUT	400000		// Updated due to suggestion from SaTa, otherwise card will timeout sometimes on a write
//#define CARD_TIMEOUT	(500*100)	// M3SD timeout nomal:500

void SendCommand(u16 command, u32 sectorn);
void PassRespond(u32 num);
void SD_crc16(u16* buff,u16 num,u16* crc16buff);
void SD_data_write(u16 *buff,u16* crc16buff);
u16 M3_SetChipReg(u32 Data);

//=========================================================
u16 M3_SetChipReg(u32 Data)
{
	u16 i,j;

	i = *(volatile u16*)(0x700001*2+0x8000000);

	i = *(volatile u16*)(0x000007*2+0x8000000);
	i = *(volatile u16*)(0x400ffe*2+0x8000000);
	i = *(volatile u16*)(0x000825*2+0x8000000);

	i = *(volatile u16*)(0x400309*2+0x8000000);
	i = *(volatile u16*)(0x000000*2+0x8000000);
	i = *(volatile u16*)(0x400db3*2+0x8000000);

	i = *(volatile u16*)((Data*2)+0x8000000);

	j = *(volatile u16*)(0x000407*2+0x8000000);
	i = *(volatile u16*)(0x000000*2+0x8000000);

	return j;
}

void M3_SelectSaver(u8 Bank)
{
	u16 i;

	i = *(volatile u16*)(0x700001*2+0x8000000);

	i = *(volatile u16*)(0x000007*2+0x8000000);
	i = *(volatile u16*)(0x400FFE*2+0x8000000);
	i = *(volatile u16*)(0x000825*2+0x8000000);

	i = *(volatile u16*)(0x400309*2+0x8000000);
	i = *(volatile u16*)(0x000000*2+0x8000000);
	i = *(volatile u16*)(0x400db3*2+0x8000000);

	i = *(volatile u16*)((Bank<<4)*2+0x8000000);

	i = *(volatile u16*)(0x000407*2+0x8000000);
	i = *(volatile u16*)(0x000000*2+0x8000000);
}

void DMA3(u32 src, u32 dst, u32 cnt)
{
	u16 i,j,cnttmp;

	cnttmp = (cnt&0xffff);
	if( ((dst&0x03) == 0)
		&&((cnttmp&0x0f) == 0)
		&&(cnttmp>0))
	{
		DC_FlushRange(dst,cnttmp*2);
		DMA3CR &= (~0x3a00);
		DMA3CR &= (~0x8000);
		i = DMA3CR;
		j = DMA3CR;

		DMA3SAD=src;
		DMA3DAD=dst;
		DMA3CNT=cnt;
	}
	else
	{
		for(j=0;j<cnttmp;j++)
		{
			*(u16*)(dst+j*2) = *(u16*)(src+j*2);
		}
	}
}

void SendCommand(u16 command, u32 sectorn)
{
	SDCON=0x8;
	SDIDA1=0x40+command;
	SDIDA2=(sectorn>>7);
	SDIDA3=(sectorn<<9);

	SDDIR=0x29;
	while ((SDSTA&0x01) != 0x01);
	SDDIR=0x09;
}

void PassRespond(u32 num)
{
	u32 i,j,dmanum;

	dmanum=(64+(num<<3))>>2;
	SDDIR=0x8;
	SDCON=0x4;

	for(j=0;j<dmanum;j++)
	{
		i = SDDIR;
	}
}

//read multi sectors function
void readsectors(u16 * p,u32 sectorn,u16 number)
{
	u32 i,j;

	SendCommand(18,sectorn);
	for(i=0;i<number;i++,p+=0x100)
	{
		SDDIR=0x49;
		while ( (SDSTA&0x40) !=0x40);
		SDDIR=0x09;

		SDDIR=0x8;
		SDCON=0x4;
		j = *(volatile u16*)0x8800000;
		DMA3(0x8800000,(u32)p,0x80000100);
		j = *(volatile u16*)0x8800000;
		j = *(volatile u16*)0x8800000;
		j = *(volatile u16*)0x8800000;
		j = *(volatile u16*)0x8800000;

		SDCON=0x8;
	}

	SendCommand(12,sectorn);
	PassRespond(6);
}

//write one sector function
void M3SD_writesector(u16 * p,u32 sectorn)
{
	u16 crc[4];
	u16* check = (u16 *) malloc(512);
	u16* data = (u16 *) malloc(512);
	memcpy(data, p, 512);

	int verify = 0;
	int tries = 0;
	do {
		SendCommand(24,sectorn);
		PassRespond(6);

		SDDIR=0x4;
		SDCON=0x0;

		SD_crc16(data,512,crc);
		SD_data_write(data,crc);

		readsectors(check, sectorn, 1);

		int r;
		verify = 0;
		for (r = 0; r < 256; r++) {
			if (check[r] != data[r]) {
				verify++;
			}
		}

		if (verify > 0) {
			tries++;
		}

	} while ((verify > 0) && (tries < 16));

	free(data);
	free(check);
}	// */

/*-----------------------------------------------------------------
M3SD_IsInserted
Is a compact flash card inserted?
bool return OUT:  true if a CF card is inserted
-----------------------------------------------------------------*/
bool M3SD_IsInserted (void)
{
	u16 sta;
	bool i;

	M3_SetChipReg(0x400003);
	M3_REG_STS = CF_STS_INSERTED1;
	sta=M3_REG_STS;
	i = ( isM3ins(sta) );

	M3_SetChipReg(0x400002);
	return i;
}

/*-----------------------------------------------------------------
M3SD_ClearStatus
Tries to make the CF card go back to idle mode
bool return OUT:  true if a CF card is idle
-----------------------------------------------------------------*/
bool M3SD_ClearStatus (void)
{
	int i;
	u16 sta;

	M3_SetChipReg(0x400003);
	i = 0;
	M3_REG_STS = CF_STS_INSERTED1;
	while (i < CARD_TIMEOUT)
	{
		sta=M3_REG_STS;
		if(  isM3ins(sta)  )break;
		i++;
	}

	M3_SetChipReg(0x400002);
	if (i >= CARD_TIMEOUT) return false;

	return true;
}

/*-----------------------------------------------------------------
M3SD_ReadSectors
Read 512 byte sector numbered "sector" into "buffer"
u32 sector IN: address of first 512 byte sector on CF card to read
u8 numSecs IN: number of 512 byte sectors to read,
 1 to 256 sectors can be read, 0 = 256
void* buffer OUT: pointer to 512 byte buffer to store data in
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool M3SD_ReadSectors(u32 sector, u8 numSecs, void* buffer)
{
	//read multi sectors function
	M3_SetChipReg(0x400003);
	readsectors((u16*)buffer,sector,numSecs);
	M3_SetChipReg(0x400002);
	return true;	// */
}
/*-----------------------------------------------------------------
M3SD_WriteSectors
Write 512 byte sector numbered "sector" from "buffer"
u32 sector IN: address of 512 byte sector on CF card to read
u8 numSecs IN: number of 512 byte sectors to read,
 1 to 256 sectors can be read, 0 = 256
void* buffer IN: pointer to 512 byte buffer to read data from
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool M3SD_WriteSectors (u32 sector, u8 numSecs, void* buffer)
{
	bool r=true;
	int i;
	M3_SetChipReg(0x400003);
	for(i=0;i<numSecs;i++)
	{
		M3SD_writesector((u16*)((u32)buffer+512*i),sector+i);
	}
	M3_SetChipReg(0x400002);
	return r;
}

/*-----------------------------------------------------------------
M3_Unlock
Returns true if M3 was unlocked, false if failed
Added by MightyMax
-----------------------------------------------------------------*/
bool M3SD_Unlock(void)
{
	vu16 sta;
	bool i;

	// run unlock sequence
	volatile unsigned short tmp;

	M3_SetChipReg(0x400003);
	tmp = *(volatile unsigned short *)0x08000000 ;
	tmp = *(volatile unsigned short *)0x08E00002 ;
	tmp = *(volatile unsigned short *)0x0800000E ;
	tmp = *(volatile unsigned short *)0x08801FFC ;
	tmp = *(volatile unsigned short *)0x0800104A ;
	tmp = *(volatile unsigned short *)0x08800612 ;
	tmp = *(volatile unsigned short *)0x08000000 ;
	tmp = *(volatile unsigned short *)0x08801B66 ;
	tmp = *(volatile unsigned short *)0x08800006 ;
	tmp = *(volatile unsigned short *)0x08000000 ;

	// test that we have register access
	sta=M3_REG_STS;
	sta=M3_REG_STS;
	if(  isM3ins(sta)  )
	{
		i = true;
	}
	else
	{
		i = false;
	}

	M3_SetChipReg(0x400002);
	return i;
}

bool M3SD_Shutdown(void)
{
	return M3SD_ClearStatus() ;
}

bool M3SD_StartUp(void)
{
	vu16* waitCr = (vu16*)0x4000204;

	*waitCr |= 0x6000;
//	*(vu16*)0x4000204=0x6000;
	// Try unlocking 3 times, because occationally it fails to detect the reader.
	return M3SD_Unlock() | M3SD_Unlock() | M3SD_Unlock();
}

IO_INTERFACE io_m3sd =
{
	0x4453334D,	// 'M3SD'
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE,
	(FN_MEDIUM_STARTUP)&M3SD_StartUp,
	(FN_MEDIUM_ISINSERTED)&M3SD_IsInserted,
	(FN_MEDIUM_READSECTORS)&M3SD_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&M3SD_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&M3SD_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&M3SD_Shutdown
};

LPIO_INTERFACE M3SD_GetInterface(void)
{
	return &io_m3sd ;
}

#endif
