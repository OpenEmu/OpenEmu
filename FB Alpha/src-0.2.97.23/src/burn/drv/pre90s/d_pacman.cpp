// FB Alpha Puckman module
// Based on MAME driver by Nicola Salmoria and many others

// Fix Shoot the Bull inputs

#include "tiles_generic.h"
#include "bitswap.h"
#include "sn76496.h"
#include "namco_snd.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvQROM;
static UINT8 *DrvGfxROM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvColRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvSprRAM2;
static UINT8 *DrvColPROM;
static UINT8 *DrvTransTable;

static INT16 *pAY8910Buffer[3];
static UINT32 *Palette;
static UINT8 DrvRecalc;

static UINT8 DrvReset;
static UINT8 DrvJoy1[8] = {0,0,0,0,0,0,0,0};
static UINT8 DrvJoy2[8] = {0,0,0,0,0,0,0,0};
static UINT8 DrvInputs[2] = {0,0};
static UINT8 DrvDips[4] = {0,0,0,0};
static INT16 DrvAxis[2] = { 0, 0 };
static INT16 nAnalogAxis[2] = {0,0};
static UINT8 nCharAxis[2] = {0,0};

enum { PACMAN=0, MSPACMAN, CANNONBP, MAKETRAX, PIRANHA, VANVAN, NMOUSE, DREMSHPR, 
       MSCHAMP, BIGBUCKS, ROCKTRV2, ALIBABA, CRUSHS, SHOOTBUL, BIRDIY, EPOS, PENGO };

static INT32 game_select;
static INT32 acitya = 0;

static UINT8 *flipscreen;

static INT32 interrupt_mode;
static INT32 interrupt_mask;

static UINT8 colortablebank;
static UINT8 palettebank;
static UINT8 spritebank;
static UINT8 charbank;
static INT32 nPacBank;

static INT32 alibaba_mystery;
static UINT8 *rocktrv2_prot_data;
static INT8  epos_hardware_counter;
static UINT8 mschamp_counter;
static UINT8 cannonb_bit_to_read;

static UINT32 watchdog;

//------------------------------------------------------------------------------------------------------

static struct BurnInputInfo DrvInputList[] = {
	{"Coin 1",		  BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"},
	{"Coin 2",		  BIT_DIGITAL,	DrvJoy1 + 6,	"p2 coin"},
	{"Start 1",		  BIT_DIGITAL,	DrvJoy2 + 5,	"p1 start"},
	{"Start 2",		  BIT_DIGITAL,	DrvJoy2 + 6, 	"p2 start"},

	{"P1 Up",		  BIT_DIGITAL,	DrvJoy1 + 0, "p1 up"},
	{"P1 Left",		  BIT_DIGITAL,	DrvJoy1 + 1, "p1 left"},
	{"P1 Right",	  	  BIT_DIGITAL,	DrvJoy1 + 2, "p1 right"},
	{"P1 Down",		  BIT_DIGITAL,	DrvJoy1 + 3, "p1 down"},

	{"P2 Up",		  BIT_DIGITAL,	DrvJoy2 + 0, "p2 up"},
	{"P2 Left",		  BIT_DIGITAL,	DrvJoy2 + 1, "p2 left"},
	{"P2 Right",	  	  BIT_DIGITAL,	DrvJoy2 + 2, "p2 right"},
	{"P2 Down",		  BIT_DIGITAL,	DrvJoy2 + 3, "p2 down"},

	{"Reset",		  BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Service Mode",	  BIT_DIGITAL,	DrvJoy1 + 7,	"diag"},

	{"Dip Switches 1 ",	BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
	{"Dip Switches 4",	BIT_DIPSWITCH,	DrvDips + 3,	"dip"},
};

STDINPUTINFO(Drv)

static struct BurnInputInfo lizwizInputList[] = {
	{"Coin 1",		  BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"},
	{"Coin 2",		  BIT_DIGITAL,	DrvJoy1 + 7,	"p2 coin"},
	{"Start 1",		  BIT_DIGITAL,	DrvJoy2 + 5,	"p1 start"},
	{"Start 2",		  BIT_DIGITAL,	DrvJoy2 + 6, 	"p2 start"},

	{"P1 Up",		  BIT_DIGITAL,	DrvJoy1 + 0, "p1 up"},
	{"P1 Left",		  BIT_DIGITAL,	DrvJoy1 + 1, "p1 left"},
	{"P1 Right",	  	  BIT_DIGITAL,	DrvJoy1 + 2, "p1 right"},
	{"P1 Down",		  BIT_DIGITAL,	DrvJoy1 + 3, "p1 down"},
	{"P1 Button 1",	  BIT_DIGITAL,	DrvJoy2 + 4, "p1 fire 1"},

	{"P2 Up",		  BIT_DIGITAL,	DrvJoy2 + 0, "p2 up"},
	{"P2 Left",		  BIT_DIGITAL,	DrvJoy2 + 1, "p2 left"},
	{"P2 Right",	  	  BIT_DIGITAL,	DrvJoy2 + 2, "p2 right"},
	{"P2 Down",		  BIT_DIGITAL,	DrvJoy2 + 3, "p2 down"},
	{"P2 Button 1",	  BIT_DIGITAL,	DrvJoy2 + 7, "p2 fire 1"},

	{"Reset",		  BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Service Mode",	  BIT_DIGITAL,	DrvJoy1 + 4,	"diag"},
	{"Tilt",	 	 BIT_DIGITAL,	DrvJoy1 + 6,	"tilt"},

	{"Dip Switches 1 ",	BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
	{"Dip Switches 4",	BIT_DIPSWITCH,	DrvDips + 3,	"dip"},
};

STDINPUTINFO(lizwiz)

static struct BurnInputInfo PengoInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 coin"},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 start"},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 fire 1"},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 coin"},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 start"},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 fire 1"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip D",		BIT_DIPSWITCH,	DrvDips + 3,	"dip"},
};

STDINPUTINFO(Pengo)

static struct BurnInputInfo mschampInputList[] = {
	{"Coin 1",		  BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"},
	{"Coin 2",		  BIT_DIGITAL,	DrvJoy1 + 6,	"p2 coin"},
	{"Coin 3",		  BIT_DIGITAL,	DrvJoy1 + 7,	"p3 coin"},
	{"Start 1",		  BIT_DIGITAL,	DrvJoy2 + 5,	"p1 start"},
	{"Start 2",		  BIT_DIGITAL,	DrvJoy2 + 6, 	"p2 start"},

	{"P1 Up",		  BIT_DIGITAL,	DrvJoy1 + 0, "p1 up"},
	{"P1 Left",		  BIT_DIGITAL,	DrvJoy1 + 1, "p1 left"},
	{"P1 Right",	  	  BIT_DIGITAL,	DrvJoy1 + 2, "p1 right"},
	{"P1 Down",		  BIT_DIGITAL,	DrvJoy1 + 3, "p1 down"},

	{"P2 Up",		  BIT_DIGITAL,	DrvJoy2 + 0, "p2 up"},
	{"P2 Left",		  BIT_DIGITAL,	DrvJoy2 + 1, "p2 left"},
	{"P2 Right",	  	  BIT_DIGITAL,	DrvJoy2 + 2, "p2 right"},
	{"P2 Down",		  BIT_DIGITAL,	DrvJoy2 + 3, "p2 down"},

	{"Reset",		  BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Dip Switches 1",	BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
	{"Dip Switches 4",	BIT_DIPSWITCH,	DrvDips + 3,	"dip"},
};

STDINPUTINFO(mschamp)

static struct BurnInputInfo eyesInputList[] = {
	{"Coin 1",		  BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"},
	{"Coin 2",		  BIT_DIGITAL,	DrvJoy1 + 6,	"p2 coin"},
	{"Start 1",		  BIT_DIGITAL,	DrvJoy2 + 5,	"p1 start"},
	{"Start 2",		  BIT_DIGITAL,	DrvJoy2 + 7, 	"p2 start"},

	{"P1 Up",		  BIT_DIGITAL,	DrvJoy1 + 0, "p1 up"},
	{"P1 Left",		  BIT_DIGITAL,	DrvJoy1 + 1, "p1 left"},
	{"P1 Right",	  	  BIT_DIGITAL,	DrvJoy1 + 2, "p1 right"},
	{"P1 Down",		  BIT_DIGITAL,	DrvJoy1 + 3, "p1 down"},
	{"P1 Button 1",		  BIT_DIGITAL,  DrvJoy2 + 4, "p1 fire 1"},

	{"P2 Up",		  BIT_DIGITAL,	DrvJoy2 + 0, "p2 up"},
	{"P2 Left",		  BIT_DIGITAL,	DrvJoy2 + 1, "p2 left"},
	{"P2 Right",	  	  BIT_DIGITAL,	DrvJoy2 + 2, "p2 right"},
	{"P2 Down",		  BIT_DIGITAL,	DrvJoy2 + 3, "p2 down"},
	{"P2 Button 1",		  BIT_DIGITAL,  DrvJoy2 + 7, "p2 fire 1"},

	{"Reset",		  BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Tilt",		  BIT_DIGITAL,	DrvJoy1 + 6,	"tilt"},
	{"Dip Switches 1",	BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
};

STDINPUTINFO(eyes)

static struct BurnInputInfo theglobpInputList[] = {
	{"Coin",		  BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"},

	{"P1 Up",		  BIT_DIGITAL,	DrvJoy1 + 0, "p1 up"},
	{"P1 Left",		  BIT_DIGITAL,	DrvJoy1 + 1, "p1 left"},
	{"P1 Right",	  	  BIT_DIGITAL,	DrvJoy1 + 2, "p1 right"},
	{"P1 Down",		  BIT_DIGITAL,	DrvJoy1 + 3, "p1 down"},
	{"P1 Button 1",		  BIT_DIGITAL,	DrvJoy2 + 5, "p1 fire 1"},
	{"P1 Button 2",		  BIT_DIGITAL,	DrvJoy2 + 6, "p1 fire 2"},

	{"P2 Up",		  BIT_DIGITAL,	DrvJoy2 + 0, "p2 up"},
	{"P2 Left",		  BIT_DIGITAL,	DrvJoy2 + 1, "p2 left"},
	{"P2 Right",	  	  BIT_DIGITAL,	DrvJoy2 + 2, "p2 right"},
	{"P2 Down",		  BIT_DIGITAL,	DrvJoy2 + 3, "p2 down"},
	{"P1 Button 1",		  BIT_DIGITAL,	DrvJoy2 + 4, "p2 fire 1"},
	{"P1 Button 2",		  BIT_DIGITAL,	DrvJoy1 + 7, "p2 fire 2"},

	{"Reset",		  BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Service Mode",	  BIT_DIGITAL,	DrvJoy1 + 7,	"diag"},

	{"Dip Switches 1",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
};

STDINPUTINFO(theglobp)

static struct BurnInputInfo ponpokoInputList[] = {
	{"Coin 1",		  BIT_DIGITAL,	DrvJoy1 + 5, "p1 coin"},
	{"Coin 2",		  BIT_DIGITAL,	DrvJoy1 + 6, "p2 coin"},
	{"Start 1",		  BIT_DIGITAL,	DrvJoy2 + 5, "p1 start"},
	{"Start 2",		  BIT_DIGITAL,	DrvJoy2 + 6, "p2 start"},

	{"P1 Up",		  BIT_DIGITAL,	DrvJoy1 + 0, "p1 up"},
	{"P1 Left",		  BIT_DIGITAL,	DrvJoy1 + 1, "p1 left"},
	{"P1 Right",	  	  BIT_DIGITAL,	DrvJoy1 + 2, "p1 right"},
	{"P1 Down",		  BIT_DIGITAL,	DrvJoy1 + 3, "p1 down"},
	{"P1 button 1",		  BIT_DIGITAL,  DrvJoy1 + 4, "p1 fire 1"},

	{"P2 Up",		  BIT_DIGITAL,	DrvJoy2 + 0, "p2 up"},
	{"P2 Left",		  BIT_DIGITAL,	DrvJoy2 + 1, "p2 left"},
	{"P2 Right",	  	  BIT_DIGITAL,	DrvJoy2 + 2, "p2 right"},
	{"P2 Down",		  BIT_DIGITAL,	DrvJoy2 + 3, "p2 down"},
	{"P2 button 1",		  BIT_DIGITAL,  DrvJoy2 + 4, "p1 fire 1"},

	{"Reset",		  BIT_DIGITAL,	&DrvReset,   "reset"},
	{"Service Mode",	  BIT_DIGITAL,	DrvJoy1 + 7, "diag"},

	{"Dip Switches 1",	BIT_DIPSWITCH,	DrvDips + 2,    "dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 3,    "dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 0,    "dip"},
	{"Dip Switches 4",	BIT_DIPSWITCH,	DrvDips + 1,    "dip"},
};

STDINPUTINFO(ponpoko)

static struct BurnInputInfo vanvanInputList[] = {
	{"Coin",		  BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"},
	{"Start 1",		  BIT_DIGITAL,	DrvJoy2 + 5,	"p1 start"},
	{"Start 2",		  BIT_DIGITAL,	DrvJoy2 + 6, 	"p2 start"},

	{"P1 Up",		  BIT_DIGITAL,	DrvJoy1 + 0, "p1 up"},
	{"P1 Left",		  BIT_DIGITAL,	DrvJoy1 + 1, "p1 left"},
	{"P1 Right",	  	  BIT_DIGITAL,	DrvJoy1 + 2, "p1 right"},
	{"P1 Down",		  BIT_DIGITAL,	DrvJoy1 + 3, "p1 down"},
	{"P1 Button 1",		  BIT_DIGITAL,  DrvJoy1 + 4, "p1 fire 1"},

	{"P2 Up",		  BIT_DIGITAL,	DrvJoy2 + 0, "p2 up"},
	{"P2 Left",		  BIT_DIGITAL,	DrvJoy2 + 1, "p2 left"},
	{"P2 Right",	  	  BIT_DIGITAL,	DrvJoy2 + 2, "p2 right"},
	{"P2 Down",		  BIT_DIGITAL,	DrvJoy2 + 3, "p2 down"},
	{"P2 Button 1",		  BIT_DIGITAL,  DrvJoy2 + 4, "p2 fire 1"},

	{"Reset",		  BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Service Mode",	  BIT_DIGITAL,	DrvJoy1 + 7,	"diag"},

	{"Dip Switches 1",	BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
};

STDINPUTINFO(vanvan)

static struct BurnInputInfo vanvankInputList[] = {
	{"Coin 1",		  BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"},
	{"Coin 2",		  BIT_DIGITAL,	DrvJoy1 + 7,	"p2 coin"},

	{"Start 1",		  BIT_DIGITAL,	DrvJoy2 + 5,	"p1 start"},
	{"Start 2",		  BIT_DIGITAL,	DrvJoy2 + 6, 	"p2 start"},

	{"P1 Up",		  BIT_DIGITAL,	DrvJoy1 + 0, "p1 up"},
	{"P1 Left",		  BIT_DIGITAL,	DrvJoy1 + 1, "p1 left"},
	{"P1 Right",	  	  BIT_DIGITAL,	DrvJoy1 + 2, "p1 right"},
	{"P1 Down",		  BIT_DIGITAL,	DrvJoy1 + 3, "p1 down"},
	{"P1 Button 1",		  BIT_DIGITAL,  DrvJoy1 + 4, "p1 fire 1"},

	{"P2 Up",		  BIT_DIGITAL,	DrvJoy2 + 0, "p2 up"},
	{"P2 Left",		  BIT_DIGITAL,	DrvJoy2 + 1, "p2 left"},
	{"P2 Right",	  	  BIT_DIGITAL,	DrvJoy2 + 2, "p2 right"},
	{"P2 Down",		  BIT_DIGITAL,	DrvJoy2 + 3, "p2 down"},
	{"P2 Button 1",		  BIT_DIGITAL,  DrvJoy2 + 4, "p2 fire 1"},

	{"Reset",		  BIT_DIGITAL,	&DrvReset,	"reset"},

	{"Dip Switches 1",	BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
};

STDINPUTINFO(vanvank)

static struct BurnInputInfo cannonbpInputList[] = {
	{"Coin 1",		  BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"},
	{"Coin 2",		  BIT_DIGITAL,	DrvJoy1 + 6,	"p2 coin"},
	{"Start 1",		  BIT_DIGITAL,	DrvJoy2 + 5,	"p1 start"},
	{"Start 2",		  BIT_DIGITAL,	DrvJoy2 + 6, 	"p2 start"},

	{"P1 Up",		  BIT_DIGITAL,	DrvJoy1 + 0, "p1 up"},
	{"P1 Left",		  BIT_DIGITAL,	DrvJoy1 + 1, "p1 left"},
	{"P1 Right",	  	  BIT_DIGITAL,	DrvJoy1 + 2, "p1 right"},
	{"P1 Down",		  BIT_DIGITAL,	DrvJoy1 + 3, "p1 down"},
	{"P1 Button 1",	  	  BIT_DIGITAL,	DrvJoy1 + 7, "p1 fire 1"},

	{"P2 Up",		  BIT_DIGITAL,	DrvJoy2 + 0, "p2 up"},
	{"P2 Left",		  BIT_DIGITAL,	DrvJoy2 + 1, "p2 left"},
	{"P2 Right",	  	  BIT_DIGITAL,	DrvJoy2 + 2, "p2 right"},
	{"P2 Down",		  BIT_DIGITAL,	DrvJoy2 + 3, "p2 down"},
	{"P2 Button 2",	  	  BIT_DIGITAL,	DrvJoy2 + 4, "p2 fire 1"},

	{"Reset",		  BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Dip Switches 1",	BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
};

STDINPUTINFO(cannonbp)

static struct BurnInputInfo rocktrv2InputList[] = {
	{"Coin 1",		  BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"},
	{"Coin 2",		  BIT_DIGITAL,	DrvJoy1 + 6,	"p2 coin"},
	{"Start 1",		  BIT_DIGITAL,	DrvJoy2 + 5,	"p1 start"},
	{"Start 2",		  BIT_DIGITAL,	DrvJoy2 + 6, 	"p2 start"},

	{"P1 Up",		  BIT_DIGITAL,	DrvJoy1 + 0, "p1 up"},
	{"P1 Left",		  BIT_DIGITAL,	DrvJoy1 + 1, "p1 left"},
	{"P1 Right",	  	  BIT_DIGITAL,	DrvJoy1 + 2, "p1 right"},
	{"P1 Down",		  BIT_DIGITAL,	DrvJoy1 + 3, "p1 down"},

	{"P2 Up",		  BIT_DIGITAL,	DrvJoy2 + 0, "p2 up"},
	{"P2 Left",		  BIT_DIGITAL,	DrvJoy2 + 1, "p2 left"},
	{"P2 Right",	  	  BIT_DIGITAL,	DrvJoy2 + 2, "p2 right"},
	{"P2 Down",		  BIT_DIGITAL,	DrvJoy2 + 3, "p2 down"},

	{"Tilt",	  	BIT_DIGITAL,	DrvJoy1 + 7,	"diag"},
	{"Reset",		  BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Dip Switches 1",	BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 3,	"dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
	{"Dip Switches 4",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
};

STDINPUTINFO(rocktrv2)

static struct BurnInputInfo bigbucksInputList[] = {
	{"Coin 1",		  BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"},
	{"Coin 2",		  BIT_DIGITAL,	DrvJoy1 + 6,	"p2 coin"},

	{"P1 Up",		  BIT_DIGITAL,	DrvJoy1 + 0, "p1 up"},
	{"P1 Left",		  BIT_DIGITAL,	DrvJoy1 + 1, "p1 left"},
	{"P1 Right",	  	  BIT_DIGITAL,	DrvJoy1 + 2, "p1 right"},
	{"P1 Down",		  BIT_DIGITAL,	DrvJoy1 + 3, "p1 down"},

	{"P2 Up",		  BIT_DIGITAL,	DrvJoy2 + 0, "p2 up"},
	{"P2 Left",		  BIT_DIGITAL,	DrvJoy2 + 1, "p2 left"},
	{"P2 Right",	  	  BIT_DIGITAL,	DrvJoy2 + 2, "p2 right"},
	{"P2 Down",		  BIT_DIGITAL,	DrvJoy2 + 3, "p2 down"},

	{"Reset",		  BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Service Mode",	  BIT_DIGITAL,	DrvJoy1 + 7,	"diag"},

	{"Dip Switches 1",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
};

STDINPUTINFO(bigbucks)

static struct BurnInputInfo woodpekInputList[] = {
	{"Coin 1",		  BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"},
	{"Coin 2",		  BIT_DIGITAL,	DrvJoy1 + 6,	"p2 coin"},
	{"Start 1",		  BIT_DIGITAL,	DrvJoy2 + 5,	"p1 start"},
	{"Start 2",		  BIT_DIGITAL,	DrvJoy2 + 6, 	"p2 start"},

	{"P1 Up",		  BIT_DIGITAL,	DrvJoy1 + 0, "p1 up"},
	{"P1 Left",		  BIT_DIGITAL,	DrvJoy1 + 1, "p1 left"},
	{"P1 Right",	  	  BIT_DIGITAL,	DrvJoy1 + 2, "p1 right"},
	{"P1 Down",		  BIT_DIGITAL,	DrvJoy1 + 3, "p1 down"},
	{"P1 Button 1",		  BIT_DIGITAL,  DrvJoy2 + 4, "p1 fire 1"},

	{"P2 Up",		  BIT_DIGITAL,	DrvJoy2 + 0, "p2 up"},
	{"P2 Left",		  BIT_DIGITAL,	DrvJoy2 + 1, "p2 left"},
	{"P2 Right",	  	  BIT_DIGITAL,	DrvJoy2 + 2, "p2 right"},
	{"P2 Down",		  BIT_DIGITAL,	DrvJoy2 + 3, "p2 down"},
	{"P2 Button 1",		  BIT_DIGITAL,  DrvJoy2 + 7, "p2 fire 2"},

	{"Reset",		  BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Service Mode",	  BIT_DIGITAL,	DrvJoy1 + 7,	"diag"},

	{"Dip Switches 1",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
};

STDINPUTINFO(woodpek)

static struct BurnInputInfo acityaInputList[] = {
	{"Coin 1",		  BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"},

	{"P1 Button 1",		  BIT_DIGITAL,	DrvJoy1 + 0, "p1 fire 1"},
	{"P1 Button 2",		  BIT_DIGITAL,	DrvJoy1 + 1, "p1 fire 2"},
	{"P1 Button 3",	  	  BIT_DIGITAL,	DrvJoy1 + 2, "p1 fire 3"},
	{"P1 Button 4",		  BIT_DIGITAL,	DrvJoy1 + 3, "p1 fire 4"},
	{"P1 Button 5",		  BIT_DIGITAL,	DrvJoy2 + 5, "p1 fire 5"},
	{"P1 Button 6",		  BIT_DIGITAL,	DrvJoy2 + 6, "p1 fire 6"},

	{"Service Mode 1",	  BIT_DIGITAL,	DrvJoy1 + 4,	"diag"},
	{"Service Mode 2",	  BIT_DIGITAL,	DrvJoy2 + 4,	"diag"},

	{"Reset",		  BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Dip Switches 1",	BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
};

STDINPUTINFO(acitya)

static struct BurnInputInfo bwcasinoInputList[] = {
	{"Coin 1",		  BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"},

	{"P1 Button 1",		  BIT_DIGITAL,	DrvJoy1 + 0, "p1 fire 1"},
	{"P1 Button 2",		  BIT_DIGITAL,	DrvJoy1 + 1, "p1 fire 2"},
	{"P1 Button 3",	  	  BIT_DIGITAL,	DrvJoy1 + 2, "p1 fire 3"},
	{"P1 Button 4",		  BIT_DIGITAL,	DrvJoy1 + 3, "p1 fire 4"},
	{"P1 Button 5",		  BIT_DIGITAL,	DrvJoy2 + 5, "p1 fire 5"},
	{"P1 Button 6",		  BIT_DIGITAL,	DrvJoy2 + 6, "p1 fire 6"},

	{"P2 Button 1",		  BIT_DIGITAL,	DrvJoy2 + 0, "p2 fire 1"},
	{"P2 Button 2",		  BIT_DIGITAL,	DrvJoy2 + 1, "p2 fire 2"},
	{"P2 Button 3",	  	  BIT_DIGITAL,	DrvJoy2 + 2, "p2 fire 3"},
	{"P2 Button 4",		  BIT_DIGITAL,	DrvJoy2 + 3, "p2 fire 4"},
	{"P2 Button 5",		  BIT_DIGITAL,	DrvJoy2 + 5, "p2 fire 5"},
	{"P2 Button 6",		  BIT_DIGITAL,	DrvJoy1 + 7, "p2 fire 6"},

	{"Service Mode ",	  BIT_DIGITAL,	DrvJoy1 + 4,	"diag"},

	{"Reset",		  BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Dip Switches 1",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
};

STDINPUTINFO(bwcasino)

static struct BurnInputInfo dremshprInputList[] = {
	{"Coin 1",		  BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"},
	{"Coin 2",		  BIT_DIGITAL,	DrvJoy1 + 7,	"p2 coin"},
	{"Start 1",		  BIT_DIGITAL,	DrvJoy2 + 5,	"p1 start"},
	{"Start 2",		  BIT_DIGITAL,	DrvJoy2 + 6, 	"p2 start"},

	{"P1 Up",		  BIT_DIGITAL,	DrvJoy1 + 0, "p1 up"},
	{"P1 Left",		  BIT_DIGITAL,	DrvJoy1 + 1, "p1 left"},
	{"P1 Right",	  	  BIT_DIGITAL,	DrvJoy1 + 2, "p1 right"},
	{"P1 Down",		  BIT_DIGITAL,	DrvJoy1 + 3, "p1 down"},
	{"P1 Button 1",		  BIT_DIGITAL,  DrvJoy1 + 4, "p1 fire 1"},

	{"P2 Up",		  BIT_DIGITAL,	DrvJoy2 + 0, "p2 up"},
	{"P2 Left",		  BIT_DIGITAL,	DrvJoy2 + 1, "p2 left"},
	{"P2 Right",	  	  BIT_DIGITAL,	DrvJoy2 + 2, "p2 right"},
	{"P2 Down",		  BIT_DIGITAL,	DrvJoy2 + 3, "p2 down"},
	{"P2 Button 1",		  BIT_DIGITAL,  DrvJoy2 + 4, "p2 fire "},

	{"Reset",		  BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Dip Switches 1",	BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
	{"Dip Switches 4",        BIT_DIPSWITCH,  DrvDips + 3,    "dip"},
};

STDINPUTINFO(dremshpr)

static struct BurnInputInfo alibabaInputList[] = {
	{"Coin 1",		  BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"},
	{"Coin 2",		  BIT_DIGITAL,	DrvJoy1 + 7,	"p2 coin"},
	{"Start 1",		  BIT_DIGITAL,	DrvJoy2 + 5,	"p1 start"},
	{"Start 2",		  BIT_DIGITAL,	DrvJoy2 + 6, 	"p2 start"},

	{"P1 Up",		  BIT_DIGITAL,	DrvJoy1 + 0, "p1 up"},
	{"P1 Left",		  BIT_DIGITAL,	DrvJoy1 + 1, "p1 left"},
	{"P1 Right",	  	  BIT_DIGITAL,	DrvJoy1 + 2, "p1 right"},
	{"P1 Down",		  BIT_DIGITAL,	DrvJoy1 + 3, "p1 down"},
	{"P1 Button 1",		  BIT_DIGITAL,  DrvJoy1 + 6, "p1 fire 1"},

	{"P2 Up",		  BIT_DIGITAL,	DrvJoy2 + 0, "p2 up"},
	{"P2 Left",		  BIT_DIGITAL,	DrvJoy2 + 1, "p2 left"},
	{"P2 Right",	  	  BIT_DIGITAL,	DrvJoy2 + 2, "p2 right"},
	{"P2 Down",		  BIT_DIGITAL,	DrvJoy2 + 3, "p2 down"},
	{"P2 Button 1",		  BIT_DIGITAL,  DrvJoy2 + 4, "p2 fire 1"},

	{"Reset",		  BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Dip Switches 1",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip Switches 4",	BIT_DIPSWITCH,	DrvDips + 3,	"dip"},
};

STDINPUTINFO(alibaba)

static struct BurnInputInfo jumpshotInputList[] = {
	{"Coin 1",		  BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"},
	{"Coin 2",		  BIT_DIGITAL,	DrvJoy1 + 6,	"p2 coin"},
	{"Start 1",		  BIT_DIGITAL,	DrvJoy2 + 5,	"p1 start"},
	{"Start 2",		  BIT_DIGITAL,	DrvJoy2 + 6, 	"p2 start"},

	{"P1 Up",		  BIT_DIGITAL,	DrvJoy1 + 0, "p1 up"},
	{"P1 Left",		  BIT_DIGITAL,	DrvJoy1 + 1, "p1 left"},
	{"P1 Right",	  	  BIT_DIGITAL,	DrvJoy1 + 2, "p1 right"},
	{"P1 Down",		  BIT_DIGITAL,	DrvJoy1 + 3, "p1 down"},
	{"P1 Button 1",		  BIT_DIGITAL,  DrvJoy2 + 5, "p1 fire 1"},

	{"P2 Up",		  BIT_DIGITAL,	DrvJoy2 + 0, "p2 up"},
	{"P2 Left",		  BIT_DIGITAL,	DrvJoy2 + 1, "p2 left"},
	{"P2 Right",	  	  BIT_DIGITAL,	DrvJoy2 + 2, "p2 right"},
	{"P2 Down",		  BIT_DIGITAL,	DrvJoy2 + 3, "p2 down"},
	{"P2 Button 1",		  BIT_DIGITAL,  DrvJoy2 + 6, "p2 fire 1"},

	{"Reset",		  BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Service Mode",	  BIT_DIGITAL,	DrvJoy1 + 7,	"diag"},

	{"Dip Switches 1",	BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
};

STDINPUTINFO(jumpshot)

static struct BurnInputInfo korosukeInputList[] = {
	{"Coin",		  BIT_DIGITAL,	DrvJoy1 + 6,	"p1 coin"},
	{"Start 1",		  BIT_DIGITAL,	DrvJoy2 + 5,	"p1 start"},
	{"Start 2",		  BIT_DIGITAL,	DrvJoy2 + 6, 	"p2 start"},

	{"P1 Up",		  BIT_DIGITAL,	DrvJoy1 + 0, "p1 up"},
	{"P1 Left",		  BIT_DIGITAL,	DrvJoy1 + 1, "p1 left"},
	{"P1 Right",	  	  BIT_DIGITAL,	DrvJoy1 + 2, "p1 right"},
	{"P1 Down",		  BIT_DIGITAL,	DrvJoy1 + 3, "p1 down"},
	{"P1 Button 1",		  BIT_DIGITAL,	DrvJoy1 + 7, "p1 fire 1"},

	{"P2 Up",		  BIT_DIGITAL,	DrvJoy2 + 0, "p2 up"},
	{"P2 Left",		  BIT_DIGITAL,	DrvJoy2 + 1, "p2 left"},
	{"P2 Right",	  	  BIT_DIGITAL,	DrvJoy2 + 2, "p2 right"},
	{"P2 Down",		  BIT_DIGITAL,	DrvJoy2 + 3, "p2 down"},
	{"P1 Button 1",		  BIT_DIGITAL,	DrvJoy1 + 5, "p2 fire 1"},

	{"Reset",		  BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Service Mode",	  BIT_DIGITAL,	DrvJoy1 + 7,	"diag"},

	{"Dip Switches 1 ",	BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
	{"Dip Switches 4",	BIT_DIPSWITCH,	DrvDips + 3,	"dip"},
};

STDINPUTINFO(korosuke)

#define A(a, b, c, d) { a, b, (UINT8*)(c), d }

static struct BurnInputInfo shootbulInputList[] = {
	{"Coin 1",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"},
	{"Coin 2",		BIT_DIGITAL,	DrvJoy1 + 6,	"p2 coin"},
	{"Coin 3",		BIT_DIGITAL,	DrvJoy1 + 7,	"p3 coin"},
	{"Start",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 start"},

    A("P1 X Axis",		BIT_ANALOG_REL,	DrvAxis + 0,	"mouse x-axis"),
    A("P1 Y Axis",		BIT_ANALOG_REL,	DrvAxis + 1,	"mouse y-axis"),

	{"P1 Button A",		BIT_DIGITAL,	DrvJoy2 + 5,	"mouse button 1"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},

	{"Dip Switches 1",	BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip Switches 2",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
	{"Dip Switches 3",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
};

STDINPUTINFO(shootbul)

#undef A


static struct BurnDIPInfo DrvDIPList[]=
{
	{0x0e, 0xff, 0xff, 0xc9, NULL                     },
	{0x0f, 0xff, 0xff, 0xff, NULL                     },
	{0x10, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0e, 0x01, 0x03, 0x03, "2C 1C"     		  },
	{0x0e, 0x01, 0x03, 0x01, "1C 1C"    		  },
	{0x0e, 0x01, 0x03, 0x02, "1C 2C"     		  },
	{0x0e, 0x01, 0x03, 0x00, "Free Play"     	  },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0e, 0x01, 0x0c, 0x00, "1"     		  },
	{0x0e, 0x01, 0x0c, 0x04, "2"    		  },
	{0x0e, 0x01, 0x0c, 0x08, "3"     		  },
	{0x0e, 0x01, 0x0c, 0x0c, "5"     		  },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0e, 0x01, 0x30, 0x00, "10000"     		  },
	{0x0e, 0x01, 0x30, 0x10, "15000"    		  },
	{0x0e, 0x01, 0x30, 0x20, "20000"     		  },
	{0x0e, 0x01, 0x30, 0x30, "None"     		  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x0e, 0x01, 0x40, 0x40, "Normal"      		  },
	{0x0e, 0x01, 0x40, 0x00, "Hard"       		  },

	{0   , 0xfe, 0   , 2   , "Ghost Names"            },
	{0x0e, 0x01, 0x80, 0x80, "Normal"     		  },
	{0x0e, 0x01, 0x80, 0x00, "Alternate"   		  },

	{0   , 0xfe, 0   , 2   , "Rack Test (Cheat)"      },
	{0x0f, 0x01, 0x10, 0x10, "Off"     		  },
	{0x0f, 0x01, 0x10, 0x00, "On"    		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"	          },
	{0x10, 0x01, 0x80, 0x80, "Upright"     		  },
	{0x10, 0x01, 0x80, 0x00, "Cocktail"    		  },
};

STDDIPINFO(Drv)

static struct BurnDIPInfo PengoDIPList[]=
{
	{0x0f, 0xff, 0xff, 0xff, NULL				},
	{0x10, 0xff, 0xff, 0xff, NULL				},
	{0x11, 0xff, 0xff, 0xb0, NULL				},
	{0x12, 0xff, 0xff, 0xcc, NULL				},

	{0   , 0xfe, 0   ,    1, "Service Mode"			},
	{0x10, 0x01, 0x10, 0x10, "Off"				},

	{0   , 0xfe, 0   ,    2, "Bonus Life"			},
	{0x11, 0x01, 0x01, 0x00, "30000"			},
	{0x11, 0x01, 0x01, 0x01, "50000"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x11, 0x01, 0x02, 0x02, "Off"				},
	{0x11, 0x01, 0x02, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Cabinet"			},
	{0x11, 0x01, 0x04, 0x00, "Upright"			},
	{0x11, 0x01, 0x04, 0x04, "Cocktail"			},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x11, 0x01, 0x18, 0x18, "2"				},
	{0x11, 0x01, 0x18, 0x10, "3"				},
	{0x11, 0x01, 0x18, 0x08, "4"				},
	{0x11, 0x01, 0x18, 0x00, "5"				},

	{0   , 0xfe, 0   ,    2, "Rack Test (Cheat)"		},
	{0x11, 0x01, 0x20, 0x20, "Off"				},
	{0x11, 0x01, 0x20, 0x00, "On"				},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x11, 0x01, 0xc0, 0xc0, "Easy"				},
	{0x11, 0x01, 0xc0, 0x80, "Medium"			},
	{0x11, 0x01, 0xc0, 0x40, "Hard"				},
	{0x11, 0x01, 0xc0, 0x00, "Hardest"			},

	{0   , 0xfe, 0   ,    16, "Coin A"			},
	{0x12, 0x01, 0x0f, 0x00, "4 Coins 1 Credits"		},
	{0x12, 0x01, 0x0f, 0x08, "3 Coins 1 Credits"		},
	{0x12, 0x01, 0x0f, 0x04, "2 Coins 1 Credits"		},
	{0x12, 0x01, 0x0f, 0x09, "2 Coins/1 Credit 5/3"		},
	{0x12, 0x01, 0x0f, 0x05, "2 Coins/1 Credit 4/3"		},
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  1 Credits"		},
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin/1 Credit 5/6"		},
	{0x12, 0x01, 0x0f, 0x03, "1 Coin/1 Credit 4/5"		},
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin/1 Credit 2/3"		},
	{0x12, 0x01, 0x0f, 0x02, "1 Coin  2 Credits"		},
	{0x12, 0x01, 0x0f, 0x07, "1 Coin/2 Credits 5/11"	},
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin/2 Credits 4/9"		},
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  3 Credits"		},
	{0x12, 0x01, 0x0f, 0x06, "1 Coin  4 Credits"		},
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  5 Credits"		},
	{0x12, 0x01, 0x0f, 0x01, "1 Coin  6 Credits"		},

	{0   , 0xfe, 0   ,    16, "Coin B"			},
	{0x12, 0x01, 0xf0, 0x00, "4 Coins 1 Credits"		},
	{0x12, 0x01, 0xf0, 0x80, "3 Coins 1 Credits"		},
	{0x12, 0x01, 0xf0, 0x40, "2 Coins 1 Credits"		},
	{0x12, 0x01, 0xf0, 0x90, "2 Coins/1 Credit 5/3"		},
	{0x12, 0x01, 0xf0, 0x50, "2 Coins/1 Credit 4/3"		},
	{0x12, 0x01, 0xf0, 0xc0, "1 Coin  1 Credits"		},
	{0x12, 0x01, 0xf0, 0xd0, "1 Coin/1 Credit 5/6"		},
	{0x12, 0x01, 0xf0, 0x30, "1 Coin/1 Credit 4/5"		},
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin/1 Credit 2/3"		},
	{0x12, 0x01, 0xf0, 0x20, "1 Coin  2 Credits"		},
	{0x12, 0x01, 0xf0, 0x70, "1 Coin/2 Credits 5/11"	},
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin/2 Credits 4/9"		},
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  3 Credits"		},
	{0x12, 0x01, 0xf0, 0x60, "1 Coin  4 Credits"		},
	{0x12, 0x01, 0xf0, 0xe0, "1 Coin  5 Credits"		},
	{0x12, 0x01, 0xf0, 0x10, "1 Coin  6 Credits"		},
};

STDDIPINFO(Pengo)

static struct BurnDIPInfo mspacmanDIPList[]=
{
	{0x0e, 0xff, 0xff, 0xc9, NULL                     },
	{0x0f, 0xff, 0xff, 0xff, NULL                     },
	{0x10, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0e, 0x01, 0x03, 0x03, "2C 1C"     		  },
	{0x0e, 0x01, 0x03, 0x01, "1C 1C"    		  },
	{0x0e, 0x01, 0x03, 0x02, "1C 2C"     		  },
	{0x0e, 0x01, 0x03, 0x00, "Free Play"     	  },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0e, 0x01, 0x0c, 0x00, "1"     		  },
	{0x0e, 0x01, 0x0c, 0x04, "2"    		  },
	{0x0e, 0x01, 0x0c, 0x08, "3"     		  },
	{0x0e, 0x01, 0x0c, 0x0c, "5"     		  },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0e, 0x01, 0x30, 0x00, "10000"     		  },
	{0x0e, 0x01, 0x30, 0x10, "15000"    		  },
	{0x0e, 0x01, 0x30, 0x20, "20000"     		  },
	{0x0e, 0x01, 0x30, 0x30, "None"     		  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x0e, 0x01, 0x40, 0x40, "Normal"      		  },
	{0x0e, 0x01, 0x40, 0x00, "Hard"       		  },

	{0   , 0xfe, 0   , 2   , "Rack Test (Cheat)"      },
	{0x0f, 0x01, 0x10, 0x10, "Off"     		  },
	{0x0f, 0x01, 0x10, 0x00, "On"    		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"	          },
	{0x10, 0x01, 0x80, 0x80, "Upright"     		  },
	{0x10, 0x01, 0x80, 0x00, "Cocktail"    		  },
};

STDDIPINFO(mspacman)

static struct BurnDIPInfo mschampDIPList[]=
{
	{0x0e, 0xff, 0xff, 0xc9, NULL                     },
	{0x0f, 0xff, 0xff, 0xff, NULL                     },
	{0x10, 0xff, 0xff, 0xff, NULL                     },
	{0x11, 0xff, 0xff, 0x00, NULL                     },

	{0   , 0xfe, 0   , 2   , "Rack Test (Cheat)"      },
	{0x0f, 0x01, 0x10, 0x10, "Off"     		  },
	{0x0f, 0x01, 0x10, 0x00, "On"    		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"	          },
	{0x10, 0x01, 0x80, 0x80, "Upright"     		  },
	{0x10, 0x01, 0x80, 0x00, "Cocktail"    		  },

	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0e, 0x01, 0x03, 0x03, "2C 1C"     		  },
	{0x0e, 0x01, 0x03, 0x01, "1C 1C"    		  },
	{0x0e, 0x01, 0x03, 0x02, "1C 2C"     		  },
	{0x0e, 0x01, 0x03, 0x00, "Free Play"     	  },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0e, 0x01, 0x0c, 0x00, "1"     		  },
	{0x0e, 0x01, 0x0c, 0x04, "2"    		  },
	{0x0e, 0x01, 0x0c, 0x08, "3"     		  },
	{0x0e, 0x01, 0x0c, 0x0c, "5"     		  },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0e, 0x01, 0x30, 0x00, "10000"     		  },
	{0x0e, 0x01, 0x30, 0x10, "15000"    		  },
	{0x0e, 0x01, 0x30, 0x20, "20000"     		  },
	{0x0e, 0x01, 0x30, 0x30, "None"     		  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x0e, 0x01, 0x40, 0x40, "Normal"      		  },
	{0x0e, 0x01, 0x40, 0x00, "Hard"       		  },

	{0   , 0xfe, 0   , 2   , "Game"                   },
	{0x11, 0x01, 0x01, 0x01, "Champion Edition"       },
	{0x11, 0x01, 0x01, 0x00, "Super Zola Pac Gal"     },
};

STDDIPINFO(mschamp)

static struct BurnDIPInfo maketraxDIPList[]=
{
	{0x0e, 0xff, 0x3f, 0x31, NULL                     },
	{0x0f, 0xff, 0xff, 0xef, NULL                     },
	{0x10, 0xff, 0x6f, 0x6f, NULL                     },

	{0   , 0xfe, 0   , 2   , "Cabinet"	          },
	{0x0f, 0x01, 0x10, 0x00, "Upright"     		  },
	{0x0f, 0x01, 0x10, 0x10, "Cocktail"    		  },

	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0e, 0x01, 0x03, 0x03, "2C 1C"     		  },
	{0x0e, 0x01, 0x03, 0x01, "1C 1C"    		  },
	{0x0e, 0x01, 0x03, 0x02, "1C 2C"     		  },
	{0x0e, 0x01, 0x03, 0x00, "Free Play"     	  },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0e, 0x01, 0x0c, 0x00, "3"     		  },
	{0x0e, 0x01, 0x0c, 0x04, "4"    		  },
	{0x0e, 0x01, 0x0c, 0x08, "5"     		  },
	{0x0e, 0x01, 0x0c, 0x0c, "6"     		  },

	{0   , 0xfe, 0   , 2   , "First Pattern"          },
	{0x0e, 0x01, 0x30, 0x10, "Easy"     		  },
	{0x0e, 0x01, 0x30, 0x00, "Hard"    		  },

	{0   , 0xfe, 0   , 2   , "Teleport Holes"         },
	{0x0e, 0x01, 0x20, 0x20, "Off"      		  },
	{0x0e, 0x01, 0x20, 0x00, "On"       		  },
};

STDDIPINFO(maketrax)

static struct BurnDIPInfo mbrushDIPList[]=
{
	{0x0e, 0xff, 0xff, 0x39, NULL                     },
	{0x0f, 0xff, 0xff, 0xef, NULL                     },
	{0x10, 0xff, 0xff, 0x6f, NULL                     },

	{0   , 0xfe, 0   , 2   , "Cabinet"	          },
	{0x0f, 0x01, 0x10, 0x00, "Upright"     		  },
	{0x0f, 0x01, 0x10, 0x10, "Cocktail"    		  },

	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0e, 0x01, 0x03, 0x03, "2C 1C"     		  },
	{0x0e, 0x01, 0x03, 0x01, "1C 1C"    		  },
	{0x0e, 0x01, 0x03, 0x02, "1C 2C"     		  },
	{0x0e, 0x01, 0x03, 0x00, "Free Play"     	  },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0e, 0x01, 0x0c, 0x00, "1"     		  },
	{0x0e, 0x01, 0x0c, 0x04, "2"    		  },
	{0x0e, 0x01, 0x0c, 0x08, "3"     		  },
	{0x0e, 0x01, 0x0c, 0x0c, "4"     		  },
};

STDDIPINFO(mbrush)

static struct BurnDIPInfo crushsDIPList[]=
{
	{0x0e, 0xff, 0xff, 0xff, NULL                     },
	{0x0f, 0xff, 0xff, 0xff, NULL                     },
	{0x10, 0xff, 0xff, 0xff, NULL                     },
	{0x11, 0xff, 0xff, 0x00, NULL                     },

	{0   , 0xfe, 0   , 2   , "Teleport Holes"         },
	{0x0f, 0x01, 0x10, 0x10, "Off"      		  },
	{0x0f, 0x01, 0x10, 0x00, "On"       		  },

	{0   , 0xfe, 0   , 16   , "Coin A"                },
	{0x11, 0x01, 0x0f, 0x08, "2C 1C"      		  },
	{0x11, 0x01, 0x0f, 0x09, "2C 2C"       		  },
	{0x11, 0x01, 0x0f, 0x00, "1C 1C"      		  },
	{0x11, 0x01, 0x0f, 0x0a, "2C 3C"       		  },
	{0x11, 0x01, 0x0f, 0x0b, "2C 4C"      		  },
	{0x11, 0x01, 0x0f, 0x01, "2C 2C"       		  },
	{0x11, 0x01, 0x0f, 0x0c, "1C 5C"      		  },
	{0x11, 0x01, 0x0f, 0x0d, "2C 6C"       		  },
	{0x11, 0x01, 0x0f, 0x02, "1C 3C"      		  },
	{0x11, 0x01, 0x0f, 0x0e, "2C 7C"       		  },
	{0x11, 0x01, 0x0f, 0x0f, "2C 8C"      		  },
	{0x11, 0x01, 0x0f, 0x03, "1C 4C"       		  },
	{0x11, 0x01, 0x0f, 0x04, "1C 5C"      		  },
	{0x11, 0x01, 0x0f, 0x05, "1C 6C"       		  },
	{0x11, 0x01, 0x0f, 0x06, "1C 7C"      		  },
	{0x11, 0x01, 0x0f, 0x07, "1C 8C"       		  },

	{0   , 0xfe, 0   , 16   , "Coin B"                },
	{0x11, 0x01, 0xf0, 0x80, "2C 1C"      		  },
	{0x11, 0x01, 0xf0, 0x90, "2C 2C"       		  },
	{0x11, 0x01, 0xf0, 0x00, "1C 1C"      		  },
	{0x11, 0x01, 0xf0, 0xa0, "2C 3C"       		  },
	{0x11, 0x01, 0xf0, 0xb0, "2C 4C"      		  },
	{0x11, 0x01, 0xf0, 0x10, "2C 2C"       		  },
	{0x11, 0x01, 0xf0, 0xc0, "1C 5C"      		  },
	{0x11, 0x01, 0xf0, 0xd0, "2C 6C"       		  },
	{0x11, 0x01, 0xf0, 0x20, "1C 3C"      		  },
	{0x11, 0x01, 0xf0, 0xe0, "2C 7C"       		  },
	{0x11, 0x01, 0xf0, 0xf0, "2C 8C"      		  },
	{0x11, 0x01, 0xf0, 0x30, "1C 4C"       		  },
	{0x11, 0x01, 0xf0, 0x40, "1C 5C"      		  },
	{0x11, 0x01, 0xf0, 0x50, "1C 6C"       		  },
	{0x11, 0x01, 0xf0, 0x60, "1C 7C"      		  },
	{0x11, 0x01, 0xf0, 0x70, "1C 8C"       		  },
};

STDDIPINFO(crushs)

static struct BurnDIPInfo ponpokoDIPList[]=
{
	{0x10, 0xff, 0xff, 0xd1, NULL                     },
	{0x11, 0xff, 0xff, 0xb1, NULL                     },
	{0x12, 0xff, 0xff, 0xe0, NULL                     },
	{0x13, 0xff, 0xff, 0x00, NULL                     },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x10, 0x01, 0x03, 0x00, "10000"     		  },
	{0x10, 0x01, 0x03, 0x01, "30000"    		  },
	{0x10, 0x01, 0x03, 0x02, "50000"     		  },
	{0x10, 0x01, 0x03, 0x03, "None"     		  },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x10, 0x01, 0x30, 0x00, "2"     		  },
	{0x10, 0x01, 0x30, 0x10, "3"    		  },
	{0x10, 0x01, 0x30, 0x20, "4"     		  },
	{0x10, 0x01, 0x30, 0x30, "5"     		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"   	          },
	{0x10, 0x01, 0x40, 0x40, "Upright"     		  },
	{0x10, 0x01, 0x40, 0x00, "Cocktail"    		  },

	{0   , 0xfe, 0   , 16   , "Coinage"               },
	{0x11, 0x01, 0x0f, 0x04, "A 3/1 B 3/1"  	  },
	{0x11, 0x01, 0x0f, 0x0e, "A 3/1 B 1/2"    	  },
	{0x11, 0x01, 0x0f, 0x0f, "A 3/1 B 1/4"     	  },
	{0x11, 0x01, 0x0f, 0x02, "A 2/1 B 2/1"     	  },
	{0x11, 0x01, 0x0f, 0x0d, "A 2/1 B 1/1"  	  },
	{0x11, 0x01, 0x0f, 0x07, "A 2/1 B 1/3"    	  },
	{0x11, 0x01, 0x0f, 0x0b, "A 2/1 B 1/5"     	  },
	{0x11, 0x01, 0x0f, 0x0c, "A 2/1 B 1/6"     	  },
	{0x11, 0x01, 0x0f, 0x01, "A 1/1 B 1/1"  	  },
	{0x11, 0x01, 0x0f, 0x06, "A 1/1 B 4/5"    	  },
	{0x11, 0x01, 0x0f, 0x05, "A 1/1 B 2/3"     	  },
	{0x11, 0x01, 0x0f, 0x0a, "A 1/1 B 1/3"     	  },
	{0x11, 0x01, 0x0f, 0x08, "A 1/1 B 1/5"  	  },
	{0x11, 0x01, 0x0f, 0x09, "A 1/1 B 1/6"    	  },
	{0x11, 0x01, 0x0f, 0x03, "A 1/2 B 1/2"     	  },
	{0x11, 0x01, 0x0f, 0x00, "Free Play"     	  },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x11, 0x01, 0x40, 0x40, "Off"     		  },
	{0x11, 0x01, 0x40, 0x00, "On"    		  },
};

STDDIPINFO(ponpoko)

static struct BurnDIPInfo bwcasinoDIPList[]=
{
	{0x0f, 0xff, 0xff, 0x7f, NULL                     },
	{0x10, 0xff, 0xff, 0xff, NULL                     },
	{0x11, 0xff, 0xff, 0xff, NULL			  },

	{0   , 0xfe, 0   , 2   , "Cabinet"	          },
	{0x0f, 0x01, 0x80, 0x00, "Upright"     		  },
	{0x0f, 0x01, 0x80, 0x80, "Cocktail"    		  },

	{0   , 0xfe, 0   , 16   , "Hands Per Game"        },
	{0x10, 0x01, 0x1e, 0x1e, "3"     		  },
	{0x10, 0x01, 0x1e, 0x1c, "4"    		  },
	{0x10, 0x01, 0x1e, 0x1a, "5"     		  },
	{0x10, 0x01, 0x1e, 0x18, "6"     	          },
	{0x10, 0x01, 0x1e, 0x16, "7"     		  },
	{0x10, 0x01, 0x1e, 0x14, "8"    		  },
	{0x10, 0x01, 0x1e, 0x12, "9"     		  },
	{0x10, 0x01, 0x1e, 0x10, "10"     	          },
	{0x10, 0x01, 0x1e, 0x0e, "11"     		  },
	{0x10, 0x01, 0x1e, 0x0c, "12"    		  },
	{0x10, 0x01, 0x1e, 0x0a, "13"     		  },
	{0x10, 0x01, 0x1e, 0x07, "14"     	          },
	{0x10, 0x01, 0x1e, 0x06, "15"     		  },
	{0x10, 0x01, 0x1e, 0x04, "16"    		  },
	{0x10, 0x01, 0x1e, 0x02, "17"     		  },
	{0x10, 0x01, 0x1e, 0x00, "18"     	          },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x10, 0x01, 0x20, 0x20, "Off"     		  },
	{0x10, 0x01, 0x20, 0x00, "On"    		  },
};

STDDIPINFO(bwcasino)

static struct BurnDIPInfo acityaDIPList[]=
{
	{0x0a, 0xff, 0xff, 0xff, NULL                     },
	{0x0b, 0xff, 0xff, 0xff, NULL                     },
	{0x0c, 0xff, 0xff, 0xff, NULL			  },

	{0   , 0xfe, 0   , 16   , "Hands Per Game"        },
	{0x0b, 0x01, 0x1e, 0x1e, "3"     		  },
	{0x0b, 0x01, 0x1e, 0x1c, "4"    		  },
	{0x0b, 0x01, 0x1e, 0x1a, "5"     		  },
	{0x0b, 0x01, 0x1e, 0x18, "6"     	          },
	{0x0b, 0x01, 0x1e, 0x16, "7"     		  },
	{0x0b, 0x01, 0x1e, 0x14, "8"    		  },
	{0x0b, 0x01, 0x1e, 0x12, "9"     		  },
	{0x0b, 0x01, 0x1e, 0x10, "10"     	          },
	{0x0b, 0x01, 0x1e, 0x0e, "11"     		  },
	{0x0b, 0x01, 0x1e, 0x0c, "12"    		  },
	{0x0b, 0x01, 0x1e, 0x0a, "13"     		  },
	{0x0b, 0x01, 0x1e, 0x07, "14"     	          },
	{0x0b, 0x01, 0x1e, 0x06, "15"     		  },
	{0x0b, 0x01, 0x1e, 0x04, "16"    		  },
	{0x0b, 0x01, 0x1e, 0x02, "17"     		  },
	{0x0b, 0x01, 0x1e, 0x00, "18"     	          },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x0b, 0x01, 0x20, 0x20, "Off"     		  },
	{0x0b, 0x01, 0x20, 0x00, "On"    		  },
};

STDDIPINFO(acitya)

static struct BurnDIPInfo eyesDIPList[]=
{
	{0x10, 0xff, 0xff, 0xfb, NULL                     },
	{0x11, 0xff, 0xff, 0xff, NULL                     },
	{0x12, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x10, 0x01, 0x03, 0x01, "2C 1C"     		  },
	{0x10, 0x01, 0x03, 0x03, "1C 1C"    		  },
	{0x10, 0x01, 0x03, 0x02, "1C 2C"     		  },
	{0x10, 0x01, 0x03, 0x00, "Free Play"     	  },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x10, 0x01, 0x0c, 0x0c, "2"     		  },
	{0x10, 0x01, 0x0c, 0x08, "3"    		  },
	{0x10, 0x01, 0x0c, 0x04, "4"     		  },
	{0x10, 0x01, 0x0c, 0x00, "5"     		  },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x10, 0x01, 0x30, 0x30, "50000"     		  },
	{0x10, 0x01, 0x30, 0x20, "75000"    		  },
	{0x10, 0x01, 0x30, 0x10, "100000"     		  },
	{0x10, 0x01, 0x30, 0x00, "125000"     		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"	          },
	{0x10, 0x01, 0x40, 0x40, "Upright"     		  },
	{0x10, 0x01, 0x40, 0x00, "Cocktail"    		  },
};

STDDIPINFO(eyes)

static struct BurnDIPInfo mrtntDIPList[]=
{
	{0x10, 0xff, 0xff, 0xfb, NULL                     },
	{0x11, 0xff, 0xff, 0xff, NULL                     },
	{0x12, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x10, 0x01, 0x03, 0x01, "2C 1C"     		  },
	{0x10, 0x01, 0x03, 0x03, "1C 1C"    		  },
	{0x10, 0x01, 0x03, 0x02, "1C 2C"     		  },
	{0x10, 0x01, 0x03, 0x00, "Free Play"     	  },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x10, 0x01, 0x0c, 0x0c, "2"     		  },
	{0x10, 0x01, 0x0c, 0x08, "3"    		  },
	{0x10, 0x01, 0x0c, 0x04, "4"     		  },
	{0x10, 0x01, 0x0c, 0x00, "5"     		  },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x10, 0x01, 0x30, 0x30, "75000"     		  },
	{0x10, 0x01, 0x30, 0x20, "100000"    		  },
	{0x10, 0x01, 0x30, 0x10, "125000"     		  },
	{0x10, 0x01, 0x30, 0x00, "150000"     		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"	          },
	{0x10, 0x01, 0x40, 0x40, "Upright"     		  },
	{0x10, 0x01, 0x40, 0x00, "Cocktail"    		  },
};

STDDIPINFO(mrtnt)

static struct BurnDIPInfo alibabaDIPList[]=
{
	{0x0f, 0xff, 0xff, 0xff, NULL                     },
	{0x10, 0xff, 0xff, 0xff, NULL                     },
	{0x11, 0xff, 0xff, 0xd9, NULL                     },
	{0x12, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 2   , "Rack Test (Cheat)"      },
	{0x0f, 0x01, 0x10, 0x10, "Off"     		  },
	{0x0f, 0x01, 0x10, 0x00, "On"    		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"	          },
	{0x10, 0x01, 0x80, 0x80, "Upright"     		  },
	{0x10, 0x01, 0x80, 0x00, "Cocktail"    		  },

	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x11, 0x01, 0x03, 0x03, "2C 1C"     		  },
	{0x11, 0x01, 0x03, 0x01, "1C 1C"    		  },
	{0x11, 0x01, 0x03, 0x02, "1C 2C"     		  },
	{0x11, 0x01, 0x03, 0x00, "Free Play"     	  },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x0c, 0x00, "1"     		  },
	{0x11, 0x01, 0x0c, 0x04, "2"    		  },
	{0x11, 0x01, 0x0c, 0x08, "3"     		  },
	{0x11, 0x01, 0x0c, 0x0c, "5"     		  },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x11, 0x01, 0x30, 0x00, "10000"     		  },
	{0x11, 0x01, 0x30, 0x10, "15000"    		  },
	{0x11, 0x01, 0x30, 0x20, "20000"     		  },
	{0x11, 0x01, 0x30, 0x30, "None"     		  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x11, 0x01, 0x40, 0x40, "Normal"      		  },
	{0x11, 0x01, 0x40, 0x00, "Hard"       		  },
};

STDDIPINFO(alibaba)

static struct BurnDIPInfo theglobpDIPList[]=
{
	{0x0f, 0xff, 0xff, 0xff, NULL                     },
	{0x10, 0xff, 0xff, 0xff, NULL                     },
	{0x11, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 2   , "Cabinet"	          },
	{0x0f, 0x01, 0x80, 0x80, "Upright"     		  },
	{0x0f, 0x01, 0x80, 0x00, "Cocktail"    		  },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x10, 0x01, 0x03, 0x03, "3"     		  },
	{0x10, 0x01, 0x03, 0x02, "4"    		  },
	{0x10, 0x01, 0x03, 0x01, "5"     		  },
	{0x10, 0x01, 0x03, 0x00, "6"     		  },

	{0   , 0xfe, 0   , 8   , "Difficulty"             },
	{0x10, 0x01, 0x1c, 0x1c, "Easiest"     		  },
	{0x10, 0x01, 0x1c, 0x18, "Very Easy"    	  },
	{0x10, 0x01, 0x1c, 0x14, "Easy"     		  },
	{0x10, 0x01, 0x1c, 0x10, "Normal"     		  },
	{0x10, 0x01, 0x1c, 0x0c, "Difficult"     	  },
	{0x10, 0x01, 0x1c, 0x08, "Very Difficult"    	  },
	{0x10, 0x01, 0x1c, 0x04, "Very Hard"              },
	{0x10, 0x01, 0x1c, 0x00, "Hardest"     		  },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x10, 0x01, 0x20, 0x20, "Off"      		  },
	{0x10, 0x01, 0x20, 0x00, "On"       		  },
};

STDDIPINFO(theglobp)

static struct BurnDIPInfo woodpekDIPList[]=
{
	{0x10, 0xff, 0xff, 0xff, NULL                     },
	{0x11, 0xff, 0xff, 0xc1, NULL                     },
	{0x12, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 2   , "Rack Test (Cheat)"      },
	{0x10, 0x01, 0x10, 0x10, "Off"     		  },
	{0x10, 0x01, 0x10, 0x00, "On"    		  },

	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x11, 0x01, 0x03, 0x03, "2C 1C"     		  },
	{0x11, 0x01, 0x03, 0x01, "1C 1C"    		  },
	{0x11, 0x01, 0x03, 0x02, "1C 2C"     		  },
	{0x11, 0x01, 0x03, 0x00, "Free Play"     	  },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x0c, 0x00, "3"     		  },
	{0x11, 0x01, 0x0c, 0x04, "4"    		  },
	{0x11, 0x01, 0x0c, 0x08, "5"     		  },
	{0x11, 0x01, 0x0c, 0x0c, "6"     		  },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x11, 0x01, 0x30, 0x00, "5000"     		  },
	{0x11, 0x01, 0x30, 0x10, "10000"    		  },
	{0x11, 0x01, 0x30, 0x20, "15000"    		  },
	{0x11, 0x01, 0x30, 0x30, "None"    		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x11, 0x01, 0x40, 0x40, "Upright"   		  },
	{0x11, 0x01, 0x40, 0x00, "Cocktail"    		  },
};

STDDIPINFO(woodpek)

static struct BurnDIPInfo lizwizDIPList[]=
{
	{0x11, 0xff, 0xff, 0xfb, NULL                     },
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x11, 0x01, 0x03, 0x01, "2C 1C"     		  },
	{0x11, 0x01, 0x03, 0x03, "1C 1C"    		  },
	{0x11, 0x01, 0x03, 0x02, "1C 2C"     		  },
	{0x11, 0x01, 0x03, 0x00, "Free Play"     	  },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x0c, 0x00, "5"     		  },
	{0x11, 0x01, 0x0c, 0x04, "4"    		  },
	{0x11, 0x01, 0x0c, 0x08, "3"     		  },
	{0x11, 0x01, 0x0c, 0x0c, "2"     		  },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x11, 0x01, 0x30, 0x00, "75000"     		  },
	{0x11, 0x01, 0x30, 0x10, "100000"    		  },
	{0x11, 0x01, 0x30, 0x20, "125000"     		  },
	{0x11, 0x01, 0x30, 0x30, "150000"     		  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x11, 0x01, 0x40, 0x40, "Normal"      		  },
	{0x11, 0x01, 0x40, 0x00, "Hard"       		  },
};

STDDIPINFO(lizwiz)

static struct BurnDIPInfo vanvanDIPList[]=
{
	{0x0f, 0xff, 0xff, 0xda, NULL                     },
	{0x10, 0xff, 0xff, 0xff, NULL                     },
	{0x11, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 2   , "Cabinet"	          },
	{0x0f, 0x01, 0x01, 0x00, "Upright"     		  },
	{0x0f, 0x01, 0x01, 0x01, "Cocktail"    		  },

	{0   , 0xfe, 0   , 2   , "Flip screen"            },
	{0x0f, 0x01, 0x02, 0x02, "Off"     		  },
	{0x0f, 0x01, 0x02, 0x00, "On"    		  },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0f, 0x01, 0x0c, 0x08, "20k and 100k"		  },
	{0x0f, 0x01, 0x0c, 0x04, "40k and 140k"		  },
	{0x0f, 0x01, 0x0c, 0x00, "70k and 200k"		  },
	{0x0f, 0x01, 0x0c, 0x0c, "None"     		  },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0f, 0x01, 0x30, 0x30, "1"     		  },
	{0x0f, 0x01, 0x30, 0x20, "2"    		  },
	{0x0f, 0x01, 0x30, 0x10, "3"     		  },
	{0x0f, 0x01, 0x30, 0x00, "5"     		  },

	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x0f, 0x01, 0xc0, 0x00, "2c_1c"      		  },
	{0x0f, 0x01, 0xc0, 0xc0, "1C_1C"       		  },
	{0x0f, 0x01, 0xc0, 0x80, "1c_2C"      		  },
	{0x0f, 0x01, 0xc0, 0x40, "1C_3C"       		  },
};

STDDIPINFO(vanvan)

static struct BurnDIPInfo nmouseDIPList[]=
{
	{0x0e, 0xff, 0xff, 0xc9, NULL                     },
	{0x0f, 0xff, 0xff, 0xff, NULL                     },
	{0x10, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0e, 0x01, 0x03, 0x03, "2C 1C"     		  },
	{0x0e, 0x01, 0x03, 0x01, "1C 1C"    		  },
	{0x0e, 0x01, 0x03, 0x02, "1C 2C"     		  },
	{0x0e, 0x01, 0x03, 0x00, "Free Play"     	  },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0e, 0x01, 0x0c, 0x00, "1"     		  },
	{0x0e, 0x01, 0x0c, 0x04, "2"    		  },
	{0x0e, 0x01, 0x0c, 0x08, "3"     		  },
	{0x0e, 0x01, 0x0c, 0x0c, "5"     		  },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0e, 0x01, 0x30, 0x00, "5000"     		  },
	{0x0e, 0x01, 0x30, 0x10, "10000"    		  },
	{0x0e, 0x01, 0x30, 0x20, "15000"     		  },
	{0x0e, 0x01, 0x30, 0x30, "None"     		  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x0e, 0x01, 0x40, 0x40, "Normal"      		  },
	{0x0e, 0x01, 0x40, 0x00, "Hard"       		  },

	{0   , 0xfe, 0   , 2   , "Rack Test (Cheat)"      },
	{0x0f, 0x01, 0x10, 0x10, "Off"     		  },
	{0x0f, 0x01, 0x10, 0x00, "On"    		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"	          },
	{0x10, 0x01, 0x80, 0x80, "Upright"     		  },
	{0x10, 0x01, 0x80, 0x00, "Cocktail"    		  },
};

STDDIPINFO(nmouse)

static struct BurnDIPInfo jumpshotDIPList[]=
{
	{0x10, 0xff, 0xff, 0xf1, NULL                     },
	{0x11, 0xff, 0xff, 0xff, NULL                     },
	{0x12, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 3   , "Time"                   },
	{0x10, 0x01, 0x03, 0x02, "2 Minutes"     	  },
	{0x10, 0x01, 0x03, 0x03, "3 Minutes"    	  },
	{0x10, 0x01, 0x03, 0x01, "4 Minutes"     	  },

	{0   , 0xfe, 0   , 4   , "Player Skin Tone"       },
	{0x10, 0x01, 0x04, 0x04, "Lighter"     		  },
	{0x10, 0x01, 0x04, 0x00, "Darker"    		  },

	{0   , 0xfe, 0   , 4   , "Player Skin Tone"       },
	{0x10, 0x01, 0x08, 0x08, "Lighter"     		  },
	{0x10, 0x01, 0x08, 0x00, "Darker"    		  },

	{0   , 0xfe, 0   , 2   , "Free play"              },
	{0x10, 0x01, 0x10, 0x10, "Off"      		  },
	{0x10, 0x01, 0x10, 0x00, "On"       		  },

	{0   , 0xfe, 0   , 2   , "2 Players Game"         },
	{0x10, 0x01, 0x20, 0x20, "1 Credit"  		  },
	{0x10, 0x01, 0x20, 0x00, "2 Credits"       	  },
};

STDDIPINFO(jumpshot)

static struct BurnDIPInfo jumpshtpDIPList[]=
{
	{0x10, 0xff, 0xff, 0xf1, NULL                     },
	{0x11, 0xff, 0xff, 0xff, NULL                     },
	{0x12, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 3   , "Time"                   },
	{0x10, 0x01, 0x03, 0x02, "Short"     		  },
	{0x10, 0x01, 0x03, 0x03, "Average"    	 	  },
	{0x10, 0x01, 0x03, 0x01, "Above Average"     	  },

	{0   , 0xfe, 0   , 4   , "Player Skin Tone"       },
	{0x10, 0x01, 0x04, 0x04, "Lighter"     		  },
	{0x10, 0x01, 0x04, 0x00, "Darker"    		  },

	{0   , 0xfe, 0   , 4   , "Player Skin Tone"       },
	{0x10, 0x01, 0x08, 0x08, "Lighter"     		  },
	{0x10, 0x01, 0x08, 0x00, "Darker"    		  },

	{0   , 0xfe, 0   , 2   , "Free play"              },
	{0x10, 0x01, 0x10, 0x10, "Off"      		  },
	{0x10, 0x01, 0x10, 0x00, "On"       		  },

	{0   , 0xfe, 0   , 2   , "2 Players Game"         },
	{0x10, 0x01, 0x20, 0x20, "1 Credit"  		  },
	{0x10, 0x01, 0x20, 0x00, "2 Credits"       	  },
};

STDDIPINFO(jumpshtp)

static struct BurnDIPInfo dremshprDIPList[]=
{
	{0x10, 0xff, 0xff, 0xff, NULL                     },
	{0x11, 0xff, 0xff, 0xff, NULL                     },
	{0x12, 0xff, 0xff, 0xfe, NULL			  },
	{0x0f, 0xff, 0xff, 0xfb, NULL                     },

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0f, 0x01, 0x01, 0x01, "Upright"     		  },
	{0x0f, 0x01, 0x01, 0x00, "Cocktail"    		  },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x0f, 0x01, 0x20, 0x20, "Off"     		  },
	{0x0f, 0x01, 0x20, 0x00, "On"    		  },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0f, 0x01, 0x0c, 0x08, "30000"     		  },
	{0x0f, 0x01, 0x0c, 0x04, "50000"    		  },
	{0x0f, 0x01, 0x0c, 0x00, "70000"    		  },
	{0x0f, 0x01, 0x0c, 0x0c, "none"			  },

	{0   , 0xfe, 0   , 4   , "Lives"	          },
	{0x0f, 0x01, 0x30, 0x30, "3"      		  },
	{0x0f, 0x01, 0x30, 0x20, "4"       		  },
	{0x0f, 0x01, 0x30, 0x10, "5"      		  },
	{0x0f, 0x01, 0x30, 0x00, "6"       		  },

	{0   , 0xfe, 0,    4,    "Coinage"		  },
	{0x0f, 0x01, 0xc0, 0x00, "2C 1C"		  },
	{0x0f, 0x01, 0xc0, 0xc0, "1C 1C"		  },
	{0x0f, 0x01, 0xc0, 0x80, "1C 2C"		  },
	{0x0f, 0x01, 0xc0, 0x40, "1C 3C"		  },
};

STDDIPINFO(dremshpr)

static struct BurnDIPInfo cannonbpDIPList[]=
{
	{0x0f, 0xff, 0xff, 0xff, NULL                     },
	{0x10, 0xff, 0xff, 0xff, NULL                     },
	{0x11, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 4   , "Display"                },
	{0x0f, 0x01, 0x03, 0x03, "Scores and Progression Bars" },
	{0x0f, 0x01, 0x03, 0x01, "Scores only"    	  },
	{0x0f, 0x01, 0x03, 0x02, "Progression Bars only"  },
	{0x0f, 0x01, 0x03, 0x00, "None"    		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0f, 0x01, 0x04, 0x04, "Upright"     		  },
	{0x0f, 0x01, 0x04, 0x00, "Cocktail"    		  },

	{0   , 0xfe, 0   , 4   , "Lives"	          },
	{0x0f, 0x01, 0x18, 0x00, "3"      		  },
	{0x0f, 0x01, 0x18, 0x08, "4"       		  },
	{0x0f, 0x01, 0x18, 0x10, "5"      		  },
	{0x0f, 0x01, 0x18, 0x18, "6"       		  },
};

STDDIPINFO(cannonbp)

static struct BurnDIPInfo rocktrv2DIPList[]=
{
	{0x0e, 0xff, 0x3f, 0x33, NULL                     },
	{0x0f, 0xff, 0xff, 0xff, NULL                     },
	{0x10, 0xff, 0xff, 0xff, NULL                     },
	{0x11, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0e, 0x01, 0x03, 0x03, "2C 1C"     		  },
	{0x0e, 0x01, 0x03, 0x01, "1C 1C"    		  },
	{0x0e, 0x01, 0x03, 0x02, "1C 2C"     		  },
	{0x0e, 0x01, 0x03, 0x00, "Free Play"     	  },

	{0   , 0xfe, 0   , 8   , "Questions Per Game"     },
	{0x0e, 0x01, 0x1c, 0x1c, "2"     		  },
	{0x0e, 0x01, 0x1c, 0x18, "3"    		  },
	{0x0e, 0x01, 0x1c, 0x14, "4"     		  },
	{0x0e, 0x01, 0x1c, 0x10, "5"     		  },
	{0x0e, 0x01, 0x1c, 0x0c, "6"     		  },
	{0x0e, 0x01, 0x1c, 0x08, "7"    		  },
	{0x0e, 0x01, 0x1c, 0x04, "8"     		  },
	{0x0e, 0x01, 0x1c, 0x00, "9"     		  },

	{0   , 0xfe, 0   , 4   , "Clock Speed"            },
	{0x0e, 0x01, 0x60, 0x60, "Beginner"   		  },
	{0x0e, 0x01, 0x60, 0x40, "Intermediate" 	  },
	{0x0e, 0x01, 0x60, 0x20, "Professional"  	  },
	{0x0e, 0x01, 0x60, 0x00, "Super - Pro"    	  },

	{0   , 0xfe, 0   , 2   , "Freeze Image"           },
	{0x0e, 0x01, 0x80, 0x80, "Off"      		  },
	{0x0e, 0x01, 0x80, 0x00, "On"       		  },

	{0   , 0xfe, 0   , 2   , "Mode"		          },
	{0x0f, 0x01, 0x01, 0x01, "Amusement"   		  },
	{0x0f, 0x01, 0x01, 0x00, "Credit"    		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"	          },
	{0x0f, 0x01, 0x02, 0x02, "Upright"   		  },
	{0x0f, 0x01, 0x02, 0x00, "Cocktail"    		  },

	{0   , 0xfe, 0   , 2   , "K.O. Switch"	          },
	{0x0f, 0x01, 0x04, 0x04, "Auto"   		  },
	{0x0f, 0x01, 0x04, 0x00, "Manual"    		  },

	{0   , 0xfe, 0   , 8   , "Bonus Life"	          },
	{0x0f, 0x01, 0x70, 0x70, "10000"   		  },
	{0x0f, 0x01, 0x70, 0x60, "17500"    		  },
	{0x0f, 0x01, 0x70, 0x50, "25000"   		  },
	{0x0f, 0x01, 0x70, 0x40, "32500"    		  },
	{0x0f, 0x01, 0x70, 0x30, "40000"   		  },
	{0x0f, 0x01, 0x70, 0x20, "47500"    		  },
	{0x0f, 0x01, 0x70, 0x10, "55000"   		  },
	{0x0f, 0x01, 0x70, 0x00, "62500"    		  },

	{0   , 0xfe, 0   , 2   , "Music"	          },
	{0x0f, 0x01, 0x80, 0x80, "On"    		  },
	{0x0f, 0x01, 0x80, 0x00, "Off"   		  },
};

STDDIPINFO(rocktrv2)

static struct BurnDIPInfo bigbucksDIPList[]=
{
	{0x0c, 0xff, 0xff, 0xff, NULL                     },
	{0x0d, 0xff, 0xff, 0xff, NULL                     },
	{0x0e, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 2   , "Enable Adult Affairs Category"      },
	{0x0c, 0x01, 0x10, 0x00, "Off"     		  },
	{0x0c, 0x01, 0x10, 0x10, "On"    		  },

	{0   , 0xfe, 0   , 2   , "Time to bet / answer"   },
	{0x0d, 0x01, 0x01, 0x00, "15 sec. / 10 sec."      },
	{0x0d, 0x01, 0x01, 0x01, "20 sec. / 15 sec."      },

	{0   , 0xfe, 0   , 2   , "Continue if player busts"},
	{0x0d, 0x01, 0x02, 0x00, "Off"     		  },
	{0x0d, 0x01, 0x02, 0x02, "On"   		  },

	{0   , 0xfe, 0   , 2   , "Show correct answer"    },
	{0x0d, 0x01, 0x04, 0x00, "Off"     		  },
	{0x0d, 0x01, 0x04, 0x04, "On"   		  },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x0d, 0x01, 0x08, 0x00, "Off"     		  },
	{0x0d, 0x01, 0x08, 0x08, "On"   		  },

	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x0d, 0x01, 0x10, 0x00, "2C 1C"     		  },
	{0x0d, 0x01, 0x10, 0x10, "1C 1C"    		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0d, 0x01, 0x20, 0x20, "Upright"     		  },
	{0x0d, 0x01, 0x20, 0x00, "Cocktail"   		  },
};

STDDIPINFO(bigbucks)

static struct BurnDIPInfo korosukeDIPList[]=
{
	{0x0f, 0xff, 0x3f, 0x31, NULL                     },
	{0x10, 0xff, 0xff, 0xef, NULL                     },
	{0x11, 0xff, 0x6f, 0x6f, NULL                     },

	{0   , 0xfe, 0   , 2   , "Cabinet"	          },
	{0x10, 0x01, 0x10, 0x00, "Upright"     		  },
	{0x10, 0x01, 0x10, 0x10, "Cocktail"    		  },

	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0f, 0x01, 0x03, 0x03, "2C 1C"     		  },
	{0x0f, 0x01, 0x03, 0x01, "1C 1C"    		  },
	{0x0f, 0x01, 0x03, 0x02, "1C 2C"     		  },
	{0x0f, 0x01, 0x03, 0x00, "Free Play"     	  },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0f, 0x01, 0x0c, 0x00, "3"     		  },
	{0x0f, 0x01, 0x0c, 0x04, "4"    		  },
	{0x0f, 0x01, 0x0c, 0x08, "5"     		  },
	{0x0f, 0x01, 0x0c, 0x0c, "6"     		  },

	{0   , 0xfe, 0   , 2   , "First Pattern"          },
	{0x0f, 0x01, 0x30, 0x10, "Easy"     		  },
	{0x0f, 0x01, 0x30, 0x00, "Hard"    		  },

	{0   , 0xfe, 0   , 2   , "Teleport Holes"         },
	{0x0f, 0x01, 0x20, 0x20, "Off"      		  },
	{0x0f, 0x01, 0x20, 0x00, "On"       		  },
};

STDDIPINFO(korosuke)

static struct BurnDIPInfo shootbulDIPList[]=
{
	{0x08, 0xff, 0xff, 0xff, NULL                     },
	{0x09, 0xff, 0xff, 0xe0, NULL                     },
	{0x0A, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 5   , "Time"                   },
	{0x08, 0x01, 0x03, 0x01, "Short"     		  },
	{0x08, 0x01, 0x03, 0x07, "Average"    		  },
	{0x08, 0x01, 0x03, 0x03, "Long"     		  },
	{0x08, 0x01, 0x03, 0x05, "Longer"     	  	  },
	{0x08, 0x01, 0x03, 0x06, "Longest"     	  	  },

	{0   , 0xfe, 0   , 2   , "Title Page Sounds"      },
	{0x08, 0x01, 0x08, 0x00, "Off"      		  },
	{0x08, 0x01, 0x08, 0x08, "On"       		  },
};

STDDIPINFO(shootbul)

//------------------------------------------------------------------------------------------------------

static void mschamp_set_bank()
{
	nPacBank = DrvDips[3] & 1;

	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80ROM + 0x0000 + (nPacBank * 0x08000));
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80ROM + 0x0000 + (nPacBank * 0x08000));
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80ROM + 0x4000 + (nPacBank * 0x08000));
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80ROM + 0x4000 + (nPacBank * 0x08000));
}

static void epos_hardware_set_bank(INT32 nBank)
{
	nPacBank = nBank & 3;

	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80ROM + 0x10000 + (nPacBank * 0x04000));
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80ROM + 0x10000 + (nPacBank * 0x04000));
}

static UINT8 cannonbp_protection_r(UINT16 offset)
{
	switch (offset)
	{
		case 0x0004:
			cannonb_bit_to_read = 7;
		case 0x0000:
		case 0x0003:
		case 0x0012:
		case 0x0105:
			return 0x00;

		case 0x0001:
			if (ZetPc(-1) == 0x2b97)
				return ((0x46 >> (cannonb_bit_to_read--)) & 1) << 7;
			else
				return 0xff;

		case 0x0107:
			return 0x40;
	}

	return 0;
}

static UINT8 epos_hardware_decrypt_rom(UINT16 offset)
{
	if (offset & 0x01)
	{
		epos_hardware_counter = epos_hardware_counter - 1;
		if (epos_hardware_counter < 0)
			epos_hardware_counter = 0x0f;
	}
	else
	{
		epos_hardware_counter = (epos_hardware_counter + 1) & 0x0f;
	}

	if (epos_hardware_counter >= 0x08 && epos_hardware_counter <= 0x0b) {
		epos_hardware_set_bank(epos_hardware_counter);
	}

	return 0;
}

static UINT8 maketrax_special_port2_r(UINT16 offset) // and korosuke
{
	INT32 data = DrvDips[2];
	INT32 pc = ZetPc(-1);

	if (pc == 0x1973 || pc == 0x2389 || pc == 0x196e || pc == 0x2387) return data | 0x40;

	switch (offset)
	{
		case 0x01:
		case 0x04: data |= 0x40; break;
		case 0x05: data |= 0xc0; break;

		default: data &= 0x3f; break;
	}

	return data;
}

static UINT8 maketrax_special_port3_r(UINT16 offset)
{
	INT32 pc = ZetPc(-1);

	if (pc == 0x040e || pc == 0x0445) return 0x20;
	if (pc == 0x115e || pc == 0x3ae2 || pc == 0x115b || pc == 0x3ae6) return 0x00;

	switch (offset)
	{
		case 0x00: return 0x1f;
		case 0x09: return 0x30;
		case 0x0c: return 0x00;

		default: return 0x20;
	}
}

UINT8 __fastcall pacman_read(UINT16 a)
{
	switch (game_select)
	{
		case ALIBABA:
		{
			if (a == 0x50c0) return rand() & 0xff;
			if (a == 0x50c1) {
				alibaba_mystery++;
				return (alibaba_mystery >> 10) & 1;
			}
		}
		break;

		case CANNONBP:
		{
			if ((a & 0xf000) == 0x3000) return cannonbp_protection_r(a & 0xfff);
		}
		break;

		case MAKETRAX:
		{
			if ((a & 0xffc0) == 0x5080) return maketrax_special_port2_r(a & 0x3f);
			if ((a & 0xffc0) == 0x50c0) return maketrax_special_port3_r(a & 0x3f);
		}
		break;

		case CRUSHS:
		{
			if (a == 0x5080) return DrvInputs[1];
		}
		break;

		case ROCKTRV2:
		{
			if ((a & 0xfff0) == 0x5fe0) return (rocktrv2_prot_data[(a >> 2) & 3] >> 4);

			if (a == 0x5fff) return DrvDips[3];

			if ((a & 0x8000) == 0x8000) return DrvQROM[(a & 0x7fff) | (nPacBank * 0x8000)];
		}
		break;
	}

	switch (a & ~0x003f)
	{
		case 0x5000: return DrvInputs[0];
		case 0x5040: return DrvInputs[1];
		case 0x5080: return DrvDips[2];
		case 0x50c0: return DrvDips[3];
	}

	if ((a & 0xfc00) == 0x4800) return 0xbf;

	return 0;
}

void __fastcall pacman_write(UINT16 a, UINT8 d)
{
	switch (game_select)
	{
		case BIRDIY:
		{
			if ((a & 0xffe0) == 0x5080) a -= 0x0040;
			if ((a & 0xfff0) == 0x50a0) a -= 0x0040;
			if (a == 0x5000) return;
			if (a == 0x5001) interrupt_mask = d & 1;
		}
		break;

		case ALIBABA:
		{
			if (a == 0x50c2) {
				ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
				interrupt_mask = d;
				return;
			}

			if ((a & 0xfff0) == 0x5040) {
				NamcoSoundWrite((a & 0x0f) | 0x00, d);
				return;
			}

			if ((a & 0xfff0) == 0x5060) {
				NamcoSoundWrite((a & 0x0f) | 0x10, d);
				return;
			}

			if ((a & 0xfff0) == 0x5050) {
				DrvSprRAM2[a & 0x0f] = d;
				return;
			}

			if (a == 0x5000) {
				watchdog = 0;
				return;
			}
		}
		break;

		case ROCKTRV2:
		{
			if ((a & 0xfffc) == 0x5fe0) {
				rocktrv2_prot_data[a & 3] = d;
				return;
			}

			if (a == 0x5ff0) {
				nPacBank = d;
				return;
			}
		}
		break;

		case VANVAN:
		{
			if (a == 0x5001) {
				for (INT32 i = 0; i < 0x100; i++) {
					if (DrvColPROM[0x100 + i] == 0) {
						Palette[i] = (d & 1) ? BurnHighCol(0xaa, 0xaa, 0xaa, 0) : 0;
					}
				}
				return;
			}
		}
		break;

		case BIGBUCKS:
			if (a == 0x6000) {
				nPacBank = d;
				return;
			}
		break;
	}
	
	if ((a & 0xffe0) == 0x5040) {
		NamcoSoundWrite(a & 0x1f, d);
		return;
	}

	if ((a & 0xfff0) == 0x5060) {
		DrvSprRAM2[a & 0x0f] = d;
		return;
	}

	switch (a)
	{
		case 0x5000:
			interrupt_mask = d & 1;
		break;

		case 0x5001:
			// pacman_sound_enable_w
		break;

		case 0x5003:
			*flipscreen = d & 1;
		break;

		case 0x5002:// nop
		case 0x5007:// coin counter
		break;

		case 0x50c0:
			watchdog = 0;
		break;
	}
}

UINT8 __fastcall pacman_in_port(UINT16 a)
{
	if (game_select == BIGBUCKS) {
		return DrvQROM[(nPacBank << 16) | (a ^ 0xffff)];
	}

	a &= 0xff;

	switch (game_select)
	{
		case CRUSHS:
			if (a == 1) return DrvDips[3];
			if (a == 2) return DrvDips[2];
			return 0;

		case EPOS:
			return epos_hardware_decrypt_rom(a);

		case MSCHAMP:
			if (a == 0) return mschamp_counter++;
			return 0;
	}

	return 0;
}

void __fastcall pacman_out_port(UINT16 a, UINT8 d)
{
	a &= 0xff;

	switch (game_select)
	{
		case CRUSHS:
			if (a == 0 || a == 1) {
				AY8910Write(0, ~a & 1, d);
			}
		return;

		case DREMSHPR:
			if (a == 6 || a == 7) {
				AY8910Write(0, ~a & 1, d);
			}
		return;
	
		case VANVAN:
			if (a == 1 || a == 2) {
				SN76496Write(a - 1, d);
			}
		return;

		case PIRANHA:
			if (a == 0) {
				if (d == 0xfa) d = 0x78;
				interrupt_mode = d;
			}
		return;

		case NMOUSE:
			if (a == 0) {
				if (d == 0xbf) d = 0x3c;
				if (d == 0xc6) d = 0x40;
				interrupt_mode = d;
			}
		return;
	}

	if (a == 0) // pacman & clones only
	{
		interrupt_mode = d;
		ZetSetVector(d);
		ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
	}
}

UINT8 __fastcall mspacman_read(UINT16 a)
{
	if ((a < 0x4000) || (a >= 0x8000 && a <= 0xbfff))
	{
		if ((a >= 0x0038 && a <= 0x003f) |
		    (a >= 0x03b0 && a <= 0x03b7) |
		    (a >= 0x1600 && a <= 0x1607) |
		    (a >= 0x2120 && a <= 0x2127) |
		    (a >= 0x3ff0 && a <= 0x3ff7) |
		    (a >= 0x8000 && a <= 0x8007) |
		    (a >= 0x97f0 && a <= 0x97f7))
		{
			nPacBank = 0;
		}

		if (a >= 0x3ff8 && a <= 0x3fff) {
			nPacBank = 1;
		}

		return DrvZ80ROM[(nPacBank << 16) + a];
	}

	if ((a & 0xfc00) == 0x4800) return 0xbf; // unmapped read

	if ((a & 0xff00) == 0x5000) a &= ~0x003f;

	switch (a)
	{
		case 0x5000: return DrvInputs[0];
		case 0x5040: return DrvInputs[1];
		case 0x5080: return DrvDips[2];
		case 0x50c0: return DrvDips[3];
	}

	return 0;
}

void __fastcall mspacman_write(UINT16 a, UINT8 d)
{
	if ((a >= 0x0038 && a <= 0x003f) |
	    (a >= 0x03b0 && a <= 0x03b7) |
	    (a >= 0x1600 && a <= 0x1607) |
	    (a >= 0x2120 && a <= 0x2127) |
	    (a >= 0x3ff0 && a <= 0x3ff7) |
	    (a >= 0x8000 && a <= 0x8007) |
	    (a >= 0x97f0 && a <= 0x97f7))
	{
		nPacBank = 0;
	}

	if (a >= 0x3ff8 && a <= 0x3fff) {
		nPacBank = 1;
	}

	if ((a & 0xffe0) == 0x5040) {
		NamcoSoundWrite(a & 0x1f, d);
		return;
	}

	if ((a & 0xfff0) == 0x5060) {
		DrvSprRAM2[a & 0x0f] = d;
		return;
	}

	switch (a)
	{
		case 0x5000:
			interrupt_mask = d & 1;
		break;

		case 0x5001:
			// pacman_sound_enable_w
		break;

		case 0x5003:
			*flipscreen = d & 1;
		break;

		case 0x5002: // nop
		case 0x5004:
		case 0x5005: // leds
		case 0x5006: // coin lockout
		case 0x5007: // coin counter
		case 0x5080: // nop
		return;

		case 0x50c0:
			watchdog = 0;
		break;
	}
}

void __fastcall pengo_write(UINT16 a, UINT8 d)
{
	if ((a & 0xffe0) == 0x9000) {
		NamcoSoundWrite(a & 0x1f, d);
		return;
	}

	if ((a & 0xfff0) == 0x9020) {
		DrvSprRAM2[a & 0x0f] = d;
		return;
	}

	switch (a)
	{
		case 0x9040:
			interrupt_mask = d & 1;
		return;

		case 0x9042:
			palettebank = d;
		return;

		case 0x9043:
			*flipscreen = d & 1;
		return;

		case 0x9044:
		case 0x9045: // coin counter
		return;

		case 0x9046:
			colortablebank = d;
		return;

		case 0x9047:
			charbank = d & 1;
			spritebank = d & 1;
		return;

		case 0x9070:
			watchdog = 0;
		return;
	}
}

UINT8 __fastcall pengo_read(UINT16 a)
{
	switch (a & ~0x003f)
	{
		case 0x9000: return DrvDips[3];
		case 0x9040: return DrvDips[2];
		case 0x9080: return DrvInputs[1];
		case 0x90c0: return DrvInputs[0];
	}

	return 0;
}

//------------------------------------------------------------------------------------------------------

static INT32 DrvDoReset(INT32 clear_ram)
{
	if (clear_ram) {
		memset (AllRam, 0, RamEnd - AllRam);
	}

	watchdog = 0;
	nPacBank = 0;

	ZetOpen(0);
	ZetReset();

	switch (game_select)
	{
		case EPOS:
			epos_hardware_counter = 0x0A + acitya;
			epos_hardware_set_bank(epos_hardware_counter);
		break;

		case MSCHAMP:
			mschamp_set_bank();
		break;
	}

	ZetClose();

	AY8910Reset(0);

	mschamp_counter = 0;
	cannonb_bit_to_read = 0;
	alibaba_mystery = 0;
	
	interrupt_mode = 0;
	interrupt_mask = 0;
	colortablebank = 0;
	palettebank = 0;
	spritebank = 0;	
	charbank = 0;
	
	return 0;
}

static void DrvTransTableInit()
{
	for (INT32 i = 0; i < 0x100; i++) {
		DrvTransTable[i] = (DrvColPROM[(i & 0xff) + 0x100] & 0x0f) ? 1 : 0;
	}
}

static void pacman_palette_init()
{
	UINT32 t_pal[32];

	for (INT32 i = 0; i < 32; i++)
	{
		INT32 bit0 = (DrvColPROM[i] >> 0) & 0x01;
		INT32 bit1 = (DrvColPROM[i] >> 1) & 0x01;
		INT32 bit2 = (DrvColPROM[i] >> 2) & 0x01;
		UINT8 r = (bit0 * 33) + (bit1 * 71) + (bit2 * 151);

		bit0 = (DrvColPROM[i] >> 3) & 0x01;
		bit1 = (DrvColPROM[i] >> 4) & 0x01;
		bit2 = (DrvColPROM[i] >> 5) & 0x01;
		UINT8 g = (bit0 * 33) + (bit1 * 71) + (bit2 * 151);

		bit0 = (DrvColPROM[i] >> 6) & 0x01;
		bit1 = (DrvColPROM[i] >> 7) & 0x01;
		UINT8 b = (bit0 * 81) + (bit1 * 174);

		t_pal[i] = BurnHighCol(r, g, b, 0);
	}

	for (INT32 i = 0; i < 256; i++)
	{
		UINT8 ctabentry = DrvColPROM[i + 0x100] & 0x0f;

		Palette[0x000 + i] = t_pal[ctabentry + 0x00];
		Palette[0x100 + i] = t_pal[ctabentry + 0x10];
	}

	DrvRecalc = 1;
}

static void convert_gfx()
{
	static INT32 PlaneOffsets[2]  = { 0, 4 };
	static INT32 CharXOffsets[8]  = { 64, 65, 66, 67, 0, 1, 2, 3 };
	static INT32 SpriXOffsets[16] = { 8*8, 8*8+1, 8*8+2, 8*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3, 0, 1, 2, 3 };
	static INT32 YOffsets[16]     = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 };

	INT32 size = (game_select == PENGO) ? 0x2000 : 0x1000;

	UINT8 *tmp = (UINT8*)BurnMalloc( size * 2 );

	if (tmp)
	{
		memcpy (tmp, DrvGfxROM, size * 2);

		GfxDecode(((size * 4) / 0x040), 2,  8,  8, PlaneOffsets, CharXOffsets + 0, YOffsets, 0x080, tmp,        DrvGfxROM + 0x0000);
		GfxDecode(((size * 4) / 0x100), 2, 16, 16, PlaneOffsets, SpriXOffsets + 0, YOffsets, 0x200, tmp + size, DrvGfxROM + 0x8000);
	}
	
	BurnFree (tmp);
}

static INT32 pacman_load()
{
	char* pRomName;
	struct BurnRomInfo ri;

	INT32 pOffset = 0;
	UINT8 *gLoad = DrvGfxROM;
	UINT8 *cLoad = DrvColPROM;
	UINT8 *sLoad = NamcoSoundProm;
	UINT8 *qLoad = DrvQROM;

	for (INT32 i = 0; !BurnDrvGetRomName(&pRomName, i, 0); i++) {

		BurnDrvGetRomInfo(&ri, i);

		if ((ri.nType & 7) == 1) {
			if (BurnLoadRom(DrvZ80ROM + pOffset, i, 1)) return 1;

			if (game_select == MSPACMAN) {
				pOffset += 0x1000;
			} else {
				pOffset += ri.nLen;
			}

			if (pOffset == 0x4000 && game_select != PENGO) {
				pOffset = 0x8000;
			}

			continue;
		}

		if ((ri.nType & 7) == 2) {
			if (BurnLoadRom(gLoad, i, 1)) return 1;
			gLoad += ri.nLen;

			continue;
		}

		if ((ri.nType & 7) == 3) {
			if (BurnLoadRom(cLoad, i, 1)) return 1;
			cLoad += 0x100;

			continue;
		}
		
		if ((ri.nType & 7) == 4) {
			if (BurnLoadRom(sLoad, i, 1)) return 1;
			sLoad += 0x100;

			continue;
		}

		if ((ri.nType & 7) == 7) {
			if (BurnLoadRom(qLoad, i, 1)) return 1;
			qLoad += ri.nLen;

			continue;
		}	
	}

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM		= Next; Next += 0x020000;

	DrvQROM			= Next;
	if (game_select == BIGBUCKS)    Next += 0x060000;
	if (game_select == ROCKTRV2)    Next += 0x040000;

	DrvGfxROM		= Next; Next += 0x010000;

	DrvColPROM		= Next; Next += 0x000500;
	NamcoSoundProm		= Next; Next += 0x000200;

	DrvTransTable		= Next; Next += 0x000200;

	Palette			= (UINT32*)Next; Next += 0x0200 * sizeof(UINT32);

	AllRam			= Next;

	DrvZ80RAM		= Next; Next += 0x001000;

	DrvSprRAM		= DrvZ80RAM + ((game_select == ALIBABA) ? 0x6f0 : 0x7f0);
	DrvSprRAM2		= Next; Next += 0x000010;

	DrvColRAM		= Next; Next += 0x000400;
	DrvVidRAM		= Next; Next += 0x000400;

	rocktrv2_prot_data	= Next; Next += 0x000004;

	flipscreen		= Next; Next += 0x000001;

	RamEnd			= Next;

	pAY8910Buffer[0] = (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);
	pAY8910Buffer[1] = (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);
	pAY8910Buffer[2] = (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);

	MemEnd			= Next;

	return 0;
}

static void StandardMap()
{
	for (INT32 i = 0; i <= 0x8000; i += 0x8000)// mirror
	{
		ZetMapArea(0x0000 + i, 0x3fff + i, 0, DrvZ80ROM);
		ZetMapArea(0x0000 + i, 0x3fff + i, 2, DrvZ80ROM);

		for (INT32 j = 0; j <= 0x2000; j+= 0x2000) // mirrors
		{
			ZetMapArea(0x4000 + i + j, 0x43ff + i + j, 0, DrvVidRAM);
			ZetMapArea(0x4000 + i + j, 0x43ff + i + j, 1, DrvVidRAM);
			ZetMapArea(0x4000 + i + j, 0x43ff + i + j, 2, DrvVidRAM);
			ZetMapArea(0x4400 + i + j, 0x47ff + i + j, 0, DrvColRAM);
			ZetMapArea(0x4400 + i + j, 0x47ff + i + j, 1, DrvColRAM);
			ZetMapArea(0x4400 + i + j, 0x47ff + i + j, 2, DrvColRAM);
			ZetMapArea(0x4c00 + i + j, 0x4fff + i + j, 0, DrvZ80RAM + 0x0400);
			ZetMapArea(0x4c00 + i + j, 0x4fff + i + j, 1, DrvZ80RAM + 0x0400);
			ZetMapArea(0x4c00 + i + j, 0x4fff + i + j, 2, DrvZ80RAM + 0x0400);
		}
	}

	ZetSetWriteHandler(pacman_write);
	ZetSetReadHandler(pacman_read);
	ZetSetOutHandler(pacman_out_port);
	ZetSetInHandler(pacman_in_port);
}

static void CannonbpMap()
{
	for (INT32 i = 0; i <= 0x8000; i += 0x8000) { // mirror

		ZetMapArea(0x0000 + i, 0x2fff + i, 0, DrvZ80ROM);
		ZetMapArea(0x0000 + i, 0x2fff + i, 2, DrvZ80ROM);

		for (INT32 j = 0; j <= 0x2000; j+= 0x2000) { // mirrors
			ZetMapArea(0x4000 + i + j, 0x43ff + i + j, 0, DrvVidRAM);
			ZetMapArea(0x4000 + i + j, 0x43ff + i + j, 1, DrvVidRAM);
			ZetMapArea(0x4000 + i + j, 0x43ff + i + j, 2, DrvVidRAM);
			ZetMapArea(0x4400 + i + j, 0x47ff + i + j, 0, DrvColRAM);
			ZetMapArea(0x4400 + i + j, 0x47ff + i + j, 1, DrvColRAM);
			ZetMapArea(0x4400 + i + j, 0x47ff + i + j, 2, DrvColRAM);
			ZetMapArea(0x4800 + i + j, 0x4bff + i + j, 0, DrvZ80RAM + 0x0000); 
			ZetMapArea(0x4800 + i + j, 0x4bff + i + j, 1, DrvZ80RAM + 0x0000);
			ZetMapArea(0x4800 + i + j, 0x4bff + i + j, 2, DrvZ80RAM + 0x0000);
			ZetMapArea(0x4c00 + i + j, 0x4fff + i + j, 0, DrvZ80RAM + 0x0400);
			ZetMapArea(0x4c00 + i + j, 0x4fff + i + j, 1, DrvZ80RAM + 0x0400);
			ZetMapArea(0x4c00 + i + j, 0x4fff + i + j, 2, DrvZ80RAM + 0x0400);
		}
	}

	ZetSetWriteHandler(pacman_write);
	ZetSetReadHandler(pacman_read);
	ZetSetOutHandler(pacman_out_port);
	ZetSetInHandler(pacman_in_port);
}

static void DremshprMap() // and vanvan
{
	CannonbpMap();

	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80ROM + 0x8000);
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80ROM + 0x8000);	
}

static void WoodpekMap()
{
	StandardMap();

	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80ROM + 0x8000);
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80ROM + 0x8000);
}

static void Rocktrv2Map()
{
	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80ROM);
	ZetMapArea(0x4000, 0x43ff, 0, DrvVidRAM);
	ZetMapArea(0x4000, 0x43ff, 1, DrvVidRAM);
	ZetMapArea(0x4000, 0x43ff, 2, DrvVidRAM);
	ZetMapArea(0x4400, 0x47ff, 0, DrvColRAM);
	ZetMapArea(0x4400, 0x47ff, 1, DrvColRAM);
	ZetMapArea(0x4400, 0x47ff, 2, DrvColRAM);
	ZetMapArea(0x4c00, 0x4fff, 0, DrvZ80RAM + 0x0400);
	ZetMapArea(0x4c00, 0x4fff, 1, DrvZ80RAM + 0x0400);
	ZetMapArea(0x4c00, 0x4fff, 2, DrvZ80RAM + 0x0400);

	ZetMapArea(0x6000, 0x7fff, 0, DrvZ80ROM + 0x8000);
	ZetMapArea(0x6000, 0x7fff, 2, DrvZ80ROM + 0x8000);

	ZetSetWriteHandler(pacman_write);
	ZetSetReadHandler(pacman_read);
	ZetSetOutHandler(pacman_out_port);
	ZetSetInHandler(pacman_in_port);
}

static void BigbucksMap()
{
	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80ROM);
	ZetMapArea(0x4000, 0x43ff, 0, DrvVidRAM);
	ZetMapArea(0x4000, 0x43ff, 1, DrvVidRAM);
	ZetMapArea(0x4000, 0x43ff, 2, DrvVidRAM);
	ZetMapArea(0x4400, 0x47ff, 0, DrvColRAM);
	ZetMapArea(0x4400, 0x47ff, 1, DrvColRAM);
	ZetMapArea(0x4400, 0x47ff, 2, DrvColRAM);
	ZetMapArea(0x4c00, 0x4fff, 0, DrvZ80RAM + 0x0400);
	ZetMapArea(0x4c00, 0x4fff, 1, DrvZ80RAM + 0x0400);
	ZetMapArea(0x4c00, 0x4fff, 2, DrvZ80RAM + 0x0400);

	ZetMapArea(0x8000, 0x9fff, 0, DrvZ80ROM + 0x8000);
	ZetMapArea(0x8000, 0x9fff, 2, DrvZ80ROM + 0x8000);

	ZetSetWriteHandler(pacman_write);
	ZetSetReadHandler(pacman_read);
	ZetSetOutHandler(pacman_out_port);
	ZetSetInHandler(pacman_in_port);
}

static void AlibabaMap()
{
	StandardMap();

	ZetMapArea(0x8000, 0x8fff, 0, DrvZ80ROM + 0x8000);
	ZetMapArea(0x8000, 0x8fff, 2, DrvZ80ROM + 0x8000);

	for (INT32 i = 0; i < 0x1000; i+= 0x400) {
		ZetMapArea(0x9000 + i, 0x93ff + i, 0, DrvZ80RAM + 0x0000);
		ZetMapArea(0x9000 + i, 0x93ff + i, 1, DrvZ80RAM + 0x0000);
		ZetMapArea(0x9000 + i, 0x93ff + i, 2, DrvZ80RAM + 0x0000);
	}

	for (INT32 i = 0x0000; i < 0x2000; i += 0x0800) {
		ZetMapArea(0xa000 + i, 0xa7ff + i, 0, DrvZ80ROM + 0xa000);
		ZetMapArea(0xa000 + i, 0xa7ff + i, 2, DrvZ80ROM + 0xa000);
	}
}

static void PengoMap()
{
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
//	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM + 0x8000, DrvZ80ROM);
	ZetMapArea(0x8000, 0x83ff, 0, DrvVidRAM);
	ZetMapArea(0x8000, 0x83ff, 1, DrvVidRAM);
	ZetMapArea(0x8000, 0x83ff, 2, DrvVidRAM);
	ZetMapArea(0x8400, 0x87ff, 0, DrvColRAM);
	ZetMapArea(0x8400, 0x87ff, 1, DrvColRAM);
	ZetMapArea(0x8400, 0x87ff, 2, DrvColRAM);
	ZetMapArea(0x8800, 0x8fff, 0, DrvZ80RAM);
	ZetMapArea(0x8800, 0x8fff, 1, DrvZ80RAM);
	ZetMapArea(0x8800, 0x8fff, 2, DrvZ80RAM);

	ZetSetWriteHandler(pengo_write);
	ZetSetReadHandler(pengo_read);
}

static void MspacmanMap()
{
	for (INT32 i = 0; i <= 0x8000; i += 0x8000)// mirror
	{
		for (INT32 j = 0; j <= 0x2000; j+= 0x2000) // mirrors
		{
			ZetMapArea(0x4000 + i + j, 0x43ff + i + j, 0, DrvVidRAM);
			ZetMapArea(0x4000 + i + j, 0x43ff + i + j, 1, DrvVidRAM);
			ZetMapArea(0x4000 + i + j, 0x43ff + i + j, 2, DrvVidRAM);
			ZetMapArea(0x4400 + i + j, 0x47ff + i + j, 0, DrvColRAM);
			ZetMapArea(0x4400 + i + j, 0x47ff + i + j, 1, DrvColRAM);
			ZetMapArea(0x4400 + i + j, 0x47ff + i + j, 2, DrvColRAM);
			ZetMapArea(0x4c00 + i + j, 0x4fff + i + j, 0, DrvZ80RAM + 0x0400);
			ZetMapArea(0x4c00 + i + j, 0x4fff + i + j, 1, DrvZ80RAM + 0x0400);
			ZetMapArea(0x4c00 + i + j, 0x4fff + i + j, 2, DrvZ80RAM + 0x0400);
		}
	}

	ZetSetWriteHandler(mspacman_write);
	ZetSetReadHandler(mspacman_read);
	ZetSetOutHandler(pacman_out_port);
}

static INT32 DrvInit(void (*mapCallback)(), void (*pInitCallback)(), INT32 select)
{
	game_select = select;

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	if (pacman_load()) return 1;

	if (pInitCallback) {
		pInitCallback();
	}

	convert_gfx();
	pacman_palette_init();
	DrvTransTableInit();

	ZetInit(0);
	ZetOpen(0);
	mapCallback();	
	ZetMemEnd();
	ZetClose();

	AY8910Init(0, 1789750, nBurnSoundRate, NULL, NULL, NULL, NULL);

	SN76496Init(0, 1789750, 0);
	SN76496Init(1, 1789750, 1);		

	NamcoSoundInit(18432000 / 6 / 32);

	GenericTilesInit();

	DrvDoReset(1);

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	ZetExit();

	AY8910Exit(0);
	SN76496Exit();
	NamcoSoundExit();

	game_select = PACMAN;
	acitya = 0;

	nPacBank = -1;

	BurnFree (AllMem);

	return 0;
}

static void DrawBackground()
{
	for (INT32 offs = 0; offs < 36 * 28; offs++)
	{
		INT32 sx = (offs % 36);
		INT32 sy = offs / 36;

		INT32 row = sy + 2;
		INT32 col = sx - 2;

		INT32 ofst;

		if (col & 0x20)
			ofst = row + ((col & 0x1f) << 5);
		else
			ofst = col + (row << 5);

		INT32 code  = (charbank << 8) | DrvVidRAM[ofst];
		INT32 color = (DrvColRAM[ofst] & 0x1f) | (colortablebank << 5) | (palettebank << 6);

		if (*flipscreen) {
			Render8x8Tile_FlipXY_Clip(pTransDraw, code, (272 - (sx * 8)) + 8, (224 - (sy * 8)) - 8, color, 2, 0, DrvGfxROM);
		} else {
			Render8x8Tile_Clip(pTransDraw, code, sx * 8, sy * 8, color, 2, 0, DrvGfxROM);
		}
	}
}

static void DrawSprites()
{
	for (INT32 offs = 0x10 - 2;offs >= 0;offs -= 2)
	{
		INT32 code   = (DrvSprRAM[offs] >> 2) | (spritebank << 6);
		INT32 color  = (DrvSprRAM[offs + 1] & 0x1f ) | (colortablebank << 5) | (palettebank << 6);

		INT32 sx     = DrvSprRAM2[offs + 1];
		INT32 sy     = DrvSprRAM2[offs];
		INT32 flipx  = DrvSprRAM [offs] & 1;
		INT32 flipy  = DrvSprRAM [offs] & 2;
		
		if (*flipscreen) {
			sy = (240 - sy) - 8;
			sx += 8;
			flipy = !flipy;
			flipx = !flipx;
		} else {
			sx = 272 - sx;
			sy = sy - 31;
		}

		RenderTileTranstab(pTransDraw, DrvGfxROM + 0x8000, code, color << 2, 0, sx, sy, flipx, flipy, 16, 16, DrvTransTable);
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		pacman_palette_init();
		DrvRecalc = 0;
	}

	DrawBackground();
	DrawSprites();

	BurnTransferCopy(Palette);

	return 0;
}

static INT32 DrvFrame()
{
	watchdog++;
	if (watchdog >= 16) {
		bprintf (0, _T("watchdog triggered\n"));
		DrvDoReset(0);
	}

	if (DrvReset) {
		DrvDoReset(1);
	}

	{
		memset (DrvInputs, 0, 2);

		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}

		if (!acitya && game_select != SHOOTBUL) {
			if ((DrvInputs[0] & 6) == 6) DrvInputs[0] &= ~0x06;
			if ((DrvInputs[0] & 9) == 9) DrvInputs[0] &= ~0x09;
			if ((DrvInputs[1] & 6) == 6) DrvInputs[1] &= ~0x06;
			if ((DrvInputs[1] & 9) == 9) DrvInputs[1] &= ~0x09;
		}

		DrvInputs[0] ^= DrvDips[0];
		DrvInputs[1] ^= DrvDips[1];

		nAnalogAxis[0] -= DrvAxis[0];
		nAnalogAxis[1] -= DrvAxis[1];
		
		nCharAxis[0] = (DrvAxis[0] >> 12) & 0x0f;
		nCharAxis[1] = (DrvAxis[1] >> 12) & 0x0f;

		if (game_select == SHOOTBUL) {
			DrvInputs[0] ^= nCharAxis[0];
			DrvInputs[1] ^= nCharAxis[1];
		}
	}

	ZetOpen(0);
	
	INT32 nInterleave = nBurnSoundLen;
	INT32 nSoundBufferPos = 0;
	
	INT32 nCyclesTotal = (18432000 / 6) / 60;
	INT32 nCyclesDone = 0;
	INT32 nCyclesSegment;
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nNext;
		
		nNext = (i + 1) * nCyclesTotal / nInterleave;
		nCyclesSegment = nNext - nCyclesDone;
		nCyclesDone += ZetRun(nCyclesSegment);
		
		if (game_select == BIGBUCKS) {
			INT32 nInterleaveIRQFire = nBurnSoundLen / 20;
			for (INT32 j = 0; j < 20; j++) {
				if (i == (nInterleaveIRQFire * j) - 1) ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
			}
		} else {
			if (game_select == DREMSHPR || game_select == VANVAN) {
				if (i == (nInterleave - 1)) ZetNmi();
			} else {
				if (i == (nInterleave - 1) && interrupt_mask) {
					ZetSetVector(interrupt_mode);
					ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
				}
			}
		}
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			
			if (nSegmentLength) {
				if (game_select == DREMSHPR || game_select == CRUSHS) {
					INT32 nSample;
				
					AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
					for (INT32 n = 0; n < nSegmentLength; n++) {
						nSample  = pAY8910Buffer[0][n];
						nSample += pAY8910Buffer[1][n];
						nSample += pAY8910Buffer[2][n];
	
						nSample /= 4;

						nSample = BURN_SND_CLIP(nSample);
						
						pSoundBuf[(n << 1) + 0] = nSample;
						pSoundBuf[(n << 1) + 1] = nSample;
					}
				} else {
					if (game_select == VANVAN) {
						SN76496Update(0, pSoundBuf, nSegmentLength);
						SN76496Update(1, pSoundBuf, nSegmentLength);
					} else {
						NamcoSoundUpdate(pSoundBuf, nSegmentLength);
					}
				}
			}
			nSoundBufferPos += nSegmentLength;
		}
	}
	
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			if (game_select == DREMSHPR || game_select == CRUSHS) {
				INT32 nSample;
				
				AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
				for (INT32 n = 0; n < nSegmentLength; n++) {
					nSample  = pAY8910Buffer[0][n];
					nSample += pAY8910Buffer[1][n];
					nSample += pAY8910Buffer[2][n];
	
					nSample /= 4;

					nSample = BURN_SND_CLIP(nSample);
						
					pSoundBuf[(n << 1) + 0] = nSample;
					pSoundBuf[(n << 1) + 1] = nSample;
				}
			} else {
				if (game_select == VANVAN) {
					SN76496Update(0, pSoundBuf, nSegmentLength);
					SN76496Update(1, pSoundBuf, nSegmentLength);
				} else {
					NamcoSoundUpdate(pSoundBuf, nSegmentLength);
				}
			}
		}
	}

	ZetClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

//------------------------------------------------------------------------------------------------------

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029693;
	}

	if (nAction & ACB_VOLATILE) {	
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);

		NamcoSoundScan(nAction, pnMin);
		AY8910Scan(nAction, pnMin);
		SN76496Scan(nAction, pnMin);

		SCAN_VAR(nPacBank);

		SCAN_VAR(interrupt_mode);
		SCAN_VAR(interrupt_mask);

		SCAN_VAR(colortablebank);
		SCAN_VAR(palettebank);
		SCAN_VAR(spritebank);
		SCAN_VAR(charbank);

		SCAN_VAR(alibaba_mystery);
		SCAN_VAR(epos_hardware_counter);
		SCAN_VAR(mschamp_counter);
		SCAN_VAR(cannonb_bit_to_read);
	}

	if (nAction & ACB_WRITE) {
		if (game_select == MSCHAMP) {
			mschamp_set_bank();
		}

		if (game_select == EPOS) {
			epos_hardware_set_bank(nPacBank);
		}
	}

	return 0;
}


//------------------------------------------------------------------------------------------------------


// PuckMan (Japan set 1, Probably Bootleg)

static struct BurnRomInfo puckmanRomDesc[] = {
	{ "namcopac.6e",  0x1000, 0xfee263b3, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "namcopac.6f",  0x1000, 0x39d1fc83, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "namcopac.6h",  0x1000, 0x02083b03, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "namcopac.6j",  0x1000, 0x7a36fe55, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "pacman.5e",    0x1000, 0x0c944964, 2 | BRF_GRA },		//  4 Graphics
	{ "pacman.5f",    0x1000, 0x958fedf9, 2 | BRF_GRA },		//  5

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  6 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  7

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(puckman)
STD_ROM_FN(puckman)

static INT32 puckmanInit()
{
	return DrvInit(StandardMap, NULL, PACMAN);
}

struct BurnDriver BurnDrvpuckman = {
	"puckman", NULL, NULL, NULL, "1980",
	"PuckMan (Japan set 1, Probably Bootleg)\0", NULL, "Namco", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, puckmanRomInfo, puckmanRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// PuckMan (Japan set 1 with speedup hack)

static struct BurnRomInfo puckmanfRomDesc[] = {
	{ "namcopac.6e",  0x1000, 0xfee263b3, 1 | BRF_ESS | BRF_PRG },  //  0 Z80 Code
	{ "nampfast.6f",  0x1000, 0x51b38db9, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "namcopac.6h",  0x1000, 0x02083b03, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "namcopac.6j",  0x1000, 0x7a36fe55, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "pacman.5e",    0x1000, 0x0c944964, 2 | BRF_GRA },		//  4 Graphics
	{ "pacman.5f",    0x1000, 0x958fedf9, 2 | BRF_GRA },		//  5

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  6 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  7

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(puckmanf)
STD_ROM_FN(puckmanf)

struct BurnDriver BurnDrvpuckmanf = {
	"puckmanf", "puckman", NULL, NULL, "1980",
	"PuckMan (Japan set 1 with speedup hack)\0", NULL, "Namco", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, puckmanfRomInfo, puckmanfRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// PuckMan (Japan set 3)

static struct BurnRomInfo puckmodRomDesc[] = {
	{ "namcopac.6e",  0x1000, 0xfee263b3, 1 | BRF_ESS | BRF_PRG },  //  0 Z80 Code
	{ "namcopac.6f",  0x1000, 0x39d1fc83, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "namcopac.6h",  0x1000, 0x02083b03, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "npacmod.6j",   0x1000, 0x7d98d5f5, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "pacman.5e",    0x1000, 0x0c944964, 2 | BRF_GRA },		//  4 Graphics
	{ "pacman.5f",    0x1000, 0x958fedf9, 2 | BRF_GRA },		//  5

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  6 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  7

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(puckmod)
STD_ROM_FN(puckmod)

struct BurnDriver BurnDrvpuckmod = {
	"puckmod", "puckman", NULL, NULL, "1981",
	"PuckMan (Japan set 3)\0", NULL, "Namco", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, puckmodRomInfo, puckmodRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// PuckMan (Japan set 2)

static struct BurnRomInfo puckmanaRomDesc[] = {
	{ "pacman.6e",    0x1000, 0xc1e6ab10, 1 | BRF_ESS | BRF_PRG },  //  0 Z80 Code
	{ "pacman.6f",    0x1000, 0x1a6fb2d4, 1 | BRF_ESS | BRF_PRG },  //  1
	{ "pacman.6h",    0x1000, 0xbcdd1beb, 1 | BRF_ESS | BRF_PRG },  //  2
	{ "prg7",         0x0800, 0xb6289b26, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "prg8",         0x0800, 0x17a88c13, 1 | BRF_ESS | BRF_PRG },	//  4

	{ "chg1",         0x0800, 0x2066a0b7, 2 | BRF_GRA },		//  5 Graphics
	{ "chg2",         0x0800, 0x3591b89d, 2 | BRF_GRA },		//  6
	{ "pacman.5f",    0x1000, 0x958fedf9, 2 | BRF_GRA },		//  7

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  8 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  9

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 10 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 11 Timing Prom (not used)
};

STD_ROM_PICK(puckmana)
STD_ROM_FN(puckmana)

struct BurnDriver BurnDrvpuckmana = {
	"puckmana", "puckman", NULL, NULL, "1981",
	"PuckMan (Japan set 2)\0", NULL, "Namco", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, puckmanaRomInfo, puckmanaRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Pac-Man (Midway)

static struct BurnRomInfo pacmanRomDesc[] = {
	{ "pacman.6e",    0x1000, 0xc1e6ab10, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "pacman.6f",    0x1000, 0x1a6fb2d4, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "pacman.6h",    0x1000, 0xbcdd1beb, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "pacman.6j",    0x1000, 0x817d94e3, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "pacman.5e",    0x1000, 0x0c944964, 2 | BRF_GRA },		//  4 Graphics
	{ "pacman.5f",    0x1000, 0x958fedf9, 2 | BRF_GRA },		//  5

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  6 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  7

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(pacman)
STD_ROM_FN(pacman)

struct BurnDriver BurnDrvpacman = {
	"pacman", "puckman", NULL, NULL, "1980",
	"Pac-Man (Midway)\0", NULL, "[Namco] (Midway license)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, pacmanRomInfo, pacmanRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Pac-Man (Midway, with speedup hack)

static struct BurnRomInfo pacmanfRomDesc[] = {
	{ "pacman.6e",    0x1000, 0xc1e6ab10, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "pacfast.6f",   0x1000, 0x720dc3ee, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "pacman.6h",    0x1000, 0xbcdd1beb, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "pacman.6j",    0x1000, 0x817d94e3, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "pacman.5e",    0x1000, 0x0c944964, 2 | BRF_GRA },		//  4 Graphics
	{ "pacman.5f",    0x1000, 0x958fedf9, 2 | BRF_GRA },		//  5

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  6 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  7

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(pacmanf)
STD_ROM_FN(pacmanf)

struct BurnDriver BurnDrvpacmanf = {
	"pacmanf", "puckman", NULL, NULL, "1980",
	"Pac-Man (Midway, with speedup hack)\0", NULL, "[Namco] (Midway license)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, pacmanfRomInfo, pacmanfRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Hangly-Man (set 1)

static struct BurnRomInfo hanglyRomDesc[] = {
	{ "hangly.6e",    0x1000, 0x5fe8610a, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "hangly.6f",    0x1000, 0x73726586, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "hangly.6h",    0x1000, 0x4e7ef99f, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "hangly.6j",    0x1000, 0x7f4147e6, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "pacman.5e",    0x1000, 0x0c944964, 2 | BRF_GRA },		//  4 Graphics
	{ "pacman.5f",    0x1000, 0x958fedf9, 2 | BRF_GRA },		//  5

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  6 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  7

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(hangly)
STD_ROM_FN(hangly)

struct BurnDriver BurnDrvhangly = {
	"hangly", "puckman", NULL, NULL, "1981",
	"Hangly-Man (set 1)\0", NULL, "hack", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, hanglyRomInfo, hanglyRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Hangly-Man (set 2)

static struct BurnRomInfo hangly2RomDesc[] = {
	{ "hangly.6e",    0x1000, 0x5fe8610a, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "hangly2.6f",   0x0800, 0x5ba228bb, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "hangly2.6m",   0x0800, 0xbaf5461e, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "hangly.6h",    0x1000, 0x4e7ef99f, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "hangly2.6j",   0x0800, 0x51305374, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "hangly2.6p",   0x0800, 0x427c9d4d, 1 | BRF_ESS | BRF_PRG },	//  5

	{ "pacmanh.5e",   0x1000, 0x299fb17a, 2 | BRF_GRA },		//  6 Graphics
	{ "pacman.5f",    0x1000, 0x958fedf9, 2 | BRF_GRA },		//  7

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  8 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  9

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 10 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 11 Timing Prom (not used)
};

STD_ROM_PICK(hangly2)
STD_ROM_FN(hangly2)

struct BurnDriver BurnDrvhangly2 = {
	"hangly2", "puckman", NULL, NULL, "1981",
	"Hangly-Man (set 2)\0", NULL, "hack", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, hangly2RomInfo, hangly2RomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Hangly-Man (set 3)

static struct BurnRomInfo hangly3RomDesc[] = {
	{ "hm1.6e",       0x0800, 0x9d027c4a, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "hm5.6k",	  0x0800, 0x194c7189, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "hangly2.6f",   0x0800, 0x5ba228bb, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "hangly2.6m",   0x0800, 0xbaf5461e, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "hm3.6h",       0x0800, 0x08419c4a, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "hm7.6n",       0x0800, 0xab74b51f, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "hm4.6j",       0x0800, 0x5039b082, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "hm8.6p",       0x0800, 0x931770d7, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "hm9.5e", 	  0x0800, 0x5f4be3cc, 2 | BRF_GRA },		//  8 Graphics
	{ "hm11.5h",      0x0800, 0x3591b89d, 2 | BRF_GRA },		//  9
	{ "hm10.5f", 	  0x0800, 0x9e39323a, 2 | BRF_GRA },		// 10
	{ "hm12.5j", 	  0x0800, 0x1b1d9096, 2 | BRF_GRA },		// 11

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
};

STD_ROM_PICK(hangly3)
STD_ROM_FN(hangly3)

struct BurnDriver BurnDrvhangly3 = {
	"hangly3", "puckman", NULL, NULL, "1981",
	"Hangly-Man (set 3)\0", NULL, "hack", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, hangly3RomInfo, hangly3RomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Popeye Man

static struct BurnRomInfo popeyemanRomDesc[] = {
	{ "pop1.6e",      0x0800, 0x9d027c4a, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "pop5.6k",	  0x0800, 0x194c7189, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "pop2.6f",      0x0800, 0x5ba228bb, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "pop6.6m",      0x0800, 0xbaf5461e, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "pop3.6h",      0x0800, 0x08419c4a, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "pop7.6n",      0x0800, 0xab74b51f, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "pop4.6j",      0x0800, 0x5039b082, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "pop8.6p",      0x0800, 0x931770d7, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "pop9.5e", 	  0x0800, 0xb569c4c1, 2 | BRF_GRA },		//  8 Graphics
	{ "pop11.5h",     0x0800, 0x3591b89d, 2 | BRF_GRA },		//  9
	{ "pop10.5f", 	  0x0800, 0x014fb5a4, 2 | BRF_GRA },		// 10
	{ "pop12.5j", 	  0x0800, 0x21b91c64, 2 | BRF_GRA },		// 11

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
};

STD_ROM_PICK(popeyeman)
STD_ROM_FN(popeyeman)

struct BurnDriver BurnDrvpopeyeman = {
	"popeyeman", "puckman", NULL, NULL, "1981",
	"Popeye-Man\0", NULL, "hack", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HACK, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, popeyemanRomInfo, popeyemanRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Crock-Man

static struct BurnRomInfo crockmanRomDesc[] = {
	{ "p1.6e",        0x0800, 0x2c0fa0ab, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "p5.6k",	  0x0800, 0xafeca2f1, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "p2.6f",        0x0800, 0x7d177853, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "p6.6m",        0x0800, 0xd3e8914c, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "p3.6h",        0x0800, 0x9045a44c, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "p7.6n",        0x0800, 0x93f344c5, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "p4.6j",        0x0800, 0xbed4a077, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "p8.6p",        0x0800, 0x800be41e, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "p9.5e", 	  0x0800, 0xa10218c4, 2 | BRF_GRA },		//  8 Graphics
	{ "p11.5h",       0x0800, 0x3591b89d, 2 | BRF_GRA },		//  9
	{ "p10.5f", 	  0x0800, 0x9e39323a, 2 | BRF_GRA },		// 10
	{ "p12.5j", 	  0x0800, 0x1b1d9096, 2 | BRF_GRA },		// 11

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
};

STD_ROM_PICK(crockman)
STD_ROM_FN(crockman)

struct BurnDriver BurnDrvcrockman = {
	"crockman", "puckman", NULL, NULL, "1980",
	"Crock-Man (bootleg, Rene-Pierre)\0", NULL, "bootleg (Rene-Pierre)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, crockmanRomInfo, crockmanRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Pac-Man (Midway, harder)

static struct BurnRomInfo pacmodRomDesc[] = {
	{ "pacmanh.6e",   0x1000, 0x3b2ec270, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "pacman.6f",    0x1000, 0x1a6fb2d4, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "pacmanh.6h",   0x1000, 0x18811780, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "pacmanh.6j",   0x1000, 0x5c96a733, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "pacmanh.5e",   0x1000, 0x299fb17a, 2 | BRF_GRA },		//  4 Graphics
	{ "pacman.5f",    0x1000, 0x958fedf9, 2 | BRF_GRA },		//  5

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  6 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  7

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(pacmod)
STD_ROM_FN(pacmod)

struct BurnDriver BurnDrvpacmod = {
	"pacmod", "puckman", NULL, NULL, "1981",
	"Pac-Man (Midway, harder)\0", NULL, "[Namco] (Midway license)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, pacmodRomInfo, pacmodRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Puckman (Falcom?)

static struct BurnRomInfo puckmanhRomDesc[] = {
	{ "pm01.6e",      0x1000, 0x5fe8610a, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "pm02.6f",      0x1000, 0x61d38c6c, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "pm03.6h",      0x1000, 0x4e7ef99f, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "pm04.6j",      0x1000, 0x8939ddd2, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "pm9.5e",       0x0800, 0x2229ab07, 2 | BRF_GRA },		//  4 Graphics
	{ "pm11.5h",      0x0800, 0x3591b89d, 2 | BRF_GRA },		//  5
	{ "pm10.5f",      0x0800, 0x9e39323a, 2 | BRF_GRA },		//  6
	{ "pm12.5j",      0x0800, 0x1b1d9096, 2 | BRF_GRA },		//  7

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  8 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  9

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 10 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 11 Timing Prom (not used)
};

STD_ROM_PICK(puckmanh)
STD_ROM_FN(puckmanh)

struct BurnDriver BurnDrvpuckmanh = {
	"puckmanh", "puckman", NULL, NULL, "1981",
	"Puckman (Falcom?)\0", NULL, "hack", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, puckmanhRomInfo, puckmanhRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// New Puck-X

static struct BurnRomInfo newpuckxRomDesc[] = {
	{ "puckman.6e",   0x1000, 0xa8ae23c5, 1 | BRF_ESS | BRF_PRG },  //  0 Z80 Code
	{ "pacman.6f",    0x1000, 0x1a6fb2d4, 1 | BRF_ESS | BRF_PRG },  //  1
	{ "puckman.6h",   0x1000, 0x197443f8, 1 | BRF_ESS | BRF_PRG },  //  2
	{ "puckman.6j",   0x1000, 0x2e64a3ba, 1 | BRF_ESS | BRF_PRG },  //  3

	{ "pacman.5e",    0x1000, 0x0c944964, 2 | BRF_GRA },		//  4 Graphics
	{ "pacman.5f",    0x1000, 0x958fedf9, 2 | BRF_GRA },		//  5

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  6 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  7

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(newpuckx)
STD_ROM_FN(newpuckx)

struct BurnDriver BurnDrvnewpuckx = {
	"newpuckx", "puckman", NULL, NULL, "1980",
	"New Puck-X\0", NULL, "hack", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, newpuckxRomInfo, newpuckxRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Pac-Man (Hearts)

static struct BurnRomInfo pacheartRomDesc[] = {
	{ "pacheart1.6e", 0x0800, 0xd844b679, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "pacheart.pg2", 0x0800, 0xb9152a38, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "pacheart2.6f", 0x0800, 0x7d177853, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "pacheart.pg4", 0x0800, 0x842d6574, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "pacheart3.6h", 0x0800, 0x9045a44c, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "pacheart7.6n", 0x0800, 0x888f3c3e, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "pacheart.pg7", 0x0800, 0xf5265c10, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "pacheart.pg8", 0x0800, 0x1a21a381, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "pacheart.ch1", 0x0800, 0xc62bbabf, 2 | BRF_GRA },		//  8 Graphics
	{ "chg2",         0x0800, 0x3591b89d, 2 | BRF_GRA },		//  9
	{ "pacheart.ch3", 0x0800, 0xca8c184c, 2 | BRF_GRA },		// 10
	{ "pacheart.ch4", 0x0800, 0x1b1d9096, 2 | BRF_GRA },		// 11

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
};

STD_ROM_PICK(pacheart)
STD_ROM_FN(pacheart)

struct BurnDriver BurnDrvpacheart = {
	"pacheart", "puckman", NULL, NULL, "1981",
	"Pac-Man (Hearts)\0", NULL, "hack", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, pacheartRomInfo, pacheartRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Joyman

static struct BurnRomInfo joymanRomDesc[] = {
	{ "joy1.6e",      0x0800, 0xd844b679, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "joy5.6k",      0x0800, 0xab9c8f29, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "joy2.6f",      0x0800, 0x7d177853, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "joy6.6m",      0x0800, 0xb3c8d32e, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "joy3.6h",      0x0800, 0x9045a44c, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "joy7.6n",      0x0800, 0x888f3c3e, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "joy4.6j",      0x0800, 0x00b553f8, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "joy8.6p",      0x0800, 0x5d5ce992, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "joy9.5e",  	  0x0800, 0x39b557bc, 2 | BRF_GRA },		//  8 Graphics
	{ "joy11.5h", 	  0x0800, 0x33e0289e, 2 | BRF_GRA },		//  9
	{ "joy10.5f", 	  0x0800, 0x338771a6, 2 | BRF_GRA },		// 10
	{ "joy12.5j", 	  0x0800, 0xf4f0add5, 2 | BRF_GRA },		// 11

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
};

STD_ROM_PICK(joyman)
STD_ROM_FN(joyman)

struct BurnDriver BurnDrvjoyman = {
	"joyman", "puckman", NULL, NULL, "1982",
	"Joyman\0", NULL, "hack", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, joymanRomInfo, joymanRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Buccaneer

static struct BurnRomInfo bucanerRomDesc[] = {
	{ "buc1.6e",      0x0800, 0x2c0fa0ab, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "buc5.6k",      0x0800, 0xafeca2f1, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "buc2.6f",      0x0800, 0x6b53ada9, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "buc6.6m",      0x0800, 0x35f3ca84, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "buc3.6h",      0x0800, 0x9045a44c, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "buc7.6n",      0x0800, 0x888f3c3e, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "buc4.6j",      0x0800, 0x292de161, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "buc8.6p",      0x0800, 0x884af858, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "buc9.5e",  	  0x0800, 0x4060c077, 2 | BRF_GRA },		//  8 Graphics
	{ "buc11.5h", 	  0x0800, 0xe3861283, 2 | BRF_GRA },		//  9
	{ "buc10.5f", 	  0x0800, 0x09f66dec, 2 | BRF_GRA },		// 10
	{ "buc12.5j", 	  0x0800, 0x653314e7, 2 | BRF_GRA },		// 11

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
};

STD_ROM_PICK(bucaner)
STD_ROM_FN(bucaner)

struct BurnDriver BurnDrvbucaner = {
	"bucaner", "puckman", NULL, NULL, "19??",
	"Buccaneer\0", NULL, "hack", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, bucanerRomInfo, bucanerRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Caterpillar Pacman Hack

static struct BurnRomInfo ctrpllrpRomDesc[] = {
	{ "c1.bin",       0x0800, 0x9d027c4a, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "c5.bin",       0x0800, 0xf39846d3, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "c2.bin",       0x0800, 0xafa149a8, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "c6.bin",       0x0800, 0xbaf5461e, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "c3.bin",       0x0800, 0x6bb282a1, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "c7.bin",       0x0800, 0xfa2140f5, 1 | BRF_ESS | BRF_PRG },	//  5 
	{ "c4.bin",       0x0800, 0x86c91e0e, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "c8.bin",       0x0800, 0x3d28134e, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "c9.bin",       0x0800, 0x1c4617be, 2 | BRF_GRA },		//  8 Graphics
	{ "c11.bin",      0x0800, 0x46f72fef, 2 | BRF_GRA },		//  9
	{ "c10.bin",      0x0800, 0xba9ec199, 2 | BRF_GRA },		// 10
	{ "c12.bin",      0x0800, 0x41c09655, 2 | BRF_GRA },		// 11

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
};

STD_ROM_PICK(ctrpllrp)
STD_ROM_FN(ctrpllrp)

struct BurnDriver BurnDrvctrpllrp = {
	"ctrpllrp", "puckman", NULL, NULL, "1982",
	"Caterpillar Pacman Hack\0", NULL, "hack", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, ctrpllrpRomInfo, ctrpllrpRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Pac-Man Plus

static struct BurnRomInfo pacplusRomDesc[] = {
	{ "pacplus.6e",   0x1000, 0xd611ef68, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "pacplus.6f",   0x1000, 0xc7207556, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "pacplus.6h",   0x1000, 0xae379430, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "pacplus.6j",   0x1000, 0x5a6dff7b, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "pacplus.5e",   0x1000, 0x022c35da, 2 | BRF_GRA },		//  4 Graphics
	{ "pacplus.5f",   0x1000, 0x4de65cdd, 2 | BRF_GRA },		//  5

	{ "pacplus.7f",   0x0020, 0x063dd53a, 3 | BRF_GRA },		//  6 Color Proms
	{ "pacplus.4a",   0x0100, 0xe271a166, 3 | BRF_GRA },		//  7

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(pacplus)
STD_ROM_FN(pacplus)

static UINT8 pacplus_decrypt(INT32 addr, UINT8 e)
{
	static const UINT8 swap_xor_table[6][9] =
	{
		{ 7,6,5,4,3,2,1,0, 0x00 },
		{ 7,6,5,4,3,2,1,0, 0x28 },
		{ 6,1,3,2,5,7,0,4, 0x96 },
		{ 6,1,5,2,3,7,0,4, 0xbe },
		{ 0,3,7,6,4,2,1,5, 0xd5 },
		{ 0,3,4,6,7,2,1,5, 0xdd }
	};

	static const INT32 picktable[32] =
	{
		0,2,4,2,4,0,4,2,2,0,2,2,4,0,4,2,
		2,2,4,0,4,2,4,0,0,4,0,4,4,2,4,2
	};

	INT32 method = picktable[(addr & 0x001) | ((addr & 0x004) >> 1) | ((addr & 0x020) >> 3) | ((addr & 0x080) >> 4) | ((addr & 0x200) >> 5)];

	if (addr & 0x800) method ^= 1;

	const UINT8 *tbl = swap_xor_table[method];

	return BITSWAP08(e,tbl[0],tbl[1],tbl[2],tbl[3],tbl[4],tbl[5],tbl[6],tbl[7]) ^ tbl[8];
}

void pacplus_decode()
{
	for (INT32 i = 0; i < 0x4000; i++)
	{
		DrvZ80ROM[i] = pacplus_decrypt(i, DrvZ80ROM[i]);
	}
}

static INT32 pacplusInit()
{
	return DrvInit(StandardMap, pacplus_decode, PACMAN);
}

struct BurnDriver BurnDrvpacplus = {
	"pacplus", NULL, NULL, NULL, "1982",
	"Pac-Man Plus\0", NULL, "[Namco] (Midway license)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, pacplusRomInfo, pacplusRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	pacplusInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Newpuc2

static struct BurnRomInfo newpuc2RomDesc[] = {
	{ "6e.cpu", 	  0x0800, 0x69496a98, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "6k.cpu", 	  0x0800, 0x158fc01c, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "6f.cpu", 	  0x0800, 0x7d177853, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "6m.cpu", 	  0x0800, 0x70810ccf, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "6h.cpu", 	  0x0800, 0x81719de8, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "6n.cpu", 	  0x0800, 0x3f250c58, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "6j.cpu", 	  0x0800, 0xe6675736, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "6p.cpu", 	  0x0800, 0x1f81e765, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "5e.cpu", 	  0x0800, 0x2066a0b7, 2 | BRF_GRA },		//  8 Graphics
	{ "5h.cpu", 	  0x0800, 0x777c70d3, 2 | BRF_GRA },		//  9
	{ "5f.cpu", 	  0x0800, 0xca8c184c, 2 | BRF_GRA },		// 10
	{ "5j.cpu", 	  0x0800, 0x7dc75a81, 2 | BRF_GRA },		// 11

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
};

STD_ROM_PICK(newpuc2)
STD_ROM_FN(newpuc2)

struct BurnDriver BurnDrvnewpuc2 = {
	"newpuc2", "puckman", NULL, NULL, "1980",
	"Newpuc2\0", NULL, "hack", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, newpuc2RomInfo, newpuc2RomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Newpuc2b

static struct BurnRomInfo newpuc2bRomDesc[] = {
	{ "np2b1.bin",    0x0800, 0x9d027c4a, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "6k.cpu", 	  0x0800, 0x158fc01c, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "6f.cpu", 	  0x0800, 0x7d177853, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "6m.cpu", 	  0x0800, 0x70810ccf, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "np2b3.bin",    0x0800, 0xf5e4b2b1, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "6n.cpu", 	  0x0800, 0x3f250c58, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "np2b4.bin",    0x0800, 0xf068e009, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "np2b8.bin",    0x0800, 0x1fadcc2f, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "5e.cpu", 	  0x0800, 0x2066a0b7, 2 | BRF_GRA },		//  8 Graphics
	{ "5h.cpu", 	  0x0800, 0x777c70d3, 2 | BRF_GRA },		//  9
	{ "5f.cpu", 	  0x0800, 0xca8c184c, 2 | BRF_GRA },		// 10
	{ "5j.cpu", 	  0x0800, 0x7dc75a81, 2 | BRF_GRA },		// 11

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Prom
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
};

STD_ROM_PICK(newpuc2b)
STD_ROM_FN(newpuc2b)

struct BurnDriver BurnDrvnewpuc2b = {
	"newpuc2b", "puckman", NULL, NULL, "1980",
	"Newpuc2b\0", NULL, "hack", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, newpuc2bRomInfo, newpuc2bRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// MS Pacman

static struct BurnRomInfo mspacmanRomDesc[] = {
	{ "pacman.6e",    0x1000, 0xc1e6ab10, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "pacman.6f",    0x1000, 0x1a6fb2d4, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "pacman.6h",    0x1000, 0xbcdd1beb, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "pacman.6j",    0x1000, 0x817d94e3, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "u5",           0x0800, 0xf45fbbcd, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "u6",           0x1000, 0xa90e7000, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "u7",           0x1000, 0xc82cd714, 1 | BRF_ESS | BRF_PRG },	//  6

	{ "5e",           0x1000, 0x5c281d01, 2 | BRF_GRA },		//  7 Graphics
	{ "5f",           0x1000, 0x615af909, 2 | BRF_GRA },		//  8

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  9 Color Prom
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		// 10

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 11 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 12 Timing Prom (not used)
};

STD_ROM_PICK(mspacman)
STD_ROM_FN(mspacman)

static void MspacmanDecode()
{
#define ADD0SWAP(x) BITSWAP16(x,15,14,13,12,11,3,7,9,10,8,6,5,4,2,1,0)
#define ADD1SWAP(x) BITSWAP16(x,15,14,13,12,11,8,7,5,9,10,6,3,4,2,1,0)
#define DATASWAP(x) BITSWAP08(x,0,4,5,7,6,3,2,1)

	static const UINT16 tab[10 * 8] = { // even is dst, odd is src
		0x0410, 0x8008, 0x08E0, 0x81D8, 0x0A30, 0x8118, 0x0BD0, 0x80D8, 
		0x0C20, 0x8120, 0x0E58, 0x8168, 0x0EA8, 0x8198, 0x1000, 0x8020, 
		0x1008, 0x8010, 0x1288, 0x8098, 0x1348, 0x8048, 0x1688, 0x8088, 
		0x16B0, 0x8188, 0x16D8, 0x80C8, 0x16F8, 0x81C8, 0x19A8, 0x80A8, 
		0x19B8, 0x81A8, 0x2060, 0x8148, 0x2108, 0x8018, 0x21A0, 0x81A0, 
		0x2298, 0x80A0, 0x23E0, 0x80E8, 0x2418, 0x8000, 0x2448, 0x8058, 
		0x2470, 0x8140, 0x2488, 0x8080, 0x24B0, 0x8180, 0x24D8, 0x80C0, 
		0x24F8, 0x81C0, 0x2748, 0x8050, 0x2780, 0x8090, 0x27B8, 0x8190, 
		0x2800, 0x8028, 0x2B20, 0x8100, 0x2B30, 0x8110, 0x2BF0, 0x81D0, 
		0x2CC0, 0x80D0, 0x2CD8, 0x80E0, 0x2CF0, 0x81E0, 0x2D60, 0x8160
	};

	memcpy (DrvZ80ROM + 0x0b000, DrvZ80ROM + 0x0a000, 0x01000);
	memcpy (DrvZ80ROM + 0x10000, DrvZ80ROM + 0x00000, 0x03000);

	for (INT32 i = 0; i < 0x1000; i++)
	{
		DrvZ80ROM[0x13000+i] = DATASWAP(DrvZ80ROM[0xb000+ADD0SWAP(i)]);
	}

	for (INT32 i = 0; i < 0x800; i++)
	{
		DrvZ80ROM[0x18000+i] = DATASWAP(DrvZ80ROM[0x8000+ADD1SWAP(i)]);
		DrvZ80ROM[0x18800+i] = DATASWAP(DrvZ80ROM[0x9800+ADD0SWAP(i)]);
		DrvZ80ROM[0x19000+i] = DATASWAP(DrvZ80ROM[0x9000+ADD0SWAP(i)]);
		DrvZ80ROM[0x19800+i] = DrvZ80ROM[0x1800+i];
	}

	memcpy (DrvZ80ROM + 0x1a000, DrvZ80ROM + 0x02000, 0x2000);

	for (INT32 i = 0; i < 80; i+=2) { // apply patches
		memcpy (DrvZ80ROM + 0x10000 + tab[i], DrvZ80ROM + 0x10000 + tab[i+1], 8);
	}

	memcpy (DrvZ80ROM + 0x8000, DrvZ80ROM, 0x4000);
}

static INT32 mspacmanInit()
{
	return DrvInit(MspacmanMap, MspacmanDecode, MSPACMAN);
}

struct BurnDriver BurnDrvmspacman = {
	"mspacman", NULL, NULL, NULL, "1980",
	"MS Pacman\0", NULL, "Namco", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, mspacmanRomInfo, mspacmanRomName, NULL, NULL, DrvInputInfo, mspacmanDIPInfo,
	mspacmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Ms. Pac-Man (with speedup hack)

static struct BurnRomInfo mspacmnfRomDesc[] = {
	{ "pacman.6e",    0x1000, 0xc1e6ab10, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "pacfast.6f",   0x1000, 0x720dc3ee, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "pacman.6h",    0x1000, 0xbcdd1beb, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "pacman.6j",    0x1000, 0x817d94e3, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "u5",           0x0800, 0xf45fbbcd, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "u6",           0x1000, 0xa90e7000, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "u7",           0x1000, 0xc82cd714, 1 | BRF_ESS | BRF_PRG },	//  6

	{ "5e",           0x1000, 0x5c281d01, 2 | BRF_GRA },		//  7 Graphics
	{ "5f",           0x1000, 0x615af909, 2 | BRF_GRA },		//  8

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  9 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 10 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 11 Timing Prom (not used)
};

STD_ROM_PICK(mspacmnf)
STD_ROM_FN(mspacmnf)

struct BurnDriver BurnDrvmspacmnf = {
	"mspacmnf", "mspacman", NULL, NULL, "1980",
	"Ms. Pac-Man (with speedup hack)\0", NULL, "Midway", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, mspacmnfRomInfo, mspacmnfRomName, NULL, NULL, DrvInputInfo, mspacmanDIPInfo,
	mspacmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Ms. Pac Attack

static struct BurnRomInfo mspacmatRomDesc[] = {
	{ "pacman.6e",    0x1000, 0xc1e6ab10, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "pacman.6f",    0x1000, 0x1a6fb2d4, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "pacman.6h",    0x1000, 0xbcdd1beb, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "pacman.6j",    0x1000, 0x817d94e3, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "u5",           0x0800, 0xf45fbbcd, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "u6pacatk",     0x1000, 0xf6d83f4d, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "u7",           0x1000, 0xc82cd714, 1 | BRF_ESS | BRF_PRG },	//  6

	{ "5e",           0x1000, 0x5c281d01, 2 | BRF_GRA },		//  7 Graphics
	{ "5f",           0x1000, 0x615af909, 2 | BRF_GRA },		//  8

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  9 Color Prom
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		// 10 

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 11
 	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 12 Sound Prom (not used)
};

STD_ROM_PICK(mspacmat)
STD_ROM_FN(mspacmat)

struct BurnDriver BurnDrvmspacmat = {
	"mspacmat", "mspacman", NULL, NULL, "1980",
	"Ms. Pac Attack\0", NULL, "hack", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, mspacmatRomInfo, mspacmatRomName, NULL, NULL, DrvInputInfo, mspacmanDIPInfo,
	mspacmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// MS Pacman (bootleg)

static struct BurnRomInfo mspacmabRomDesc[] = {
	{ "boot1",        0x1000, 0xd16b31b7, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "boot2",        0x1000, 0x0d32de5e, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "boot3",        0x1000, 0x1821ee0b, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "boot4",        0x1000, 0x165a9dd8, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "boot5",        0x1000, 0x8c3e6de6, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "boot6",        0x1000, 0x368cb165, 1 | BRF_ESS | BRF_PRG },	//  5

	{ "5e",           0x1000, 0x5c281d01, 2 | BRF_GRA },		//  6 Graphics
	{ "5f",           0x1000, 0x615af909, 2 | BRF_GRA },		//  7

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  8 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  9

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 10 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 11 Timing Prom (not used)
};

STD_ROM_PICK(mspacmab)
STD_ROM_FN(mspacmab)

static INT32 mspacmanbInit()
{
	return DrvInit(MspacmanMap, NULL, PACMAN);
}

struct BurnDriver BurnDrvmspacmab = {
	"mspacmab", "mspacman", NULL, NULL, "1981",
	"MS Pacman (bootleg)\0", NULL, "Namco", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, mspacmabRomInfo, mspacmabRomName, NULL, NULL, DrvInputInfo, mspacmanDIPInfo,
	mspacmanbInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Ms. Pac-Man (bootleg, (encrypted))

static struct BurnRomInfo mspacmbeRomDesc[] = {
	{ "boot1",        0x1000, 0xd16b31b7, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "2.bin",        0x1000, 0x04e6c486, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "boot3",        0x1000, 0x1821ee0b, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "boot4",        0x1000, 0x165a9dd8, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "boot5",        0x1000, 0x8c3e6de6, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "6.bin",        0x1000, 0x206a9623, 1 | BRF_ESS | BRF_PRG },	//  5

	{ "5e",           0x1000, 0x5c281d01, 2 | BRF_GRA },		//  6 Graphics
	{ "5f",           0x1000, 0x615af909, 2 | BRF_GRA },		//  7

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  8 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  9

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 10 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 11 Timing Prom (not used)
};

STD_ROM_PICK(mspacmbe)
STD_ROM_FN(mspacmbe)

static void mspacmbe_decode()
{
	for(INT32 i = 0x1000; i < 0x2000; i+=4)
	{
		if (!(i & 8))
		{
			INT32 t = DrvZ80ROM[i+1];
			DrvZ80ROM[i+1] = DrvZ80ROM[i+2];
			DrvZ80ROM[i+2] = t;
		};
	}
}

static INT32 mspacmbeInit()
{
	return DrvInit(MspacmanMap, mspacmbe_decode, PACMAN);
}

struct BurnDriver BurnDrvmspacmbe = {
	"mspacmbe", "mspacman", NULL, NULL, "1981",
	"Ms. Pac-Man (bootleg, (encrypted))\0", NULL, "bootleg", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, mspacmbeRomInfo, mspacmbeRomName, NULL, NULL, DrvInputInfo, mspacmanDIPInfo,
	mspacmbeInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Pac-Gal

static struct BurnRomInfo pacgalRomDesc[] = {
	{ "boot1",        0x1000, 0xd16b31b7, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "boot2",        0x1000, 0x0d32de5e, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "pacman.7fh",   0x1000, 0x513f4d5c, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "pacman.7hj",   0x1000, 0x70694c8e, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "boot5",        0x1000, 0x8c3e6de6, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "boot6",        0x1000, 0x368cb165, 1 | BRF_ESS | BRF_PRG },	//  5

	{ "5e",           0x1000, 0x5c281d01, 2 | BRF_GRA },		//  6 Graphics
	{ "pacman.5ef",   0x0800, 0x65a3ee71, 2 | BRF_GRA },		//  7
	{ "pacman.5hj",   0x0800, 0x50c7477d, 2 | BRF_GRA },		//  8

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  9 Color Proms
	{ "82s129.4a",    0x0100, 0x63efb927, 3 | BRF_GRA },		// 10

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 11 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 12 Timing Prom (not used)
};

STD_ROM_PICK(pacgal)
STD_ROM_FN(pacgal)

struct BurnDriver BurnDrvpacgal = {
	"pacgal", "mspacman", NULL, NULL, "1981",
	"Pac-Gal\0", NULL, "hack", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, pacgalRomInfo, pacgalRomName, NULL, NULL, DrvInputInfo, mspacmanDIPInfo,
	mspacmanbInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Ms. Pac-Man Plus

static struct BurnRomInfo mspacplsRomDesc[] = {
	{ "boot1",        0x1000, 0xd16b31b7, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "mspacatk.2",   0x1000, 0x0af09d31, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "boot3",        0x1000, 0x1821ee0b, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "boot4",        0x1000, 0x165a9dd8, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "mspacatk.5",   0x1000, 0xe6e06954, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "mspacatk.6",   0x1000, 0x3b5db308, 1 | BRF_ESS | BRF_PRG },	//  5

	{ "5e",           0x1000, 0x5c281d01, 2 | BRF_GRA },		//  6 Graphics
	{ "5f",           0x1000, 0x615af909, 2 | BRF_GRA },		//  7

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  8 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  9

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 10 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 11 Timing Prom (not used)
};

STD_ROM_PICK(mspacpls)
STD_ROM_FN(mspacpls)

struct BurnDriver BurnDrvmspacpls = {
	"mspacpls", "mspacman", NULL, NULL, "1981",
	"Ms. Pac-Man Plus\0", NULL, "hack", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, mspacplsRomInfo, mspacplsRomName, NULL, NULL, DrvInputInfo, mspacmanDIPInfo,
	mspacmanbInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Ms. Pacman Champion Edition / Zola-Puc Gal

static struct BurnRomInfo mschampRomDesc[] = {
	{ "9fg.bin",     0x10000, 0x04dba113, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code

	{ "8e.bin",       0x2000, 0x17435f53, 2 | BRF_GRA },		//  1 Graphics

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  2 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  3 

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  4 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  5 Timing Prom (not used)) )
};

STD_ROM_PICK(mschamp)
STD_ROM_FN(mschamp)

static void mschampCallback()
{
	UINT8 *tmp = (UINT8*)BurnMalloc(0x1000);

	memcpy (tmp + 0x0000, DrvGfxROM + 0x0800, 0x1000);
	memcpy (DrvGfxROM + 0x1000, tmp + 0x0000, 0x0800);
	memcpy (DrvGfxROM + 0x0800, tmp + 0x0800, 0x0800);

	BurnFree(tmp);
}

static INT32 mschampInit()
{
	return DrvInit(WoodpekMap, mschampCallback, MSCHAMP);
}

struct BurnDriver BurnDrvmschamp = {
	"mschamp", "mspacman", NULL, NULL, "1995",
	"Ms. Pacman Champion Edition / Zola-Puc Gal\0", NULL, "hack", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, mschampRomInfo, mschampRomName, NULL, NULL, mschampInputInfo, mschampDIPInfo,
	mschampInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Ms. Pacman Champion Edition / Super Zola Pac Gal

static struct BurnRomInfo mschampsRomDesc[] = {
	{ "pm4.bin",     0x10000, 0x7d6b6303, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code

	{ "pm5.bin",      0x2000, 0x7fe6b9e2, 2 | BRF_GRA },		//  1 Graphics

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  2 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  3 

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  4 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  5 Timing Prom (not used)) )
};

STD_ROM_PICK(mschamps)
STD_ROM_FN(mschamps)

struct BurnDriver BurnDrvmschamps = {
	"mschamps", "mspacman", NULL, NULL, "1995",
	"Ms. Pacman Champion Edition / Super Zola Pac Gal\0", NULL, "hack", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, mschampsRomInfo, mschampsRomName, NULL, NULL, mschampInputInfo, mschampDIPInfo,
	mschampInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Crush Roller (Kural Samno)

static struct BurnRomInfo crushRomDesc[] = {
	{ "crushkrl.6e",  0x1000, 0xa8dd8f54, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "crushkrl.6f",  0x1000, 0x91387299, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "crushkrl.6h",  0x1000, 0xd4455f27, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "crushkrl.6j",  0x1000, 0xd59fc251, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "maketrax.5e",  0x1000, 0x91bad2da, 2 | BRF_GRA },		//  4 Graphics
	{ "maketrax.5f",  0x1000, 0xaea79f55, 2 | BRF_GRA },		//  5

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  6 Color Prom
	{ "2s140.4a",     0x0100, 0x63efb927, 3 | BRF_GRA },		//  7

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(crush)
STD_ROM_FN(crush)

static void maketraxCallback()
{
	DrvZ80ROM[0x0224] = 0x00;
	DrvZ80ROM[0x0225] = 0x00;
	DrvZ80ROM[0x0226] = 0x00;
	DrvZ80ROM[0x022D] = 0x00;
	DrvZ80ROM[0x022F] = 0x00;
	DrvZ80ROM[0x023C] = 0x00;
	DrvZ80ROM[0x023D] = 0x00;
	DrvZ80ROM[0x023E] = 0x00;
	DrvZ80ROM[0x0415] = 0xc9;
	DrvZ80ROM[0x0428] = 0xC6;
	DrvZ80ROM[0x0429] = 0x12;
	DrvZ80ROM[0x115E] = 0xC9;
	DrvZ80ROM[0x1481] = 0x28;
	DrvZ80ROM[0x1492] = 0x20;
	DrvZ80ROM[0x14A5] = 0xC8;
	DrvZ80ROM[0x1978] = 0x18;
	DrvZ80ROM[0x1C9F] = 0xA7;
	DrvZ80ROM[0x1CA0] = 0xC9;
	DrvZ80ROM[0x238e] = 0xc9;
	DrvZ80ROM[0x3AE5] = 0x3E;
	DrvZ80ROM[0x3AE7] = 0xFE;
	DrvZ80ROM[0x3AE8] = 0x3F;
	DrvZ80ROM[0x3AE9] = 0xC9;
}

static INT32 crushInit()
{
	return DrvInit(StandardMap, maketraxCallback, PACMAN);
}

struct BurnDriver BurnDrvcrush = {
	"crush", NULL, NULL, NULL, "1981",
	"Crush Roller (Kural Samno)\0", NULL, "Kural Samno Electric", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, crushRomInfo, crushRomName, NULL, NULL, DrvInputInfo, maketraxDIPInfo,
	crushInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Crush Roller (bootleg)

static struct BurnRomInfo crushblRomDesc[] = {
	{ "cr1.bin",      0x1000, 0xe2e84cd1, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "cr2.bin",      0x1000, 0xec020e6f, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "cr3.bin",      0x1000, 0xd4455f27, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "cr4.bin",      0x1000, 0x9936ae06, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "maketrax.5e",  0x1000, 0x91bad2da, 2 | BRF_GRA },		//  4 Graphics
	{ "maketrax.5f",  0x1000, 0xaea79f55, 2 | BRF_GRA },		//  5

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  6 Color Prom
	{ "2s140.4a",     0x0100, 0x63efb927, 3 | BRF_GRA },		//  7

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(crushbl)
STD_ROM_FN(crushbl)

struct BurnDriver BurnDrvcrushbl = {
	"crushbl", "crush", NULL, NULL, "1981",
	"Crush Roller (bootleg)\0", NULL, "Kural Samno Electric", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, crushblRomInfo, crushblRomName, NULL, NULL, DrvInputInfo, maketraxDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Crush Roller (bootleg?)

static struct BurnRomInfo crushbl2RomDesc[] = {
	{ "cr5.7d",       0x1000, 0x4954d51d, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "cr6.7e",       0x1000, 0x27eb4299, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "cr7.7h",       0x1000, 0xd297108e, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "cr8.7j",       0x1000, 0xbcc40eaf, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "cr1.5e",       0x0800, 0xc7617198, 2 | BRF_GRA },		//  4 Graphics
	{ "cr3.5h",       0x0800, 0xc15b6967, 2 | BRF_GRA },		//  5
	{ "cr2.5f",       0x0800, 0xd5bc5cb8, 2 | BRF_GRA },		//  6 
	{ "cr4.5j",       0x0800, 0xd35d1caf, 2 | BRF_GRA },		//  7 

	{ "74s288.8a",    0x0020, 0xff344446, 3 | BRF_GRA },		//  8 Color Prom
	{ "2s140.4a",     0x0100, 0x63efb927, 3 | BRF_GRA },		//  9

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 10 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 11 Timing Prom (not used)
};

STD_ROM_PICK(crushbl2)
STD_ROM_FN(crushbl2)

struct BurnDriver BurnDrvcrushbl2 = {
	"crushbl2", "crush", NULL, NULL, "1981",
	"Crush Roller (bootleg?)\0", NULL, "bootleg", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, crushbl2RomInfo, crushbl2RomName, NULL, NULL, DrvInputInfo, maketraxDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Crush Roller (Kural Esco - bootleg?)

static struct BurnRomInfo crush2RomDesc[] = {
	{ "tp1",          0x0800, 0xf276592e, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "tp5a",         0x0800, 0x3d302abe, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "tp2",          0x0800, 0x25f42e70, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "tp6",          0x0800, 0x98279cbe, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "tp3",          0x0800, 0x8377b4cb, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "tp7",          0x0800, 0xd8e76c8c, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "tp4",          0x0800, 0x90b28fa3, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "tp8",          0x0800, 0x10854e1b, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "tpa",          0x0800, 0xc7617198, 2 | BRF_GRA },		//  8 Graphics
	{ "tpc",          0x0800, 0xe129d76a, 2 | BRF_GRA },		//  9
	{ "tpb",          0x0800, 0xd1899f05, 2 | BRF_GRA },		// 10 
	{ "tpd",          0x0800, 0xd35d1caf, 2 | BRF_GRA },		// 11 

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Proms
	{ "2s140.4a",     0x0100, 0x63efb927, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
};

STD_ROM_PICK(crush2)
STD_ROM_FN(crush2)

struct BurnDriver BurnDrvcrush2 = {
	"crush2", "crush", NULL, NULL, "1981",
	"Crush Roller (Kural Esco - bootleg?)\0", NULL, "Kural Esco Electric", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, crush2RomInfo, crush2RomName, NULL, NULL, DrvInputInfo, maketraxDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Crush Roller (Kural - bootleg?)

static struct BurnRomInfo crush3RomDesc[] = {
	{ "unkmol.4e",    0x0800, 0x49150ddf, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "unkmol.6e",    0x0800, 0x21f47e17, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "unkmol.4f",    0x0800, 0x9b6dd592, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "unkmol.6f",    0x0800, 0x755c1452, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "unkmol.4h",    0x0800, 0xed30a312, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "unkmol.6h",    0x0800, 0xfe4bb0eb, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "unkmol.4j",    0x0800, 0x072b91c9, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "unkmol.6j",    0x0800, 0x66fba07d, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "unkmol.5e",    0x0800, 0x338880a0, 2 | BRF_GRA },		//  8 Graphics
	{ "unkmol.5h",    0x0800, 0x4ce9c81f, 2 | BRF_GRA },		//  9
	{ "unkmol.5f",    0x0800, 0x752e3780, 2 | BRF_GRA },		// 10
	{ "unkmol.5j",    0x0800, 0x6e00d2ac, 2 | BRF_GRA },		// 11

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Proms
	{ "2s140.4a",     0x0100, 0x63efb927, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
};

STD_ROM_PICK(crush3)
STD_ROM_FN(crush3)

static void eyes_gfx_decode(UINT8 *src)
{
	UINT8 buf[8];

	for (INT32 i = 0; i < 8; i++) {
		buf[i] = BITSWAP08(src[((i & 1) << 2) | ((i & 4) >> 2) | (i & 2)],7,4,5,6,3,2,1,0);
	}

	memcpy (src, buf, 8);
}

static void eyes_decode()
{
	for (INT32 i = 0; i < 0x4000; i++)
		DrvZ80ROM[i] = BITSWAP08(DrvZ80ROM[i],7,6,3,4,5,2,1,0);

	for (INT32 i = 0;i < 0x2000; i += 8)
		eyes_gfx_decode(DrvGfxROM + i);
}

static INT32 crush3Init()
{
	return DrvInit(StandardMap, eyes_decode, PACMAN);
}

struct BurnDriver BurnDrvcrush3 = {
	"crush3", "crush", NULL, NULL, "1981",
	"Crush Roller (Kural - bootleg?)\0", NULL, "Kural Electric", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, crush3RomInfo, crush3RomName, NULL, NULL, DrvInputInfo, maketraxDIPInfo,
	crush3Init, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Crush Roller (Kural TWT)

static struct BurnRomInfo crush4RomDesc[] = {
	{ "crtwt.2",     0x10000, 0xadbd21f7, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code (banked)

	{ "crtwt.1",      0x4000, 0x4250a9ea, 2 | BRF_GRA },		//  1 Graphics

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  2 Color Proms
	{ "82s129.bin",   0x0100, 0x2bc5d339, 3 | BRF_GRA },		//  3

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  4 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  5 Timing Prom (not used)
};

STD_ROM_PICK(crush4)
STD_ROM_FN(crush4)

static void crush4Callback()
{
	UINT8 *tmp = (UINT8*)BurnMalloc(0x4000);

	memcpy (tmp + 0x0000, DrvGfxROM + 0x0000, 0x04000);
	memcpy (DrvGfxROM + 0x1000, tmp + 0x0800, 0x00800);
	memcpy (DrvGfxROM + 0x0800, tmp + 0x1000, 0x00800);
	memcpy (DrvGfxROM + 0x3000, tmp + 0x2800, 0x00800);
	memcpy (DrvGfxROM + 0x2800, tmp + 0x3000, 0x00800);

	memcpy (DrvZ80ROM + 0x00000, DrvZ80ROM + 0x08000, 0x04000);
	memcpy (DrvZ80ROM + 0x08000, DrvZ80ROM + 0x0c000, 0x04000);
	memset (DrvZ80ROM + 0x0c000, 0, 0x4000);

	BurnFree (tmp);
}

static INT32 crush4Init()
{
	return DrvInit(StandardMap, crush4Callback, PACMAN);
}

struct BurnDriver BurnDrvcrush4 = {
	"crush4", "crush", NULL, NULL, "1981",
	"Crush Roller (Kural TWT)\0", NULL, "Kural TWT", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, crush4RomInfo, crush4RomName, NULL, NULL, DrvInputInfo, maketraxDIPInfo,
	crush4Init, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Paint Roller

static struct BurnRomInfo paintrlrRomDesc[] = {
	{ "paintrlr.1",   0x0800, 0x556d20b5, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "paintrlr.5",   0x0800, 0x4598a965, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "paintrlr.2",   0x0800, 0x2da29c81, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "paintrlr.6",   0x0800, 0x1f561c54, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "paintrlr.3",   0x0800, 0xe695b785, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "paintrlr.7",   0x0800, 0x00e6eec0, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "paintrlr.4",   0x0800, 0x0fd5884b, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "paintrlr.8",   0x0800, 0x4900114a, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "tpa",          0x0800, 0xc7617198, 2 | BRF_GRA },		//  8 Graphics
	{ "mbrush.5h",    0x0800, 0xc15b6967, 2 | BRF_GRA },		//  9
	{ "mbrush.5f",    0x0800, 0xd5bc5cb8, 2 | BRF_GRA },		// 10
	{ "tpd",          0x0800, 0xd35d1caf, 2 | BRF_GRA },		// 11

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Proms
	{ "2s140.4a",     0x0100, 0x63efb927, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (unused)
};

STD_ROM_PICK(paintrlr)
STD_ROM_FN(paintrlr)

struct BurnDriver BurnDrvpaintrlr = {
	"paintrlr", "crush", NULL, NULL, "1981",
	"Paint Roller\0", NULL, "bootleg", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, paintrlrRomInfo, paintrlrRomName, NULL, NULL, DrvInputInfo, mbrushDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Crush Roller (Sidam bootleg)

static struct BurnRomInfo crushsRomDesc[] = {
	{ "11105-0.0j",   0x1000, 0xdd425429, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "11105-1.1j",   0x1000, 0xf9d89eef, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "11105-2.2j",   0x1000, 0x40c23a27, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "11105-3.3j",   0x1000, 0x5802644f, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "11105-4.4j",   0x1000, 0x91bad2da, 2 | BRF_GRA },		//  4 Graphics
	{ "11105-5.5j",   0x1000, 0xb5c14376, 2 | BRF_GRA },		//  5 

	{ "74s288.8a",    0x0020, 0xff344446, 3 | BRF_GRA },		//  6 Color Proms
	{ "24s10.6b",     0x0100, 0x63efb927, 3 | BRF_GRA },		//  7

	// uses AY8910
};

STD_ROM_PICK(crushs)
STD_ROM_FN(crushs)

static INT32 crushsInit()
{
	return DrvInit(StandardMap, NULL, CRUSHS);
}

struct BurnDriver BurnDrvcrushs = {
	"crushs", "crush", NULL, NULL, "19??",
	"Crush Roller (Sidam bootleg)\0", NULL, "[Kural] (Sidam bootleg)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, crushsRomInfo, crushsRomName, NULL, NULL, DrvInputInfo, crushsDIPInfo,
	crushsInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Make Trax (set 1)

static struct BurnRomInfo maketraxRomDesc[] = {
	{ "maketrax.6e",  0x1000, 0x0150fb4a, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "maketrax.6f",  0x1000, 0x77531691, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "maketrax.6h",  0x1000, 0xa2cdc51e, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "maketrax.6j",  0x1000, 0x0b4b5e0a, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "maketrax.5e",  0x1000, 0x91bad2da, 2 | BRF_GRA },		//  4 Graphics
	{ "maketrax.5f",  0x1000, 0xaea79f55, 2 | BRF_GRA },		//  5

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  6 Color Proms
	{ "2s140.4a",     0x0100, 0x63efb927, 3 | BRF_GRA },		//  7

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(maketrax)
STD_ROM_FN(maketrax)

static INT32 maketraxInit()
{
	return DrvInit(StandardMap, maketraxCallback, MAKETRAX);
}

struct BurnDriver BurnDrvmaketrax = {
	"maketrax", "crush", NULL, NULL, "1981",
	"Make Trax (set 1)\0", NULL, "[Kural] (Williams license)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, maketraxRomInfo, maketraxRomName, NULL, NULL, DrvInputInfo, maketraxDIPInfo,
	maketraxInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Make Trax (set 2)

static struct BurnRomInfo maketrxbRomDesc[] = {
	{ "maketrax.6e",  0x1000, 0x0150fb4a, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "maketrax.6f",  0x1000, 0x77531691, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "maketrxb.6h",  0x1000, 0x6ad342c9, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "maketrxb.6j",  0x1000, 0xbe27f729, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "maketrax.5e",  0x1000, 0x91bad2da, 2 | BRF_GRA },		//  4 Graphics
	{ "maketrax.5f",  0x1000, 0xaea79f55, 2 | BRF_GRA },		//  5

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  6 Color Proms
	{ "2s140.4a",     0x0100, 0x63efb927, 3 | BRF_GRA },		//  7

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(maketrxb)
STD_ROM_FN(maketrxb)

struct BurnDriver BurnDrvmaketrxb = {
	"maketrxb", "crush", NULL, NULL, "1981",
	"Make Trax (set 2)\0", NULL, "[Kural] (Williams license)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, maketrxbRomInfo, maketrxbRomName, NULL, NULL, DrvInputInfo, maketraxDIPInfo,
	maketraxInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Magic Brush

static struct BurnRomInfo mbrushRomDesc[] = {
	{ "mbrush.6e",    0x1000, 0x750fbff7, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "mbrush.6f",    0x1000, 0x27eb4299, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "mbrush.6h",    0x1000, 0xd297108e, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "mbrush.6j",    0x1000, 0x6fd719d0, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "tpa",          0x0800, 0xc7617198, 2 | BRF_GRA },		//  4 Graphics
	{ "mbrush.5h",    0x0800, 0xc15b6967, 2 | BRF_GRA },		//  5
	{ "mbrush.5f",    0x0800, 0xd5bc5cb8, 2 | BRF_GRA },		//  6
	{ "tpd",          0x0800, 0xd35d1caf, 2 | BRF_GRA },		//  7

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  8 Color Proms
	{ "2s140.4a",     0x0100, 0x63efb927, 3 | BRF_GRA },		//  9

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 10 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 11 Timing Prom (not used)
};

STD_ROM_PICK(mbrush)
STD_ROM_FN(mbrush)

static void mbrushCallback()
{
	DrvZ80ROM[0x3AE2] = 0x3E;
	DrvZ80ROM[0x3AE3] = 0x00;
	DrvZ80ROM[0x3AE4] = 0x00;
}

static INT32 mbrushInit()
{
	return DrvInit(StandardMap, mbrushCallback, PACMAN);
}

struct BurnDriver BurnDrvmbrush = {
	"mbrush", "crush", NULL, NULL, "1981",
	"Magic Brush\0", NULL, "bootleg", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, mbrushRomInfo, mbrushRomName, NULL, NULL, DrvInputInfo, mbrushDIPInfo,
	mbrushInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Korosuke Roller

static struct BurnRomInfo korosukeRomDesc[] = {
	{ "kr.6e",        0x1000, 0x69f6e2da, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "kr.6f",        0x1000, 0xabf34d23, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "kr.6h",        0x1000, 0x76a2e2e2, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "kr.6j",        0x1000, 0x33e0e3bb, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "kr.5e",        0x1000, 0xe0380be8, 2 | BRF_GRA },		//  4 Graphics
	{ "kr.5f",        0x1000, 0x63fec9ee, 2 | BRF_GRA },		//  5

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  6 Color Proms
	{ "2s140.4a",     0x0100, 0x63efb927, 3 | BRF_GRA },		//  7

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(korosuke)
STD_ROM_FN(korosuke)

static void korosukeCallback()
{
	DrvZ80ROM[0x0233] = 0x00;
	DrvZ80ROM[0x0234] = 0x00;
	DrvZ80ROM[0x0235] = 0x00;
	DrvZ80ROM[0x023c] = 0x00;
	DrvZ80ROM[0x023e] = 0x00;
	DrvZ80ROM[0x024b] = 0x00;
	DrvZ80ROM[0x024c] = 0x00;
	DrvZ80ROM[0x024d] = 0x00;
	DrvZ80ROM[0x044c] = 0xc9;
	DrvZ80ROM[0x115B] = 0xC9;
	DrvZ80ROM[0x1AE9] = 0x18;
	DrvZ80ROM[0x1CA7] = 0xA7;
	DrvZ80ROM[0x1CA8] = 0xC9;
	DrvZ80ROM[0x238c] = 0xc9;
	DrvZ80ROM[0x3AE9] = 0x3E;
	DrvZ80ROM[0x3AEB] = 0xFE;
	DrvZ80ROM[0x3AEC] = 0x3F;
	DrvZ80ROM[0x3AEE] = 0xC9;
	DrvZ80ROM[0x045d] = 0xC6;
	DrvZ80ROM[0x045e] = 0x12;
	DrvZ80ROM[0x1481] = 0x28;
	DrvZ80ROM[0x1492] = 0x20;
	DrvZ80ROM[0x14A5] = 0xC8;
}

static INT32 korosukeInit()
{
	return DrvInit(StandardMap, korosukeCallback, MAKETRAX);
}

struct BurnDriver BurnDrvkorosuke = {
	"korosuke", "crush", NULL, NULL, "1981",
	"Korosuke Roller\0", NULL, "Kural Electric", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, korosukeRomInfo, korosukeRomName, NULL, NULL, korosukeInputInfo, korosukeDIPInfo,
	korosukeInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Eyes (Digitrex Techstar)

static struct BurnRomInfo eyesRomDesc[] = {
	{ "d7",           0x1000, 0x3b09ac89, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "e7",           0x1000, 0x97096855, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "f7",           0x1000, 0x731e294e, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "h7",           0x1000, 0x22f7a719, 1 | BRF_ESS | BRF_PRG },	//  4
 
	{ "d5",           0x1000, 0xd6af0030, 2 | BRF_GRA },		//  5 Graphics
	{ "e5",           0x1000, 0xa42b5201, 2 | BRF_GRA },		//  6

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  7 Color Proms
	{ "82s129.4a",    0x0100, 0xd8d78829, 3 | BRF_GRA },		//  8

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  9 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 10 Timing Prom (not used)
};

STD_ROM_PICK(eyes)
STD_ROM_FN(eyes)

static INT32 eyesInit()
{
	return DrvInit(StandardMap, eyes_decode, PACMAN);
}

struct BurnDriver BurnDrveyes = {
	"eyes", NULL, NULL, NULL, "1982",
	"Eyes (Digitrex Techstar)\0", NULL, "Digitrex Techstar (Rock-ola license)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, eyesRomInfo, eyesRomName, NULL, NULL, eyesInputInfo, eyesDIPInfo,
	eyesInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Eyes (Techstar)

static struct BurnRomInfo eyes2RomDesc[] = {
	{ "g38201.7d",    0x1000, 0x2cda7185, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "g38202.7e",    0x1000, 0xb9fe4f59, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "g38203.7f",    0x1000, 0xd618ba66, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "g38204.7h",    0x1000, 0xcf038276, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "g38205.5d",    0x1000, 0x03b1b4c7, 2 | BRF_GRA },		//  4 Graphics
	{ "g38206.5e",    0x1000, 0xa42b5201, 2 | BRF_GRA },		//  5

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  6 Color Proms
	{ "82s129.4a",    0x0100, 0xd8d78829, 3 | BRF_GRA },		//  7

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(eyes2)
STD_ROM_FN(eyes2)

struct BurnDriver BurnDrveyes2 = {
	"eyes2", "eyes", NULL, NULL, "1982",
	"Eyes (Techstar)\0", NULL, "Techstar (Rock-ola license)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, eyes2RomInfo, eyes2RomName, NULL, NULL, eyesInputInfo, eyesDIPInfo,
	eyesInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Eyes (bootleg set 1)

static struct BurnRomInfo eyesbRomDesc[] = {
	{ "1.bin",	  0x0800, 0x339d279a, 1 | BRF_ESS | BRF_PRG },  //  0 Z80 Code
	{ "5.bin",	  0x0800, 0x1b68a61d, 1 | BRF_ESS | BRF_PRG },  //  1
	{ "2.bin",	  0x0800, 0xd4f9aaf8, 1 | BRF_ESS | BRF_PRG },  //  2
	{ "6.bin",	  0x0800, 0x6b41bb80, 1 | BRF_ESS | BRF_PRG },  //  3
	{ "3.bin",	  0x0800, 0x748e0e48, 1 | BRF_ESS | BRF_PRG },  //  4
	{ "7.bin",	  0x0800, 0x7b7f4a74, 1 | BRF_ESS | BRF_PRG },  //  5
	{ "4.bin",	  0x0800, 0x367a3884, 1 | BRF_ESS | BRF_PRG },  //  6
	{ "8.bin",	  0x0800, 0x2baaadae, 1 | BRF_ESS | BRF_PRG },  //  7

	{ "9.bin",	  0x0800, 0x342c0653, 2 | BRF_GRA },		//  8 Graphics
	{ "11.bin",	  0x0800, 0xaaa7a537, 2 | BRF_GRA },		//  9
	{ "10.bin",	  0x0800, 0xb247b82c, 2 | BRF_GRA },		// 10
	{ "12.bin",	  0x0800, 0x99af4b30, 2 | BRF_GRA },		// 11

	{ "7051.bin",	  0x0020, 0x0dad2ccb, 3 | BRF_GRA },		// 12 Color Proms
	{ "7051-3.bin",	  0x0100, 0xd8d78829, 3 | BRF_GRA },		// 13

	{ "82s126.1m",	  0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "7051-2.bin",	  0x0100, 0x77245b66, 4 | BRF_OPT },     	// 15 Timing Prom (not used)
};

STD_ROM_PICK(eyesb)
STD_ROM_FN(eyesb)

struct BurnDriver BurnDrveyesb = {
	"eyesb", "eyes", NULL, NULL, "1982",
	"Eyes (bootleg set 1)\0", NULL, "bootleg", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, eyesbRomInfo, eyesbRomName, NULL, NULL, eyesInputInfo, eyesDIPInfo,
	eyesInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Eyes (bootleg set 2, decrypted)

static struct BurnRomInfo eyeszacbRomDesc[] = {
	{ "zacb_1.bin",   0x0800, 0xa4a9d7a0, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "zacb_5.bin",   0x0800, 0xc32b3f73, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "zacb_2.bin",   0x0800, 0x195b9473, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "zacb_6.bin",   0x0800, 0x292886cb, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "zacb_3.bin",   0x0800, 0xff94b015, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "zacb_7.bin",   0x0800, 0x9271c58c, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "zacb_4.bin",   0x0800, 0x965cf32b, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "zacb_8.bin",   0x0800, 0xc254e92e, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "x.bin",        0x0800, 0x59dce22e, 2 | BRF_GRA },		//  8 Graphics
	{ "c.bin",        0x0800, 0xaaa7a537, 2 | BRF_GRA },		//  9
	{ "b.bin",        0x0800, 0x1969792b, 2 | BRF_GRA },		// 10
	{ "p.bin",        0x0800, 0x99af4b30, 2 | BRF_GRA },		// 11

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Proms
	{ "82s129.4a",    0x0100, 0xd8d78829, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
	
	{ "zacb_11.bin",  0x0800, 0x69c1602a, 0 | BRF_PRG },	//  16 (Not used)
};

STD_ROM_PICK(eyeszacb)
STD_ROM_FN(eyeszacb)

struct BurnDriver BurnDrveyeszac = {
	"eyeszacb", "eyes", NULL, NULL, "1982",
	"Eyes (bootleg set 2, decrypted)\0", NULL, "bootleg", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, eyeszacbRomInfo, eyeszacbRomName, NULL, NULL, eyesInputInfo, eyesDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Mr. TNT

static struct BurnRomInfo mrtntRomDesc[] = {
	{ "tnt.1",        0x1000, 0x0e836586, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "tnt.2",        0x1000, 0x779c4c5b, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "tnt.3",        0x1000, 0xad6fc688, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "tnt.4",        0x1000, 0xd77557b3, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "tnt.5",        0x1000, 0x3038cc0e, 2 | BRF_GRA },		//  4 Graphics
	{ "tnt.6",        0x1000, 0x97634d8b, 2 | BRF_GRA },		//  5

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  6 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  7 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  8 Timing Prom (not used)
};

STD_ROM_PICK(mrtnt)
STD_ROM_FN(mrtnt)

struct BurnDriver BurnDrvmrtnt = {
	"mrtnt", NULL, NULL, NULL, "1982",
	"Mr. TNT\0", NULL, "Telko", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, mrtntRomInfo, mrtntRomName, NULL, NULL, eyesInputInfo, mrtntDIPInfo,
	eyesInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Gorkans

static struct BurnRomInfo gorkansRomDesc[] = {
	{ "gorkans8.rom", 0x0800, 0x55100b18, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "gorkans4.rom", 0x0800, 0xb5c604bf, 1 | BRF_ESS | BRF_PRG },	//  1
 	{ "gorkans7.rom", 0x0800, 0xb8c6def4, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "gorkans3.rom", 0x0800, 0x4602c840, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "gorkans6.rom", 0x0800, 0x21412a62, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "gorkans2.rom", 0x0800, 0xa013310b, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "gorkans5.rom", 0x0800, 0x122969b2, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "gorkans1.rom", 0x0800, 0xf2524b11, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "gorkgfx4.rom", 0x0800, 0x39cd0dbc, 2 | BRF_GRA },		//  8 Graphics
	{ "gorkgfx2.rom", 0x0800, 0x33d52535, 2 | BRF_GRA },		//  9
	{ "gorkgfx3.rom", 0x0800, 0x4b6b7970, 2 | BRF_GRA },		// 10
	{ "gorkgfx1.rom", 0x0800, 0xed70bb3c, 2 | BRF_GRA },		// 11

	{ "gorkprom.4",   0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Proms
	{ "gorkprom.1",   0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		// 13

	{ "gorkprom.3",   0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "gorkprom.2",   0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
};

STD_ROM_PICK(gorkans)
STD_ROM_FN(gorkans)

struct BurnDriver BurnDrvgorkans = {
	"gorkans", "mrtnt", NULL, NULL, "1983",
	"Gorkans\0", NULL, "Techstar", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, gorkansRomInfo, gorkansRomName, NULL, NULL, eyesInputInfo, mrtntDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Eggor

static struct BurnRomInfo eggorRomDesc[] = {
	{ "1.bin",        0x0800, 0x818ed154, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "5.bin",        0x0800, 0xa4b21d93, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "2.bin",        0x0800, 0x5d7a23ed, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "6.bin",        0x0800, 0xe9dbca8d, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "3.bin",        0x0800, 0x4318ab85, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "7.bin",        0x0800, 0x03214d7f, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "4.bin",        0x0800, 0xdc805be4, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "8.bin",        0x0800, 0xf9ae204b, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "9.bin",        0x0800, 0x96ad8626, 2 | BRF_GRA },		//  8 Graphics
	{ "11.bin",       0x0800, 0xcc324017, 2 | BRF_GRA },		//  9
	{ "10.bin",       0x0800, 0x7c97f513, 2 | BRF_GRA },		// 10
	{ "12.bin",       0x0800, 0x2e930602, 2 | BRF_GRA },		// 11

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
};

STD_ROM_PICK(eggor)
STD_ROM_FN(eggor)

struct BurnDriver BurnDrveggor = {
	"eggor", NULL, NULL, NULL, "1982",
	"Eggor\0", NULL, "Telko", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, eggorRomInfo, eggorRomName, NULL, NULL, eyesInputInfo, mrtntDIPInfo,
	eyesInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};



// Piranha

static struct BurnRomInfo piranhaRomDesc[] = {
	{ "pir1.bin",     0x0800, 0x69a3e6ea, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "pir5.bin",     0x0800, 0x245e753f, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "pir2.bin",     0x0800, 0x62cb6954, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "pir6.bin",     0x0800, 0xcb0700bc, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "pir3.bin",     0x0800, 0x843fbfe5, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "pir7.bin",     0x0800, 0x73084d5e, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "pir4.bin",     0x0800, 0x4cdf6704, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "pir8.bin",     0x0800, 0xb86fedb3, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "pir9.bin",     0x0800, 0x0f19eb28, 2 | BRF_GRA },		//  8 Graphics
	{ "pir11.bin",    0x0800, 0x5f8bdabe, 2 | BRF_GRA },		//  9
	{ "pir10.bin",    0x0800, 0xd19399fb, 2 | BRF_GRA },		// 10
	{ "pir12.bin",    0x0800, 0xcfb4403d, 2 | BRF_GRA },		// 11

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Prom
	{ "piranha.4a",   0x0100, 0x08c9447b, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
};

STD_ROM_PICK(piranha)
STD_ROM_FN(piranha)

static INT32 piranhaInit()
{
	return DrvInit(StandardMap, eyes_decode, PIRANHA);
}

struct BurnDriver BurnDrvpiranha = {
	"piranha", "puckman", NULL, NULL, "1981",
	"Piranha\0", NULL, "GL (US Billiards License)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, piranhaRomInfo, piranhaRomName, NULL, NULL, DrvInputInfo, mspacmanDIPInfo,
	piranhaInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Piranha (older)

static struct BurnRomInfo piranhaoRomDesc[] = {
	{ "p1.bin",       0x0800, 0xc6ce1bfc, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "p5.bin",       0x0800, 0xa2655a33, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "pir2.bin",     0x0800, 0x62cb6954, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "pir6.bin",     0x0800, 0xcb0700bc, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "pir3.bin",     0x0800, 0x843fbfe5, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "pir7.bin",     0x0800, 0x73084d5e, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "p4.bin",       0x0800, 0x9363a4d1, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "p8.bin",       0x0800, 0x2769979c, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "p9.bin",       0x0800, 0x94eb7563, 2 | BRF_GRA },		//  8 Graphics
	{ "p11.bin",      0x0800, 0xa3606973, 2 | BRF_GRA },		//  9
	{ "p10.bin",      0x0800, 0x84165a2c, 2 | BRF_GRA },		// 10
	{ "p12.bin",      0x0800, 0x2699ba9e, 2 | BRF_GRA },		// 11

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Proms
	{ "piranha.4a",   0x0100, 0x08c9447b, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
};

STD_ROM_PICK(piranhao)
STD_ROM_FN(piranhao)

struct BurnDriver BurnDrvpiranhao = {
	"piranhao", "puckman", NULL, NULL, "1981",
	"Piranha (older)\0", NULL, "GL (US Billiards License)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, piranhaoRomInfo, piranhaoRomName, NULL, NULL, DrvInputInfo, mspacmanDIPInfo,
	piranhaInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Piranha (hack)

static struct BurnRomInfo piranhahRomDesc[] = {
	{ "pr1.cpu",      0x1000, 0xbc5ad024, 1 | BRF_ESS | BRF_PRG },  //  0 Z80 Code
	{ "pacman.6f",    0x1000, 0x1a6fb2d4, 1 | BRF_ESS | BRF_PRG },  //  1
	{ "pr3.cpu",      0x1000, 0x473c379d, 1 | BRF_ESS | BRF_PRG },  //  2
	{ "pr4.cpu",      0x1000, 0x63fbf895, 1 | BRF_ESS | BRF_PRG },  //  3

	{ "pr5.cpu",      0x0800, 0x3fc4030c, 2 | BRF_GRA },		//  4 Graphics
	{ "pr7.cpu",      0x0800, 0x30b9a010, 2 | BRF_GRA },		//  5
	{ "pr6.cpu",      0x0800, 0xf3e9c9d5, 2 | BRF_GRA },		//  6
	{ "pr8.cpu",      0x0800, 0x133d720d, 2 | BRF_GRA },		//  7

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  8 Color Prom
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  9

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 10 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 11 Timing Prom (not used)
};

STD_ROM_PICK(piranhah)
STD_ROM_FN(piranhah)

struct BurnDriver BurnDrvpiranhah = {
	"piranhah", "puckman", NULL, NULL, "1981",
	"Piranha (hack)\0", NULL, "hack", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, piranhahRomInfo, piranhahRomName, NULL, NULL, DrvInputInfo, mspacmanDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Abscam

static struct BurnRomInfo abscamRomDesc[] = {
	{ "as0.bin",      0x0800, 0x0b102302, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "as4.bin",      0x0800, 0x3116a8ec, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "as1.bin",      0x0800, 0xbc0281e0, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "as5.bin",      0x0800, 0x428ee2e8, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "as2.bin",      0x0800, 0xe05d46ad, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "as6.bin",      0x0800, 0x3ae9a8cb, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "as3.bin",      0x0800, 0xb39eb940, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "as7.bin",      0x0800, 0x16cf1c67, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "as8.bin",      0x0800, 0x61daabe5, 2 | BRF_GRA },		//  8 Graphics
	{ "as10.bin",     0x0800, 0x81d50c98, 2 | BRF_GRA },		//  9
	{ "as9.bin",      0x0800, 0xa3bd1613, 2 | BRF_GRA },		// 10
	{ "as11.bin",     0x0800, 0x9d802b68, 2 | BRF_GRA },		// 11

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Proms
	{ "as4a.bin",     0x0100, 0x1605b324, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
};

STD_ROM_PICK(abscam)
STD_ROM_FN(abscam)

struct BurnDriver BurnDrvabscam = {
	"abscam", "puckman", NULL, NULL, "1981",
	"Abscam\0", NULL, "GL (US Billiards License)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, abscamRomInfo, abscamRomName, NULL, NULL, DrvInputInfo, mspacmanDIPInfo,
	piranhaInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Jump Shot

static struct BurnRomInfo jumpshotRomDesc[] = {
	{ "6e",           0x1000, 0xf00def9a, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "6f",           0x1000, 0xf70deae2, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "6h",           0x1000, 0x894d6f68, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "6j",           0x1000, 0xf15a108a, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "5e",           0x1000, 0xd9fa90f5, 2 | BRF_GRA },		//  4 Graphics
	{ "5f",           0x1000, 0x2ec711c1, 2 | BRF_GRA },		//  5

	{ "prom.7f",      0x0020, 0x872b42f3, 3 | BRF_GRA },		//  6 Color Prom
	{ "prom.4a",      0x0100, 0x0399f39f, 3 | BRF_GRA },		//  7

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(jumpshot)
STD_ROM_FN(jumpshot)

static UINT8 jumpshot_decrypt(INT32 addr, UINT8 e)
{
	static const UINT8 swap_xor_table[6][9] = {
		{ 7,6,5,4,3,2,1,0, 0x00 },
		{ 7,6,3,4,5,2,1,0, 0x20 },
		{ 5,0,4,3,7,1,2,6, 0xa4 },
		{ 5,0,4,3,7,1,2,6, 0x8c },
		{ 2,3,1,7,4,6,0,5, 0x6e },
		{ 2,3,4,7,1,6,0,5, 0x4e }
	};

	static const INT32 picktable[32] = {
		0,2,4,4,4,2,0,2,2,0,2,4,4,2,0,2,
		5,3,5,1,5,3,5,3,1,5,1,5,5,3,5,3
	};

	INT32 method = picktable[(addr & 0x001) | ((addr & 0x004) >> 1) | ((addr & 0x020) >> 3) | ((addr & 0x080) >> 4) |((addr & 0x200) >> 5)];

	if ((addr & 0x800) == 0x800)
		method ^= 1;

	const UINT8 *tbl = swap_xor_table[method];
	return BITSWAP08(e,tbl[0],tbl[1],tbl[2],tbl[3],tbl[4],tbl[5],tbl[6],tbl[7]) ^ tbl[8];
}

static void jumpshot_decode()
{
	for (INT32 i = 0; i < 0x4000; i++) {
		DrvZ80ROM[i] = jumpshot_decrypt(i, DrvZ80ROM[i]);
	}
}

static INT32 jumpshotInit()
{
	return DrvInit(StandardMap, jumpshot_decode, PACMAN);
}

struct BurnDriver BurnDrvjumpshot = {
	"jumpshot", NULL, NULL, NULL, "1985",
	"Jump Shot\0", NULL, "Bally Midway", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_SPORTSMISC, 0,
	NULL, jumpshotRomInfo, jumpshotRomName, NULL, NULL, jumpshotInputInfo, jumpshotDIPInfo,
	jumpshotInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Jump Shot Engineering Sample

static struct BurnRomInfo jumpshotpRomDesc[] = {
	{ "js6e.bin",     0x1000, 0xacc5e15e, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "js6f.bin",     0x1000, 0x62b48ba4, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "js6h.bin",     0x1000, 0x7c9b5e30, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "js6j.bin",     0x1000, 0x9f0c39f6, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "5e",           0x1000, 0xd9fa90f5, 2 | BRF_GRA },		//  4 Graphics
	{ "5f",           0x1000, 0x2ec711c1, 2 | BRF_GRA },		//  5

	{ "prom.7f",      0x0020, 0x872b42f3, 3 | BRF_GRA },		//  6 Color Proms
	{ "prom.4a",      0x0100, 0x0399f39f, 3 | BRF_GRA },		//  7

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(jumpshotp)
STD_ROM_FN(jumpshotp)

struct BurnDriver BurnDrvjumpshotp = {
	"jumpshotp", "jumpshot", NULL, NULL, "1985",
	"Jump Shot Engineering Sample\0", NULL, "Bally Midway", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_SPORTSMISC, 0,
	NULL, jumpshotpRomInfo, jumpshotpRomName, NULL, NULL, jumpshotInputInfo, jumpshtpDIPInfo,
	jumpshotInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Shoot the Bull

static struct BurnRomInfo shootbulRomDesc[] = {
	{ "sb6e.cpu",     0x1000, 0x25daa5e9, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "sb6f.cpu",     0x1000, 0x92144044, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "sb6h.cpu",     0x1000, 0x43b7f99d, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "sb6j.cpu",     0x1000, 0xbc4d3bbf, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "sb5e.cpu",     0x1000, 0x07c6c5aa, 2 | BRF_GRA },		//  4 Graphics
	{ "sb5f.cpu",     0x1000, 0xeaec6837, 2 | BRF_GRA },		//  5

	{ "7f.rom",       0x0020, 0xec578b98, 3 | BRF_GRA },		//  6 Color Proms
	{ "4a.rom",       0x0100, 0x81a6b30f, 3 | BRF_GRA },		//  7

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(shootbul)
STD_ROM_FN(shootbul)

static INT32 shootbulInit()
{
	return DrvInit(StandardMap, jumpshot_decode, SHOOTBUL);
}

struct BurnDriverD BurnDrvshootbul = {
	"shootbul", NULL, NULL, NULL, "1985",
	"Shoot the Bull\0", "Analog inputs not hooked up", "Bally Midway", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_SPORTSMISC, 0,
	NULL, shootbulRomInfo, shootbulRomName, NULL, NULL, shootbulInputInfo, shootbulDIPInfo,
	shootbulInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Cannon Ball (Pacman Hardware)

static struct BurnRomInfo cannonbpRomDesc[] = {
	{ "n1-6e",        0x0800, 0xc68878c7, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "n2-6k",        0x0800, 0xff3951a5, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "n3-6f",        0x0800, 0x2329079d, 1 | BRF_ESS | BRF_PRG },	//  2
 	{ "n4-6m",        0x0800, 0xfcc57ecb, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "n5-6h",        0x0800, 0x52846c9d, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "n6-6n",        0x0800, 0x59e890dd, 1 | BRF_ESS | BRF_PRG },	//  5

	{ "z1-5e",        0x0800, 0x125779e0, 2 | BRF_GRA },		//  6 Graphics
	{ "z3-5h",        0x0800, 0x78f866c0, 2 | BRF_GRA },		//  7
	{ "z2-5f",        0x0800, 0xfbd2c99d, 2 | BRF_GRA },		//  8
	{ "z4-5j",        0x0800, 0x8734c904, 2 | BRF_GRA },		//  9

	{ "colorprom_1",  0x0020, 0x08f8ae7e, 3 | BRF_GRA },		// 10 Color Proms
	{ "colorprom_2",  0x0100, 0x359a15dc, 3 | BRF_GRA },		// 11

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 12 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 13 Timing Prom (not used)
};

STD_ROM_PICK(cannonbp)
STD_ROM_FN(cannonbp)

static INT32 cannonbpInit()
{
	return DrvInit(CannonbpMap, NULL, CANNONBP);
}

struct BurnDriver BurnDrvcannonbp = {
	"cannonbp", NULL, NULL, NULL, "198?",
	"Cannon Ball (Pacman Hardware)\0", "wrong colors", "Novomatic", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_BREAKOUT, 0,
	NULL, cannonbpRomInfo, cannonbpRomName, NULL, NULL, cannonbpInputInfo, cannonbpDIPInfo,
	cannonbpInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Woodpecker (set 1)

static struct BurnRomInfo woodpeckRomDesc[] = {
	{ "f.bin", 0x1000, 0x37ea66ca, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "i.bin", 0x1000, 0xcd115dba, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "e.bin", 0x1000, 0xd40b2321, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "g.bin", 0x1000, 0x024092f4, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "h.bin", 0x1000, 0x18ef0fc8, 1 | BRF_ESS | BRF_PRG },	//  4

	{ "a.5e",  0x0800, 0x15a87f62, 2 | BRF_GRA },		//  5 Graphics
	{ "c.5h",  0x0800, 0xab4abd88, 2 | BRF_GRA },		//  6
	{ "b.5f",  0x0800, 0x5b9ba95b, 2 | BRF_GRA },		//  7
	{ "d.5j",  0x0800, 0xd7b80a45, 2 | BRF_GRA },		//  8

	{ "pr.8h", 0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  9 Color Prom
	{ "pr.4a", 0x0100, 0xd8772167, 3 | BRF_GRA },		// 10

	{ "pr.1k", 0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 11 Sound Prom
	{ "pr.3k", 0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 12 Timing Prom (not used)
};

STD_ROM_PICK(woodpeck)
STD_ROM_FN(woodpeck)

static void woodpeckCallback()
{
	memcpy (DrvZ80ROM + 0xb000, DrvZ80ROM + 0x8000, 0x1000);
	memcpy (DrvZ80ROM + 0x8000, DrvZ80ROM + 0x1000, 0x3000);

	memset (DrvZ80ROM + 0x1000, 0, 0x3000);

	for (INT32 i = 0; i < 0x2000; i += 8)
		eyes_gfx_decode(DrvGfxROM + i);
}

static INT32 woodpeckInit()
{
	return DrvInit(WoodpekMap, woodpeckCallback, PACMAN);
}

struct BurnDriver BurnDrvwoodpeck = {
	"woodpeck", NULL, NULL, NULL, "1981",
	"Woodpecker (set 1)\0", NULL, "Amenip (Palcom Queen River)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_PLATFORM, 0,
	NULL, woodpeckRomInfo, woodpeckRomName, NULL, NULL, woodpekInputInfo, woodpekDIPInfo,
	woodpeckInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Woodpecker (set 2)

static struct BurnRomInfo woodpekaRomDesc[] = {
	{ "0",     0x1000, 0xb5ee8bca, 1 | BRF_PRG | BRF_ESS },	//  0 Z80 Code
	{ "1",     0x1000, 0xc5ec2de6, 1 | BRF_PRG | BRF_ESS },	//  1
	{ "2",     0x1000, 0x07ea534e, 1 | BRF_PRG | BRF_ESS },	//  2
	{ "3",     0x1000, 0xa3a3253a, 1 | BRF_PRG | BRF_ESS },	//  3
	{ "4",     0x1000, 0x6c50546b, 1 | BRF_PRG | BRF_ESS },	//  4

	{ "10.5f", 0x1000, 0x0bf52102, 2 | BRF_GRA },		//  5 Graphics
	{ "11.5h", 0x1000, 0x0ed8def8, 2 | BRF_GRA },		//  6

	{ "pr.8h", 0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  7 Color Proms
	{ "pr.4a", 0x0100, 0xd8772167, 3 | BRF_GRA },		//  8

	{ "pr.1k", 0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  9
	{ "pr.3k", 0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 10 Timing Prom (not used)
};

STD_ROM_PICK(woodpeka)
STD_ROM_FN(woodpeka)

struct BurnDriver BurnDrvwoodpeka = {
	"woodpeca", "woodpeck", NULL, NULL, "1981",
	"Woodpecker (set 2)\0", NULL, "Amenip (Palcom Queen River)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_PLATFORM, 0,
	NULL, woodpekaRomInfo, woodpekaRomName, NULL, NULL, woodpekInputInfo, woodpekDIPInfo,
	woodpeckInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Lizard Wizard

static struct BurnRomInfo lizwizRomDesc[] = {
	{ "6e.cpu",       0x1000, 0x32bc1990, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "6f.cpu",       0x1000, 0xef24b414, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "6h.cpu",       0x1000, 0x30bed83d, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "6j.cpu",       0x1000, 0xdd09baeb, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "wiza",         0x1000, 0xf6dea3a6, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "wizb",         0x1000, 0xf27fb5a8, 1 | BRF_ESS | BRF_PRG },	//  5

	{ "5e.cpu",       0x1000, 0x45059e73, 2 | BRF_GRA },		//  6 Graphics
	{ "5f.cpu",       0x1000, 0xd2469717, 2 | BRF_GRA },		//  7

	{ "7f.cpu",       0x0020, 0x7549a947, 3 | BRF_GRA },		//  8 Color Proms
	{ "4a.cpu",       0x0100, 0x5fdca536, 3 | BRF_GRA },		//  9

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 10 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 11 Timing Prom (not used)
};

STD_ROM_PICK(lizwiz)
STD_ROM_FN(lizwiz)

static INT32 lizwizInit()
{
	return DrvInit(WoodpekMap, NULL, PACMAN);
}

struct BurnDriver BurnDrvlizwiz = {
	"lizwiz", NULL, NULL, NULL, "1985",
	"Lizard Wizard\0", NULL, "Techstar (Sunn license)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_SHOOT, 0,
	NULL, lizwizRomInfo, lizwizRomName, NULL, NULL, lizwizInputInfo, lizwizDIPInfo,
	lizwizInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Ponpoko

static struct BurnRomInfo ponpokoRomDesc[] = {
	{ "ppokoj1.bin",  0x1000, 0xffa3c004, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "ppokoj2.bin",  0x1000, 0x4a496866, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "ppokoj3.bin",  0x1000, 0x17da6ca3, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "ppokoj4.bin",  0x1000, 0x9d39a565, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "ppoko5.bin",   0x1000, 0x54ca3d7d, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "ppoko6.bin",   0x1000, 0x3055c7e0, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "ppoko7.bin",   0x1000, 0x3cbe47ca, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "ppokoj8.bin",  0x1000, 0x04b63fc6, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "ppoko9.bin",   0x1000, 0xb73e1a06, 2 | BRF_GRA },		//  8 Graphics
	{ "ppoko10.bin",  0x1000, 0x62069b5d, 2 | BRF_GRA },		//  9

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 10 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		// 11

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 12 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 13 Timing Prom (not used)
};

STD_ROM_PICK(ponpoko)
STD_ROM_FN(ponpoko)

static void ponpoko_decode()
{
	for (INT32 i = 0; i < 0x1000; i += 0x10)	// Characters
	{
		for (INT32 j = 0; j < 8; j++)
		{
			INT32 t             = DrvGfxROM[i+j+0x08];
			DrvGfxROM[i+j+0x08] = DrvGfxROM[i+j+0x00];
			DrvGfxROM[i+j+0x00] = t;
		}
	}

	for (INT32 i = 0x1000; i < 0x2000; i += 0x20)	// Sprites
	{
		for (INT32 j = 0; j < 8; j++)
		{
			INT32 t             = DrvGfxROM[i+j+0x18];
			DrvGfxROM[i+j+0x18] = DrvGfxROM[i+j+0x10];
			DrvGfxROM[i+j+0x10] = DrvGfxROM[i+j+0x08];
			DrvGfxROM[i+j+0x08] = DrvGfxROM[i+j+0x00];
			DrvGfxROM[i+j+0x00] = t;
		}
	}
}

static INT32 ponpokoInit()
{
	return DrvInit(WoodpekMap, ponpoko_decode, PACMAN);
}

struct BurnDriver BurnDrvponpoko = {
	"ponpoko", NULL, NULL, NULL, "1982",
	"Ponpoko\0", NULL, "Sigma Enterprises Inc.", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PACMAN, GBF_PLATFORM, 0,
	NULL, ponpokoRomInfo, ponpokoRomName, NULL, NULL, ponpokoInputInfo, ponpokoDIPInfo,
	ponpokoInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	288, 224, 4, 3
};


// Ponpoko (Venture Line)

static struct BurnRomInfo ponpokovRomDesc[] = {
	{ "ppoko1.bin",   0x1000, 0x49077667, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "ppoko2.bin",   0x1000, 0x5101781a, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "ppoko3.bin",   0x1000, 0xd790ed22, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "ppoko4.bin",   0x1000, 0x4e449069, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "ppoko5.bin",   0x1000, 0x54ca3d7d, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "ppoko6.bin",   0x1000, 0x3055c7e0, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "ppoko7.bin",   0x1000, 0x3cbe47ca, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "ppoko8.bin",   0x1000, 0xb39be27d, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "ppoko9.bin",   0x1000, 0xb73e1a06, 2 | BRF_GRA },		//  8 Graphics
	{ "ppoko10.bin",  0x1000, 0x62069b5d, 2 | BRF_GRA },		//  9

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 10 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		// 11

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 12 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 13 Timing Prom (not used)
};

STD_ROM_PICK(ponpokov)
STD_ROM_FN(ponpokov)

struct BurnDriver BurnDrvponpokov = {
	"ponpokov", "ponpoko", NULL, NULL, "1982",
	"Ponpoko (Venture Line)\0", NULL, "Sigma Enterprises Inc. (Venture Line license)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PACMAN, GBF_PLATFORM, 0,
	NULL, ponpokovRomInfo, ponpokovRomName, NULL, NULL, ponpokoInputInfo, ponpokoDIPInfo,
	ponpokoInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	288, 224, 4, 3
};


// The Glob (Pac-Man hardware)

static struct BurnRomInfo theglobpRomDesc[] = {
	{ "glob.u2",      0x2000, 0x829d0bea, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "glob.u3",      0x2000, 0x31de6628, 1 | BRF_ESS | BRF_PRG },	//  1

	{ "glob.5e",      0x1000, 0x53688260, 2 | BRF_GRA },		//  2 Graphics
	{ "glob.5f",      0x1000, 0x051f59c7, 2 | BRF_GRA },		//  3

	{ "glob.7f",      0x0020, 0x1f617527, 3 | BRF_GRA },		//  4 Color Proms
	{ "glob.4a",      0x0100, 0x28faa769, 3 | BRF_GRA },		//  5

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  6 Sound Prom
	{ "82s126.3m"  ,  0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  7 Timing Prom (not used)
};

STD_ROM_PICK(theglobp)
STD_ROM_FN(theglobp)

static void theglobp_decrypt()
{
	for (INT32 i = 0; i < 0x4000; i++) {
		DrvZ80ROM[0x10000 + i] = BITSWAP08(DrvZ80ROM[i] ^ 0xfc, 3, 7, 0, 6, 4, 1, 2, 5);
		DrvZ80ROM[0x14000 + i] = BITSWAP08(DrvZ80ROM[i] ^ 0xf6, 1, 7, 0, 3, 4, 6, 2, 5);
		DrvZ80ROM[0x18000 + i] = BITSWAP08(DrvZ80ROM[i] ^ 0x7d, 3, 0, 4, 6, 7, 1, 2, 5);
		DrvZ80ROM[0x1c000 + i] = BITSWAP08(DrvZ80ROM[i] ^ 0x77, 1, 0, 4, 3, 7, 6, 2, 5);
	}
}

static INT32 theglobpInit()
{
	return DrvInit(StandardMap, theglobp_decrypt, EPOS);
}

struct BurnDriver BurnDrvtheglobp = {
	"theglobp", "suprglob", NULL, NULL, "1983",
	"The Glob (Pac-Man hardware)\0", NULL, "Epos Corporation", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_PLATFORM, 0,
	NULL, theglobpRomInfo, theglobpRomName, NULL, NULL, theglobpInputInfo, theglobpDIPInfo,
	theglobpInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Super Glob (Pac-Man hardware)

static struct BurnRomInfo sprglobpRomDesc[] = {
	{ "glob.u2",      0x2000, 0x829d0bea, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "glob.u3",      0x2000, 0x31de6628, 1 | BRF_ESS | BRF_PRG },	//  1

	{ "5e_2532.dat",  0x1000, 0x1aa16109, 2 | BRF_GRA },		//  2 Graphics
	{ "5f_2532.dat",  0x1000, 0xafe72a89, 2 | BRF_GRA },		//  3

	{ "glob.7f",      0x0020, 0x1f617527, 3 | BRF_GRA },		//  4 Color Prom
	{ "glob.4a",      0x0100, 0x28faa769, 3 | BRF_GRA },		//  5

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  6 Sound Prom
	{ "82s126.3m"  ,  0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  7 Timing Prom (not used)
};

STD_ROM_PICK(sprglobp)
STD_ROM_FN(sprglobp)

struct BurnDriver BurnDrvsprglobp = {
	"sprglobp", "suprglob", NULL, NULL, "1983",
	"Super Glob (Pac-Man hardware)\0", NULL, "Epos Corporation", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_PLATFORM, 0,
	NULL, sprglobpRomInfo, sprglobpRomName, NULL, NULL, theglobpInputInfo, theglobpDIPInfo,
	theglobpInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Super Glob (Pac-Man hardware) German

static struct BurnRomInfo sprglbpgRomDesc[] = {
	{ "ic8.1",        0x1000, 0xa2df2073, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "ic7.2",        0x1000, 0x3d2c22d9, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "ic15.3",       0x1000, 0xa252047f, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "ic14.4",       0x1000, 0x7efa81f1, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "ic92.5",       0x2000, 0xE54F484D, 2 | BRF_GRA },		//  4 Graphics

	{ "ic78.prm",     0x0020, 0x1f617527, 3 | BRF_GRA },		//  5 Color Prom
	{ "ic88.prm",     0x0100, 0x28faa769, 3 | BRF_GRA },		//  6

	{ "ic51.prm",     0x0100, 0xc29dea27, 4 | BRF_SND },		//  7 Sound Prom
	{ "ic70.prm"  ,   0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  8 Timing Prom (not used)
};

STD_ROM_PICK(sprglbpg)
STD_ROM_FN(sprglbpg)

struct BurnDriver BurnDrvsprglbpg = {
	"sprglbpg", "suprglob", NULL, NULL, "1983",
	"Super Glob (Pac-Man hardware) German\0", NULL, "bootleg", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_PLATFORM, 0,
	NULL, sprglbpgRomInfo, sprglbpgRomName, NULL, NULL, theglobpInputInfo, theglobpDIPInfo,
	puckmanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Beastie Feastie

static struct BurnRomInfo beastfRomDesc[] = {
	{ "bf-u2.bin",    0x2000, 0x3afc517b, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "bf-u3.bin",    0x2000, 0x8dbd76d0, 1 | BRF_ESS | BRF_PRG },	//  1

	{ "beastf.5e",    0x1000, 0x5654dc34, 2 | BRF_GRA },		//  2 Graphics
	{ "beastf.5f",    0x1000, 0x1b30ca61, 2 | BRF_GRA },		//  3

	{ "glob.7f",      0x0020, 0x1f617527, 3 | BRF_GRA },		//  4 Color Proms
	{ "glob.4a",      0x0100, 0x28faa769, 3 | BRF_GRA },		//  5

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  6 Sound Prom
	{ "82s126.3m"  ,  0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  7 Timing Prom (not used)
};

STD_ROM_PICK(beastf)
STD_ROM_FN(beastf)

struct BurnDriver BurnDrvbeastf = {
	"beastf", "suprglob", NULL, NULL, "1984",
	"Beastie Feastie\0", NULL, "Epos Corporation", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_PLATFORM, 0,
	NULL, beastfRomInfo, beastfRomName, NULL, NULL, theglobpInputInfo, theglobpDIPInfo,
	theglobpInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Van-Van Car

static struct BurnRomInfo vanvanRomDesc[] = {
	{ "van-1.50",     0x1000, 0xcf1b2df0, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "van-2.51",     0x1000, 0xdf58e1cb, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "van-3.52",     0x1000, 0x15571e24, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "van-4.53",     0x1000, 0xb724cbe0, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "van-5.39",     0x1000, 0xdb67414c, 1 | BRF_ESS | BRF_PRG },	//  4

	{ "van-20.18",    0x1000, 0x60efbe66, 2 | BRF_GRA },		//  5 Graphics
	{ "van-21.19",    0x1000, 0x5dd53723, 2 | BRF_GRA },		//  6

	{ "6331-1.6",     0x0020, 0xce1d9503, 3 | BRF_GRA },		//  7 Color Proms
	{ "6301-1.37",    0x0100, 0x4b803d9f, 3 | BRF_GRA },		//  8
};

STD_ROM_PICK(vanvan)
STD_ROM_FN(vanvan)

static void vanvanCallback()
{
	memcpy (DrvZ80ROM + 0x9000, DrvZ80ROM + 0x8000, 0x1000);
	memcpy (DrvZ80ROM + 0xa000, DrvZ80ROM + 0x8000, 0x1000);
	memcpy (DrvZ80ROM + 0xb000, DrvZ80ROM + 0x8000, 0x1000);
}

static INT32 vanvanInit()
{
	return DrvInit(DremshprMap, vanvanCallback, VANVAN);
}

struct BurnDriver BurnDrvvanvan = {
	"vanvan", NULL, NULL, NULL, "1983",
	"Van-Van Car\0", NULL, "Sanritsu", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, vanvanRomInfo, vanvanRomName, NULL, NULL, vanvanInputInfo, vanvanDIPInfo,
	vanvanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Van-Van Car (Karateco)

static struct BurnRomInfo vanvankRomDesc[] = {
	{ "van1.bin",	  0x1000, 0x00f48295, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "van-2.51",     0x1000, 0xdf58e1cb, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "van-3.52",     0x1000, 0x15571e24, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "van4.bin",     0x1000, 0xf8b37ed5, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "van5.bin",     0x1000, 0xb8c1e089, 1 | BRF_ESS | BRF_PRG },	//  4

	{ "van-20.18",    0x1000, 0x60efbe66, 2 | BRF_GRA },		//  5 Graphics
	{ "van-21.19",    0x1000, 0x5dd53723, 2 | BRF_GRA },		//  6

	{ "6331-1.6",     0x0020, 0xce1d9503, 3 | BRF_GRA },		//  7 Color Proms
	{ "6301-1.37",    0x0100, 0x4b803d9f, 3 | BRF_GRA },		//  8
};

STD_ROM_PICK(vanvank)
STD_ROM_FN(vanvank)

struct BurnDriver BurnDrvvanvank = {
	"vanvank", "vanvan", NULL, NULL, "1983",
	"Van-Van Car (Karateco)\0", NULL, "Karateco", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, vanvankRomInfo, vanvankRomName, NULL, NULL, vanvankInputInfo, vanvanDIPInfo,
	vanvanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Van-Van Car (set 3)

static struct BurnRomInfo vanvanbRomDesc[] = {
	{ "vv1.bin",      0x1000, 0xcf1b2df0, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "vv2.bin",      0x1000, 0x80eca6a5, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "vv3.bin",      0x1000, 0x15571e24, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "vv4.bin",      0x1000, 0xb1f04006, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "vv5.bin",      0x1000, 0xdb67414c, 1 | BRF_ESS | BRF_PRG },	//  4

	{ "vv20.bin",     0x1000, 0xeb56cb51, 2 | BRF_GRA },		//  5 Graphics
	{ "vv21.bin",     0x1000, 0x5dd53723, 2 | BRF_GRA },		//  6

	{ "6331-1.6",     0x0020, 0xce1d9503, 3 | BRF_GRA },		//  7 Color Proms
	{ "6301-1.37",    0x0100, 0x4b803d9f, 3 | BRF_GRA },		//  8
};

STD_ROM_PICK(vanvanb)
STD_ROM_FN(vanvanb)

struct BurnDriver BurnDrvvanvanb = {
	"vanvanb", "vanvan", NULL, NULL, "1983",
	"Van-Van Car (set 3)\0", NULL, "Karateco", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, vanvanbRomInfo, vanvanbRomName, NULL, NULL, vanvankInputInfo, vanvanDIPInfo,
	vanvanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Naughty Mouse (set 1)

static struct BurnRomInfo nmouseRomDesc[] = {
	{ "naumouse.d7",  0x0800, 0xe447ecfa, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "naumouse.d6",  0x0800, 0x2e6f13d9, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "naumouse.e7",  0x0800, 0x44a80f97, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "naumouse.e6",  0x0800, 0x9c7a46bd, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "naumouse.h7",  0x0800, 0x5bc94c5d, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "naumouse.h6",  0x0800, 0x1af29e22, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "naumouse.j7",  0x0800, 0xcc3be185, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "naumouse.j6",  0x0800, 0x66b3e5dc, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "naumouse.d5",  0x0800, 0x2ea7cc3f, 2 | BRF_GRA },		//  8 Graphics
	{ "naumouse.h5",  0x0800, 0x0511fcea, 2 | BRF_GRA },		//  9
	{ "naumouse.e5",  0x0800, 0xf5a627cd, 2 | BRF_GRA },		// 10
	{ "naumouse.j5",  0x0800, 0x65f2580e, 2 | BRF_GRA },		// 11

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Proms
	{ "naumouse.a4",  0x0100, 0xd8772167, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
};

STD_ROM_PICK(nmouse)
STD_ROM_FN(nmouse)

static INT32 nmouseInit()
{
	return DrvInit(StandardMap, eyes_decode, NMOUSE);
}

struct BurnDriver BurnDrvnmouse = {
	"nmouse", NULL, NULL, NULL, "1981",
	"Naughty Mouse (set 1)\0", NULL, "Amenip (Palcom Queen River)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_PLATFORM, 0,
	NULL, nmouseRomInfo, nmouseRomName, NULL, NULL, DrvInputInfo, nmouseDIPInfo,
	nmouseInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Naughty Mouse (set 2)

static struct BurnRomInfo nmousebRomDesc[] = {
	{ "naumouse.d7",  0x0800, 0xe447ecfa, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "naumouse.d6",  0x0800, 0x2e6f13d9, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "naumouse.e7",  0x0800, 0x44a80f97, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "naumouse.e6",  0x0800, 0x9c7a46bd, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "snatch2.bin",  0x0800, 0x405aa389, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "snatch6.bin",  0x0800, 0xf58e7df4, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "snatch3.bin",  0x0800, 0x06fb18ec, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "snatch7.bin",  0x0800, 0xd187b82b, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "naumouse.d5",  0x0800, 0x2ea7cc3f, 2 | BRF_GRA },		//  8 Graphics
	{ "naumouse.h5",  0x0800, 0x0511fcea, 2 | BRF_GRA },		//  9
	{ "naumouse.e5",  0x0800, 0xf5a627cd, 2 | BRF_GRA },		// 10
	{ "snatch11.bin", 0x0800, 0x330230a5, 2 | BRF_GRA },		// 11

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 12 Color Proms
	{ "naumouse.a4",  0x0100, 0xd8772167, 3 | BRF_GRA },		// 13

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 14 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 15 Timing Prom (not used)
};

STD_ROM_PICK(nmouseb)
STD_ROM_FN(nmouseb)

struct BurnDriver BurnDrvnmouseb = {
	"nmouseb", "nmouse", NULL, NULL, "1981",
	"Naughty Mouse (set 2)\0", NULL, "Amenip Nova Games Ltd.", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_PLATFORM, 0,
	NULL, nmousebRomInfo, nmousebRomName, NULL, NULL, DrvInputInfo, nmouseDIPInfo,
	nmouseInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Dream Shopper

static struct BurnRomInfo dremshprRomDesc[] = {
	{ "red_1.50",	  0x1000, 0x830c6361, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "red_2.51",     0x1000, 0xd22551cc, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "red_3.52",     0x1000, 0x0713a34a, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "red_4.53",     0x1000, 0xf38bcaaa, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "red_5.39",     0x1000, 0x6a382267, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "red_6.40",     0x1000, 0x4cf8b121, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "red_7.41",     0x1000, 0xbd4fc4ba, 1 | BRF_ESS | BRF_PRG },	//  6
 
	{ "red-20.18",    0x1000, 0x2d6698dc, 2 | BRF_GRA },		//  7 Graphics
	{ "red-21.19",    0x1000, 0x38c9ce9b, 2 | BRF_GRA },		//  8

	{ "6331-1.6",     0x0020, 0xce1d9503, 3 | BRF_GRA },		//  9 Color Proms
	{ "6301-1.37",    0x0100, 0x39d6fb5c, 3 | BRF_GRA },		// 10

	// Uses AY8910
};

STD_ROM_PICK(dremshpr)
STD_ROM_FN(dremshpr)

static INT32 dremshprInit()
{
	return DrvInit(DremshprMap, NULL, DREMSHPR);
}

struct BurnDriver BurnDrvdremshpr = {
	"dremshpr", NULL, NULL, NULL, "1982",
	"Dream Shopper\0", NULL, "Sanritsu", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_PACMAN, GBF_PUZZLE, 0,
	NULL, dremshprRomInfo, dremshprRomName, NULL, NULL, dremshprInputInfo, dremshprDIPInfo,
	dremshprInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Boardwalk Casino

static struct BurnRomInfo bwcasinoRomDesc[] = {
	{ "bwc_u2.bin",   0x2000, 0xe2eea868, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "bwc_u3.bin",   0x2000, 0xa935571e, 1 | BRF_ESS | BRF_PRG },	//  1

	{ "bwc_5e.bin",   0x1000, 0xe334c01e, 2 | BRF_GRA },		//  2 Graphics

	{ "aca_7f.bin",   0x0020, 0x133bb744, 3 | BRF_GRA },		//  3 Color Proms
	{ "aca_4a.bin",   0x0100, 0x8e29208f, 3 | BRF_GRA },		//  4

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  5 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  6 Timing Prom (not used)
};

STD_ROM_PICK(bwcasino)
STD_ROM_FN(bwcasino)

static void acitya_decode()
{
	for (INT32 i = 0; i < 0x4000; i++) {
		DrvZ80ROM[i + 0x10000] = BITSWAP08(DrvZ80ROM[i] ^ 0xb5, 1, 6, 7, 3, 4, 0, 2, 5);
		DrvZ80ROM[i + 0x14000] = BITSWAP08(DrvZ80ROM[i] ^ 0xa7, 7, 6, 1, 3, 4, 0, 2, 5);
		DrvZ80ROM[i + 0x18000] = BITSWAP08(DrvZ80ROM[i] ^ 0xfc, 1, 0, 7, 6, 4, 3, 2, 5); 
		DrvZ80ROM[i + 0x1c000] = BITSWAP08(DrvZ80ROM[i] ^ 0xee, 7, 0, 1, 6, 4, 3, 2, 5);
	}
}

static INT32 acityaInit()
{
	acitya = 1;

	return DrvInit(StandardMap, acitya_decode, EPOS);
}

struct BurnDriver BurnDrvbwcasino = {
	"bwcasino", NULL, NULL, NULL, "1983",
	"Boardwalk Casino\0", NULL, "EPOS Corporation", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_CASINO, 0,
	NULL, bwcasinoRomInfo, bwcasinoRomName, NULL, NULL, bwcasinoInputInfo, bwcasinoDIPInfo,
	acityaInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Atlantic City Action

static struct BurnRomInfo acityaRomDesc[] = {
	{ "aca_u2.bin",   0x2000, 0x261c2fdc, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "aca_u3.bin",   0x2000, 0x05fab4ca, 1 | BRF_ESS | BRF_PRG },	//  1

	{ "aca_5e.bin",   0x1000, 0x7f2dd2c9, 2 | BRF_GRA },		//  2 Graphics

	{ "aca_7f.bin",   0x0020, 0x133bb744, 3 | BRF_GRA },		//  3 Color Proms
	{ "aca_4a.bin",   0x0100, 0x8e29208f, 3 | BRF_GRA },		//  4

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  5 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  6 Timing Prom (not used)
};

STD_ROM_PICK(acitya)
STD_ROM_FN(acitya)

struct BurnDriver BurnDrvacitya = {
	"acitya", "bwcasino", NULL, NULL, "1983",
	"Atlantic City Action\0", NULL, "EPOS Corporation", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_CASINO, 0,
	NULL, acityaRomInfo, acityaRomName, NULL, NULL, acityaInputInfo, acityaDIPInfo,
	acityaInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Big Bucks

static struct BurnRomInfo bigbucksRomDesc[] = {
	{ "p.rom",        0x4000, 0xeea6c1c9, 1 | BRF_ESS | BRF_PRG },	//  0  Z80 Code
	{ "m.rom",        0x2000, 0xbb8f7363, 1 | BRF_ESS | BRF_PRG },	//  1

	{ "5e.cpu",       0x1000, 0x18442c37, 2 | BRF_GRA },		//  2 Graphics

	{ "82s123.7f",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		//  3 Color Proms
	{ "82s126.4a",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		//  4
 
	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  5 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  6 Timing Prom

	{ "rom1.rom",     0x8000, 0x90b7785f, 7 | BRF_PRG | BRF_ESS },	//  7 Question Roms
	{ "rom2.rom",     0x8000, 0x60172d77, 7 | BRF_PRG | BRF_ESS },	//  8
	{ "rom3.rom",     0x8000, 0xa2207320, 7 | BRF_PRG | BRF_ESS },	//  9
	{ "rom4.rom",     0x8000, 0x5a74c1f9, 7 | BRF_PRG | BRF_ESS },	// 10
	{ "rom5.rom",     0x8000, 0x93bc1080, 7 | BRF_PRG | BRF_ESS },	// 11
	{ "rom6.rom",     0x8000, 0xeea2423f, 7 | BRF_PRG | BRF_ESS },	// 12
	{ "rom7.rom",     0x8000, 0x96694055, 7 | BRF_PRG | BRF_ESS },	// 13
	{ "rom8.rom",     0x8000, 0xe68ebf8e, 7 | BRF_PRG | BRF_ESS },	// 14
	{ "rom9.rom",     0x8000, 0xfd20921d, 7 | BRF_PRG | BRF_ESS },	// 15
	{ "rom10.rom",    0x8000, 0x5091b951, 7 | BRF_PRG | BRF_ESS },	// 16
	{ "rom11.rom",    0x8000, 0x705128db, 7 | BRF_PRG | BRF_ESS },	// 17
	{ "rom12.rom",    0x8000, 0x74c776e7, 7 | BRF_PRG | BRF_ESS },	// 18
};

STD_ROM_PICK(bigbucks)
STD_ROM_FN(bigbucks)

static INT32 bigbucksInit()
{
	return DrvInit(BigbucksMap, NULL, BIGBUCKS);
}

struct BurnDriver BurnDrvbigbucks = {
	"bigbucks", NULL, NULL, NULL, "1986",
	"Big Bucks\0", NULL, "Dynasoft Inc.", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_QUIZ, 0,
	NULL, bigbucksRomInfo, bigbucksRomName, NULL, NULL, bigbucksInputInfo, bigbucksDIPInfo,
	bigbucksInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// MTV Rock-N-Roll Trivia (Part 2)

static struct BurnRomInfo rocktrv2RomDesc[] = {
	{ "1.aux",        0x4000, 0xd182947b, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "2.aux",        0x2000, 0x27a7461d, 1 | BRF_ESS | BRF_PRG },	//  1

	{ "5e.cpu",       0x1000, 0x0a6cc43b, 2 | BRF_GRA },		//  2 Graphics

	{ "7f.cpu",       0x0020, 0x7549a947, 3 | BRF_GRA },		//  3 Color Proms
	{ "4a.cpu",       0x0100, 0xddd5d88e, 3 | BRF_GRA },		//  4

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  5 Sound Prom
	{ "82s126.3m"  ,  0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  6 Timing Prom (not used)

	{ "3.aux",        0x4000, 0x5b117ca6, 7 | BRF_PRG | BRF_ESS },	//  7 Question Roms
	{ "4.aux",        0x4000, 0x81bfd4c3, 7 | BRF_PRG | BRF_ESS },	//  8
	{ "5.aux",        0x4000, 0xe976423c, 7 | BRF_PRG | BRF_ESS },	//  9
	{ "6.aux",        0x4000, 0x425946bf, 7 | BRF_PRG | BRF_ESS },	// 10
	{ "7.aux",        0x4000, 0x7056fc8f, 7 | BRF_PRG | BRF_ESS },	// 11
	{ "8.aux",        0x4000, 0x8b86464f, 7 | BRF_PRG | BRF_ESS },	// 12
	{ "9.aux",        0x4000, 0x17d8eba4, 7 | BRF_PRG | BRF_ESS },	// 13
	{ "10.aux",       0x4000, 0x398c8eb4, 7 | BRF_PRG | BRF_ESS },	// 14
	{ "11.aux",       0x4000, 0x7f376424, 7 | BRF_PRG | BRF_ESS },	// 15
	{ "12.aux",       0x4000, 0x8d5bbf81, 7 | BRF_PRG | BRF_ESS },	// 16
	{ "13.aux",       0x4000, 0x99fe2c21, 7 | BRF_PRG | BRF_ESS },	// 17
	{ "14.aux",       0x4000, 0xdf4cf5e7, 7 | BRF_PRG | BRF_ESS },	// 18
	{ "15.aux",       0x4000, 0x2a32de26, 7 | BRF_PRG | BRF_ESS },	// 19
	{ "16.aux",       0x4000, 0xfcd42187, 7 | BRF_PRG | BRF_ESS },	// 20
	{ "17.aux",       0x4000, 0x24d5c388, 7 | BRF_PRG | BRF_ESS },	// 21
	{ "18.aux",       0x4000, 0xfeb195fd, 7 | BRF_PRG | BRF_ESS },	// 22
};

STD_ROM_PICK(rocktrv2)
STD_ROM_FN(rocktrv2)

static void rocktrv2Callback()
{
	DrvZ80ROM[0x9ffe] = 0xa7;
	DrvZ80ROM[0x9fee] = 0x6d;
}

static INT32 rocktrv2Init()
{
	return DrvInit(Rocktrv2Map, rocktrv2Callback, ROCKTRV2);
}

struct BurnDriver BurnDrvrocktrv2 = {
	"rocktrv2", NULL, NULL, NULL, "1986",
	"MTV Rock-N-Roll Trivia (Part 2)\0", NULL, "Triumph Software Inc.", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_QUIZ, 0,
	NULL, rocktrv2RomInfo, rocktrv2RomName, NULL, NULL, rocktrv2InputInfo, rocktrv2DIPInfo,
	rocktrv2Init, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Ali Baba and 40 Thieves

static struct BurnRomInfo alibabaRomDesc[] = {
	{ "6e",           0x1000, 0x38d701aa, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "6f",           0x1000, 0x3d0e35f3, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "6h",           0x1000, 0x823bee89, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "6k",           0x1000, 0x474d032f, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "6l",           0x1000, 0x5ab315c1, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "6m",           0x0800, 0x438d0357, 1 | BRF_ESS | BRF_PRG },	//  5

	{ "5e",           0x0800, 0x85bcb8f8, 2 | BRF_GRA },		//  6 Graphics
	{ "5h",           0x0800, 0x38e50862, 2 | BRF_GRA },		//  7
	{ "5f",           0x0800, 0xb5715c86, 2 | BRF_GRA },		//  8
	{ "5k",           0x0800, 0x713086b3, 2 | BRF_GRA },		//  9

	{ "82s123.e7",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 10 Color Proms
	{ "82s129.a4",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		// 11

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 12 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 13 Timing Prom

	{ "ab7.bin",      0x0800, 0x52294ef5, 0 | BRF_OPT },		// 14 Mystery Rom? (1st & 2nd halves identical)
};

STD_ROM_PICK(alibaba)
STD_ROM_FN(alibaba)

static void alibabaCallback()
{
	memcpy (DrvZ80ROM + 0xa000, DrvZ80ROM + 0x9000, 0x0800);
	memset (DrvZ80ROM + 0x9000, 0, 0x800);
}

static INT32 alibabaInit()
{
	return DrvInit(AlibabaMap, alibabaCallback, ALIBABA);
}

struct BurnDriver BurnDrvalibaba = {
	"alibaba", NULL, NULL, NULL, "1982",
	"Ali Baba and 40 Thieves\0", NULL, "Sega", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, alibabaRomInfo, alibabaRomName, NULL, NULL, alibabaInputInfo, alibabaDIPInfo,
	alibabaInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Mustafa and 40 Thieves (bootleg)

static struct BurnRomInfo alibababRomDesc[] = {
	{ "6e",           0x1000, 0x38d701aa, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "6f",           0x1000, 0x3d0e35f3, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "6h",           0x1000, 0x823bee89, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "6k",           0x1000, 0x474d032f, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "ab5.bin",      0x1000, 0xae2f4aac, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "6m",           0x0800, 0x438d0357, 1 | BRF_ESS | BRF_PRG },	//  5

	{ "5e",           0x0800, 0x85bcb8f8, 2 | BRF_GRA },		//  6 Graphics
	{ "5h",           0x0800, 0x38e50862, 2 | BRF_GRA },		//  7
	{ "5f",           0x0800, 0xb5715c86, 2 | BRF_GRA },		//  8
	{ "5k",           0x0800, 0x713086b3, 2 | BRF_GRA },		//  9

	{ "82s123.e7",    0x0020, 0x2fc650bd, 3 | BRF_GRA },		// 10 Color Proms
	{ "82s129.a4",    0x0100, 0x3eb3a8e4, 3 | BRF_GRA },		// 11

	{ "82s126.1m",    0x0100, 0xa9cc86bf, 4 | BRF_SND },		// 12 Sound Prom
	{ "82s126.3m",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 13 Timing Prom

	{ "ab7.bin",      0x0800, 0x52294ef5, 0 | BRF_OPT },		// 14 Mystery Rom? (1st & 2nd halves identical)
};

STD_ROM_PICK(alibabab)
STD_ROM_FN(alibabab)

struct BurnDriver BurnDrvalibabab = {
	"alibabab", "alibaba", NULL, NULL, "1982",
	"Mustafa and 40 Thieves (bootleg)\0", NULL, "bootleg", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, alibababRomInfo, alibababRomName, NULL, NULL, alibabaInputInfo, alibabaDIPInfo,
	alibabaInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Birdiy

static struct BurnRomInfo birdiyRomDesc[] = {
	{ "a6.6a",        0x1000, 0x3a58f8ad, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "c6.6c",        0x1000, 0xfec61ea2, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "a4.4a",        0x1000, 0x3392783b, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "c4.4c",        0x1000, 0x2391d83d, 1 | BRF_ESS | BRF_PRG },	//  3

	{ "c1.1c",        0x1000, 0x8f6bf54f, 2 | BRF_GRA },		//  4 Graphics
	{ "c3.3c",        0x1000, 0x10b55440, 2 | BRF_GRA },		//  5

	{ "n82s123n.10n", 0x0020, 0xff344446, 3 | BRF_GRA },		//  6 Color Proms
	{ "n82s129n.9m",  0x0100, 0x63efb927, 3 | BRF_GRA },		//  7

	{ "n82s129n.4k",  0x0100, 0xa9cc86bf, 4 | BRF_SND },		//  8 Sound Prom
	{ "n82s129n.6l",  0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	//  9 Timing Prom (not used)
};

STD_ROM_PICK(birdiy)
STD_ROM_FN(birdiy)

static INT32 birdiyInit()
{
	return DrvInit(StandardMap, NULL, BIRDIY);
}

struct BurnDriver BurnDrvbirdiy = {
	"birdiy", NULL, NULL, NULL, "1982",
	"Birdiy\0", NULL, "Mama Top", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, birdiyRomInfo, birdiyRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	birdiyInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Pengo (set 1 rev c)

static struct BurnRomInfo pengoRomDesc[] = {
	{ "ep1689c.8",    0x1000, 0xf37066a8, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "ep1690b.7",    0x1000, 0xbaf48143, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "ep1691b.15",   0x1000, 0xadf0eba0, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "ep1692b.14",   0x1000, 0xa086d60f, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "ep1693b.21",   0x1000, 0xb72084ec, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "ep1694b.20",   0x1000, 0x94194a89, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "ep5118b.32",   0x1000, 0xaf7b12c4, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "ep5119c.31",   0x1000, 0x933950fe, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "ep1640.92",    0x2000, 0xd7eec6cd, 2 | BRF_GRA },		//  8 Graphics
	{ "ep1695.105",   0x2000, 0x5bfd26e9, 2 | BRF_GRA },		//  9

	{ "pr1633.78",    0x0020, 0x3a5844ec, 3 | BRF_GRA },		// 10 Color Proms
	{ "pr1634.88",    0x0400, 0x766b139b, 3 | BRF_GRA },		// 11

	{ "pr1635.51",    0x0100, 0xc29dea27, 4 | BRF_SND },		// 12 Sound Prom
	{ "pr1636.70",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 13 Timing Prom
};

STD_ROM_PICK(pengo)
STD_ROM_FN(pengo)

static void PengoDecode()
{
	static const UINT8 convtable[32][4] =
	{
		{ 0xa0,0x80,0xa8,0x88 }, { 0x28,0xa8,0x08,0x88 },
		{ 0x28,0xa8,0x08,0x88 }, { 0xa0,0x80,0xa8,0x88 },
		{ 0xa0,0x80,0x20,0x00 }, { 0xa0,0x80,0x20,0x00 },
		{ 0x08,0x28,0x88,0xa8 }, { 0xa0,0x80,0xa8,0x88 },
		{ 0x08,0x00,0x88,0x80 }, { 0x28,0xa8,0x08,0x88 },
		{ 0xa0,0x80,0x20,0x00 }, { 0x08,0x00,0x88,0x80 },
		{ 0xa0,0x80,0x20,0x00 }, { 0xa0,0x80,0x20,0x00 },
		{ 0xa0,0x80,0x20,0x00 }, { 0x00,0x08,0x20,0x28 },
		{ 0x88,0x80,0x08,0x00 }, { 0xa0,0x80,0x20,0x00 },
		{ 0x88,0x80,0x08,0x00 }, { 0x00,0x08,0x20,0x28 },
		{ 0x08,0x28,0x88,0xa8 }, { 0x08,0x28,0x88,0xa8 },
		{ 0xa0,0x80,0xa8,0x88 }, { 0xa0,0x80,0x20,0x00 },
		{ 0x08,0x00,0x88,0x80 }, { 0x88,0x80,0x08,0x00 },
		{ 0x00,0x08,0x20,0x28 }, { 0x88,0x80,0x08,0x00 },
		{ 0x08,0x28,0x88,0xa8 }, { 0x08,0x28,0x88,0xa8 },
		{ 0x08,0x00,0x88,0x80 }, { 0xa0,0x80,0x20,0x00 }
	};

	UINT8 *decrypted = DrvZ80ROM + 0x8000;

	for (INT32 A = 0x0000;A < 0x8000;A++)
	{
		INT32 xorval = 0;
		UINT8 src = DrvZ80ROM[A];

		INT32 row = (A & 1) + (((A >> 4) & 1) << 1) + (((A >> 8) & 1) << 2) + (((A >> 12) & 1) << 3);

		INT32 col = ((src >> 3) & 1) + (((src >> 5) & 1) << 1);

		if (src & 0x80)
		{
			col = 3 - col;
			xorval = 0xa8;
		}

		decrypted[A] = (src & ~0xa8) | (convtable[2*row][col] ^ xorval);
		DrvZ80ROM[A] = (src & ~0xa8) | (convtable[2*row+1][col] ^ xorval);

		if (convtable[2*row][col] == 0xff) decrypted[A] = 0xee;
		if (convtable[2*row+1][col] == 0xff) DrvZ80ROM[A] = 0xee;
	}
}

static void PengoGraphicsReorder()
{
	UINT8 *tmp = (UINT8*)BurnMalloc(0x2000);

	memcpy (tmp + 0x0000, DrvGfxROM + 0x2000, 0x1000);
	memcpy (tmp + 0x1000, DrvGfxROM + 0x1000, 0x1000);
	memcpy (DrvGfxROM + 0x1000, tmp + 0x0000, 0x2000);

	BurnFree (tmp);
}

static void PengoCallback()
{
	PengoDecode();

	PengoGraphicsReorder();
}

static INT32 pengoInit()
{
	return DrvInit(PengoMap, PengoCallback, PENGO);
}

struct BurnDriver BurnDrvpengo = {
	"pengo", NULL, NULL, NULL, "1982",
	"Pengo (set 1 rev c)\0", NULL, "Sega", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, pengoRomInfo, pengoRomName, NULL, NULL, PengoInputInfo, PengoDIPInfo,
	pengoInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Pengo (set 2)

static struct BurnRomInfo pengo2RomDesc[] = {
	{ "ic8.2",        0x1000, 0xe4924b7b, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "ic7.2",        0x1000, 0x72e7775d, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "ic15.2",       0x1000, 0x7410ef1e, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "ic14.2",       0x1000, 0x55b3f379, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "ep1693b.21",   0x1000, 0xb72084ec, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "ic20.2",       0x1000, 0x770570cf, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "ep5118b.32",   0x1000, 0xaf7b12c4, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "ic31.2",       0x1000, 0x669555c1, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "ep1640.92",    0x2000, 0xd7eec6cd, 2 | BRF_GRA },		//  8 Graphics
	{ "ep1695.105",   0x2000, 0x5bfd26e9, 2 | BRF_GRA },		//  9

	{ "pr1633.78",    0x0020, 0x3a5844ec, 3 | BRF_GRA },		// 10 Color Proms
	{ "pr1634.88",    0x0400, 0x766b139b, 3 | BRF_GRA },		// 11

	{ "pr1635.51",    0x0100, 0xc29dea27, 4 | BRF_SND },		// 12 Sound Prom
	{ "pr1636.70",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 13 Timing Prom
};

STD_ROM_PICK(pengo2)
STD_ROM_FN(pengo2)

struct BurnDriver BurnDrvpengo2 = {
	"pengo2", "pengo", NULL, NULL, "1982",
	"Pengo (set 2)\0", NULL, "Sega", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, pengo2RomInfo, pengo2RomName, NULL, NULL, PengoInputInfo, PengoDIPInfo,
	pengoInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Pengo (set 2 not encrypted)

static struct BurnRomInfo pengo2uRomDesc[] = {
	{ "epr5128.u8",   0x1000, 0x3dfeb20e, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "epr5129.u7",   0x1000, 0x1db341bd, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "epr5130.u15",  0x1000, 0x7c2842d5, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "epr5131.u14",  0x1000, 0x6e3c1f2f, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "ep5124.21",    0x1000, 0x95f354ff, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "epr5133.u20",  0x1000, 0x0fdb04b8, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "ep5126.32",    0x1000, 0xe5920728, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "epr5135.u31",  0x1000, 0x13de47ed, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "ep1640.92",    0x2000, 0xd7eec6cd, 2 | BRF_GRA },		//  8 Graphics
	{ "ep1695.105",   0x2000, 0x5bfd26e9, 2 | BRF_GRA },		//  9

	{ "pr1633.78",    0x0020, 0x3a5844ec, 3 | BRF_GRA },		// 10 Color Proms
	{ "pr1634.88",    0x0400, 0x766b139b, 3 | BRF_GRA },		// 11

	{ "pr1635.51",    0x0100, 0xc29dea27, 4 | BRF_SND },		// 12 Sound Prom
	{ "pr1636.70",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 13 Timing Prom
};

STD_ROM_PICK(pengo2u)
STD_ROM_FN(pengo2u)

static void PengouCallback()
{
	memcpy (DrvZ80ROM + 0x8000, DrvZ80ROM, 0x8000);

	PengoGraphicsReorder();
}

static INT32 pengouInit()
{
	return DrvInit(PengoMap, PengouCallback, PENGO);
}

struct BurnDriver BurnDrvpengo2u = {
	"pengo2u", "pengo", NULL, NULL, "1982",
	"Pengo (set 2 not encrypted)\0", NULL, "Sega", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, pengo2uRomInfo, pengo2uRomName, NULL, NULL, PengoInputInfo, PengoDIPInfo,
	pengouInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Pengo (set 3 not encrypted)

static struct BurnRomInfo pengo3uRomDesc[] = {
	{ "ep5120.8",     0x1000, 0xf01afb60, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "ep5121.7",     0x1000, 0x2eb38353, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "ep5122.15",    0x1000, 0xc33400d7, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "ep5123.14",    0x1000, 0x6a85c6a2, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "ep5124.21",    0x1000, 0x95f354ff, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "ep5125.20",    0x1000, 0x1a42310f, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "ep5126.32",    0x1000, 0xe5920728, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "ep5127.31",    0x1000, 0xa7d3d1d6, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "ep1640.92",    0x2000, 0xd7eec6cd, 2 | BRF_GRA },		//  8 Graphics
	{ "ep1695.105",   0x2000, 0x5bfd26e9, 2 | BRF_GRA },		//  9

	{ "pr1633.78",    0x0020, 0x3a5844ec, 3 | BRF_GRA },		// 10 Color Proms
	{ "pr1634.88",    0x0400, 0x766b139b, 3 | BRF_GRA },		// 11

	{ "pr1635.51",    0x0100, 0xc29dea27, 4 | BRF_SND },		// 12 Sound Prom
	{ "pr1636.70",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 13 Timing Prom
};

STD_ROM_PICK(pengo3u)
STD_ROM_FN(pengo3u)

struct BurnDriver BurnDrvpengo3u = {
	"pengo3u", "pengo", NULL, NULL, "1982",
	"Pengo (set 3 not encrypted)\0", NULL, "Sega", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, pengo3uRomInfo, pengo3uRomName, NULL, NULL, PengoInputInfo, PengoDIPInfo,
	pengouInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Pengo (set 4)

static struct BurnRomInfo pengo4RomDesc[] = {
	{ "ep1738.8",     0x1000, 0x68ba25ea, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "ep1739.7",     0x1000, 0x41e7b5b3, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "ep1740.15",    0x1000, 0x27f05f59, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "ep1741.14",    0x1000, 0x27d93ec1, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "ep1742.21",    0x1000, 0xb72084ec, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "ep1743.20",    0x1000, 0x770570cf, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "ep1744.32",    0x1000, 0xaf7b12c4, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "ep1745.31",    0x1000, 0x507e18b9, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "ep1640.92",    0x2000, 0xd7eec6cd, 2 | BRF_GRA },		//  8 Graphics
	{ "ep1695.105",   0x2000, 0x5bfd26e9, 2 | BRF_GRA },		//  9

	{ "pr1633.78",    0x0020, 0x3a5844ec, 3 | BRF_GRA },		// 10 Color Proms
	{ "pr1634.88",    0x0400, 0x766b139b, 3 | BRF_GRA },		// 11

	{ "pr1635.51",    0x0100, 0xc29dea27, 4 | BRF_SND },		// 12 Sound Prom
	{ "pr1636.70",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 13 Timing Prom
};

STD_ROM_PICK(pengo4)
STD_ROM_FN(pengo4)

struct BurnDriver BurnDrvpengo4 = {
	"pengo4", "pengo", NULL, NULL, "1982",
	"Pengo (set 4)\0", NULL, "Sega", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, pengo4RomInfo, pengo4RomName, NULL, NULL, PengoInputInfo, PengoDIPInfo,
	pengoInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Pengo (bootleg)

static struct BurnRomInfo pengobRomDesc[] = {
	{ "1",            0x2000, 0xe04064db, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "2",            0x2000, 0x75752424, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "021_pn03.bin", 0x1000, 0x7824e3ef, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "020_pn07.bin", 0x1000, 0x377b9663, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "032_pn04.bin", 0x1000, 0xbfde44c1, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "031_pn08.bin", 0x1000, 0x64e8c30d, 1 | BRF_ESS | BRF_PRG },	//  5

	{ "5",            0x2000, 0x1232437b, 2 | BRF_GRA },		//  6 Graphics
	{ "ep1695.105",   0x2000, 0x5bfd26e9, 2 | BRF_GRA },		//  7

	{ "pr1633.78",    0x0020, 0x3a5844ec, 3 | BRF_GRA },		//  8 Color Proms
	{ "pr1634.88",    0x0400, 0x766b139b, 3 | BRF_GRA },		//  9

	{ "pr1635.51",    0x0100, 0xc29dea27, 4 | BRF_SND },		// 10 Sound Prom
	{ "pr1636.70",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 11 Timing Prom
};

STD_ROM_PICK(pengob)
STD_ROM_FN(pengob)

static void PentaDecode()
{
	static const UINT8 data_xortable[2][8] =
	{
		{ 0xa0,0x82,0x28,0x0a,0x82,0xa0,0x0a,0x28 },
		{ 0x88,0x0a,0x82,0x00,0x88,0x0a,0x82,0x00 }
	};

	static const UINT8 opcode_xortable[8][8] =
	{
		{ 0x02,0x08,0x2a,0x20,0x20,0x2a,0x08,0x02 },
		{ 0x88,0x88,0x00,0x00,0x88,0x88,0x00,0x00 },
		{ 0x88,0x0a,0x82,0x00,0xa0,0x22,0xaa,0x28 },
		{ 0x88,0x0a,0x82,0x00,0xa0,0x22,0xaa,0x28 },
		{ 0x2a,0x08,0x2a,0x08,0x8a,0xa8,0x8a,0xa8 },
		{ 0x2a,0x08,0x2a,0x08,0x8a,0xa8,0x8a,0xa8 },
		{ 0x88,0x0a,0x82,0x00,0xa0,0x22,0xaa,0x28 },
		{ 0x88,0x0a,0x82,0x00,0xa0,0x22,0xaa,0x28 }
	};

	UINT8 *decrypt = DrvZ80ROM + 0x8000;

	for (INT32 A = 0; A < 0x8000; A++)
	{
		UINT8 src = DrvZ80ROM[A];

		INT32 i = A & 1;

		INT32 j = ((src >> 1) & 1) + (((src >> 3) & 1) << 1) + (((src >> 5) & 1) << 2);

		if (src & 0x80) j = 7 - j;

		DrvZ80ROM[A] = src ^ data_xortable[i][j];

		i = ((A >> 4) & 1) + (((A >> 8) & 1) << 1) + (((A >> 12) & 1) << 2);

		decrypt[A] = src ^ opcode_xortable[i][j];
	}
}

static void PengobCallback()
{
	PentaDecode();

	PengoGraphicsReorder();
}

static INT32 pengobInit()
{
	return DrvInit(PengoMap, PengobCallback, PENGO);
}

struct BurnDriver BurnDrvpengob = {
	"pengob", "pengo", NULL, NULL, "1982",
	"Pengo (bootleg)\0", NULL, "Bootleg", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, pengobRomInfo, pengobRomName, NULL, NULL, PengoInputInfo, PengoDIPInfo,
	pengobInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


// Penta

static struct BurnRomInfo pentaRomDesc[] = {
	{ "008_pn01.bin", 0x1000, 0x22f328df, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "007_pn05.bin", 0x1000, 0x15bbc7d3, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "015_pn02.bin", 0x1000, 0xde82b74a, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "014_pn06.bin", 0x1000, 0x160f3836, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "021_pn03.bin", 0x1000, 0x7824e3ef, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "020_pn07.bin", 0x1000, 0x377b9663, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "032_pn04.bin", 0x1000, 0xbfde44c1, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "031_pn08.bin", 0x1000, 0x64e8c30d, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "092_pn09.bin", 0x2000, 0x6afeba9d, 2 | BRF_GRA },		//  6 Graphics
	{ "ep1695.105",   0x2000, 0x5bfd26e9, 2 | BRF_GRA },		//  7

	{ "pr1633.78",    0x0020, 0x3a5844ec, 3 | BRF_GRA },		//  8 Color Proms
	{ "pr1634.88",    0x0400, 0x766b139b, 3 | BRF_GRA },		//  9

	{ "pr1635.51",    0x0100, 0xc29dea27, 4 | BRF_SND },		// 10 Sound Prom
	{ "pr1636.70",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 11 Timing Prom
};

STD_ROM_PICK(penta)
STD_ROM_FN(penta)

struct BurnDriver BurnDrvpenta = {
	"penta", "pengo", NULL, NULL, "1982",
	"Penta\0", NULL, "Bootleg", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MAZE, 0,
	NULL, pentaRomInfo, pentaRomName, NULL, NULL, PengoInputInfo, PengoDIPInfo,
	pengobInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 288, 3, 4
};


