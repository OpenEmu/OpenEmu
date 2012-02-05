/*
 Copyright (c) 2010 OpenEmu Team
 
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

#import "MupenGameCore.h"
#import "OEMupenSupport.h"
#import "api/config.h"
#import "api/m64p_config.h"
#import "version.h"

#import <OEGameDocument.h>
#import <OERingBuffer.h>
#import <OEHIDEvent.h>
#import <OpenGL/gl.h>
#import "OEN64SystemResponderClient.h"

pthread_mutex_t gEmuVIMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  gEmuVICond  = PTHREAD_COND_INITIALIZER;

static void MupenDebugCallback(void *Context, int level, const char *message)
{
    NSLog(@"Mupen (%d): %s", level, message);
}

static void MupenStateCallback(void *Context, m64p_core_param ParamChanged, int NewValue)
{
    NSLog(@"Mupen: param %d -> %d", ParamChanged, NewValue);
}

NSString *MupenControlNames[] = {
    @"N64_DPadR", @"N64_DPadL", @"N64_DPadD", @"N64_DPadU",
    @"N64_Start", @"N64_Z", @"N64_B", @"N64_A", @"N64_CR",
    @"N64_CL", @"N64_CD", @"N64_CU", @"N64_R", @"N64_L"
}; // FIXME: missing: joypad X, joypad Y, mempak switch, rumble switch

@implementation MupenGameCore

- (BOOL)loadFileAtPath:(NSString*) path
{
    NSBundle *coreBundle = [NSBundle bundleForClass:[self class]];
    const char *configPath, *dataPath;
    
    configPath = [[self supportDirectoryPath] UTF8String];
    dataPath   = [[coreBundle resourcePath] UTF8String]; // FIXME: should be path to bundle
    
    // open core here
    //CoreStartup(MUPEN_API_VERSION, configPath, dataPath, self, MupenDebugCallback, self, MupenStateCallback);
    CoreStartup(CONFIG_API_VERSION, configPath, dataPath, self, MupenDebugCallback, self, MupenStateCallback);
    
    // plugins + config
    m64p_handle section;
    int ival = 0;
    ConfigOpenSection("Core", &section);
    ConfigSetParameter(section, "R4300Emulator", M64TYPE_INT, &ival);
    
    // load rom here
    if (romData) [romData release];
    romData = [NSData dataWithContentsOfMappedFile:path];
    
    CoreDoCommand(M64CMD_ROM_OPEN, [romData length], (void*)[romData bytes]);
    
    return YES;
}

#if 0
- (void)setupEmulation
{
    // ?
}
#endif

- (void)startEmulation
{
    if (!isRunning) {
        [super startEmulation];
        [NSThread detachNewThreadSelector:@selector(mupenEmuThread) toTarget:self withObject:nil];  
    }
}

- (void)mupenEmuThread
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    CoreDoCommand(M64CMD_EXECUTE, 0, NULL);
    
    [pool drain];
}

- (void)executeFrameSkippingFrame: (BOOL) skip
{
    // FIXME: skip
    pthread_mutex_lock(&gEmuVIMutex);
    pthread_cond_broadcast(&gEmuVICond);
    pthread_mutex_unlock(&gEmuVIMutex);
}

- (void)executeFrame
{
    [self executeFrameSkippingFrame:NO];
}

- (void)stopEmulation
{    
    // FIXME: this needs to send a quit event into the input
    // which will be read by the emu thread
    // which will then die
    //CoreDoCommand(M64CMD_STOP, 0, NULL);
}

- (BOOL)saveStateToFileAtPath: (NSString *)fileName
{
    // freeze save
    //FIXME how to fit into emu event loop?
    //CoreDoCommand(M64CMD_STATE_SAVE, 1, (void*)[fileName UTF8String]);
    return YES;
}

- (BOOL)loadStateFromFileAtPath: (NSString *)fileName
{
    // freeze load
    //FIXME how to fit into emu event loop?
    //CoreDoCommand(M64CMD_STATE_LOAD, 1, (void*)[fileName UTF8String]);
    return YES;
}

#pragma mark stub methods

- (OEIntSize)bufferSize
{
    return OESizeMake(640, 480);
}

- (const void *)videoBuffer
{
    if (!black) {
        black = calloc(1, 640 * 480 * 2);
    }
    return black;
}

- (GLenum)pixelFormat
{
    return GL_RGB;
}

- (GLenum)pixelType
{
    return GL_UNSIGNED_SHORT_5_6_5;
}

- (GLenum)internalPixelFormat
{
    return GL_RGB5;
}

- (const void *)soundBuffer
{
    return NULL;
}

- (NSUInteger)channelCount
{
    return 0;
}

- (NSUInteger)frameSampleCount
{
    return 0;
}

- (NSUInteger)soundBufferSize
{
    return 0;
}

- (NSUInteger)frameSampleRate
{
    return 0;
}

@end
