//
//  OEDBRom+ArchiveVGAdditions.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 04.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEDBRom+ArchiveVGAdditions.h"
#import "ArchiveVG.h"
@implementation OEDBRom (ArchiveVGAdditions)
- (NSDictionary*)batchCallDescription
{
    NSMutableDictionary *description = [NSMutableDictionary dictionaryWithCapacity:3];
    NSString* romName = [[[self URL] lastPathComponent] stringByDeletingPathExtension];
    if(romName)
        [description setValue:romName forKey:AVGGameListItemRomFileKey];
    if([self md5HashIfAvailable])
        [description setValue:[self md5HashIfAvailable] forKey:AVGGameListItemMD5Key];
    if([self crcHashIfAvailable])
        [description setValue:[self crcHashIfAvailable] forKey:AVGGameListItemCRC32Key];
    if([[self fileSize] boolValue])
        [description setValue:[NSString stringWithFormat:@"%@", [self fileSize]] forKey:AVGGameListItemSizeKey];
    
    [description setValue:[[self objectID] URIRepresentation] forKey:AVGGameListItemRequestAttributeKey];
    
    return description;
}
@end
