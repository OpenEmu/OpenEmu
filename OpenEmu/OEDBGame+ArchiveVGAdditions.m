//
//  OEDBGame+ArchiveVGAdditions.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 04.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEDBGame+ArchiveVGAdditions.h"
#import "OEDBRom+ArchiveVGAdditions.h"
@implementation OEDBGame (ArchiveVGAdditions)
- (NSArray*)batchCallDescription
{
    NSMutableArray* descriptions = [NSMutableArray arrayWithCapacity:[[self roms] count]];
    for(OEDBRom *rom in [self roms])
    {
        [descriptions addObject:[rom batchCallDescription]];
    }
    return descriptions;
}
@end
