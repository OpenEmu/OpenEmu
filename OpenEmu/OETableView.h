//
//  OETableView.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 03.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface OETableView : NSTableView 
{
@private
	NSColor *selectionColor;
}
- (void)setHeaderClickable:(BOOL)flag;
@property (strong, readwrite) NSColor *selectionColor;
@end
