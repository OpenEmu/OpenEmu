//
//  Screenshot.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 27/06/14.
//
//

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
    [self setLocation:[[url URLByStandardizingPath] absoluteString]];
}

- (NSURL*)URL
{
    return [NSURL URLWithString:[self location]];
}

- (void)prepareForDeletion
{
    [[NSFileManager defaultManager] trashItemAtURL:[self URL] resultingItemURL:nil error:nil];
}

- (void)updateFile
{
    OELibraryDatabase *database = [self libraryDatabase];
    NSURL *screenshotDirectory = [database screenshotFolderURL];
    NSString *fileName = [self name];
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
