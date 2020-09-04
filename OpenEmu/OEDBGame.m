/*
 Copyright (c) 2020, OpenEmu Team
 
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

#import "OEDBGame+CoreDataProperties.h"

#import "OELibraryDatabase.h"

#import "OEDBSystem.h"
#import "OEDBRom+CoreDataProperties.h"
#import "OEDBImage.h"

#import "OEGameInfoHelper.h"
#import "OEDownload.h"

#import "NSFileManager+OEHashingAdditions.h"

NS_ASSUME_NONNULL_BEGIN

NSString *const OEPasteboardTypeGame = @"org.openemu.game";
NSString *const OEDisplayGameTitle = @"displayGameTitle";

NSString *const OEGameArtworkFormatKey = @"artworkFormat";
NSString *const OEGameArtworkPropertiesKey = @"artworkProperties";

@interface OEDBGame ()
@property(nullable) OEDownload *romDownload;
@end

@implementation OEDBGame
@synthesize romDownload=_romDownload;
@dynamic displayName;

+ (void)initialize
{
     if (self == [OEDBGame class])
     {
         [[NSUserDefaults standardUserDefaults] registerDefaults:@{
                                                                   OEGameArtworkFormatKey : @(NSBitmapImageFileTypeJPEG),
                                                                   OEGameArtworkPropertiesKey : @{
                                                                           NSImageCompressionFactor : @(0.9)
                                                                           }
                                                                   }];
     }
}

#pragma mark - Creating and Obtaining OEDBGames

+ (instancetype)createGameWithName:(NSString *)name andSystem:(OEDBSystem *)system inDatabase:(OELibraryDatabase *)database
{
    NSManagedObjectContext *context = database.mainThreadContext;

    __block OEDBGame *game = nil;
    [context performBlockAndWait:^{
        NSEntityDescription *description = [NSEntityDescription entityForName:@"Game" inManagedObjectContext:context];
        game = [[OEDBGame alloc] initWithEntity:description insertIntoManagedObjectContext:context];

        game.name = name;
        game.importDate = [NSDate date];
        game.system = system;
    }];
    
    return game;
}

// returns the game from the default database that represents the file at url
+ (instancetype)gameWithURL:(nullable NSURL *)url error:(NSError *__autoreleasing*)outError
{
    return [self gameWithURL:url inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
}

// returns the game from the specified database that represents the file at url
+ (instancetype _Nullable)gameWithURL:(nullable NSURL *)url inDatabase:(OELibraryDatabase *)database error:(NSError **)outError
{
    if(url == nil)
    {
        // TODO: create error saying that url is nil
        return nil;
    }
    
    NSError __autoreleasing *nilerr;
    if(outError == NULL) outError = &nilerr;

    url = url.URLByStandardizingPath;
    BOOL urlReachable = [url checkResourceIsReachableAndReturnError:outError];

    // TODO: FIX
    OEDBGame *game = nil;
    NSManagedObjectContext *context = database.mainThreadContext;
    OEDBRom *rom = [OEDBRom romWithURL:url inContext:context error:outError];
    if(rom != nil)
    {
        game = rom.game;
    }
    
    NSString *md5 = nil;
    NSFileManager *defaultFileManager = [NSFileManager defaultManager];
    if(game == nil && urlReachable)
    {
        [defaultFileManager hashFileAtURL:url md5:&md5 error:outError];
        OEDBRom *rom = [OEDBRom romWithMD5HashString:md5 inContext:context error:outError];
        if(rom) game = rom.game;
    }
    
    if(!urlReachable)
        game.status = @(OEDBGameStatusAlert);

    return game;
}


#pragma mark - Cover Art Database Sync / Info Lookup

- (void)requestCoverDownload
{
    if([self.status isEqualTo:@(OEDBGameStatusAlert)] || [self.status isEqualTo:@(OEDBGameStatusOK)])
    {
        self.status = @(OEDBGameStatusProcessing);
        [self save];
        [self.libraryDatabase startOpenVGDBSync];
    }
}

- (void)cancelCoverDownload
{
    if([self.status isEqualTo:@(OEDBGameStatusProcessing)])
    {
        self.status = @(OEDBGameStatusOK);
        [self save];
    }
}

- (void)requestInfoSync
{
    if([self.status isEqualTo:@(OEDBGameStatusAlert)] || [self.status isEqualTo:@(OEDBGameStatusOK)])
    {
        self.status = @(OEDBGameStatusProcessing);
        [self save];
        [self.libraryDatabase startOpenVGDBSync];
    }
}

#pragma mark - ROM Downloading

- (void)requestROMDownload
{
    if(_romDownload != nil) return;
    self.status = @(OEDBGameStatusDownloading);

    OEDBRom *rom = self.defaultROM;
    NSString *source = rom.source;
    if(source == nil || [source length] == 0)
    {
        DLog(@"Invalid URL to download!");
        return;
    }

    NSURL *url = [NSURL URLWithString:source];
    if(url == nil)
    {
        DLog(@"Invalid URL to download!");
        return;
    }

    __block __strong OEDBGame *blockSelf = self; // We don't want to be deallocated while the download is still running
    _romDownload = [[OEDownload alloc] initWithURL:url];
    _romDownload.completionHandler = ^(NSURL *url, NSError *error) {
        if(!url || error)
        {
            DLog(@"ROM download Failed!");
            DLog(@"%@", error);
        }
        else
        {
            DLog(@"Downloaded to %@", url);
            rom.URL = url;
            NSError *err = nil;
            if(![rom consolidateFilesWithError:&err])
            {
                DLog(@"%@", err);
                rom.URL = nil;
            }
            [rom save];
        }

        blockSelf.status = @(OEDBGameStatusOK);
        blockSelf.romDownload = nil;
        [blockSelf save];
    };
    [_romDownload startDownload];
}

- (void)cancelROMDownload
{
    [_romDownload cancelDownload];
    _romDownload = nil;
    self.status = @(OEDBGameStatusOK);
    [self save];
}

#pragma mark - Accessors
- (NSDate *)lastPlayed
{
    NSArray <OEDBRom *> *roms = self.roms.allObjects;
    
    NSArray <OEDBRom *> *sortedByLastPlayed =
    [roms sortedArrayUsingComparator:
     ^ NSComparisonResult (OEDBRom *obj1, OEDBRom *obj2)
     {
         return [obj1.lastPlayed compare:obj2.lastPlayed];
     }];
    
    return sortedByLastPlayed.lastObject.lastPlayed;
}

- (OEDBSaveState *)autosaveForLastPlayedRom
{
    NSArray <OEDBRom *> *roms = self.roms.allObjects;
    
    NSArray <OEDBRom *> *sortedByLastPlayed =
    [roms sortedArrayUsingComparator:
     ^ NSComparisonResult (OEDBRom *obj1, OEDBRom *obj2)
     {
         return [obj1.lastPlayed compare:obj2.lastPlayed];
     }];
	
    return sortedByLastPlayed.lastObject.autosaveState;
}

- (NSNumber *)saveStateCount
{
    NSUInteger count = 0;
    for(OEDBRom *rom in self.roms) count += rom.saveStateCount;
    return @(count);
}

- (OEDBRom *)defaultROM
{
    // TODO: if multiple roms are available we should select one based on version/revision and language
    return self.roms.anyObject;
}

- (NSNumber *)playCount
{
    NSUInteger count = 0;
    for(OEDBRom *rom in self.roms) count += [rom.playCount unsignedIntegerValue];
    return @(count);
}

- (NSNumber *)playTime
{
    NSTimeInterval time = 0;
    for(OEDBRom *rom in self.roms) time += [rom.playTime doubleValue];
    return @(time);
}

- (BOOL)filesAvailable
{
    BOOL result = YES;
    for(OEDBRom *rom in self.roms)
    {
        if(!rom.filesAvailable)
        {
            result = YES;
            break;
        }
    }

    if([self.status isEqualTo:@(OEDBGameStatusDownloading)] || [self.status isEqualTo:@(OEDBGameStatusProcessing)])
       return result;

    if(!result)
       self.status = @(OEDBGameStatusAlert);
    else if([self.status intValue] == OEDBGameStatusAlert)
        self.status = @(OEDBGameStatusOK);
    
    return result;
}

#pragma mark -

- (void)setBoxImageByImage:(NSImage *)img
{
    NSDictionary *dictionary = [OEDBImage prepareImageWithNSImage:img];
    NSManagedObjectContext *context = self.managedObjectContext;
    [context performBlockAndWait:^{
        OEDBImage *currentImage = self.boxImage;
        if(currentImage) [context deleteObject:currentImage];

        OEDBImage *newImage = [OEDBImage createImageWithDictionary:dictionary];
        if(newImage) self.boxImage = newImage;
        else [context deleteObject:newImage];
    }];
}

- (void)setBoxImageByURL:(NSURL *)url
{
    url = url.URLByStandardizingPath;
    NSString *urlString = url.absoluteString;

    NSDictionary *dictionary = [OEDBImage prepareImageWithURLString:urlString];
    NSManagedObjectContext *context = self.managedObjectContext;
    [context performBlockAndWait:^{
        OEDBImage *currentImage = self.boxImage;
        if(currentImage) [context deleteObject:currentImage];

        OEDBImage *newImage = [OEDBImage createImageWithDictionary:dictionary];
        if(newImage) self.boxImage = newImage;
        else [context deleteObject:newImage];
    }];
}

#pragma mark - Core Data utilities

- (void)awakeFromFetch
{
    if([self.status isEqualTo:@(OEDBGameStatusDownloading)])
        self.status = @(OEDBGameStatusOK);
}

- (void)deleteByMovingFile:(BOOL)moveToTrash keepSaveStates:(BOOL)statesFlag
{
    NSMutableSet <OEDBRom *> *mutableRoms = self.mutableRoms;
    while (mutableRoms.count > 0)
    {
        OEDBRom *aRom = mutableRoms.anyObject;
        [aRom deleteByMovingFile:moveToTrash keepSaveStates:statesFlag];
        [mutableRoms removeObject:aRom];
    }
    self.roms = [NSSet set];
    [self.managedObjectContext deleteObject:self];
}

+ (NSString *)entityName
{
    return @"Game";
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context
{
    return [NSEntityDescription entityForName:self.entityName inManagedObjectContext:context];
}

- (void)prepareForDeletion
{
    [self.boxImage delete];

    [_romDownload cancelDownload];
    _romDownload = nil;
}

#pragma mark - NSPasteboardWriting

- (NSArray *)writableTypesForPasteboard:(NSPasteboard *)pasteboard
{
    return @[ OEPasteboardTypeGame, (NSString *)kUTTypeFileURL ];
}

- (nullable id)pasteboardPropertyListForType:(NSString *)type
{
    if(type == OEPasteboardTypeGame)
    {
        return self.permanentIDURI.absoluteString;
    }
    else if([type isEqualToString:(NSString *)kUTTypeFileURL])
    {
        OEDBRom *rom = self.defaultROM;
        NSURL *url = rom.URL.absoluteURL;
        return [url pasteboardPropertyListForType:(NSString *)kUTTypeFileURL];
    }

    DLog(@"Unkown type %@", type);
    return nil;
}

#pragma mark - NSPasteboardReading

- (nullable id)initWithPasteboardPropertyList:(id)propertyList ofType:(NSString *)type
{
    if(type == OEPasteboardTypeGame)
    {
        OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];
        NSManagedObjectContext *context = database.mainThreadContext;
        NSURL    *uri  = [NSURL URLWithString:propertyList];
        return (self = [OEDBGame objectWithURI:uri inContext:context]);
    } 
    return nil;
}

+ (NSArray *)readableTypesForPasteboard:(NSPasteboard *)pasteboard
{
    return @[ OEPasteboardTypeGame ];
}

+ (NSPasteboardReadingOptions)readingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard
{
    return NSPasteboardReadingAsString;
}

#pragma mark - Data Model Relationships

- (nullable NSMutableSet <OEDBRom *> *)mutableRoms
{
    return [self mutableSetValueForKey:@"roms"];
}

- (nullable NSMutableSet <NSManagedObject *> *)mutableGenres
{
    return [self mutableSetValueForKey:@"genres"];
}

- (nullable NSMutableSet <OEDBCollection *> *)mutableCollections
{
    return [self mutableSetValueForKeyPath:@"collections"];
}

- (nullable NSMutableSet <NSManagedObject *> *)mutableCredits
{
    return [self mutableSetValueForKeyPath:@"credits"];
}

- (nullable NSString *)displayName
{
    if([[NSUserDefaults standardUserDefaults] boolForKey:OEDisplayGameTitle])
        return (self.gameTitle != nil ? self.gameTitle : self.name);
    else
        return self.name;
}

- (void)setDisplayName:(nullable NSString *)displayName
{
    if([[NSUserDefaults standardUserDefaults] boolForKey:OEDisplayGameTitle])
    {
        if(self.gameTitle != nil)
            self.gameTitle = displayName;
        else
            self.name = displayName;
    }
    else
        self.name = displayName;
}

- (nullable NSString *)cleanDisplayName
{
    NSString *displayName = self.displayName;
    NSDictionary <NSString *, NSString *> *articlesDictionary = @{
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
            return [displayName substringFromIndex:articlesDictionary[key].integerValue];
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

    NSLog(@"%@ Game name is %@", prefix, self.name);
    NSLog(@"%@ title is %@", prefix, self.gameTitle);
    NSLog(@"%@ rating is %@", prefix, self.rating);
    NSLog(@"%@ description is %@", prefix, self.gameDescription);
    NSLog(@"%@ import date is %@", prefix, self.importDate);
    NSLog(@"%@ last info sync is %@", prefix, self.lastInfoSync);
    NSLog(@"%@ last played is %@", prefix, self.lastPlayed);
    NSLog(@"%@ status is %@", prefix, self.status);

    NSLog(@"%@ Number of ROMs for this game is %lu", prefix, (unsigned long)self.roms.count);

    for(id rom in self.roms)
    {
        if([rom respondsToSelector:@selector(dumpWithPrefix:)]) [rom dumpWithPrefix:subPrefix];
        else NSLog(@"%@ ROM is %@", subPrefix, rom);
    }

    NSLog(@"%@ End of game dump\n\n", prefix);
}

@end

NS_ASSUME_NONNULL_END
