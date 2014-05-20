/*
 Copyright (c) 2013, OpenEmu Team

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

#import "OEGameInfoHelper.h"
#import "OESQLiteDatabase.h"
#import "NSURL+OELibraryAdditions.h"
#import "NSFileManager+OEHashingAdditions.h"
#import "OEHUDAlert.h"
#import <XADMaster/XADArchive.h>

#import <OpenEmuSystem/OpenEmuSystem.h> // we only need OELocalizationHelper

NSString * const OEOpenVGDBVersionKey        = @"OpenVGDBVersion";
NSString * const OEOpenVGDBUpdateCheckKey    = @"OpenVGDBUpdatesChecked";
NSString * const OEOpenVGDBUpdateIntervalKey = @"OpenVGDBUpdateInterval";


NSString * const OpenVGDBFileName = @"openvgdb";
NSString * const OpenVGDBDownloadURL = @"https://github.com/OpenVGDB/OpenVGDB/releases/download";
NSString * const OpenVGDBUpdateURL = @"https://api.github.com/repos/OpenVGDB/OpenVGDB/releases?page=1&per_page=1";

NSString * const OEGameInfoHelperWillUpdateNotificationName = @"OEGameInfoHelperWillUpdateNotificationName";
NSString * const OEGameInfoHelperDidChangeUpdateProgressNotificationName = @"OEGameInfoHelperDidChangeUpdateProgressNotificationName";
NSString * const OEGameInfoHelperDidUpdateNotificationName = @"OEGameInfoHelperDidUpdateNotificationName";

@interface OEGameInfoHelper () <NSURLDownloadDelegate>
@property OESQLiteDatabase *database;

@property NSString *downloadPath;
@property NSInteger expectedLength, downloadedSize;
@property (strong) NSURLDownload *fileDownload;
@end
@implementation OEGameInfoHelper
@synthesize updating=_updating, downloadVerison=_downloadVerison;

+ (void)initialize
{
    if(self == [OEGameInfoHelper class])
    {
        NSDictionary *defaults = @{ OEOpenVGDBVersionKey:@"",
                                    OEOpenVGDBUpdateCheckKey:[NSDate dateWithTimeIntervalSince1970:0],
                                 OEOpenVGDBUpdateIntervalKey:@(60*60*24*1) // once a day
                                  };
        [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];
    }
}

+ (id)sharedHelper
{
    static OEGameInfoHelper *sharedHelper = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedHelper = [[OEGameInfoHelper alloc] init];

        NSError *error = nil;
        NSURL *databaseURL = [sharedHelper databaseFileURL];
        if(![databaseURL checkResourceIsReachableAndReturnError:nil])
        {
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0), ^{
                NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
                [standardUserDefaults removeObjectForKey:OEOpenVGDBUpdateCheckKey];
                [standardUserDefaults removeObjectForKey:OEOpenVGDBVersionKey];

                NSString *tag = nil;
                NSURL *newRelease = [sharedHelper checkForUpdates:&tag];
                [sharedHelper installVersion:tag withDownloadURL:newRelease];
            });
        }
        else
        {
            OESQLiteDatabase *database = [[OESQLiteDatabase alloc] initWithURL:databaseURL error:&error];
            if(database != nil)
                [sharedHelper setDatabase:database];
            else
                [NSApp presentError:error];


            // check for updates
            dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0);
            dispatch_async(queue, ^{
                NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
                NSDate *lastUpdateCheck = [standardUserDefaults  objectForKey:OEOpenVGDBUpdateCheckKey];
                double updateInterval   = [standardUserDefaults  doubleForKey:OEOpenVGDBUpdateIntervalKey];

                if([[NSDate date] timeIntervalSinceDate:lastUpdateCheck] > updateInterval)
                {
                    NSLog(@"Check for updates (%f > %f)", [[NSDate date] timeIntervalSinceDate:lastUpdateCheck], updateInterval);
                    NSString *version = nil;
                    NSURL *newRelease = [sharedHelper checkForUpdates:&version];
                    if(newRelease != nil)
                        dispatch_async(dispatch_get_main_queue(), ^{
                            [sharedHelper installVersion:version withDownloadURL:newRelease];
                        });
                }
            });
        }
    });
    return sharedHelper;
}
#pragma mark -
- (NSURL*)databaseFileURL
{
    NSURL *applicationSupport = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil];
    return [applicationSupport URLByAppendingPathComponent:@"OpenEmu/openvgdb.sqlite"];
}
#pragma mark -
- (NSURL*)checkForUpdates:(NSString**)outVersion
{
    DLog();
    NSError *error = nil;
    NSURL   *url = [NSURL URLWithString:OpenVGDBUpdateURL];
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url cachePolicy:NSURLRequestReloadIgnoringCacheData timeoutInterval:30];
    [request setValue:@"OpenEmu" forHTTPHeaderField:@"User-Agent"];

    NSData *result = [NSURLConnection sendSynchronousRequest:request returningResponse:NULL error:&error];
    if(result != nil)
    {
        NSArray *releases = [NSJSONSerialization JSONObjectWithData:result options:NSJSONReadingAllowFragments error:&error];
        if(releases != nil)
        {
            NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
            NSString *currentVersion = [defaults objectForKey:OEOpenVGDBVersionKey];
            NSString *nextVersion = [defaults objectForKey:OEOpenVGDBVersionKey];
            [defaults setObject:[NSDate date] forKey:OEOpenVGDBUpdateCheckKey];

            for(id aRelease in releases)
            {
                if([aRelease isKindOfClass:[NSDictionary class]] && [aRelease objectForKey:@"tag_name"])
                {
                    
                    NSString *tagName = [aRelease objectForKey:@"tag_name"];
                    if([tagName compare:nextVersion] != NSOrderedSame)
                        nextVersion = tagName;
                }
            }
            
            if(![nextVersion isEqualToString:currentVersion])
            {
                NSString *URLString = [NSString stringWithFormat:@"%@/%@/%@.zip", OpenVGDBDownloadURL, nextVersion, OpenVGDBFileName];
                if(outVersion != NULL)
                    *outVersion = nextVersion;
                DLog(@"Update from %@ to %@", currentVersion, nextVersion);
                return [NSURL URLWithString:URLString];
            } else DLog(@"No Update");

        }
    }
    
    if(outVersion != NULL)
        *outVersion = nil;
    return nil;
}

- (void)cancelUpdate
{
    [[self fileDownload] cancel];
    [self setFileDownload:nil];
    _updating = YES;
    _downloadProgress = 1.0;
    _downloadVerison  = nil;

    [[NSNotificationCenter defaultCenter] postNotificationName:OEGameInfoHelperDidUpdateNotificationName object:self];
}

- (void)installVersion:(NSString*)versionTag withDownloadURL:(NSURL*)url
{
    if(url != nil)
    {
        _updating = YES;
        [[NSNotificationCenter defaultCenter] postNotificationName:OEGameInfoHelperWillUpdateNotificationName object:self];
        _downloadProgress = 0.0;
        _downloadVerison = versionTag;

        dispatch_async(dispatch_get_main_queue(), ^{
            NSURLRequest  *request = [NSURLRequest requestWithURL:url];
            [self setFileDownload:[[NSURLDownload alloc] initWithRequest:request delegate:self]];
        });
    }
}
#pragma mark -
- (id)executeQuery:(NSString*)sql error:(NSError *__autoreleasing *)error
{
    return [_database executeQuery:sql error:error];
}

- (NSDictionary*)gameInfoForROM:(OEDBRom*)rom error:(NSError *__autoreleasing*)error
{
    // TODO: this method could use some cleanup
    // TODO: remove extracted rom if necessary
    if(![self database]) return @{};
    
    BOOL isSystemWithHashlessROM = [self hashlessROMCheckForSystem:[[[rom game] system] systemIdentifier]];
    BOOL isSystemWithROMHeader   = [self headerROMCheckForSystem:[[[rom game] system] systemIdentifier]];
    BOOL isSystemWithROMSerial   = [self serialROMCheckForSystem:[[[rom game] system] systemIdentifier]];

    NSString * const DBMD5Key= @"romHashMD5";
    NSString * const DBCRCKey= @"romHashCRC";
    NSString * const DBROMFileNameKey= @"romFileName";
    NSString * const DBROMHeaderKey= @"romHeader";
    NSString * const DBROMSerialKey= @"romSerial";

    NSString __block *key = nil, __block *value = nil;
    void(^determineQueryParams)(void) = ^{
        if(value != nil) return;
        
        // check if the system is 'hashless' in the db and instead match by filename (arcade)
        if (isSystemWithHashlessROM)
        {
            key = DBROMFileNameKey;
            value = [[[rom location] lastPathComponent] lowercaseString];
        }
        // check if the system has headers in the db and instead match by header
        else if (isSystemWithROMHeader)
        {
            key = DBROMHeaderKey;
            value = [[rom header] uppercaseString];
        }
        // check if the system has serials in the db and instead match by serial
        else if (isSystemWithROMSerial)
        {
            key = DBROMSerialKey;
            value = [[rom serial] uppercaseString];
        }
        else
        {
            int headerSize = [self sizeOfROMHeaderForSystem:[[[rom game] system] systemIdentifier]];
            
            // if rom has no header we can use the hash we calculated at import
            if(headerSize == 0 && (value = [rom md5HashIfAvailable]) != nil)
            {
                key = DBMD5Key;
            }
            else if(headerSize == 0 && (value = [rom crcHashIfAvailable]) != nil)
            {
                key = DBCRCKey;
            }
            value = [value uppercaseString];
        }
    };
    
    determineQueryParams();
    
    // try to fetch header, serial or hash from file
    if(value == nil)
    {
        NSURL *romURL = [self _urlOfExtractedRom:rom];
        if(romURL == nil) // rom is no archive, use original file URL
            romURL = [rom URL];
        
        NSString *systemIdentifier = [[[rom game] system] systemIdentifier];
        NSString *headerFound = [OEDBSystem headerForFileWithURL:romURL forSystem:systemIdentifier];
        NSString *serialFound = [OEDBSystem serialForFileWithURL:romURL forSystem:systemIdentifier];

        if(headerFound == nil && serialFound == nil)
        {
            int headerSize = [self sizeOfROMHeaderForSystem:[[[rom game] system] systemIdentifier]];
            [[NSFileManager defaultManager] hashFileAtURL:romURL headerSize:headerSize md5:&value crc32:nil error:error];
            key   = DBMD5Key;
            value = [value uppercaseString];
        }
        else
        {
            [rom setHeader:headerFound];
            [rom setSerial:serialFound];
        }
    }
    
    determineQueryParams();
    
    if(value == nil)
        return nil;
    
    NSString *sql = [NSString stringWithFormat:@"SELECT DISTINCT releaseTitleName as 'gameTitle', releaseCoverFront as 'boxImageURL', releaseDescription as 'gameDescription', regionName as 'region'\
                     FROM ROMs rom LEFT JOIN RELEASES release USING (romID) LEFT JOIN REGIONS region on (regionLocalizedID=region.regionID)\
                     WHERE %@ = '%@'", key, value];

    __block NSArray *result = [_database executeQuery:sql error:error];
    if([result count] > 1)
    {
        // the database holds multiple regions for this rom (probably WORLD rom)
        // so we pick the preferred region if it's available or just any if not
        NSString *preferredRegion = [[OELocalizationHelper sharedHelper] regionName];
        [result enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            if([[obj valueForKey:@"region"] isEqualToString:preferredRegion])
            {
                *stop = YES;
                result = @[obj];
            }
        }];

        // preferred region not found, just pick one
        if([result count] != 1)
            result = @[[result lastObject]];
    }

    // remove the region key so the result can be directly passed to OEDBGame
    [result enumerateObjectsUsingBlock:^(NSMutableDictionary *obj, NSUInteger idx, BOOL *stop) {
        [obj removeObjectForKey:@"region"];
    }];

    return [result lastObject];
}

- (BOOL)hashlessROMCheckForSystem:(NSString*)system
{
    if(![self database]) return NO;
    
    NSString *sql = [NSString stringWithFormat:@"select systemhashless as 'hashless' from systems where systemoeid = '%@'", system];
    NSArray *result = [[self database] executeQuery:sql error:nil];
    return [[[result lastObject] objectForKey:@"hashless"] boolValue];
}

- (BOOL)headerROMCheckForSystem:(NSString*)system
{
    if(![self database]) return NO;
    
    NSString *sql = [NSString stringWithFormat:@"select systemheader as 'header' from systems where systemoeid = '%@'", system];
    NSArray *result = [[self database] executeQuery:sql error:nil];
    return [[[result lastObject] objectForKey:@"header"] boolValue];
}

- (BOOL)serialROMCheckForSystem:(NSString*)system
{
    if(![self database]) return NO;
    
    NSString *sql = [NSString stringWithFormat:@"select systemserial as 'serial' from systems where systemoeid = '%@'", system];
    NSArray *result = [[self database] executeQuery:sql error:nil];
    return [[[result lastObject] objectForKey:@"serial"] boolValue];
}

- (int)sizeOfROMHeaderForSystem:(NSString*)system
{
    if(![self database]) return NO;

    NSString *sql = [NSString stringWithFormat:@"select systemheadersizebytes as 'size' from systems where systemoeid = '%@'", system];
    NSArray *result = [[self database] executeQuery:sql error:nil];
    return [[[result lastObject] objectForKey:@"size"] intValue];
}

- (NSURL*)_urlOfExtractedRom:(OEDBRom*)rom
{
    if([rom archiveFileIndex] == nil)
        return nil;

    NSString *path = [[rom URL] path];
    XADArchive *archive;
    @try {
        archive = [XADArchive archiveForFile:path];
    }
    @catch (NSException *exception)
    {
        archive = nil;
    }

    int entryIndex = [[rom archiveFileIndex] intValue];
    if (archive && [archive numberOfEntries] > entryIndex)
    {
        NSString *formatName = [archive formatName];
        if ([formatName isEqualToString:@"MacBinary"])
            return nil;
        
        if( [formatName isEqualToString:@"LZMA_Alone"])
            return nil;
        
        if (![archive entryHasSize:entryIndex] || [archive entryIsEncrypted:entryIndex] || [archive entryIsDirectory:entryIndex] || [archive entryIsArchive:entryIndex])
            return nil;

        NSString *folder = temporaryDirectoryForDecompressionOfPath(path);
        NSString *name = [archive nameOfEntry:entryIndex];
        if ([[name pathExtension] length] == 0 && [[path pathExtension] length] > 0) {
            // this won't do. Re-add the archive's extension in case it's .smc or the like
            name = [name stringByAppendingPathExtension:[path pathExtension]];
        }
        NSString *tmpPath = [folder stringByAppendingPathComponent:name];

        BOOL isdir;
        NSURL *tmpURL = [NSURL fileURLWithPath:tmpPath];
        NSFileManager *fm = [NSFileManager new];
        if ([fm fileExistsAtPath:tmpPath isDirectory:&isdir] && !isdir) {
            return tmpURL;
        }

        BOOL success = YES;
        @try {
            success = [archive _extractEntry:entryIndex as:tmpPath deferDirectories:NO dataFork:YES resourceFork:NO];
        }
        @catch (NSException *exception) {
            success = NO;
        }
        if (success)
            return tmpURL;
        else
            [fm removeItemAtPath:folder error:nil];
    }
    return nil;
}
#pragma mark - NSURLDownload Delegate
- (void)download:(NSURLDownload *)download decideDestinationWithSuggestedFilename:(NSString *)filename
{
    _downloadPath = [NSTemporaryDirectory() stringByAppendingPathComponent:[NSString stringWithFormat:@"OpenVGDB.%@", [NSString stringWithUUID]]];
    [download setDestination:_downloadPath allowOverwrite:NO];
}

- (void)download:(NSURLDownload *)download didCreateDestination:(NSString *)path
{}

- (void)download:(NSURLDownload *)download didReceiveDataOfLength:(NSUInteger)length
{
    _downloadedSize += length;
    _downloadProgress = (CGFloat) _downloadedSize /  (CGFloat) _expectedLength;

    [[NSNotificationCenter defaultCenter] postNotificationName:OEGameInfoHelperDidChangeUpdateProgressNotificationName object:self];
}

- (void)download:(NSURLDownload *)download didReceiveResponse:(NSURLResponse *)response
{
    _expectedLength = [response expectedContentLength];
}

- (void)downloadDidFinish:(NSURLDownload *)download
{
    XADArchive *archive = nil;
    @try
    {
        archive = [XADArchive archiveForFile:_downloadPath];
    }
    @catch (NSException *exc)
    {
        archive = nil;
    }

    NSURL *url = [self databaseFileURL];
    NSURL *databaseFolder = [url URLByDeletingLastPathComponent];
    [archive extractTo:[databaseFolder path]];

    [[NSFileManager defaultManager] removeItemAtPath:_downloadPath error:nil];

    OESQLiteDatabase *database = [[OESQLiteDatabase alloc] initWithURL:url error:nil];
    [self setDatabase:database];

    if(database) [[NSUserDefaults standardUserDefaults] setObject:_downloadVerison forKey:OEOpenVGDBVersionKey];

    _updating = NO;
    _downloadProgress = 1.0;
    _downloadVerison  = nil;
    [self setFileDownload:nil];
    [[NSNotificationCenter defaultCenter] postNotificationName:OEGameInfoHelperDidUpdateNotificationName object:self];
}

- (void)download:(NSURLDownload *)download didFailWithError:(NSError *)error
{
    _updating = NO;
    _downloadProgress = 1.0;
    [self setFileDownload:nil];
    [[NSNotificationCenter defaultCenter] postNotificationName:OEGameInfoHelperDidUpdateNotificationName object:self];
}

@end
