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
    
    // Initialize and load ROMs
    Model3->Init();
    
	Model3->LoadROMSet(g_Model3GameList, zipPath);
    
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

@end
