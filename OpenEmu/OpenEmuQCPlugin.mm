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
#import "OpenEmuQCPlugin.h"
#import <Quartz/Quartz.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import "GameCore.h"
#import "GameAudio.h"
#import "OECorePlugin.h"

#define    kQCPlugIn_Name               @"OpenEmu"
#define    kQCPlugIn_Description        @"Wraps the OpenEmu emulator - Play NES, Atari, Gameboy, Sega roms in QC"

static void _TextureReleaseCallback(CGLContextObj cgl_ctx, GLuint name, void* info)
{
//   glDeleteTextures(1, &name);
}

static void _BufferReleaseCallback(const void* address, void* info)
{
    DLog(@"called buffer release callback");
    //free((void*)address);
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


@implementation OpenEmuQC

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
@dynamic outputImage;

@synthesize loadedRom, userPaused;

+ (NSDictionary *)attributes
{
    /*
     Return a dictionary of attributes describing the plug-in (QCPlugInAttributeNameKey, QCPlugInAttributeDescriptionKey...).
     */
    
    return [NSDictionary dictionaryWithObjectsAndKeys:kQCPlugIn_Name, QCPlugInAttributeNameKey, kQCPlugIn_Description, QCPlugInAttributeDescriptionKey, nil];
}

+ (NSDictionary *)attributesForPropertyPortWithKey:(NSString*)key
{
    /*
     Specify the optional attributes for property based ports (QCPortAttributeNameKey, QCPortAttributeDefaultValueKey...).
     */
    if([key isEqualToString:@"inputRom"]) 
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"ROM Path", QCPortAttributeNameKey, 
                @"~/relative/or/abs/path/to/rom", QCPortAttributeDefaultValueKey, 
                nil]; 
    
    if([key isEqualToString:@"inputVolume"]) 
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Volume", QCPortAttributeNameKey, 
                [NSNumber numberWithFloat:0.5], QCPortAttributeDefaultValueKey, 
                [NSNumber numberWithFloat:1.0], QCPortAttributeMaximumValueKey,
                [NSNumber numberWithFloat:0.0], QCPortAttributeMinimumValueKey,
                nil]; 
    
    
    if([key isEqualToString:@"inputControllerData"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Controller Data", QCPortAttributeNameKey, nil];
    
    // NSArray with player count in index 0, index 1 is eButton "struct" (see GameButtons.h for typedef)
    if([key isEqualToString:@"inputPauseEmulation"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Pause Emulator", QCPortAttributeNameKey,
                [NSNumber numberWithBool:NO], QCPortAttributeDefaultValueKey, 
                nil];
    
    
    if([key isEqualToString:@"inputSaveStatePath"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Save State", QCPortAttributeNameKey,
                @"~/roms/saves/save", QCPortAttributeDefaultValueKey, 
                nil];
    
    if([key isEqualToString:@"inputLoadStatePath"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Load State", QCPortAttributeNameKey,
                @"~/roms/saves/save", QCPortAttributeDefaultValueKey, 
                nil];
    if([key isEqualToString:@"outputImage"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Image", QCPortAttributeNameKey, nil];
    
    return nil;
}

+ (NSArray *)sortedPropertyPortKeys
{
    return [NSArray arrayWithObjects:@"inputRom", @"inputControllerData", @"inputVolume",
            @"inputPauseEmulation", @"inputSaveStatePath", @"inputLoadStatePath", nil]; 
}


+ (QCPlugInExecutionMode)executionMode
{
    /*
     Return the execution mode of the plug-in: kQCPlugInExecutionModeProvider, kQCPlugInExecutionModeProcessor, or kQCPlugInExecutionModeConsumer.
     */
    return kQCPlugInExecutionModeProvider;
}

+ (QCPlugInTimeMode)timeMode
{
    /*
     Return the time dependency mode of the plug-in: kQCPlugInTimeModeNone, kQCPlugInTimeModeIdle or kQCPlugInTimeModeTimeBase.
     */
    return kQCPlugInTimeModeIdle;
}

- (id)init
{
    if(self = [super init])
    {
        gameLock = [[NSRecursiveLock alloc] init];
        persistantControllerData = [[NSMutableArray alloc] init];
        [persistantControllerData retain];
        
        plugins = [[OECorePlugin allPlugins] retain];
        validExtensions = [[OECorePlugin supportedTypeExtensions] retain];
        
        self.userPaused = NO; self.loadedRom = NO;
    }
    
    return self;
}

- (void)finalize
{
    /* Destroy variables intialized in init and not released by GC */
    [super finalize];
}

- (void)dealloc
{
    /* Release any resources created in -init. */
    [validExtensions release];
    [persistantControllerData release];
    [gameLock release];
    [plugins release];
    [super dealloc];
}

+ (NSArray *)plugInKeys
{
    /*
     Return a list of the KVC keys corresponding to the internal settings of the plug-in.
     */
    return nil;
}

- (id)serializedValueForKey:(NSString *)key;
{
    /*
     Provide custom serialization for the plug-in internal settings that are not values complying to the <NSCoding> protocol.
     The return object must be nil or a PList compatible i.e. NSString, NSNumber, NSDate, NSData, NSArray or NSDictionary.
     */
    return [super serializedValueForKey:key];
}

- (void)setSerializedValue:(id)serializedValue forKey:(NSString *)key
{
    /*
     Provide deserialization for the plug-in internal settings that were custom serialized in -serializedValueForKey.
     Deserialize the value, then call [self setValue:value forKey:key] to set the corresponding internal setting of the plug-in instance to that deserialized value.
     */
    [super setSerializedValue:serializedValue forKey:key];
}

@end

@implementation OpenEmuQC (Execution)


- (BOOL)startExecution:(id<QCPlugInContext>)context
{   
    CGLContextObj cgl_ctx = [context CGLContextObj];
    CGLLockContext(cgl_ctx);

    DLog(@"called startExecution");
    if(self.loadedRom && ([gameCore screenWidth] > 10) )
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
    if(self.loadedRom)
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

- (BOOL)execute:(id<QCPlugInContext>)context atTime:(NSTimeInterval)time withArguments:(NSDictionary *)arguments
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

    // our output image via convenience methods
    id    provider = nil;
    
    // Process ROM loads
    if([self didValueForInputKeyChange:@"inputRom"] &&
       ([self valueForInputKey:@"inputRom"] != [[OpenEmuQC attributesForPropertyPortWithKey:@"inputRom"]
                                                valueForKey:QCPortAttributeDefaultValueKey])
        && (![[self valueForInputKey:@"inputRom"] isEqualToString:@""] ))
    {
        [self loadRom:[self valueForInputKey:@"inputRom"]];
        
        if(self.loadedRom) {
        [gameAudio setVolume:[[self valueForInputKey:@"inputVolume"] floatValue]];
        glDeleteTextures(1, &gameTexture);
        gameTexture = createNewTexture(cgl_ctx, [gameCore internalPixelFormat], [gameCore bufferWidth], [gameCore bufferHeight], [gameCore pixelFormat], [gameCore pixelType], [gameCore videoBuffer]);
        }
    }
    
    if(self.loadedRom)
    {
        // Process controller data
        if([self didValueForInputKeyChange:@"inputControllerData"])
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
        if([self didValueForInputKeyChange:@"inputVolume"])
        {
            [gameAudio setVolume:[[self valueForInputKey:@"inputVolume"] floatValue]];
        }
        
        // Process emulation pausing FTW
        if([self didValueForInputKeyChange: @"inputPauseEmulation"])    
        {
            if(self.inputPauseEmulation)
            {
                [gameAudio pauseAudio];
                [gameCore setPauseEmulation:YES]; 
                self.userPaused = YES;
            }
            else 
            {
                [gameAudio startAudio];
                [gameCore setPauseEmulation:NO];
                self.userPaused = NO;
            }
        }
        
        // Process state saving 
        if([self didValueForInputKeyChange: @"inputSaveStatePath"]
           && ([self valueForInputKey:@"inputSaveStatePath"] != [[OpenEmuQC attributesForPropertyPortWithKey:@"inputSaveStatePath"]
                                                                 valueForKey: QCPortAttributeDefaultValueKey])
           && (![[self valueForInputKey:@"inputSaveStatePath"] isEqualToString:@""] ))
        {
            DLog(@"save path changed");
            [self saveState:[[self valueForInputKey:@"inputSaveStatePath"] stringByStandardizingPath]];
        }
        
        // Process state loading
        if([self didValueForInputKeyChange:@"inputLoadStatePath"] 
           && ([self valueForInputKey:@"inputLoadStatePath"] != [[OpenEmuQC attributesForPropertyPortWithKey:@"inputLoadStatePath"]
                                                                 valueForKey: QCPortAttributeDefaultValueKey])
           && (![[self valueForInputKey:@"inputLoadStatePath"] isEqualToString:@""] ))    
        {
            DLog(@"load path changed");
            [self loadState:[[self valueForInputKey:@"inputLoadStatePath"] stringByStandardizingPath]];
        }
    }

#pragma mark provide an image 
    
    // handle our image output. (sanity checking)
    if(self.loadedRom && ([gameCore bufferWidth] > 10) && [gameCore frameFinished])
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
    
    // if we have a ROM running and the patch's image output is disconnected, pause the emulator and audio
    if(self.loadedRom)
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
        //    sleep(0.5); // race condition workaround. 
    }
}

- (void)stopExecution:(id<QCPlugInContext>)context
{
    DLog(@"called stopExecution");
    if(self.loadedRom)
    {
        [gameCore stopEmulation];
        [gameAudio stopAudio];
        [gameCore release];
        [gameAudio release];
        self.loadedRom = NO;
                
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

- (BOOL)loadRom:(NSString *)romPath
{
    NSString *theRomPath = [romPath stringByStandardizingPath];
    BOOL isDir;    
    
    DLog(@"New ROM path is: %@",theRomPath);
    
    if([[NSFileManager defaultManager] fileExistsAtPath:theRomPath isDirectory:&isDir] && !isDir)
    {
        NSString *extension = [theRomPath pathExtension];
        DLog(@"extension is: %@", extension);
        
        // cleanup
        if(self.loadedRom)
        {
            [gameCore stopEmulation];
            [gameAudio stopAudio];
            [gameCore release];
            [gameAudio release];
            
            DLog(@"released/cleaned up for new ROM");            
        }
        self.loadedRom = NO;
        
        OECorePlugin *plugin = [self pluginForType:extension];
        
        gameCoreController = [plugin controller];
        gameCore = [gameCoreController newGameCore];
        
        DLog(@"Loaded bundle. About to load rom...");
        
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
            return self.loadedRom = YES;
        }    
        else NSLog(@"ROM did not load.");
    }
    else NSLog(@"bad ROM path or filename");
    return NO;
}


- (void)handleControllerData
{
    // iterate through our NSArray of controller data. We know the player, we know the structure.
    // pull it out, and hand it off to our gameCore
    
    // player number 
    NSNumber *playerNumber = [persistantControllerData objectAtIndex:0];
    NSArray  *controllerArray = [persistantControllerData objectAtIndex:1];
    
    //NSLog(@"Player Number: %u", [playerNumber intValue]);
    
    NSUInteger i;
    for(i = 0; i < [controllerArray count]; i++)
    {
        if(i > 5 && i < 10)
            continue;
        //NSLog(@"index is %u", i);
        if([[controllerArray objectAtIndex:i] boolValue] == TRUE) // down
        {
            //NSLog(@"button %u is down", i);
            [gameCore player:[playerNumber intValue] didPressButton:(i + 1)];
        }        
        else if([[controllerArray objectAtIndex:i] boolValue] == FALSE) // up
        {
            //NSLog(@"button %u is up", i);
            [gameCore player:[playerNumber intValue] didReleaseButton:(i + 1)];
        }
    } 
}

// callback for audio from plugin
- (void)refresh
{
    //[gameAudio advanceBuffer];
}

- (OECorePlugin *)pluginForType:(NSString *)extension
{
    for(OECorePlugin *plugin in plugins)
        if([plugin supportsFileType:extension])
            return plugin;
    return nil;
}

- (void)saveState:(NSString *)fileName
{
    BOOL isDir;
    DLog(@"saveState filename is %@", fileName);
    
    NSString *filePath = [fileName stringByDeletingLastPathComponent];
    
    // if the extension isn't .sav, make it so
    if([[fileName pathExtension] caseInsensitiveCompare:@"sav"] != 0) 
        fileName = [fileName stringByAppendingPathExtension:@"sav"];
    
    // see if directory exists
    if([[NSFileManager defaultManager] fileExistsAtPath:filePath isDirectory:&isDir] && isDir)
    {
        // if so, save the state
        [gameCore saveStateToFileAtPath: fileName];
    } 
    else if (![[NSFileManager defaultManager] fileExistsAtPath:filePath])
    {
        // if not, bitch about it
        NSLog(@"Save state directory does not exist");
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
        // DO NOT CONCERN YOURSELF WITH EFFICIENCY OR ELEGANCE AT THIS JUNCTURE, DANIEL MORGAN WINCKLER.
        
        //if no ROM has been loaded, don't load the state
        if(!self.loadedRom) {
            NSLog(@"no ROM loaded -- please load a ROM before loading a state");
            return NO;
        }
        else {
            [gameCore loadStateFromFileAtPath: fileName];
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

@end
