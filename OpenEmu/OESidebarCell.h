//
//  SidebarCell.h
//  SidebarViewTest
//
//  Created by Christoph Leimbrock on 16.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface OESidebarCell : NSTextFieldCell 
{
@private
	BOOL isGroup, isEditing;
}
@property (readwrite) BOOL isGroup;
@property (readwrite) BOOL isEditing;
@property (strong) NSImage* image;
@end
