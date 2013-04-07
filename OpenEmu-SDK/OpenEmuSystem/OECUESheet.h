/*
 Copyright (c) 2012, OpenEmu Team

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

#import <Foundation/Foundation.h>

// Options to handle 'broken' cue files:
#define OECUESheetImproveReadingByUsingBinExtension 1 // If a referneced file does not have an extension we automatically append .bin and see if that works better
#define OECUESheetImproveReadingByUsingSheetBin 1 // if there's only one referenced file which is not available we check if there's a bin file with the same name as the cue and use that instead

@interface OECUESheet : NSObject

- (id)initWithPath:(NSString *)path;
- (id)initWithPath:(NSString *)path andReferencedFilesDirectory:(NSString *)referencedFiles;

#pragma mark - File Handling
- (BOOL)moveReferencedFilesToPath:(NSString *)newDirectory withError:(NSError **)outError;
- (BOOL)copyReferencedFilesToPath:(NSString *)newDirectory withError:(NSError **)outError;

- (BOOL)allFilesAvailable;
- (NSArray *)referencedFiles;
- (NSString *)dataTrackPath;

@end
