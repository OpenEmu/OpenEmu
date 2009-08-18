//
//  OEROMOrganizer.h
//  OpenEmu
//
//  Created by Steve Streza on 8/15/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <CoreData/CoreData.h>

@interface OEROMOrganizer : NSWindowController {
	NSPersistentStoreCoordinator *persistentStoreCoordinator;
    NSManagedObjectModel *managedObjectModel;
    NSManagedObjectContext *managedObjectContext;	
	
	IBOutlet NSArrayController *allROMSController;
}

-(IBAction)addROM:sender;
-(IBAction)removeROM:sender;
-(IBAction)playROMs:sender;

@end
