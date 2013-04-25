/*
 Copyright (c) 2011, OpenEmu Team
 
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the OpenEmu Team nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#import "ATR800GameCore.h"
#import <OERingBuffer.h>
#import "OE5200SystemResponderClient.h"
#import <OpenGL/gl.h>

//#define _UINT32

#include "platform.h"
#include "memory.h"
#include "atari.h"
#include "config.h"
#include "monitor.h"
#include "log.h"
#ifdef SOUND
#include "sound.h"
#endif
#include "screen.h"
#include "colours.h"
#include "cfg.h"
#include "devices.h"
#include "input.h"
#include "rtime.h"
#include "sio.h"
#include "cassette.h"
#include "pbi.h"
#include "antic.h"
#include "gtia.h"
#include "pia.h"
#include "pokey.h"
#include "ide.h"
#include "cartridge.h"
#include "ui.h"
#include "akey.h"
#include "sysrom.h"

typedef struct {
	int up;
	int down;
	int left;
	int right;
	int fire;
	int fire2;
	int start;
	int pause;
	int reset;
} ATR5200ControllerState;

@interface ATR800GameCore () <OE5200SystemResponderClient>
{
	unsigned char *screenBuffer;
    double sampleRate;
	ATR5200ControllerState controllerStates[4];
}
- (void)renderToBuffer;
- (ATR5200ControllerState)controllerStateForPlayer:(NSUInteger)playerNum;
int Atari_POT(int);
int16_t convertSample(uint8_t);
@end

static ATR800GameCore *currentCore;

void ATR800WriteSoundBuffer(uint8_t *buffer, unsigned int len);

static int num_cont = 4;

#pragma mark - atari800 platform calls

int PLATFORM_Initialise(int *argc, char *argv[])
{
#ifdef SOUND
	Sound_Initialise(argc, argv);
#endif
	
	return TRUE;
}

int PLATFORM_Exit(int run_monitor)
{
	Log_flushlog();
	
	if (run_monitor && MONITOR_Run())
		return TRUE;
	
#ifdef SOUND
	Sound_Exit();
#endif
	
	return FALSE;
}

// believe these are used for joystick/input or something
// they get called off of the frame call
int PLATFORM_PORT(int num)
{
	if(num < 4 && num >= 0) {
		ATR5200ControllerState state = [currentCore controllerStateForPlayer:num];
		if(state.up == 1 && state.left == 1) {
			return INPUT_STICK_UL;
		}
		else if(state.up == 1 && state.right == 1) {
			return INPUT_STICK_UR;
		}
		else if(state.up == 1) {
			//NSLog(@"UP");
			return INPUT_STICK_FORWARD;
		}
		else if(state.down == 1 && state.left == 1) {
			return INPUT_STICK_LL;
		}
		else if(state.down == 1 && state.right == 1) {
			//NSLog(@"Left-right");
			return INPUT_STICK_LR;
		}
		else if(state.down == 1) {
			//NSLog(@"DOWN");
			return INPUT_STICK_BACK;
		}
		else if(state.left == 1) {
			//NSLog(@"Left");
			return INPUT_STICK_LEFT;
		}
		else if(state.right == 1) {
			//NSLog(@"Right");
			return INPUT_STICK_RIGHT;
		}
		return INPUT_STICK_CENTRE;
	}
	return 0xff;
}

int PLATFORM_TRIG(int num)
{
	if(num < 4 && num >= 0) {
		ATR5200ControllerState state = [currentCore controllerStateForPlayer:num];
		if(state.fire == 1) {
			//NSLog(@"Pew pew");
		}
		return state.fire == 1 ? 0 : 1;
	}
	return 1;
}

// Looks to be called when the atari UI is on screen
// in ui.c & ui_basic.c
int PLATFORM_Keyboard(void)
{
	return 0;
}

// maybe we can update our RGB buffer in this?
void PLATFORM_DisplayScreen(void)
{
}

int Atari_POT(int num)
{
	int val;
//	cont_cond_t *cond;
	
	if (Atari800_machine_type != Atari800_MACHINE_5200) {
		if (0 /*emulate_paddles*/) {
//			if (num + 1 > num_cont) return(228);
//			
//			cond = &mcond[num];
//			val = cond->joyx;
//			val = val * 228 / 255;
//			if (val > 227) return(1);
//			return(228 - val);
		}
		else {
			return(228);
		}
	}
	else {	/* 5200 version:
			 *
			 * num / 2: which controller
			 * num & 1 == 0: x axis
			 * num & 1 == 1: y axis
			 */
		
		if (num / 2 + 1 > num_cont) return(INPUT_joy_5200_center);
		ATR5200ControllerState state = [currentCore controllerStateForPlayer:(num/2)];
//		cond = &mcond[num / 2];
		if(num & 1) { // y-axis
			if(state.up)
				val = 255;
			else if(state.down)
				val = 0;
			else
				val = 127;
		}
		else { // x-axis
			if(state.right)
				val = 255;
			else if(state.left)
				val = 0;
			else
				val = 127;
		}
//		val = (num & 1) ? cond->joyy : cond->joyx;
		
		/* normalize into 5200 range */
		//NSLog(@"joystick value: %i", val);
		if (val == 127) return(INPUT_joy_5200_center);
		if (val < 127) {
			/*val -= INPUT_joy_5200_min;*/
			val = val * (INPUT_joy_5200_center - INPUT_joy_5200_min) / 127;
			return(val + INPUT_joy_5200_min);
		}
		else {
			val = val * INPUT_joy_5200_max / 255;
			if (val < INPUT_joy_5200_center)
				val = INPUT_joy_5200_center;
			return(val);
		}
	}
}

int16_t convertSample(uint8_t sample)
{
	float floatSample = (float)sample / 255;
	return (int16_t)(floatSample * 65535 - 32768);
}

void ATR800WriteSoundBuffer(uint8_t *buffer, unsigned int len) {
	int samples = len / sizeof(uint8_t);
	NSUInteger newLength = len * sizeof(int16_t);
	int16_t *newBuffer = malloc(len * sizeof(int16_t));
	int16_t *dest = newBuffer;
	uint8_t *source = buffer;
	for(int i = 0; i < samples; i++) {
		*dest = convertSample(*source);
		dest++;
		source++;
	}
    [[currentCore ringBufferAtIndex:0] write:newBuffer maxLength:newLength];
	free(newBuffer);
}

@implementation ATR800GameCore

- (id)init
{
    if((self = [super init]))
    {
        screenBuffer = malloc(Screen_WIDTH * Screen_HEIGHT * 4);
    }
    
    currentCore = self;
    
    return self;
}

- (void)dealloc
{
	Atari800_Exit(false);
	free(screenBuffer);
	[super dealloc];
}

- (void)executeFrame
{
//	NSLog(@"Executing");
	// Note: this triggers UI code and also calls the input functions above
	Atari800_Frame();
	[self renderToBuffer];
}

- (void)setupEmulation
{
//	if (!Atari800_Initialise(0, NULL))
//		NSLog(@"Failed to initialize Atari800 emulation");
}

- (ATR5200ControllerState)controllerStateForPlayer:(NSUInteger)playerNum
{
	ATR5200ControllerState state = {0,0,0,0,0,0,0,0};
	if(playerNum < 4) {
		state = controllerStates[playerNum];
	}
	return state;
}

- (BOOL)loadFileAtPath:(NSString*)path
{
    //Get the size of the rom so that Atari800 knows which 5200 cart type to load
    NSData* dataObj = [NSData dataWithContentsOfFile:[path stringByStandardizingPath]];
    size_t size = [dataObj length];

    DLog(@"Loadeding File: ", path);
    
    char biosFileName[2048];
    NSString *appSupportPath = [NSString pathWithComponents:@[
                                [NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) lastObject],
                                @"OpenEmu", @"BIOS"]];
    
    strcpy(biosFileName, [[appSupportPath stringByAppendingPathComponent:@"5200.rom"] UTF8String]);
	
    Atari800_tv_mode = Atari800_TV_NTSC;
    
	Colours_PreInitialise();
	
    // set 5200.rom BIOS path
    SYSROM_SetPath(biosFileName, 3, SYSROM_5200);
	
    // setup what machine
	Atari800_machine_type = Atari800_MACHINE_5200;
	MEMORY_ram_size = 16;

	int test = 0;
	int *argc = &test;
	char *argv[] = {};
	if (
#if !defined(BASIC) && !defined(CURSES_BASIC)
		!Colours_Initialise(argc, argv) ||
#endif
		!Devices_Initialise(argc, argv)
		|| !RTIME_Initialise(argc, argv)
#ifdef IDE
		|| !IDE_Initialise(argc, argv)
#endif
		|| !SIO_Initialise (argc, argv)
		|| !CASSETTE_Initialise(argc, argv)
		|| !PBI_Initialise(argc,argv)
#ifdef VOICEBOX
		|| !VOICEBOX_Initialise(argc, argv)
#endif
#ifndef BASIC
		|| !INPUT_Initialise(argc, argv)
#endif
#ifdef XEP80_EMULATION
		|| !XEP80_Initialise(argc, argv)
#endif
#ifdef AF80
		|| !AF80_Initialise(argc, argv)
#endif
#ifdef NTSC_FILTER
		|| !FILTER_NTSC_Initialise(argc, argv)
#endif
#if SUPPORTS_CHANGE_VIDEOMODE
		|| !VIDEOMODE_Initialise(argc, argv)
#endif
#ifndef DONT_DISPLAY
		/* Platform Specific Initialisation */
		|| !PLATFORM_Initialise(argc, argv)
#endif
		|| !Screen_Initialise(argc, argv)
		/* Initialise Custom Chips */
		|| !ANTIC_Initialise(argc, argv)
		|| !GTIA_Initialise(argc, argv)
		|| !PIA_Initialise(argc, argv)
		|| !POKEY_Initialise(argc, argv)
		) {
		NSLog(@"Failed to initialize part of atari800");
		return NO;
	}
	
	// this gets called again, maybe we can skip this first one?
	if(!Atari800_InitialiseMachine()) {
		NSLog(@"** Failed to initialize machine");
		return NO;
	}
	
	/* Install requested ROM cartridge */
	if (path) {
		int r = CARTRIDGE_Insert([path UTF8String]);
		if (r < 0) {
			Log_print("Error inserting cartridge \"%s\": %s", [path UTF8String],
					  r == CARTRIDGE_CANT_OPEN ? "Can't open file" :
					  r == CARTRIDGE_BAD_FORMAT ? "Bad format" :
					  r == CARTRIDGE_BAD_CHECKSUM ? "Bad checksum" :
					  "Unknown error");
		}
		if (r > 0) {
#ifdef BASIC
			Log_print("Raw cartridge images not supported in BASIC version!");
#else /* BASIC */
			
#ifndef __PLUS
//			UI_is_active = TRUE;
//			CARTRIDGE_type = UI_SelectCartType(r);
//			UI_is_active = FALSE;

            NSLog(@"Cart size: %zd", size >> 10);
            
            //Tell Atari800 which 5200 cart type to load based on size
            switch (size >> 10) {
                case 40:
                    //CARTRIDGE_SetType
                    CARTRIDGE_main.type = CARTRIDGE_5200_40; //bounty bob strikes back
                    break;
                case 32:
                    CARTRIDGE_main.type = CARTRIDGE_5200_32;
                    break;
                //case 16:
                //    CARTRIDGE_type = CARTRIDGE_5200_EE_16; //two chip: congo bongo, etc
                case 16:
                    CARTRIDGE_main.type = CARTRIDGE_5200_NS_16; //one chip: chop lifter, miner 2049er, etc
                    break;
                case 8:
                    CARTRIDGE_main.type = CARTRIDGE_5200_8;
                    break;
                case 4:
                    CARTRIDGE_main.type = CARTRIDGE_5200_4;
                    break;
            }
            
#else /* __PLUS */
			CARTRIDGE_main.type = (CARTRIDGE_NONE == nCartType ? UI_SelectCartType(r) : nCartType);
#endif /* __PLUS */
			CARTRIDGE_ColdStart();
			
#endif /* BASIC */
		}
#ifndef __PLUS
		if (CARTRIDGE_main.type != CARTRIDGE_NONE) {
			//int for5200 = CARTRIDGE_IsFor5200(CARTRIDGE_main.type);
			//if (for5200 && Atari800_machine_type != Atari800_MACHINE_5200) {
            if (Atari800_machine_type != Atari800_MACHINE_5200) {
				Atari800_machine_type = Atari800_MACHINE_5200;
				MEMORY_ram_size = 16;
				Atari800_InitialiseMachine();
			}
			//else if (!for5200 && Atari800_machine_type == Atari800_MACHINE_5200) {
			//	Atari800_machine_type = Atari800_MACHINE_XLXE;
			//	MEMORY_ram_size = 64;
			//	Atari800_InitialiseMachine();
			//}
		}
#endif /* __PLUS */
	}

    return YES;
}
- (void)resetEmulation
{
	Atari800_Coldstart();
}

- (void)stopEmulation
{
//	Atari800_Exit(false);
    [super stopEmulation];
}

- (OEIntSize)bufferSize
{
    return OEIntSizeMake(Screen_WIDTH, Screen_HEIGHT);
}

- (const void *)videoBuffer
{
    return screenBuffer;
}

- (GLenum)pixelFormat
{
    return GL_BGRA;
}

- (GLenum)pixelType
{
    return GL_UNSIGNED_INT_8_8_8_8_REV;
}

- (GLenum)internalPixelFormat
{
    return GL_RGB8;
}

- (double)audioSampleRate
{
    return 22050;
}

- (NSTimeInterval)frameInterval
{
    return Atari800_tv_mode == Atari800_TV_NTSC ? Atari800_FPS_NTSC : Atari800_FPS_PAL;
}

- (NSUInteger)channelCount
{
    return 2;
}


- (BOOL)saveStateToFileAtPath:(NSString *)fileName
{
    return NO;
}

- (BOOL)loadStateFromFileAtPath:(NSString *)fileName
{
    return NO;
}

#pragma mark -
//Think this is converting to 32-bit BGRA
- (void)renderToBuffer
{
	int i, j;
	UBYTE *source = (UBYTE *)(Screen_atari);
	UBYTE *destination = screenBuffer;
	for (i = 0; i < Screen_HEIGHT; i++) {
//		printf("ROW-%i", i);
		for (j = 0; j < Screen_WIDTH; j++) {
			UBYTE r,g,b;
			r = Colours_GetR(*source);
			g = Colours_GetG(*source);
			b = Colours_GetB(*source);
			*destination++ = b;
			*destination++ = g;
			*destination++ = r;
			*destination++ = 0xff;
			source++;
//			printf(",%u", *source);
		}
//		source += Screen_WIDTH - ATARI_VISIBLE_WIDTH;
//		printf("\n");
	}
//	NSLog(@"Done render");
}

#pragma mark - Input handling

- (oneway void)didPush5200Button:(OE5200Button)button forPlayer:(NSUInteger)player
{
	player--;
	//NSLog(@"Pressed: %i", button);
	switch (button) {
		case OE5200ButtonFire1:
			controllerStates[player].fire = 1;
			break;
        case OE5200ButtonFire2:
			//controllerStates[player].fire2 = 1;
            INPUT_key_shift = 1; //AKEY_SHFTCTRL
			break;
		case OE5200ButtonUp:
			controllerStates[player].up = 1;
            //INPUT_key_code = AKEY_UP ^ AKEY_CTRL;
            //INPUT_key_code = INPUT_STICK_FORWARD;
			break;
		case OE5200ButtonDown:
			controllerStates[player].down = 1;
			break;
		case OE5200ButtonLeft:
			controllerStates[player].left = 1;
			break;
		case OE5200ButtonRight:
			controllerStates[player].right = 1;
			break;
		case OE5200ButtonStart:
//			controllerStates[player].start = 1;
			INPUT_key_code = AKEY_5200_START;
			break;
        case OE5200ButtonPause:
            INPUT_key_code = AKEY_5200_PAUSE;
            break;
        case OE5200ButtonReset:
            INPUT_key_code = AKEY_5200_RESET;
		default:
			break;
	}
}

- (oneway void)didRelease5200Button:(OE5200Button)button forPlayer:(NSUInteger)player
{
    player--;
    //NSLog(@"Released: %i", button);
    switch (button) {
        case OE5200ButtonFire1:
            controllerStates[player].fire = 0;
            break;
        case OE5200ButtonFire2:
            //controllerStates[player].fire2 = 0;
            INPUT_key_shift = 0;
            break;
        case OE5200ButtonUp:
            controllerStates[player].up = 0;
            //INPUT_key_code = 0xff;
            break;
        case OE5200ButtonDown:
            controllerStates[player].down = 0;
            break;
        case OE5200ButtonLeft:
            controllerStates[player].left = 0;
            break;
        case OE5200ButtonRight:
            controllerStates[player].right = 0;
            break;
        case OE5200ButtonStart:
            //			controllerStates[player].start = 1;
            INPUT_key_code = AKEY_NONE;
            break;
        case OE5200ButtonPause:
            INPUT_key_code = AKEY_NONE;
            break;
        case OE5200ButtonReset:
            INPUT_key_code = AKEY_NONE;
        default:
            break;
    }
}

@end
