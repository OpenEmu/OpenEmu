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

#import "OEDBScreenshot+CoreDataProperties.h"
#import "OEDBRom.h"

#import "OELibraryDatabase.h"

#import "OpenEmu-Swift.h"

NS_ASSUME_NONNULL_BEGIN

NSString * const OEDBScreenshotImportRequired = @"OEDBScreenshotImportRequired";

@implementation OEDBScreenshot

+ (nullable instancetype)createObjectInContext:(NSManagedObjectContext *)context forROM:(OEDBRom *)rom withFile:(NSURL*)file
{
    OEDBScreenshot *screenshot = nil;
    if([file checkResourceIsReachableAndReturnError:nil])
    {
        NSString *name = file.lastPathComponent.stringByDeletingPathExtension;
        screenshot = [OEDBScreenshot createObjectInContext:context];
        screenshot.URL = file;
        screenshot.rom = rom;
        screenshot.timestamp = [NSDate date];
        screenshot.name = name;

        [screenshot updateFile];
        [screenshot save];
    }
    return screenshot;
}

+ (NSString *)entityName
{
    return @"Screenshot";
}

#pragma mark - NSPasteboardWriting

- (NSArray *)writableTypesForPasteboard:(NSPasteboard *)pasteboard
{
    return @[(NSString*)kUTTypeFileURL, (NSString*)kUTTypeImage];
}

- (nullable id)pasteboardPropertyListForType:(NSString *)type
{
    NSURL *url = self.URL.absoluteURL;
    if([type isEqualToString:(NSString*)kUTTypeFileURL])
    {
        return [url pasteboardPropertyListForType:type];
    }
    else if([type isEqualToString:(NSString *)kUTTypeImage])
    {
        NSImage *image = [[NSImage alloc] initWithContentsOfURL:url];
        return [image pasteboardPropertyListForType:type];
    }

    return nil;
}

#pragma mark - Core Data Properties

- (void)setURL:(NSURL *)url
{
    NSURL *screenshotDirectory = self.libraryDatabase.screenshotFolderURL;
    self.location = [url URLRelativeToURL:screenshotDirectory].relativeString;
}

- (NSURL *)URL
{
    NSURL *screenshotDirectory = self.libraryDatabase.screenshotFolderURL;
    return [NSURL URLWithString:self.location relativeToURL:screenshotDirectory];
}

- (NSURL *)screenshotURL
{
    return self.URL;
}

- (void)prepareForDeletion
{
    NSURL *url = self.URL;
    if(url)
        [[NSFileManager defaultManager] trashItemAtURL:url resultingItemURL:nil error:nil];
}

- (void)updateFile
{
    OELibraryDatabase *database = self.libraryDatabase;
    NSURL *screenshotDirectory = database.screenshotFolderURL;
    NSString *fileName = [NSURL validFilenameFromString:self.name];
    NSString *fileExtension = @"png";
    NSURL *targetURL = [screenshotDirectory URLByAppendingPathComponent:[NSString stringWithFormat:@"%@.%@", fileName, fileExtension]]. URLByStandardizingPath;
    NSFileManager *fm = [NSFileManager defaultManager];
    NSURL  *sourceURL = self.URL;

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
        self.URL = targetURL;
    }
}


#pragma mark - QLPreviewItem


- (NSURL *)previewItemURL
{
    return [self URL];
}


- (NSString *)previewItemTitle
{
    return [self name];
}


@end

NS_ASSUME_NONNULL_END
