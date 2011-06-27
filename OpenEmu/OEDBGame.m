//
//  OEDBRom.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEDBGame.h"

@interface OEDBGame (Private)
- (void)_performUpdate;
@end
@implementation OEDBGame

- (id)copyWithZone:(NSZone *)zone
{
    NSLog(@"-----------");
    NSLog(@"-----------");
    NSLog(@"-----------");
    NSLog(@"-----------");
    NSLog(@"-----------");
    NSLog(@"-----------");
    NSLog(@"-----------");
    NSLog(@"-----------");
    NSLog(@"Game was copied with zone");
	
    return [self retain];
}

/*
 - (id)initWithPasteboardPropertyList:(id)propertyList ofType:(NSString *)type{}
 + (NSArray *)readableTypesForPasteboard:(NSPasteboard *)pasteboard{}
 + (NSPasteboardReadingOptions)readingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard{
 return 0;
 }
 */

#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName{
    return @"System";
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}

#pragma mark -
#pragma mark NSPasteboardWriting
- (NSArray *)writableTypesForPasteboard:(NSPasteboard *)pasteboard{
	return [NSArray arrayWithObject:@"org.openEmu.rom"];
}
 
- (NSPasteboardWritingOptions)writingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard{
	return 0;
}

- (id)pasteboardPropertyListForType:(NSString *)type{
    // TODO: return appropriate obj
    NSLog(@"OEDBGame pasteboardPropertyListForType");
    return @"";
}

- (void)updateInfoInBackground{
    [self performSelectorInBackground:@selector(_performUpdate) withObject:nil];
}

#pragma mark -
#pragma mark Private
- (void)_performUpdate{
    // get file checksum if none exists
    // contact archive, get infos
}

@end
