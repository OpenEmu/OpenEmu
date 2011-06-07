//
//  SidebarCell.h
//  SidebarViewTest
//
//  Created by Christoph Leimbrock on 16.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface SidebarCell : NSTextFieldCell {
@private
	BOOL isGroup, isEditing;
	NSImage* image;
}
@property (readwrite) BOOL isGroup;
@property (readwrite) BOOL isEditing;
@property (retain, readwrite) NSImage* image;
@end
