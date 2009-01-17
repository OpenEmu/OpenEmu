//
//  OpenEmuQCPlugin.m
//  OpenEmuQC
//
//  Started by Dan Winckler on 4/11/08 with soooooooooooooooo much help from vade and daemoncollector.
//

/* It's highly recommended to use CGL macros instead of changing the current context for plug-ins that perform OpenGL rendering */
//#import <OpenGL/CGLMacro.h>

#import "OpenEmuQCPlugin.h"

#define	kQCPlugIn_Name				@"OpenEmu"
#define	kQCPlugIn_Description		@"Wraps the OpenEmu emulator - Play NES, Atari, Gameboy, Sega roms in QC"

static void _TextureReleaseCallback(CGLContextObj cgl_ctx, GLuint name, void* info)
{
	
	glDeleteTextures(1, &name);
}

static void _BufferReleaseCallback(const void* address, void* info)
{
	NSLog(@"called buffer release callback");
	//	free((void*)address);
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
		return [NSDictionary dictionaryWithObjectsAndKeys:	@"ROM Path", QCPortAttributeNameKey, 
															 @"~/roms/NES/1943.NES", QCPortAttributeDefaultValueKey, 
															nil]; 
	
	if([key isEqualToString:@"inputVolume"]) 
		return [NSDictionary dictionaryWithObjectsAndKeys:	@"Volume", QCPortAttributeNameKey, 
				[NSNumber numberWithFloat:0.5], QCPortAttributeDefaultValueKey, 
				[NSNumber numberWithFloat:1.0], QCPortAttributeMaximumValueKey,
				[NSNumber numberWithFloat:0.0], QCPortAttributeMinimumValueKey,
				nil]; 
	

	if([key isEqualToString:@"inputControllerData"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Controller Data", QCPortAttributeNameKey, nil];
	
	// NSArray with player count in index 0, index 1 is eButton "struct", which is an array which has the following indices:
	
	/*
	 enum eButton_Type {
	0 eButton_A,
	1 eButton_B,
	2 eButton_START,
	3 eButton_SELECT,
	4 eButton_UP,
	5 eButton_DOWN,
	6 eButton_RIGHT,
	7 eButton_LEFT,
	8 eButton_L,
	9 eButton_R,
	10 eButton_X,
	11 eButton_Y
	 };
	 
	*/

	if([key isEqualToString:@"inputPauseEmulation"])
		return [NSDictionary dictionaryWithObjectsAndKeys:	@"Pause Emulator", QCPortAttributeNameKey,
				NO, QCPortAttributeDefaultValueKey, 
				nil];
	
	
	if([key isEqualToString:@"inputSaveStatePath"])
		return [NSDictionary dictionaryWithObjectsAndKeys:	@"Save State", QCPortAttributeNameKey,
															 @"~/roms/saves/save", QCPortAttributeDefaultValueKey, 
															nil];

	if([key isEqualToString:@"inputLoadStatePath"])
		return [NSDictionary dictionaryWithObjectsAndKeys:	@"Load State", QCPortAttributeNameKey,
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
		persistantControllerData = [[NSMutableArray alloc] init];
		[persistantControllerData retain];

	//	bundle = [NSBundle bundleWithPath:[[NSBundle bundleForClass:[self class]] pathForResource:@"Nestopia" ofType:@"bundle"]];
		
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
//	if(loadedRom)
//	{
//		[gameAudio startAudio];
//		[gameCore start]; 
//	}
	
	return YES;
}

- (void) enableExecution:(id<QCPlugInContext>)context
{
	NSLog(@"called enableExecution");
	// if we have a ROM loaded and the patch's image output is reconnected, unpause the emulator
	if(loadedRom)
	{
		[gameAudio startAudio];
		[gameCore pause:NO];
	}
	
	/*
	Called by Quartz Composer when the plug-in instance starts being used by Quartz Composer.
	*/
}

- (BOOL) execute:(id<QCPlugInContext>)context atTime:(NSTimeInterval)time withArguments:(NSDictionary*)arguments
{	
	CGLSetCurrentContext([context CGLContextObj]);
	CGLLockContext([context CGLContextObj]);
	
//	[gameLock lock];
	// our output image via convenience methods
	id	provider = nil;
	
	// Process controller data
	if([self didValueForInputKeyChange: @"inputControllerData"])
	{
//		NSLog(@"New Controller Data");
		
		// hold on to the controller data, which we are going to feed gameCore every frame.  Mmmmm...controller data.
		if([self controllerDataValidate:[self inputControllerData]])
		{
			persistantControllerData = [NSMutableArray arrayWithArray:[self inputControllerData]]; 
			[persistantControllerData retain];
			
			[self handleControllerData];
		}
	}	
		
	// Process ROM loads
	if([self didValueForInputKeyChange: @"inputRom"] && ([self valueForInputKey:@"inputRom"] != [[OpenEmuQC	attributesForPropertyPortWithKey:@"inputRom"] valueForKey: QCPortAttributeDefaultValueKey]))
	{
		[self loadRom:[self valueForInputKey:@"inputRom"]];
	}
	
	// Process audio volume changes
	if([self didValueForInputKeyChange: @"inputVolume"] && ([self valueForInputKey:@"inputVolume"] != [[OpenEmuQC attributesForPropertyPortWithKey:@"inputVolume"] valueForKey: QCPortAttributeDefaultValueKey]))
	{
		// if inputVolume is set to 0, pause the audio
		if([self valueForInputKey: @"inputVolume"] == 0) {
			[gameAudio pauseAudio];
		}
		
		[gameAudio setVolume:[[self valueForInputKey:@"inputVolume"] floatValue]];
	}
	// Process emulation pausing FTW
	if([self didValueForInputKeyChange: @"inputPauseEmulation"])	
	{
		if([[self valueForInputKey:@"inputPauseEmulation"] boolValue])	
		{
			[gameAudio pauseAudio];
			[gameCore pause:YES]; 
		}
		else 
		{
			[gameAudio startAudio];
			[gameCore pause:NO];
		}
	}

	// Process state saving 
	if([self didValueForInputKeyChange: @"inputSaveStatePath"] && ([self valueForInputKey:@"inputSaveStatePath"] != [[OpenEmuQC	attributesForPropertyPortWithKey:@"inputSaveStatePath"] valueForKey: QCPortAttributeDefaultValueKey]))
	{
		NSLog(@"save path changed");
		[self saveState:[[self valueForInputKey:@"inputSaveStatePath"] stringByStandardizingPath]];
	}

	// Process state loading
	if([self didValueForInputKeyChange: @"inputLoadStatePath"] && ([self valueForInputKey:@"inputLoadStatePath"] != [[OpenEmuQC	attributesForPropertyPortWithKey:@"inputLoadStatePath"] valueForKey: QCPortAttributeDefaultValueKey]))	
	{
		NSLog(@"load path changed");
		[self loadState:[[self valueForInputKey:@"inputLoadStatePath"] stringByStandardizingPath]];
	}
	
	// handle our image output. (sanity checking)
	if(loadedRom && ([gameCore width] > 10) )
	{
	
	//	NSLog(@"buffer address is : %x", [gameCore buffer]);
		
	
		/*
		 BELOW IS THE BUFFER METHOD, LETTING QC LOAD THE TEXTURE ITSELF
		 
		 This seems to fix some issues with handling texture upload ourseleves, No idea why. 
		 Seems like there is also no real performance difference here.
		 
		 Note, some emulators may output empty alpha. Ive included a 'fix' via a simple 
		 core image kernel to add in an alpha channel.
		 */
	/*
		switch ([gameCore pixelFormat])
		{
			case GL_BGRA:
				{					
					provider = [context outputImageProviderFromBufferWithPixelFormat:QCPlugInPixelFormatBGRA8
																		  pixelsWide:[gameCore width]
																		  pixelsHigh:[gameCore height]
																		 baseAddress:[gameCore buffer]
																		 bytesPerRow:([gameCore width] * 4 )
																	 releaseCallback:_BufferReleaseCallback
																	  releaseContext:nil
																		  colorSpace:CGColorSpaceCreateWithName(kCGColorSpaceGenericRGBLinear)
																	shouldColorMatch:YES];
				}
				break;
			case GL_RGBA:
				{
					provider = [context outputImageProviderFromBufferWithPixelFormat:QCPlugInPixelFormatARGB8
																		  pixelsWide:[gameCore width]
																		  pixelsHigh:[gameCore height]
																		 baseAddress:[gameCore buffer]
																		 bytesPerRow:([gameCore width] * 4 )
																	 releaseCallback:_BufferReleaseCallback
																	  releaseContext:nil
																		  colorSpace:CGColorSpaceCreateWithName(kCGColorSpaceGenericRGBLinear)
																	shouldColorMatch:YES];
					
				}
			default:
				{
					provider = [context outputImageProviderFromBufferWithPixelFormat:QCPlugInPixelFormatARGB8
																		  pixelsWide:[gameCore width]
																		  pixelsHigh:[gameCore height]
																		 baseAddress:[gameCore buffer]
																		 bytesPerRow:([gameCore width] * 4 )
																	 releaseCallback:_BufferReleaseCallback
																	  releaseContext:nil
																		  colorSpace:CGColorSpaceCreateWithName(kCGColorSpaceGenericRGBLinear)
																	shouldColorMatch:YES];
					
				}
				break;
		}
	}	
	*/
																		
		/*
		// BELOW IS OPEN GL METHOD. SEEMS TO BREAK WITH kCGColorSpaceGenericRGB vs kCGColorSpaceGenericRGBLinear ? But only for Gambatte ?!	
		*/	
		
		glEnable( GL_TEXTURE_RECTANGLE_EXT );
		
		GLenum status;
		GLuint texName;
		glGenTextures(1, &texName);
						
		glBindTexture( GL_TEXTURE_RECTANGLE_EXT, texName);
		glTexImage2D( GL_TEXTURE_RECTANGLE_EXT, 0, [gameCore internalPixelFormat], [gameCore width], [gameCore height], 0, [gameCore pixelFormat], [gameCore pixelType], [gameCore buffer]);
					
		// Check for OpenGL errors 
		status = glGetError();
		if(status)
		{
			NSLog(@"OpenGL error %04X", status);
			glDeleteTextures(1, &texName);
			texName = 0;
		}
		
//		glFlush();		
		glFlushRenderAPPLE();

	#if __BIG_ENDIAN__
		provider = [context outputImageProviderFromTextureWithPixelFormat:QCPlugInPixelFormatARGB8 
															   pixelsWide:[gameCore width]
															   pixelsHigh:[gameCore height]
																	 name:texName 
																  flipped:YES 
														  releaseCallback:_TextureReleaseCallback 
														   releaseContext:NULL
															   colorSpace:CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB)
														 shouldColorMatch:YES];		
	#else 
		provider = [context outputImageProviderFromTextureWithPixelFormat:QCPlugInPixelFormatBGRA8  
															   pixelsWide:[gameCore width]
															   pixelsHigh:[gameCore height]
																	 name:texName 
																  flipped:YES 
														  releaseCallback:_TextureReleaseCallback 
														   releaseContext:NULL 
															   colorSpace:CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB)
														 shouldColorMatch:YES];
	#endif

	}

	// output OpenEmu Texture - note we CAN output a nil image. This is 'correct'
	self.outputImage = provider;
	
//	[gameLock unlock];
	CGLUnlockContext([context CGLContextObj]);

	return YES;

	
}

- (void) disableExecution:(id<QCPlugInContext>)context
{
	NSLog(@"called disableExecution");

	// if we have a ROM running and the patch's image output is disconnected, pause the emulator and audio
	if(loadedRom)
	{
		[gameAudio pauseAudio];
		[gameCore pause:YES]; 
//		sleep(0.5); // race condition workaround. 
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
		[gameCore pause:YES]; 
	}
}

# pragma mark -

-(BOOL) controllerDataValidate:(NSArray*) cData
{
	// sanity check
	if([cData count] == 2 && [[cData objectAtIndex:1] count] == 12)
	{
//		NSLog(@"validated controller data");
		return YES;
	}	
	
	return NO;
}

- (void) loadRom:(NSString*) romPath
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
			[gameCore stop];
			[gameAudio stopAudio];
			//	[gameCore release];
			
			//	[gameBuffer release];
			//	[gameAudio release];
			
			NSLog(@"released/cleaned up for new rom");
			
		}
		loadedRom = NO;
		
		NSBundle* loadBundle = [self bundleForType:extension];
		
		NSLog(@"Bundle loaded for path is: %@", [loadBundle bundlePath]);
		
		gameCore = [[[loadBundle principalClass] alloc] init];
		
		NSLog(@"Loaded bundle. About to load rom...");
		
		loadedRom = [gameCore load:theRomPath withParent:(NSDocument*)self ];
		
		if(loadedRom)
		{
			NSLog(@"Loaded new Rom: %@", theRomPath);
			[gameCore setup];
			
			//	gameBuffer = [[GameBuffer alloc] initWithGameCore:gameCore];
			//	[gameBuffer setFilter:eFilter_None];
			// audio!
			gameAudio = [[GameAudio alloc] initWithCore:gameCore];
			NSLog(@"initialized audio");
			
			// starts the threaded tick callback
			[gameCore start];
			
			NSLog(@"About to start audio");
			[gameAudio startAudio];
			[gameAudio setVolume:[self inputVolume]];
			
			NSLog(@"finished loading/starting rom");
			
		}	
		else
		{
			NSLog(@"ROM did not load.");
		}
	}
	else {
		NSLog(@"bad ROM path or filename");
	}
	
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

	//	NSLog(@"Player Number: %u", [playerNumber intValue]);

		NSUInteger i;
		for(i = 0; i < [controllerArray count]; i++)
		{
	//		NSLog(@"index is %u", i);
			if([[controllerArray objectAtIndex:i] boolValue] == TRUE) // down
			{
	//			NSLog(@"button %u is down", i);
				[gameCore buttonPressed:i forPlayer:[playerNumber intValue]];
			}		
			else if([[controllerArray objectAtIndex:i] boolValue] == FALSE) // up
			{
	//			NSLog(@"button %u is up", i);
				[gameCore buttonRelease:i forPlayer:[playerNumber intValue]];
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
		[gameCore saveState: fileName];
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
			[gameCore loadState: fileName];
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

// note: the version of loadState below is my aborted attempt at a way too complicated version. its background spotlight-based approach might be good for OpenEmu.app -- dmw
//- (BOOL) loadState: (NSString *) fileName
//{
//	BOOL isDir;
//	NSUInteger index = 0;
//	
//	NSLog(@"loadState path is %@", fileName);
//	
//	if([[NSFileManager defaultManager] fileExistsAtPath:fileName isDirectory:&isDir] && !isDir)
//	{
//		//DO NOT CONCERN YOURSELF WITH EFFICIENCY OR ELEGANCE AT THIS JUNCTURE, DANIEL MORGAN WINCKLER.
//		
//		// note for later: add a step checking the load name against previously loaded names
//		NSString* theFilePath = [fileName stringByDeletingLastPathComponent];
//		NSString* theFileName = [fileName lastPathComponent];
//		NSString* theFileExtension = [fileName pathExtension];
//		NSLog(@"the file extension is %@", theFileExtension);
//		
//		if([theFileExtension caseInsensitiveCompare:@"sav"] != 0) 
//		{
//			NSLog(@"Save files must have the extension \".sav\".");
//			return NO;
//		}
//		
//		NSLog(@"loadState file name is %@", theFileName);
//		
//		NSArray* theFileNamePieces = [theFileName componentsSeparatedByString:@"."];
//		NSLog(@"loadState file name pieces separated by a '.' are %@", theFileNamePieces);
//		
//		//search the filename pieces for our known type extensions.  
//		for (NSString* aPiece in theFileNamePieces)
//		{
//			NSLog(@"aPiece is %@", aPiece);
//			NSString* ext;
//			
//			for (ext in validExtensions)
//			{
//				NSLog(@"ext is %@",ext);
//				
//				if([aPiece caseInsensitiveCompare:ext] == 0) 
//				{
//					break;
//				}
//				
//			}
//			
//		if([aPiece caseInsensitiveCompare:ext] == 0) 
//			{
//				break;
//			}
//			index++;
//			NSLog(@"index is %i",index);
//
////			NSLog(@"a supported extension was not found in the loaded state/save filename.");
//		}
//		//if a known extension is in the filename, get the ROM name from the preceding part of the filename
//		
//		NSLog(@"object at index %i is %@",index,[theFileNamePieces objectAtIndex:index]);
//		NSString* romExtensionFromFileName = [theFileNamePieces objectAtIndex:index];
//		NSLog(@"rom extension from file name is %@", romExtensionFromFileName);
//		
//		NSRange range = [theFileName rangeOfString:romExtensionFromFileName	options:NSCaseInsensitiveSearch];
//		int endOfExtension = (range.location + range.length);
//
////		NSLog(@"endOfExtension is %i", endOfExtension);
//		
////		NSEnumerator *enumerator = [theFileNamePieces allObjects];
//		NSString* romNameFromFileName = [theFileName substringToIndex:endOfExtension];
//		NSLog(@"romNameFromFileName is %@", romNameFromFileName);
//
////		for (NSString *aPiece in theFileNamePieces) {
////			NSLog(@"aPiece is %@", aPiece);
////			if([romNameFromFileName isEqualToString:@""]) {
////				[romNameFromFileName stringWithString:aPiece];
////				NSLog(@"initalizing romnamefromfilename as %@", romNameFromFileName);
////			}
////				
////			[romNameFromFileName stringByAppendingString:aPiece];
////			NSLog(@"string we're building is %@", romNameFromFileName);
////			
////			if(aPiece == romExtensionFromFileName)
////			{
////				break;
////			}
////		}
//				
//		//if loadedRom == YES, check to see if the new ROM is the same as the loaded ROM
//		if(loadedRom) {
//			if([romNameFromFileName caseInsensitiveCompare:[theRomPath lastPathComponent]] == 0) 
//			{
//				//if it's the same, just load the state
//				[gameCore loadState: fileName]; 
//				NSLog(@"ROM is the same as currently loaded ROM, loading saved state %@",theFileName);
//			}
//			else {
//				//if it's different, stop the gameCore and such
//				[gameCore stop];
//				[gameAudio stopAudio];
//				
//				NSLog(@"released/cleaned up for newly loaded state");
//			}
//		} 
//		//else if loadedRom == NO,
//		else {
//			// future feature: see if the ROM is one that has been loaded successfully in this session (NSDictionary)
//
//			
//			// look for the ROM relative to our 
//			
//			//note for future: maybe look for the ROM with a Spotlight query instead.  I started to write this but I got bogged down and decided to keep it simple for now. (look for kMDItemFSName.  might need kMDItemPath)
//						
//			//if the ROM is found, load it with loadRom
//			
//		}
//		
//		
//		
//		
//			//end if
//		
//		// then load the new ROM
//		
//		//if the ROM loads properly, load the state file
//		
//	
//		
//		[gameCore loadState: fileName]; 
//		return YES;
//	}
//	else 
//	{
//		NSLog(@"loadState: bad path or filename");
//		return NO;
//	} 
//}

//-(void)startRomQuery:(NSString*)romName 
//{
//	NSString* theRomExtension = [romName pathExtension];
//	id predicate = [NSPredicate predicateWithFormat:@"(kMDItemDisplayName like[c] '*.%@')",romName];
//	[query setPredicate:predicate];
//	
//}

@end
