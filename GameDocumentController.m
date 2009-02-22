//
//  GameDocumentController.m
//  Gambatte
//
//  Created by Ben on 24/08/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "GamePickerController.h"
#import "GamePreferencesController.h"
#import "GameDocumentController.h"
#import "GameDocument.h"
#import "GameButton.h"
#import "GameCore.h"
#import <Sparkle/Sparkle.h>
#import <XADMaster/XADArchive.h>

@implementation GameDocumentController

@synthesize gameLoaded;
@synthesize bundles;


+ (NSDictionary*) defaultGamepadControls
{
	GameButton *rightButton = [[[GameButton alloc] initWithPage:kHIDPage_GenericDesktop
														 usage:kHIDUsage_GD_X 
														 value:1
													 forButton:eButton_RIGHT player:0] autorelease];
	
	GameButton *leftButton = [[[GameButton alloc] initWithPage:kHIDPage_GenericDesktop
														usage:kHIDUsage_GD_X
														value:-1
													forButton:eButton_LEFT player:0] autorelease];
	
	
	GameButton *upButton =[[[GameButton alloc] initWithPage:kHIDPage_GenericDesktop
													 usage:kHIDUsage_GD_Y 
													 value:-1
												 forButton:eButton_UP player:0] autorelease];
	
	
	GameButton *downButton = [[[GameButton alloc] initWithPage:kHIDPage_GenericDesktop
														usage:kHIDUsage_GD_Y 
														value:1
													forButton:eButton_DOWN player:0] autorelease];
	
	GameButton *aButton = [[[GameButton alloc] initWithPage:kHIDPage_Button
													 usage:2 
													 value:1
												 forButton:eButton_A player:0] autorelease];

	GameButton *bButton = [[[GameButton alloc] initWithPage:kHIDPage_Button
													 usage:1
													 value:1
												 forButton:eButton_B player:0] autorelease];
	
	
	GameButton *xButton = [[[GameButton alloc] initWithPage:kHIDPage_Button
													  usage:6 
													  value:1
												  forButton:eButton_X player:0] autorelease];
	
	GameButton *yButton = [[[GameButton alloc] initWithPage:kHIDPage_Button
													  usage:5
													  value:1
												  forButton:eButton_Y player:0] autorelease];
	
	GameButton *startButton =[[[GameButton alloc] initWithPage:kHIDPage_Button
														usage:4 
														value:1
													forButton:eButton_START player:0] autorelease];
	
	
	GameButton *selectButton = [[[GameButton alloc] initWithPage:kHIDPage_Button
														  usage:3 
														  value:1
													  forButton:eButton_SELECT player:0] autorelease];
	
	GameButton *lButton =[[[GameButton alloc] initWithPage:kHIDPage_Button
														 usage:9 
														 value:1
													 forButton:eButton_L player:0] autorelease];
	
	
	GameButton *rButton = [[[GameButton alloc] initWithPage:kHIDPage_Button
														   usage:8 
														   value:1
													   forButton:eButton_R player:0] autorelease];
	// Create defaults
	return [NSDictionary dictionaryWithObjects:[NSArray arrayWithObjects:aButton,bButton,xButton,yButton,startButton,selectButton,upButton,downButton,leftButton,rightButton,lButton, rButton, nil]
									   forKeys:[NSArray arrayWithObjects:@"A",@"B",@"X",@"Y",@"Start",@"Select",@"Up",@"Down",@"Left",@"Right", @"L", @"R",nil]];	
}

+ (NSDictionary*) defaultControls
{
	/*
	NSArray *aArray		=	[NSArray arrayWithObject:[NSNumber numberWithInt:0]];
	NSArray *bArray		=	[NSArray arrayWithObject:[NSNumber numberWithInt:1]];
	
	NSArray *startArray		=	[NSArray arrayWithObject:[NSNumber numberWithInt:36]];
	NSArray *selectArray	=	[NSArray arrayWithObject:[NSNumber numberWithInt:48]];
	
	NSArray *upArray	=	[NSArray arrayWithObject:[NSNumber numberWithInt:126]];
	NSArray *downArray	=	[NSArray arrayWithObject:[NSNumber numberWithInt:125]];
	NSArray *leftArray	=	[NSArray arrayWithObject:[NSNumber numberWithInt:123]];
	NSArray *rightArray	=	[NSArray arrayWithObject:[NSNumber numberWithInt:124]];
	
	NSArray *lArray	=	[NSArray arrayWithObject:[NSNumber numberWithInt:36]];
	NSArray *rArray	=	[NSArray arrayWithObject:[NSNumber numberWithInt:48]];
	*/
	
	KeyboardButton * aButton = [[[KeyboardButton alloc] initWithKeycode:0]autorelease];
	KeyboardButton * bButton = [[[KeyboardButton alloc] initWithKeycode:1]autorelease];
	KeyboardButton * xButton = [[[KeyboardButton alloc] initWithKeycode:6]autorelease];
	KeyboardButton * yButton = [[[KeyboardButton alloc] initWithKeycode:7]autorelease];	
	
	KeyboardButton * startButton = [[[KeyboardButton alloc] initWithKeycode:36]autorelease];
	KeyboardButton * selectButton = [[[KeyboardButton alloc] initWithKeycode:48]autorelease];
	
	KeyboardButton * upButton = [[[KeyboardButton alloc] initWithKeycode:126]autorelease];
	KeyboardButton * downButton = [[[KeyboardButton alloc] initWithKeycode:125]autorelease];
	KeyboardButton * leftButton = [[[KeyboardButton alloc] initWithKeycode:123]autorelease];
	KeyboardButton * rightButton = [[[KeyboardButton alloc] initWithKeycode:124]autorelease];
	
	KeyboardButton * lButton = [[[KeyboardButton alloc] initWithKeycode:12]autorelease];
	KeyboardButton * rButton = [[[KeyboardButton alloc] initWithKeycode:13]autorelease];
	
	// Create defaults
	return [NSDictionary dictionaryWithObjects:[NSArray arrayWithObjects:aButton,bButton,xButton, yButton,startButton,selectButton,upButton,downButton,leftButton,rightButton, lButton, rButton, nil] 
																 forKeys:[NSArray arrayWithObjects:@"A",@"B",@"X",@"Y",@"Start",@"Select",@"Up",@"Down",@"Left",@"Right", @"L", @"R", nil]];

}

+ (void)initialize
{
	if(self  == [GameDocumentController class])
	{
		// Create a dictionary
		NSMutableDictionary *defaultValues = [NSMutableDictionary dictionary];
		
		NSData* gamepadControls = [NSKeyedArchiver archivedDataWithRootObject:[GameDocumentController defaultGamepadControls]];
		NSData* keyboardControls = [NSKeyedArchiver archivedDataWithRootObject:[GameDocumentController defaultControls]];
		
		[defaultValues setObject:gamepadControls forKey:OEGamepadControls];
		[defaultValues setObject:keyboardControls forKey:OEGameControls];
		[defaultValues setValue:NO forKey:OEFullScreen];
		[defaultValues setValue:0 forKey:OEFilter];
		[defaultValues setValue:[NSNumber numberWithFloat:1.0] forKey:OEVolume];
		[defaultValues setValue:NO forKey:OEPauseBackground];
		
		NSLog(@"%@", defaultValues);
		[[NSUserDefaults standardUserDefaults] registerDefaults:defaultValues];
	}
}

- (void)menuNeedsUpdate:(NSMenu *)menu
{
	NSLog(@"Menu!?!?!");
	for(NSBundle * bundle in [self bundles])
	{
		NSNib * nib = [[NSNib alloc] initWithNibNamed:@"Menu" bundle:bundle];
		NSArray * objects;
		if([nib instantiateNibWithOwner:self topLevelObjects:&objects])
		{
			for(NSObject*object in objects)
			{
				if([object isKindOfClass:[NSMenuItem class]])
				{
					[menu addItem:(NSMenuItem*) object];
					NSLog(@"Adding item to menu?");
								
				}
			}
		}
	}
}

-(void)updateBundles: (id) sender
{
	for(NSBundle* bundle in bundles)
	{
		@try {
			if( [[SUUpdater updaterForBundle:bundle] feedURL] )
			{
				[[SUUpdater updaterForBundle:bundle] resetUpdateCycle];
		//		[[SUUpdater updaterForBundle:bundle] checkForUpdates:self];
			}
		}
		@catch (NSException * e) {
			NSLog(@"Tried to update bundle without sparkle");
		}
	}
}

- (id)init
{
	self = [super init];
	if(self) {
		[self setGameLoaded:NO];
		
		NSString *file;
		NSString *bundleDir = [[[[NSBundle mainBundle] infoDictionary] valueForKey:@"OEBundlePath"] stringByStandardizingPath];
		NSMutableArray* bundlePaths = [[NSMutableArray alloc] init];
	
		// get our core bundles
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
		
		NSMutableSet* mutableExtensions = [[NSMutableSet alloc] init];
		
		//go through the bundles Info.plist files to get the type extensions
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
		
		// When a class conforms to both NSCopying and NSMutableCopying protocols
		// -copy returns a immutable object and
		// -mutableCopy returns a mutable object.
		NSMutableArray* tempArray = [[NSMutableArray alloc] init];
		[tempArray addObjectsFromArray:[mutableExtensions allObjects]];
		validExtensions = [[NSArray arrayWithArray:tempArray] retain];
		[tempArray release];
		
		//validExtensions = [[NSArray arrayWithArray:mutableExtensions] retain];
		
		[self updateInfoPlist];
		
		[mutableExtensions release];
	}
	return self;
}

- (void) dealloc
{
	[validExtensions release];
	[bundles release];
	[super dealloc];
}

- (void) updateInfoPlist 
{
	// updates OpenEmu.app's Info.plist with the valid extensions so that users can drag ROMs onto the icon to get opened

	NSString *errorDesc = nil;
	NSPropertyListFormat format;
	
	NSString *infoPlistPath =  [[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/Contents/Info.plist"];
	
	// get Info.plist as data
	NSData * infoPlistXml = [[NSFileManager defaultManager] contentsAtPath:infoPlistPath];
	
	// store it mutably
	NSMutableDictionary *infoPlist = (NSMutableDictionary*)[NSPropertyListSerialization 
															propertyListFromData: infoPlistXml
															mutabilityOption: NSPropertyListMutableContainersAndLeaves
															format:&format
															errorDescription:&errorDesc];
	if (!infoPlist) {
		NSLog(errorDesc);
		[errorDesc release];
	}
	
	// get the current doctypes and extensions from Info.plist
	NSMutableArray* docTypes = [NSMutableArray arrayWithArray:[infoPlist objectForKey:@"CFBundleDocumentTypes"]];
	
	
	// replace extensions with validExtensions
	// FIXME: the CFBundleTypeExtensions array gets larger every time you open the app, i.e., the array is getting appended, not replaced :X
	[docTypes setValue:validExtensions forKey:@"CFBundleTypeExtensions"];
	
	DLog(@"docTypes is: %@",docTypes);
	
	// update Info.plist
	
	[infoPlist setObject:docTypes forKey: @"CFBundleDocumentTypes"];
	
	DLog(@"revised Info.plist will be %@",infoPlist);
	
	NSString* err;
	// turn it back into proper XML data
	NSData* updatedInfoPlistData = [NSPropertyListSerialization dataFromPropertyList:infoPlist
																			  format:NSPropertyListXMLFormat_v1_0
																	errorDescription:&err];
	BOOL updatedPlist;
	if(updatedInfoPlistData) 
	{
		//save it out to Info.plist
		updatedPlist = [updatedInfoPlistData writeToFile:infoPlistPath atomically:YES];
		DLog(@"updated plist?  %U", updatedPlist);
		
		[err release];
	}			
	else 
		NSLog(@"error: %@",err);	
	
}

- (id)openDocumentWithContentsOfURL:(NSURL *)absoluteURL display:(BOOL)displayDocument error:(NSError **)outError
{
	NSLog(@"URL: %@", absoluteURL);
	NSLog([absoluteURL path]);
	XADArchive* archive = [XADArchive archiveForFile:[absoluteURL path]];
	NSLog(@"Opened?");
	if(archive)
	{
		NSString *filePath;
		NSString *appSupportPath = [[[NSHomeDirectory() stringByAppendingPathComponent:@"Library"] stringByAppendingPathComponent:@"Application Support"] stringByAppendingPathComponent:@"OpenEmu"];
		if(![[NSFileManager defaultManager] fileExistsAtPath:appSupportPath])
			[[NSFileManager defaultManager] createDirectoryAtPath:appSupportPath attributes:nil];
		filePath = [appSupportPath stringByAppendingPathComponent:@"Temp Rom Extraction"];
		
		if ([archive numberOfEntries] != 1) //more than one rom in the archive
		{
			GamePickerController *c = [[GamePickerController alloc] init];
			[c setArchive:archive];
			
			if([[NSApplication sharedApplication] runModalForWindow:[c window]] == 1)
			{
				int idx = [c selectedIndex];
				NSLog(@"Selected index %d", [c selectedIndex]);
				
				if([archive extractEntry:idx to:filePath])
				{
					filePath = [filePath stringByAppendingPathComponent:[archive nameOfEntry:idx]];
					NSLog(filePath);
					absoluteURL = [NSURL fileURLWithPath:filePath];
				}
				else
				{
					NSLog(@"Failed to extract");
				}
			}
			else
			{
			//	return nil;
			}
		}
		else //only one rom in the archive
		{
			if([archive extractEntry:0 to:filePath])
			{
				
				filePath = [filePath stringByAppendingPathComponent:[archive nameOfEntry:0]];
				NSLog(filePath);
				absoluteURL = [NSURL fileURLWithPath:filePath];
			}
			else
			{
				NSLog(@"Failed to extract");
			}
		}
	}
	
	NSLog(@"Final path: %@", absoluteURL);
	//[self closeWindow: self];
	return [super openDocumentWithContentsOfURL: absoluteURL display: displayDocument error: outError];
}

-(IBAction) closeWindow: (id) sender
{
	[[self currentDocument] close];
}

- (void) setCurrentGame: (GameDocument*) game
{
	currentGame = game;
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
				if([str caseInsensitiveCompare:type    ] == 0)  //ignoring case so it doesn't matter if the extension is NES or nes or NeS or nES
				{
					NSLog(@"Found Bundle");
					return bundle;
				}
			}
		}
		
	}
	
	return nil;
}

- (NSString *)typeForContentsOfURL:(NSURL *)inAbsoluteURL error:(NSError **)outError
{
	NSLog(@"Type");
	return [[inAbsoluteURL path] pathExtension];
}

- (NSString *)displayNameForType:(NSString *)documentTypeName
{
	return [super displayNameForType:documentTypeName];
}

- (Class)documentClassForType:(NSString *)documentTypeName
{
	//Force it to load a game document
	return [GameDocument class];
}

- (NSInteger)runModalOpenPanel:(NSOpenPanel *)openPanel forTypes:(NSArray *)extensions
{
	NSLog(@"%@",validExtensions);
	return [super runModalOpenPanel:openPanel forTypes:validExtensions];
}

- (GamePreferencesController*) preferenceController
{//	if(!preferenceController)
//	{
//		preferenceController = [[GamePreferencesController alloc] initWithDocController:self];
//	}	
	return preferenceController;
}

- (GameCore*) currentGame
{
	return [(GameDocument*)[self currentDocument] gameCore];
}

- (IBAction) resetGame: (id) sender
{
	[[self currentGame] resetEmulation];
}

- (IBAction) switchFullscreen: (id) sender
{
	[[self currentDocument] switchFullscreen];
}

- (IBAction) scrambleRam: (id) sender
{
	[[self currentDocument] scrambleRam:100];
}

- (IBAction) saveState: (id) sender
{
	NSString* fileName;
	NSSavePanel* sPanel = [NSSavePanel savePanel];
	[sPanel setAlphaValue:0.95]; 
	
	[sPanel setTitle:@"Save Current State"];
	if ( [sPanel runModalForDirectory:nil file:nil] == NSOKButton ){
		fileName = [sPanel filename];
		
		[[self currentGame] saveStateToFileAtPath: fileName];
	}
}

- (IBAction) loadState: (id) sender
{
	NSString* fileName;
	NSOpenPanel* oPanel = [NSOpenPanel openPanel]; 
	[oPanel setCanChooseDirectories:NO]; 
	[oPanel setCanChooseFiles:YES]; 
	[oPanel setCanCreateDirectories:NO]; 
	[oPanel setAllowsMultipleSelection:NO]; 
	[oPanel setAlphaValue:0.95]; 
	
	[oPanel setTitle:@"Load Saved State"];
	if ( [oPanel runModalForDirectory:nil file:nil types: nil] == NSOKButton ){
		NSArray* files = [oPanel filenames];
		fileName = [files objectAtIndex:0];
		
		[[self currentGame] loadStateFromFileAtPath: fileName];
	}
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
	//delete all the temp files
	NSString *appSupportPath = [[[NSHomeDirectory() stringByAppendingPathComponent:@"Library"] stringByAppendingPathComponent:@"Application Support"] stringByAppendingPathComponent:@"OpenEmu"];
	if(![[NSFileManager defaultManager] fileExistsAtPath:appSupportPath])
		[[NSFileManager defaultManager] createDirectoryAtPath:appSupportPath attributes:nil];
	NSString* filePath = [appSupportPath stringByAppendingPathComponent:@"Temp Rom Extraction"];
	
	NSError* error = nil;
	[[NSFileManager defaultManager] removeItemAtPath:filePath error:&error];
	
	if(error)
		NSLog(@"%@",error);
	else
		NSLog(@"Deleted temp files");
}

- (BOOL)isGameKey
{
	if( [(GameDocument*)[self currentDocument] isFullScreen] )
		return YES;
	
	NSDocument * doc = [self documentForWindow:[[NSApplication sharedApplication] keyWindow]];
	return doc != nil;
	
}

@end
