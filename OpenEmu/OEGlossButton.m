//
//  OEGlossButton.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 26.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEGlossButton.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OEGlossButton
@dynamic buttonColor;
- (void)_setup
{
    if(![[self cell] isKindOfClass:[OEGlossButtonCell class]])
    {
        NSString* title = [self title];
        [self setCell:[[[OEGlossButtonCell alloc] init] autorelease]];
        [self setTitle:title];
    }
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
- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];

    [super dealloc];
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
#pragma mark -
- (OEGlossButtonColor)buttonColor
{
    return [[self cell] buttonColor];
}

- (void)setButtonColor:(OEGlossButtonColor)color
{
    [[self cell] setButtonColor:color];
}

@end
@interface OEGlossButtonCell (Private)
- (NSImage*)_imageForButtonColor:(OEGlossButtonColor)color unfocused:(BOOL)unfocused;
@end
@implementation OEGlossButtonCell
@synthesize buttonColor;

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if (self != [OEGlossButtonCell class])
        return;

	NSImage* image = [NSImage imageNamed:@"gloss_button"];
    
    float buttonHeight = 23.0;
    float step =(image.size.height);
    
    [image setName:@"gloss_button_pressed" forSubimageInRect:(NSRect){{0, 5*step/7},{image.size.width,buttonHeight}}];
    
    [image setName:@"gloss_button_default_inactive" forSubimageInRect:(NSRect){{0, 6*step/7},{image.size.width,buttonHeight}}];
    [image setName:@"gloss_button_default_unfocused" forSubimageInRect:(NSRect){{0, 4*step/7},{image.size.width,buttonHeight}}];
    
    [image setName:@"gloss_button_blue_inactive" forSubimageInRect:(NSRect){{0, 3*step/7},{image.size.width,buttonHeight}}];
    [image setName:@"gloss_button_blue_unfocused" forSubimageInRect:(NSRect){{0, 2*step/7},{image.size.width,buttonHeight}}];
    
    [image setName:@"gloss_button_green_inactive" forSubimageInRect:(NSRect){{0, 1*step/7},{image.size.width,buttonHeight}}];
    [image setName:@"gloss_button_green_unfocused" forSubimageInRect:(NSRect){{0, 0*step/7},{image.size.width,buttonHeight}}];
}

- (void)_setup
{
    [self setBordered:YES];
    [self setFocusRingType:NSFocusRingTypeNone];
    [self setBezeled:NO];
    [self setBackgroundColor:nil];
    [self setButtonType:NSMomentaryPushInButton];
    [self setBezelStyle:NSSmallSquareBezelStyle];
    
    [self setButtonColor:OEGlossButtonColorDefault];
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
- (void)dealloc
{
    [super dealloc];
}
#pragma mark -
- (BOOL)isOpaque
{
	return NO;
}
#pragma mark 
#pragma mark Drawing
- (void)drawBezelWithFrame:(NSRect)frame inView:(NSView *)controlView
{
	NSRect cellFrame = frame;
	cellFrame.size.height = 23;
	
	NSImage* img;
    
    if([self isHighlighted])
    {
        img = [NSImage imageNamed:@"gloss_button_pressed"];
    } 
    else
    {
        BOOL unfocused = ![self isEnabled] || ![[controlView window] isMainWindow];
		img = [self _imageForButtonColor:self.buttonColor unfocused:unfocused];
	}
    
	[img drawInRect:cellFrame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:5 rightBorder:5 topBorder:0 bottomBorder:0];
}

- (NSImage*)_imageForButtonColor:(OEGlossButtonColor)color unfocused:(BOOL)unfocused
{
    NSString* imgName = @"gloss_button_default";
    switch (color) 
    {
        case OEGlossButtonColorBlue:
            imgName = @"gloss_button_blue";            
            break;
        case OEGlossButtonColorGreen:
            imgName = @"gloss_button_green";
            break;            
        default:
            break;
    }
    
    if(unfocused)
        return [NSImage imageNamed:[imgName stringByAppendingString:@"_unfocused"]];
    return [NSImage imageNamed:[imgName stringByAppendingString:@"_inactive"]];
}
#pragma mark -
#pragma mark Drawing the title
- (NSRect)drawTitle:(NSAttributedString *)title withFrame:(NSRect)frame inView:(NSView *)controlView
{
	NSRect titleRect = frame;
	
	titleRect.origin.y -= 1;
	
	[title drawInRect:titleRect];
	return titleRect;
}

- (NSAttributedString*)attributedTitle
{
	NSMutableDictionary *attributes = [[[NSMutableDictionary alloc] init] autorelease];
	
	NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:3.0 size:11.0];
	NSShadow* shadow = [[[NSShadow alloc] init] autorelease];
	[shadow setShadowBlurRadius:1.0];
	NSMutableParagraphStyle* ps = [[[NSMutableParagraphStyle alloc] init] autorelease];
	[ps setAlignment:NSCenterTextAlignment];
	
	if([self isHighlighted])
    {
		[attributes setObject:[NSColor colorWithDeviceWhite:0.03 alpha:1.0] forKey:NSForegroundColorAttributeName];	
		[shadow setShadowColor:[NSColor colorWithDeviceWhite:1.0 alpha:0.4]];
		[shadow setShadowOffset:NSMakeSize(0, -1)];
	} 
    else 
    {
		[shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:1.0]];
		[attributes setObject:[NSColor colorWithDeviceWhite:0.91 alpha:1.0] forKey:NSForegroundColorAttributeName];
		[shadow setShadowOffset:NSMakeSize(0, 1)];
	}
	
	[attributes setObject:ps forKey:NSParagraphStyleAttributeName];
	[attributes setObject:font forKey:NSFontAttributeName];
	[attributes setObject:shadow forKey:NSShadowAttributeName];
	
	return [[[NSAttributedString alloc] initWithString:[self title] attributes:attributes] autorelease];
}
@end