#ifndef _MDFN_DRIVERS_INPUT_H
#define _MDFN_DRIVERS_INPUT_H

typedef struct {
        uint8  ButtType;
        uint8  DeviceNum;;
        uint32 ButtonNum;
	uint64 DeviceID;
} ButtConfig;

extern CFGSTRUCT InputConfig[];

#define BUTTC_NONE		0x00
#define BUTTC_KEYBOARD          0x01
#define BUTTC_JOYSTICK          0x02
#define BUTTC_MOUSE             0x03

#define MKK(k) SDLK_##k
#define MKK_COUNT (SDLK_LAST+1)

// Called after a game is loaded.
void InitGameInput(MDFNGI *GI);

// Called to synch
void ResyncGameInputSettings(void);

// Called when a game is closed.
void KillGameInput(void);

void MDFND_UpdateInput(void);

void MakeInputSettings(std::vector <MDFNSetting> &settings);
void KillInputSettings(void); // Called after MDFNI_Kill() is called

extern bool DNeedRewind; // Only read/write in game thread.

bool InitCommandInput(void);
void KillCommandInput(void);

#endif
