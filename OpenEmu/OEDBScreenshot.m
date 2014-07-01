/*
 Copyright (c) 2014, OpenEmu Team

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

#import "OEDBScreenshot.h"
#import "OEDBRom.h"

#import "OELibraryDatabase.h"

#import "NSURL+OELibraryAdditions.h"

NSString * const OEDBScreenshotImportRequired = @"OEDBScreenshotImportRequired";

@implementation OEDBScreenshot
+ (NSString*)entityName
{
    return @"Screenshot";
}

#pragma mark - Core Data Properties
@dynamic location, name, timestamp, userDescription, rom;

- (void)setURL:(NSURL *)url
{
    NSURL *screenshotDirectory = [[self libraryDatabase] screenshotFolderURL];
    [self setLocation:[[url urlRelativeToURL:screenshotDirectory] relativeString]];
}

- (NSURL*)URL
{
    NSURL *screenshotDirectory = [[self libraryDatabase] screenshotFolderURL];
    return [NSURL URLWithString:[self location] relativeToURL:screenshotDirectory];
}

- (void)prepareForDeletion
{
    NSURL *url = [self URL];
    if(url)
        [[NSFileManager defaultManager] trashItemAtURL:url resultingItemURL:nil error:nil];
}

- (void)updateFile
{
    OELibraryDatabase *database = [self libraryDatabase];
    NSURL *screenshotDirectory = [database screenshotFolderURL];
    NSString *fileName = [NSURL validFilenameFromString:[self name]];
    NSString *fileExtension = @"png";
    NSURL *targetURL = [[screenshotDirectory URLByAppendingPathComponent:[NSString stringWithFormat:@"%@.%@", fileName, fileExtension]] URLByStandardizingPath];
    NSFileManager *fm = [NSFileManager defaultManager];
    NSURL  *sourceURL = [self URL];

    if([targetURL isEqualTo:sourceURL])
    {
        return;
    }

    if([targetURL checkResourceIsReachableAndReturnError:nil])
    {
        targetURL = [targetURL uniqueURLUsingBlock:^NSURL *(NSInteger triesCount) {
            return [screenshotDirectory URLByAppendingPathComponent:[NSString stringWithFormat:@"%@ %ld.%@", fileName, (long)triesCount, fileExtension]];
        }];
    }

    if([fm moveItemAtURL:sourceURL toURL:targetURL error:nil])
    {
        [self setURL:targetURL];
    }
}
@end
