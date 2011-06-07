//
//  OEDBRom  (DataSourceAdditions).h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "OEDBRom.h"
#import "OEDBConsole.h"
#import "OEDBCollection.h"
#import "OEDBSmartCollection.h"

#import <ImageKit/ImageKit.h>

#import "CoverGridDataSourceItem.h"
#import "CoverFlowDataSourceItem.h"
#import "SideBarDataSourceItem.h"
#import "ListViewDataSourceItem.h"

@interface OEDBRom (DataSourceAdditions) <CoverGridDataSourceItem, CoverFlowDataSourceItem, ListViewDataSourceItem>
@end

@interface OEDBConsole (DataSourceAdditions) <SidebarDataSourceItem> 
@end

@interface OEDBCollection (DataSourceAdditions) <SidebarDataSourceItem> 
@end

@interface OEDBSmartCollection (DataSourceAdditions) <SidebarDataSourceItem> 
@end