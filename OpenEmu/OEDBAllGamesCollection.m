//
//  OEDBAllGamesCollection.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 24.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEDBAllGamesCollection.h"


@implementation OEDBAllGamesCollection

- (NSImage*)sidebarIcon{
    return [NSImage imageNamed:@"collections_smart"];
}
- (NSString*)sidebarName{
    return NSLocalizedString(@"All Games", @"");
}

- (void)setSidebarName:(NSString*)newName{
    NSLog(@"OEDBAllGamesCollection: can not change name!");
}

- (BOOL)isSelectableInSdebar{
    return YES;
}
- (BOOL)isEditableInSdebar{
    return NO;
}
- (BOOL)isGroupHeaderInSdebar{
    return NO;
}

- (BOOL)hasSubCollections{
    return NO;
}
@end
