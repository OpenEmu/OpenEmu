//
//  OETableView.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 03.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@class OEMenu;
@class OETableView;
@protocol OETableViewMenuSource <NSObject>
- (NSMenu *)tableView:(OETableView*)tableView menuForItemsAtIndexes:(NSIndexSet*)indexes;
@end

@interface OETableView : NSTableView 
{
@private
	NSColor *selectionColor;
}
- (void)setHeaderClickable:(BOOL)flag;
@property (strong, readwrite) NSColor *selectionColor;
@end
