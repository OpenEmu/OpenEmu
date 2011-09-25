//
//  OEControlsKeyButton.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 25.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <AppKit/AppKit.h>

@interface OEControlsKeyButton : NSView{
	NSString* title;
	
	id target;
	SEL action;
	
	NSCellStateValue state;
	NSPoint highlightPoint;
}

@property (nonatomic) NSCellStateValue state;
@property (nonatomic, copy) NSString* title;
@property (assign) id target;
@property SEL action;

@property NSPoint highlightPoint;
@end
