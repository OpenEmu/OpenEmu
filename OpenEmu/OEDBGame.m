/*
 Copyright (c) 2011, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "OEDBGame.h"

#import "OELibraryDatabase.h"

#import "OEDBSystem.h"
#import "OEDBRom.h"

#import "OEDBImage.h"
#import "OEDBImageThumbnail.h"

#import "OEGameInfoHelper.h"

#import "NSFileManager+OEHashingAdditions.h"
#import "NSArray+OEAdditions.h"

NSString *const OEPasteboardTypeGame = @"org.openemu.game";
NSString *const OEBoxSizesKey = @"BoxSizes";
NSString *const OEDisplayGameTitle = @"displayGameTitle";

NSString *const OEGameArtworkFormatKey = @"artworkFormat";
NSString *const OEGameArtworkPropertiesKey = @"artworkProperties";

@implementation OEDBGame
@dynamic name, gameTitle, rating, gameDescription, importDate, lastInfoSync, status, displayName;
@dynamic boxImage, system, roms, genres, collections, credits;

+ (void)initialize
{
     if (self == [OEDBGame class])
     {
         [[NSUserDefaults standardUserDefaults] registerDefaults:@{
                                                                   OEBoxSizesKey:@[@"{75,75}", @"{150,150}", @"{300,300}", @"{450,450}"],
                                                                   OEGameArtworkFormatKey : @(NSPNGFileType),
                                                                   OEGameArtworkPropertiesKey : @{}}];
     }
}

#pragma mark - Creating and Obtaining OEDBGames

+ (id)createGameWithName:(NSString *)name andSystem:(OEDBSystem *)system inDatabase:(OELibraryDatabase *)database
{
    NSManagedObjectContext *context = [database unsafeContext];

    __block OEDBGame *game = nil;
    [context performBlockAndWait:^{
        NSEntityDescription *description = [NSEntityDescription entityForName:@"Game" inManagedObjectContext:context];
        game = [[OEDBGame alloc] initWithEntity:description insertIntoManagedObjectContext:context];

        [game setName:name];
        [game setImportDate:[NSDate date]];
        [game setSystem:system];
    }];
    
    return game;
}

+ (id)gameWithID:(NSManagedObjectID *)objID
{
    return [self gameWithID:objID inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)gameWithID:(NSManagedObjectID *)objID inDatabase:(OELibraryDatabase *)database
{
    return [database objectWithID:objID];
}

+ (id)gameWithURIURL:(NSURL *)objIDUrl
{
    return [self gameWithURIURL:objIDUrl inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)gameWithURIURL:(NSURL *)objIDUrl inDatabase:(OELibraryDatabase *)database
{
    NSManagedObjectID *objID = [database managedObjectIDForURIRepresentation:objIDUrl];
    return [self gameWithID:objID inDatabase:database];
}

+ (id)gameWithURIString:(NSString *)objIDString
{
    return [self gameWithURIString:objIDString inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)gameWithURIString:(NSString *)objIDString inDatabase:(OELibraryDatabase *)database
{
    NSURL *url = [NSURL URLWithString:objIDString];
    return [self gameWithURIURL:url inDatabase:database];
}

// returns the game from the default database that represents the file at url
+ (id)gameWithURL:(NSURL *)url error:(NSError *__autoreleasing*)outError
{
    return [self gameWithURL:url inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
}

// returns the game from the specified database that represents the file at url
+ (id)gameWithURL:(NSURL *)url inDatabase:(OELibraryDatabase *)database error:(NSError *__autoreleasing*)outError
{
    if(url == nil)
    {
        // TODO: create error saying that url is nil
        return nil;
    }
    
    NSError __autoreleasing *nilerr;
    if(outError == NULL) outError = &nilerr;
    
    BOOL urlReachable = [url checkResourceIsReachableAndReturnError:outError];
    
    OEDBGame *game = nil;
    OEDBRom *rom = [OEDBRom romWithURL:url error:outError];
    if(rom != nil)
    {
        game = [rom game];
    }
    
    NSString *md5 = nil, *crc = nil;
    NSFileManager *defaultFileManager = [NSFileManager defaultManager];
    if(game == nil && urlReachable)
    {
        [defaultFileManager hashFileAtURL:url md5:&md5 crc32:&crc error:outError];
        OEDBRom *rom = [OEDBRom romWithMD5HashString:md5 inDatabase:database error:outError];
        if(!rom) rom = [OEDBRom romWithCRC32HashString:crc inDatabase:database error:outError];
        if(rom) game = [rom game];
    }
    
    if(!urlReachable)
        [game setStatus:[NSNumber numberWithInt:OEDBGameStatusAlert]];

    return game;
}

+ (NSArray *)allGames
{
    return [self allGamesWithError:nil];
}

+ (NSArray *)allGamesWithError:(NSError *__autoreleasing *)error
{
    return [self allGamesInDatabase:[OELibraryDatabase defaultDatabase] error:error];
}

+ (NSArray *)allGamesInDatabase:(OELibraryDatabase *)database
{
    return [self allGamesInDatabase:database error:nil];
}

+ (NSArray *)allGamesInDatabase:(OELibraryDatabase *)database error:(NSError *__autoreleasing *)error;
{
    NSFetchRequest *request = [[NSFetchRequest alloc] initWithEntityName:[self entityName]];
    return [database executeFetchRequest:request error:error];
}

#pragma mark - Cover Art Database Sync / Info Lookup
- (void)requestCoverDownload
{
    [self setStatus:[NSNumber numberWithInt:OEDBGameStatusProcessing]];
    [[self libraryDatabase] save:nil];
    [[self libraryDatabase] startOpenVGDBSync];
}

- (void)cancelCoverDownload
{
    [self setStatus:[NSNumber numberWithInt:OEDBGameStatusOK]];
    [[self libraryDatabase] save:nil];
}

- (void)requestInfoSync
{
    [self setStatus:[NSNumber numberWithInt:OEDBGameStatusProcessing]];
    [[self libraryDatabase] save:nil];
    [[self libraryDatabase] startOpenVGDBSync];
}

- (void)performInfoSync
{
    __block NSMutableDictionary *result = nil;
    __block NSError *error = nil;

    NSString * const boxImageURLKey = @"boxImageURL";

    OEDBRom *rom = [[self roms] anyObject];
    result = [[[OEGameInfoHelper sharedHelper] gameInfoForROM:rom error:&error] mutableCopy];

    if(result != nil && [result objectForKey:boxImageURLKey] != nil)
    {
        NSString *normalizedURL = [[result objectForKey:boxImageURLKey] stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
        NSURL   *url = [NSURL URLWithString:normalizedURL];
        [self setBoxImageByURL:url];
    }

    if(result != nil)
    {
        [self setValuesForKeysWithDictionary:result];
        [self setLastInfoSync:[NSDate date]];
    }
    [self setStatus:@(OEDBGameStatusOK)];
    [[self libraryDatabase] save:nil];
}

#pragma mark -

- (id)mergeInfoFromGame:(OEDBGame *)game
{
    // TODO: (low priority): improve merging
    // we could merge with priority based on last info sync for example
    if([self name] == nil)
        [self setName:[game name]];
    
    if([self gameTitle] == nil)
        [self setGameTitle:[game gameTitle]];
	
    if([self gameDescription] == nil)
        [self setGameDescription:[game gameDescription]];
    
    if([self lastInfoSync] == nil)
        [self setLastInfoSync:[game lastInfoSync]];
	
    if([self importDate] == nil)
        [self setImportDate:[game importDate]];
    
    if([self rating] == nil)
        [self setRating:[game rating]];
    
    if([self boxImage] == nil)
        [self setBoxImage:[game boxImage]];
    
    NSMutableSet *ownCollections = [self mutableCollections];
    NSSet *gameCollections = [game collections];
    [ownCollections unionSet:gameCollections];
    
    NSMutableSet *ownCredits = [self mutableCredits];
    NSSet *gameCredits = [game credits];
    [ownCredits unionSet:gameCredits];
    
    NSMutableSet *ownRoms = [self mutableRoms];
    NSSet *gameRoms = [game roms];
    [ownRoms unionSet:gameRoms];
    
    NSMutableSet *ownGenres = [self mutableGenres];
    NSSet *gameGenres = [game genres];
    [ownGenres unionSet:gameGenres];
    
    return self;
}

#pragma mark - Accessors

- (NSDate *)lastPlayed
{
    NSArray *roms = [[self roms] allObjects];
    
    NSArray *sortedByLastPlayed =
    [roms sortedArrayUsingComparator:
     ^ NSComparisonResult (id obj1, id obj2)
     {
         return [[obj1 lastPlayed] compare:[obj2 lastPlayed]];
     }];
    
    return [[sortedByLastPlayed lastObject] lastPlayed];
}

- (OEDBSaveState *)autosaveForLastPlayedRom
{
    NSArray *roms = [[self roms] allObjects];
    
    NSArray *sortedByLastPlayed =
    [roms sortedArrayUsingComparator:
     ^ NSComparisonResult (id obj1, id obj2)
     {
         return [[obj1 lastPlayed] compare:[obj2 lastPlayed]];
     }];
	
    return [[sortedByLastPlayed lastObject] autosaveState];
}

- (NSNumber *)saveStateCount
{
    NSUInteger count = 0;
    for(OEDBRom *rom in [self roms]) count += [rom saveStateCount];
    return @(count);
}

- (OEDBRom *)defaultROM
{
    NSSet *roms = [self roms];
    // TODO: if multiple roms are available we should select one based on version/revision and language
    
    return [roms anyObject];
}

- (NSNumber *)playCount
{
    NSUInteger count = 0;
    for(OEDBRom *rom in [self roms]) count += [[rom playCount] unsignedIntegerValue];
    return @(count);
}

- (NSNumber *)playTime
{
    NSTimeInterval time = 0;
    for(OEDBRom *rom in [self roms]) time += [[rom playTime] doubleValue];
    return @(time);
}

- (BOOL)filesAvailable
{
    __block BOOL result = YES;
    [[self roms] enumerateObjectsUsingBlock:^(OEDBRom *rom, BOOL *stop) {
        if(![rom filesAvailable])
        {
            result = NO;
            *stop = YES;
        }
    }];
    
    if(!result)
       [self setStatus:[NSNumber numberWithInt:OEDBGameStatusAlert]];
    else if([[self status] intValue] == OEDBGameStatusAlert)
        [self setStatus:[NSNumber numberWithInt:OEDBGameStatusOK]];
    
    return result;
}

#pragma mark -
- (NSString*)boxImageURL
{
    OEDBImage *image = [self boxImage];
    return [image sourceURL];
}

- (void)setBoxImageURL:(NSString *)boxImageURL
{
    NSString *e = [boxImageURL stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
    NSURL *url = [NSURL URLWithString:e];
    [self setBoxImageByURL:url];
}

#pragma mark - Core Data utilities

- (void)deleteByMovingFile:(BOOL)moveToTrash keepSaveStates:(BOOL)statesFlag
{
    NSMutableSet *mutableRoms = [self mutableRoms];
    while ([mutableRoms count]) {
        OEDBRom *aRom = [mutableRoms anyObject];
        [aRom deleteByMovingFile:moveToTrash keepSaveStates:statesFlag];
        [mutableRoms removeObject:aRom];
    }

    NSManagedObjectContext *context = [self managedObjectContext];
    [context performBlockAndWait:^{
        [context deleteObject:self];
    }];
}

+ (NSString *)entityName
{
    return @"Game";
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context
{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}

#pragma mark -

- (void)setBoxImageByImage:(NSImage *)img
{
    [self OE_setBoxImage:img withSourceURL:nil];
}

- (void)setBoxImageByURL:(NSURL *)url
{
    NSImage *image = [[NSImage alloc] initWithContentsOfURL:url];
    [self OE_setBoxImage:image withSourceURL:url];
}


- (void)OE_setBoxImage:(NSImage*)image withSourceURL:(NSURL*)url
{
    NSManagedObjectContext *context  = [self managedObjectContext];
    OEDBImage *boxImage = [self boxImage];

    if(boxImage) // delete previous image if any
    {
        [context performBlockAndWait:^{
            [context deleteObject:boxImage];
        }];
        boxImage = nil;
        [self setBoxImage:nil];
    }

    if(image != nil) // create new image and thumbnails
    {
        NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
        NSArray        *sizes            = [standardDefaults objectForKey:OEBoxSizesKey];
        NSMutableArray *thumbnailImages  = [NSMutableArray arrayWithCapacity:[sizes count]+1];
        id original = [self OE_generateThumbnailFromImage:image withSize:NSZeroSize];
        if(original != nil) [thumbnailImages addObject:original];
        [sizes enumerateObjectsUsingBlock:^(NSString *aSize, NSUInteger idx, BOOL *stop) {
            id result = [self OE_generateThumbnailFromImage:image withSize:NSSizeFromString(aSize)];
            if(result != nil)
                [thumbnailImages addObject:result];
        }];

        [context performBlockAndWait:^{

            // create core data objects for thumbnails
            NSMutableSet *versions = [NSMutableSet setWithCapacity:[thumbnailImages count]+1];
            [thumbnailImages enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                [versions addObject:[self OE_thumbnailFromDescription:obj inContext:context]];
            }];

            if([versions count])
            {
                // create core data objec for box image
                NSEntityDescription *imageDescription = [OEDBImage entityDescriptionInContext:context];
                OEDBImage *boxImage = [[OEDBImage alloc] initWithEntity:imageDescription insertIntoManagedObjectContext:context];
                [boxImage setVersions:versions];
                if(url) [boxImage setSourceURL:[url absoluteString]];

                [self setBoxImage:boxImage];
            }
        }];
    }
}

- (OEDBImageThumbnail*)OE_thumbnailFromDescription:(NSArray*)description inContext:(NSManagedObjectContext*)context
{
    NSSize size = [description[0] sizeValue];
    NSString *relativePath = description[1];
    NSEntityDescription *thumbDescription = [OEDBImageThumbnail entityDescriptionInContext:context];
    OEDBImageThumbnail *thumbnail = [[OEDBImageThumbnail alloc] initWithEntity:thumbDescription insertIntoManagedObjectContext:context];
    [thumbnail setWidth:@(size.width)];
    [thumbnail setHeight:@(size.height)];
    [thumbnail setRelativePath:relativePath];

    return thumbnail;
}

- (id)OE_generateThumbnailFromImage:(NSImage*)image withSize:(NSSize)size
{
    BOOL     resize      = !NSEqualSizes(size, NSZeroSize);
    NSString *version    = !resize ? @"original" : [NSString stringWithFormat:@"%d", (int)size.width];
    NSString *uuid       = [NSString stringWithUUID];

    NSURL    *coverFolderURL = [[self libraryDatabase] coverFolderURL];
    coverFolderURL = [coverFolderURL URLByAppendingPathComponent:version isDirectory:YES];
    NSURL *url          = [coverFolderURL URLByAppendingPathComponent:uuid];

    // find a bitmap representation
    NSBitmapImageRep *bitmapRep = [[image representations] firstObjectMatchingBlock:^BOOL(id obj) {
        return [obj isKindOfClass:[NSBitmapImageRep class]];
    }];

    NSSize imageSize = [image size];
    if(bitmapRep)
        imageSize = NSMakeSize([bitmapRep pixelsWide], [bitmapRep pixelsHigh]);

    float  aspectRatio = imageSize.width / imageSize.height;
    NSSize thumbnailSize;
    if(resize)
    {
        thumbnailSize = aspectRatio<1 ? (NSSize){size.height * aspectRatio, size.height} : (NSSize){size.width, size.width / aspectRatio};

        // thumbnails only make sense if they are smaller than the original
        if(thumbnailSize.width >= imageSize.width || thumbnailSize.height >= imageSize.height)
        {
            return nil;
        }

        NSImage *thumbnailImage = [[NSImage alloc] initWithSize:thumbnailSize];
        [thumbnailImage lockFocus];
        [image drawInRect:(NSRect){{0, 0}, {thumbnailSize.width, thumbnailSize.height}} fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
        bitmapRep = [[NSBitmapImageRep alloc] initWithFocusedViewRect:(NSRect){{0,0}, thumbnailSize}];
        [thumbnailImage unlockFocus];
    }
    else
    {
        thumbnailSize = imageSize;

        if(!bitmapRep) // no bitmap found, create one
        {
            [image lockFocus];
            bitmapRep = [[NSBitmapImageRep alloc] initWithFocusedViewRect:(NSRect){{0,0}, thumbnailSize}];
            [image unlockFocus];
        }
    }

    // write image file
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    NSBitmapImageFileType fileType = [standardUserDefaults integerForKey:OEGameArtworkFormatKey];
    NSDictionary   *fileProperties = [standardUserDefaults dictionaryForKey:OEGameArtworkPropertiesKey];
    NSData       *imageData  = [bitmapRep representationUsingType:fileType properties:fileProperties];

    [[NSFileManager defaultManager] createDirectoryAtURL:[url URLByDeletingLastPathComponent] withIntermediateDirectories:YES attributes:nil error:nil];

    if([imageData writeToURL:url options:0 error:nil])
    {
        return @[ [NSValue valueWithSize:thumbnailSize], [NSString stringWithFormat:@"%@/%@", version, uuid] ];
    }
    return nil;
}

#pragma mark - NSPasteboardWriting
// TODO: fix pasteboard writing
- (NSArray *)writableTypesForPasteboard:(NSPasteboard *)pasteboard
{
    return [NSArray arrayWithObjects:(NSString *)kPasteboardTypeFileURLPromise, OEPasteboardTypeGame, /* NSPasteboardTypeTIFF,*/ nil];
}

- (NSPasteboardWritingOptions)writingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard
{
    if(type ==(NSString *)kPasteboardTypeFileURLPromise)
        return NSPasteboardWritingPromised;
    
    return 0;
}

- (id)pasteboardPropertyListForType:(NSString *)type
{
    if(type == (NSString *)kPasteboardTypeFileURLPromise)
    {
        NSSet *roms = [self roms];
        NSMutableArray *paths = [NSMutableArray arrayWithCapacity:[roms count]];
        for(OEDBRom *aRom in roms)
        {
            NSString *urlString = [[aRom URL] absoluteString];
            [paths addObject:urlString];
        }
        return paths;
    } 
    else if(type == OEPasteboardTypeGame)
    {
        return [[[self objectID] URIRepresentation] absoluteString];
    }
    
    // TODO: return appropriate obj
    return nil;
}

#pragma mark - NSPasteboardReading

- (id)initWithPasteboardPropertyList:(id)propertyList ofType:(NSString *)type
{
    if(type == OEPasteboardTypeGame)
    {
        OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];
        NSURL    *uri  = [NSURL URLWithString:propertyList];
        OEDBGame *game = [database objectWithURI:uri];
        return game;
    }    
    return nil;
}

+ (NSArray *)readableTypesForPasteboard:(NSPasteboard *)pasteboard
{
    return [NSArray arrayWithObjects:OEPasteboardTypeGame, nil];
}

+ (NSPasteboardReadingOptions)readingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard
{
    return NSPasteboardReadingAsString;
}

#pragma mark - Data Model Relationships

- (NSMutableSet *)mutableRoms
{
    return [self mutableSetValueForKey:@"roms"];
}

- (NSMutableSet *)mutableGenres
{
    return [self mutableSetValueForKey:@"genres"];
}
- (NSMutableSet *)mutableCollections
{
    return [self mutableSetValueForKeyPath:@"collections"];
}
- (NSMutableSet *)mutableCredits
{
    return [self mutableSetValueForKeyPath:@"credits"];
}

- (NSString *)displayName
{
    if([[NSUserDefaults standardUserDefaults] boolForKey:OEDisplayGameTitle])
        return ([self gameTitle] != nil ? [self gameTitle] : [self name]);
    else
        return [self name];
}

- (void)setDisplayName:(NSString *)displayName
{
    if([[NSUserDefaults standardUserDefaults] boolForKey:OEDisplayGameTitle])
        ([self gameTitle] != nil ? [self setGameTitle:displayName] : [self setName:displayName]);
    else
        [self setName:displayName];
}

- (NSString *)cleanDisplayName
{
    NSString *displayName = [self displayName];
    NSDictionary *articlesDictionary = @{
                                 @"A "   : @"2",
                                 @"An "  : @"3",
                                 @"Das " : @"4",
                                 @"Der " : @"4",
                                 //@"Die " : @"4", Biased since some English titles start with Die
                                 @"Gli " : @"4",
                                 @"L'"   : @"2",
                                 @"La "  : @"3",
                                 @"Las " : @"4",
                                 @"Le "  : @"3",
                                 @"Les " : @"4",
                                 @"Los " : @"4",
                                 @"The " : @"4",
                                 @"Un "  : @"3",
                                 };
    
    for (id key in articlesDictionary) {
        if([displayName hasPrefix:key])
        {
            return [displayName substringFromIndex:[articlesDictionary[key] integerValue]];
        }
        
    }
    
    return  displayName;
}

#pragma mark - Debug

- (void)dump
{
    [self dumpWithPrefix:@"---"];
}

- (void)dumpWithPrefix:(NSString *)prefix
{
    NSString *subPrefix = [prefix stringByAppendingString:@"-----"];
    NSLog(@"%@ Beginning of game dump", prefix);

    NSLog(@"%@ Game name is %@", prefix, [self name]);
    NSLog(@"%@ title is %@", prefix, [self gameTitle]);
    NSLog(@"%@ rating is %@", prefix, [self rating]);
    NSLog(@"%@ description is %@", prefix, [self gameDescription]);
    NSLog(@"%@ import date is %@", prefix, [self importDate]);
    NSLog(@"%@ last info sync is %@", prefix, [self lastInfoSync]);
    NSLog(@"%@ last played is %@", prefix, [self lastPlayed]);
    NSLog(@"%@ status is %@", prefix, [self status]);

    NSLog(@"%@ Number of ROMs for this game is %lu", prefix, (unsigned long)[[self roms] count]);

    for(id rom in [self roms])
    {
        if([rom respondsToSelector:@selector(dumpWithPrefix:)]) [rom dumpWithPrefix:subPrefix];
        else NSLog(@"%@ ROM is %@", subPrefix, rom);
    }

    NSLog(@"%@ End of game dump\n\n", prefix);
}

@end
