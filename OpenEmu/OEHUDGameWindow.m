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

#import "OEHUDGameWindow.h"
#import "OEGameViewController.h"
@implementation OEHUDGameWindow
@synthesize gameViewController=_gameViewController;
- (id)initWithContentRect:(NSRect)contentRect andGameViewController:(OEGameViewController*)gameViewCtrl
{
    NSLog(@"OEHUDGameWindow init");
    NSUInteger styleMask = (NSHUDWindowMask | NSNonactivatingPanelMask | NSTitledWindowMask | NSResizableWindowMask | NSUtilityWindowMask | NSClosableWindowMask);
    self = [super initWithContentRect:contentRect styleMask:styleMask  backing:NSBackingStoreBuffered defer:NO];
    
    if (self) 
    {
		[self setReleasedWhenClosed:YES];
		[self setGameViewController:gameViewCtrl];
        [self setFloatingPanel:NO];
        [self setReleasedWhenClosed:YES];
        [self setMovableByWindowBackground:NO];
        [self setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    }
    
    return self;
}

- (void)dealloc{
    NSLog(@"OEHUDGameWindow dealloc");
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
	
	if([self gameViewController]){
		[[self gameViewController] terminateEmulation];
	}
	
	[gameViewController retain];
	[_gameViewController release];
	_gameViewController = gameViewController;
	
	if(gameViewController){
		NSView* gameView = (NSView*)[gameViewController view];
		NSRect frame = [[self contentView] bounds];
        [gameView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
		[gameView setFrame:frame];
		[[self contentView] addSubview:gameView];
	}
}

@end
