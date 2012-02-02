//
//  OEControlsKeyButton.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 25.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEControlsKeyButton.h"
#import "NSImage+OEDrawingAdditions.h"
@interface OEControlsKeyButton (Private)
- (void)_setup;
@end
@implementation OEControlsKeyButton

+ (void)initialize
{
    [self exposeBinding:@"title"];
    
    // Make sure not to reinitialize for subclassed objects
    if (self != [OEControlsKeyButton class])
        return;

    NSImage *image = [NSImage imageNamed:@"wood_textfield"];
    
    [image setName:@"wood_textfield_active" forSubimageInRect:NSMakeRect(0, 0, 5, 24)];
    [image setName:@"wood_textfield_inactive" forSubimageInRect:NSMakeRect(0, 24, 5, 24)];
}

- (id)init 
{
    self = [super init];
    if (self) 
    {
        [self _setup];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)coder 
{
    self = [super initWithCoder:coder];
    if (self) 
    {
        [self _setup];
    }
    return self;
}

- (id)initWithFrame:(NSRect)frame 
{
    self = [super initWithFrame:frame];
    if (self) 
    {
        [self _setup];
    }
    return self;
}
- (void)_setup
{
    state = NSOffState;
}
- (void)dealloc 
{
    self.title = nil;
    self.target = nil;
    self.action = NULL;
    
    [super dealloc];
}

- (void)setTitle:(NSString *)_title
{
    NSString *newTitle = [_title copy];
    [title release];
    title = newTitle;
    
    [self setNeedsDisplay:YES];
}
#pragma mark -
#pragma mark NSView Overrides
- (void)drawRect:(NSRect)dirtyRect
{
    // Draw Backgrounds
    NSImage *backgroundImage = self.state==NSOnState? [NSImage imageNamed:@"wood_textfield_active"] : [NSImage imageNamed:@"wood_textfield_inactive"];
    [backgroundImage drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:2 rightBorder:2 topBorder:5 bottomBorder:5];
        
    NSMutableDictionary *attributes = [[NSMutableDictionary alloc] init];
    
    NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:15 size:11.0];
    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowBlurRadius:1.0];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:1.0 alpha:0.25]];
    [shadow setShadowOffset:NSMakeSize(0, -1)];
    
    [attributes setObject:[NSColor colorWithDeviceWhite:0.0 alpha:1.0] forKey:NSForegroundColorAttributeName];
    [attributes setObject:font forKey:NSFontAttributeName];
    [attributes setObject:shadow forKey:NSShadowAttributeName];
    
    [shadow release];
    
    NSPoint p = NSMakePoint([self bounds].origin.x+4, [self bounds].origin.y+4);
    [self.title drawAtPoint:p withAttributes:attributes];
    [attributes release];
}

- (BOOL)isOpaque
{
    return NO;
}
- (BOOL)isFlipped
{
    return YES;
}

- (void)mouseDown:(NSEvent *)theEvent
{
    if(self.state == NSOnState)
    {
        self.state = NSOffState;
    } 
    else 
    {
        self.state = NSOnState;
    }
    
    if(self.target && self.action!=NULL && [self.target respondsToSelector:self.action])
    {
        [self.target performSelector:self.action withObject:self];
    }
}

- (void)setState:(NSCellStateValue)_state
{
    state = _state;
    [self setNeedsDisplay:YES];
}
#pragma mark -
@synthesize highlightPoint;
@synthesize target;
@synthesize action;
@synthesize title;
@synthesize state;
@end