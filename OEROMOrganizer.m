//
//  OEROMOrganizer.m
//  OpenEmu
//
//  Created by Steve Streza on 8/15/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "OEROMOrganizer.h"
#import "GameDocumentController.h"
#import "OECorePlugin.h"
#import "OEROMFile.h"

@implementation OEROMOrganizer

/**
 Creates, retains, and returns the managed object model for the application 
 by merging all of the models found in the application bundle.
 */

- (NSManagedObjectModel *)managedObjectModel {
	
    if (managedObjectModel != nil) {
        return managedObjectModel;
    }
	
    managedObjectModel = [[NSManagedObjectModel mergedModelFromBundles:nil] retain];    
    return managedObjectModel;
}


/**
 Returns the persistent store coordinator for the application.  This 
 implementation will create and return a coordinator, having added the 
 store for the application to it.  (The folder for the store is created, 
 if necessary.)
 */

- (NSPersistentStoreCoordinator *) persistentStoreCoordinator {
	
    if (persistentStoreCoordinator != nil) {
        return persistentStoreCoordinator;
    }
	
    NSFileManager *fileManager;
    NSString *applicationSupportFolder = nil;
    NSURL *url;
    NSError *error;
    
    fileManager = [NSFileManager defaultManager];
    applicationSupportFolder = [[NSApp delegate] applicationSupportFolder];
    if ( ![fileManager fileExistsAtPath:applicationSupportFolder isDirectory:NULL] ) {
        [fileManager createDirectoryAtPath:applicationSupportFolder attributes:nil];
    }
	
	persistentStoreCoordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel: [self managedObjectModel]];
	if(![persistentStoreCoordinator addPersistentStoreWithType:NSXMLStoreType
												 configuration:nil
														   URL:[NSURL fileURLWithPath:[applicationSupportFolder stringByAppendingPathComponent:@"ROMs.xml"] isDirectory:NO]
													   options:nil
														 error:&error]){
		NSLog(@"Persistent store fail %@",error);
		[[NSApplication sharedApplication] presentError:error];
	}
    return persistentStoreCoordinator;
}


/**
 Returns the managed object context for the application (which is already
 bound to the persistent store coordinator for the application.) 
 */

- (NSManagedObjectContext *) managedObjectContext {
	
    if (managedObjectContext != nil) {
        return managedObjectContext;
    }
	
    NSPersistentStoreCoordinator *coordinator = [self persistentStoreCoordinator];
    if (coordinator != nil) {
        managedObjectContext = [[NSManagedObjectContext alloc] init];
        [managedObjectContext setPersistentStoreCoordinator: coordinator];
    }
    
    return managedObjectContext;
}

-(IBAction)addROM:sender{
	NSOpenPanel *panel = [[NSOpenPanel openPanel] retain];
	[panel setAllowedFileTypes:[OECorePlugin supportedTypeExtensions]];
	[panel setAllowsMultipleSelection:YES];
	
	[panel beginSheetForDirectory:nil file:nil modalForWindow:[self window]
					modalDelegate:self didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
					  contextInfo:self];
}

-(IBAction)playROMs:sender{
	GameDocumentController *controller = (GameDocumentController *)[NSApp delegate];
	NSError *error = nil;
	
	NSArray *romFiles = [allROMSController selectedObjects];
	for(OEROMFile *romFile in romFiles){
		if(![controller openDocumentWithContentsOfURL:[romFile pathURL] display:YES error:&error] || error){
			NSLog(@"Error! %@",error);
		}else{
			[romFile setLastPlayedDate:[NSDate date]];
		}
	}
	
	[self save];
}

-(void)save{
	NSError *error = nil;
	[[self managedObjectContext] save:&error];
	if(error){ 
		NSLog(@"Couldn't save! %@",error);
	}	
}

- (void)openPanelDidEnd:(NSOpenPanel *)openPanel returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo{
	if(returnCode == NSOKButton){
		NSString *basePath = [openPanel directory];
		NSArray *filenames = [openPanel filenames];
		for(NSString *file in filenames){
			NSString *path = [basePath stringByAppendingPathComponent:file];

			OEROMFile *romFile = [OEROMFile fileWithPath:path createIfNecessary:YES inManagedObjectContext:[self managedObjectContext]];
		}

		[self save];
	}
	
	[openPanel release];
}

- (id) init
{
	return [super initWithWindowNibName:@"OEROMOrganizer"];
}

- (id)initWithWindowNibName:(NSString *)aName
{
	return [self init];
}

-(void)dealloc{
	[super dealloc];
}

@end
