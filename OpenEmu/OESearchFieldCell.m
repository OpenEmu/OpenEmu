//
//  OESearchFieldCell.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 03.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OESearchFieldCell.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OESearchFieldCell
@synthesize isInBackground;
- (id)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) 
    {
        NSColor* activeColor = [NSColor colorWithDeviceWhite:0.89 alpha:1.0];
        NSColor* disabledColor = [NSColor colorWithDeviceWhite:0.33 alpha:1.0];
        NSColor* placeholderColor = [NSColor colorWithDeviceWhite:0.52 alpha:1.0];
        
        [self setAllowsEditingTextAttributes:NO];
        
        
        NSShadow* shadow = [[NSShadow new] autorelease];
        NSParagraphStyle* paraStyle = [[NSParagraphStyle new] autorelease];
        
        [shadow setShadowBlurRadius:1];
        [shadow setShadowOffset:NSMakeSize(1, 1)];
        [shadow setShadowColor:[NSColor colorWithDeviceWhite:0 alpha:0.4]];
        
        placeholder = [[NSDictionary dictionaryWithObjectsAndKeys:
                        placeholderColor ,NSForegroundColorAttributeName,
                        [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:5 size:11.0], NSFontAttributeName,
                        shadow, NSShadowAttributeName,
                        paraStyle, NSParagraphStyleAttributeName,
                        nil] retain];
        
        NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:5 size:11.0];
        inactive = [[NSDictionary dictionaryWithObjectsAndKeys:
                     activeColor ,NSForegroundColorAttributeName,
                     font, NSFontAttributeName,
                     shadow, NSShadowAttributeName,
                     paraStyle, NSParagraphStyleAttributeName,
                     nil] retain];
        disabled = [[NSDictionary dictionaryWithObjectsAndKeys:
                     disabledColor ,NSForegroundColorAttributeName,
                     font, NSFontAttributeName,
                     shadow, NSShadowAttributeName,
                     paraStyle, NSParagraphStyleAttributeName,
                     nil] retain];
        
        current = [self isEnabled]?inactive:disabled;
        
        [self setFont:[[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:5 size:11.0]];
        
        if(![NSImage imageNamed:@"search_cancel_active"])
        {
            // Load and split cnacel button
            NSImage* image = [NSImage imageNamed:@"search_cancel"];
            [image setName:@"search_cancel_active" forSubimageInRect:NSMakeRect(0, 0, 20, 20)];
            [image setName:@"search_cancel_pressed" forSubimageInRect:NSMakeRect(20, 0, 20, 20)];
            
            // load and split loupe button
            image = [NSImage imageNamed:@"search_loupe"];
            [image setName:@"search_loupe_disabled" forSubimageInRect:NSMakeRect(0, 0, 13, 14)];
            [image setName:@"search_loupe_inactive" forSubimageInRect:NSMakeRect(13, 0, 13, 14)];
            [image setName:@"search_loupe_active" forSubimageInRect:NSMakeRect(26, 0, 13, 14)];
        }
        
        if(![self placeholderAttributedString] && [self placeholderString]) 
        {
            [self setPlaceholderAttributedString:[[[NSAttributedString alloc] initWithString:[self placeholderString] attributes:placeholder] autorelease]];
        }
        [[self searchButtonCell] setImageDimsWhenDisabled:NO];
        [[self searchButtonCell] setImageScaling:NSScaleNone];
        
        [[self cancelButtonCell] setImage:[NSImage imageNamed:@"search_cancel_active"]];
        [[self cancelButtonCell] setAlternateImage:[NSImage imageNamed:@"search_cancel_pressed"]];
    }
    return self;
}

- (void)dealloc{
    [active release];
    [inactive release];
    [disabled release];
    
    [super dealloc];
}

- (NSDictionary *)_textAttributes
{
    return current;
}

- (void)setEnabled:(BOOL)flag
{
    current = flag?inactive:disabled;
    
    [super setEnabled:flag];
}

- (BOOL)isEnabled
{
    return [super isEnabled] && ![self isInBackground];
}

- (NSText *)setUpFieldEditorAttributes:(NSText *)textObj 
{
    textObj = [super setUpFieldEditorAttributes:textObj];
    if([textObj isKindOfClass:[NSTextView class]]) 
    {
        NSShadow* shadow = [[NSShadow new] autorelease];
        NSParagraphStyle* paraStyle = [[NSParagraphStyle new] autorelease];
        
        [shadow setShadowBlurRadius:1];
        [shadow setShadowOffset:NSMakeSize(1, 1)];
        [shadow setShadowColor:[NSColor colorWithDeviceWhite:0 alpha:0.4]];
        
        NSDictionary* selectionDict = [NSDictionary dictionaryWithObjectsAndKeys:
                                       [NSColor blackColor], NSForegroundColorAttributeName,
                                       [NSColor colorWithDeviceWhite:0.54 alpha:1.0], NSBackgroundColorAttributeName,
                                       
                                       [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:5 size:11.0], NSFontAttributeName,
                                       shadow, NSShadowAttributeName,
                                       paraStyle, NSParagraphStyleAttributeName,
                                       
                                       nil];
        
        
        [(NSTextView *)textObj setSelectedTextAttributes:selectionDict];
        [(NSTextView *)textObj setTypingAttributes:current];
        
        NSColor* color = [NSColor colorWithDeviceWhite:0.72 alpha:1.0];
        [(NSTextView *)textObj setInsertionPointColor:color];
    }
    return textObj;
}
#pragma mark -
- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView 
{    
    OEUIState state = OEUIStateInactive;
    if([self isEnabled] && [super showsFirstResponder]) 
    {
        BOOL pressed = NO;
        if(pressed)
        {
            state = OEUIStatePressed;
        } 
        else 
        {
            state = OEUIStateActive;
        }
    } 
    else if([self isEnabled])
    {
        state = OEUIStateEnabled;
    }
    else 
    {
        state = OEUIStateInactive;
    }
    
    NSRect imageRect = [self imageRectForState:state];
    NSRect targetRect = cellFrame;
    targetRect.size.height = 21;
    
    NSImage* image = [NSImage imageNamed:@"search_field"];
    [image drawInRect:targetRect fromRect:imageRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:12 rightBorder:12 topBorder:0 bottomBorder:0];
    
    [self setStylesForState:state];
    
    cellFrame.size.height -= 3;
    
    [self drawInteriorWithFrame:cellFrame inView:controlView];
}
#pragma mark -
- (NSRect)searchTextRectForBounds:(NSRect)aRect
{
    aRect.origin.x += 24;
    aRect.size.width = aRect.size.width-24-21;
    aRect.size.height = 14;
    aRect.origin.y += (22-aRect.size.height)/2-1;
    
    return aRect;
}
- (NSRect)searchButtonRectForBounds:(NSRect) aRect
{
    return NSMakeRect(6, 4, 13, 14);
}

- (NSRect)cancelButtonRectForBounds:(NSRect)rect
{
    return NSMakeRect(rect.size.width-21, 0, 20, 20);
}

#pragma mark -
- (NSRect)imageRectForState:(OEUIState)state
{
    NSRect imageRect = NSMakeRect(0, 0, 26, 21);
    switch (state) 
    {
        case OEUIStateInactive:
            imageRect.origin.y = 63;
            break;
        case OEUIStateEnabled:
            imageRect.origin.y = 42;
            break;
        case OEUIStateActive:
            imageRect.origin.y = 21;
            break;
        case OEUIStatePressed:
            imageRect.origin.y = 0;
            break;
        default: return NSZeroRect;
    }
    
    [[self searchButtonCell] setImage:[self searchButtonImageForState:state]];
    [[self searchButtonCell] setAlternateImage:[self alternateSearchButtonImageForState:state]];
    
    return imageRect;
}

- (void)setStylesForState:(OEUIState)state
{
    switch (state) 
    {
        case OEUIStateInactive:
        {
            current = disabled;
            [self setTextColor:[NSColor colorWithDeviceWhite:0.33 alpha:1.0]];
            break;
        }
        case OEUIStateEnabled:
        {
            current = inactive;
            [self setTextColor:[NSColor colorWithDeviceWhite:0.89 alpha:1.0]];
            break;
        }
        case OEUIStateActive:
        {
            current = active;
            break;
        }
        case OEUIStatePressed:
        {
            current = active;
            
            break;
        }
        default: ;
    }
}

- (NSImage*)searchButtonImageForState:(OEUIState)state
{
    switch (state)
    {
        case OEUIStateInactive:
            return [NSImage imageNamed:@"search_loupe_disabled"];
        case OEUIStateEnabled:
            return [NSImage imageNamed:@"search_loupe_inactive"];
        case OEUIStateActive:
            return [NSImage imageNamed:@"search_loupe_active"];
        case OEUIStatePressed:
            return [NSImage imageNamed:@"search_loupe_active"];
        default: break;
    }
    return nil;
}
- (NSImage*)alternateSearchButtonImageForState:(OEUIState)state
{
    switch (state) 
    {
        case OEUIStateInactive:
            return [NSImage imageNamed:@"search_loupe_disabled"];
        case OEUIStateEnabled:
            return [NSImage imageNamed:@"search_loupe_inactive"];
        case OEUIStateActive:
            return [NSImage imageNamed:@"search_loupe_active"];
        case OEUIStatePressed:
            return [NSImage imageNamed:@"search_loupe_active"];
        default: break;
            
    }
    return nil;
}
@end
#pragma mark -
@interface OESearchField (Private)
- (void)_setup;
@end
#pragma mark -
@implementation OESearchField
- (id)initWithCoder:(NSCoder *)coder 
{
    self = [super initWithCoder:coder];
    if (self) {
        [self _setup];
    }
    return self;
}

- (void)awakeFromNib
{
    [self _setup];
}

- (void)_setup
{
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowChanged:) name:NSWindowDidBecomeMainNotification object:[self window]];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowChanged:) name:NSWindowDidResignMainNotification object:[self window]];
    
    [[self cell] setIsInBackground:NO];
}
#pragma mark -
- (void)windowChanged:(id)sender
{
    if([[self window] isMainWindow])
    {
        // restore enabled property
        [[self cell] setIsInBackground:NO];
    }
    else
    {
        // disabled self when closing the window to keep the user from jumping directly into typing mode and type with "inactive" syles
        // actually, jumping directly to typing still works, but this fixes the style issue
        [[self cell] setIsInBackground:YES];
    }
    [self setNeedsDisplay:YES];
}

- (void)dealloc 
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidBecomeMainNotification object:[self window]];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidResignMainNotification object:[self window]];
    
    [super dealloc];
}


- (NSRect)frame
{
    NSRect frame = [super frame];
    
    frame.size.height = 21;
    
    return frame;
}

- (NSRect)bounds
{
    NSRect bounds = [super bounds];
    
    bounds.size.height = 21;
    return bounds;
}

@end