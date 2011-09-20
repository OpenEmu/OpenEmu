//
//  OEDBRom.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEDBGame.h"
#import "OEDBImage.h"

#import "ArchiveVG.h"
#import "LibraryDatabase.h"
@interface OEDBGame (Private)
- (void)_performUpdate;
+ (void)_cpyValForKey:(const NSString*)keyA of:(NSDictionary*)dictionary toKey:(const NSString*)keyB ofGame:(OEDBGame*)game;
@end
@implementation OEDBGame

- (id)copyWithZone:(NSZone *)zone{
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

+ (id)gameWithArchiveDictionary:(NSDictionary*)gameInfo inDatabase:(LibraryDatabase*)database{
	NSManagedObjectContext* context = [database managedObjectContext];
	NSEntityDescription* description = [NSEntityDescription entityForName:@"Game" inManagedObjectContext:context];
	
	NSNumber* archiveID = [gameInfo valueForKey:(NSString*)AVGGameIDKey];
	OEDBGame* resultGame = [database gameWithArchiveID:archiveID];
	if(resultGame){
		// TODO: Merge gameInfo and game
		return resultGame;
	}
	
	if([[gameInfo valueForKey:(NSString*)AVGGameIDKey] intValue]==0)
		return nil;
	
	// Create new game
	
	// TODO: Merge full info
	resultGame = [[OEDBGame alloc] initWithEntity:description insertIntoManagedObjectContext:context];
	
	[resultGame setPrimitiveValue:[gameInfo valueForKey:(NSString*)AVGGameIDKey] forKey:@"archiveID"];
	[resultGame setValue:[gameInfo valueForKey:(NSString*)AVGGameTitleKey] forKey:@"name"];
	[resultGame setValue:[NSDate date] forKey:@"lastArchiveSync"];
	[resultGame setValue:[NSDate date] forKey:@"addedDate"];
	
	NSString* boxURLString = [gameInfo valueForKey:(NSString*)AVGGameBoxURLKey];
	if(boxURLString){
		NSURL* url = [NSURL URLWithString:boxURLString];
		[resultGame setBoxImageByURL:url];
	}
		
	NSString* gameDescription = [gameInfo valueForKey:(NSString*)AVGGameDescriptionKey];
	if(gameDescription){
		[resultGame setValue:gameDescription forKey:@"gameDescription"];
	}
	
	return [resultGame autorelease];
}
#pragma mark -
- (void)setBoxImageByImage:(NSImage*)img{
	OEDBImage* boxImage = [self valueForKey:@"box"];
    if(boxImage!=nil){
		[[boxImage managedObjectContext] deleteObject:boxImage];
    }
	
	NSManagedObjectContext* context = [self managedObjectContext];
    boxImage = [OEDBImage newFromImage:img inContext:context];
	
	NSUserDefaults* standardDefaults = [NSUserDefaults standardUserDefaults];
	NSArray* sizes = [standardDefaults objectForKey:UDBoxSizesKey];
	// For each thumbnail size...
	for(NSString* aSizeString in sizes){
		NSSize size = NSSizeFromString(aSizeString);
		// ...generate thumbnail ;)
		[boxImage generateImageForSize:size];	
	}
	
	[self setValue:boxImage forKey:@"box"];
	[boxImage release];
}
- (void)setBoxImageByURL:(NSURL*)url{
	NSImage* img = [[NSImage alloc] initWithContentsOfURL:url];
	[self setBoxImageByImage:img];
	[img release];
}
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

+ (void)_cpyValForKey:(NSString*)keyA of:(NSDictionary*)dictionary toKey:(NSString*)keyB ofGame:(OEDBGame*)game{
	if([dictionary valueForKey:keyA])
		[game setValue:[dictionary valueForKey:keyA] forKey:keyB];
}

@end
