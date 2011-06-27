//
//  OESearchFieldCell.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 03.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OEUIDrawingUtils.h"

@interface OESearchFieldCell : NSSearchFieldCell {
@private
    NSDictionary* active;
	NSDictionary* inactive;
	NSDictionary* disabled;
	
	NSDictionary* placeholder;
	
	NSDictionary* current;
}
#pragma mark -
- (NSRect)imageRectForState:(OEUIState)state;
- (void)setStylesForState:(OEUIState)state;
- (NSImage*)searchButtonImageForState:(OEUIState)state;
- (NSImage*)alternateSearchButtonImageForState:(OEUIState)state;
@end

@interface OESearchField : NSSearchField {
@private
	BOOL wasEnabled;
}
@end