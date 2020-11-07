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

#import "OEGameInfoHelper.h"
#import "OESQLiteDatabase.h"
#import "NSFileManager+OEHashingAdditions.h"
#import "OEDBSystem.h"
#import <XADMaster/XADArchive.h>

#import <OpenEmuSystem/OpenEmuSystem.h> // we only need OELocalizationHelper

#import "OpenEmu-Swift.h"

NSString * const OEOpenVGDBVersionKey = @"OpenVGDBVersion";
NSString * const OEOpenVGDBUpdateCheckKey = @"OpenVGDBUpdatesChecked";
NSString * const OEOpenVGDBUpdateIntervalKey = @"OpenVGDBUpdateInterval";


NSString * const OpenVGDBFileName = @"openvgdb";
NSString * const OpenVGDBDownloadURL = @"https://github.com/OpenVGDB/OpenVGDB/releases/download";
NSString * const OpenVGDBUpdateURL = @"https://api.github.com/repos/OpenVGDB/OpenVGDB/releases?page=1&per_page=1";

NSNotificationName const OEGameInfoHelperWillUpdateNotification = @"OEGameInfoHelperWillUpdateNotificationName";
NSNotificationName const OEGameInfoHelperDidChangeUpdateProgressNotification = @"OEGameInfoHelperDidChangeUpdateProgressNotificationName";
NSNotificationName const OEGameInfoHelperDidUpdateNotification = @"OEGameInfoHelperDidUpdateNotificationName";

@interface OEGameInfoHelper () <NSURLSessionDownloadDelegate>

@property OESQLiteDatabase *database;

@property (nonatomic) NSURLSession *downloadSession;

@property (readwrite) CGFloat downloadProgress;
@property (readwrite, getter=isUpdating) BOOL updating;

@end

@implementation OEGameInfoHelper

+ (void)initialize {
    
    if (self == [OEGameInfoHelper class]) {
        
        NSNumber *onceADayInterval = @(60 * 60 * 24 * 1);
        
        NSDictionary *defaults = @{ OEOpenVGDBVersionKey:@"",
                                    OEOpenVGDBUpdateCheckKey:[NSDate dateWithTimeIntervalSince1970:0],
                                 OEOpenVGDBUpdateIntervalKey: onceADayInterval
                                  };
        [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];
    }
}

+ (instancetype)sharedHelper {
    
    static OEGameInfoHelper *sharedHelper = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        
        sharedHelper = [[OEGameInfoHelper alloc] init];

        NSError *error = nil;
        NSURL *databaseURL = [sharedHelper databaseFileURL];
        if (![databaseURL checkResourceIsReachableAndReturnError:nil]) {
            
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0), ^{
                
                NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
                [defaults removeObjectForKey:OEOpenVGDBUpdateCheckKey];
                [defaults removeObjectForKey:OEOpenVGDBVersionKey];

                [sharedHelper checkForUpdatesWithHandler:^(NSURL * _Nullable newRelease, NSString * _Nullable tag) {
                    
                    if (newRelease && tag) {
                        [sharedHelper installVersion:tag withDownloadURL:newRelease];
                    }
                }];
            });
            
        } else {
            
            OESQLiteDatabase *database = [[OESQLiteDatabase alloc] initWithURL:databaseURL error:&error];
            
            if (database != nil) {
                sharedHelper.database = database;
            } else {
                [NSApp presentError:error];
            }

            // check for updates
            dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0);
            dispatch_async(queue, ^{
                
                NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
                NSDate *lastUpdateCheck = [defaults objectForKey:OEOpenVGDBUpdateCheckKey];
                double updateInterval = [defaults doubleForKey:OEOpenVGDBUpdateIntervalKey];

                if ([[NSDate date] timeIntervalSinceDate:lastUpdateCheck] > updateInterval) {
                    
                    NSLog(@"Check for updates (%f > %f)", [[NSDate date] timeIntervalSinceDate:lastUpdateCheck], updateInterval);
                    
                    [sharedHelper checkForUpdatesWithHandler:^(NSURL * _Nullable newRelease, NSString * _Nullable version) {
                        
                        if (newRelease && version) {
                            [sharedHelper installVersion:version withDownloadURL:newRelease];
                        }
                    }];
                }
            });
        }
    });
    
    return sharedHelper;
}

#pragma mark -

- (NSURL *)databaseFileURL {
    NSURL *applicationSupport = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil];
    return [applicationSupport URLByAppendingPathComponent:@"OpenEmu/openvgdb.sqlite"];
}

#pragma mark -

- (void)checkForUpdatesWithHandler:(void (^)(NSURL * _Nullable newURL, NSString * _Nullable newVersion))handler {
    
    DLog();
    
    NSURL   *url = [NSURL URLWithString:OpenVGDBUpdateURL];
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url cachePolicy:NSURLRequestReloadIgnoringCacheData timeoutInterval:30];
    [request setValue:@"OpenEmu" forHTTPHeaderField:@"User-Agent"];

    NSURLSessionDataTask *task = [[NSURLSession sharedSession] dataTaskWithRequest:request completionHandler:^(NSData * _Nullable result, NSURLResponse * _Nullable response, NSError * _Nullable error) {
        
        if (result != nil) {
            
            NSArray *releases = [NSJSONSerialization JSONObjectWithData:result options:NSJSONReadingAllowFragments error:nil];
            
            if (releases != nil) {
                
                NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
                
                NSString *currentVersion = [defaults stringForKey:OEOpenVGDBVersionKey];
                NSString *nextVersion = currentVersion;
                
                [defaults setObject:[NSDate date] forKey:OEOpenVGDBUpdateCheckKey];
                
                for (id aRelease in releases) {
                    
                    if ([aRelease isKindOfClass:[NSDictionary class]] && aRelease[@"tag_name"]) {
                        
                        NSString *tagName = aRelease[@"tag_name"];
                        if ([tagName compare:nextVersion] != NSOrderedSame) {
                            nextVersion = tagName;
                        }
                    }
                }
                
                if (![nextVersion isEqualToString:currentVersion]) {
                    
                    NSString *URLString = [NSString stringWithFormat:@"%@/%@/%@.zip", OpenVGDBDownloadURL, nextVersion, OpenVGDBFileName];
                    
                    DLog(@"Updating OpenVGDB version from %@ to %@.", currentVersion.length > 0 ? currentVersion : @"(none)", nextVersion);
                    
                    handler([NSURL URLWithString:URLString], nextVersion);
                    
                    return;
                    
                } else {
                    DLog(@"OpenVGDB not updated.");
                }
            }
        }
        
        handler(nil, nil);
    }];
    
    [task resume];
}

- (void)cancelUpdate {
    
    DLog(@"Cancelling OpenVGDB download.");
    
    [self.downloadSession invalidateAndCancel];
}

- (void)installVersion:(NSString *)versionTag withDownloadURL:(NSURL *)url {
    
    if (url != nil) {
        
        dispatch_async(dispatch_get_main_queue(), ^{
            
            self.updating = YES;
            
            [[NSNotificationCenter defaultCenter] postNotificationName:OEGameInfoHelperWillUpdateNotification object:self];
            
            self.downloadProgress = 0.0;
            self.downloadVersion = versionTag;

            NSURLRequest *request = [NSURLRequest requestWithURL:url];
            
            self.downloadSession = [NSURLSession sessionWithConfiguration:[NSURLSessionConfiguration defaultSessionConfiguration] delegate:self delegateQueue:[NSOperationQueue mainQueue]];
            
            NSURLSessionDownloadTask *downloadTask = [self.downloadSession downloadTaskWithRequest:request];
            
            DLog(@"Starting OpenVGDB download.");
            
            [downloadTask resume];
        });
    }
}

#pragma mark -

- (id)executeQuery:(NSString*)sql error:(NSError *__autoreleasing *)error {
    return [self.database executeQuery:sql error:error];
}

- (NSDictionary * _Nullable)gameInfoWithDictionary:(NSDictionary *)gameInfo {
    
    @synchronized(self) {
        
        NSArray *keys = [gameInfo.allKeys filteredArrayUsingPredicate:[NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary *bindings) {
            return gameInfo[evaluatedObject] != [NSNull null];
        }]];
        gameInfo = [gameInfo dictionaryWithValuesForKeys:keys];

        NSMutableDictionary *resultDict = [NSMutableDictionary dictionary];

        NSString *systemIdentifier = gameInfo[@"systemIdentifier"];
        NSString *header = gameInfo[@"header"];
        NSString *serial = gameInfo[@"serial"];
        NSString *md5 = gameInfo[@"md5"];
        NSURL *url = gameInfo[@"URL"];
        NSNumber *archiveFileIndex = gameInfo[@"archiveFileIndex"];

        if (![self database]) {
            return resultDict;
        }

        __block BOOL isSystemWithHashlessROM = [self hashlessROMCheckForSystem:systemIdentifier];
        __block BOOL isSystemWithROMHeader   = [self headerROMCheckForSystem:systemIdentifier];
        __block BOOL isSystemWithROMSerial   = [self serialROMCheckForSystem:systemIdentifier];

        __block int headerSize = [self sizeOfROMHeaderForSystem:systemIdentifier];

        NSString * const DBMD5Key = @"romHashMD5";
        NSString * const DBROMExtensionlessFileNameKey = @"romExtensionlessFileName";
        NSString * const DBROMHeaderKey = @"romHeader";
        NSString * const DBROMSerialKey = @"romSerial";

        NSString __block *key = nil, __block *value = nil;
        void(^determineQueryParams)(void) = ^{
            
            if (value != nil) {
                return;
            }

            // check if the system is 'hashless' in the db and instead match by filename (Arcade)
            if (isSystemWithHashlessROM) {
                key = DBROMExtensionlessFileNameKey;
                value = url.lastPathComponent.stringByDeletingPathExtension.lowercaseString;
            }
            // check if the system has headers in the db and instead match by header
            else if (isSystemWithROMHeader) {
                key = DBROMHeaderKey;
                value = [header uppercaseString];
            }
            // check if the system has serials in the db and instead match by serial
            else if (isSystemWithROMSerial) {
                key = DBROMSerialKey;
                value = [serial uppercaseString];
            } else {
                // if rom has no header we can use the hash we calculated at import
                if (headerSize == 0 && (value = md5) != nil) {
                    key = DBMD5Key;
                }
                value = value.uppercaseString;
            }
        };

        determineQueryParams();

        // try to fetch header, serial or hash from file
        if (!value) {
            
            BOOL removeFile = NO;
            NSURL *romURL = [self _urlOfExtractedFile:url archiveFileIndex:archiveFileIndex];
            if (!romURL) { // rom is no archive, use original file URL
                romURL = url;
            } else {
                 removeFile = YES;
            }

            OEFile *file = [OEFile fileWithURL:romURL error:NULL];
            if (file == nil)
                return nil;

            NSString *headerFound = [OEDBSystem headerForFile:file forSystem:systemIdentifier];
            NSString *serialFound = [OEDBSystem serialForFile:file forSystem:systemIdentifier];

            if (!headerFound && !serialFound) {
                
                [[NSFileManager defaultManager] hashFileAtURL:romURL headerSize:headerSize md5:&value error:nil];
                
                key = DBMD5Key;
                value = value.uppercaseString;

                if (value) {
                    resultDict[@"md5"] = value;
                }
                
            } else {
                
                if (headerFound) {
                    resultDict[@"header"] = headerFound;
                }
                if (serialFound) {
                    resultDict[@"serial"] = serialFound;
                }
            }

            if (removeFile) {
                [[NSFileManager defaultManager] removeItemAtURL:romURL error:nil];
            }
        }

        determineQueryParams();

        if (!value) {
            
            // Still nothing to look up, force determineQueryParams to use Hashes
            isSystemWithHashlessROM = NO;
            isSystemWithROMHeader = NO;
            isSystemWithROMSerial = NO;
            headerSize = 0;

            determineQueryParams();
        }

        if (!value) {
            return nil;
        }

        NSString *sql = [NSString stringWithFormat:@"SELECT DISTINCT releaseTitleName as 'gameTitle', releaseCoverFront as 'boxImageURL', releaseDescription as 'gameDescription', regionName as 'region'\
                         FROM ROMs rom LEFT JOIN RELEASES release USING (romID) LEFT JOIN REGIONS region on (regionLocalizedID=region.regionID)\
                         WHERE %@ = '%@'", key, value];

        __block NSArray *result = [_database executeQuery:sql error:nil];
        
        if (result.count > 1) {
            
            // the database holds multiple regions for this rom (probably WORLD rom)
            // so we pick the preferred region if it's available or just any if not
            NSString *preferredRegion = [[OELocalizationHelper sharedHelper] regionName];
            // TODO: Associate regionName's in the database with -[OELocalizationHelper regionName]'s
            if([preferredRegion isEqualToString:@"North America"]) preferredRegion = @"USA";

            for (id obj in result) {
                if ([obj[@"region"] isEqualToString:preferredRegion]) {
                    result = @[obj];
                    break;
                }
            }
            
            // preferred region not found, just pick one
            if (result.count != 1) {
                result = @[result.lastObject];
            }
        }
        
        // remove the region key so the result can be directly passed to OEDBGame
        for (NSMutableDictionary *obj in result) {
            [obj removeObjectForKey:@"region"];
        }
        
        [resultDict addEntriesFromDictionary:result.lastObject];
        
        return resultDict;
    }
}

- (BOOL)hashlessROMCheckForSystem:(NSString *)system {
    
    if (!self.database) {
        return NO;
    }
    
    NSString *sql = [NSString stringWithFormat:@"select systemhashless as 'hashless' from systems where systemoeid = '%@'", system];
    NSArray *result = [self.database executeQuery:sql error:nil];
    return [result.lastObject[@"hashless"] boolValue];
}

- (BOOL)headerROMCheckForSystem:(NSString *)system {
    
    if (!self.database) {
        return NO;
    }
    
    NSString *sql = [NSString stringWithFormat:@"select systemheader as 'header' from systems where systemoeid = '%@'", system];
    
    NSArray *result = [self.database executeQuery:sql error:nil];
    
    return [result.lastObject[@"header"] boolValue];
}

- (BOOL)serialROMCheckForSystem:(NSString *)system {
    
    if (!self.database) {
        return NO;
    }
    
    NSString *sql = [NSString stringWithFormat:@"select systemserial as 'serial' from systems where systemoeid = '%@'", system];
    
    NSArray *result = [self.database executeQuery:sql error:nil];
    
    return [result.lastObject[@"serial"] boolValue];
}

- (int)sizeOfROMHeaderForSystem:(NSString*)system {
    
    if (!self.database) {
        return NO;
    }

    NSString *sql = [NSString stringWithFormat:@"select systemheadersizebytes as 'size' from systems where systemoeid = '%@'", system];
    NSArray *result = [self.database executeQuery:sql error:nil];
    return [result.lastObject[@"size"] intValue];
}

- (NSURL*)_urlOfExtractedFile:(NSURL *)url archiveFileIndex:(id)archiveFileIndex {
    
    if (!archiveFileIndex && archiveFileIndex != [NSNull null]) {
        return nil;
    }

    NSString *path = url.path;

    if (!path || ![[NSFileManager defaultManager] fileExistsAtPath:path]) {
        return nil;
    }
    
    XADArchive *archive;
    @try {
        archive = [XADArchive archiveForFile:path];
    } @catch (NSException *exception) {
        archive = nil;
    }

    int entryIndex = [archiveFileIndex intValue];
    if (archive && archive.numberOfEntries > entryIndex) {
        
        NSString *formatName = archive.formatName;
        if ([formatName isEqualToString:@"MacBinary"]) {
            return nil;
        }

        if ([formatName isEqualToString:@"LZMA_Alone"]) {
            return nil;
        }

        if ([formatName isEqualToString:@"ISO 9660"]) {
            return nil;
        }

        if (![archive entryHasSize:entryIndex] || [archive entryIsEncrypted:entryIndex] || [archive entryIsDirectory:entryIndex] || [archive entryIsArchive:entryIndex]) {
            return nil;
        }

        NSString *folder = temporaryDirectoryForDecompressionOfPath(path);
        NSString *name = [archive nameOfEntry:entryIndex];
        if (name.pathExtension.length == 0 && path.pathExtension.length > 0) {
            // this won't do. Re-add the archive's extension in case it's .smc or the like
            name = [name stringByAppendingPathExtension:path.pathExtension];
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
        } @catch (NSException *exception) {
            success = NO;
        }
        if (success) {
            return tmpURL;
        } else {
            [fm removeItemAtPath:folder error:nil];
        }
    }
    return nil;
}

#pragma mark - NSURLSessionDownloadDelegate

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task didCompleteWithError:(NSError *)error {
    
    DLog(@"OpenVGDB download did complete: %@", error ? error.localizedDescription : @"no errors");
    
    self.updating = NO;
    self.downloadProgress = 0.0;
    
    [self.downloadSession finishTasksAndInvalidate];
    self.downloadSession = nil;
    
    [self OE_postDidUpdateNotification];
}

- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask didWriteData:(int64_t)bytesWritten totalBytesWritten:(int64_t)totalBytesWritten totalBytesExpectedToWrite:(int64_t)totalBytesExpectedToWrite {
    
    self.downloadProgress = (CGFloat)totalBytesWritten / (CGFloat)totalBytesExpectedToWrite;
    
    [[NSNotificationCenter defaultCenter] postNotificationName:OEGameInfoHelperDidChangeUpdateProgressNotification object:self];
}

- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask didFinishDownloadingToURL:(NSURL *)location {

    DLog(@"OpenVGDB download did finish downloading temporary data.");

    XADArchive *archive = nil;
    @try {
        archive = [XADArchive archiveForFile:location.path];
    } @catch (NSException *exc) {
        archive = nil;
    }
    
    NSURL *url = self.databaseFileURL;
    NSURL *databaseFolder = url.URLByDeletingLastPathComponent;
    [archive extractTo:databaseFolder.path];
    
    DLog(@"OpenVGDB extracted to database folder.");
    
    OESQLiteDatabase *database = [[OESQLiteDatabase alloc] initWithURL:url error:nil];
    self.database = database;
    
    if (database) {
        [[NSUserDefaults standardUserDefaults] setObject:self.downloadVersion forKey:OEOpenVGDBVersionKey];
    }
    
    self.updating = NO;
    self.downloadProgress = 1.0;
    self.downloadVersion = nil;
    
    [self OE_postDidUpdateNotification];
}

#pragma mark - Private

- (void)OE_postDidUpdateNotification {
    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSNotificationCenter defaultCenter] postNotificationName:OEGameInfoHelperDidUpdateNotification object:self];
    });
}

@end
