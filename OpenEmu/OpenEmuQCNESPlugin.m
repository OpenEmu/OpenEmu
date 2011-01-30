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

#import "GameCore.h"
#import "OECorePlugin.h"

#import <IOSurface/IOSurface.h>
#import <OpenGL/CGLIOSurface.h>

static NSString *const kQCPlugInName        = @"OpenEmu NES";
static NSString *const kQCPlugInDescription = @"Wraps the OpenEmu emulator - play and manipulate the NES";

static void _TextureReleaseCallback(CGLContextObj cgl_ctx, GLuint name, void *info)
{
    glDeleteTextures(1, &name);
}

@implementation OpenEmuQCNES

@synthesize persistantControllerData;
@synthesize persistantNameTableData;
@synthesize debugMode;

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

@dynamic inputNmtRamCorrupt;
@dynamic inputNmtRamOffset;
@dynamic inputNmtRamValue;

@dynamic inputCorruptNameTable;
@dynamic inputNameTableData;

@dynamic inputChrRamCorrupt;
@dynamic inputChrRamOffset;
@dynamic inputChrRamValue;

@dynamic outputImage;

#ifdef DEBUG_PRINT
@dynamic inputEnableDebugMode;
#endif

+ (NSDictionary *)attributes
{
    return [NSDictionary dictionaryWithObjectsAndKeys:
            kQCPlugInName       , QCPlugInAttributeNameKey,
            kQCPlugInDescription, QCPlugInAttributeDescriptionKey,
            nil];
}

+ (NSDictionary *)attributesForPropertyPortWithKey:(NSString *)key
{
    if([key isEqualToString:@"inputRom"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"ROM Path"              , QCPortAttributeNameKey,
                @"~/roms/NES/RomName.nes", QCPortAttributeDefaultValueKey,
                nil];
    
    if([key isEqualToString:@"inputVolume"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Volume"                     , QCPortAttributeNameKey,
                [NSNumber numberWithFloat:0.5], QCPortAttributeDefaultValueKey,
                [NSNumber numberWithFloat:1.0], QCPortAttributeMaximumValueKey,
                [NSNumber numberWithFloat:0.0], QCPortAttributeMinimumValueKey,
                nil];
    
    // NSArray with player count in index 0, index 1 is eButton "struct" (see GameButtons.h for typedef)
    if([key isEqualToString:@"inputControllerData"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Controller Data", QCPortAttributeNameKey, nil];
    
    if([key isEqualToString:@"inputSaveStatePath"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Save State"               , QCPortAttributeNameKey,
                @"~/roms/saves/savefilename", QCPortAttributeDefaultValueKey,
                nil];
    
    if([key isEqualToString:@"inputLoadStatePath"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Load State"                   , QCPortAttributeNameKey,
                @"~/roms/saves/loadsavefilename", QCPortAttributeDefaultValueKey,
                nil];
    
    if([key isEqualToString:@"inputPauseEmulation"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Pause Emulator"           , QCPortAttributeNameKey,
                [NSNumber numberWithBool:NO], QCPortAttributeDefaultValueKey,
                nil];
    
    if([key isEqualToString:@"inputCheatCode"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Cheat Code", QCPortAttributeNameKey,
                @""          , QCPortAttributeDefaultValueKey,
                nil];
    
    if([key isEqualToString:@"inputEnableRewinder"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Enable Rewinder"                    , QCPortAttributeNameKey,
                [NSNumber numberWithUnsignedInteger:0], QCPortAttributeDefaultValueKey,
                nil];
    
    if([key isEqualToString:@"inputRewinderDirection"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Rewinder Direction"                                      , QCPortAttributeNameKey,
                [NSArray arrayWithObjects:@"Backwards", @"Frontwards", nil], QCPortAttributeMenuItemsKey,
                [NSNumber numberWithUnsignedInteger:1]                     , QCPortAttributeDefaultValueKey,
                [NSNumber numberWithUnsignedInteger:1]                     , QCPortAttributeMaximumValueKey,
                nil];
    
    if([key isEqualToString:@"inputEnableRewinderBackwardsSound"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Enable Backwards Sound"   , QCPortAttributeNameKey,
                [NSNumber numberWithBool:NO], QCPortAttributeDefaultValueKey,
                nil];
    
    //    if([key isEqualToString:@"inputRewinderReset"])
    //        return [NSDictionary dictionaryWithObjectsAndKeys:
    //                @"Rewinder Reset"           , QCPortAttributeNameKey,
    //                [NSNumber numberWithBool:NO], QCPortAttributeDefaultValueKey,
    //                nil];
    
    if([key isEqualToString:@"inputNmtRamCorrupt"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Corrupt NMT RAM"          , QCPortAttributeNameKey,
                [NSNumber numberWithBool:NO], QCPortAttributeDefaultValueKey,
                nil];
    
    if([key isEqualToString:@"inputNmtRamOffset"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"NMT RAM Offset"                     , QCPortAttributeNameKey,
                [NSNumber numberWithUnsignedInteger:0], QCPortAttributeMinimumValueKey,
                [NSNumber numberWithUnsignedInteger:0], QCPortAttributeDefaultValueKey,
                [NSNumber numberWithUnsignedInteger:1], QCPortAttributeMaximumValueKey,
                nil];
    
    if([key isEqualToString:@"inputNmtRamValue"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"NMT RAM Value"                      ,QCPortAttributeNameKey,
                [NSNumber numberWithUnsignedInteger:0], QCPortAttributeMinimumValueKey,
                [NSNumber numberWithUnsignedInteger:0], QCPortAttributeDefaultValueKey,
                [NSNumber numberWithUnsignedInteger:1], QCPortAttributeMaximumValueKey,
                nil];
    
    if([key isEqualToString:@"inputCorruptNameTable"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Corrupt Name Table"       , QCPortAttributeNameKey,
                [NSNumber numberWithBool:NO], QCPortAttributeDefaultValueKey,
                nil];
    
    if([key isEqualToString:@"inputNameTableData"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Name Table Data", QCPortAttributeNameKey, nil];
    
    if([key isEqualToString:@"inputChrRamCorrupt"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Corrupt Character RAM"    , QCPortAttributeNameKey,
                [NSNumber numberWithBool:NO], QCPortAttributeDefaultValueKey,
                nil];
    
    if([key isEqualToString:@"inputChrRamOffset"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Character RAM Offset"               , QCPortAttributeNameKey,
                [NSNumber numberWithUnsignedInteger:0], QCPortAttributeMinimumValueKey,
                [NSNumber numberWithUnsignedInteger:0], QCPortAttributeDefaultValueKey,
                [NSNumber numberWithUnsignedInteger:1], QCPortAttributeMaximumValueKey,
                nil];
    
    if([key isEqualToString:@"inputChrRamValue"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Character RAM Value"                , QCPortAttributeNameKey,
                [NSNumber numberWithUnsignedInteger:0], QCPortAttributeMinimumValueKey,
                [NSNumber numberWithUnsignedInteger:0], QCPortAttributeDefaultValueKey,
                [NSNumber numberWithUnsignedInteger:1], QCPortAttributeMaximumValueKey,
                nil];
    
    if([key isEqualToString:@"outputImage"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Image", QCPortAttributeNameKey, nil];
    
    if([key isEqualToString:@"inputEnableDebugMode"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Enable Debug Mode"        , QCPortAttributeNameKey,
                [NSNumber numberWithBool:NO], QCPortAttributeDefaultValueKey,
                nil];
    
    return nil;
}

+ (NSArray*)sortedPropertyPortKeys
{
    return [NSArray arrayWithObjects:
            @"inputRom",
            @"inputControllerData",
            @"inputVolume",
            @"inputPauseEmulation",
            @"inputSaveStatePath",
            @"inputLoadStatePath",
            @"inputCheatCode",
            @"inputEnableRewinder",
            @"inputEnableRewinderBackwardsSound",
            @"inputRewinderDirection",
            //@"inputRewinderReset",
            @"inputNmtRamCorrupt",
            @"inputNmtRamOffset",
            @"inputNmtRamValue",
            @"inputChrRamCorrupt",
            @"inputChrRamOffset",
            @"inputChrRamValue",
            @"inputCorruptNameTable",
            @"inputNameTableData",
            @"inputEnableDebugMode",
            nil];
}


+ (QCPlugInExecutionMode)executionMode
{
    return kQCPlugInExecutionModeProvider;
}

+ (QCPlugInTimeMode)timeMode
{
    return kQCPlugInTimeModeIdle;
}

- (id) init
{
    if((self = [super init]))
    {
    }
    
    return self;
}

- (void) finalize
{
    [super finalize];
}

- (void) dealloc
{
    [super dealloc];
}

+ (NSArray*)plugInKeys
{
    return nil;
}

@end

@implementation OpenEmuQCNES (Execution)

#pragma mark - State loading/saving

- (void) saveState: (NSString *) fileName
{
    BOOL isDir;
    DLog(@"saveState filename is %@", fileName);
    
    NSString *filePath = [fileName stringByDeletingLastPathComponent];
    
    // If the extension isn't .sav, make it so
    if(![[fileName pathExtension] caseInsensitiveCompare:@"sav"])
        fileName = [fileName stringByAppendingPathExtension:@"sav"];
    
    // See if directory exists
    if([[NSFileManager defaultManager] fileExistsAtPath:filePath isDirectory:&isDir] && isDir)
    {
        // if so, save the state
        [[[gameCoreManager rootProxy] gameCore] saveStateToFileAtPath: fileName];
    } 
    else if (![[NSFileManager defaultManager] fileExistsAtPath:filePath])
    {
        // if not, bitch about it
        DLog(@"Save state directory does not exist");
    }
}

- (BOOL)loadState:(NSString *)fileName
{
    BOOL isDir;
    DLog(@"loadState path is %@", fileName);
    
    if(![fileName caseInsensitiveCompare:@"sav"])
    {
        NSLog(@"Saved state files must have the extension \".sav\" to be loaded.");
        return NO;
    }
    
    if([[NSFileManager defaultManager] fileExistsAtPath:fileName isDirectory:&isDir] && !isDir)
    {
        //DO NOT CONCERN YOURSELF WITH EFFICIENCY OR ELEGANCE AT THIS JUNCTURE, DANIEL MORGAN WINCKLER.
        
        //if no ROM has been loaded, don't load the state
        //if(!self.loadedRom)
        //{
        //    NSLog(@"no ROM loaded -- please load a ROM before loading a state");
        //    return NO;
        //}
        //else
        {
            [[[gameCoreManager rootProxy] gameCore] loadStateFromFileAtPath:fileName];
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

#pragma mark Execution

- (BOOL)startExecution:(id<QCPlugInContext>)context
{    
    return YES;
}

- (void)enableExecution:(id<QCPlugInContext>)context
{
    DLog(@"enableExecution: was called");
    [[gameCoreManager rootProxy] setPauseEmulation:NO];
}

- (BOOL)execute:(id<QCPlugInContext>)context atTime:(NSTimeInterval)time withArguments:(NSDictionary *)arguments
{
    // handle input keys changing
    
#ifdef DEBUG_PRINT
    // turn debug mode on or off
    if([self didValueForInputKeyChange:@"inputEnableDebugMode"])
        [self enableDebugMode:[[self valueForInputKey:@"inputEnableDebugMode"] boolValue]];
#endif
    
    if([self didValueForInputKeyChange:@"inputRom"])
    {
        NSString *romPath;
        if([[self.inputRom stringByStandardizingPath] isAbsolutePath])
            romPath = [self.inputRom stringByStandardizingPath];
        else
            romPath = [[[[context compositionURL] path] stringByDeletingLastPathComponent] stringByAppendingPathComponent:[self.inputRom stringByStandardizingPath]];
        
        if([[NSFileManager defaultManager] fileExistsAtPath:romPath])
        {
            [self terminateEmulation];
            [self readFromURL:[NSURL fileURLWithPath:romPath]];
        }
    }
    
    if([self didValueForInputKeyChange:@"inputVolume"])
        [[gameCoreManager rootProxy] setVolume:self.inputVolume];
    
    if([self didValueForInputKeyChange:@"inputPauseEmulation"])
        [[gameCoreManager rootProxy] setPauseEmulation:self.inputPauseEmulation];
    
    // Process controller data
    if([self didValueForInputKeyChange: @"inputControllerData"])
    {
        // hold on to the controller data, which we are going to feed gameCore every frame.  Mmmmm...controller data.
        if([self controllerDataValidate:[self inputControllerData]])
        {
            [self setPersistantControllerData:[self inputControllerData]];
            
            [self handleControllerData];
        }
    }  
           
    // Process state saving 
    if([self didValueForInputKeyChange: @"inputSaveStatePath"] &&
       (self.inputSaveStatePath != [[OpenEmuQCNES attributesForPropertyPortWithKey:@"inputSaveStatePath"] valueForKey:QCPortAttributeDefaultValueKey]))
    {
        NSString* saveStatePath;
        if ([[self.inputRom stringByStandardizingPath] isAbsolutePath])
        {
            saveStatePath = [self.inputSaveStatePath stringByStandardizingPath];
        }
        else
        {
            saveStatePath = [[[[context compositionURL] path] stringByDeletingLastPathComponent] stringByAppendingPathComponent:[self.inputSaveStatePath stringByStandardizingPath]];
        }
        
        DLog(@"save path changed");
        [self saveState:saveStatePath];
    }
    
    // Process state loading
    if([self didValueForInputKeyChange:@"inputLoadStatePath"]
       && (self.inputLoadStatePath != [[OpenEmuQCNES attributesForPropertyPortWithKey:@"inputSaveStatePath"] valueForKey: QCPortAttributeDefaultValueKey]))
    {
        NSString* saveStatePath;
        if ([[self.inputRom stringByStandardizingPath] isAbsolutePath])
        {
            saveStatePath = [self.inputSaveStatePath stringByStandardizingPath];
        }
        else
        {
            saveStatePath = [[[[context compositionURL] path] stringByDeletingLastPathComponent] stringByAppendingPathComponent:[self.inputSaveStatePath stringByStandardizingPath]];
        }
        
        DLog(@"load path changed");
        [self loadState:saveStatePath];
    }
        
    // Process cheat codes
    if([self didValueForInputKeyChange: @"inputCheatCode"] &&
       ([self valueForInputKey:@"inputCheatCode"] != [[OpenEmuQCNES attributesForPropertyPortWithKey:@"inputCheatCode"] valueForKey: QCPortAttributeDefaultValueKey]))
    {
        DLog(@"cheat code entered");
        [[[gameCoreManager rootProxy] gameCore] setCode:[self valueForInputKey:@"inputCheatCode"]];
    }
    
    // Process rewinder
    if([self didValueForInputKeyChange: @"inputEnableRewinder"])
    {
        [[[gameCoreManager rootProxy] gameCore] enableRewinder:[[self valueForInputKey:@"inputEnableRewinder"] boolValue]];
        
        if([[[gameCoreManager rootProxy] gameCore] isRewinderEnabled])
            DLog(@"rewinder is enabled");
        else
            DLog(@"rewinder is disabled");
    }
    
    //int *rewindTimer;
    //rewindTimer = [[NSNumber alloc] initWithUnsignedInteger:0];
    //
    //if([(NESGameEmu *)gameCore isRewinderEnabled])
    //{
    //    rewindTimer++;
    //    if((rewindTimer % 60) == 0)
    //        DLog(@"rewind timer count is %d",rewindTimer);
    //}
    
    if([self didValueForInputKeyChange: @"inputRewinderDirection"])
    {
        DLog(@"rewinder direction changed");
        [[[gameCoreManager rootProxy] gameCore] rewinderDirection:[[self valueForInputKey:@"inputRewinderDirection"] boolValue]];
    }
    
    if([self didValueForInputKeyChange:@"inputEnableRewinderBackwardsSound"])
    {
        [[[gameCoreManager rootProxy] gameCore] enableRewinderBackwardsSound:[[self valueForInputKey:@"inputEnableRewinderBackwardsSound"] boolValue]];
        
        if([[[gameCoreManager rootProxy] gameCore] isRewinderBackwardsSoundEnabled])
        {
            DLog(@"rewinder backwards sound is enabled");
        }
        else 
        {
            DLog(@"rewinder backwards sound is disabled");
        }
    }
    
    //glitch methods (CORRUPTION FTW)
    if([[[gameCoreManager rootProxy] gameCore] cartVRamSize] && self.inputNmtRamCorrupt &&
       ([self didValueForInputKeyChange:@"inputNmtRamOffset"] || [self didValueForInputKeyChange:@"inputNmtRamValue"]))
    {
        [[[gameCoreManager rootProxy] gameCore] setNmtRamBytes:self.inputNmtRamOffset value:self.inputNmtRamValue];
    }
    
    if([[[gameCoreManager rootProxy] gameCore] chrRomSize] && self.inputChrRamCorrupt &&
       ([self didValueForInputKeyChange:@"inputChrRamOffset"] || [self didValueForInputKeyChange:@"inputChrRamValue"]))
    {
        [[[gameCoreManager rootProxy] gameCore] setChrRamBytes:self.inputChrRamOffset value:self.inputChrRamValue];
    }
    
    if([[[gameCoreManager rootProxy] gameCore] cartVRamSize] && self.inputCorruptNameTable && [self didValueForInputKeyChange:@"inputNameTableData"])
    {
        if([self validateNameTableData:[self inputNameTableData]])
        {
            [self setPersistantNameTableData:[self inputNameTableData]];
            [[[gameCoreManager rootProxy] gameCore] setNMTRamByTable:[persistantNameTableData objectAtIndex:0] array:[persistantNameTableData objectAtIndex:1]];
        }
    }
    
    // according to CGLIOSurface we must rebind our texture every time we want a new stuff from it.
    // since our ID may change every frame we make a new texture each pass. 
    
    //    NSLog(@"Surface ID: %u", (NSUInteger) surfaceID);
    
    IOSurfaceRef surfaceRef = NULL;
    IOSurfaceID  surfaceID  = 0;
    if([gameCoreManager rootProxy] != nil)
    {
        surfaceID = [[gameCoreManager rootProxy] surfaceID];
        // WHOA - This causes a retain.
        surfaceRef = IOSurfaceLookup(surfaceID);
    }
    
    // get our IOSurfaceRef from our passed in IOSurfaceID from our background process.
    if(surfaceRef != NULL)
    {
        CGLContextObj cgl_ctx = [context CGLContextObj];
        
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        GLuint captureTexture;
        glGenTextures(1, &captureTexture);
        glEnable(GL_TEXTURE_RECTANGLE_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, captureTexture);
        
        CGLError err = CGLTexImageIOSurface2D(cgl_ctx, GL_TEXTURE_RECTANGLE_ARB, GL_RGBA8, IOSurfaceGetWidth(surfaceRef), IOSurfaceGetHeight(surfaceRef), GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, surfaceRef, 0);
        if(err != kCGLNoError)
        {
            NSLog(@"Error creating IOSurface texture: %s & %x", CGLErrorString(err), glGetError());
        }
        
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
        glDisable(GL_TEXTURE_RECTANGLE_ARB);
        glPopAttrib();
        
        self.outputImage = [context outputImageProviderFromTextureWithPixelFormat:QCPlugInPixelFormatBGRA8 pixelsWide:IOSurfaceGetWidth(surfaceRef) pixelsHigh:IOSurfaceGetHeight(surfaceRef) name:captureTexture flipped:NO releaseCallback:_TextureReleaseCallback releaseContext:nil colorSpace:[context colorSpace] shouldColorMatch:YES];
        
        // release the surface
        CFRelease(surfaceRef);
    }
    else
        self.outputImage = nil;

    return YES;
}

- (void)disableExecution:(id<QCPlugInContext>)context
{
    DLog(@"### disableExecution was called.");
    [[gameCoreManager rootProxy] setPauseEmulation:YES];
}

- (void)stopExecution:(id<QCPlugInContext>)context
{
    DLog(@"### stopExecution was called.");
    if([[gameCoreManager helper] isRunning])
        [self terminateEmulation];
}

#pragma mark Helper Process

- (void)terminateEmulation
{
    DLog("terminateEmulation was called");
    
    // kill our background friend
    [gameCoreManager stop];
    [gameCoreManager release];
    gameCoreManager = nil;
}

#pragma mark Loading

- (OECorePlugin *)OE_pluginForFileExtension:(NSString *)ext
{
    OECorePlugin *ret = nil;
    
    NSArray *validPlugins = [OECorePlugin pluginsForFileExtension:ext];
    
    //if([validPlugins count] <= 1)
    ret = [validPlugins lastObject];
    
    return ret;
}

- (BOOL)readFromURL:(NSURL *)absoluteURL
{
    NSString *romPath = [absoluteURL path];
    if([[NSFileManager defaultManager] fileExistsAtPath:romPath])
    {
        OECorePlugin *plugin = [self OE_pluginForFileExtension:[absoluteURL pathExtension]];
        
        if(plugin == nil) return NO;

        Class managerClass = (self.debugMode
                              ? [OEGameCoreThreadManager  class]
                              : [OEGameCoreProcessManager class]);
        
        DLog(@"managerClass = %@", managerClass);
        gameCoreManager = [[managerClass alloc] initWithROMAtPath:romPath corePlugin:plugin owner:nil error:nil];
        
        if(gameCoreManager != nil)
        {
            NSLog(@"have manager");
            [[gameCoreManager rootProxy] setupEmulation];
            
            return YES;
        }
    }
    return NO;
}

#pragma mark Controller

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

- (void)handleControllerData
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
        //NSLog(@"index is %u", i);
        // FIXME: Are there really any other cases than YES and NO for a boolean?
        if([[controllerArray objectAtIndex:i] boolValue] == YES) // down
        {
            //NSLog(@"button %u is down", i);
            //[gameCore buttonPressed:i forPlayer:[playerNumber intValue]];
            [[gameCoreManager rootProxy] player:[playerNumber intValue] didPressButton:(i + 1)];
        }
        else if([[controllerArray objectAtIndex:i] boolValue] == NO) // up
        {
            //NSLog(@"button %u is up", i);
            //[gameCore buttonRelease:i forPlayer:[playerNumber intValue]];
            [[gameCoreManager rootProxy] player:[playerNumber intValue] didReleaseButton:(i + 1)];
        }
    }
}

#pragma mark Nestopia Add-ons

- (BOOL)validateNameTableData:(NSArray *)nameTableData
{
    if([nameTableData count] == 2 && ([[nameTableData objectAtIndex:0] intValue] <= 3) &&
       [[nameTableData objectAtIndex:1] count] == 960)
    {
        //DLog(@"validated name table data");
        return YES;
    }
    
    NSLog(@"error: missing or invalid name table data structure.");
    return NO;
}

#pragma mark Debuggery

- (void)enableDebugMode:(BOOL)flag
{
    self.debugMode = flag;
}

@end
