//
//  SidebarGroupItem.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OESidebarDataSourceItem.h"


@interface OESidebarGroupItem : NSObject <OESidebarDataSourceItem> {
@private
    NSString* name;
}
+ (id)groupItemWithName:(NSString*)name;
@property (readwrite, copy) NSString* name;
@end
