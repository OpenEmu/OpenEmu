//
//  OEDBRom.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
@class OELibraryDatabase;
@interface OEDBGame : NSManagedObject <NSPasteboardWriting>{
}
+ (id)gameWithArchiveDictionary:(NSDictionary*)gameInfo inDatabase:(OELibraryDatabase*)database;
#pragma mark -
- (void)setBoxImageByImage:(NSImage*)img;
- (void)setBoxImageByURL:(NSURL*)url;
#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName;
+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;

- (void)updateInfoInBackground;
@end
