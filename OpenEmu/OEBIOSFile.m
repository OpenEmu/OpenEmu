/*
 Copyright (c) 2015, OpenEmu Team

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

#import "OEBIOSFile.h"
#import "OECorePlugin.h"
#import "OEHUDAlert+DefaultAlertsAdditions.h"
#import "NSFileManager+OEHashingAdditions.h"

NS_ASSUME_NONNULL_BEGIN

NSString * const OEBIOSFileGuideURLString = @"https://github.com/OpenEmu/OpenEmu/wiki/User-guide:-BIOS-files";
@implementation OEBIOSFile

+ (NSString *)biosPath {
    static NSString *biosPath;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        biosPath = [NSString pathWithComponents:@[[NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) lastObject], @"OpenEmu", @"BIOS"]];
    });
    return biosPath;
}

#pragma mark - File Handling

/**
 Determine if BIOS file exists and has correct MD5 hash.
 @param file    NSDictionary to required file
 @returns YES if file exists with correct MD5.
 */
- (BOOL)isBIOSFileAvailable:(NSDictionary <NSString *, NSString *> *)file
{
    NSString *md5;
    NSError  *error;
    NSString *biosSystemFileName = file[@"Name"];
    NSString *biosSystemFileMD5  = file[@"MD5"];

    NSString *destination = [[OEBIOSFile biosPath] stringByAppendingPathComponent:biosSystemFileName];
    NSURL *url = [NSURL fileURLWithPath:destination];

    if([url checkResourceIsReachableAndReturnError:nil] && [[NSFileManager defaultManager] hashFileAtURL:url md5:&md5 crc32:nil error:&error])
    {
        if([md5 caseInsensitiveCompare:biosSystemFileMD5] == NSOrderedSame)
            return YES;
        else
        {
            DLog(@"Incorrect MD5, deleting %@", url);
            [[NSFileManager defaultManager] removeItemAtURL:url error:nil];
        }
    }

    return NO;
}

/**
 Check if all required files by the current core plugin are available
 @param files    NSArray to valid required files by the core plugin
 @returns YES if all required files exist. Returns NO and alert with missing files.
 */
- (BOOL)allRequiredFilesAvailableForSystemIdentifier:(NSArray *)files
{
    BOOL missingFileStatus = NO;
    NSSortDescriptor *sortedRequiredFiles = [NSSortDescriptor sortDescriptorWithKey:@"Name" ascending:YES selector:@selector(caseInsensitiveCompare:)];
    NSArray *validRequiredFiles = [files sortedArrayUsingDescriptors:@[sortedRequiredFiles]];
    NSMutableString *missingFilesList = [[NSMutableString alloc] init];

    for(NSDictionary *validRequiredFile in validRequiredFiles)
    {
        NSString *biosFilename = validRequiredFile[@"Name"];
        NSString *biosDescription = validRequiredFile[@"Description"];
        BOOL biosOptional = [validRequiredFile[@"Optional"] boolValue];

        // Check if the required files exist and are optional
        if (![self isBIOSFileAvailable:validRequiredFile] && !biosOptional)
        {
            missingFileStatus = YES;
            [missingFilesList appendString:[NSString stringWithFormat:@"%@\n\t\"%@\"\n\n", biosDescription, biosFilename]];
        }
    }
    // Alert the user of missing BIOS/system files that are required for the core
    if (missingFileStatus)
    {
        if([[OEHUDAlert missingBIOSFilesAlert:[NSString stringWithString:missingFilesList]] runModal] == NSAlertSecondButtonReturn)
        {
            [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:OEBIOSFileGuideURLString]];
        }

        return NO;
    }

    return YES;
}

/**
 Determine if file is BIOS file and copy to BIOS folder
 @param url    url to file
 @returns YES if BIOS file and copied successfully. Returns NO if an error occurred.
 */
- (BOOL)checkIfBIOSFileAndImportAtURL:(NSURL *)url
{
    NSString *md5 = nil;
    NSError  *error = nil;
    NSFileManager *fileManager = [NSFileManager defaultManager];

    [fileManager hashFileAtURL:url md5:&md5 crc32:nil error:&error];

    return [self checkIfBIOSFileAndImportAtURL:url withMD5:md5];
}

/**
 Determine if file with MD5 is BIOS file and copy to BIOS folder
 @param url    url to file
 @param md5    md5 of file
 @returns YES if BIOS file and copied successfully. Returns NO if an error occurred.
 */
- (BOOL)checkIfBIOSFileAndImportAtURL:(NSURL *)url withMD5:(NSString *)md5
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *biosPath = [OEBIOSFile biosPath];

    // Copy known BIOS / System Files to BIOS folder
    for(NSDictionary *validFile in [OECorePlugin requiredFiles])
    {
        NSString *biosSystemFileName = [validFile valueForKey:@"Name"];
        NSString *biosSystemFileMD5  = [validFile valueForKey:@"MD5"];
        NSError  *error              = nil;

        NSString *destination = [biosPath stringByAppendingPathComponent:biosSystemFileName];
        NSURL    *destinationURL = [NSURL fileURLWithPath:destination];
        if([md5 caseInsensitiveCompare:biosSystemFileMD5] == NSOrderedSame)
        {
            if(![fileManager createDirectoryAtURL:[NSURL fileURLWithPath:biosPath] withIntermediateDirectories:YES attributes:nil error:&error])
            {
                DLog(@"Could not create directory before copying bios at %@", url);
                DLog(@"%@", error);
                error = nil;
            }

            if(![fileManager copyItemAtURL:url toURL:destinationURL error:&error])
            {
                DLog(@"Could not copy bios file %@ to %@", url, destinationURL);
            }

            return YES;
        }
    }

    return NO;
}

@end

NS_ASSUME_NONNULL_END
