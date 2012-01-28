//
//  OEHUDTextFieldCell.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 22.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "OEHUDTextFieldCell.h"
#import "NSImage+OEDrawingAdditions.h"

#import "OEHUDTextFieldEditor.h"
@interface NSTextFieldCell (ApplePrivate)
- (NSDictionary*)_textAttributes;
@end

@implementation OEHUDTextFieldCell
@synthesize isEditing;
+ (void)initialize
{
    NSImage* image = [NSImage imageNamed:@"hud_textfield"];
    [image setName:@"hud_textfield_unfocused" forSubimageInRect:(NSRect){{0, 2*(image.size.height/3)},{image.size.width, image.size.height/3}}];
    [image setName:@"hud_textfield_inactive" forSubimageInRect:(NSRect){{0, 1*(image.size.height/3)},{image.size.width, image.size.height/3}}];
    [image setName:@"hud_textfield_active" forSubimageInRect:(NSRect){{0, 0*(image.size.height/3)},{image.size.width, image.size.height/3}}];
}

- (id)init
{
    if ((self = [super init]))
    {
        [self setLineBreakMode:NSLineBreakByTruncatingTail];
        [self setSelectable:YES];
    }
    return self;
}

- (void)dealloc
{
    [super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
    OEHUDTextFieldCell *cell = (OEHUDTextFieldCell *)[super copyWithZone:zone];
    return cell;
}

- (NSRect)imageRectForBounds:(NSRect)cellFrame
{
    return NSZeroRect;
}

- (NSRect)titleRectForBounds:(NSRect)cellFrame 
{
    return [super titleRectForBounds:cellFrame];
}

- (void)editWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject event:(NSEvent *)theEvent
{
    if(![textObj isKindOfClass:[OEHUDTextFieldEditor class]])
    {        
        textObj = [OEHUDTextFieldEditor fieldEditor];
        [textObj setFrame:[textObj frame]];
    }
	
    NSRect textFrame = [self titleRectForBounds:NSInsetRect(aRect, 2, 0)];
    [super editWithFrame:textFrame inView:controlView editor:textObj delegate:anObject event: theEvent];
}

- (void)selectWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength
{
    if(![textObj isKindOfClass:[OEHUDTextFieldEditor class]])
    {        
        textObj = [OEHUDTextFieldEditor fieldEditor];
        [textObj setFrame:[textObj frame]];
    }
	
    NSRect textFrame = [self titleRectForBounds:NSInsetRect(aRect, 2, 0)];
    [super selectWithFrame:textFrame inView:controlView editor:textObj delegate:anObject start:selStart length:selLength];
}

- (NSText *)setUpFieldEditorAttributes:(NSText *)textObj
{	
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
	
	if([textObj isKindOfClass:[NSTextView class]])
    {
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

- (NSDictionary *)_textAttributes
{
	NSDictionary* typeAttributes = [super _textAttributes];
	
	if(self.isEditing){
        NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.5];
        NSColor* textColor = [NSColor colorWithDeviceWhite:0.86 alpha:1.0];
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

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView 
{
    if(self.isEditing)
    {
        [[NSImage imageNamed:@"hud_textfield_active"] drawInRect:cellFrame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:2 rightBorder:2 topBorder:2 bottomBorder:2];
        return;
	}
    
	NSRect titleFrame = [self titleRectForBounds:cellFrame];
	
	NSFont* font;
	NSColor* textColor;
	NSShadow* shadow = [[[NSShadow alloc] init] autorelease];
	NSMutableParagraphStyle *paragraphStyle = [[[NSMutableParagraphStyle alloc] init] autorelease];
	[paragraphStyle setLineBreakMode:NSLineBreakByTruncatingTail];
	
	
	NSWindow* win = [controlView window];
	BOOL isSelected = [self isHighlighted];
	BOOL isActive = [win isMainWindow] && [win firstResponder]==controlView;
	
	// set style
    if( (isSelected && isActive))
    { // selected active
		[shadow setShadowBlurRadius:1];
		[shadow setShadowOffset:NSMakeSize(0, -1)];	
		font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:9 size:11.0];
		textColor = [NSColor whiteColor];
		[shadow setShadowColor:[NSColor blackColor]];
		
		// Adjust cell frame
	}
    else if(isSelected)
    {  // selected inactive
		font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:9 size:11.0];
		[shadow setShadowBlurRadius:1];
		[shadow setShadowOffset:NSMakeSize(0, -1)];
		[shadow setShadowColor:[NSColor colorWithDeviceRed:0.682 green:0.678 blue:0.678 alpha:1.0]];
		
		textColor = [NSColor colorWithDeviceRed:0.141 green:0.141 blue:0.141 alpha:1.0];
	}
    else
    {
		font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:7 size:11.0];
		textColor = [NSColor colorWithDeviceRed:0.882 green:0.878 blue:0.878 alpha:1.0];
		[shadow setShadowColor:[NSColor blackColor]];
		[shadow setShadowBlurRadius:1];
		[shadow setShadowOffset:NSMakeSize(0, -1)];
	}
	
	
	
	NSDictionary* attributes = [NSDictionary dictionaryWithObjectsAndKeys:
								textColor, NSForegroundColorAttributeName,
								paragraphStyle, NSParagraphStyleAttributeName,
								shadow, NSShadowAttributeName,
								font, NSFontAttributeName,
								nil];
	
	NSAttributedString* strVal = [[NSAttributedString alloc] initWithString:[self stringValue] attributes:attributes];
	[self setAttributedStringValue:strVal];
	[strVal release];
    
	[super drawWithFrame:titleFrame inView:controlView];
}

- (NSUInteger)hitTestForEvent:(NSEvent *)event inRect:(NSRect)cellFrame ofView:(NSView *)controlView 
{
    NSPoint point = [controlView convertPoint:[event locationInWindow] fromView:nil];
    // If we have an image, we need to see if the user clicked on the image portion.
   
	if(NSMouseInRect(point, cellFrame, [controlView isFlipped]))
    {
		return NSCellHitEditableTextArea;
	}
	
    // At this point, the cellFrame has been modified to exclude the portion for the image. Let the superclass handle the hit testing at this point.
    return [super hitTestForEvent:event inRect:cellFrame ofView:controlView];    
}

- (void)endEditing:(NSText *)textObj{
    
    NSLog(@"end editing %@", [textObj string]);
    [super endEditing:textObj];
}
@end
