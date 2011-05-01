#ifndef __PCE_CDROM_H
#define __PCE_CDROM_H

#include <blip/Blip_Buffer.h>

typedef struct
{
	double CDDA_Volume;

	unsigned int CD_Speed;

	double ADPCM_Volume;

	bool ADPCM_LPF;
} PCECD_Settings;


enum
{
 CD_GSREG_BSY = 0,
 CD_GSREG_REQ,	// RO
 CD_GSREG_MSG,	// RO
 CD_GSREG_CD,	// RO
 CD_GSREG_IO,	// RO
 CD_GSREG_SEL,

 CD_GSREG_ADPCM_CONTROL,
 CD_GSREG_ADPCM_FREQ,
 CD_GSREG_ADPCM_CUR,
 CD_GSREG_ADPCM_WRADDR,
 CD_GSREG_ADPCM_RDADDR,
 CD_GSREG_ADPCM_LENGTH,
 CD_GSREG_ADPCM_PLAYNIBBLE,

 CD_GSREG_ADPCM_PLAYING,
 CD_GSREG_ADPCM_HALFREACHED,
 CD_GSREG_ADPCM_ENDREACHED,
};

uint32 PCECD_GetRegister(const unsigned int id, char *special, const uint32 special_len);
void PCECD_SetRegister(const unsigned int id, const uint32 value);


int32 PCECD_Run(uint32 in_timestamp) MDFN_WARN_UNUSED_RESULT;
void PCECD_ResetTS(void);

bool PCECD_Init(const PCECD_Settings *settings, void (*irqcb)(bool), double master_clock, unsigned int ocm, Blip_Buffer *soundbuf_l, Blip_Buffer *soundbuf_r);
bool PCECD_SetSettings(const PCECD_Settings *settings);

void PCECD_Close();

// Returns number of cycles until next CD event.
int32 PCECD_Power(uint32 timestamp) MDFN_WARN_UNUSED_RESULT;

uint8 PCECD_Read(uint32 timestamp, uint32, int32 &next_event, const bool PeekMode = false);
int32 PCECD_Write(uint32 timestamp, uint32, uint8 data) MDFN_WARN_UNUSED_RESULT;

bool PCECD_IsBRAMEnabled();

int PCECD_StateAction(StateMem *sm, int load, int data_only);

void ADPCM_PeekRAM(uint32 Address, uint32 Length, uint8 *Buffer);
void ADPCM_PokeRAM(uint32 Address, uint32 Length, const uint8 *Buffer);

#endif

