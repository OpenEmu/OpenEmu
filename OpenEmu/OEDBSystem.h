//
//  OEDBConsole.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@class OESystemPlugin, OELibraryDatabase;
@interface OEDBSystem : NSManagedObject {
	NSImage* icon;
}
+ (id)createSystemFromPlugin:(OESystemPlugin*)plugin inDatabase:(OELibraryDatabase*)database;
#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName;
+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;
#pragma mark -
- (OESystemPlugin*)plugin;

- (NSImage*)icon;

@property (readonly) NSString* name;
@end
