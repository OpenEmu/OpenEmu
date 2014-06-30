//
//  NSUserDefaults+OEAdditions.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 30/06/14.
//
//

#import "NSUserDefaults+OEAdditions.h"

@implementation NSUserDefaults (OEAdditions)
- (NSURL*)urlForKey:(NSString*)key
{
    NSString *urlString = [self objectForKey:key];
    if(urlString == nil)
        return nil;

    NSURL *url = [NSURL URLWithString:urlString];
    return [url standardizedURL];
}
@end
