//
//  Screenshot.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 27/06/14.
//
//

#import "OEDBScreenshot.h"
#import "OEDBRom.h"


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
    [self setLocation:[url absoluteString]];
}

- (NSURL*)URL
{
    return [NSURL URLWithString:[self location]];
}
@end
