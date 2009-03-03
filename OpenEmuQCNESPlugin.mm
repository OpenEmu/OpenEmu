//
//  OpenEmuQCNESPlugin.m
//  OpenEmuQCNES
//
//  A NES-only QC plugin for teh glitchy insanity.  Started by Dan Winckler on 11/16/08.
//

/* It's highly recommended to use CGL macros instead of changing the current context for plug-ins that perform OpenGL rendering */
//#import <OpenGL/CGLMacro.h>

#import "OpenEmuQCNESPlugin.h"

#import <Quartz/Quartz.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <AudioToolbox/AudioToolbox.h>

#import "GameBuffer.h"
#import "GameAudio.h"
#import "GameCore.h"
//#import "../Nestopia/NESGameEmu.h"

#define	kQCPlugIn_Name				@"OpenEmu NES"
#define	kQCPlugIn_Description		@"Wraps the OpenEmu emulator - play and manipulate the NES"

static void _TextureReleaseCallback(CGLContextObj cgl_ctx, GLuint name, void* info)
{	
	glDeleteTextures(1, &name);
}

static void _BufferReleaseCallback(const void* address, void* info)
{
	DLog(@"called buffer release callback");
	//	free((void*)address);
}

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
@dynamic inputRewinderReset;

@dynamic inputNmtRamCorrupt;
@dynamic inputNmtRamOffset;
@dynamic inputNmtRamValue;

@dynamic inputChrRamCorrupt;
@dynamic inputChrRamOffset;
@dynamic inputChrRamValue;

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
															 @"~/roms/NES/RomName.nes", QCPortAttributeDefaultValueKey, 
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
	
	if([key isEqualToString:@"inputSaveStatePath"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Save State", QCPortAttributeNameKey,
														@"~/roms/saves/savefilename", QCPortAttributeDefaultValueKey, 
														nil];

	if([key isEqualToString:@"inputLoadStatePath"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Load State", QCPortAttributeNameKey,
														@"~/roms/saves/loadsavefilename", QCPortAttributeDefaultValueKey, 
														nil];
	
	if([key isEqualToString:@"inputPauseEmulation"])
		return [NSDictionary dictionaryWithObjectsAndKeys:	@"Pause Emulator", QCPortAttributeNameKey,
				[NSNumber numberWithBool:NO], QCPortAttributeDefaultValueKey, 
				nil];
	
	if([key isEqualToString:@"inputCheatCode"])
		return [NSDictionary dictionaryWithObjectsAndKeys:	@"Cheat Code", QCPortAttributeNameKey,
				@"", QCPortAttributeDefaultValueKey, 
				nil];
	
	if([key isEqualToString:@"inputEnableRewinder"])
		return [NSDictionary dictionaryWithObjectsAndKeys:	@"Enable Rewinder", QCPortAttributeNameKey,
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

	if([key isEqualToString:@"inputRewinderReset"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Rewinder Reset", QCPortAttributeNameKey,
				[NSNumber numberWithBool:NO], QCPortAttributeDefaultValueKey, 
				nil];

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
			@"inputRewinderReset",
			@"inputNmtRamCorrupt",
			@"inputNmtRamOffset",
			@"inputNmtRamValue",
			@"inputChrRamCorrupt",
			@"inputChrRamOffset",
			@"inputChrRamValue",
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

		NSBundle *theBundle = [NSBundle bundleForClass:[self class]];
		NSDictionary *ourBundleInfo = [theBundle infoDictionary];
		NSString *nesBundleDir = [[ourBundleInfo valueForKey:@"OENESBundlePath"] stringByStandardizingPath];
		bundle = [NSBundle bundleWithPath:nesBundleDir];
		loadedRom,romFinishedLoading = NO;
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

@implementation OpenEmuQCNES (Execution)


- (BOOL) startExecution:(id<QCPlugInContext>)context
{	
	DLog(@"called startExecution");
//	if(loadedRom)
//	{
//		[gameAudio startAudio];
//		[gameCore start]; 
//	}
	
	return YES;
}

- (void) enableExecution:(id<QCPlugInContext>)context
{
	DLog(@"called enableExecution");
	// if we have a ROM loaded and the patch's image output is reconnected, unpause the emulator
	if(loadedRom && romFinishedLoading)
	{
		if(!self.inputPauseEmulation) 
		{
			[gameAudio startAudio];
			[gameCore setPauseEmulation:NO];
		}
	}
	
	/*
	Called by Quartz Composer when the plug-in instance starts being used by Quartz Composer.
	*/
}

- (BOOL) execute:(id<QCPlugInContext>)context atTime:(NSTimeInterval)time withArguments:(NSDictionary*)arguments
{
	CGLSetCurrentContext([context CGLContextObj]);
	
	// Process ROM loads
	if([self didValueForInputKeyChange: @"inputRom"] && ([self valueForInputKey:@"inputRom"] != [[OpenEmuQCNES	attributesForPropertyPortWithKey:@"inputRom"] valueForKey: QCPortAttributeDefaultValueKey]))
	{
		[self loadRom:[self valueForInputKey:@"inputRom"]];
	}
	
	if(loadedRom && romFinishedLoading) {
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
		if([self didValueForInputKeyChange: @"inputSaveStatePath"] && ([self valueForInputKey:@"inputSaveStatePath"] != [[OpenEmuQCNES attributesForPropertyPortWithKey:@"inputSaveStatePath"] valueForKey: QCPortAttributeDefaultValueKey]))
		{
			DLog(@"save path changed");
			[self saveState:[[self valueForInputKey:@"inputSaveStatePath"] stringByStandardizingPath]];
		}

		// Process state loading
		if([self didValueForInputKeyChange: @"inputLoadStatePath"] && ([self valueForInputKey:@"inputLoadStatePath"] != [[OpenEmuQCNES attributesForPropertyPortWithKey:@"inputLoadStatePath"] valueForKey: QCPortAttributeDefaultValueKey]))	
		{
			DLog(@"load path changed");
			[self loadState:[[self valueForInputKey:@"inputLoadStatePath"] stringByStandardizingPath]];
		}
		
		// Process emulation pausing 
		if([self didValueForInputKeyChange: @"inputPauseEmulation"])	
		{
			if([[self valueForInputKey:@"inputPauseEmulation"] boolValue])	
			{
				DLog(@"user paused emulation");
				[gameAudio pauseAudio];
				[gameCore setPauseEmulation:YES]; 
			}
			else 
			{
				DLog(@"user unpaused emulation");
				[gameAudio startAudio];
				[gameCore setPauseEmulation:NO];
			}
		}
		
		// Process cheat codes
		if([self didValueForInputKeyChange: @"inputCheatCode"] && ([self valueForInputKey:@"inputCheatCode"] != [[OpenEmuQCNES attributesForPropertyPortWithKey:@"inputCheatCode"] valueForKey: QCPortAttributeDefaultValueKey]))	
		{
			DLog(@"cheat code entered");
			[self setCode:[self valueForInputKey:@"inputCheatCode"]];
		}
		
		// process rewinder stuff
		if([self didValueForInputKeyChange: @"inputEnableRewinder"])	
		{
	//		DLog(@"rewinder state changed");
			[self enableRewinder:[[self valueForInputKey:@"inputEnableRewinder"] boolValue]];

			if([(NESGameEmu*)gameCore isRewinderEnabled]) 
			{
				NSLog(@"rewinder is enabled");
			} else 
			{ 
				NSLog(@"rewinder is disabled");
			}
		}
		
	//	int* rewindTimer;
	//	rewindTimer = [[NSNumber alloc] initWithUnsignedInteger:0];
	//	
	//	if([(NESGameEmu*)gameCore isRewinderEnabled]) 
	//	{
	//		rewindTimer++;
	//		if((rewindTimer % 60) == 0) {
	//		DLog(@"rewind timer count is %d",rewindTimer);
	//		}
	//	} 
		
		if([self didValueForInputKeyChange: @"inputRewinderDirection"])	
		{
	//		DLog(@"rewinder direction changed");
			[(NESGameEmu*)gameCore rewinderDirection:[[self valueForInputKey:@"inputRewinderDirection"] boolValue]];
		}
		
		if([self didValueForInputKeyChange:@"inputEnableRewinderBackwardsSound"])
		{
			[(NESGameEmu*)gameCore enableRewinderBackwardsSound:[[self valueForInputKey:@"inputEnableRewinderBackwardsSound"] boolValue]];
			
			if([(NESGameEmu*)gameCore isRewinderBackwardsSoundEnabled])
			{
				NSLog(@"rewinder backwards sound is enabled");
			}
			else 
			{
				NSLog(@"rewinder backwards sound is disabled");
			}
		}

		// CORRUPTION FTW
		if(hasNmtRam && self.inputNmtRamCorrupt && ( [self didValueForInputKeyChange:@"inputNmtRamOffset"] || [self didValueForInputKeyChange:@"inputNmtRamValue"] ))
		{
			[(NESGameEmu*)gameCore setNmtRamBytes:self.inputNmtRamOffset value:self.inputNmtRamValue];
		}
		
		if(hasChrRom && self.inputChrRamCorrupt && ( [self didValueForInputKeyChange:@"inputChrRamOffset"] || [self didValueForInputKeyChange:@"inputChrRamValue"] ))
		{
			[(NESGameEmu*)gameCore setChrRamBytes:self.inputChrRamOffset value:self.inputChrRamValue];
		}
	}
	
	// our output image
	id	provider = nil;
	
	// handle our image output. (sanity checking)
	if(loadedRom && romFinishedLoading && ([gameCore width] > 10) )
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

	return YES;
}

- (void) disableExecution:(id<QCPlugInContext>)context
{
	DLog(@"called disableExecution");

	// if we have a ROM running and the patch's image output is disconnected, pause the emulator
	if(loadedRom && romFinishedLoading)
	{
		if(!self.inputPauseEmulation) 
		{
			[gameAudio pauseAudio];
			[gameCore setPauseEmulation:YES]; 
		}
//		sleep(0.5); // race condition workaround. 
	}
	/*
	Called by Quartz Composer when the plug-in instance stops being used by Quartz Composer.
	*/
}

- (void) stopExecution:(id<QCPlugInContext>)context
{
	DLog(@"called stopExecution");
	if(loadedRom)
	{
		[gameCore stopEmulation]; 		
		[gameAudio stopAudio];
		[gameAudio release];
		[gameCore release];
		gameCore = nil;
		loadedRom = NO;
		romFinishedLoading = NO;
	}
}

# pragma mark -

-(BOOL) controllerDataValidate:(NSArray*) cData
{
	// sanity check
	if([cData count] == 2 && [[cData objectAtIndex:1] count] == 12)
	{
		DLog(@"validated controller data");
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
		DLog(@"extension is: %@", extension);
		
		// cleanup
		if(loadedRom && romFinishedLoading)
		{
			[gameAudio stopAudio];
			[gameCore stopEmulation];
			[gameCore release];
			gameCore = nil;
			//	[gameBuffer release];
			[gameAudio release];
			
			DLog(@"released/cleaned up for new rom");
			
		}
		loadedRom = NO;
		romFinishedLoading = NO;
		hasChrRom = NO;
		hasNmtRam = NO;
		
		//load NES bundle
		gameCore = [[[bundle principalClass] alloc] init];
		
		// add a pointer to NESGameEmu so we can call NES-specific methods without getting fucking warnings (doesn't work, needs fixing)
//		(NESGameEmu*)gameCore = (NESGameEmu*)gameCore;

		DLog(@"Loaded NES bundle. About to load rom...");
		
		//loadedRom = [gameCore load:theRomPath withParent:(NSDocument*)self ];
		[gameCore initWithDocument:(GameDocument*)self];
		[gameCore loadFileAtPath:theRomPath];
		loadedRom = TRUE;
		
		if(loadedRom)
		{
			NSLog(@"Loaded new Rom: %@", theRomPath);
			[gameCore setupEmulation];
			
			//	gameBuffer = [[GameBuffer alloc] initWithGameCore:gameCore];
			//	[gameBuffer setFilter:eFilter_None];
			// audio!
			gameAudio = [[GameAudio alloc] initWithCore:gameCore];
			DLog(@"initialized audio");
			
			// starts the threaded emulator timer
			[gameCore startEmulation];
			
			DLog(@"About to start audio");
			[gameAudio startAudio];
			[gameAudio setVolume:[self inputVolume]];
			
			DLog(@"finished loading/starting rom");			
			
			if([(NESGameEmu*)gameCore chrRomSize]) 
			{
				hasChrRom = YES;
				DLog(@"Reported Character ROM size is %i", [(NESGameEmu*)gameCore chrRomSize]);
			}
			else 
			{
				hasChrRom = NO;
				DLog(@"This game does not have Character ROM");
			}
			
			hasNmtRam = YES;	//because if the cartridge doesn't have VRAM, the PPU will just use its 2K RAM for the nametables
			DLog(@"Reported NMT RAM size is %i", [(NESGameEmu*)gameCore cartVRamSize]);
			
			romFinishedLoading = YES;
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

		NSUInteger i;
		for(i = 0; i < [controllerArray count]; i++)
		{
			if([[controllerArray objectAtIndex:i] boolValue] == TRUE) // down
			{
				[gameCore player:[playerNumber intValue] didPressButton:i];
			}		
			else if([[controllerArray objectAtIndex:i] boolValue] == FALSE) // up
			{
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
			[gameCore loadFileAtPath:fileName];
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

#pragma mark --Experimental Features--

- (void) setCode: (NSString*) cheatCode
{
	NSLog(@"cheat code is: %@",cheatCode);
	[(NESGameEmu*)gameCore setCode:cheatCode];
}


- (void) enableRewinder:(BOOL) rewind
{
	[(NESGameEmu*)gameCore enableRewinder:rewind];
}

@end
