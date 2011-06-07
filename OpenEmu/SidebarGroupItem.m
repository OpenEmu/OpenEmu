//
//  SidebarGroupItem.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "SidebarGroupItem.h"


@implementation SidebarGroupItem

+ (id)groupItemWithName:(NSString*)name{
	SidebarGroupItem* newItem = [[self alloc] init];
	newItem.name = name;
	return [newItem autorelease];
}

- (NSImage*)sidebarIcon{
	return nil;
}
- (NSString*)sidebarName{
	return self.name;
}
- (BOOL)isSelectableInSdebar{
	return NO;
}
- (BOOL)isEditableInSdebar{
	return NO;
}
- (BOOL)isGroupHeaderInSdebar{
	return YES;
}

- (void)setSidebarName:(NSString *)newName{}

- (id)copyWithZone:(NSZone *)zone{
    SidebarGroupItem *copy = [[self class] allocWithZone:zone];

    [copy setName:self.name];
    
    return copy;
}


- (void)dealloc {
	self.name = nil;
	
    [super dealloc];
}

@synthesize name;
@end
