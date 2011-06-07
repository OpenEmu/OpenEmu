	//
//  OEFieldEditor.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 18.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEFieldEditor.h"

@interface TestField : NSTextField {}
@end
#pragma mark -
@implementation OEFieldEditor
@synthesize borderColor;

- (id)initWithFrame:(NSRect)frame{
    self = [super initWithFrame:frame];
    if (self) {
		self.borderColor = [NSColor greenColor];
		[self setAutoresizesSubviews:NO];
		
		textView = [[NSTextField alloc] initWithFrame:NSMakeRect(10, 10, 80, 80)];
		[self setHidden:YES];
		[textView setBezeled:NO];
		[self setHidden:YES];
		
		textView = [[NSTextField alloc] initWithFrame:NSMakeRect(10, 10, 80, 80)];
		[textView setBezeled:NO];
		[textView setAllowsEditingTextAttributes:NO];
		
		[self setWantsLayer:YES];
		
		[self addSubview:textView];
		[self setFrame:self.frame];
    }
    
    return self;
}

- (void)dealloc{
	[textView release];
	textView = nil;
	
    [super dealloc];
}

#pragma mark -
- (void)drawRect:(NSRect)dirtyRect{
	[super drawRect:dirtyRect];
	
	[self.borderColor setStroke];
	
	NSBezierPath* borderPath = [NSBezierPath bezierPathWithRect:NSInsetRect([self bounds], 0.5, 0.5)];
	[borderPath stroke];

	[[NSColor whiteColor] setStroke];
	borderPath = [NSBezierPath bezierPathWithRect:NSInsetRect([self bounds], 1.5, 1.5)];
	[borderPath stroke];
}
#pragma mark -
- (void)setFrameSize:(NSSize)newSize{
	[super setFrameSize:newSize];
	
	if(newSize.width >= 2)
		newSize.width -= 2;
	if(newSize.height >= 2)
		newSize.height -= 2;
	
	[textView setFrameSize:newSize];
	[textView setFrameOrigin:NSMakePoint(1, 1)];
}

#pragma mark -
- (void)setString:(NSString*)newString{
	[textView setStringValue:newString];
}
- (NSString*)string{
	return [textView stringValue];
}
#pragma mark -
- (void)setAlignment:(NSTextAlignment)alignment{
	[textView setAlignment:alignment];
}
- (NSTextAlignment)alignment{
	return [textView alignment];
}

- (void)setFont:(NSFont*)newFont{
	[textView setFont:newFont];
}
- (NSFont*)font{
	return [textView font];
}
#pragma mark -
- (void)setDelegate:(id)delegate{
	[textView setDelegate:delegate];
}
- (id)delegate{
	return [textView delegate];
}
@end