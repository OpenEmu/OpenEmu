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
#import "OELibraryDatabase.h"
@interface OEDBGame (Private)
- (void)_performUpdate;
+ (void)_cpyValForKey:(const NSString*)keyA of:(NSDictionary*)dictionary toKey:(const NSString*)keyB ofGame:(OEDBGame*)game;
@end
@implementation OEDBGame

- (id)copyWithZone:(NSZone *)zone
{
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

+ (id)gameWithArchiveDictionary:(NSDictionary*)gameInfo inDatabase:(OELibraryDatabase*)database
{
    NSManagedObjectContext* context = [database managedObjectContext];
    NSEntityDescription* description = [NSEntityDescription entityForName:@"Game" inManagedObjectContext:context];
    
    NSNumber* archiveID = [gameInfo valueForKey:(NSString*)AVGGameIDKey];
    OEDBGame* resultGame = [database gameWithArchiveID:archiveID];
    if(resultGame)
    {
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
    if(boxURLString)
    {
        NSURL* url = [NSURL URLWithString:boxURLString];
        [resultGame setBoxImageByURL:url];
    }
    
    NSString* gameDescription = [gameInfo valueForKey:(NSString*)AVGGameDescriptionKey];
    if(gameDescription)
    {
        [resultGame setValue:gameDescription forKey:@"gameDescription"];
    }
    
    return [resultGame autorelease];
}
#pragma mark -
- (void)setBoxImageByImage:(NSImage*)img
{
    OEDBImage* boxImage = [self valueForKey:@"boxImage"];
    if(boxImage!=nil)
    {
        [[boxImage managedObjectContext] deleteObject:boxImage];
    }
    
    NSManagedObjectContext* context = [self managedObjectContext];
    boxImage = [OEDBImage newFromImage:img inContext:context];
    
    NSUserDefaults* standardDefaults = [NSUserDefaults standardUserDefaults];
    NSArray* sizes = [standardDefaults objectForKey:UDBoxSizesKey];
    // For each thumbnail size...
    for(NSString* aSizeString in sizes)
    {
        NSSize size = NSSizeFromString(aSizeString);
        // ...generate thumbnail ;)
        [boxImage generateImageForSize:size];
    }
    
    [self setValue:boxImage forKey:@"boxImage"];
    [boxImage release];
}
- (void)setBoxImageByURL:(NSURL*)url
{
    NSImage* img = [[NSImage alloc] initWithContentsOfURL:url];
    [self setBoxImageByImage:img];
    [img release];
}

- (NSDate*)lastPlayed
{
    NSLog(@"OEDBGameStatusOK: %d", OEDBGameStatusOK);
    
    NSArray* roms = [self valueForKey:@"roms"];
    NSArray* sortedByLastPlayed = [roms sortedArrayUsingComparator:^NSComparisonResult(id obj1, id obj2) {
        NSDate* d1 = [obj1 valueForKey:@"lastPlayed"], *d2=[obj2 valueForKey:@"lastPlayed"];
        return [d1 compare:d2];
    }];
    if(![sortedByLastPlayed count])
        return nil;
    
    return [[sortedByLastPlayed lastObject] valueForKey:@"lastPlayed"];
}
#pragma mark -
- (void)mergeWithGameInfo:(NSDictionary*)archiveGameDict
{  
    if([[archiveGameDict valueForKey:(NSString*)AVGGameIDKey] intValue]==0)
    return;
    
    [self setValue:[archiveGameDict valueForKey:(NSString*)AVGGameIDKey] forKey:@"archiveID"];
    [self setValue:[archiveGameDict valueForKey:(NSString*)AVGGameTitleKey] forKey:@"name"];
    [self setValue:[NSDate date] forKey:@"lastArchiveSync"];
    [self setValue:[NSDate date] forKey:@"addedDate"];
    
    NSString* boxURLString = [archiveGameDict valueForKey:(NSString*)AVGGameBoxURLKey];
    if(boxURLString)
    {
        NSURL* url = [NSURL URLWithString:boxURLString];
        [self setBoxImageByURL:url];
    }
    
    NSString* gameDescription = [archiveGameDict valueForKey:(NSString*)AVGGameDescriptionKey];
    if(gameDescription)
    {
        [self setValue:gameDescription forKey:@"gameDescription"];
    }
}

#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName
{
    return @"Game";
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context
{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}

#pragma mark -
#pragma mark NSPasteboardWriting
- (NSArray *)writableTypesForPasteboard:(NSPasteboard *)pasteboard
{
    return [NSArray arrayWithObject:@"org.openEmu.rom"];
}

- (NSPasteboardWritingOptions)writingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard
{
    return 0;
}

- (id)pasteboardPropertyListForType:(NSString *)type
{
    // TODO: return appropriate obj
    NSLog(@"OEDBGame pasteboardPropertyListForType");
    return @"";
}

- (void)updateInfoInBackground
{
    [self performSelectorInBackground:@selector(_performUpdate) withObject:nil];
}

#pragma mark -
#pragma mark Private
- (void)_performUpdate
{
    // TODO: get file checksum if none exists
    // TODO: contact archive, get infos
}

+ (void)_cpyValForKey:(NSString*)keyA of:(NSDictionary*)dictionary toKey:(NSString*)keyB ofGame:(OEDBGame*)game
{
    if([dictionary valueForKey:keyA])
        [game setValue:[dictionary valueForKey:keyA] forKey:keyB];
}
@end
