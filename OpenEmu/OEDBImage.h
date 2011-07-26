//
//  ODEBImage.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 08.07.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface OEDBImage : NSManagedObject

#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName;
+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;
#pragma mark -
+ (id)newFromImage:(NSImage*)image inContext:(NSManagedObjectContext*)context;
+ (id)newFromPath:(NSString*)path inContext:(NSManagedObjectContext*)context;
+ (id)newFromURL:(NSURL*)url inContext:(NSManagedObjectContext*)context;
+ (id)newFromData:(NSData*)data inContext:(NSManagedObjectContext*)context;
// returns image with highest resolution
- (NSImage*)image;
- (NSImage*)imageForSize:(NSSize)size;

// generates thumbnail to fill size
- (void)generateImageForSize:(NSSize)size;
@end
