//
//  SidebarFieldEditor.m
//  SidebarViewTest
//
//  Created by Christoph Leimbrock on 16.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OESidebarFieldEditor.h"

@interface NSTextView (ApplePrivate)
- (void)_drawRect:(NSRect)dirtyRect clip:(NSRect)aClipRect;
@end

@implementation OESidebarFieldEditor

- (id)init{
    self = [super init];
    if (self) {
	
	}
    
    return self;
}

+ (id)fieldEditor{
	static OESidebarFieldEditor* fieldEditor = nil;
	if (fieldEditor == nil){
		fieldEditor = [[OESidebarFieldEditor alloc] initWithFrame:NSZeroRect];
	
		[fieldEditor setFieldEditor:YES];
		[fieldEditor setEditable:YES];
		[fieldEditor setSelectable:YES];
		
		[fieldEditor setTextContainerInset:NSMakeSize(1, 2)];
	}
	return fieldEditor;	
}


- (void)_drawRect:(NSRect)dirtyRect clip:(NSRect)aClipRect{
	aClipRect.size.width = ceilf(aClipRect.size.width);
	aClipRect.origin.x = floorf(aClipRect.origin.x);
	
	clipRect = aClipRect;
	
	aClipRect.size.height += 2;
	aClipRect.origin.y -= 1;
	
	[super _drawRect:dirtyRect clip:aClipRect];
}

- (void)drawRect:(NSRect)dirtyRect {
	NSColor* strokeColor = [NSColor colorWithDeviceRed:0.09 green:0.153 blue:0.553 alpha:1.0];
	NSColor* backgroundColor = [NSColor whiteColor];
	
	[strokeColor setFill];
	NSRectFill([self bounds]);
	
	[backgroundColor setFill];
	NSRectFill(NSInsetRect(clipRect, 1, 1));
	
	[NSGraphicsContext saveGraphicsState];
	
	NSBezierPath* path = [NSBezierPath bezierPathWithRect:NSInsetRect(clipRect, 1, 1)];
	[path setWindingRule:NSEvenOddWindingRule];
	[path addClip];
	
	[super drawRect:[self bounds]];
	[NSGraphicsContext restoreGraphicsState];
}

- (void)dealloc {
    [super dealloc];
}

@end
