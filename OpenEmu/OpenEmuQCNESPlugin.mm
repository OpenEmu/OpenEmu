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

/* It's highly recommended to use CGL macros instead of changing the current context for plug-ins that perform OpenGL rendering */
#import <OpenGL/CGLMacro.h>
#import "OpenEmuQCNESPlugin.h"
#import <Quartz/Quartz.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import "GameAudio.h"
#import "GameCore.h"
#import "OECorePlugin.h"

#define    kQCPlugIn_Name                @"OpenEmu NES"
#define    kQCPlugIn_Description        @"Wraps the OpenEmu emulator - play and manipulate the NES"

static void _TextureReleaseCallback(CGLContextObj cgl_ctx, GLuint name, void* info)
{    
    //    glDeleteTextures(1, &name);
}

static void _BufferReleaseCallback(const void* address, void* info)
{
    DLog(@"called buffer release callback");
    //    free((void*)address);
}

static GLint createNewTexture(CGLContextObj context, GLenum internalPixelFormat, NSUInteger pixelsWide, NSUInteger pixelsHigh, GLenum pixelFormat, GLenum pixelType, const void *pixelBuffer)
{
    GLenum status;
    GLuint gameTexture;
    
    CGLContextObj cgl_ctx = context;
    CGLLockContext(cgl_ctx);
    
    glEnable(GL_TEXTURE_RECTANGLE_EXT);    
    // create our texture 
    glGenTextures(1, &gameTexture);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, gameTexture);
    
    status = glGetError();
    if(status)
    {
        NSLog(@"createNewTexture, after bindTex: OpenGL error %04X", status);
    }
    
    // with storage hints & texture range -- assuming image depth should be 32 (8 bit rgba + 8 bit alpha ?) 
    glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE_EXT,  pixelsWide * pixelsHigh * (32 >> 3), pixelBuffer); 
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_CACHED_APPLE);
    glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
    
    // proper tex params.
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    
    glTexImage2D( GL_TEXTURE_RECTANGLE_EXT, 0, internalPixelFormat, pixelsWide, pixelsHigh, 0, pixelFormat, pixelType, pixelBuffer);
    
    status = glGetError();
    if(status)
    {
        NSLog(@"createNewTexture, after creating tex: OpenGL error %04X", status);
        glDeleteTextures(1, &gameTexture);
        gameTexture = 0;
    }
    
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_PRIVATE_APPLE);
    glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
    
    CGLUnlockContext(cgl_ctx);
    return gameTexture;
}

#pragma mark NES-specific features
@interface GameCore (NesAdditions)



- (BOOL)isUnlimitedSpritesEnabled;
- (int)brightness;
- (int)saturation;
- (int)contrast;
- (int)sharpness;
- (int)colorRes;
- (int)colorBleed;
- (int)colorArtifacts;
- (int)colorFringing;
- (int)hue;


- (void) applyNTSC: (id) sender;
- (BOOL) isNTSCEnabled;

- (void) toggleUnlimitedSprites: (id) sender;
- (void) enableUnlimitedSprites: (BOOL) enable;
- (void) setCode:(NSString*) code;
- (void) enableRewinder:(BOOL) rewind;
- (BOOL) isRewinderEnabled;
- (void) rewinderDirection: (NSUInteger) rewinderDirection;
- (void) enableRewinderBackwardsSound: (BOOL) rewindSound;
- (BOOL) isRewinderBackwardsSoundEnabled;


- (int) cartVRamSize;
- (int) chrRomSize;
//- (void) setRandomNmtRamByte;
//- (void) setRandomChrRamByte;

- (void)setNMTRamByTable:(NSNumber*)table array:(NSArray*)nmtValueArray;
- (void) setNmtRamBytes:(double)off value:(double)val;
- (void) setChrRamBytes:(double)off value:(double)val;

//movie methods
- (void) recordMovie:(NSString*) moviePath mode:(BOOL)append;
- (void) playMovie:(NSString*) moviePath;
- (void) stopMovie;
- (BOOL) isMovieRecording;
- (BOOL) isMoviePlaying;
- (BOOL) isMovieStopped;

@end



@implementation OpenEmuQCNES

/*
 Here you need to declare the input / output properties as dynamic as Quartz Composer will handle their implementation
 @dynamic inputFoo, outputBar;
 */
@dynamic inputRom;
@dynamic inputControllerData;
@dynamic inputVolume;
@dynamic inputSaveStatePath;
@dynamic inputLoadStatePath;
@dynamic inputPauseEmulation;
@dynamic inputCheatCode;
@dynamic inputEnableRewinder;
@dynamic inputRewinderDirection;
@dynamic inputEnableRewinderBackwardsSound;
// @dynamic inputRewinderReset;

@dynamic inputNmtRamCorrupt;
@dynamic inputNmtRamOffset;
@dynamic inputNmtRamValue;

@dynamic inputCorruptNameTable;
@dynamic inputNameTableData;

@dynamic inputChrRamCorrupt;
@dynamic inputChrRamOffset;
@dynamic inputChrRamValue;

@dynamic outputImage;

@synthesize loadedRom, romFinishedLoading, userPaused;

+ (NSDictionary*) attributes
{
    /*
     Return a dictionary of attributes describing the plug-in (QCPlugInAttributeNameKey, QCPlugInAttributeDescriptionKey...).
     */
    
    return [NSDictionary dictionaryWithObjectsAndKeys:kQCPlugIn_Name, QCPlugInAttributeNameKey, kQCPlugIn_Description, QCPlugInAttributeDescriptionKey, nil];
}

+ (NSDictionary*) attributesForPropertyPortWithKey:(NSString*)key
{
    /*
     Specify the optional attributes for property based ports (QCPortAttributeNameKey, QCPortAttributeDefaultValueKey...).
     */
    if([key isEqualToString:@"inputRom"]) 
        return [NSDictionary dictionaryWithObjectsAndKeys:    @"ROM Path", QCPortAttributeNameKey, 
                @"~/roms/NES/RomName.nes", QCPortAttributeDefaultValueKey, 
                nil]; 
    
    if([key isEqualToString:@"inputVolume"]) 
        return [NSDictionary dictionaryWithObjectsAndKeys:    @"Volume", QCPortAttributeNameKey, 
                [NSNumber numberWithFloat:0.5], QCPortAttributeDefaultValueKey, 
                [NSNumber numberWithFloat:1.0], QCPortAttributeMaximumValueKey,
                [NSNumber numberWithFloat:0.0], QCPortAttributeMinimumValueKey,
                nil]; 
    
    // NSArray with player count in index 0, index 1 is eButton "struct" (see GameButtons.h for typedef)
    if([key isEqualToString:@"inputControllerData"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Controller Data", QCPortAttributeNameKey, nil];
    
    if([key isEqualToString:@"inputSaveStatePath"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Save State", QCPortAttributeNameKey,
                @"~/roms/saves/savefilename", QCPortAttributeDefaultValueKey, 
                nil];
    
    if([key isEqualToString:@"inputLoadStatePath"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Load State", QCPortAttributeNameKey,
                @"~/roms/saves/loadsavefilename", QCPortAttributeDefaultValueKey, 
                nil];
    
    if([key isEqualToString:@"inputPauseEmulation"])
        return [NSDictionary dictionaryWithObjectsAndKeys:    @"Pause Emulator", QCPortAttributeNameKey,
                [NSNumber numberWithBool:NO], QCPortAttributeDefaultValueKey, 
                nil];
    
    if([key isEqualToString:@"inputCheatCode"])
        return [NSDictionary dictionaryWithObjectsAndKeys:    @"Cheat Code", QCPortAttributeNameKey,
                @"", QCPortAttributeDefaultValueKey, 
                nil];
    
    if([key isEqualToString:@"inputEnableRewinder"])
        return [NSDictionary dictionaryWithObjectsAndKeys:    @"Enable Rewinder", QCPortAttributeNameKey,
                [NSNumber numberWithUnsignedInteger:0], QCPortAttributeDefaultValueKey, 
                nil];
    
    if([key isEqualToString:@"inputRewinderDirection"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Rewinder Direction",QCPortAttributeNameKey,
                [NSArray arrayWithObjects:@"Backwards", @"Frontwards",nil], QCPortAttributeMenuItemsKey,
                [NSNumber numberWithUnsignedInteger:1], QCPortAttributeDefaultValueKey,
                [NSNumber numberWithUnsignedInteger:1], QCPortAttributeMaximumValueKey,
                nil];
    
    if([key isEqualToString:@"inputEnableRewinderBackwardsSound"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Enable Backwards Sound", QCPortAttributeNameKey,
                [NSNumber numberWithBool:NO], QCPortAttributeDefaultValueKey, 
                nil];
    
    //    if([key isEqualToString:@"inputRewinderReset"])
    //        return [NSDictionary dictionaryWithObjectsAndKeys:@"Rewinder Reset", QCPortAttributeNameKey,
    //                [NSNumber numberWithBool:NO], QCPortAttributeDefaultValueKey, 
    //                nil];
    
    if([key isEqualToString:@"inputNmtRamCorrupt"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Corrupt NMT RAM", QCPortAttributeNameKey,
                [NSNumber numberWithBool:NO], QCPortAttributeDefaultValueKey, 
                nil];
    
    if([key isEqualToString:@"inputNmtRamOffset"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"NMT RAM Offset",QCPortAttributeNameKey,
                [NSNumber numberWithUnsignedInteger:0], QCPortAttributeMinimumValueKey,
                [NSNumber numberWithUnsignedInteger:0], QCPortAttributeDefaultValueKey,
                [NSNumber numberWithUnsignedInteger:1], QCPortAttributeMaximumValueKey,
                nil];
    
    if([key isEqualToString:@"inputNmtRamValue"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"NMT RAM Value",QCPortAttributeNameKey,
                [NSNumber numberWithUnsignedInteger:0], QCPortAttributeMinimumValueKey,
                [NSNumber numberWithUnsignedInteger:0], QCPortAttributeDefaultValueKey,
                [NSNumber numberWithUnsignedInteger:1], QCPortAttributeMaximumValueKey,
                nil];
    
    if([key isEqualToString:@"inputCorruptNameTable"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Corrupt Name Table", QCPortAttributeNameKey,
                [NSNumber numberWithBool:NO], QCPortAttributeDefaultValueKey,
                nil];
    
    if([key isEqualToString:@"inputNameTableData"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Name Table Data", QCPortAttributeNameKey, nil];
    
    if([key isEqualToString:@"inputChrRamCorrupt"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Corrupt Character RAM", QCPortAttributeNameKey,
                [NSNumber numberWithBool:NO], QCPortAttributeDefaultValueKey, 
                nil];
    
    if([key isEqualToString:@"inputChrRamOffset"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Character RAM Offset",QCPortAttributeNameKey,
                [NSNumber numberWithUnsignedInteger:0], QCPortAttributeMinimumValueKey,
                [NSNumber numberWithUnsignedInteger:0], QCPortAttributeDefaultValueKey,
                [NSNumber numberWithUnsignedInteger:1], QCPortAttributeMaximumValueKey,
                nil];
    
    if([key isEqualToString:@"inputChrRamValue"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Character RAM Value",QCPortAttributeNameKey,
                [NSNumber numberWithUnsignedInteger:0], QCPortAttributeMinimumValueKey,
                [NSNumber numberWithUnsignedInteger:0], QCPortAttributeDefaultValueKey,
                [NSNumber numberWithUnsignedInteger:1], QCPortAttributeMaximumValueKey,
                nil];
    
    if([key isEqualToString:@"outputImage"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Image", QCPortAttributeNameKey, nil];
    
    return nil;
}

+ (NSArray*) sortedPropertyPortKeys
{
    return [NSArray arrayWithObjects:@"inputRom", 
            @"inputControllerData", 
            @"inputVolume", 
            @"inputPauseEmulation",
            @"inputSaveStatePath", 
            @"inputLoadStatePath", 
            @"inputCheatCode", 
            @"inputEnableRewinder",
            @"inputEnableRewinderBackwardsSound",
            @"inputRewinderDirection",
            //            @"inputRewinderReset",
            @"inputNmtRamCorrupt",
            @"inputNmtRamOffset",
            @"inputNmtRamValue",
            @"inputChrRamCorrupt",
            @"inputChrRamOffset",
            @"inputChrRamValue",
            @"inputCorruptNameTable",
            @"inputNameTableData",
            nil]; 
}


+ (QCPlugInExecutionMode) executionMode
{
    /*
     Return the execution mode of the plug-in: kQCPlugInExecutionModeProvider, kQCPlugInExecutionModeProcessor, or kQCPlugInExecutionModeConsumer.
     */
    
    return kQCPlugInExecutionModeProvider;
}

+ (QCPlugInTimeMode) timeMode
{
    /*
     Return the time dependency mode of the plug-in: kQCPlugInTimeModeNone, kQCPlugInTimeModeIdle or kQCPlugInTimeModeTimeBase.
     */
    
    return kQCPlugInTimeModeIdle;
}

- (id) init
{
    if(self = [super init])
    {
        gameLock = [[NSRecursiveLock alloc] init];
        persistantControllerData = [[NSMutableArray alloc] init];
        [persistantControllerData retain];
        
        persistantNameTableData = [[NSMutableArray alloc] init];
        [persistantNameTableData retain];
        
        //FIXME: maybe just get Nestopia
        plugins = [[OECorePlugin allPlugins] retain];
        validExtensions = [[OECorePlugin supportedTypeExtensions] retain];
        
        self.userPaused = NO; self.loadedRom = NO; self.romFinishedLoading = NO;
    }
    
    return self;
}

- (void) finalize
{
    /* Destroy variables intialized in init and not released by GC */
    [super finalize];
}

- (void) dealloc
{
    /* Release any resources created in -init. */
    [plugins release];
    [validExtensions release];
    [persistantControllerData release];
    [persistantNameTableData release];
    [gameLock release];
    [super dealloc];
}

+ (NSArray*) plugInKeys
{
    /*
     Return a list of the KVC keys corresponding to the internal settings of the plug-in.
     */
    
    return nil;
}

- (id) serializedValueForKey:(NSString*)key;
{
    /*
     Provide custom serialization for the plug-in internal settings that are not values complying to the <NSCoding> protocol.
     The return object must be nil or a PList compatible i.e. NSString, NSNumber, NSDate, NSData, NSArray or NSDictionary.
     */
    
    return [super serializedValueForKey:key];
}

- (void) setSerializedValue:(id)serializedValue forKey:(NSString*)key
{
    /*
     Provide deserialization for the plug-in internal settings that were custom serialized in -serializedValueForKey.
     Deserialize the value, then call [self setValue:value forKey:key] to set the corresponding internal setting of the plug-in instance to that deserialized value.
     */
    
    [super setSerializedValue:serializedValue forKey:key];
}

@end

@implementation OpenEmuQCNES (Execution)


- (BOOL) startExecution:(id<QCPlugInContext>)context
{    
    DLog(@"called startExecution");
    
    CGLContextObj cgl_ctx = [context CGLContextObj];
    CGLLockContext(cgl_ctx);
    
    if(self.loadedRom && ([gameCore    bufferWidth] > 10) )
    {
        if(gameTexture == 0)
            gameTexture = createNewTexture(cgl_ctx, 
                                           [gameCore internalPixelFormat], 
                                           [gameCore bufferWidth], 
                                           [gameCore bufferHeight], 
                                           [gameCore pixelFormat], 
                                           [gameCore pixelType],
                                           [gameCore videoBuffer]);
        
        if(!self.userPaused)
        {
            [gameCore setPauseEmulation:NO];
            [gameAudio startAudio];
        }
    }
    
    CGLUnlockContext(cgl_ctx);
    return YES;
}

- (void)enableExecution:(id<QCPlugInContext>)context
{
    CGLContextObj cgl_ctx = [context CGLContextObj];
    CGLLockContext(cgl_ctx);
    /*
     Called by Quartz Composer when the plug-in instance starts being used by Quartz Composer.
     */
    DLog(@"called enableExecution");
    // if we have a ROM loaded and the patch's image output is reconnected
    if(self.loadedRom) // FIXME: && self.romFinishedLoading ?
    {        
        if(gameTexture == 0)
            gameTexture = createNewTexture(cgl_ctx, 
                                           [gameCore internalPixelFormat], 
                                           [gameCore bufferWidth], 
                                           [gameCore bufferHeight], 
                                           [gameCore pixelFormat], 
                                           [gameCore pixelType],
                                           [gameCore videoBuffer]);
        
        // if emulation hasn't been paused by the user, restart it
        if(!self.userPaused) 
        {
            @try
            {
                [gameCore setPauseEmulation:NO];
                [gameAudio startAudio];
            }
            @catch (NSException * e) {
                NSLog(@"Failed to unpause");
            }
        }
    }
    CGLUnlockContext(cgl_ctx);
}

- (BOOL) execute:(id<QCPlugInContext>)context atTime:(NSTimeInterval)time withArguments:(NSDictionary*)arguments
{
    /*
     Called by Quartz Composer whenever the plug-in instance needs to execute.
     Only read from the plug-in inputs and produce a result (by writing to the plug-in outputs or rendering to the destination OpenGL context) within that method and nowhere else.
     Return NO in case of failure during the execution (this will prevent rendering of the current frame to complete).
     
     The OpenGL context for rendering can be accessed and defined for CGL macros using:
     CGLContextObj cgl_ctx = [context CGLContextObj];
     */
    
    CGLContextObj cgl_ctx = [context CGLContextObj];
    CGLLockContext(cgl_ctx);
    
    // Process ROM loads
    if([self didValueForInputKeyChange: @"inputRom"] 
       && ([self valueForInputKey:@"inputRom"] != [[OpenEmuQCNES    attributesForPropertyPortWithKey:@"inputRom"] valueForKey: QCPortAttributeDefaultValueKey])
       && (![[self valueForInputKey:@"inputRom"] isEqualToString:@""] ))
    {
        if([self loadRom:[self valueForInputKey:@"inputRom"]]) 
        {
            [gameAudio setVolume:[[self valueForInputKey:@"inputVolume"] floatValue]];
            glDeleteTextures(1, &gameTexture);
            gameTexture = createNewTexture(cgl_ctx, [gameCore internalPixelFormat], [gameCore bufferWidth], [gameCore bufferHeight], [gameCore pixelFormat], [gameCore pixelType], [gameCore videoBuffer]);
        }
    }
    
    if(self.loadedRom && self.romFinishedLoading) {
        // Process controller data
        if([self didValueForInputKeyChange: @"inputControllerData"])
        {
            // hold on to the controller data, which we are going to feed gameCore every frame.  Mmmmm...controller data.
            if([self controllerDataValidate:[self inputControllerData]])
            {
                persistantControllerData = [NSMutableArray arrayWithArray:[self inputControllerData]]; 
                [persistantControllerData retain];
                
                [self handleControllerData];
            }
        }    
        
        // Process audio volume changes
        if([self didValueForInputKeyChange: @"inputVolume"] && ([self valueForInputKey:@"inputVolume"] != [[OpenEmuQCNES attributesForPropertyPortWithKey:@"inputVolume"] valueForKey: QCPortAttributeDefaultValueKey]))
        {
            [gameAudio setVolume:[[self valueForInputKey:@"inputVolume"] floatValue]];
        }
        
        // Process state saving 
        if([self didValueForInputKeyChange: @"inputSaveStatePath"]
           && ([self valueForInputKey:@"inputSaveStatePath"] != [[OpenEmuQCNES attributesForPropertyPortWithKey:@"inputSaveStatePath"]
                                                                 valueForKey: QCPortAttributeDefaultValueKey])
           && (![[self valueForInputKey:@"inputSaveStatePath"] isEqualToString:@""] ))
        {
            DLog(@"save path changed");
            [self saveState:[[self valueForInputKey:@"inputSaveStatePath"] stringByStandardizingPath]];
        }
        
        // Process state loading
        if([self didValueForInputKeyChange:@"inputLoadStatePath"] 
           && ([self valueForInputKey:@"inputLoadStatePath"] != [[OpenEmuQCNES attributesForPropertyPortWithKey:@"inputLoadStatePath"]
                                                                 valueForKey: QCPortAttributeDefaultValueKey])
           && (![[self valueForInputKey:@"inputLoadStatePath"] isEqualToString:@""] ))    
        {
            DLog(@"load path changed");
            [self loadState:[[self valueForInputKey:@"inputLoadStatePath"] stringByStandardizingPath]];
        }
        
        // Process emulation pausing 
        if([self didValueForInputKeyChange: @"inputPauseEmulation"])    
        {
            if(self.inputPauseEmulation)    
            {
                DLog(@"user paused emulation");
                [gameAudio pauseAudio];
                [gameCore setPauseEmulation:YES]; 
                self.userPaused = YES;
            }
            else 
            {
                DLog(@"user unpaused emulation");
                [gameAudio startAudio];
                [gameCore setPauseEmulation:NO];
                self.userPaused = NO;
            }
        }
        
        // Process cheat codes
        if([self didValueForInputKeyChange: @"inputCheatCode"] && ([self valueForInputKey:@"inputCheatCode"] != [[OpenEmuQCNES attributesForPropertyPortWithKey:@"inputCheatCode"] valueForKey: QCPortAttributeDefaultValueKey]))    
        {
            DLog(@"cheat code entered");
            [gameCore setCode:[self valueForInputKey:@"inputCheatCode"]];
        }
        
#pragma mark process rewinder stuff
        if([self didValueForInputKeyChange: @"inputEnableRewinder"])    
        {
            [gameCore enableRewinder:[[self valueForInputKey:@"inputEnableRewinder"] boolValue]];
            
            if([gameCore isRewinderEnabled]) 
            {
                DLog(@"rewinder is enabled");
            } else 
            { 
                DLog(@"rewinder is disabled");
            }
        }
        
        //    int* rewindTimer;
        //    rewindTimer = [[NSNumber alloc] initWithUnsignedInteger:0];
        //    
        //    if([(NESGameEmu*)gameCore isRewinderEnabled]) 
        //    {
        //        rewindTimer++;
        //        if((rewindTimer % 60) == 0) {
        //        DLog(@"rewind timer count is %d",rewindTimer);
        //        }
        //    } 
        
        if([self didValueForInputKeyChange: @"inputRewinderDirection"])    
        {
            //        DLog(@"rewinder direction changed");
            [gameCore rewinderDirection:[[self valueForInputKey:@"inputRewinderDirection"] boolValue]];
        }
        
        if([self didValueForInputKeyChange:@"inputEnableRewinderBackwardsSound"])
        {
            [gameCore enableRewinderBackwardsSound:[[self valueForInputKey:@"inputEnableRewinderBackwardsSound"] boolValue]];
            
            if([gameCore isRewinderBackwardsSoundEnabled])
            {
                DLog(@"rewinder backwards sound is enabled");
            }
            else 
            {
                DLog(@"rewinder backwards sound is disabled");
            }
        }
        
#pragma mark glitch methods (CORRUPTION FTW)
        
        if(executedFrame && hasNmtRam && self.inputNmtRamCorrupt && ( [self didValueForInputKeyChange:@"inputNmtRamOffset"] || [self didValueForInputKeyChange:@"inputNmtRamValue"] ))
        {
            [gameCore setNmtRamBytes:self.inputNmtRamOffset value:self.inputNmtRamValue];
        }
        
        if(executedFrame && hasChrRom && self.inputChrRamCorrupt && ( [self didValueForInputKeyChange:@"inputChrRamOffset"] || [self didValueForInputKeyChange:@"inputChrRamValue"] ))
        {
            [gameCore setChrRamBytes:self.inputChrRamOffset value:self.inputChrRamValue];
        }
        
        if(executedFrame && hasNmtRam && self.inputCorruptNameTable && [self didValueForInputKeyChange:@"inputNameTableData"])
        {
            if([self validateNameTableData:[self inputNameTableData]])
            {
                persistantNameTableData = [NSMutableArray arrayWithArray:[self inputNameTableData]]; 
                [persistantNameTableData retain];
                [gameCore setNMTRamByTable:[persistantNameTableData objectAtIndex:0] array:[persistantNameTableData objectAtIndex:1]];
            }
        }
        
    } // END if(self.loadedRom && self.romFinishedLoading)
    
#pragma mark provide an image 
    
    // our output image
    id    provider = nil;
    
    // handle our image output. (sanity checking)
    if(self.loadedRom && self.romFinishedLoading && ([gameCore bufferWidth] > 10) && [gameCore frameFinished])
    {
        GLenum status;
        
        glEnable( GL_TEXTURE_RECTANGLE_EXT );
        glBindTexture( GL_TEXTURE_RECTANGLE_EXT, gameTexture);
        
        status = glGetError();
        if(status)
        {
            NSLog(@"after bindTexture in execute: OpenGL error %04X", status);
        }
        //new texture upload method
        glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, [gameCore bufferWidth], [gameCore bufferHeight], [gameCore pixelFormat], [gameCore pixelType], [gameCore videoBuffer]); 
        
        // Check for OpenGL errors 
        status = glGetError();
        if(status)
        {
            NSLog(@"after updating texture: OpenGL error %04X", status);
            glDeleteTextures(1, &gameTexture);
            gameTexture = 0;
        }
        
        glFlushRenderAPPLE();
#if __BIG_ENDIAN__
#define OEPlugInPixelFormat QCPlugInPixelFormatARGB8
#else
#define OEPlugInPixelFormat QCPlugInPixelFormatBGRA8
#endif
        
        provider = [context outputImageProviderFromTextureWithPixelFormat:OEPlugInPixelFormat 
                                                               pixelsWide:[gameCore sourceRect].size.width
                                                               pixelsHigh:[gameCore sourceRect].size.height
                                                                     name:gameTexture 
                                                                  flipped:YES 
                                                          releaseCallback:_TextureReleaseCallback 
                                                           releaseContext:NULL
                                                               colorSpace:CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB)
                                                         shouldColorMatch:YES];
        executedFrame = YES;
    }
    
    // output OpenEmu Texture - note we CAN output a nil image. 
    self.outputImage = provider;
    
    CGLUnlockContext(cgl_ctx);
    return YES;
}

- (void) disableExecution:(id<QCPlugInContext>)context
{
    /*
     Called by Quartz Composer when the plug-in instance stops being used by Quartz Composer.
     */
    
    DLog(@"called disableExecution");
    
    // if we have a ROM running and the patch's image output is disconnected, pause the emulator
    if(self.loadedRom && self.romFinishedLoading)
    {
        if(!self.userPaused) 
        {
            @try
            {
                [gameCore setPauseEmulation:YES]; 
                [gameAudio pauseAudio];
            }
            @catch (NSException * e) {
                NSLog(@"Failed to pause");
                self.userPaused = NO;
            }
        }          
    }
}

- (void) stopExecution:(id<QCPlugInContext>)context
{
    DLog(@"called stopExecution");
    if(self.loadedRom)
    {
        [gameCore stopEmulation];         
        [gameAudio stopAudio];
        [gameAudio release];
        [gameCore release];
        gameCore = nil;
        self.loadedRom = NO; self.romFinishedLoading = NO; executedFrame = NO;
        
        CGLContextObj cgl_ctx = [context CGLContextObj];
        CGLLockContext(cgl_ctx);
        glDeleteTextures(1, &gameTexture);
        gameTexture = 0;
        CGLUnlockContext(cgl_ctx);
        
    }
}

# pragma mark -

- (BOOL)controllerDataValidate:(NSArray *)cData
{
    // sanity check
    if([cData count] == 2 && [[cData objectAtIndex:1] count] == 30)
    {
        //NSLog(@"validated controller data");
        return YES;
    }    
    NSLog(@"error: missing or invalid controller data structure.");
    return NO;
}


- (BOOL) loadRom:(NSString*) romPath
{
    NSString* theRomPath = [romPath stringByStandardizingPath];
    BOOL isDir;
    
    if(![theRomPath caseInsensitiveCompare:@"nes"])
    {
        NSLog(@"ROM is not .NES");
        return NO;
    }
    
    DLog(@"New ROM path is: %@",theRomPath);
    
    if([[NSFileManager defaultManager] fileExistsAtPath:theRomPath isDirectory:&isDir] && !isDir)
    {
        NSString * extension = [theRomPath pathExtension];
        DLog(@"extension is: %@", extension);
        
        // cleanup
        if(self.loadedRom && self.romFinishedLoading)
        {
            self.romFinishedLoading = NO;
            [gameAudio stopAudio];
            [gameCore stopEmulation];
            [gameCore release];
            [gameAudio release];
            
            DLog(@"released/cleaned up for new rom");
            
        }
        self.loadedRom = NO;
        hasChrRom = NO;
        hasNmtRam = NO;
        
        //load NES bundle
        OECorePlugin *plugin = [self pluginForType:extension];
        
        gameCoreController = [plugin controller];
        gameCore = [gameCoreController newGameCore];
        
        DLog(@"Loaded NES bundle. About to load rom...");
        
        if([gameCore loadFileAtPath:theRomPath])
        {
            DLog(@"Loaded new Rom: %@", theRomPath);
            [gameCore setupEmulation];
            
            // audio!
            gameAudio = [[GameAudio alloc] initWithCore:gameCore];
            DLog(@"initialized audio");
            
            // starts the threaded emulator timer
            [gameCore startEmulation];
            
            DLog(@"About to start audio");
            [gameAudio startAudio];
            
            DLog(@"finished loading/starting rom");            
            
            if([gameCore chrRomSize])
            {
                hasChrRom = YES;
                DLog(@"Reported Character ROM size is %i", [gameCore chrRomSize]);
            }
            else 
            {
                hasChrRom = NO;
                DLog(@"This game does not have Character ROM");
            }
            
            hasNmtRam = YES;    //because if the cartridge doesn't have VRAM, the PPU will just use its 2K RAM for the nametables
            DLog(@"Reported NMT RAM size is %i", [gameCore cartVRamSize]);
            
            self.loadedRom = YES;
            return self.romFinishedLoading = YES;
        }    
        else NSLog(@"ROM did not load.");
    }
    else NSLog(@"bad ROM path or filename");
    return NO;    
}


-(void) handleControllerData
{
    // iterate through our NSArray of controller data. We know the player, we know the structure.
    // pull it out, and hand it off to our gameCore
    
    // player number 
    NSNumber*  playerNumber = [persistantControllerData objectAtIndex:0];
    NSArray * controllerArray = [persistantControllerData objectAtIndex:1];
    
    NSUInteger i;
    for(i = 0; i < [controllerArray count]; i++)
    {
        if(i > 5 && i < 10)
            continue;
        //       NSLog(@"index is %u", i);
        if([[controllerArray objectAtIndex:i] boolValue] == TRUE) // down
        {
            //    NSLog(@"button %u is down", i);
            //    [gameCore buttonPressed:i forPlayer:[playerNumber intValue]];
            [gameCore player:[playerNumber intValue] didPressButton:(i + 1)];
        }        
        else if([[controllerArray objectAtIndex:i] boolValue] == FALSE) // up
        {
            //    NSLog(@"button %u is up", i);
            //    [gameCore buttonRelease:i forPlayer:[playerNumber intValue]];
            [gameCore player:[playerNumber intValue] didReleaseButton:(i + 1)];
        }
    } 
}

// callback for audio from plugin
- (void) refresh
{
    //    [gameAudio advanceBuffer];
}

- (OECorePlugin *)pluginForType:(NSString *)extension
{
    for(OECorePlugin *plugin in plugins)
        if([plugin supportsFileType:extension])
            return plugin;
    return nil;
}

- (void) saveState: (NSString *) fileName
{
    BOOL isDir;
    DLog(@"saveState filename is %@", fileName);
    
    NSString *filePath = [fileName stringByDeletingLastPathComponent];
    
    // if the extension isn't .sav, make it so
    if([[fileName pathExtension] caseInsensitiveCompare:@"sav"] != 0) 
    {
        fileName = [fileName stringByAppendingPathExtension:@"sav"];
    }
    
    // see if directory exists
    if([[NSFileManager defaultManager] fileExistsAtPath:filePath isDirectory:&isDir] && isDir)
    {
        // if so, save the state
        [gameCore saveStateToFileAtPath: fileName];
    } 
    else if (![[NSFileManager defaultManager] fileExistsAtPath:filePath])
    {
        // if not, bitch about it
        DLog(@"Save state directory does not exist");
    }
}

- (BOOL) loadState: (NSString *) fileName
{
    BOOL isDir;    
    DLog(@"loadState path is %@", fileName);
    
    if([[fileName pathExtension] caseInsensitiveCompare:@"sav"] != 0) 
    {
        NSLog(@"Saved state files must have the extension \".sav\" to be loaded.");
        return NO;
    }
    
    if([[NSFileManager defaultManager] fileExistsAtPath:fileName isDirectory:&isDir] && !isDir)
    {
        //DO NOT CONCERN YOURSELF WITH EFFICIENCY OR ELEGANCE AT THIS JUNCTURE, DANIEL MORGAN WINCKLER.
        
        //if no ROM has been loaded, don't load the state
        if(!self.loadedRom) {
            NSLog(@"no ROM loaded -- please load a ROM before loading a state");
            return NO;
        }
        else {
            [gameCore loadStateFromFileAtPath:fileName];
            DLog(@"loaded new state");
        }
    }
    else 
    {
        NSLog(@"loadState: bad path or filename");
        return NO;
    }
    return YES;
}

- (BOOL) validateNameTableData: (NSArray*) nameTableData
{
    if([nameTableData count] == 2 && ([[nameTableData objectAtIndex:0] intValue] <= 3)
       && [[nameTableData objectAtIndex:1] count] == 960)
    {
        //DLog(@"validated name table data");
        return YES;
    }
    NSLog(@"error: missing or invalid name table data structure.");
    return NO;
}

@end
