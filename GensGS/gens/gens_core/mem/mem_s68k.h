#ifndef GENS_MEM_S68K_H
#define GENS_MEM_S68K_H

#ifdef __cplusplus
extern "C" {
#endif

extern struct
{
	unsigned int Flag;
	unsigned short Command[8];
	unsigned short Status[8];
} COMM;

extern unsigned char Ram_Prg[512 * 1024];
extern unsigned char Ram_Word_2M[256 * 1024];
extern unsigned char Ram_Word_1M[256 * 1024];
extern unsigned short Cell_Conv_Tab[64 * 1024];
extern unsigned char Ram_Backup[8 * 1024];
extern unsigned int LED_Status;
extern unsigned int S68K_Mem_WP;
extern unsigned int S68K_Mem_PM;
extern unsigned int Ram_Word_State;
extern unsigned int Memory_Control_Status;
extern unsigned int Init_Timer_INT3;
extern unsigned int Timer_INT3;
extern unsigned int Timer_Step;
extern unsigned int Int_Mask_S68K;
extern unsigned int Font_COLOR;
extern unsigned int Font_BITS;
extern unsigned int CD_Access_Timer;

unsigned char S68K_RB(unsigned int Adr);
unsigned short S68K_RW(unsigned int Adr);
void S68K_WB(unsigned int Adr, unsigned char Data);
void S68K_WW(unsigned int Adr, unsigned short Data);

void Update_SegaCD_Timer(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_MEM_S68K_H */
