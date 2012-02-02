//
//  OEGlossCheckBox.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 26.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEGlossCheckBox.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OEGlossCheckBox
- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];

    [super dealloc];
}
- (void)setupCell
{
    if(![[self cell] isKindOfClass:[OEGlossCheckBoxCell class]])
    {
        NSString *title = [self title];
        OEGlossCheckBoxCell *cell = [[OEGlossCheckBoxCell alloc] init];
        [self setCell:cell];
        [cell release];
        [self setTitle:title];
    
        [self setBordered:NO];
        [self setButtonType:NSSwitchButton];
        [self setTransparent:NO];
    }
}

#pragma mark -
- (void)viewDidMoveToWindow
{
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(display) name:NSWindowDidResignMainNotification object:[self window]];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(display) name:NSWindowDidBecomeMainNotification object:[self window]];
}

- (void)viewWillMoveToWindow:(NSWindow *)newWindow
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];        
}
@end

@implementation OEGlossCheckBoxCell
- (id)init 
{
    self = [super init];
    if (self) 
    {
        [self setBezeled:NO];
        [self setBordered:NO];
        [self setImagePosition:NSImageLeft];
    }
    return self;
}
- (void)awakeFromNib
{
}

+ (void)initialize
{	
    // Make sure not to reinitialize for subclassed objects
    if (self != [OEGlossCheckBoxCell class])
        return;

	NSImage *image = [NSImage imageNamed:@"gloss_checkbox"];
	
	[image setName:@"gloss_checkbox_off" forSubimageInRect:NSMakeRect(0, 32, 16, 16)];
    [image setName:@"gloss_checkbox_off_unfocused" forSubimageInRect:NSMakeRect(0, 0, 16, 16)];
	[image setName:@"gloss_checkbox_off_pressed" forSubimageInRect:NSMakeRect(16, 32, 16, 16)];
	[image setName:@"gloss_checkbox_on" forSubimageInRect:NSMakeRect(16, 0, 16, 16)];
	[image setName:@"gloss_checkbox_on_unfocused" forSubimageInRect:NSMakeRect(16, 0, 16, 16)];
	[image setName:@"gloss_checkbox_on_pressed" forSubimageInRect:NSMakeRect(16, 16, 16, 16)];
}

- (NSAttributedString *)attributedTitle
{
	NSMutableDictionary *attributes = [[[NSMutableDictionary alloc] init] autorelease];
	
	NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:0.0 size:11.0];
	NSShadow *shadow = [[[NSShadow alloc] init] autorelease];
	[shadow setShadowBlurRadius:1.0];
	[shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:0.4]];
	[shadow setShadowOffset:NSMakeSize(0, -1)];
	
	[attributes setObject:[NSColor colorWithDeviceWhite:0.89 alpha:1.0] forKey:NSForegroundColorAttributeName];
	[attributes setObject:font forKey:NSFontAttributeName];
	[attributes setObject:shadow forKey:NSShadowAttributeName];
	
	return [[[NSAttributedString alloc] initWithString:[self title] attributes:attributes] autorelease];
}

- (NSRect)drawTitle:(NSAttributedString *)title withFrame:(NSRect)frame inView:(NSView *)controlView
{
	CGContextRef context = [[NSGraphicsContext currentContext] graphicsPort];
	CGContextSaveGState(context);
	CGContextSetShouldSmoothFonts(context, NO);
	
	frame.origin.x += 3;
	frame.size.width -= 3;
	
	frame.origin.y += 2;
	
	NSRect rect = [super drawTitle:title withFrame:frame inView:controlView];
    
	
	CGContextRestoreGState(context);
	
	return rect;
}

- (void)drawImage:(NSImage*)image withFrame:(NSRect)frame inView:(NSView*)controlView
{
    BOOL pressed = [self isHighlighted];
    BOOL on = [self state]==NSOnState;
    BOOL unfocused = ![[[self controlView] window] isMainWindow];
    
    NSString *stateName = on ? @"on" : @"off";
    NSString *behaviourName = unfocused ? @"_unfocused" : pressed ? @"_pressed" : @"";
    
    NSString *imageName = [NSString stringWithFormat:@"gloss_checkbox_%@%@", stateName, behaviourName];
	NSImage *checkboximage = [NSImage imageNamed:imageName];	
	
	CGFloat y = NSMaxY(frame) - (frame.size.height - checkboximage.size.height) / 2.0 - 15;
	[checkboximage drawInRect:NSMakeRect(frame.origin.x, roundf(y), checkboximage.size.width, checkboximage.size.height) fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
}
@end
