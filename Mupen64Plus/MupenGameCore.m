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
#import "rom.h"
#import "osal/dynamiclib.h"
#import "version.h"

#import <OpenEmuBase/OERingBuffer.h>
#import <OpenGL/gl.h>

#import "plugin.h"

#import <dlfcn.h>

NSString *MupenControlNames[] = {
    @"N64_DPadU", @"N64_DPadD", @"N64_DPadL", @"N64_DPadR",
    @"N64_CU", @"N64_CD", @"N64_CL", @"N64_CR",
    @"N64_B", @"N64_A", @"N64_R", @"N64_L", @"N64_Z", @"N64_Start"
}; // FIXME: missing: joypad X, joypad Y, mempak switch, rumble switch

@interface MupenGameCore () <OEN64SystemResponderClient>
@end

MupenGameCore *g_core;

@implementation MupenGameCore
{
    NSData *romData;
    
    dispatch_semaphore_t mupenWaitToBeginFrameSemaphore;
    dispatch_semaphore_t coreWaitToEndFrameSemaphore;
}

- (instancetype)init
{
    if (self = [super init]) {
        mupenWaitToBeginFrameSemaphore = dispatch_semaphore_create(0);
        coreWaitToEndFrameSemaphore    = dispatch_semaphore_create(0);
        
        videoWidth  = 640;
        videoHeight = 480;
        videoBitDepth = 32; // ignored
        videoDepthBitDepth = 0; // TODO
        
        sampleRate = 33600;
        
        isNTSC = YES;
    }
    g_core = self;
    return self;
}

- (void)dealloc
{
    dispatch_release(mupenWaitToBeginFrameSemaphore);
    dispatch_release(coreWaitToEndFrameSemaphore);
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

static void MupenGetKeys(int Control, BUTTONS *Keys)
{
    Keys->R_DPAD = g_core->padData[Control][OEN64ButtonDPadRight];
    Keys->L_DPAD = g_core->padData[Control][OEN64ButtonDPadLeft];
    Keys->D_DPAD = g_core->padData[Control][OEN64ButtonDPadDown];
    Keys->U_DPAD = g_core->padData[Control][OEN64ButtonDPadUp];
    Keys->START_BUTTON = g_core->padData[Control][OEN64ButtonStart];
    Keys->Z_TRIG = g_core->padData[Control][OEN64ButtonZ];
    Keys->B_BUTTON = g_core->padData[Control][OEN64ButtonB];
    Keys->A_BUTTON = g_core->padData[Control][OEN64ButtonA];
    Keys->R_CBUTTON = g_core->padData[Control][OEN64ButtonCRight];
    Keys->L_CBUTTON = g_core->padData[Control][OEN64ButtonCLeft];
    Keys->D_CBUTTON = g_core->padData[Control][OEN64ButtonCDown];
    Keys->U_CBUTTON = g_core->padData[Control][OEN64ButtonCUp];
    Keys->R_TRIG = g_core->padData[Control][OEN64ButtonR];
    Keys->L_TRIG = g_core->padData[Control][OEN64ButtonL];
    Keys->X_AXIS = (g_core->xAxis[Control][0] ?: (g_core->xAxis[Control][1] ?: 0));
    Keys->Y_AXIS = (g_core->yAxis[Control][0] ?: (g_core->yAxis[Control][1] ?: 0));
}

static void MupenInitiateControllers (CONTROL_INFO ControlInfo)
{
    ControlInfo.Controls[0].Present = 1;
    ControlInfo.Controls[1].Present = 1;
    ControlInfo.Controls[2].Present = 1;
    ControlInfo.Controls[3].Present = 1;
}

static AUDIO_INFO AudioInfo;

static void MupenAudioSampleRateChanged(int SystemType)
{
    float currentRate = g_core->sampleRate;
    
    switch (SystemType)
    {
        default:
        case SYSTEM_NTSC:
            g_core->sampleRate = 48681812 / (*AudioInfo.AI_DACRATE_REG + 1);
            break;
        case SYSTEM_PAL:
            g_core->sampleRate = 49656530 / (*AudioInfo.AI_DACRATE_REG + 1);
            break;
    }

    [[g_core audioDelegate] audioSampleRateDidChange];
    NSLog(@"Mupen rate changed %f -> %f\n", currentRate, g_core->sampleRate);
}

static void MupenAudioLenChanged()
{
    int LenReg = *AudioInfo.AI_LEN_REG;
    uint8_t *ptr = (uint8_t*)(AudioInfo.RDRAM + (*AudioInfo.AI_DRAM_ADDR_REG & 0xFFFFFF));
    
    [[g_core ringBufferAtIndex:0] write:ptr maxLength:LenReg];
}

static void SetIsNTSC()
{
    extern rom_header *ROM_HEADER;
    switch (ROM_HEADER->Country_code&0xFF)
    {
        case 0x44:
        case 0x46:
        case 0x49:
        case 0x50:
        case 0x53:
        case 0x55:
        case 0x58:
        case 0x59:
            g_core->isNTSC = NO;
            break;
        case 0x37:
        case 0x41:
        case 0x45:
        case 0x4a:
            g_core->isNTSC = YES;
            break;
    }
}

static int MupenOpenAudio(AUDIO_INFO info)
{
    AudioInfo = info;
    
    SetIsNTSC();
    
    return M64ERR_SUCCESS;
}

static void MupenSetAudioSpeed(int percent)
{
    // do we need this?
}

- (BOOL)loadFileAtPath:(NSString *)path
{
    NSBundle *coreBundle = [NSBundle bundleForClass:[self class]];
    const char *dataPath;

    NSString *configPath = [[self supportDirectoryPath] stringByAppendingString:@"/"];
    dataPath = [[coreBundle resourcePath] fileSystemRepresentation];
    
    [[NSFileManager defaultManager] createDirectoryAtPath:configPath withIntermediateDirectories:YES attributes:nil error:nil];
    
    NSString *batterySavesDirectory = [self batterySavesDirectoryPath];
    [[NSFileManager defaultManager] createDirectoryAtPath:batterySavesDirectory withIntermediateDirectories:YES attributes:nil error:NULL];
    
    // open core here
    CoreStartup(FRONTEND_API_VERSION, [configPath fileSystemRepresentation], dataPath, (__bridge void *)self, MupenDebugCallback, (__bridge void *)self, MupenStateCallback);
    
    // set SRAM path
    m64p_handle config;
    ConfigOpenSection("Core", &config);
    ConfigSetParameter(config, "SaveSRAMPath", M64TYPE_STRING, [batterySavesDirectory UTF8String]);
    ConfigSaveSection("Core");

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
    
    // Load Video
    LoadPlugin(M64PLUGIN_GFX, @"mupen64plus-video-rice.so");
    // Load Audio
    aiDacrateChanged = MupenAudioSampleRateChanged;
    aiLenChanged = MupenAudioLenChanged;
    initiateAudio = MupenOpenAudio;
    setSpeedFactor = MupenSetAudioSpeed;
    plugin_start(M64PLUGIN_AUDIO);
    
    // Load Input
    getKeys = MupenGetKeys;
    initiateControllers = MupenInitiateControllers;
    plugin_start(M64PLUGIN_INPUT);
    // Load RSP
    LoadPlugin(M64PLUGIN_RSP, @"mupen64plus-rsp-hle.so");
    
    return YES;
}

- (void)startEmulation
{
    if(!isRunning)
    {
        [super startEmulation];
        [self.renderDelegate willRenderOnAlternateThread];
        [NSThread detachNewThreadSelector:@selector(mupenEmuThread) toTarget:self withObject:nil];
    }
}

- (void)mupenEmuThread
{
    @autoreleasepool
    {
        [self.renderDelegate startRenderingOnAlternateThread];
        CoreDoCommand(M64CMD_EXECUTE, 0, NULL);
    }
}

- (void)videoInterrupt
{
    dispatch_semaphore_signal(coreWaitToEndFrameSemaphore);
    
    [self.renderDelegate willRenderFrameOnAlternateThread];
    dispatch_semaphore_wait(mupenWaitToBeginFrameSemaphore, DISPATCH_TIME_FOREVER);
}

- (void)swapBuffers
{
    [self.renderDelegate didRenderFrameOnAlternateThread];
}

- (void)executeFrameSkippingFrame:(BOOL)skip
{
    dispatch_semaphore_signal(mupenWaitToBeginFrameSemaphore);
    
    dispatch_semaphore_wait(coreWaitToEndFrameSemaphore, DISPATCH_TIME_FOREVER);
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
    return OESizeMake(videoWidth, videoHeight);
}

- (BOOL)rendersToOpenGL
{
    return YES;
}

- (const void *)videoBuffer
{
    return NULL;
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

#pragma mark Mupen Audio

- (NSTimeInterval)frameInterval
{
    // Mupen uses 60 but it's probably wrong
    return isNTSC ? 60 : 50;
}

- (NSUInteger)channelCount
{
    return 2;
}

- (double)audioSampleRate
{
    return sampleRate;
}

- (oneway void)didMoveN64JoystickDirection:(OEN64Button)button withValue:(CGFloat)value forPlayer:(NSUInteger)player
{
    player -= 1;
    switch (button)
    {
        case OEN64AnalogUp:
            yAxis[player][0] = value * INT8_MAX;
            break;
        case OEN64AnalogDown:
            yAxis[player][1] = value * INT8_MIN;
            break;
        case OEN64AnalogLeft:
            xAxis[player][0] = value * INT8_MIN;
            break;
        case OEN64AnalogRight:
            xAxis[player][1] = value * INT8_MAX;
            break;
        default:
            break;
    }
}

- (oneway void)didPushN64Button:(OEN64Button)button forPlayer:(NSUInteger)player
{
    player -= 1;
    padData[player][button] = 1;
}

- (oneway void)didReleaseN64Button:(OEN64Button)button forPlayer:(NSUInteger)player
{
    player -= 1;
    padData[player][button] = 0;
}


@end
