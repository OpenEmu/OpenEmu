//
//  OEDBImageThumbnail.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 04.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>
#import "OEDBItem.h"
@class OEDBImage, OELibraryDatabase;
@interface OEDBImageThumbnail : OEDBItem

- (id)initWithImage:(NSImage*)image size:(NSSize)size inLibrary:(OELibraryDatabase*)library;

@property (nonatomic, retain) NSNumber * height;
@property (nonatomic, retain) NSNumber * width;
@property (nonatomic, retain) NSString * path;
@property (nonatomic, retain) OEDBImage *image;

#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName;
+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;

@end
