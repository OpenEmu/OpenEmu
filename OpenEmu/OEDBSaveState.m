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


#define OESaveStateSuffix           @"oesavestate"
#define OESaveStateDataFile         @"State"
#define OESaveStateScreenshotFile   @"ScreenShot"
#define OESaveStateLatestVersion    @"1.0"

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

@interface OEDBSaveState ()
+ (id)OE_newSaveStateInContext:(NSManagedObjectContext*)context;
- (BOOL)OE_createBundleAtURL:(NSURL*)url withStateFile:(NSURL*)stateFile error:(NSError*__autoreleasing*)error;
- (void)replaceStateFileWithFile:(NSURL*)stateFile;
- (NSURL*)infoPlistURL;
- (NSDictionary*)infoPlist;
@end

@implementation OEDBSaveState
+ (NSArray*)allStates
{
    return [self allStatesInDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (NSArray*)allStatesInDatabase:(OELibraryDatabase*)database
{
    NSManagedObjectContext *context = [database managedObjectContext];
    NSFetchRequest *request = [NSFetchRequest fetchRequestWithEntityName:@"SaveState"];
    return [context executeFetchRequest:request error:nil];
}


+ (OEDBSaveState*)saveStateWithURL:(NSURL*)url
{
    return [self saveStateWithURL:url inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (OEDBSaveState*)saveStateWithURL:(NSURL*)url inDatabase:(OELibraryDatabase*)database
{
    NSManagedObjectContext  *context    = [database managedObjectContext];
    NSFetchRequest          *request    = [NSFetchRequest fetchRequestWithEntityName:@"SaveState"];
    
    NSString *absoluteString = [url absoluteString];
    if([absoluteString characterAtIndex:[absoluteString length]-1] != '/')
    {
        absoluteString = [absoluteString stringByAppendingString:@"/"];
    }
    
    NSPredicate *predicate  = [NSPredicate predicateWithFormat:@"location == %@", absoluteString];
    [request setPredicate:predicate];
    
    return [[context executeFetchRequest:request error:nil] lastObject];
}


+ (id)OE_newSaveStateInContext:(NSManagedObjectContext*)context{
	NSEntityDescription *description = [NSEntityDescription entityForName:@"SaveState" inManagedObjectContext:context];
	OEDBSaveState *result = [[OEDBSaveState alloc] initWithEntity:description insertIntoManagedObjectContext:context];
	
	[result setTimestamp:[NSDate date]];
	
	return result;
}

+ (id)createSaveStateWithURL:(NSURL*)url
{
    return [self createSaveStateWithURL:url inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)createSaveStateWithURL:(NSURL *)url inDatabase:(OELibraryDatabase*)database
{
    OEDBSaveState *newSaveState = [self OE_newSaveStateInContext:[database managedObjectContext]];
    [newSaveState setLocation:[url absoluteString]];
    if(![newSaveState reloadFromInfoPlist])
    {
        // setting path to nil so file won't be deleted in -remove
        [newSaveState setLocation:nil];
        [newSaveState remove];
        newSaveState = nil;
    }
    
    NSError *error = nil;
//TODO: use validation here instead of save
    if(![[newSaveState managedObjectContext] save:&error])
    {
        [newSaveState setLocation:nil];
        [newSaveState remove];
        newSaveState = nil;
        
        NSLog(@"State verification failed: %@", error);
    }

    return newSaveState;
}

+ (id)createSaveStateNamed:(NSString*)name forRom:(OEDBRom*)rom core:(OECorePlugin*)core withFile:(NSURL*)stateFileURL
{
    return [self createSaveStateNamed:name forRom:rom core:core withFile:stateFileURL inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)createSaveStateNamed:(NSString*)name forRom:(OEDBRom*)rom core:(OECorePlugin*)core withFile:(NSURL*)stateFileURL inDatabase:(OELibraryDatabase *)database
{
    OEDBSaveState *newSaveState = [self OE_newSaveStateInContext:[database managedObjectContext]];
    [newSaveState setName:name];
    [newSaveState setRom:rom];
    [newSaveState setCoreIdentifier:[core bundleIdentifier]];
    [newSaveState setCoreVersion:[core version]];
    [newSaveState setTimestamp:[NSDate date]];
    
    if([name hasPrefix:OESaveStateSpecialNamePrefix])
    {
        name = NSLocalizedString(name, @"Localized special save state name");
    }
    
    NSError  *error              = nil;
    NSString *fileName           = [NSURL validFilenameFromString:name];
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

        return nil;
    }

    return newSaveState;
}

- (BOOL)OE_createBundleAtURL:(NSURL*)bundleURL withStateFile:(NSURL*)stateFile error:(NSError*__autoreleasing*)error
{
    NSDictionary *directoryAttributes = nil;
    if(![[NSFileManager defaultManager] createDirectoryAtURL:bundleURL withIntermediateDirectories:YES attributes:directoryAttributes error:error])
    {
        return NO;
    }

    NSURL *stateURLInBundle = [bundleURL URLByAppendingPathComponent:OESaveStateDataFile];
    if(![[NSFileManager defaultManager] moveItemAtURL:stateFile toURL:stateURLInBundle error:error])
    {
        return NO;
    }
    [self setURL:bundleURL];
    if(![self rewriteInfoPlist])
    {
        return NO;
    }
    
    return YES;
}

+ (void)updateStateWithPath:(NSString*)path
{
    NSRange range     = [path rangeOfString:@".oesavestate" options:NSCaseInsensitiveSearch];
    if(range.location == NSNotFound) return;
    
    NSFileManager   *defaultManager = [NSFileManager defaultManager];
    NSString        *saveStatePath  = [path substringToIndex:range.location+range.length];
    NSURL           *url            = [NSURL fileURLWithPath:saveStatePath];
    OEDBSaveState   *saveState      = [OEDBSaveState saveStateWithURL:url]; 

    if(saveState)
        if([defaultManager fileExistsAtPath:saveStatePath])
        {
           [saveState reloadFromInfoPlist]; 
        }
        else
        {
            [saveState remove];
            return;
        }
    else if([defaultManager fileExistsAtPath:saveStatePath])
    {
        saveState = [OEDBSaveState createSaveStateWithURL:url];
    }

    [saveState moveFileToDefaultLocation];
}
#pragma mark -
- (BOOL)reloadFromInfoPlist
{
    NSDictionary    *infoPlist  = [self infoPlist];
    NSString        *version    = [infoPlist valueForKey:OESaveStateInfoVersionKey];
    if([version isEqualTo:@"1.0"])
    {
        NSString *infoName              = [infoPlist valueForKey:OESaveStateInfoNameKey];
        NSString *infoCoreIdentifier    = [infoPlist valueForKey:OESaveStateInfoCoreIdentifierKey];
        NSString *infoCoreVersion       = [infoPlist valueForKey:OESaveStateInfoCoreVersionKey];
        NSString *infoUserDescription   = [infoPlist valueForKey:OESaveStateInfoDescriptionKey];
        NSString *infoRomMD5            = [infoPlist valueForKey:OESaveStateInfoROMMD5Key];
        NSDate   *infoTimestamp         = [infoPlist valueForKey:OESaveStateInfoTimestampKey];
        
        OEDBRom  *rom                   = [OEDBRom romWithMD5HashString:infoRomMD5 error:nil];
        if(!infoName || !infoCoreIdentifier || !infoRomMD5 || !rom)
            return NO;
    
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
        return NO;
    }
    
    return YES;
}

- (BOOL)rewriteInfoPlist
{
    NSMutableDictionary *infoPlist = [[self infoPlist] mutableCopy];
    
    // Save State Values
    [infoPlist setObject:OESaveStateLatestVersion   forKey:OESaveStateInfoVersionKey];
    [infoPlist setObject:[self name]                forKey:OESaveStateInfoNameKey];
    [infoPlist setObject:[self coreIdentifier]      forKey:OESaveStateInfoCoreIdentifierKey];
    if([self coreVersion])
        [infoPlist setObject:[self coreVersion]         forKey:OESaveStateInfoCoreVersionKey];
    [infoPlist setObject:[[self rom] md5Hash]       forKey:OESaveStateInfoROMMD5Key];
    [infoPlist setObject:[self timestamp]           forKey:OESaveStateInfoTimestampKey];
    if([self userDescription])
        [infoPlist setObject:[self userDescription] forKey:OESaveStateInfoDescriptionKey];

    if(![infoPlist writeToURL:[self infoPlistURL] atomically:YES])
        return NO;
    
    return YES;
}

- (void)remove
{
    [[NSFileManager defaultManager] removeItemAtURL:[self URL] error:nil];
    NSManagedObjectContext *moc = [self managedObjectContext];
    [moc deleteObject:self];
    [moc save:nil];
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

- (void)replaceStateFileWithFile:(NSURL*)stateFile
{
    [[NSFileManager defaultManager] removeItemAtURL:[self stateFileURL] error:nil];
    [[NSFileManager defaultManager] copyItemAtURL:stateFile toURL:[self stateFileURL] error:nil];
}

- (void)moveFileToDefaultLocation
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
}
#pragma mark -
#pragma mark Data Model Properties
@dynamic name, userDescription, timestamp;
@dynamic coreIdentifier, location, coreVersion;

- (NSURL*)URL
{
    return [NSURL URLWithString:[self location]];
}

- (void)setURL:(NSURL *)url
{
    [self setLocation:[url absoluteString]];
}

- (NSURL*)screenshotURL
{
    return [[self URL] URLByAppendingPathComponent:OESaveStateScreenshotFile];
}

- (NSURL*)stateFileURL
{
    return [[self URL] URLByAppendingPathComponent:OESaveStateDataFile];
}

- (NSString*)systemIdentifier
{
    return [[[[self rom] game] system] systemIdentifier];
}

- (NSDictionary*)infoPlist
{
    NSDictionary *infoPlist = [NSDictionary dictionaryWithContentsOfURL:[self infoPlistURL]];
    
    if(!infoPlist)
        infoPlist = [NSDictionary dictionary];
    
    return infoPlist;
}

- (NSURL*)infoPlistURL
{
    return [[self URL] URLByAppendingPathComponent:@"Info.plist"];
}
#pragma mark -
#pragma mark Data Model Relationships
@dynamic rom;

@end
