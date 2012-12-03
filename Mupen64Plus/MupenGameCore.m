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

// We need to mess with core internals
#define M64P_CORE_PROTOTYPES 1

#import "MupenGameCore.h"
#import "api/config.h"
#import "api/m64p_common.h"
#import "api/m64p_config.h"
#import "api/m64p_frontend.h"
#import "osal/dynamiclib.h"
#import "version.h"

#import <OpenEmuBase/OERingBuffer.h>
#import <OpenGL/gl.h>
#import "OEN64SystemResponderClient.h"

#import "plugin.h"

#import <dlfcn.h>

NSString *MupenControlNames[] = {
    @"N64_DPadU", @"N64_DPadD", @"N64_DPadL", @"N64_DPadR",
    @"N64_CU", @"N64_CD", @"N64_CL", @"N64_CR",
    @"N64_B", @"N64_A", @"N64_R", @"N64_L", @"N64_Z", @"N64_Start"
}; // FIXME: missing: joypad X, joypad Y, mempak switch, rumble switch

@interface MupenGameCore () <OEN64SystemResponderClient>
{
    NSData  *romData;
    uint8_t *black;
@public
    uint8_t padData[OEN64ButtonCount];
}

@end

dispatch_semaphore_t gMupenWaitForVISemaphore;
dispatch_semaphore_t gCoreWaitForFinishSemaphore;

struct MupenVideoSettings gMupenVideoSettings = {640*4, 480*4, 32};
MupenGameCore *g_core;

@implementation MupenGameCore

- (instancetype)init
{
    if (self = [super init]) {
        gMupenWaitForVISemaphore = dispatch_semaphore_create(0);
        gCoreWaitForFinishSemaphore = dispatch_semaphore_create(0);
    }
    g_core = self;
    return self;
}

- (void)dealloc
{
    dispatch_release(gMupenWaitForVISemaphore);
    dispatch_release(gCoreWaitForFinishSemaphore);
}

static void MupenDebugCallback(void *context, int level, const char *message)
{
    NSLog(@"Mupen (%d): %s", level, message);
}

static void MupenStateCallback(void *Context, m64p_core_param ParamChanged, int NewValue)
{
    NSLog(@"Mupen: param %d -> %d", ParamChanged, NewValue);
}

static void *dlopen_myself()
{
    Dl_info info;
    
    dladdr(dlopen_myself, &info);
    
    return dlopen(info.dli_fname, 0);
}

void GetKeys(int Control, BUTTONS *Keys)
{
    Keys->R_DPAD = g_core->padData[OEN64ButtonDPadRight];
    Keys->L_DPAD = g_core->padData[OEN64ButtonDPadLeft];
    Keys->D_DPAD = g_core->padData[OEN64ButtonDPadDown];
    Keys->U_DPAD = g_core->padData[OEN64ButtonDPadUp];
    Keys->START_BUTTON = g_core->padData[OEN64ButtonStart];
    Keys->Z_TRIG = g_core->padData[OEN64ButtonZ];
    Keys->B_BUTTON = g_core->padData[OEN64ButtonB];
    Keys->A_BUTTON = g_core->padData[OEN64ButtonA];
    Keys->R_CBUTTON = g_core->padData[OEN64ButtonCRight];
    Keys->L_CBUTTON = g_core->padData[OEN64ButtonCLeft];
    Keys->D_CBUTTON = g_core->padData[OEN64ButtonCDown];
    Keys->U_CBUTTON = g_core->padData[OEN64ButtonCUp];
    Keys->R_TRIG = g_core->padData[OEN64ButtonR];
    Keys->L_TRIG = g_core->padData[OEN64ButtonL];
    Keys->X_AXIS = (Keys->R_DPAD ? INT8_MAX : (Keys->L_DPAD ? INT8_MIN : 0));
    Keys->Y_AXIS = (Keys->D_DPAD ? INT8_MIN : (Keys->U_DPAD ? INT8_MAX : 0));
}

void InitiateControllers (CONTROL_INFO ControlInfo)
{
    ControlInfo.Controls[0].Present = 1;
}

- (BOOL)loadFileAtPath:(NSString *)path
{
    NSBundle *coreBundle = [NSBundle bundleForClass:[self class]];
    const char *configPath, *dataPath;

    configPath = [[[self supportDirectoryPath] stringByAppendingString:@"/"] fileSystemRepresentation];
    dataPath   = [[coreBundle resourcePath] fileSystemRepresentation];
    
    // open core here
    CoreStartup(FRONTEND_API_VERSION, configPath, dataPath, (__bridge void *)self, MupenDebugCallback, (__bridge void *)self, MupenStateCallback);

#ifdef DEBUG
    // Disable dynarec (for debugging)
    m64p_handle section;
    int ival = 0;

    ConfigOpenSection("Core", &section);
    ConfigSetParameter(section, "R4300Emulator", M64TYPE_INT, &ival);
#endif
        
    // Load ROM
    romData = [NSData dataWithContentsOfMappedFile:path];
    
    if (CoreDoCommand(M64CMD_ROM_OPEN, [romData length], (void *)[romData bytes]) != M64ERR_SUCCESS)
        return NO;
    
    m64p_dynlib_handle core_handle = dlopen_myself();
    
    void (^LoadPlugin)(m64p_plugin_type, NSString *) = ^(m64p_plugin_type pluginType, NSString *pluginName){
        m64p_dynlib_handle rsp_handle;
        NSString *rspPath = [[coreBundle builtInPlugInsPath] stringByAppendingPathComponent:pluginName];
        
        osal_dynlib_open(&rsp_handle, [rspPath fileSystemRepresentation]);
        ptr_PluginStartup rsp_start = osal_dynlib_getproc(rsp_handle, "PluginStartup");
        rsp_start(core_handle, (__bridge void *)self, MupenDebugCallback);
        CoreAttachPlugin(pluginType, rsp_handle);
    };
    
    getKeys = GetKeys;
    initiateControllers = InitiateControllers;
    // Load Video
    LoadPlugin(M64PLUGIN_GFX, @"mupen64plus-video-rice.so");
    plugin_start(M64PLUGIN_INPUT);
    // Load Audio
    // Load Input
    // Load RSP
    LoadPlugin(M64PLUGIN_RSP, @"mupen64plus-rsp-hle.so");
    
    return YES;
}

- (void)startEmulation
{
    if(!isRunning)
    {
        [super startEmulation];
        [NSThread detachNewThreadSelector:@selector(mupenEmuThread) toTarget:self withObject:nil];
    }
}

- (void)mupenEmuThread
{
    @autoreleasepool
    {
        [self.renderDelegate willRenderOnAlternateThread];
        CoreDoCommand(M64CMD_EXECUTE, 0, NULL);
    }
}

- (void)executeFrameSkippingFrame:(BOOL)skip
{
    dispatch_semaphore_signal(gMupenWaitForVISemaphore);
    
    dispatch_semaphore_wait(gCoreWaitForFinishSemaphore, DISPATCH_TIME_FOREVER);
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

- (BOOL)saveStateToFileAtPath:(NSString *)fileName
{
    // freeze save
    //FIXME how to fit into emu event loop?
    //CoreDoCommand(M64CMD_STATE_SAVE, 1, (void*)[fileName UTF8String]);
    return YES;
}

- (BOOL)loadStateFromFileAtPath:(NSString *)fileName
{
    // freeze load
    //FIXME how to fit into emu event loop?
    //CoreDoCommand(M64CMD_STATE_LOAD, 1, (void*)[fileName UTF8String]);
    return YES;
}

- (OEIntSize)bufferSize
{
    return OESizeMake(gMupenVideoSettings.width, gMupenVideoSettings.height);
}

- (BOOL)rendersToOpenGL
{
    return YES;
}

- (const void *)videoBuffer
{
    if(black == NULL) black = calloc(1, 640 * 480 * 2);

    return black;
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

- (const void *)soundBuffer
{
    return NULL;
}

- (NSUInteger)audioBufferCount
{
    return 0;
}

- (NSUInteger)channelCount
{
    return 0;
}

- (double)audioSampleRate
{
    return 0;
}

- (oneway void)didPushN64Button:(OEN64Button)button forPlayer:(NSUInteger)player
{
    padData[button] = 1;
}

- (oneway void)didReleaseN64Button:(OEN64Button)button forPlayer:(NSUInteger)player
{
    padData[button] = 0;
}


@end
