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

#import "OEDBSaveState.h"
#import "OELibraryDatabase.h"
#import "OEDBRom.h"
#import "OEDBGame.h"
#import "OEDBSystem.h"
#import "OECorePlugin.h"
#import "NSURL+OELibraryAdditions.h"

#define OESaveStateDataFile         @"State"
#define OESaveStateScreenshotFile   @"ScreenShot"
#define OESaveStateLatestVersion    @"1.0"
NSString *const OESaveStateSuffix = @"oesavestate";

NSString *const OESaveStateInfoVersionKey           = @"Version";
NSString *const OESaveStateInfoNameKey              = @"Name";
NSString *const OESaveStateInfoDescriptionKey       = @"Description";
NSString *const OESaveStateInfoROMMD5Key            = @"ROM MD5";
NSString *const OESaveStateInfoCoreIdentifierKey    = @"Core Identifier";
NSString *const OESaveStateInfoCoreVersionKey       = @"Core Version";
NSString *const OESaveStateInfoTimestampKey         = @"Timestamp";

// NSString *const OESaveStateInfoCreationDateKey   = @"Creation Date";
// NSString *const OESaveStateInfoBookmarkDataKey   = @"Bookmark Data";

NSString *const OESaveStateSpecialNamePrefix    = @"OESpecialState_";
NSString *const OESaveStateAutosaveName         = @"OESpecialState_auto";
NSString *const OESaveStateQuicksaveName        = @"OESpecialState_quick";

NSString *const OESaveStateUseQuickSaveSlotsKey = @"UseQuickSaveSlots";

@implementation OEDBSaveState

+ (OEDBSaveState *)saveStateWithURL:(NSURL *)url inContext:(NSManagedObjectContext *)context
{
    NSFetchRequest *request = [NSFetchRequest fetchRequestWithEntityName:[self entityName]];

    url = [url URLByStandardizingPath];
    NSString *absoluteString = [url absoluteString];
    if([absoluteString characterAtIndex:[absoluteString length]-1] != '/')
    {
        absoluteString = [absoluteString stringByAppendingString:@"/"];
    }
    
    NSPredicate *predicate  = [NSPredicate predicateWithFormat:@"location == %@", absoluteString];
    [request setPredicate:predicate];
    
    return [[context executeFetchRequest:request error:nil] lastObject];
}


+ (instancetype)createObjectInContext:(NSManagedObjectContext *)context
{
    id result = [super createObjectInContext:context];
    [result setTimestamp:[NSDate date]];
	return result;
}

+ (id)createSaveStateWithURL:(NSURL *)url inContext:(NSManagedObjectContext *)context
{
    OEDBSaveState *newSaveState = [self createObjectInContext:context];
    url = [url URLByStandardizingPath];
    [newSaveState setLocation:[url absoluteString]];
    if(![newSaveState readInfoPlist])
    {
        // setting path to nil so file won't be deleted in -remove
        [newSaveState setLocation:nil];
        [newSaveState remove];
        newSaveState = nil;
    }

    NSError *error = nil;

    //TODO: use validation here instead of save
    if(newSaveState && ![newSaveState save])
    {
        [newSaveState setLocation:nil];
        [newSaveState remove];
        newSaveState = nil;
        DLog(@"State verification failed: %@ : %@", error, url);
        [context save:nil];
    }

    // state is saved in if statement above!

    return newSaveState;
}


+ (id)createSaveStateNamed:(NSString *)name forRom:(OEDBRom *)rom core:(OECorePlugin *)core withFile:(NSURL *)stateFileURL inContext:(NSManagedObjectContext *)context
{
    stateFileURL = [stateFileURL URLByStandardizingPath];

    OEDBSaveState *newSaveState = [self createObjectInContext:context];
    [newSaveState setName:name];
    [newSaveState setRom:rom];
    [newSaveState setTimestamp:[NSDate date]];

    NSString *coreIdentifier = [core bundleIdentifier];
    NSString *coreVersion = [core version];
    [newSaveState setCoreIdentifier:coreIdentifier];
    [newSaveState setCoreVersion:coreVersion];

    if([name hasPrefix:OESaveStateSpecialNamePrefix])
    {
        name = NSLocalizedString(name, @"Localized special save state name");
    }

    NSError  *error              = nil;
    NSString *fileName           = [NSURL validFilenameFromString:name];
    OELibraryDatabase *database = [newSaveState libraryDatabase];
    NSURL    *saveStateFolderURL = [database stateFolderURLForROM:rom];
    NSURL    *saveStateURL       = [saveStateFolderURL URLByAppendingPathComponent:[NSString stringWithFormat:@"%@.%@/", fileName, OESaveStateSuffix]];

    saveStateURL = [saveStateURL uniqueURLUsingBlock:^NSURL *(NSInteger triesCount) {
        return [saveStateFolderURL URLByAppendingPathComponent:[NSString stringWithFormat:@"%@ %ld.%@/", fileName, triesCount, OESaveStateSuffix]];
    }];

    if(![newSaveState OE_createBundleAtURL:saveStateURL withStateFile:stateFileURL error:&error])
    {
        // TODO: remove temp files
        NSLog(@"could not create state bundle at url: %@!", saveStateURL);
        NSLog(@"%@", [error localizedDescription]);
        [newSaveState delete];
        [newSaveState save];

        newSaveState = nil;
    }

    [newSaveState save];
    if(newSaveState)
    {
        NSManagedObjectContext *mainContext = [context parentContext];
        [mainContext performBlock:^{
            [mainContext save:nil];
        }];
    }

    return newSaveState;
}

- (BOOL)OE_createBundleAtURL:(NSURL *)bundleURL withStateFile:(NSURL *)stateFile error:(NSError **)error
{
    bundleURL = [bundleURL URLByStandardizingPath];
    stateFile = [stateFile URLByStandardizingPath];

    NSFileManager *fileManager         = [NSFileManager defaultManager];
    NSDictionary  *directoryAttributes = @{};
    if(![fileManager createDirectoryAtURL:bundleURL withIntermediateDirectories:YES attributes:directoryAttributes error:error])
    {
        return NO;
    }

    NSURL *stateURLInBundle = [bundleURL URLByAppendingPathComponent:OESaveStateDataFile];
    if(![fileManager moveItemAtURL:stateFile toURL:stateURLInBundle error:error])
    {
        [fileManager removeItemAtURL:bundleURL error:nil];
        return NO;
    }

    [[self managedObjectContext] performBlockAndWait:^{
        [self setURL:bundleURL];
    }];

    if(![self writeInfoPlist])
    {
        [fileManager removeItemAtURL:stateFile error:nil];
        [fileManager removeItemAtURL:bundleURL error:nil];

        return NO;
    }
    
    return YES;
}

+ (OEDBSaveState*)updateOrCreateStateWithURL:(NSURL *)url inContext:(NSManagedObjectContext *)context
{
    NSString *path = [url path];
    NSRange range     = [path rangeOfString:@".oesavestate" options:NSCaseInsensitiveSearch];
    if(range.location == NSNotFound) return nil;

    OELibraryDatabase *database         = [OELibraryDatabase defaultDatabase];
    NSFileManager     *defaultManager   = [NSFileManager defaultManager];
    NSString          *saveStatePath    = [path substringToIndex:range.location+range.length];
    NSURL             *originalStateUrl = [NSURL fileURLWithPath:saveStatePath];
    NSURL             *stateURL         = originalStateUrl;
    NSURL             *stateFolderURL   = [database stateFolderURL];

    BOOL stateOutsideStateDir = ![originalStateUrl isSubpathOfURL:stateFolderURL];
    if(stateOutsideStateDir)
    {
        NSString *currentFileName = [[stateURL lastPathComponent] stringByDeletingPathExtension];
        NSString *extension       = [stateURL pathExtension];
        stateURL = [stateURL uniqueURLUsingBlock:^NSURL *(NSInteger triesCount) {
            return [stateFolderURL URLByAppendingPathComponent:[NSString stringWithFormat:@"%@ %ld.%@", currentFileName, triesCount, extension]];
        }];

        if(![defaultManager copyItemAtURL:originalStateUrl toURL:stateURL error:nil])
            return nil;
    }

    OEDBSaveState *saveState = [OEDBSaveState saveStateWithURL:stateURL inContext:context];
    BOOL fileAvailable = [stateURL checkResourceIsReachableAndReturnError:nil];
    if(fileAvailable)
    {
        if(saveState)
        {
            // update state info from plist
            [saveState readInfoPlist];
        }
        else
        {
            // create new save state
            saveState = [OEDBSaveState createSaveStateWithURL:stateURL inContext:context];
        }
    }
    else
    {
        // file missing, delete state from db
        [saveState remove];
        [saveState save];
        saveState = nil;
    }

    [saveState moveToDefaultLocation];
    [context save:nil];

    return saveState;
}

+ (NSString *)nameOfQuickSaveInSlot:(NSInteger)slot
{
    return slot == 0 ? OESaveStateQuicksaveName:[NSString stringWithFormat:@"%@%ld", OESaveStateQuicksaveName, slot];
}
+ (NSString*)entityName
{
    return @"SaveState";
}

#pragma mark - Management
- (BOOL)readInfoPlist
{
    __block BOOL result = YES;
    [[self managedObjectContext] performBlockAndWait:^{
        NSDictionary *infoPlist = [self infoPlist];
        NSString     *version   = [infoPlist valueForKey:OESaveStateInfoVersionKey];
        if([version isEqualTo:@"1.0"])
        {
            NSString *infoName              = [infoPlist valueForKey:OESaveStateInfoNameKey];
            NSString *infoCoreIdentifier    = [infoPlist valueForKey:OESaveStateInfoCoreIdentifierKey];
            NSString *infoCoreVersion       = [infoPlist valueForKey:OESaveStateInfoCoreVersionKey];
            NSString *infoUserDescription   = [infoPlist valueForKey:OESaveStateInfoDescriptionKey];
            NSString *infoRomMD5            = [infoPlist valueForKey:OESaveStateInfoROMMD5Key];
            NSDate   *infoTimestamp         = [infoPlist valueForKey:OESaveStateInfoTimestampKey];

            OEDBRom  *rom                   = [OEDBRom romWithMD5HashString:infoRomMD5 inContext:[self managedObjectContext] error:nil];
            if(infoName==nil || infoCoreIdentifier==nil || infoRomMD5==nil || rom==nil)
            {
                result = NO;
                return;
            }

            [self setName:infoName];
            [self setCoreIdentifier:infoCoreIdentifier];
            [self setCoreVersion:infoCoreVersion];
            [self setRom:rom];

            if(infoTimestamp)
                [self setTimestamp:infoTimestamp];
            else
                [self setTimestamp:[NSDate date]];

            if(infoUserDescription)
                [self setUserDescription:infoUserDescription];
        }
        else
        {
            NSLog(@"Unkown Save State Version (%@)", version?:@"none");
            result = NO;
            return;
        }

        [[self managedObjectContext] save:nil];
    }];
    
    return result;
}

- (BOOL)writeInfoPlist
{
    __block NSString *name = nil;
    __block NSString *coreIdentifier = nil;
    __block NSString *coreVersion = nil;
    __block NSString *md5Hash = nil;
    __block NSDate   *timestamp = nil;
    __block NSString *userDescription = nil;
    __block NSURL    *infoPlistURL = nil;
    __block NSMutableDictionary *infoPlist = nil;

    [[self managedObjectContext] performBlockAndWait:^{
        name = [self name];
        coreIdentifier = [self coreIdentifier];
        coreVersion    = [self coreVersion];
        md5Hash = [[self rom] md5Hash];
        timestamp = [self timestamp];
        userDescription = [self userDescription];
        infoPlistURL = [self infoPlistURL];

        infoPlist = [[self infoPlist] mutableCopy];
    }];

    // Save State Values
    [infoPlist setObject:OESaveStateLatestVersion   forKey:OESaveStateInfoVersionKey];
    [infoPlist setObject:name                       forKey:OESaveStateInfoNameKey];
    [infoPlist setObject:coreIdentifier             forKey:OESaveStateInfoCoreIdentifierKey];
    if(coreVersion)
        [infoPlist setObject:coreVersion            forKey:OESaveStateInfoCoreVersionKey];
    [infoPlist setObject:md5Hash                    forKey:OESaveStateInfoROMMD5Key];
    [infoPlist setObject:timestamp                  forKey:OESaveStateInfoTimestampKey];
    if(userDescription)
        [infoPlist setObject:userDescription forKey:OESaveStateInfoDescriptionKey];

    if(![infoPlist writeToURL:infoPlistURL atomically:YES])
        return NO;
    
    return YES;
}

- (void)remove
{
    [[NSFileManager defaultManager] removeItemAtURL:[self URL] error:nil];
    [[self managedObjectContext] deleteObject:self];
}

- (void)removeIfMissing
{
    NSError *error;
    if(![[self URL] checkResourceIsReachableAndReturnError:&error])
    {
        NSLog(@"Removing save state: %@", [self URL]);
        NSLog(@"Reason: %@", [error localizedDescription]);
        
        [self remove];
    }
}

- (void)replaceStateFileWithFile:(NSURL *)stateFile
{
    [[NSFileManager defaultManager] removeItemAtURL:[self stateFileURL] error:nil];
    [[NSFileManager defaultManager] copyItemAtURL:stateFile toURL:[self stateFileURL] error:nil];
}

- (void)moveToDefaultLocation
{
    NSURL    *saveStateFolderURL = [[self libraryDatabase] stateFolderURLForROM:[self rom]];
    if([[self URL] isSubpathOfURL:saveStateFolderURL]) return;

    NSURL    *newStateURL        = [saveStateFolderURL URLByAppendingPathComponent:[[self URL] lastPathComponent]];
    NSString *currentFileName    = [[[self URL] lastPathComponent] stringByDeletingPathExtension];
    newStateURL                  = [newStateURL uniqueURLUsingBlock:^NSURL *(NSInteger triesCount) {
        return [saveStateFolderURL URLByAppendingPathComponent:[NSString stringWithFormat:@"%@ %ld.%@", currentFileName, triesCount, OESaveStateSuffix]];
    }];

    NSError *error = nil;
    if(![[NSFileManager defaultManager] moveItemAtURL:[self URL] toURL:newStateURL error:&error])
    {
        DLog(@"Error occured while moving State to default location");
        DLog(@"%@", [error localizedDescription]);
        return;
    }
    [self setURL:newStateURL];
    [[self managedObjectContext] save:nil];
}

#pragma mark - Data Accessors

- (NSString *)displayName
{
    if(![self isSpecialState])
        return [self name];
    
    NSString *name = [self name];
    if([name isEqualToString:OESaveStateAutosaveName])
    {
        return NSLocalizedString(@"Auto Save State", @"Autosave state display name");
    }
    else if([name isEqualToString:OESaveStateQuicksaveName])
    {
        return NSLocalizedString(@"Quick Save State", @"Quicksave state display name");
    }
    return name;
}

- (BOOL)isSpecialState
{
    return [[self name] rangeOfString:OESaveStateSpecialNamePrefix].location == 0;
}

#pragma mark -
#pragma mark Data Model Properties

@dynamic name, userDescription, timestamp;
@dynamic coreIdentifier, location, coreVersion;

- (NSURL *)URL
{
    return [NSURL URLWithString:[self location]];
}

- (void)setURL:(NSURL *)url
{
    url = [url URLByStandardizingPath];
    [self setLocation:[url absoluteString]];
}

- (NSURL *)screenshotURL
{
    return [[self URL] URLByAppendingPathComponent:OESaveStateScreenshotFile];
}

- (NSURL *)stateFileURL
{
    return [[self URL] URLByAppendingPathComponent:OESaveStateDataFile];
}

- (NSString *)systemIdentifier
{
    return [[[[self rom] game] system] systemIdentifier];
}

- (NSDictionary *)infoPlist
{
    NSDictionary *infoPlist = [NSDictionary dictionaryWithContentsOfURL:[self infoPlistURL]];
    
    if(!infoPlist)
        infoPlist = [NSDictionary dictionary];
    
    return infoPlist;
}

- (NSURL *)infoPlistURL
{
    return [[self URL] URLByAppendingPathComponent:@"Info.plist"];
}

#pragma mark -
#pragma mark Data Model Relationships

@dynamic rom;

@end
