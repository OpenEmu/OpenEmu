#ifndef _INPUT_H_
#define _INPUT_H_

#include <ostream>

#include "git.h"

//MBG TODO - COMBINE THESE INPUTC AND INPUTCFC

//The interface for standard joystick port device drivers
struct INPUTC
{
	//these methods call the function pointers (or not, if they are null)
	uint8 Read(int w) { if(_Read) return _Read(w); else return 0; }
	void Write(uint8 w) { if(_Write) _Write(w); }
	void Strobe(int w) { if(_Strobe) _Strobe(w); }
	void Update(int w, void *data, int arg) { if(_Update) _Update(w,data,arg); }
	void SLHook(int w, uint8 *bg, uint8 *spr, uint32 linets, int final) { if(_SLHook) _SLHook(w,bg,spr,linets,final); }

	uint8 (*_Read)(int w);
	void (*_Write)(uint8 v);
	void (*_Strobe)(int w);
	//update will be called if input is coming from the user. refresh your logical state from user input devices
	void (*_Update)(int w, void *data, int arg);
	void (*_SLHook)(int w, uint8 *bg, uint8 *spr, uint32 linets, int final);
};

//The interface for the expansion port device drivers
struct INPUTCFC
{
	//these methods call the function pointers (or not, if they are null)
	uint8 Read(int w, uint8 ret) { if(_Read) return _Read(w,ret); else return ret; }
	void Write(uint8 v) { if(_Write) _Write(v); }
	void Strobe() { if(_Strobe) _Strobe(); }
	void Update(void *data, int arg) { if(_Update) _Update(data,arg); }
	void SLHook(uint8 *bg, uint8 *spr, uint32 linets, int final) { if(_SLHook) _SLHook(bg,spr,linets,final); }

	uint8 (*_Read)(int w, uint8 ret);
	void (*_Write)(uint8 v);
	void (*_Strobe)();
	//update will be called if input is coming from the user. refresh your logical state from user input devices
	void (*_Update)(void *data, int arg);
	void (*_SLHook)(uint8 *bg, uint8 *spr, uint32 linets, int final);
};

extern struct JOYPORT
{
	JOYPORT(int _w) : w(_w) {}
	int w;
	int attrib;
	ESI type;
	void* ptr;
	INPUTC* driver;
} joyports[2];

extern struct FCPORT
{
	int attrib;
	ESIFC type;
	void* ptr;
	INPUTCFC* driver;
} portFC;


void FCEU_UpdateInput(void);
void InitializeInput(void);
void FCEU_UpdateBot(void);
extern void (*PStrobe[2])(void);

//called from PPU on scanline events.
extern void InputScanlineHook(uint8 *bg, uint8 *spr, uint32 linets, int final);

void FCEU_DoSimpleCommand(int cmd);

enum EMUCMD
{
	EMUCMD_POWER=0,
	EMUCMD_RESET,
	EMUCMD_SCREENSHOT,
	EMUCMD_HIDE_MENU_TOGGLE,
	//fixed: current command key handling handle only command table record index with
	//the same as cmd enumerarot index, or else does wrong key mapping, fixed it but placed this enum here anyway 
	//...i returned it back.
	//adelikat, try to find true cause of problem before reversing it
	EMUCMD_EXIT,

	EMUCMD_SPEED_SLOWEST,
	EMUCMD_SPEED_SLOWER,
	EMUCMD_SPEED_NORMAL,
	EMUCMD_SPEED_FASTER,
	EMUCMD_SPEED_FASTEST,
	EMUCMD_SPEED_TURBO,
	EMUCMD_SPEED_TURBO_TOGGLE,

	EMUCMD_SAVE_SLOT_0,
	EMUCMD_SAVE_SLOT_1,
	EMUCMD_SAVE_SLOT_2,
	EMUCMD_SAVE_SLOT_3,
	EMUCMD_SAVE_SLOT_4,
	EMUCMD_SAVE_SLOT_5,
	EMUCMD_SAVE_SLOT_6,
	EMUCMD_SAVE_SLOT_7,
	EMUCMD_SAVE_SLOT_8,
	EMUCMD_SAVE_SLOT_9,
	EMUCMD_SAVE_SLOT_NEXT,
	EMUCMD_SAVE_SLOT_PREV,
	EMUCMD_SAVE_STATE,
	EMUCMD_SAVE_STATE_AS,
	EMUCMD_SAVE_STATE_SLOT_0,
	EMUCMD_SAVE_STATE_SLOT_1,
	EMUCMD_SAVE_STATE_SLOT_2,
	EMUCMD_SAVE_STATE_SLOT_3,
	EMUCMD_SAVE_STATE_SLOT_4,
	EMUCMD_SAVE_STATE_SLOT_5,
	EMUCMD_SAVE_STATE_SLOT_6,
	EMUCMD_SAVE_STATE_SLOT_7,
	EMUCMD_SAVE_STATE_SLOT_8,
	EMUCMD_SAVE_STATE_SLOT_9,
	EMUCMD_LOAD_STATE,
	EMUCMD_LOAD_STATE_FROM,
	EMUCMD_LOAD_STATE_SLOT_0,
	EMUCMD_LOAD_STATE_SLOT_1,
	EMUCMD_LOAD_STATE_SLOT_2,
	EMUCMD_LOAD_STATE_SLOT_3,
	EMUCMD_LOAD_STATE_SLOT_4,
	EMUCMD_LOAD_STATE_SLOT_5,
	EMUCMD_LOAD_STATE_SLOT_6,
	EMUCMD_LOAD_STATE_SLOT_7,
	EMUCMD_LOAD_STATE_SLOT_8,
	EMUCMD_LOAD_STATE_SLOT_9,

	EMUCMD_SCRIPT_RELOAD,

	EMUCMD_SOUND_TOGGLE,
	EMUCMD_SOUND_VOLUME_UP,
	EMUCMD_SOUND_VOLUME_DOWN,
	EMUCMD_SOUND_VOLUME_NORMAL,

	EMUCMD_FDS_EJECT_INSERT,
	EMUCMD_FDS_SIDE_SELECT,

	EMUCMD_VSUNI_COIN,
	EMUCMD_VSUNI_TOGGLE_DIP_0,
	EMUCMD_VSUNI_TOGGLE_DIP_1,
	EMUCMD_VSUNI_TOGGLE_DIP_2,
	EMUCMD_VSUNI_TOGGLE_DIP_3,
	EMUCMD_VSUNI_TOGGLE_DIP_4,
	EMUCMD_VSUNI_TOGGLE_DIP_5,
	EMUCMD_VSUNI_TOGGLE_DIP_6,
	EMUCMD_VSUNI_TOGGLE_DIP_7,
	EMUCMD_VSUNI_TOGGLE_DIP_8,
	EMUCMD_VSUNI_TOGGLE_DIP_9,
	EMUCMD_MISC_AUTOSAVE,
	EMUCMD_MISC_SHOWSTATES,
	EMUCMD_MISC_USE_INPUT_PRESET_1,
	EMUCMD_MISC_USE_INPUT_PRESET_2,
	EMUCMD_MISC_USE_INPUT_PRESET_3,
	EMUCMD_MISC_DISPLAY_BG_TOGGLE,
	EMUCMD_MISC_DISPLAY_OBJ_TOGGLE,
	EMUCMD_MISC_UNDOREDOSAVESTATE,
	EMUCMD_MAX
};

enum EMUCMDTYPE
{
	EMUCMDTYPE_MISC=0,
	EMUCMDTYPE_SPEED,
	EMUCMDTYPE_STATE,
	EMUCMDTYPE_MOVIE,
	EMUCMDTYPE_SOUND,
	EMUCMDTYPE_FDS,
	EMUCMDTYPE_VSUNI,

	EMUCMDTYPE_MAX
};

extern const char* FCEUI_CommandTypeNames[];

typedef void EMUCMDFN(void);

struct EMUCMDTABLE
{
	int cmd;
	int type;
	EMUCMDFN* fn_on;
	EMUCMDFN* fn_off;
	int state;
	char* name;
	int flags; //EMUCMDFLAG
};

extern struct EMUCMDTABLE FCEUI_CommandTable[];

#endif //_INPUT_H_
