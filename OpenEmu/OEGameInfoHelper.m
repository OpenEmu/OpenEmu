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

@interface OEGameInfoHelper ()
@property OESQLiteDatabase *database;
@end
@implementation OEGameInfoHelper
+ (id)sharedHelper
{
    static OEGameInfoHelper *sharedHelper = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedHelper = [[OEGameInfoHelper alloc] init];

        NSError *error = nil;
        NSURL   *url   = [[NSBundle mainBundle] URLForResource:@"db sample" withExtension:@"sqlite"];
        NSURL *applicationSupport = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil];
        NSURL *alternativeDBUrl = [applicationSupport URLByAppendingPathComponent:@"OpenEmu/db sample.sqlite"];
        if([alternativeDBUrl checkResourceIsReachableAndReturnError:nil])
            url = alternativeDBUrl;

        if(![alternativeDBUrl checkResourceIsReachableAndReturnError:nil])
        {
            OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:@"Unable to find ~/Library/Application Support/OpenEmu/db sample.sqlite" defaultButton:@":(" alternateButton:nil];
            [alert runModal];
            return;
        }

        OESQLiteDatabase *database = [[OESQLiteDatabase alloc] initWithURL:url error:&error];
        if(database)
        {
            [sharedHelper setDatabase:database];
        }
        else
        {
            [NSApp presentError:error];
        }
    });
    return sharedHelper;
}

- (NSDictionary*)gameInfoForROM:(OEDBRom*)rom error:(NSError *__autoreleasing*)error
{
    if(![self database]) return @{};

    NSString *key, *value;
    
    int headerSize = [self sizeOfROMHeaderForSystem:[[rom game] system]];
    if(headerSize == 0) // rom has no header, that means we can use the hash we calculated at import
    {
        if((value = [rom md5HashIfAvailable]) != nil)
        {
            key = @"romHashMD5";
        }
        else if((value = [rom crcHashIfAvailable]) != nil)
        {
            key = @"romHashCRC";
        }
        /*
         else if((value = [rom sha1HashIfAvailable]) != nil)
         {
         key = @"romHashSHA1";
         }
         */
    }
    else
    {
        key = @"romHashMD5";

        NSURL *url = [self _urlOfExtractedRom:rom];
        if(url == nil) url = [rom URL];
        if(![[NSFileManager defaultManager] hashFileAtURL:url headerSize:headerSize md5:&value crc32:nil error:error])
            return nil;
    }

    NSString *sql = [NSString stringWithFormat:@"SELECT DISTINCT releaseTitleName as 'name', releaseCoverFront as 'boxImageURL', releaseDescription as 'gameDescription'\
                     FROM ROMs rom LEFT JOIN RELEASES release USING (romID) LEFT JOIN REGIONS region on (regionLocalizedID=region.regionID)\
                     WHERE %@ = '%@'", key, [value uppercaseString]];

    NSArray *result = [[self database] executeQuery:sql error:error];
    return [result lastObject];
}

- (int)sizeOfROMHeaderForSystem:(OEDBSystem*)system
{
    if(![self database]) return 0;

    NSString *sql = [NSString stringWithFormat:@"select systemheadersizebytes as 'size' from systems where systemoeid = '%@'", [system systemIdentifier]];
    NSArray *result = [[self database] executeQuery:sql error:nil];
    return [[[result lastObject] objectForKey:@"size"] intValue];
}

- (NSURL*)_urlOfExtractedRom:(OEDBRom*)rom
{
    NSString *path = [[rom URL] path];
    XADArchive *archive = [XADArchive archiveForFile:path];
    if (archive && [archive numberOfEntries] == 1)
    {
        NSString *formatName = [archive formatName];
        if ([formatName isEqualToString:@"MacBinary"])
            return nil;

        if (![archive entryHasSize:0] || [archive entryIsEncrypted:0] || [archive entryIsDirectory:0] || [archive entryIsArchive:0])
            return nil;

        NSString *folder = temporaryDirectoryForDecompressionOfPath(path);
        NSString *name = [archive nameOfEntry:0];
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
            success = [archive _extractEntry:0 as:tmpPath];
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

@end
