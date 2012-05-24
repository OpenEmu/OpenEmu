/*
 Copyright (c) 2012, OpenEmu Team
 
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


#import "OEDistantViewController.h"
#import "OEDistantView.h"

#import "NSViewController+OEAdditions.h"
@interface OEDistantViewController ()
- (OEDistantView*)view;

@property (strong) NSWindow* distantWindow;
@end

@implementation OEDistantViewController

- (id)init
{
    self = [super init];
    if (self) {
        OEDistantView *view = [[OEDistantView alloc] init];
        [view setPostsFrameChangedNotifications:YES];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(viewBoundsChanged:) name:NSViewFrameDidChangeNotification object:view];
        [view setController:self];
        [self setView:view];
        
        NSWindow *window = [[NSWindow alloc] initWithContentRect:(NSRect){{0,0}, {0,0}}
                                                       styleMask:NSBorderlessWindowMask
                                                         backing:NSWindowBackingLocationDefault defer:NO];
        [window setAlphaValue:0.0];
        
        [self setDistantWindow:window];
    }
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSViewFrameDidChangeNotification object:[self view]];
}
#pragma mark -
- (void)viewBoundsChanged:(NSNotification*)notification
{
    NSRect newBounds  = [[self view] bounds];
    NSWindow *window = [[self view] window];
    NSRect newDistantWindowFrame = [window convertRectToScreen:[[self view] convertRectToBase:newBounds]];
    
    [[self distantWindow] setFrame:newDistantWindowFrame display:YES];
}
#pragma mark -
- (void)viewWillAppear
{
    [super viewWillAppear];
    
    if([[self distantWindow] contentView] != [[self realViewController] view]) [[self distantWindow] setContentView:[[self realViewController] view]];
    
    [[self realViewController] viewWillAppear];
}

- (void)viewDidAppear
{   
    NSWindow *originalWindow = [[self view] window];
    NSWindow *childWindow     = [self distantWindow];
    [originalWindow addChildWindow:childWindow ordered:NSWindowAbove];
    [self viewBoundsChanged:nil];
    [childWindow setAlphaValue:1.0];
    
    [[self realViewController] viewDidAppear];
    [[self realViewController] setNextResponder:[self view]];
    
    [super viewDidAppear];
}

- (void)viewWillDisappear
{
    [super viewWillDisappear];
    
    [[self distantWindow] setAlphaValue:0.0];
    
    [[self realViewController] viewWillDisappear];
    [[[self distantWindow] parentWindow] removeChildWindow:[self distantWindow]];
    [self setNextResponder:nil];
}

- (void)viewDidDisappear
{   
    [[self realViewController] viewDidDisappear];
    [[self distantWindow] setContentView:[[NSView alloc] init]];
    
    [super viewDidDisappear];
}

#pragma mark -
@synthesize realViewController;
- (void)setRealViewController:(NSViewController *)_realViewController
{
    realViewController = _realViewController;
    NSView *realView = [realViewController view];
    [realView setFrame:[[[self distantWindow] contentView] bounds]];
    [realView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];

    [[self distantWindow] setContentView:realView];
}

@synthesize distantWindow;
- (OEDistantView*)view
{
    return (OEDistantView*)[super view];
}
@end
