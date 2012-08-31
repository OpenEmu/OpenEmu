//
//  OEROMImporter+OESidebarAdditions.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 31.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEROMImporter+OESidebarAdditions.h"
#import "OEImportViewController.h"
@implementation OEROMImporter (OESidebarAdditions)
- (NSImage*)sidebarIcon
{
    return [NSImage imageNamed:@"collections_smart"];
}

- (NSString*)sidebarName
{
    return NSLocalizedString(@"Importing…", @"");
}

- (void)setSidebarName:(NSString*)newName
{}

- (BOOL)isSelectableInSdebar
{
    return NO;
}
- (BOOL)isEditableInSdebar
{
    return NO;
}
- (BOOL)isGroupHeaderInSdebar
{
    return NO;
}

- (BOOL)hasSubCollections
{
    return NO;
}

- (NSString*)viewControllerClassName
{
    return [OEImportViewController className];
}

- (NSString*)sidebarID
{
    return nil;
}

@end
