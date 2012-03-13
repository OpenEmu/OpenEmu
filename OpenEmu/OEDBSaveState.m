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

// NSString *const OESaveStateInfoCreationDateKey   = @"Creation Date";
// NSString *const OESaveStateInfoBookmarkDataKey   = @"Bookmark Data";

@interface OEDBSaveState ()
+ (id)OE_newSaveStateInContext:(NSManagedObjectContext*)context;
- (BOOL)OE_createBundleAtURL:(NSURL*)url withStateFile:(NSURL*)stateFile error:(NSError*__autoreleasing*)error;
- (NSDictionary*)OE_newInfoPlist;
@end

@implementation OEDBSaveState

+ (id)OE_newSaveStateInContext:(NSManagedObjectContext*)context{
	NSEntityDescription *description = [NSEntityDescription entityForName:@"SaveState" inManagedObjectContext:context];
	OEDBSaveState *result = [[OEDBSaveState alloc] initWithEntity:description insertIntoManagedObjectContext:context];
	
	[result setTimestamp:[NSDate date]];
	
	return result;
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
    
    NSError  *error              = nil;
    NSString *fileName           = [NSURL validFilenameFromString:name];
    NSURL    *saveStateFolderURL = [database stateFolderURLForSystem:[[rom game] system]];
    NSURL    *saveStateURL       = [saveStateFolderURL URLByAppendingPathComponent:[NSString stringWithFormat:@"%@.%@", fileName, OESaveStateSuffix]];
    
    saveStateURL = [saveStateURL uniqueURLUsingBlock:^NSURL *(NSInteger triesCount) {
        return [saveStateFolderURL URLByAppendingPathComponent:[NSString stringWithFormat:@"%@ %d.%@", fileName, triesCount, OESaveStateSuffix]];
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
    
    NSURL           *infoPlistURL   = [bundleURL URLByAppendingPathComponent:@"Info.plist"];
    NSDictionary    *infoPlist      = [self OE_newInfoPlist];
    if(![infoPlist writeToURL:infoPlistURL atomically:YES])
    {
        return NO;
    }
    
    return YES;
}

- (NSDictionary*)OE_newInfoPlist
{
    NSMutableDictionary *infoPlist = [[NSMutableDictionary alloc] initWithCapacity:4];
    // Usual Document stuff
#warning TODO: implement
    
    // Save State Values
    [infoPlist setObject:OESaveStateLatestVersion   forKey:OESaveStateInfoVersionKey];
    [infoPlist setObject:[self name]                forKey:OESaveStateInfoNameKey];
    [infoPlist setObject:[self coreIdentifier]      forKey:OESaveStateInfoCoreIdentifierKey];

    if([self userDescription])
        [infoPlist setObject:[self userDescription] forKey:OESaveStateInfoDescriptionKey];

    return infoPlist;
}
#pragma mark -
#pragma mark Data Model Properties
@dynamic name, userDescription, timestamp;
@dynamic coreIdentifier, bookmarkData;

- (NSURL*)URL
{
    return [NSURL URLByResolvingBookmarkData:[self bookmarkData] options:0 relativeToURL:nil bookmarkDataIsStale:nil error:nil];
}

- (void)setURL:(NSURL *)url
{
    NSError *error = nil;
    NSData  *bookmarkData = [url bookmarkDataWithOptions:NSURLBookmarkCreationMinimalBookmark includingResourceValuesForKeys:nil relativeToURL:nil error:&error];
    if(!bookmarkData)
    {
        NSLog(@"error while creating bookmark data for state save");
        NSLog(@"%@", [error localizedDescription]);
        return;        
    }
    
    [self setBookmarkData:bookmarkData];
}

- (NSURL*)screenshotURL
{
    return [NSURL URLWithString:OESaveStateScreenshotFile relativeToURL:[self URL]];
}

- (NSURL*)stateFileURL
{
    return [[self URL] URLByAppendingPathComponent:OESaveStateDataFile];
}

- (NSString*)systemIdentifier
{
    return [[[[self rom] game] system] systemIdentifier];
}
#pragma mark -
#pragma mark Data Model Relationships
@dynamic rom;

@end
