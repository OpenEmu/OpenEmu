	//
//  SidebarCell.m
//  SidebarViewTest
//
//  Created by Christoph Leimbrock on 16.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OESidebarCell.h"
#import <AppKit/NSCell.h>
#import "OESidebarFieldEditor.h"

@interface NSTextFieldCell (ApplePrivate)
- (NSDictionary*)_textAttributes;
@end

@implementation OESidebarCell
@synthesize isGroup, isEditing, image;
- (id)init {
    if ((self = [super init])) {
        [self setLineBreakMode:NSLineBreakByTruncatingTail];
        [self setSelectable:YES];
		
    }
    return self;
}

- (void)dealloc {
    [super dealloc];
}

- (id)copyWithZone:(NSZone *)zone {
    OESidebarCell *cell = (OESidebarCell *)[super copyWithZone:zone];
    cell.image = image;
    return cell;
}

- (NSRect)imageRectForBounds:(NSRect)cellFrame {
    NSRect result;
    if (image != nil) {
		NSSize iconSize = [image size];
		result = NSMakeRect(cellFrame.origin.x, cellFrame.origin.y+ (cellFrame.size.height-iconSize.height)/2, iconSize.width, iconSize.height);
        result.origin.y = ceil(result.origin.y);
    } else {
        result = NSZeroRect;
    }
    return result;
}

// We could manually implement expansionFrameWithFrame:inView: and drawWithExpansionFrame:inView: or just properly implement titleRectForBounds to get expansion tooltips to automatically work for us
- (NSRect)titleRectForBounds:(NSRect)cellFrame {
    NSRect result;
    if (image != nil) {
        CGFloat imageWidth = [image size].width;
        result = cellFrame;
        result.origin.x += (6.0 + imageWidth);
        result.size.width -= (6.0 + imageWidth);
		
		result.size.height = 14.0;
		result.origin.y += (cellFrame.size.height-result.size.height)/2;
	} else {
        result = [super titleRectForBounds:cellFrame];
    }
	
    return result;
}

- (void)editWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject event:(NSEvent *)theEvent {
	NSRect textFrame = [self titleRectForBounds:aRect];
	textFrame.size.width -= (6.0);
	
	OESidebarFieldEditor* fieldEditor = [OESidebarFieldEditor fieldEditor];
	[fieldEditor setFrame:[textObj frame]];

    [super editWithFrame:textFrame inView:controlView editor:fieldEditor delegate:anObject event: theEvent];
}

- (void)selectWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength {
	NSRect textFrame = [self titleRectForBounds:aRect];
	textFrame.size.width -= (6.0);
	
	textFrame.size.height += 4;
	textFrame.origin.y -= 2;
	
	OESidebarFieldEditor* fieldEditor = [OESidebarFieldEditor fieldEditor];
	[fieldEditor setFrame:[textObj frame]];
	
    [super selectWithFrame:textFrame inView:controlView editor:fieldEditor delegate:anObject start:selStart length:selLength];
}

- (NSText *)setUpFieldEditorAttributes:(NSText *)textObj{	
	textObj = [super setUpFieldEditorAttributes:textObj];
	
	NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:9 size:11.0];
	NSColor* textColor = [NSColor blackColor];
	
	NSMutableParagraphStyle *paragraphStyle = [[[NSMutableParagraphStyle alloc] init] autorelease];
	[paragraphStyle setLineBreakMode:NSLineBreakByTruncatingTail];
	NSDictionary* typeAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
								textColor, NSForegroundColorAttributeName,
								paragraphStyle, NSParagraphStyleAttributeName,
								font, NSFontAttributeName,
								nil];
	
	NSColor* backgroundColor = [NSColor colorWithDeviceRed:0.788 green:0.871 blue:0.992 alpha:1.0];
	NSDictionary* selectAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
									  textColor, NSForegroundColorAttributeName,
									  paragraphStyle, NSParagraphStyleAttributeName,
									  font, NSFontAttributeName,
									  backgroundColor, NSBackgroundColorAttributeName,
									  nil];
	
	if([textObj isKindOfClass:[NSTextView class]]){
//		[(NSTextView*)textObj setFont:font];
//		[(NSTextView*)textObj setTextColor:textColor];
		[(NSTextView*)textObj setFocusRingType:NSFocusRingTypeNone];
		[(NSTextView*)textObj setInsertionPointColor:textColor];

		[(NSTextView*)textObj setTypingAttributes:typeAttributes];
		[(NSTextView*)textObj setSelectedTextAttributes:selectAttributes];
	}
	
	NSColor* color = [NSColor colorWithDeviceWhite:0.72 alpha:1.0];
	[(NSTextView *)textObj setInsertionPointColor:color];
		
	self.isEditing = YES;
	
	return textObj;
}

- (NSDictionary *)_textAttributes{
	NSDictionary* typeAttributes = [super _textAttributes];
	
	if(self.isEditing){
	NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:9 size:11.0];
	NSColor* textColor = [NSColor blackColor];
	NSMutableParagraphStyle *paragraphStyle = [[[NSMutableParagraphStyle alloc] init] autorelease];
	[paragraphStyle setLineBreakMode:NSLineBreakByTruncatingTail];
	typeAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
									textColor, NSForegroundColorAttributeName,
									paragraphStyle, NSParagraphStyleAttributeName,
									font, NSFontAttributeName,
									nil];
	}
	
	return typeAttributes;
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView {
	
	NSRect imageFrame = [self imageRectForBounds:cellFrame];
    if (image != nil) {		
        if ([self drawsBackground]) {
            [[self backgroundColor] set];
            NSRectFill(imageFrame);
        }
		[image drawInRect:imageFrame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
    }
	
	NSRect titleFrame = [self titleRectForBounds:cellFrame];
	
	NSFont* font;
	NSColor* textColor;
	NSShadow* shadow = [[[NSShadow alloc] init] autorelease];;
	NSMutableParagraphStyle *paragraphStyle = [[[NSMutableParagraphStyle alloc] init] autorelease];
	[paragraphStyle setLineBreakMode:NSLineBreakByTruncatingTail];
	
	
	NSWindow* win = [controlView window];
	NSInteger row = [(NSTableView*)controlView rowAtPoint:cellFrame.origin]; 
	BOOL isSelected = [self isHighlighted];
	BOOL isActive = [win isMainWindow] && [win firstResponder]==controlView;
	
	// set style
	if(self.isGroup){
		font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:9 size:11.0];
		textColor = [NSColor colorWithDeviceRed:0.682 green:0.678 blue:0.678 alpha:1.0];
		[shadow setShadowColor:[NSColor blackColor]];
		[shadow setShadowBlurRadius:1];
		[shadow setShadowOffset:NSMakeSize(0, 1)];
		
		titleFrame = cellFrame;
		titleFrame.size.height -= 9;
		titleFrame.origin.y += 9;
		titleFrame.origin.x += 2;
		titleFrame.size.width -= 2;
		
		if(row==0){
			titleFrame.origin.x -= 10;
		}
	} else if( (isSelected && isActive)){ // selected active
		[shadow setShadowBlurRadius:1];
		[shadow setShadowOffset:NSMakeSize(0, -1)];	
		font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:9 size:11.0];
		textColor = [NSColor whiteColor];
		[shadow setShadowColor:[NSColor blackColor]];
		
		// Adjust cell frame
		/*
		 cellFrame.size.height -= 3;
		 cellFrame.origin.y += 3;
		 cellFrame.origin.x += 5;
		 cellFrame.size.width -= 10;*/
	} else if(isSelected){  // selected inactive
		font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:9 size:11.0];
		[shadow setShadowBlurRadius:1];
		[shadow setShadowOffset:NSMakeSize(0, -1)];
		[shadow setShadowColor:[NSColor colorWithDeviceRed:0.682 green:0.678 blue:0.678 alpha:1.0]];
		
		textColor = [NSColor colorWithDeviceRed:0.141 green:0.141 blue:0.141 alpha:1.0];
		/*
		 // Adjust cell frame
		 cellFrame.size.height -= 3;
		 cellFrame.origin.y += 3;
		 cellFrame.origin.x += 5;
		 cellFrame.size.width -= 10;*/
	} else {
		font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:7 size:11.0];
		textColor = [NSColor colorWithDeviceRed:0.882 green:0.878 blue:0.878 alpha:1.0];
		[shadow setShadowColor:[NSColor blackColor]];
		[shadow setShadowBlurRadius:1];
		[shadow setShadowOffset:NSMakeSize(0, -1)];
		/*
		 // Adjust cell frame
		 cellFrame.size.height -= 3;
		 cellFrame.origin.y += 3;
		 cellFrame.origin.x += 5;
		 cellFrame.size.width -= 10;*/
	}
	
	
	
	NSDictionary* attributes = [NSDictionary dictionaryWithObjectsAndKeys:
								textColor, NSForegroundColorAttributeName,
								paragraphStyle, NSParagraphStyleAttributeName,
								shadow, NSShadowAttributeName,
								font, NSFontAttributeName,
								nil];

	
	if(self.isEditing){
		/*NSColor* strokeColor = [NSColor colorWithDeviceRed:0.09 green:0.153 blue:0.553 alpha:1.0];
		NSColor* backgroundColor = [NSColor whiteColor];
		
		titleFrame.size.width -= 5;
		
		NSRect borderRect = NSInsetRect(titleFrame, 0, -2);
		[backgroundColor setFill];
		NSRectFill(borderRect);
		
		[strokeColor setStroke];
		NSBezierPath* bezierPath = [NSBezierPath bezierPathWithRect:NSInsetRect(borderRect, 0.5, 0.5)];
		[bezierPath stroke];	*/	
	}
	
	NSAttributedString* strVal = [[NSAttributedString alloc] initWithString:[self stringValue] attributes:attributes];
	[self setAttributedStringValue:strVal];
	[strVal release];
    
	[super drawWithFrame:titleFrame inView:controlView];
}

- (NSUInteger)hitTestForEvent:(NSEvent *)event inRect:(NSRect)cellFrame ofView:(NSView *)controlView {
    NSPoint point = [controlView convertPoint:[event locationInWindow] fromView:nil];
    // If we have an image, we need to see if the user clicked on the image portion.
    if (image != nil) {
        // This code closely mimics drawWithFrame:inView:
        NSRect imageFrame = [self imageRectForBounds:cellFrame];
		
		// If the point is in the image rect, then it is a content hit
        if (NSMouseInRect(point, imageFrame, [controlView isFlipped])) {
            // We consider this just a content area. It is not trackable, nor it it editable text. If it was, we would or in the additional items.
            // By returning the correct parts, we allow NSTableView to correctly begin an edit when the text portion is clicked on.
            return NSCellHitNone;
		}
    }
	
	if(NSMouseInRect(point, cellFrame, [controlView isFlipped])){
		return NSCellHitEditableTextArea;
	}
	
    // At this point, the cellFrame has been modified to exclude the portion for the image. Let the superclass handle the hit testing at this point.
    return [super hitTestForEvent:event inRect:cellFrame ofView:controlView];    
}

@end
