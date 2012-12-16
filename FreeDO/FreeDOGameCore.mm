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

#import "FreeDOGameCore.h"
#import <OpenEmuBase/OERingBuffer.h>
#import "OEThreeDOSystemResponderClient.h"
#import <OpenGL/gl.h>

#include "freedocore.h"
#include "libcue.h"
#include "cd.h"

#define TEMP_BUFFER_SIZE 5512

static uint32_t reverseBytes(uint32_t value)
{
    return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 |
    (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24;
}



@interface FreeDOGameCore () <OEThreeDOSystemResponderClient>


- (void)fdcCallbackKPrint:(void *)pv;

- (void)fdcCallbackReadROM:(void *)pv;

- (void)fdcCallbackReadNVRAM:(void *)pv;

- (void)fdcCallbackWriteNVRAM:(void *)pv;

- (void *)fdcCallbackSwapFrame:(void *)pv;

- (void)fdcCallbackFrameTrigger;

- (void)fdcCallbackRead2048:(void *)pv;

- (void)fdcCallbackArmSync:(intptr_t)t;

- (intptr_t)fdcCallbackGetDiscSize;

- (void)fdcCallbackOnSector:(intptr_t)t;

- (intptr_t)fdcCallbackGetPbusLength;

- (void *)fdcCallbackGetPbus;

- (void)fdcCallbackPushSample:(uintptr_t)t;
@end

FreeDOGameCore *current;
@implementation FreeDOGameCore {
    Byte* biosRom1Copy;
    Byte* biosRom2Copy;
    volatile unsigned int pbus[5];
    void *nvramCopy;
    VDLFrame *frame;
    
    NSFileHandle *isoStream;
    TrackMode isoMode;
    int sectorCount;
    int currentSector;
    BOOL isSwapFrameSignaled;
    NSMutableString* kprintBuffer;
    
    int fver1,fver2;
    
    
    uintptr_t sampleBuffer[TEMP_BUFFER_SIZE];
    uint sampleCurrent;
    
    
    unsigned char FIXED_CLUTR[32];
    unsigned char FIXED_CLUTG[32];
    unsigned char FIXED_CLUTB[32];

    

}



static void* fdcCallback(int procedure, void* data) {
    switch (procedure) {
        case ExternalFunction_EXT_KPRINT:
            [current fdcCallbackKPrint:data];
            break;
        case ExternalFunction_EXT_READ_ROMS:
            [current fdcCallbackReadROM:data];
            break;
        case ExternalFunction_EXT_READ_NVRAM:
            [current fdcCallbackReadNVRAM:data];
            break;
        case ExternalFunction_EXT_WRITE_NVRAM:
            [current fdcCallbackWriteNVRAM:data];
            break;
        case ExternalFunction_EXT_SWAPFRAME:
            return [current fdcCallbackSwapFrame:data];
        case ExternalFunction_EXT_PUSH_SAMPLE:
            [current fdcCallbackPushSample:(uintptr_t)data];
            break;
        case ExternalFunction_EXT_GET_PBUSLEN:
            return (void*)[current fdcCallbackGetPbusLength];
        case ExternalFunction_EXT_GETP_PBUSDATA:
            return [current fdcCallbackGetPbus];
        case ExternalFunction_EXT_FRAMETRIGGER_MT:
            [current fdcCallbackFrameTrigger];
            break;
        case ExternalFunction_EXT_READ2048:
            [current fdcCallbackRead2048:data];
            break;
        case ExternalFunction_EXT_GET_DISC_SIZE:
            return (void *)[current fdcCallbackGetDiscSize];
        case ExternalFunction_EXT_ON_SECTOR:
            [current fdcCallbackOnSector:(intptr_t)data];
            break;
        case ExternalFunction_EXT_ARM_SYNC:
            [current fdcCallbackArmSync:(intptr_t)data];
            break;
        default:
            break;
    }
    return (void*)0;
}



unsigned int _setBitTo(unsigned int storage, BOOL set, unsigned int bitmask) {
    if (set)
        return storage|bitmask;
    else
        return storage;
}

- (oneway void)button:(OEThreeDOButton)button forPlayer:(NSUInteger)player pushed:(BOOL) pushed
{
    int i;

    for(i=0;i<1;i++) {
        unsigned int joybin=0x8000;
        if (i==player-1)
          switch (button) {
            case OEThreeDOButtonA:
                joybin = _setBitTo(joybin, pushed, PbusButtonAddress_A);
                break;
            case OEThreeDOButtonB:
                joybin = _setBitTo(joybin, pushed, OEThreeDOButtonB);
                break;
            case OEThreeDOButtonC:
                joybin = _setBitTo(joybin, pushed, OEThreeDOButtonC);
                break;
            case OEThreeDOButtonX:
                joybin = _setBitTo(joybin, pushed, OEThreeDOButtonX);
                break;
            case OEThreeDOButtonP:
                joybin = _setBitTo(joybin, pushed, OEThreeDOButtonP);
                break;
            case OEThreeDOButtonLeft:
                joybin = _setBitTo(joybin, pushed, OEThreeDOButtonLeft);
                break;
            case OEThreeDOButtonRight:
                joybin = _setBitTo(joybin, pushed, OEThreeDOButtonRight);
                break;
            case OEThreeDOButtonUp:
                joybin = _setBitTo(joybin, pushed, OEThreeDOButtonUp);
                break;
            case OEThreeDOButtonDown:
                joybin = _setBitTo(joybin, pushed, OEThreeDOButtonDown);
                break;
            case OEThreeDOButtonTriggerLeft:
                joybin = _setBitTo(joybin, pushed, OEThreeDOButtonTriggerLeft);
                break;
            case OEThreeDOButtonTriggerRight:
                joybin = _setBitTo(joybin, pushed, OEThreeDOButtonTriggerRight);
                break;
                
            default:
                break;
        }
        if(i&1)pbus[i>>1]=(pbus[i>>1]&0xffff0000)|joybin;
        else pbus[i>>1]=(pbus[i>>1]&0xffff)|(joybin<<16);
    }
    for(;i<10;i++)
    {
        if(i&1)pbus[i>>1]=(pbus[i>>1]&0xffff0000)|0xffff;
        else pbus[i>>1]=(pbus[i>>1]&0xffff)|(0xffff<<16);
    }
}

- (oneway void)didReleaseThreeDOButton:(OEThreeDOButton)button forPlayer:(NSUInteger)player
{ 
    [self button:button forPlayer:player pushed:NO];
}

- (oneway void)didPushThreeDOButton:(OEThreeDOButton)button forPlayer:(NSUInteger)player
{ 
    [self button:button forPlayer:player pushed:YES];
}

- (id)init
{
	self = [super init];
    if(self != nil)
    {
       
        for(int j = 0; j < 32; j++)
        {
            FIXED_CLUTR[j] = (unsigned char)(((j & 0x1f) << 3) | ((j >> 2) & 7));
            FIXED_CLUTG[j] = FIXED_CLUTR[j];
            FIXED_CLUTB[j] = FIXED_CLUTR[j];
        }
    }
	
	current = self;
    
	return self;
}



- (void) initVideo {

    if(videoBuffer)
        free(videoBuffer);
    videoBuffer = (char*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * 4);
    frame = new VDLFrame;
    fver2=fver1=0;

}

-(void) initKPrintBuffer {
    kprintBuffer = [NSMutableString string];

}

- (void)initPBusAndNVRAM {
    memset((void*)pbus,0xff,PhysicalParameters_PBUS_DATA_MAX_SIZE);
    
    nvramCopy = malloc(65536/2);
    memset(nvramCopy,0,65536/2);
    memcpy(nvramCopy,nvramhead,sizeof(nvramhead));
    frame = (VDLFrame*)malloc(sizeof(VDLFrame));
    memset(frame, 0, sizeof(VDLFrame));
}
-(void) loadBIOSes {
    NSString *rom1Path = [[NSBundle bundleForClass:[self class]] pathForResource:@"bios1" ofType:@"bin"];
    NSData *data = [NSData dataWithContentsOfFile:rom1Path];
    NSUInteger len = [data length];
    assert(len==PhysicalParameters_ROM1_SIZE);
    biosRom1Copy = (Byte*)malloc(len);
    
    memcpy(biosRom1Copy, [data bytes], len);
    
    
    NSString *rom2Path = [[NSBundle bundleForClass:[self class]] pathForResource:@"bios2" ofType:@"bin"];
    data = [NSData dataWithContentsOfFile:rom2Path];
//    len = [data length];

//    assert(len==PhysicalParameters_ROM2_SIZE);
    biosRom2Copy = (Byte*)malloc(len);
    memset(biosRom2Copy,0,len);

    
//    memcpy(biosRom2Copy, [data bytes], len);
}

#pragma mark Exectuion

- (void)executeFrame
{
    [self executeFrameSkippingFrame:NO];
    
    
    
}

- (void)executeFrameSkippingFrame: (BOOL) skip
{
    [self fdcDoExecuteFrame:frame];
    
    
    
    
}

- (void) readSector:(uint) sectorNumber toBuffer:(uint8_t*) buffer {
    if (isoMode==MODE_MODE1_RAW) {
        [isoStream seekToFileOffset:2352 * sectorNumber + 0x10];
    } else {
        [isoStream seekToFileOffset:2048 * sectorNumber];

    }
    NSData *data = [isoStream readDataOfLength:2048];
    memcpy(buffer, [data bytes], 2048);

}

- (BOOL)loadFileAtPath: (NSString*) path {
    NSString* isoPath = [path stringByStandardizingPath];
    NSString* cuePath = [[isoPath stringByDeletingPathExtension] stringByAppendingPathExtension:@"cue"];
    NSError* error;
    
    NSString* cue = [NSString stringWithContentsOfFile:cuePath encoding:NSUTF8StringEncoding error:&error];
    if (error){
        //guess metadata
        return NO;  //fixme remove me
    }
    else {
        const char* cueCString = [cue UTF8String];
        Cd* cd = cue_parse_string(cueCString);
        NSLog(@"CUE file found and parsed");
        if (cd_get_ntrack(cd)!=1){
            NSLog(@"Cue file found, but the number of tracks within was not 1.");
            return NO;
        }
        Track* track = cd_get_track(cd, 1);
        isoMode = (TrackMode)track_get_mode(track);
    //    bool trackProperFlag = (bool)track_is_set_flag(track, FLAG_DATA);
        //todo implement BINARY checking
        if ((isoMode!=MODE_MODE1&&isoMode!=MODE_MODE1_RAW)) {
            NSLog(@"Cue file found, but the track within was not in the right format (should be BINARY and Mode1+2048 or Mode1+2352)");
            return NO;
        }
    }
    isoStream = [NSFileHandle fileHandleForReadingAtPath:isoPath];    
    
    uint8_t sectorZero[2048];
    [self readSector:0 toBuffer:sectorZero];
    VolumeHeader* header = (VolumeHeader*)sectorZero;
    sectorCount = (int)reverseBytes(header->blockCount);
    NSLog(@"Sector count is %d", sectorCount);
    
    return YES;
}

#pragma mark Video
- (const void *)videoBuffer
{
    
    if (isSwapFrameSignaled) {
        if(fver2==fver1)
        {
            isSwapFrameSignaled = NO;
            [self frame:frame toVideoBuffer:(Byte*)videoBuffer];
            fver1++;
        }
    }
    fver2=fver1;
    return videoBuffer;
}

- (OEIntRect)screenRect
{
    // hope this handles hires :/
    OEIntRect rect = OERectMake(0,0,320,240);
    return rect;
//    return OERectMake(0, 0, frame->srcw*2, frame->srch*2);
}

- (OEIntSize)bufferSize
{
    return OESizeMake(SCREEN_WIDTH, SCREEN_HEIGHT);
}

- (void)setupEmulation
{
    [self loadBIOSes];
    [self initPBusAndNVRAM];
    [self initKPrintBuffer];
    [self initVideo];
    currentSector = 0;

    sampleCurrent = 0;
    memset(sampleBuffer, 0, sizeof(uintptr_t)*TEMP_BUFFER_SIZE);
    
    

    
}

- (void)resetEmulation
{
    [self setupEmulation];
}
- (void) startEmulation {
    NSLog(@"FreeDO version: %d",[self fdcGetCoreVersion]);
    [self fdcInitialize];
    [super startEmulation];
}

- (void)stopEmulation
{
    [super stopEmulation];

    [self fdcDestroy];
}

- (void)dealloc
{
}

- (GLenum)pixelFormat
{
    return GL_RGB;
}

- (GLenum)pixelType
{
    return GL_UNSIGNED_BYTE;
}

- (GLenum)internalPixelFormat
{
    return GL_RGB;
}

- (double)audioSampleRate
{
    return sampleRate ? sampleRate : 44100;
}

- (NSTimeInterval)frameInterval
{
    return frameInterval ? frameInterval : 60;
}

- (NSUInteger)channelCount
{
    return 2;
}

- (BOOL)saveStateToFileAtPath:(NSString *)fileName
{   
    
    size_t size = [self fdcGetSaveSize];
    void *data = malloc(sizeof(uintptr_t)*size);
    [self fdcDoSave:data];
    NSData *saveData = [NSData dataWithBytesNoCopy:data length:size freeWhenDone:YES];
    NSLog(@"Game saved, length in bytes: %lu", saveData.length);
    return [saveData writeToFile:fileName atomically:NO];

}

- (BOOL)loadStateFromFileAtPath:(NSString *)fileName
{
    NSData *saveData = [NSData dataWithContentsOfFile:fileName];
    size_t size = sizeof(uintptr_t)*saveData.length;
    void* loadBuffer = malloc(size);
    [saveData getBytes:loadBuffer];
    return [self fdcDoLoad:loadBuffer];
}

#pragma mark - FreeDoInterface

-(void*) _freedoActionWithInterfaceFunction:(InterfaceFunction) interfaceFunction datum:(void*) datum {
    return _freedo_Interface(interfaceFunction, datum);
}

-(int) fdcGetCoreVersion {
    NSLog(@"fdcGetCoreVersion");
    return (uintptr_t)[self _freedoActionWithInterfaceFunction:InterfaceFunction_FDP_FREEDOCORE_VERSION datum:(void*)0];
}

-(int) fdcGetSaveSize {
    return (uintptr_t)[self _freedoActionWithInterfaceFunction:InterfaceFunction_FDP_GET_SAVE_SIZE datum:(void*)0];
}

-(void*) fdcGetPointerNVRAM {
    return [self _freedoActionWithInterfaceFunction:InterfaceFunction_FDP_GETP_NVRAM datum:(void*)0];
}

-(void*) fdcGetPointerRAM {
    return [self _freedoActionWithInterfaceFunction:InterfaceFunction_FDP_GETP_RAMS datum:(void*)0];
}

-(void*) fdcGetPointerROM {
    return [self _freedoActionWithInterfaceFunction:InterfaceFunction_FDP_GETP_ROMS datum:(void*)0];
}

-(void*) fdcGetPointerProfile {
    return [self _freedoActionWithInterfaceFunction:InterfaceFunction_FDP_GETP_PROFILE datum:(void*)0];
}

-(int) fdcInitialize {
    NSLog(@"fdcInit");
    return (intptr_t)[self _freedoActionWithInterfaceFunction:InterfaceFunction_FDP_INIT datum:(void*)*fdcCallback]; 
}

-(void) fdcDestroy {
    [self _freedoActionWithInterfaceFunction:InterfaceFunction_FDP_DESTROY datum:(void*)0];
}

-(void) fdcDoExecuteFrame:(void*)vdlFrame {
    [self _freedoActionWithInterfaceFunction:InterfaceFunction_FDP_DO_EXECFRAME datum:vdlFrame];
}

-(void) fdcDoExecuteFrameMultitask:(void*)vdlFrame {
    [self _freedoActionWithInterfaceFunction:InterfaceFunction_FDP_DO_EXECFRAME_MT datum:vdlFrame];
}

-(void) fdcDoFrameMultitask:(void*)vdlFrame {
    [self _freedoActionWithInterfaceFunction:InterfaceFunction_FDP_DO_FRAME_MT datum:vdlFrame];
}

-(BOOL) fdcDoLoad:(void*)buffer {
    return [self _freedoActionWithInterfaceFunction:InterfaceFunction_FDP_DO_LOAD datum:buffer]!=0;
}

-(void) fdcDoSave:(void*)buffer {
    [self _freedoActionWithInterfaceFunction:InterfaceFunction_FDP_DO_SAVE datum:buffer];
}


-(void*) fdcSetArmClock:(int)clock { //untested!
    return [self _freedoActionWithInterfaceFunction:InterfaceFunction_FDP_SET_ARMCLOCK datum:(void*) clock];
}

-(void*) fdcSetFixMode:(int)fixMode { 
    return [self _freedoActionWithInterfaceFunction:InterfaceFunction_FDP_SET_FIX_MODE datum:(void*) fixMode];
}


-(void*) fdcSetTextureQuality:(int)textureQuality { 
    return [self _freedoActionWithInterfaceFunction:InterfaceFunction_FDP_SET_TEXQUALITY datum:(void*) textureQuality];
}

- (void)fdcCallbackKPrint:(void *)pv
{
    NSString* currentChar = [NSString stringWithFormat:@"%c", pv];
    if ([currentChar isEqualToString:@"\n"]) {
        NSLog(@"KPRINT: %@", kprintBuffer);
        kprintBuffer = [NSMutableString string];
    } else {
        [kprintBuffer appendString:currentChar];
    }
    
}

- (void)fdcCallbackReadROM:(void *)pv
{
    NSLog(@"fdcCallbackReadROM");
    memcpy(pv, biosRom1Copy, PhysicalParameters_ROM1_SIZE);
    void *biosRom2Dest = (void*)((intptr_t)pv+PhysicalParameters_ROM2_SIZE);
    memcpy(biosRom2Dest, biosRom2Copy, PhysicalParameters_ROM2_SIZE);

}

- (void)fdcCallbackReadNVRAM:(void *)buffer
{
    NSLog(@"fdcCallbackReadNVRAM");
    memcpy(buffer, nvramCopy, PhysicalParameters_NVRAM_SIZE);

}

- (void)fdcCallbackWriteNVRAM:(void *)pv
{
    NSLog(@"fdcCallbackWriteNVRAM");

}

- (void *)fdcCallbackSwapFrame:(void *)pv
{
    isSwapFrameSignaled = YES;
    
    return frame;
}

- (void)fdcCallbackFrameTrigger
{
    NSLog(@"fdcCallbackFrameTrigger");
    isSwapFrameSignaled = YES;
    [self fdcDoFrameMultitask:frame];   

}

- (void)fdcCallbackRead2048:(void *)buffer
{
    NSLog(@"fdcCallbackRead2048");
    [self readSector:currentSector toBuffer:(uint8_t*)buffer];
}

- (void)fdcCallbackArmSync:(intptr_t)t
{
    NSLog(@"fdcCallbackArmSync");

}

- (intptr_t)fdcCallbackGetDiscSize
{
    NSLog(@"fdcCallbackGetDiscSize");
    return sectorCount;
}

- (void)fdcCallbackOnSector:(intptr_t)sector
{
    NSLog(@"fdcCallbackOnSector");
    currentSector = sector;

}

- (intptr_t)fdcCallbackGetPbusLength
{
    return PhysicalParameters_PBUS_DATA_MAX_SIZE;
}

- (void *)fdcCallbackGetPbus
{    
    int i;
    for (i = 0; i < PhysicalParameters_PBUS_DATA_MAX_SIZE; i++)
    {
        if (i > 0) printf(":");
        printf("%02X", pbus[i]);
    }
    printf("\n");
    
    return  (void*)pbus;
}

- (void)fdcCallbackPushSample:(uintptr_t)sample
{
    //fixme implement audio
    //NSLog(@"fdcCallbackPushSample");
    sampleBuffer[sampleCurrent] = sample;
    sampleCurrent ++;
    if (sampleCurrent>TEMP_BUFFER_SIZE) {
        sampleCurrent = 0;
        [[self ringBufferAtIndex:0] write:sampleBuffer maxLength:sizeof(uintptr_t)*TEMP_BUFFER_SIZE];
        memset(sampleBuffer, 0, sizeof(uintptr_t)*TEMP_BUFFER_SIZE);

    }

}







-(void) frame:(VDLFrame*) framePtr toVideoBuffer:(Byte*) destPtr {
    int copyHeight = framePtr->srch*2;
    int copyWidth = framePtr->srcw*2;
    for (int line = 0; line < copyHeight; line++)
    {
        VDLLine* linePtr = &framePtr->lines[line];
        short* srcPtr = (short*)linePtr;
        bool allowFixedClut = (linePtr->xOUTCONTROLL & 0x2000000) > 0;
        for (int pix = 0; pix < copyWidth; pix++)
        {
            Byte bPart = 0;
            Byte gPart = 0;
            Byte rPart = 0;
            if (*srcPtr == 0)
            {
                bPart = (Byte)(linePtr->xBACKGROUND & 0x1F);
                gPart = (Byte)((linePtr->xBACKGROUND >> 5) & 0x1F);
                rPart = (Byte)((linePtr->xBACKGROUND >> 10) & 0x1F);
            }
            else if (allowFixedClut && (*srcPtr & 0x8000) > 0)
            {
                bPart = FIXED_CLUTB[(*srcPtr) & 0x1F];
                gPart = FIXED_CLUTG[((*srcPtr) >> 5) & 0x1F];
                rPart = FIXED_CLUTR[(*srcPtr) >> 10 & 0x1F];
            }
            else
            {
                bPart = (Byte)(linePtr->xCLUTB[(*srcPtr) & 0x1F]);
                gPart = linePtr->xCLUTG[((*srcPtr) >> 5) & 0x1F];
                rPart = linePtr->xCLUTR[(*srcPtr) >> 10 & 0x1F];
            }
            *destPtr++ = bPart;
            *destPtr++ = gPart;
            *destPtr++ = rPart;
            
            srcPtr++;
        }
    }
    
    
}

@end
