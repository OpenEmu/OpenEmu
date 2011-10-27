/*
 Copyright (c) 2009, OpenEmu Team
 
 
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

#import "OESetupAssistant.h"

#import "OECorePlugin.h"
#import "OEMainWindowController.h"
#import "OEGlossButton.h"
#import "OESetupAssistantKeyMapView.h"
@implementation OESetupAssistant

@synthesize searchResults;
@synthesize transition;
@synthesize replaceView;
@synthesize step1;
@synthesize step2;
@synthesize step3;
@synthesize step4;
@synthesize step5;
@synthesize resultTableView;
@synthesize resultProgress;
@synthesize resultFinishedLabel;
@synthesize resultController;
@synthesize dontSearchCommonTypes;

- (id)init
{
    NSLog(@"Init Assistant");
    
    self = [self initWithNibName:[self nibName] bundle:[NSBundle mainBundle]];
    if (self)
    {
    }
    
    return self;
}

- (void)dealloc {
    NSLog(@"Dealloc Assistant");
    
    [super dealloc];
}

- (NSString*)nibName{
    return @"OESetupAssistant";
}

- (void) awakeFromNib
{    
    [(OEGlossButton*)[self back] setButtonColor:OEGlossButtonColorBlue];
    [(OEGlossButton*)[self next] setButtonColor:OEGlossButtonColorGreen];
    
    [[self keyMapView] setKey:OESetupAssistantKeyQuestionMark];
}

- (void)contentWillShow
{
    NSWindow* window = [[self windowController] window];
    [window setMovableByWindowBackground:YES];
    [[self windowController] setAllowWindowResizing:NO];
}

- (void)contentWillHide
{
    NSWindow* window = [[self windowController] window];
    [window setMovableByWindowBackground:NO];
    [[self windowController] setAllowWindowResizing:YES];
}

@synthesize back, skip, next;
@synthesize keyMapView;
#pragma mark -
- (IBAction)toStep1:(id)sender
{
    [[back superview] removeFromSuperview];
    
    self.searchResults = [[[NSMutableArray alloc] initWithCapacity:1] autorelease];
    [self.resultFinishedLabel setHidden:YES];
    
    self.transition = [CATransition animation];
    self.transition.type = kCATransitionFade;
    self.transition.subtype = kCATransitionFromRight;
    self.transition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault];
    self.transition.duration = 1.0;
    
    CGColorRef colorRef = CGColorCreateGenericGray(0.129, 1.0);
    [[[self.replaceView superview] layer] setBackgroundColor:colorRef];
    
    [[self.replaceView layer] setBackgroundColor:colorRef];
    CGColorRelease(colorRef);
    
    [self.replaceView setWantsLayer:YES];
    
    [self.replaceView setAnimations: [NSDictionary dictionaryWithObject:self.transition forKey:@"subviews"]];
    
    [[self.replaceView animator] addSubview:step1];
    
    [self.resultController setContent:self.searchResults];
}
- (IBAction) backToStep1:(id)sender
{
    [self goBackToView:self.step1];
}

- (IBAction) toStep2:(id)sender
{
    [self goForwardToView:self.step2];
}

- (IBAction) backToStep2:(id)sender
{
    [self goBackToView:self.step2];
}

- (IBAction) toStep3:(id)sender
{
    [self goForwardToView:self.step3];
}

- (IBAction) backToStep3:(id)sender
{
    [self goBackToView:self.step3];
}

- (IBAction) toStep4:(id)sender;
{
    [self goForwardToView:self.step4];
}

- (IBAction) backToStep4:(id)sender
{
    [self goBackToView:self.step4];
}

- (IBAction) toStep5:(id)sender;
{
    [self goForwardToView:self.step5];
}

- (void) goBackToView:(NSView*)view
{
    self.transition.type = kCATransitionPush;
    self.transition.subtype = kCATransitionFromLeft;
    
    NSView* subview  = [[replaceView subviews] objectAtIndex:0];
    
    [[self.replaceView animator] replaceSubview:subview with:view];
}

- (void) goForwardToView:(NSView*)view
{
    self.transition.type = kCATransitionPush;
    self.transition.subtype = kCATransitionFromRight;
    
    NSView* subview  = [[self.replaceView subviews] objectAtIndex:0];
    
    [[self.replaceView animator] replaceSubview:subview with:view];
}

- (void) dissolveToView:(NSView*)view
{
    self.transition.type = kCATransitionFade;
    
    NSView* subview  = [[replaceView subviews] objectAtIndex:0];
    
    [[self.replaceView animator] replaceSubview:subview with:view];
}

#pragma mark -
#pragma Import Rom Discovery

- (IBAction)discoverRoms:(id)sender
{
#warning OESetupAssistant actual importing is deactivated here 
    [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithBool:YES] forKey:UDSetupAssistantHasRun];
    [[self windowController] setCurrentContentController:nil];
    return;
    
    NSMutableArray* supportedFileExtensions = [[OECorePlugin supportedTypeExtensions] mutableCopy];
    
    if([dontSearchCommonTypes state] == NSOnState)
    {
        NSArray* commonTypes = [NSArray arrayWithObjects:@"bin", @"zip", @"elf", nil];
        
        [supportedFileExtensions removeObjectsInArray:commonTypes];
    }
    
    NSLog(@"Supported search Extensions are: %@", supportedFileExtensions);
    
    NSString* searchString = @"";
    for(NSString* extension in supportedFileExtensions)
    {
        searchString = [searchString stringByAppendingFormat:@"(kMDItemDisplayName == *.%@)", extension, nil];
        searchString = [searchString stringByAppendingString:@" || "];
    }
    
    [supportedFileExtensions release];
    
    searchString = [searchString substringWithRange:NSMakeRange(0, [searchString length] - 4)];
    
    NSLog(@"SearchString: %@", searchString);
    
    MDQueryRef searchQuery = MDQueryCreate(kCFAllocatorDefault, (CFStringRef)searchString, NULL, NULL);
    
    if(searchQuery)
    {
        NSLog(@"Valid search query ref");
        
        [self.searchResults removeAllObjects];
        [self.resultProgress startAnimation:nil];
        [self.resultProgress setHidden:NO];
        [self.resultFinishedLabel setHidden:YES];    
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(finalizeSearchResults:)
                                                     name:(NSString*)kMDQueryDidFinishNotification
                                                   object:(id)searchQuery];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(updateSearchResults:)
                                                     name:(NSString*)kMDQueryProgressNotification
                                                   object:(id)searchQuery];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(updateSearchResults:)
                                                     name:(NSString*)kMDQueryDidUpdateNotification
                                                   object:(id)searchQuery];
        
        MDQuerySetSearchScope(searchQuery, (CFArrayRef) [NSArray arrayWithObject:(NSString*) kMDQueryScopeComputer], 0);
        
        if(MDQueryExecute(searchQuery, kMDQueryWantsUpdates))
            NSLog(@"Searching for importable roms");
        
        [self.resultProgress startAnimation:nil];
        
        [self dissolveToView:self.step5];
    }
}

- (void) updateSearchResults:(NSNotification*)notification
{    
    MDQueryRef searchQuery = (MDQueryRef)[notification object]; 
    
    MDItemRef resultItem = NULL;
    NSString* resultPath = nil;
    
    // assume the latest result is the last index?
    CFIndex index = 0;
    
    for(index = 0; index < MDQueryGetResultCount(searchQuery); index++)
    {
        resultItem = (MDItemRef)MDQueryGetResultAtIndex(searchQuery, index);
        resultPath = (NSString*)MDItemCopyAttribute(resultItem, kMDItemPath);
        
        NSArray* dontTouchThisDunDunDunDunHammerTime = [NSArray arrayWithObjects:
                                                        @"System",
                                                        @"Library",
                                                        @"Developer",
                                                        @"Volumes",
                                                        @"Applications",
                                                        @"bin",
                                                        @"cores",
                                                        @"dev",
                                                        @"etc",
                                                        @"home",
                                                        @"net",
                                                        @"sbin",
                                                        @"private",
                                                        @"tmp",
                                                        @"usr",
                                                        @"var",
                                                        nil];
        // Nothing in common
        if(![[resultPath pathComponents] firstObjectCommonWithArray:dontTouchThisDunDunDunDunHammerTime])
        {                
            if(![[self.resultController content] containsObject:resultPath])
            {
                NSMutableDictionary* resultDict = [NSMutableDictionary dictionary];
                
                [resultDict setValue:resultPath forKey:@"Path"];
                [resultDict setValue:[[resultPath lastPathComponent] stringByDeletingPathExtension] forKey:@"Name"];
                
                [self.resultController addObject:resultDict];
            }
        }
        
        [resultPath release];
        resultPath = nil;    
    } 
}

- (void) finalizeSearchResults:(NSNotification*)notification
{
    MDQueryRef searchQuery = (MDQueryRef)[notification object]; 
    
    NSLog(@"Finished searching, found: %lu items", MDQueryGetResultCount(searchQuery));
    
    [self.resultFinishedLabel setStringValue:[NSString stringWithFormat:@"Found %i Games", [[self.resultController content] count], nil]]; 
    
    [self.resultProgress stopAnimation:nil];
    [self.resultProgress setHidden:YES];
    [self.resultFinishedLabel setHidden:NO];    
}

- (IBAction) import:(id)sender;
{
    NSMutableArray* URLS = [NSMutableArray array];
    
    for(NSDictionary* searchResult in self.searchResults)
    {
        [URLS addObject:[NSURL fileURLWithPath:[searchResult objectForKey:@"Path"]]];
    }
    
    NSLog(@"OESetupAssistant import: need to used new import method");
    NSWindow* win = [[self view] window];
    OEMainWindowController* controller = (OEMainWindowController*)[win windowController];
    [controller setCurrentContentController:self];
}
@end
