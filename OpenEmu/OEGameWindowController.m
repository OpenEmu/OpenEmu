//
//  OEGameWindowController.m
//  OpenEmu
//
//  Created by Remy Demarest on 12/02/2012.
//  Copyright (c) 2012 NuLayer Inc. All rights reserved.
//

#import "OEGameWindowController.h"
#import "NSViewController+OEAdditions.h"
#import "OEGameViewController.h"

@interface OEGameWindowController () <NSWindowDelegate>
{
    NSRect contentRect;
}

@end

@implementation OEGameWindowController
@synthesize gameViewController;

- (id)initWithGameViewController:(OEGameViewController *)aController contentRect:(NSRect)aRect;
{
    if(aController == nil) return nil;
    
    if((self = [super initWithWindow:nil]))
    {
        gameViewController = aController;
        contentRect        = aRect;
    }
    
    return self;
}

- (id)initWithWindow:(NSWindow *)window
{
    return nil;
}

- (void)close
{
    [gameViewController viewWillDisappear];
    [super close];
    [gameViewController viewDidDisappear];
}

- (void)loadWindow
{
    NSUInteger styleMask = (NSHUDWindowMask | NSNonactivatingPanelMask | NSTitledWindowMask | NSResizableWindowMask | NSUtilityWindowMask | NSClosableWindowMask);
    NSPanel *window = [[NSPanel alloc] initWithContentRect:contentRect styleMask:styleMask  backing:NSBackingStoreBuffered defer:NO];
    [window setReleasedWhenClosed:YES];
    [window setFloatingPanel:NO];
    [window setMovableByWindowBackground:YES];
    [window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [window setDelegate:self];
    
    [self setWindow:window];
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    if([gameViewController rom] != nil)
    {
		id rom = [gameViewController rom];
		NSString* title = [[[[rom URL] pathComponents] lastObject] stringByDeletingPathExtension];
		[[self window] setTitle:title];
	}
    
    [gameViewController viewWillAppear];
    
    NSView *gameView = [gameViewController view];
    [gameView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [gameView setFrame:[[[self window] contentView] bounds]];
    
    [[[self window] contentView] addSubview:gameView];
    
    [gameViewController viewDidAppear];
}

- (IBAction)terminateEmulation:(id)sender;
{
    [self close];
}

@end
