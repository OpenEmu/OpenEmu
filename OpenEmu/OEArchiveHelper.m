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

#import "OEArchiveHelper.h"
#import <XADMaster/XADArchive.h>

@implementation OEArchiveHelper

+ (nullable NSURL *)urlOfExtractedFile:(NSURL *)url archiveFileIndex:(NSNumber *)archiveFileIndex
{
    
    if (!archiveFileIndex) {
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

@end
