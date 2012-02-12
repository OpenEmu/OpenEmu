/* FCE Ultra - NES/Famicom Emulator
*
* Copyright notice for this file:
*  Copyright (C) 1998 BERO
*  Copyright (C) 2002 Xodnizel
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <string>
#include <ostream>
#include <string.h>

#include "types.h"
#include "x6502.h"

#include "fceu.h"
#include "sound.h"
#include "state.h"
#include "input/zapper.h"
#include "input.h"
#include "vsuni.h"
#include "fds.h"
#include "driver.h"

#define FCEUNPCMD_RESET 	0x01
#define FCEUNPCMD_POWER 	0x02

#define FCEUNPCMD_VSUNICOIN     0x07
#define FCEUNPCMD_VSUNIDIP0	0x08
#define FCEUNPCMD_FDSINSERTx	0x10
#define FCEUNPCMD_FDSINSERT	0x18
#define FCEUNPCMD_FDSEJECT	0x19
#define FCEUNPCMD_FDSSELECT	0x1A

#define FCEUNPCMD_LOADSTATE     0x80

#define FCEUNPCMD_SAVESTATE     0x81 /* Sent from server to client. */
#define FCEUNPCMD_LOADCHEATS	0x82
#define FCEUNPCMD_TEXT		0x90

//it is easier to declare these input drivers extern here than include a bunch of files
//-------------
extern INPUTC *FCEU_InitZapper(int w);
extern INPUTC *FCEU_InitPowerpadA(int w);
extern INPUTC *FCEU_InitPowerpadB(int w);
extern INPUTC *FCEU_InitArkanoid(int w);

extern INPUTCFC *FCEU_InitArkanoidFC(void);
extern INPUTCFC *FCEU_InitSpaceShadow(void);
extern INPUTCFC *FCEU_InitFKB(void);
extern INPUTCFC *FCEU_InitSuborKB(void);
extern INPUTCFC *FCEU_InitHS(void);
extern INPUTCFC *FCEU_InitMahjong(void);
extern INPUTCFC *FCEU_InitQuizKing(void);
extern INPUTCFC *FCEU_InitFamilyTrainerA(void);
extern INPUTCFC *FCEU_InitFamilyTrainerB(void);
extern INPUTCFC *FCEU_InitOekaKids(void);
extern INPUTCFC *FCEU_InitTopRider(void);
extern INPUTCFC *FCEU_InitBarcodeWorld(void);
extern INPUTCFC *FCEU_InitFamicom3D(void);

static uint8 joy_readbit[2];
static uint8 joy[4]={0,0,0,0};
static uint8 LastStrobe;

bool replaceP2StartWithMicrophone = false;

extern uint8 coinon;

//set to true if the fourscore is attached
static bool FSAttached = false;

JOYPORT joyports[2] = { JOYPORT(0), JOYPORT(1) };
FCPORT portFC;

static DECLFR(JPRead)
{
	uint8 ret=0;
	static bool microphone = false;

	ret|=joyports[A&1].driver->Read(A&1);

	// Test if the port 2 start button is being pressed.
	// On a famicom, port 2 start shouldn't exist, so this removes it.
	// Games can't automatically be checked for NES/Famicom status,
	// so it's an all-encompassing change in the input config menu.
	if ((replaceP2StartWithMicrophone) && (A&1) && (joy_readbit[1] == 4)) {
	// Nullify Port 2 Start Button
	ret&=0xFE;
	}

	if(portFC.driver)
		ret = portFC.driver->Read(A&1,ret);

	// Not verified against hardware.
	if (replaceP2StartWithMicrophone)
	{
		if (joy[1]&8)
		{
			microphone = !microphone;
			if (microphone)
				ret|=4;
		}
		else
			microphone = false;
	}

	ret|=X.DB&0xC0;

	return(ret);
}

static DECLFW(B4016)
{
	if(portFC.driver)
		portFC.driver->Write(V&7);

	joyports[0].driver->Write(V&1);
	joyports[1].driver->Write(V&1);

	if((LastStrobe&1) && (!(V&1)))
	{
		//old comment:
		//This strobe code is just for convenience.  If it were
		//with the code in input / *.c, it would more accurately represent
		//what's really going on.  But who wants accuracy? ;)
		//Seriously, though, this shouldn't be a problem.
		//new comment:
		
		//mbg 6/7/08 - I guess he means that the input drivers could track the strobing themselves
		//I dont see why it is unreasonable here.
		joyports[0].driver->Strobe(0);
		joyports[1].driver->Strobe(1);

		if(portFC.driver)
			portFC.driver->Strobe();
	}
	LastStrobe=V&0x1;
}

//a main joystick port driver representing the case where nothing is plugged in
static INPUTC DummyJPort={0,0,0,0,0};
//and an expansion port driver for the same ting
static INPUTCFC DummyPortFC={0,0,0,0,0};

//--------4 player driver for expansion port--------
static uint8 F4ReadBit[2];
static void StrobeFami4(void)
{
	F4ReadBit[0]=F4ReadBit[1]=0;
}

static uint8 ReadFami4(int w, uint8 ret)
{
	ret&=1;

	ret |= ((joy[2+w]>>(F4ReadBit[w]))&1)<<1;
	if(F4ReadBit[w]>=8) ret|=2;
	else F4ReadBit[w]++;

	return(ret);
}

static INPUTCFC FAMI4C={ReadFami4,0,StrobeFami4,0,0};

static uint8 ReadGPVS(int w)
{
	uint8 ret=0;

	if(joy_readbit[w]>=8)
		ret=1;
	else
	{
		ret = ((joy[w]>>(joy_readbit[w]))&1);
		joy_readbit[w]++;
	}
	return ret;
}

static void UpdateGP(int w, void *data, int arg)
{
	if(w==0)	//adelikat, 3/14/09: Changing the joypads to inclusive OR the user's joypad + the Lua joypad, this way lua only takes over the buttons it explicity says to
	{			//FatRatKnight: Assume lua is always good. If it's doing nothing in particular using my logic, it'll pass-through the values anyway.
		joy[0] = *(uint32 *)joyports[0].ptr;
		joy[2] = *(uint32 *)joyports[0].ptr >> 16;
	}
	else
	{
		joy[1] = *(uint32 *)joyports[1].ptr >> 8;
		joy[3] = *(uint32 *)joyports[1].ptr >> 24;
	}

}

//basic joystick port driver
static uint8 ReadGP(int w)
{
	uint8 ret;

	if(joy_readbit[w]>=8)
		ret = ((joy[2+w]>>(joy_readbit[w]&7))&1);
	else
		ret = ((joy[w]>>(joy_readbit[w]))&1);

	if(joy_readbit[w]>=16)
		ret=0;

	if(!FSAttached)
		if(joy_readbit[w]>=8) ret|=1;
		else
			if(joy_readbit[w]==19-w) ret|=1;

	joy_readbit[w]++;

	return ret;
}

static void StrobeGP(int w)
{
	joy_readbit[w]=0;
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^6


static INPUTC GPC={ReadGP,0,StrobeGP,UpdateGP,0};
static INPUTC GPCVS={ReadGPVS,0,StrobeGP,UpdateGP,0};

void FCEU_UpdateInput(void)
{
	//tell all drivers to poll input and set up their logical states
	joyports[0].driver->Update(0,joyports[0].ptr,joyports[0].attrib);
	joyports[1].driver->Update(1,joyports[1].ptr,joyports[1].attrib);
	portFC.driver->Update(portFC.ptr,portFC.attrib);

	if(GameInfo->type==GIT_VSUNI)
		if(coinon) coinon--;

	if(GameInfo->type==GIT_VSUNI)
		FCEU_VSUniSwap(&joy[0],&joy[1]);
}

static DECLFR(VSUNIRead0)
{
	uint8 ret=0;

	ret|=(joyports[0].driver->Read(0))&1;

	ret|=(vsdip&3)<<3;
	if(coinon)
		ret|=0x4;
	return ret;
}

static DECLFR(VSUNIRead1)
{
	uint8 ret=0;

	ret|=(joyports[1].driver->Read(1))&1;
	ret|=vsdip&0xFC;
	return ret;
}



//calls from the ppu;
//calls the SLHook for any driver that needs it
void InputScanlineHook(uint8 *bg, uint8 *spr, uint32 linets, int final)
{
	joyports[0].driver->SLHook(0,bg,spr,linets,final);
	joyports[1].driver->SLHook(1,bg,spr,linets,final);
	portFC.driver->SLHook(bg,spr,linets,final);
}

//binds JPorts[pad] to the driver specified in JPType[pad]
static void SetInputStuff(int port)
{
	switch(joyports[port].type)
	{
		case SI_GAMEPAD:
			if(GameInfo->type==GIT_VSUNI)
				joyports[port].driver = &GPCVS;
			else
				joyports[port].driver= &GPC;
			break;
		case SI_ARKANOID:
			joyports[port].driver=FCEU_InitArkanoid(port);
			break;
		case SI_ZAPPER:
			joyports[port].driver=FCEU_InitZapper(port);
			break;
		case SI_POWERPADA:
			joyports[port].driver=FCEU_InitPowerpadA(port);
			break;
		case SI_POWERPADB:
			joyports[port].driver=FCEU_InitPowerpadB(port);
			break;
		case SI_NONE:
			joyports[port].driver=&DummyJPort;
			break;
	}
}

static void SetInputStuffFC()
{
	switch(portFC.type)
	{
		case SIFC_NONE: 
			portFC.driver=FCEU_InitFamicom3D(); //CAK: originally this used &DummyPortFC;
			break;
		case SIFC_ARKANOID:
			portFC.driver=FCEU_InitArkanoidFC();
			break;
		case SIFC_SHADOW:
			portFC.driver=FCEU_InitSpaceShadow();
			break;
		case SIFC_OEKAKIDS:
			portFC.driver=FCEU_InitOekaKids();
			break;
		case SIFC_4PLAYER:
			portFC.driver=&FAMI4C;
			memset(&F4ReadBit,0,sizeof(F4ReadBit));
			break;
		case SIFC_FKB:
			portFC.driver=FCEU_InitFKB();
			break;
		case SIFC_SUBORKB:
			portFC.driver=FCEU_InitSuborKB();
			break;
		case SIFC_HYPERSHOT:
			portFC.driver=FCEU_InitHS();
			break;
		case SIFC_MAHJONG:
			portFC.driver=FCEU_InitMahjong();
			break;
		case SIFC_QUIZKING:
			portFC.driver=FCEU_InitQuizKing();
			break;
		case SIFC_FTRAINERA:
			portFC.driver=FCEU_InitFamilyTrainerA();
			break;
		case SIFC_FTRAINERB:
			portFC.driver=FCEU_InitFamilyTrainerB();
			break;
		case SIFC_BWORLD:
			portFC.driver=FCEU_InitBarcodeWorld();
			break;
		case SIFC_TOPRIDER:
			portFC.driver=FCEU_InitTopRider();
			break;
	}
}

void FCEUI_SetInput(int port, ESI type, void *ptr, int attrib)
{
	joyports[port].attrib = attrib;
	joyports[port].type = type;
	joyports[port].ptr = ptr;
	SetInputStuff(port);
}

void FCEUI_SetInputFC(ESIFC type, void *ptr, int attrib)
{
	portFC.attrib = attrib;
	portFC.type = type;
	portFC.ptr = ptr;
	SetInputStuffFC();
}


//initializes the input system to power-on state
void InitializeInput(void)
{
	memset(joy_readbit,0,sizeof(joy_readbit));
	memset(joy,0,sizeof(joy));
	LastStrobe = 0;

	if(GameInfo->type==GIT_VSUNI)
	{
		SetReadHandler(0x4016,0x4016,VSUNIRead0);
		SetReadHandler(0x4017,0x4017,VSUNIRead1);
	}
	else
		SetReadHandler(0x4016,0x4017,JPRead);

	SetWriteHandler(0x4016,0x4016,B4016);

	//force the port drivers to be setup
	SetInputStuff(0);
	SetInputStuff(1);
	SetInputStuffFC();
}


bool FCEUI_GetInputFourscore()
{
	return FSAttached;
}
bool FCEUI_GetInputMicrophone()
{
	return replaceP2StartWithMicrophone;
}
void FCEUI_SetInputFourscore(bool attachFourscore)
{
	FSAttached = attachFourscore;
}

//mbg 6/18/08 HACK
extern ZAPPER ZD[2];
SFORMAT FCEUCTRL_STATEINFO[]={
	{ joy_readbit,	2, "JYRB"},
	{ joy,			4, "JOYS"},
	{ &LastStrobe,	1, "LSTS"},
	{ &ZD[0].bogo,	1, "ZBG0"},
	{ &ZD[1].bogo,	1, "ZBG1"},
	{ 0, 4, "FRAM"},
	{ 0 }
};

void FCEU_DoSimpleCommand(int cmd)
{
	switch(cmd)
	{
		case FCEUNPCMD_FDSINSERT:
			FCEU_FDSInsert();
			break;
		case FCEUNPCMD_FDSSELECT:
			FCEU_FDSSelect();
			break;
		case FCEUNPCMD_VSUNICOIN:
			FCEU_VSUniCoin();
			break;
		case FCEUNPCMD_VSUNIDIP0: 
		case FCEUNPCMD_VSUNIDIP0+1:
		case FCEUNPCMD_VSUNIDIP0+2:
		case FCEUNPCMD_VSUNIDIP0+3:
		case FCEUNPCMD_VSUNIDIP0+4:
		case FCEUNPCMD_VSUNIDIP0+5:
		case FCEUNPCMD_VSUNIDIP0+6:
		case FCEUNPCMD_VSUNIDIP0+7:
			FCEU_VSUniToggleDIP(cmd - FCEUNPCMD_VSUNIDIP0);
			break;
		case FCEUNPCMD_POWER:
			PowerNES();
			break;
		case FCEUNPCMD_RESET:
			ResetNES();
			break;
	}
}

void FCEU_QSimpleCommand(int cmd)
{
	FCEU_DoSimpleCommand(cmd);
}

void FCEUI_FDSSelect(void)
{
	FCEU_QSimpleCommand(FCEUNPCMD_FDSSELECT);
}

void FCEUI_FDSInsert(void)
{
	FCEU_QSimpleCommand(FCEUNPCMD_FDSINSERT);
}


void FCEUI_VSUniToggleDIP(int w)
{
	FCEU_QSimpleCommand(FCEUNPCMD_VSUNIDIP0 + w);
}

void FCEUI_VSUniCoin(void)
{
	FCEU_QSimpleCommand(FCEUNPCMD_VSUNICOIN);
}

//Resets the NES
void FCEUI_ResetNES(void)
{
	if(!FCEU_IsValidUI(FCEUI_RESET))
		return;
	FCEU_QSimpleCommand(FCEUNPCMD_RESET);
}

//Powers off the NES
void FCEUI_PowerNES(void)
{
	if(!FCEU_IsValidUI(FCEUI_POWER))
		return;
	FCEU_QSimpleCommand(FCEUNPCMD_POWER);
}

const char* FCEUI_CommandTypeNames[]=
{
	"Misc.",
	"Speed",
	"State",
	"Movie",
	"Sound",
	"AVI",
	"FDS",
	"VS Sys",
	"Tools",
};

static void CommandUnImpl(void);
static void CommandToggleDip(void);
static void CommandStateLoad(void);
static void CommandStateSave(void);
static void CommandSelectSaveSlot(void);
static void ViewSlots(void);

struct EMUCMDTABLE FCEUI_CommandTable[]=
{
	{ EMUCMD_POWER,							EMUCMDTYPE_MISC,	FCEUI_PowerNES,		  0, 0, "Power", 0 },
	{ EMUCMD_RESET,							EMUCMDTYPE_MISC,	FCEUI_ResetNES,		  0, 0, "Reset", 0 },
	{ EMUCMD_SAVE_SLOT_0,					EMUCMDTYPE_STATE,	CommandSelectSaveSlot, 0, 0, "Savestate Slot 0", 0 },
	{ EMUCMD_SAVE_SLOT_1,					EMUCMDTYPE_STATE,	CommandSelectSaveSlot, 0, 0, "Savestate Slot 1", 0 },
	{ EMUCMD_SAVE_SLOT_2,					EMUCMDTYPE_STATE,	CommandSelectSaveSlot, 0, 0, "Savestate Slot 2", 0 },
	{ EMUCMD_SAVE_SLOT_3,					EMUCMDTYPE_STATE,	CommandSelectSaveSlot, 0, 0, "Savestate Slot 3", 0 },
	{ EMUCMD_SAVE_SLOT_4,					EMUCMDTYPE_STATE,	CommandSelectSaveSlot, 0, 0, "Savestate Slot 4", 0 },
	{ EMUCMD_SAVE_SLOT_5,					EMUCMDTYPE_STATE,	CommandSelectSaveSlot, 0, 0, "Savestate Slot 5", 0 },
	{ EMUCMD_SAVE_SLOT_6,					EMUCMDTYPE_STATE,	CommandSelectSaveSlot, 0, 0, "Savestate Slot 6", 0 },
	{ EMUCMD_SAVE_SLOT_7,					EMUCMDTYPE_STATE,	CommandSelectSaveSlot, 0, 0, "Savestate Slot 7", 0 },
	{ EMUCMD_SAVE_SLOT_8,					EMUCMDTYPE_STATE,	CommandSelectSaveSlot, 0, 0, "Savestate Slot 8", 0 },
	{ EMUCMD_SAVE_SLOT_9,					EMUCMDTYPE_STATE,	CommandSelectSaveSlot, 0, 0, "Savestate Slot 9", 0 },
	{ EMUCMD_SAVE_SLOT_NEXT,				EMUCMDTYPE_STATE,	CommandSelectSaveSlot, 0, 0, "Next Savestate Slot", 0 },
	{ EMUCMD_SAVE_SLOT_PREV,				EMUCMDTYPE_STATE,	CommandSelectSaveSlot, 0, 0, "Previous Savestate Slot", 0 },
	{ EMUCMD_SAVE_STATE,					EMUCMDTYPE_STATE,	CommandStateSave, 0, 0, "Save State", 0 },
	{ EMUCMD_SAVE_STATE_SLOT_0,				EMUCMDTYPE_STATE,	CommandStateSave, 0, 0, "Save State to Slot 0", 0 },
	{ EMUCMD_SAVE_STATE_SLOT_1,				EMUCMDTYPE_STATE,	CommandStateSave, 0, 0, "Save State to Slot 1", 0 },
	{ EMUCMD_SAVE_STATE_SLOT_2,				EMUCMDTYPE_STATE,	CommandStateSave, 0, 0, "Save State to Slot 2", 0 },
	{ EMUCMD_SAVE_STATE_SLOT_3,				EMUCMDTYPE_STATE,	CommandStateSave, 0, 0, "Save State to Slot 3", 0 },
	{ EMUCMD_SAVE_STATE_SLOT_4,				EMUCMDTYPE_STATE,	CommandStateSave, 0, 0, "Save State to Slot 4", 0 },
	{ EMUCMD_SAVE_STATE_SLOT_5,				EMUCMDTYPE_STATE,	CommandStateSave, 0, 0, "Save State to Slot 5", 0 },
	{ EMUCMD_SAVE_STATE_SLOT_6,				EMUCMDTYPE_STATE,	CommandStateSave, 0, 0, "Save State to Slot 6", 0 },
	{ EMUCMD_SAVE_STATE_SLOT_7,				EMUCMDTYPE_STATE,	CommandStateSave, 0, 0, "Save State to Slot 7", 0 },
	{ EMUCMD_SAVE_STATE_SLOT_8,				EMUCMDTYPE_STATE,	CommandStateSave, 0, 0, "Save State to Slot 8", 0 },
	{ EMUCMD_SAVE_STATE_SLOT_9,				EMUCMDTYPE_STATE,	CommandStateSave, 0, 0, "Save State to Slot 9", 0 },
	{ EMUCMD_LOAD_STATE,					EMUCMDTYPE_STATE,	CommandStateLoad, 0, 0, "Load State", 0 },
	{ EMUCMD_LOAD_STATE_SLOT_0,				EMUCMDTYPE_STATE,	CommandStateLoad, 0, 0, "Load State from Slot 0", 0 },
	{ EMUCMD_LOAD_STATE_SLOT_1,				EMUCMDTYPE_STATE,	CommandStateLoad, 0, 0, "Load State from Slot 1", 0 },
	{ EMUCMD_LOAD_STATE_SLOT_2,				EMUCMDTYPE_STATE,	CommandStateLoad, 0, 0, "Load State from Slot 2", 0 },
	{ EMUCMD_LOAD_STATE_SLOT_3,				EMUCMDTYPE_STATE,	CommandStateLoad, 0, 0, "Load State from Slot 3", 0 },
	{ EMUCMD_LOAD_STATE_SLOT_4,				EMUCMDTYPE_STATE,	CommandStateLoad, 0, 0, "Load State from Slot 4", 0 },
	{ EMUCMD_LOAD_STATE_SLOT_5,				EMUCMDTYPE_STATE,	CommandStateLoad, 0, 0, "Load State from Slot 5", 0 },
	{ EMUCMD_LOAD_STATE_SLOT_6,				EMUCMDTYPE_STATE,	CommandStateLoad, 0, 0, "Load State from Slot 6", 0 },
	{ EMUCMD_LOAD_STATE_SLOT_7,				EMUCMDTYPE_STATE,	CommandStateLoad, 0, 0, "Load State from Slot 7", 0 },
	{ EMUCMD_LOAD_STATE_SLOT_8,				EMUCMDTYPE_STATE,	CommandStateLoad, 0, 0, "Load State from Slot 8", 0 },
	{ EMUCMD_LOAD_STATE_SLOT_9,				EMUCMDTYPE_STATE,	CommandStateLoad, 0, 0, "Load State from Slot 9", 0 },

	{ EMUCMD_SOUND_TOGGLE,					EMUCMDTYPE_SOUND,	FCEUD_SoundToggle, 0, 0, "Sound Mute Toggle", 0},

	{ EMUCMD_FDS_EJECT_INSERT,				EMUCMDTYPE_FDS,		FCEUI_FDSInsert, 0, 0, "Eject or Insert FDS Disk", 0 },
	{ EMUCMD_FDS_SIDE_SELECT,				EMUCMDTYPE_FDS,		FCEUI_FDSSelect, 0, 0, "Switch FDS Disk Side", 0 },

	{ EMUCMD_VSUNI_COIN,					EMUCMDTYPE_VSUNI,	FCEUI_VSUniCoin, 0, 0, "Insert Coin", 0 },
	{ EMUCMD_VSUNI_TOGGLE_DIP_0,			EMUCMDTYPE_VSUNI,	CommandToggleDip, 0, 0, "Toggle Dipswitch 0", 0 },
	{ EMUCMD_VSUNI_TOGGLE_DIP_1,			EMUCMDTYPE_VSUNI,	CommandToggleDip, 0, 0, "Toggle Dipswitch 1", 0 },
	{ EMUCMD_VSUNI_TOGGLE_DIP_2,			EMUCMDTYPE_VSUNI,	CommandToggleDip, 0, 0, "Toggle Dipswitch 2", 0 },
	{ EMUCMD_VSUNI_TOGGLE_DIP_3,			EMUCMDTYPE_VSUNI,	CommandToggleDip, 0, 0, "Toggle Dipswitch 3", 0 },
	{ EMUCMD_VSUNI_TOGGLE_DIP_4,			EMUCMDTYPE_VSUNI,	CommandToggleDip, 0, 0, "Toggle Dipswitch 4", 0 },
	{ EMUCMD_VSUNI_TOGGLE_DIP_5,			EMUCMDTYPE_VSUNI,	CommandToggleDip, 0, 0, "Toggle Dipswitch 5", 0 },
	{ EMUCMD_VSUNI_TOGGLE_DIP_6,			EMUCMDTYPE_VSUNI,	CommandToggleDip, 0, 0, "Toggle Dipswitch 6", 0 },
	{ EMUCMD_VSUNI_TOGGLE_DIP_7,			EMUCMDTYPE_VSUNI,	CommandToggleDip, 0, 0, "Toggle Dipswitch 7", 0 },
	{ EMUCMD_VSUNI_TOGGLE_DIP_8,			EMUCMDTYPE_VSUNI,	CommandToggleDip, 0, 0, "Toggle Dipswitch 8", 0 },
	{ EMUCMD_VSUNI_TOGGLE_DIP_9,			EMUCMDTYPE_VSUNI,	CommandToggleDip, 0, 0, "Toggle Dipswitch 9", 0 },
	{ EMUCMD_MISC_AUTOSAVE,					EMUCMDTYPE_MISC,	FCEUI_Autosave,   0, 0, "Load Last Auto-save", 0},
	{ EMUCMD_MISC_SHOWSTATES,				EMUCMDTYPE_MISC,	ViewSlots,        0, 0, "View save slots",    0 },
};

#define NUM_EMU_CMDS		(sizeof(FCEUI_CommandTable)/sizeof(FCEUI_CommandTable[0]))

static int execcmd, i;

void FCEUI_HandleEmuCommands(TestCommandState* testfn)
{
	for(i = 0; i < NUM_EMU_CMDS; ++i)
	{
		int new_state;
		int old_state = FCEUI_CommandTable[i].state;
		execcmd = FCEUI_CommandTable[i].cmd;
		new_state = (*testfn)(execcmd);

		if (new_state == 1 && old_state == 0 && FCEUI_CommandTable[i].fn_on)
			(*(FCEUI_CommandTable[i].fn_on))();
		else if (new_state == 0 && old_state == 1 && FCEUI_CommandTable[i].fn_off)
			(*(FCEUI_CommandTable[i].fn_off))();
		FCEUI_CommandTable[i].state = new_state;
	}
}

static void CommandUnImpl(void)
{
	//FCEU_DispMessage("command '%s' unimplemented.",0, FCEUI_CommandTable[i].name);
}

static void CommandToggleDip(void)
{
	if (GameInfo->type==GIT_VSUNI)
		FCEUI_VSUniToggleDIP(execcmd-EMUCMD_VSUNI_TOGGLE_DIP_0);
}

void FCEUI_SelectStateNext(int);

static void ViewSlots(void)
{
	FCEUI_SelectState(CurrentState, 1);
}

static void CommandSelectSaveSlot(void)
{
	if(execcmd <= EMUCMD_SAVE_SLOT_9)
		FCEUI_SelectState(execcmd-EMUCMD_SAVE_SLOT_0, 1);
	else if(execcmd == EMUCMD_SAVE_SLOT_NEXT)
		FCEUI_SelectStateNext(1);
	else if(execcmd == EMUCMD_SAVE_SLOT_PREV)
		FCEUI_SelectStateNext(-1);
}

static void CommandStateSave(void)
{
	if(execcmd >= EMUCMD_SAVE_STATE_SLOT_0 && execcmd <= EMUCMD_SAVE_STATE_SLOT_9)
	{
		int oldslot=FCEUI_SelectState(execcmd-EMUCMD_SAVE_STATE_SLOT_0, 0);
		FCEUI_SaveState(0);
		FCEUI_SelectState(oldslot, 0);
	}
	else
		FCEUI_SaveState(0);
}

static void CommandStateLoad(void)
{
	if(execcmd >= EMUCMD_LOAD_STATE_SLOT_0 && execcmd <= EMUCMD_LOAD_STATE_SLOT_9)
	{
		int oldslot=FCEUI_SelectState(execcmd-EMUCMD_LOAD_STATE_SLOT_0, 0);
		FCEUI_LoadState(0);
		FCEUI_SelectState(oldslot, 0);
	}
	else
		FCEUI_LoadState(0);
}
