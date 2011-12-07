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
+ (id)systemFromPlugin:(OESystemPlugin*)plugin inDatabase:(OELibraryDatabase*)database;
+ (id)systemForPluginIdentifier:(NSString*)identifier inDatabase:(OELibraryDatabase*)database;
+ (id)systemForArchiveID:(NSNumber*)number;
+ (id)systemForArchiveID:(NSNumber*)number inDatabase:(OELibraryDatabase*)database;
+ (id)systemForArchiveName:(NSString*)name;
+ (id)systemForArchiveName:(NSString*)name inDatabase:(OELibraryDatabase*)database;
+ (id)systemForArchiveShortName:(NSString*)shortName;
+ (id)systemForArchiveShortName:(NSString*)shortName inDatabase:(OELibraryDatabase*)database;
+ (id)systemForFile:(NSString*)filepath;
+ (id)systemForFile:(NSString*)filepath inDatabase:(OELibraryDatabase*)database;
#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName;
+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;
#pragma mark -
- (OESystemPlugin*)plugin;

- (NSImage*)icon;

@property (readonly) NSString* name;
@end
