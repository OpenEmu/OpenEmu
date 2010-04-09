/*-----------------------------------------------------------------------------
	[ADPCM.h]
		ＡＤＰＣＭを記述するのに必要な定義および
	関数のプロトタイプ宣言を行ないます。

	Copyright (C) 2004 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**---------------------------------------------------------------------------*/
#ifndef ADPCM_H_INCLUDED
#define ADPCM_H_INCLUDED

#define ADPCM_STATE_NORMAL			0
#define ADPCM_STATE_HALF_PLAYED		1
#define ADPCM_STATE_FULL_PLAYED		2
#define ADPCM_STATE_STOPPED			4

bool ADPCM_Init();

void ADPCM_Power();

void ADPCM_Close();

void ADPCM_SetNotificationFunction(void	(*pfnNotification)(uint32));

void ADPCM_SetAddrLo(uint8	addrLo);

void ADPCM_SetAddrHi(uint8	addrHi);

uint8 ADPCM_ReadBuffer();

uint8 ADPCM_Read180D(void);

void ADPCM_Write180D(uint8 data);

void ADPCM_WriteBuffer(uint8	data);

void ADPCM_Play(uint8);

void ADPCM_SetFreq(uint32 freq);

bool ADPCM_IsPlaying();

void ADPCM_FadeOut(int32 ms);

void ADPCM_FadeIn(int32 ms);

bool ADPCM_IsWritePending(void);
bool ADPCM_IsBusyReading(void);

int ADPCM_StateAction(StateMem *sm, int load, int data_only);

void ADPCM_Run(int32 clocks);

#ifdef WANT_DEBUGGER
uint32 ADPCM_GetRegister(const std::string &name, std::string *special);
#endif

#endif		/* ADPCM_H_INCLUDED */

