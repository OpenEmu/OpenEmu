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
#import "OESidebarOutlineView.h"

typedef NS_OPTIONS(NSInteger, OESidebarCellState)
{
    OESidebarCellStateFocused        = 1 <<  0,
    OESidebarCellStateUnfocused      = 1 <<  1,
    OESidebarCellStateWindowActive   = 1 <<  2,
    OESidebarCellStateWindowInactive = 1 <<  3,
};

@interface OESidebarCell ()
@property (readonly) NSDictionary *groupAttributes;
- (NSDictionary *) itemAttributesForState:(OESidebarCellState)state;
@end

@implementation OESidebarCell
@synthesize isGroup = _isGroup, isEditing=_isEditing, image=_image;

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

    [cell setImage:[self image]];

    return cell;
}
#pragma mark - Theming
- (NSDictionary*)groupAttributes
{
    static NSDictionary *attributes;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        NSFont *font = [NSFont systemFontOfSize:11 weight:NSFontWeightMedium];
        NSColor *color = NSColor.secondaryLabelColor;
        
        attributes = @{
            NSFontAttributeName : font,
            NSForegroundColorAttributeName : color
        };
    });
    
    return attributes;
}

- (NSDictionary*)itemAttributesForState:(OESidebarCellState)state
{
    static NSDictionary *sharedAttributes;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        NSFont *font = [NSFont systemFontOfSize:13];
        
        NSColor *color = NSColor.labelColor;
        
        NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
        paragraphStyle.lineBreakMode = NSLineBreakByTruncatingTail;
        
        sharedAttributes = @{
            NSFontAttributeName : font,
            NSForegroundColorAttributeName: color,
            NSParagraphStyleAttributeName : paragraphStyle
        };
    });
    
    NSColor *color;
    
    if(state & OESidebarCellStateUnfocused || state & OESidebarCellStateWindowInactive)
    {
        return sharedAttributes;
    }
    else
    {
        color = NSColor.selectedControlTextColor;
    }
    
    NSMutableDictionary *attributes = [sharedAttributes mutableCopy];
    [attributes setObject:color forKey:NSForegroundColorAttributeName];
    
    return attributes;
}

#pragma mark - Frames
- (NSRect)imageRectForBounds:(NSRect)cellFrame 
{
    NSRect result = NSZeroRect;

    if(_image != nil)
    {
		NSSize iconSize = [_image size];
		result = NSMakeRect(cellFrame.origin.x, cellFrame.origin.y+ (cellFrame.size.height-iconSize.height)/2, iconSize.width, iconSize.height);
        result.origin.y = ceil(result.origin.y);
    }

    return result;
}

- (NSRect)titleRectForBounds:(NSRect)cellFrame 
{
    NSRect result;
    if(_image != nil)
    {
        CGFloat imageWidth = [_image size].width;
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

#pragma mark - Drawing
- (void)editWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject event:(NSEvent *)theEvent 
{
	NSRect textFrame = [self titleRectForBounds:aRect];
	textFrame.size.width -= 6.0;
    textFrame.size.height += 2;
    textFrame.origin.y -= 2;
    
    [super editWithFrame:textFrame inView:controlView editor:textObj delegate:anObject event: theEvent];
    [self OE_setupFieldEditor:(NSTextView *)textObj];
}

- (void)selectWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength 
{
	NSRect textFrame = [self titleRectForBounds:aRect];
	textFrame.size.width -= 6.0;
	textFrame.size.height += 2;
    textFrame.origin.y -= 2;
	
    [super selectWithFrame:textFrame inView:controlView editor:textObj delegate:anObject start:selStart length:selLength];
    [self OE_setupFieldEditor:(NSTextView *)textObj];
}

- (void)OE_setupFieldEditor:(NSTextView *)fieldEditor
{
    NSMutableDictionary *typingAttrib = [fieldEditor.typingAttributes mutableCopy];
    [typingAttrib addEntriesFromDictionary:[self itemAttributesForState:OESidebarCellStateFocused]];
    
    [fieldEditor.textStorage setAttributes:typingAttrib range:NSMakeRange(0, fieldEditor.string.length)];
    fieldEditor.typingAttributes = typingAttrib;
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView 
{
	NSRect imageFrame = [self imageRectForBounds:cellFrame];
    if([self image] != nil)
    {
        [[self image] drawInRect:imageFrame fromRect:NSZeroRect operation:NSCompositingOperationSourceOver fraction:1.0 respectFlipped:YES hints:nil];
    }
	
	NSRect titleFrame = [self titleRectForBounds:cellFrame];
    NSWindow *win = [controlView window];
	BOOL isSelected = [self isHighlighted];
	BOOL isActive = [win isMainWindow] && [win firstResponder]==controlView;
    OESidebarCellState state = (isActive   ? OESidebarCellStateWindowActive : OESidebarCellStateWindowInactive) |
                               (isSelected ? OESidebarCellStateFocused : OESidebarCellStateUnfocused);
    
    NSDictionary *attributes = nil;
	if([self isGroup])
    {
        attributes = self.groupAttributes;
		
		titleFrame = cellFrame;
		titleFrame.size.height -= 9;
		titleFrame.origin.y += 9;
		titleFrame.origin.x -= 8;
		titleFrame.size.width += 8;
    }
    else
    {
        attributes = [self itemAttributesForState:state];
        
        // Adjust the title frame to fit the system typeface.
        titleFrame.size.height += 3;
        titleFrame.origin.y -= 2;
    }

	NSAttributedString *strVal = [[NSAttributedString alloc] initWithString:[self stringValue] attributes:attributes];
	[self setAttributedStringValue:strVal];

    [super drawWithFrame:titleFrame inView:controlView];
}

- (NSCellHitResult)hitTestForEvent:(NSEvent *)event inRect:(NSRect)cellFrame ofView:(NSView *)controlView
{
    NSPoint point = [controlView convertPoint:[event locationInWindow] fromView:nil];
    // If we have an image, we need to see if the user clicked on the image portion.
    if(_image != nil)
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
