/*
 Copyright (c) 2009, OpenEmu Team
 
 
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

#include "Pkgs/glew.h"

#import "SupermodelGameCore.h"
#import <OERingBuffer.h>
#import "OEArcadeSystemResponderClient.h"
#import <OpenGL/gl.h>

#include <new>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdarg>

#include "Supermodel.h"

@interface SupermodelGameCore () <OEArcadeSystemResponderClient>
@end

CModel3			*Model3 = new CModel3();
SupermodelGameCore *current;
@implementation SupermodelGameCore

- (id)init
{
	self = [super init];
    if(self != nil)
    {
        if(videoBuffer) 
            free(videoBuffer);
        videoBuffer = (uint16_t*)malloc(496 * 384 * 2);
    }
	
	current = self;
    
	return self;
}

#pragma mark Exectuion

- (BOOL)rendersToOpenGL
{
    return YES;
}

- (void)executeFrame
{
    [self executeFrameSkippingFrame:NO];
}

- (void)executeFrameSkippingFrame: (BOOL) skip
{
    Model3->RunFrame();
}

- (BOOL)loadFileAtPath: (NSString*) path
{
    const char *zipPath = [path UTF8String];
        
	char			baseTitleStr[128], titleStr[128];
	CRender2D		*Render2D = new CRender2D();
	CRender3D		*Render3D = new CRender3D();
	unsigned		prevFPSTicks, currentFPSTicks, currentTicks, targetTicks, startTicks;
	unsigned		fpsFramesElapsed, framesElapsed;
	unsigned		showCrosshairs = 0;	// bit 1: player 1 crosshair, bit 0: player 2
	bool			gameHasLightguns = false;
	bool			quit = 0;
	bool            paused = 0;
    unsigned		xOffset=0, yOffset=0, xRes=496, yRes=384;
    
    // Initialize and load ROMs
    Model3->Init();
    
	Model3->LoadROMSet(g_Model3GameList, zipPath);
    
    // Apply game-specific settings and then, lastly, command line settings
	//ReadConfigFile(Model3->GetGameInfo()->id);
	//ApplySettings(CmdLine, "Global");
    
	// Load NVRAM
	//LoadNVRAM(Model3);
    
    // Start up SDL and open a GL window
  	//xRes = g_Config.xRes;
  	//yRes = g_Config.yRes;
  	//sprintf(baseTitleStr, "Supermodel - %s", Model3->GetGameInfo()->title);
    NSLog(@"Supermodel - %s", Model3->GetGameInfo()->title);
	//CreateGLScreen(baseTitleStr,&xOffset,&yOffset,&xRes,&yRes,true,g_Config.fullScreen);
    
	// Info log GL information and user options
	//PrintGLInfo(false, true, false);
	//LogConfig();
	
	// Initialize audio system
	OpenAudio();
    
    // Hide mouse if fullscreen, enable crosshairs for gun games
	//Inputs->GetInputSystem()->SetMouseVisibility(!g_Config.fullScreen);
	//gameHasLightguns = !!(Model3->GetGameInfo()->inputFlags & (GAME_INPUT_GUN1|GAME_INPUT_GUN2));
	//if (g_Config.fullScreen && gameHasLightguns)
	//	showCrosshairs = 1;	// show player 1 cursor only by default (TODO: add an IsMapped() member to CInput to allow testing for both lightguns)
    
	// Attach the inputs to the emulator
	//Model3->AttachInputs(Inputs);
	
	// Initialize the renderer
	Render2D->Init(xOffset, yOffset, xRes, yRes);
	Render3D->Init(xOffset, yOffset, xRes, yRes);
	Model3->AttachRenderers(Render2D,Render3D);
    
	// Reset emulator
	Model3->Reset();
    
    // Emulate!
	fpsFramesElapsed = 0;
	framesElapsed = 0;
	//prevFPSTicks = SDL_GetTicks();
	//startTicks = prevFPSTicks;
	
    
    
    
    
    /*
    while (!quit)
	
    
    {
		// Check if paused
		if (!paused)
		{
			// If not, run one frame
			Model3->RunFrame();
			
			// Show crosshairs for light gun games
			UpdateCrosshairs(Inputs, showCrosshairs);
			
			// Swap the buffers
			SDL_GL_SwapBuffers();
		}
		
		// Poll the inputs
		if (!Inputs->Poll(Model3->GetGameInfo(), xOffset, yOffset, xRes, yRes))
			quit = 1;
		
#ifdef SUPERMODEL_DEBUGGER
		bool processUI = true;
		if (Debugger != NULL)
		{
			Debugger->Poll();
            
			// Check if debugger requests exit or pause
			if (Debugger->CheckExit())
			{
				quit = 1;
				processUI = false;
			}
			else if (Debugger->CheckPause())
			{
				paused = 1;
				processUI = false;
			}
		}
		if (processUI)
		{
#endif // SUPERMODEL_DEBUGGER
            
            // Check UI controls
            if (Inputs->uiExit->Pressed())
            {
                // Quit emulator
                quit = 1;
            }
            else if (Inputs->uiReset->Pressed())
            {
                if (!paused)
                {
                    Model3->PauseThreads();
                    SetAudioEnabled(false);
                }
                
                // Reset emulator
                Model3->Reset();
                
#ifdef SUPERMODEL_DEBUGGER
                // If debugger was supplied, reset it too
                if (Debugger != NULL)
                    Debugger->Reset();
#endif // SUPERMODEL_DEBUGGER
                
                if (!paused)
                {
                    Model3->ResumeThreads();
                    SetAudioEnabled(true);
                }
                
                puts("Model 3 reset.");
            }
            else if (Inputs->uiPause->Pressed())
            {
                // Toggle emulator paused flag
                paused = !paused;
                
                if (paused)
                {
                    Model3->PauseThreads();
                    SetAudioEnabled(false);
                    sprintf(titleStr, "%s (Paused)", baseTitleStr);
                    SDL_WM_SetCaption(titleStr,NULL);
                }
                else
                {
                    Model3->ResumeThreads();
                    SetAudioEnabled(true);
                    SDL_WM_SetCaption(baseTitleStr,NULL);
                }
            }
            else if (Inputs->uiSaveState->Pressed())
            {
                if (!paused)
                {
                    Model3->PauseThreads();
                    SetAudioEnabled(false);
                }
                
                // Save game state
                SaveState(Model3);
                
                if (!paused)
                {
                    Model3->ResumeThreads();
                    SetAudioEnabled(true);
                }
            }
            else if (Inputs->uiChangeSlot->Pressed())
            {
                // Change save slot
                ++saveSlot;
                saveSlot %= 10;	// clamp to [0,9]
                printf("Save slot: %d\n", saveSlot);
            }
            else if (Inputs->uiLoadState->Pressed())
            {
                if (!paused)
                {
                    Model3->PauseThreads();
                    SetAudioEnabled(false);
                }
                
                // Load game state
                LoadState(Model3);
                
#ifdef SUPERMODEL_DEBUGGER
                // If debugger was supplied, reset it after loading state
                if (Debugger != NULL)
                    Debugger->Reset();
#endif // SUPERMODEL_DEBUGGER
                
                if (!paused)
                {
                    Model3->ResumeThreads();
                    SetAudioEnabled(true);
                }
            }
            else if (Inputs->uiMusicVolUp->Pressed())
            {
                // Increase music volume by 10%
                if (Model3->GetGameInfo()->mpegBoard)
                {
                    int vol = (int) g_Config.GetMusicVolume() + 10;
                    if (vol > 200)
                        vol = 200;
                    g_Config.SetMusicVolume(vol);
                    printf("Music volume: %d%%", vol);
                    if (200 == vol)
                        puts(" (maximum)");
                    else
                        printf("\n");
                }
                else
                    puts("This game does not have an MPEG music board.");
            }
            else if (Inputs->uiMusicVolDown->Pressed())
            {
                // Decrease music volume by 10%
                if (Model3->GetGameInfo()->mpegBoard)
                {
                    int vol = (int) g_Config.GetMusicVolume() - 10;
                    if (vol < 0)
                        vol = 0;
                    g_Config.SetMusicVolume(vol);
                    printf("Music volume: %d%%", vol);
                    if (0 == vol)
                        puts(" (muted)");
                    else
                        printf("\n");
                }
                else
                    puts("This game does not have an MPEG music board.");
            }
            else if (Inputs->uiSoundVolUp->Pressed())
            {
                // Increase sound volume by 10%
                int vol = (int) g_Config.GetSoundVolume() + 10;
                if (vol > 200)
                    vol = 200;
                g_Config.SetSoundVolume(vol);
                printf("Sound volume: %d%%", vol);
                if (200 == vol)
                    puts(" (maximum)");
                else
                    printf("\n");
            }
            else if (Inputs->uiSoundVolDown->Pressed())
            {
                // Decrease sound volume by 10%
                int vol = (int) g_Config.GetSoundVolume() - 10;
                if (vol < 0)
                    vol = 0;
                g_Config.SetSoundVolume(vol);
                printf("Sound volume: %d%%", vol);
                if (0 == vol)
                    puts(" (muted)");
                else
                    printf("\n");
            }
            else if (Inputs->uiDumpInpState->Pressed())
            {
                // Dump input states
                Inputs->DumpState(Model3->GetGameInfo());
            }
            else if (Inputs->uiSelectCrosshairs->Pressed() && gameHasLightguns)
            {
                
                showCrosshairs++;
                switch ((showCrosshairs&3))
                {
                    case 0:	puts("Crosshairs disabled."); 				break;
                    case 3:	puts("Crosshairs enabled.");				break;
                    case 1:	puts("Showing Player 1 crosshair only.");	break;
                    case 2: puts("Showing Player 2 crosshair only.");	break;
                }
            }
            else if (Inputs->uiClearNVRAM->Pressed())
            {
                // Clear NVRAM
                Model3->ClearNVRAM();
                puts("NVRAM cleared.");
            }
            else if (Inputs->uiToggleFrLimit->Pressed())
            {
                // Toggle frame limiting
                g_Config.throttle = !g_Config.throttle;
                printf("Frame limiting: %s\n", g_Config.throttle?"On":"Off");
            }
#ifdef SUPERMODEL_DEBUGGER
			else if (Debugger != NULL && Inputs->uiEnterDebugger->Pressed())
			{
				// Break execution and enter debugger
				Debugger->ForceBreak(true);
			}
		}
#endif // SUPERMODEL_DEBUGGER
 		
 		// FPS and frame rate
 		currentFPSTicks = SDL_GetTicks();
 		currentTicks = currentFPSTicks;
 		
 		// FPS
 		if (g_Config.showFPS)
 		{
 			++fpsFramesElapsed;
			if((currentFPSTicks-prevFPSTicks) >= 1000)	// update FPS every 1 second (each tick is 1 ms)
			{
				sprintf(titleStr, "%s - %1.1f FPS%s", baseTitleStr, (float)fpsFramesElapsed*(float)(currentFPSTicks-prevFPSTicks)/1000.0f, paused ? " (Paused)" : "");
				SDL_WM_SetCaption(titleStr,NULL);
				prevFPSTicks = currentFPSTicks;			// reset tick count
				fpsFramesElapsed = 0;					// reset frame count
			}
		}
		
		// Frame limiting/paused
		if (paused || g_Config.throttle)
		{
			++framesElapsed;
			targetTicks = startTicks + (unsigned) ((float)framesElapsed * 1000.0f/60.0f);
			if (currentTicks <= targetTicks)	// add a delay until we reach the next (target) frame time
				SDL_Delay(targetTicks-currentTicks);
			else								// begin a new frame
			{
				framesElapsed = 0;
				startTicks = currentTicks;
			}
		}
	}
    
	// Make sure all threads are paused before shutting down
	Model3->PauseThreads();		
	
#ifdef SUPERMODEL_DEBUGGER
	// If debugger was supplied, detach it from system and restore old logger
	if (Debugger != NULL)
	{
		Debugger->Detach();
		SetLogger(oldLogger);
	}
#endif // SUPERMODEL_DEBUGGER
	
	// Save NVRAM
	SaveNVRAM(Model3);
	
	// Close audio
	CloseAudio();
    
	// Shut down
#ifndef SUPERMODEL_DEBUGGER
	delete Model3;
#endif // SUPERMODEL_DEBUGGER
	delete Render2D;
	delete Render3D;
	
	// Dump PowerPC registers
#ifdef DEBUG
	for (int i = 0; i < 32; i += 4)
        printf("R%d=%08X\tR%d=%08X\tR%d=%08X\tR%d=%08X\n",
               i + 0, ppc_get_gpr(i + 0),
               i + 1, ppc_get_gpr(i + 1),
               i + 2, ppc_get_gpr(i + 2),
               i + 3, ppc_get_gpr(i + 3));
	printf("PC =%08X\n", ppc_get_pc());
  	printf("LR =%08X\n", ppc_get_lr());
#endif
	
	return 0;
    
	// Quit with an error
QuitError:
#ifndef SUPERMODEL_DEBUGGER
	delete Model3;
#endif // SUPERMODEL_DEBUGGER
	delete Render2D;
	delete Render3D;
	return 1;
}*/
    
    return YES;
}

#pragma mark Video
- (const void *)videoBuffer
{
    return videoBuffer;
}

- (OEIntRect)screenRect
{
    return OERectMake(0, 0, 496, 384);
}

- (OEIntSize)bufferSize
{
    return OESizeMake(496, 384);
}

- (void)setupEmulation
{
}

- (void)resetEmulation
{
    // Reset emulator
	Model3->Reset();
}

- (void)stopEmulation
{
    [super stopEmulation];
}

- (void)dealloc
{
    free(videoBuffer);
}

- (GLenum)pixelFormat
{
    return GL_BGRA;
}

- (GLenum)pixelType
{
    return GL_UNSIGNED_SHORT_1_5_5_5_REV;
}

- (GLenum)internalPixelFormat
{
    return GL_RGB5;
}

- (double)audioSampleRate
{
    return 44100;
}

- (NSTimeInterval)frameInterval
{
    return 60;
}

- (NSUInteger)channelCount
{
    return 2;
}

- (oneway void)didPushArcadeButton:(OEArcadeButton)button forPlayer:(NSUInteger)player;
{
    //pad[player-1][FBAEmulatorValues[button]] = 1; //1 or 0xFFFF
}

- (oneway void)didReleaseArcadeButton:(OEArcadeButton)button forPlayer:(NSUInteger)player;
{
    //pad[player-1][FBAEmulatorValues[button]] = 0;
}

- (BOOL)saveStateToFileAtPath:(NSString *)fileName
{
    // Save game state
    //SaveState(Model3);
    return YES;
}

- (BOOL)loadStateFromFileAtPath:(NSString *)fileName
{
    // Load game state
    //LoadState(Model3);
    return YES;
}

/******************************************************************************
 Error and Debug Logging
 ******************************************************************************/

// Log file names
#define DEBUG_LOG_FILE	"debug.log"
#define ERROR_LOG_FILE	"error.log"

// Logger object is used to redirect log messages appropriately
static CLogger *s_Logger = NULL;

CLogger *GetLogger()
{
	return s_Logger;
}

void SetLogger(CLogger *Logger)
{
	s_Logger = Logger;
}

void DebugLog(const char *fmt, ...)
{
	if (s_Logger == NULL)
		return;
	va_list vl;
	va_start(vl, fmt);
	s_Logger->DebugLog(fmt, vl);
	va_end(vl);
}

void InfoLog(const char *fmt, ...)
{
	if (s_Logger == NULL)
		return;
	va_list vl;
	va_start(vl, fmt);
	s_Logger->InfoLog(fmt, vl);
	va_end(vl);
}

bool ErrorLog(const char *fmt, ...)
{
	if (s_Logger == NULL)
		return FAIL;
	va_list vl;
	va_start(vl, fmt);
	s_Logger->ErrorLog(fmt, vl);
	va_end(vl);
	return FAIL;
}

void SetAudioCallback(AudioCallbackFPtr newCallback, void *newData)
{
}

bool OutputAudio(unsigned numSamples, INT16 *leftBuffer, INT16 *rightBuffer)
{
    return NO;
}

bool OpenAudio()
{
    return NO;
}

@end
