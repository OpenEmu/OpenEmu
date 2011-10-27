//
//  OESetupAssistantKeyMapView.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 26.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OESetupAssistantKeyMapView.h"
#import "NSImage+OEDrawingAdditions.h"
@interface OESetupAssistantKeyMapView (Private)
- (NSImage*)imageForKey:(OESetupAssistantKey)key;
- (void)setup;
- (void)_updateKeyView;
@end
@implementation OESetupAssistantKeyMapView
@synthesize key;
+ (void)initialize
{
    NSSize itemSize = NSMakeSize(100, 101);
    NSImage* spritesheet = [NSImage imageNamed:@"installer_gamepad_graphics"];
    [spritesheet setName:@"installer_gamepad_up" forSubimageInRect:   (NSRect){{0*itemSize.width, 1*itemSize.height},itemSize}];
    [spritesheet setName:@"installer_gamepad_down" forSubimageInRect: (NSRect){{1*itemSize.width, 1*itemSize.height},itemSize}];
    [spritesheet setName:@"installer_gamepad_left" forSubimageInRect: (NSRect){{2*itemSize.width, 1*itemSize.height},itemSize}];
    [spritesheet setName:@"installer_gamepad_right" forSubimageInRect:(NSRect){{0*itemSize.width, 0*itemSize.height},itemSize}];
    
    [spritesheet setName:@"installer_gamepad_success"  forSubimageInRect:(NSRect){{2*itemSize.width, 0*itemSize.height},itemSize}];
    [spritesheet setName:@"installer_gamepad_questionMark" forSubimageInRect:(NSRect){{1*itemSize.width, 0*itemSize.height},itemSize}];
}
#pragma mark -
- (id)init
{
    self = [super init];
    if (self) 
    {
        [self setup];
    }
    return self;
}
- (id)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) 
    {
        [self setup];
    }
    return self;
}
- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) 
    {
        [self setup];
    }
    
    return self;
}

- (void)dealloc {
    [self removeObserver:self forKeyPath:@"key"];
    [super dealloc];
}
#pragma mark -
- (void)setup
{
    [self addObserver:self forKeyPath:@"key" options:0 context:nil];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if([keyPath isEqualTo:@"key"])
    {
        [self _updateKeyView];
    }
}

- (void)_updateKeyView
{
    NSImageView* imageView = [[NSImageView alloc] initWithFrame:(NSRect){{0,0},{100,101}}];
    NSImage* image = [self imageForKey:[self key]];
    [imageView setImage:image];
    
    [self addSubview:imageView];
    [imageView release];
}
#pragma mark -
- (BOOL)acceptsFirstResponder
{
    return YES;
}
- (BOOL)canBecomeKeyView
{
    return YES;
}

- (void)keyDown:(NSEvent *)theEvent
{
    [self setNeedsDisplay:YES];
}

- (NSImage*)imageForKey:(OESetupAssistantKey)aKey
{
    switch (aKey) {
        case OESetupAssistantKeyUp:
            return [NSImage imageNamed:@"installer_gamepad_up"];
            break;
        case OESetupAssistantKeyDown:
            return [NSImage imageNamed:@"installer_gamepad_down"];
            break;
        case OESetupAssistantKeyLeft:
            return [NSImage imageNamed:@"installer_gamepad_left"];
            break;
        case OESetupAssistantKeyRight:
            return [NSImage imageNamed:@"installer_gamepad_right"];
            break;
        default:
            return [NSImage imageNamed:@"installer_gamepad_questionMark"];
    }
}
@end
