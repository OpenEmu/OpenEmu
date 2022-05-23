// Copyright (c) 2022, OpenEmu Team
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the OpenEmu Team nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#import "XADArchive+OE.h"
@import XADMaster.XADArchive;

@implementation XADArchive(NSExceptionHandling)

+ (XADArchive *)OE_archiveForFileAtURL:(NSURL *)url
{
    return [self OE_archiveForFileAtPath:url.path];
}

+ (XADArchive *)OE_archiveForFileAtPath:(NSString *)path
{
    XADArchive *archive;
    @try {
        archive = [XADArchive archiveForFile:path];
    } @catch (NSException *exception) {
        archive = nil;
    }
    return archive;
}

- (BOOL)OE_extractEntry:(int)n as:(NSString *)destfile deferDirectories:(BOOL)defer
            dataFork:(BOOL)datafork resourceFork:(BOOL)resfork
{
    BOOL success;
    @try {
        success = [self _extractEntry:n as:destfile deferDirectories:defer dataFork:datafork resourceFork:resfork];
    } @catch (NSException *exception) {
        success = NO;
    }
    return success;
}

@end
