//
//  SidebarDataSourceItem.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@protocol SidebarDataSourceItem <NSObject>
- (NSImage*)sidebarIcon;
- (NSString*)sidebarName;
- (void)setSidebarName:(NSString*)newName;
- (BOOL)isSelectableInSdebar;
- (BOOL)isEditableInSdebar;
- (BOOL)isGroupHeaderInSdebar;

- (BOOL)hasSubCollections;
@end
