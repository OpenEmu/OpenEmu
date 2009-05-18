//
//  OpenEmuQCPlugin.m
//  OpenEmuQC
//
//  Started by Dan Winckler on 4/11/08 with soooooooooooooooo much help from vade and daemoncollector.
//

/* It's highly recommended to use CGL macros instead of changing the current context for plug-ins that perform OpenGL rendering */
//#import <OpenGL/CGLMacro.h>

#import "OpenEmuQCPlugin.h"
#import "GameCore.h"
#import "GameBuffer.h"
#import "GameAudio.h"

#define    kQCPlugIn_Name                @"OpenEmu"
#define    kQCPlugIn_Description        @"Wraps the OpenEmu emulator - Play NES, Atari, Gameboy, Sega roms in QC"

static void _TextureReleaseCallback(CGLContextObj cgl_ctx, GLuint name, void* info)
{
    
    glDeleteTextures(1, &name);
}

static void _BufferReleaseCallback(const void* address, void* info)
{
    NSLog(@"called buffer release callback");
    //    free((void*)address);
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
                @"~/relative/or/abs/path/to/rom", QCPortAttributeDefaultValueKey, 
                nil]; 
    
    if([key isEqualToString:@"inputVolume"]) 
        return [NSDictionary dictionaryWithObjectsAndKeys:    @"Volume", QCPortAttributeNameKey, 
                [NSNumber numberWithFloat:0.5], QCPortAttributeDefaultValueKey, 
                [NSNumber numberWithFloat:1.0], QCPortAttributeMaximumValueKey,
                [NSNumber numberWithFloat:0.0], QCPortAttributeMinimumValueKey,
                nil]; 
    
    
    if([key isEqualToString:@"inputControllerData"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Controller Data", QCPortAttributeNameKey, nil];
    
    // NSArray with player count in index 0, index 1 is eButton "struct" (see GameButtons.h for typedef)
    
    if([key isEqualToString:@"inputPauseEmulation"])
        return [NSDictionary dictionaryWithObjectsAndKeys:    @"Pause Emulator", QCPortAttributeNameKey,
                NO, QCPortAttributeDefaultValueKey, 
                nil];
    
    
    if([key isEqualToString:@"inputSaveStatePath"])
        return [NSDictionary dictionaryWithObjectsAndKeys:    @"Save State", QCPortAttributeNameKey,
                @"~/roms/saves/save", QCPortAttributeDefaultValueKey, 
                nil];
    
    if([key isEqualToString:@"inputLoadStatePath"])
        return [NSDictionary dictionaryWithObjectsAndKeys:    @"Load State", QCPortAttributeNameKey,
                @"~/roms/saves/save", QCPortAttributeDefaultValueKey, 
                nil];
    if([key isEqualToString:@"outputImage"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Image", QCPortAttributeNameKey, nil];
    
    return nil;
}

+ (NSArray*) sortedPropertyPortKeys
{
    return [NSArray arrayWithObjects:@"inputRom", @"inputControllerData", @"inputVolume", @"inputPauseEmulation", @"inputSaveStatePath", @"inputLoadStatePath", nil]; 
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
        audioPaused = NO;
        persistantControllerData = [[NSMutableArray alloc] init];
        [persistantControllerData retain];
        
        //    bundle = [NSBundle bundleWithPath:[[NSBundle bundleForClass:[self class]] pathForResource:@"Nestopia" ofType:@"bundle"]];
        
        NSString *file;
        NSBundle *theBundle = [NSBundle bundleForClass:[self class]];
        NSDictionary *ourBundleInfo = [theBundle infoDictionary];
        NSString *bundleDir = [[ourBundleInfo valueForKey:@"OEBundlePath"] stringByStandardizingPath];
        
        // NSString *bundleDir = [[[[NSBundle bundleForClass:[self class]] infoDictionary] valueForKey:@"OEBundlePath"] stringByStandardizingPath];
        NSMutableArray* bundlePaths = [[NSMutableArray alloc] init];
        
        NSDirectoryEnumerator *enumerator = [[NSFileManager defaultManager] enumeratorAtPath: bundleDir];
        while (file = [enumerator nextObject])
        {
            if([[file pathExtension] isEqualToString:@"bundle"]) 
            {
                [bundlePaths addObject:[bundleDir stringByAppendingPathComponent:file]];
                [enumerator skipDescendents];
            }
            
        }
        
        
        NSMutableArray* mutableBundles = [[NSMutableArray alloc] init];
        
        for(NSString* path in bundlePaths)
        {
            [mutableBundles addObject:[NSBundle bundleWithPath:path]];
        }
        
        [bundlePaths release];
        //All bundles that are available
        bundles = [[NSArray arrayWithArray:mutableBundles] retain];
        
        [mutableBundles release];
        
        NSMutableArray* mutableExtensions = [[NSMutableArray alloc] init];
        
        for(NSBundle* bundle in bundles)
        {
            NSArray* types = [[bundle infoDictionary] objectForKey:@"CFBundleDocumentTypes"];
            
            for (NSDictionary* key in types)
            {
                [mutableExtensions addObjectsFromArray:[key objectForKey:@"CFBundleTypeExtensions"]];
            }
        }
        
        NSArray* types = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleDocumentTypes"];
        
        for (NSDictionary* key in types)
        {
            [mutableExtensions addObjectsFromArray:[key objectForKey:@"CFBundleTypeExtensions"]];
        }        
        
        validExtensions = [[NSArray arrayWithArray:mutableExtensions] retain];
        [mutableExtensions release];
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
    [bundles release];
    [validExtensions release];
    [persistantControllerData release];
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

@implementation OpenEmuQC (Execution)


- (BOOL) startExecution:(id<QCPlugInContext>)context
{    
    NSLog(@"called startExecution");
    //    if(loadedRom)
    //    {
    //        [gameAudio startAudio];
    //        [gameCore start]; 
    //    }
    
    return YES;
}

- (void) enableExecution:(id<QCPlugInContext>)context
{
    NSLog(@"called enableExecution");
    // if we have a ROM loaded and the patch's image output is reconnected, unpause the emulator
    if(loadedRom)
    {
        if(!self.inputPauseEmulation) 
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
    
    /*
     Called by Quartz Composer when the plug-in instance starts being used by Quartz Composer.
     */
}

- (BOOL) execute:(id<QCPlugInContext>)context atTime:(NSTimeInterval)time withArguments:(NSDictionary*)arguments
{    
    CGLSetCurrentContext([context CGLContextObj]);
    CGLLockContext([context CGLContextObj]);
    
    // our output image via convenience methods
    id    provider = nil;
    
    // Process ROM loads
    if([self didValueForInputKeyChange: @"inputRom"] && ([self valueForInputKey:@"inputRom"] != [[OpenEmuQC attributesForPropertyPortWithKey:@"inputRom"] valueForKey: QCPortAttributeDefaultValueKey]))
    {
        [self loadRom:[self valueForInputKey:@"inputRom"]];
    }
    
    if(loadedRom)
    {
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
        if([self didValueForInputKeyChange: @"inputVolume"])
        {
            [gameAudio setVolume:[[self valueForInputKey:@"inputVolume"] floatValue]];
        }
        
        // Process emulation pausing FTW
        if([self didValueForInputKeyChange: @"inputPauseEmulation"])    
        {
            if([[self valueForInputKey:@"inputPauseEmulation"] boolValue])    
            {
                [gameAudio pauseAudio];
                [gameCore setPauseEmulation:YES]; 
            }
            else 
            {
                [gameAudio startAudio];
                [gameCore setPauseEmulation:NO];
            }
        }
        
        // Process state saving 
        if([self didValueForInputKeyChange: @"inputSaveStatePath"]
           && ([self valueForInputKey:@"inputSaveStatePath"] != [[OpenEmuQC    attributesForPropertyPortWithKey:@"inputSaveStatePath"] valueForKey: QCPortAttributeDefaultValueKey])
           && (![[self valueForInputKey:@"inputSaveStatePath"] isEqualToString:@""] ))
        {
            NSLog(@"save path changed");
            [self saveState:[[self valueForInputKey:@"inputSaveStatePath"] stringByStandardizingPath]];
        }
        
        // Process state loading
        if([self didValueForInputKeyChange: @"inputLoadStatePath"] 
           && ([self valueForInputKey:@"inputLoadStatePath"] != [[OpenEmuQC    attributesForPropertyPortWithKey:@"inputLoadStatePath"] valueForKey: QCPortAttributeDefaultValueKey])
           && (![[self valueForInputKey:@"inputLoadStatePath"] isEqualToString:@""] ))    
        {
            NSLog(@"load path changed");
            [self loadState:[[self valueForInputKey:@"inputLoadStatePath"] stringByStandardizingPath]];
        }
    }
    // handle our image output. (sanity checking)
    if(loadedRom && ([gameCore width] > 10) )
    {        
        glEnable( GL_TEXTURE_RECTANGLE_EXT );
        
        GLenum status;
        GLuint texName;
        glGenTextures(1, &texName);
        
        glBindTexture( GL_TEXTURE_RECTANGLE_EXT, texName);
        glTexImage2D( GL_TEXTURE_RECTANGLE_EXT, 0, [gameCore internalPixelFormat], [gameCore width], [gameCore height], 0, [gameCore pixelFormat], [gameCore pixelType], [gameCore videoBuffer]);
        
        // Check for OpenGL errors 
        status = glGetError();
        if(status)
        {
            NSLog(@"OpenGL error %04X", status);
            glDeleteTextures(1, &texName);
            texName = 0;
        }
        
        glFlushRenderAPPLE();
        
#if __BIG_ENDIAN__
#define OEPlugInPixelFormat QCPlugInPixelFormatARGB8
#else
#define OEPlugInPixelFormat QCPlugInPixelFormatBGRA8
#endif
        
        provider = [context outputImageProviderFromTextureWithPixelFormat:OEPlugInPixelFormat 
                                                               pixelsWide:[gameCore width]
                                                               pixelsHigh:[gameCore height]
                                                                     name:texName 
                                                                  flipped:YES 
                                                          releaseCallback:_TextureReleaseCallback 
                                                           releaseContext:NULL
                                                               colorSpace:CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB)
                                                         shouldColorMatch:YES];
    }
    
    // output OpenEmu Texture - note we CAN output a nil image. This is 'correct'
    self.outputImage = provider;
    
    CGLUnlockContext([context CGLContextObj]);
    
    return YES;
}

- (void) disableExecution:(id<QCPlugInContext>)context
{
    NSLog(@"called disableExecution");
    
    // if we have a ROM running and the patch's image output is disconnected, pause the emulator and audio
    if(loadedRom)
    {
        if(!self.inputPauseEmulation) 
        {
            @try
            {
                [gameCore setPauseEmulation:YES]; 
                [gameAudio pauseAudio];
            }
            @catch (NSException * e) {
                NSLog(@"Failed to unpause");
            }
        }  
        //    sleep(0.5); // race condition workaround. 
    }
    /*
     Called by Quartz Composer when the plug-in instance stops being used by Quartz Composer.
     */
}

- (void) stopExecution:(id<QCPlugInContext>)context
{
    NSLog(@"called stopExecution");
    if(loadedRom)
    {
        [gameCore stopEmulation];
        [gameAudio stopAudio];
        [gameCore release];
        [gameAudio release];
        loadedRom = NO;
    }
}

# pragma mark -

-(BOOL) controllerDataValidate:(NSArray*) cData
{
    // sanity check
    if([cData count] == 2 && [[cData objectAtIndex:1] count] == 12)
    {
        //        NSLog(@"validated controller data");
        return YES;
    }    
    
    return NO;
}

- (BOOL) loadRom:(NSString*) romPath
{
    NSString* theRomPath = [romPath stringByStandardizingPath];
    BOOL isDir;    
    
    NSLog(@"New ROM path is: %@",theRomPath);
    
    if([[NSFileManager defaultManager] fileExistsAtPath:theRomPath isDirectory:&isDir] && !isDir)
    {
        NSString * extension = [theRomPath pathExtension];
        NSLog(@"extension is: %@", extension);
        
        // cleanup
        if(loadedRom)
        {
            [gameCore stopEmulation];
            [gameAudio stopAudio];
            [gameCore release];
            //    [gameBuffer release];
            [gameAudio release];
            
            NSLog(@"released/cleaned up for new rom");
            
			[gameCoreController release];
			
			NSLog(@"Released/cleaned up gameCoreController");
			
        }
        loadedRom = NO;
        
        NSBundle* loadBundle = [self bundleForType:extension];
        
        NSLog(@"Bundle loaded for path is: %@", [loadBundle bundlePath]);
        
        gameCoreController = [[[loadBundle principalClass] alloc] init];
		gameCore = [gameCoreController newGameCoreWithDocument:nil];
		
        NSLog(@"Loaded bundle. About to load rom...");
        
		[gameCore initWithDocument:(GameDocument*)self];
        [gameCore loadFileAtPath:theRomPath];
        loadedRom = TRUE;
        
        if(loadedRom)
        {
            NSLog(@"Loaded new Rom: %@", theRomPath);
            [gameCore setupEmulation];
            
            //    gameBuffer = [[GameBuffer alloc] initWithGameCore:gameCore];
            //    [gameBuffer setFilter:eFilter_None];
            // audio!
            gameAudio = [[GameAudio alloc] initWithCore:gameCore];
            NSLog(@"initialized audio");
            
            // starts the threaded emulator timer
            [gameCore startEmulation];
            
            NSLog(@"About to start audio");
            [gameAudio startAudio];
            [gameAudio setVolume:[self inputVolume]];
            
            NSLog(@"finished loading/starting rom");
            return YES;
        }    
        else
        {
            NSLog(@"ROM did not load.");
        }
    }
    else {
        NSLog(@"bad ROM path or filename");
    }
    return NO;
}


-(void) handleControllerData
{
    // iterate through our NSArray of controller data. We know the player, we know the structure.
    // pull it out, and hand it off to our gameCore
    
    // sanity check (again? sure!)
    if([self controllerDataValidate:persistantControllerData])
    {
        
        // player number 
        NSNumber*  playerNumber = [persistantControllerData objectAtIndex:0];
        NSArray * controllerArray = [persistantControllerData objectAtIndex:1];
        
        //    NSLog(@"Player Number: %u", [playerNumber intValue]);
        
        NSUInteger i;
        for(i = 0; i < [controllerArray count]; i++)
        {
            //    NSLog(@"index is %u", i);
            if([[controllerArray objectAtIndex:i] boolValue] == TRUE) // down
            {
                //    NSLog(@"button %u is down", i);
                //    [gameCore buttonPressed:i forPlayer:[playerNumber intValue]];
                [gameCore player:[playerNumber intValue] didPressButton:i];
            }        
            else if([[controllerArray objectAtIndex:i] boolValue] == FALSE) // up
            {
                //    NSLog(@"button %u is up", i);
                //    [gameCore buttonRelease:i forPlayer:[playerNumber intValue]];
                [gameCore player:[playerNumber intValue] didReleaseButton:i];
            }
        } 
    }    
    
}

// callback for audio from plugin
- (void) refresh
{
    [gameAudio advanceBuffer];
}

- (NSBundle*)bundleForType:(NSString*) type
{
    NSLog(@"Bundle");
    //Need to make it so if multiple bundles load same extensions, it presents a picker
    for(NSBundle* bundle in bundles)
    {
        NSArray* types = [[bundle infoDictionary] objectForKey:@"CFBundleDocumentTypes"];
        
        for (NSDictionary* key in types)
        {
            NSArray* exts = [key objectForKey:@"CFBundleTypeExtensions"];
            
            for(NSString* str in exts)
            {
                if([str caseInsensitiveCompare:type] == 0)  //ignoring case so it doesn't matter if the extension is NES or nes or NeS or nES
                    return bundle;
            }
        }
        
    }
    
    return nil;
}

- (void) saveState: (NSString *) fileName
{
    BOOL isDir;
    NSLog(@"saveState filename is %@", fileName);
    
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
        NSLog(@"Save state directory does not exist");
    }
}

- (BOOL) loadState: (NSString *) fileName
{
    BOOL isDir;    
    NSLog(@"loadState path is %@", fileName);
    
    if([[fileName pathExtension] caseInsensitiveCompare:@"sav"] != 0) 
    {
        NSLog(@"Saved state files must have the extension \".sav\" to be loaded.");
        return NO;
    }
    
    if([[NSFileManager defaultManager] fileExistsAtPath:fileName isDirectory:&isDir] && !isDir)
    {
        //DO NOT CONCERN YOURSELF WITH EFFICIENCY OR ELEGANCE AT THIS JUNCTURE, DANIEL MORGAN WINCKLER.
        
        //if no ROM has been loaded, don't load the state
        if(!loadedRom) {
            NSLog(@"no ROM loaded -- please load a ROM before loading a state");
            return NO;
        }
        else {
            [gameCore loadStateFromFileAtPath: fileName];
            NSLog(@"loaded new state");
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
