/*
 Copyright (c) 2011, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "OESidebarCell.h"
#import <AppKit/NSCell.h>
#import "OESidebarFieldEditor.h"
#import "OESidebarOutlineView.h"

@interface NSTextFieldCell (ApplePrivate)

- (NSDictionary *)_textAttributes;

@end

@implementation OESidebarCell
@synthesize isGroup, isEditing, image;

- (id)init 
{
    if((self = [super init]))
    {
        [self setLineBreakMode:NSLineBreakByTruncatingTail];
        [self setSelectable:YES];
		
    }
    return self;
}

- (id)copyWithZone:(NSZone *)zone 
{
    OESidebarCell *cell = (OESidebarCell *)[super copyWithZone:zone];
    cell.image = image;
    return cell;
}

- (NSRect)imageRectForBounds:(NSRect)cellFrame 
{
    NSRect result;
    if(image != nil)
    {
		NSSize iconSize = [image size];
		result = NSMakeRect(cellFrame.origin.x, cellFrame.origin.y+ (cellFrame.size.height-iconSize.height)/2, iconSize.width, iconSize.height);
        result.origin.y = ceil(result.origin.y);
    } 
    else 
    {
        result = NSZeroRect;
    }
    return result;
}

- (NSRect)titleRectForBounds:(NSRect)cellFrame 
{
    NSRect result;
    if(image != nil)
    {
        CGFloat imageWidth = [image size].width;
        result = cellFrame;
        result.origin.x += (6.0 + imageWidth);
        result.size.width -= (6.0 + imageWidth);
		
		result.size.height = 14.0;
		result.origin.y += (cellFrame.size.height-result.size.height)/2;
	} 
    else 
    {
        result = [super titleRectForBounds:cellFrame];
    }
	
    return result;
}

- (void)editWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject event:(NSEvent *)theEvent 
{
	NSRect textFrame = [self titleRectForBounds:NSInsetRect(aRect, 0, 1)];
	textFrame.size.width -= 6.0;
	
	OESidebarFieldEditor *fieldEditor = [OESidebarFieldEditor fieldEditor];
	[fieldEditor setFrame:[textObj frame]];
    
    [super editWithFrame:textFrame inView:controlView editor:fieldEditor delegate:anObject event: theEvent];
}

- (void)selectWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength 
{
	NSRect textFrame = [self titleRectForBounds:aRect];
	textFrame.size.width -= 6.0;
	
	textFrame.size.height += 2;
	textFrame.origin.y -= 1;
	
	OESidebarFieldEditor *fieldEditor = [OESidebarFieldEditor fieldEditor];
	[fieldEditor setFrame:[textObj frame]];
	
    [super selectWithFrame:textFrame inView:controlView editor:fieldEditor delegate:anObject start:selStart length:selLength];
}

- (NSText *)setUpFieldEditorAttributes:(NSText *)textObj
{	
	textObj = [super setUpFieldEditorAttributes:textObj];
	
	NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:9 size:11.0];
	NSColor *textColor = [NSColor blackColor];
	
	NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
	[paragraphStyle setLineBreakMode:NSLineBreakByTruncatingTail];
	NSDictionary *typeAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
                                    textColor, NSForegroundColorAttributeName,
                                    paragraphStyle, NSParagraphStyleAttributeName,
                                    font, NSFontAttributeName,
                                    nil];
	
	NSColor *backgroundColor = [NSColor colorWithDeviceRed:0.788 green:0.871 blue:0.992 alpha:1.0];
	NSDictionary *selectAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
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
	
	NSColor *color = [NSColor colorWithDeviceWhite:0.72 alpha:1.0];
	[(NSTextView *)textObj setInsertionPointColor:color];
    
	self.isEditing = YES;
	
	return textObj;
}

- (NSDictionary *)_textAttributes
{
	NSDictionary *typeAttributes = [super _textAttributes];
	
	if([self isEditing])
    {
        NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:9 size:11.0];
        NSColor *textColor = [NSColor blackColor];
        NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
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
	NSRect imageFrame = [self imageRectForBounds:cellFrame];
    if([self image] != nil)
    {
        if([self drawsBackground])
        {
            [[self backgroundColor] set];
            NSRectFill(imageFrame);
        }
		[[self image] drawInRect:imageFrame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
    }
	
	NSRect titleFrame = [self titleRectForBounds:cellFrame];
	
	NSFont *font;
	NSColor *textColor;
	NSShadow *shadow = [[NSShadow alloc] init];
	NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
	[paragraphStyle setLineBreakMode:NSLineBreakByTruncatingTail];
	
	
	NSWindow *win = [controlView window];
	BOOL isSelected = [self isHighlighted];
	BOOL isActive = [win isMainWindow] && [win firstResponder]==controlView;

    font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:0 size:11.0];

	// set style
	if([self isGroup])
    {
		textColor = [NSColor colorWithDeviceRed:0.682 green:0.678 blue:0.678 alpha:1.0];
		[shadow setShadowColor:[NSColor blackColor]];
		[shadow setShadowBlurRadius:1];
		[shadow setShadowOffset:NSMakeSize(0, 1)];
		
		titleFrame = cellFrame;
		titleFrame.size.height -= 9;
		titleFrame.origin.y += 9;
		titleFrame.origin.x += 2;
		titleFrame.size.width -= 2;
	} 
    else if(isSelected && isActive)
    {
        // selected active
		[shadow setShadowBlurRadius:1];
		[shadow setShadowOffset:NSMakeSize(0, -1)];	
		textColor = [NSColor whiteColor];
		[shadow setShadowColor:[NSColor blackColor]];
	}
    else if(isSelected)
    {
        // selected inactive
		[shadow setShadowBlurRadius:1];
		[shadow setShadowOffset:NSMakeSize(0, -1)];
		[shadow setShadowColor:[NSColor colorWithDeviceRed:0.682 green:0.678 blue:0.678 alpha:1.0]];
		
		textColor = [NSColor colorWithDeviceRed:0.141 green:0.141 blue:0.141 alpha:1.0];
	}
    else
    {
        // not a group, not selected
		textColor = [NSColor colorWithDeviceRed:(225/255.0) green:(224/255.0) blue:(224/255.0) alpha:1.0];
		[shadow setShadowColor:[NSColor blackColor]];
		[shadow setShadowBlurRadius:1];
		[shadow setShadowOffset:NSMakeSize(0, -1)];
	}
	
	NSDictionary *attributes = [NSDictionary dictionaryWithObjectsAndKeys:
								textColor, NSForegroundColorAttributeName,
								paragraphStyle, NSParagraphStyleAttributeName,
								shadow, NSShadowAttributeName,
								font, NSFontAttributeName,
								nil];
    
	NSAttributedString *strVal = [[NSAttributedString alloc] initWithString:[self stringValue] attributes:attributes];
	[self setAttributedStringValue:strVal];
    
    [super drawWithFrame:titleFrame inView:controlView];
}

- (NSUInteger)hitTestForEvent:(NSEvent *)event inRect:(NSRect)cellFrame ofView:(NSView *)controlView 
{
    NSPoint point = [controlView convertPoint:[event locationInWindow] fromView:nil];
    // If we have an image, we need to see if the user clicked on the image portion.
    if(image != nil)
    {
        // This code closely mimics drawWithFrame:inView:
        NSRect imageFrame = [self imageRectForBounds:cellFrame];
		
		// If the point is in the image rect, then it is a content hit
        if(NSMouseInRect(point, imageFrame, [controlView isFlipped]))
        {
            // We consider this just a content area. It is not trackable, nor it it editable text. If it was, we would or in the additional items.
            // By returning the correct parts, we allow NSTableView to correctly begin an edit when the text portion is clicked on.
            return NSCellHitNone;
		}
    }
	
	if(NSMouseInRect(point, cellFrame, [controlView isFlipped]))
		return NSCellHitEditableTextArea;
	
    // At this point, the cellFrame has been modified to exclude the portion for the image. Let the superclass handle the hit testing at this point.
    return [super hitTestForEvent:event inRect:cellFrame ofView:controlView];    
}

@end
