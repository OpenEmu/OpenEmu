//
//  OEHUDGameWindow.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 05.09.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEHUDGameWindow.h"
#import "OEGameViewController.h"
@implementation OEHUDGameWindow
@synthesize gameViewController=_gameViewController;
- (id)initWithContentRect:(NSRect)contentRect andGameViewController:(OEGameViewController*)gameViewCtrl
{
    
    self = [super initWithContentRect:contentRect styleMask: (NSHUDWindowMask | NSNonactivatingPanelMask | NSTitledWindowMask | NSResizableWindowMask | NSUtilityWindowMask | NSClosableWindowMask)  backing:NSBackingStoreBuffered defer:NO];
    
    //self = [super initWithContentRect:contentRect];
    if (self) 
    {
		[self setReleasedWhenClosed:YES];
		[self setGameViewController:gameViewCtrl];
        [self setFloatingPanel:NO];
        [self setMovableByWindowBackground:NO];
        [self setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    }
    
    return self;
}

- (void)dealloc{
	if([self gameViewController]){
		[[self gameViewController] terminateEmulation];
	}
	[self setGameViewController:nil];
	
	[super dealloc];
}

- (void)setGameViewController:(OEGameViewController *)gameViewController
{
    if(gameViewController && [gameViewController rom]){
		id rom = [gameViewController rom];
		NSString* path = [rom valueForKey:@"path"];
		self.title = [[path lastPathComponent] stringByDeletingPathExtension];
	}
	
	if([[[self contentView] subviews] count]){
		[[[[self contentView] subviews] lastObject] removeFromSuperview]; 
	}
	
	if(gameViewController){
		[gameViewController terminateEmulation];
	}
	
	[gameViewController retain];
	[_gameViewController release];
	_gameViewController = gameViewController;
	
	if(gameViewController){
		NSView* gameView = (NSView*)[gameViewController view];
		NSRect frame = [[self contentView] frame];
		frame.origin = NSMakePoint(0, 0);
		[gameView setFrame:frame];
		[[self contentView] addSubview:gameView];
	}
}

@end
