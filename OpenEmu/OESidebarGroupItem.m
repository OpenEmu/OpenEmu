//
//  SidebarGroupItem.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OESidebarGroupItem.h"


@implementation OESidebarGroupItem

+ (id)groupItemWithName:(NSString*)name
{
	OESidebarGroupItem* newItem = [[self alloc] init];
	newItem.name = name;
	return [newItem autorelease];
}

- (NSImage*)sidebarIcon
{
	return nil;
}
- (NSString*)sidebarName
{
	return self.name;
}
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
	return YES;
}

- (BOOL)hasSubCollections
{
    return FALSE;
}

- (void)setSidebarName:(NSString *)newName
{}


- (void)dealloc 
{
	self.name = nil;
	
    [super dealloc];
}

@synthesize name;
@end
