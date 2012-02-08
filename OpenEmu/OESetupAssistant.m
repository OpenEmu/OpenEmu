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

#import "OESystemPlugin.h"

#import "OELibraryController.h"
#import "OELibraryDatabase.h"

#import "OEGlossButton.h"
#import "OESetupAssistantKeyMapView.h"

#import "OEApplicationDelegate.h"
#import "NSApplication+OEHIDAdditions.h"

#import "OEROMImporter.h"
#import "OECoreDownload.h"
#import "OEHIDDeviceHandler.h"

@implementation OESetupAssistant

@synthesize transition;
@synthesize replaceView;
@synthesize step1;
@synthesize step2;
@synthesize step3;
@synthesize step3a;
@synthesize step4;
@synthesize step5;
@synthesize step6;
@synthesize step7;
@synthesize step8;
@synthesize step9;
@synthesize step10;
@synthesize lastStep;

// decision tree
@synthesize allowScanForGames;

// Tables
@synthesize installCoreTableView;
@synthesize mountedVolumes;
@synthesize gamePadTableView;


// Buttons
@synthesize gamePadSelectionNextButton;
@synthesize gamePadUpNextButton;
@synthesize gamePadDownNextButton;
@synthesize gamePadLeftNextButton;
@synthesize gamePadRightNextButton;
@synthesize gamePadRunNextButton;
@synthesize gamePadJumpNextButton;

@synthesize selectedGamePadDeviceNum;
@synthesize gotNewEvent;

// key map views
@synthesize upKeyMapView;
@synthesize downKeyMapView;
@synthesize leftKeyMapView;
@synthesize rightKeyMapView;
@synthesize runKeyMapView;
@synthesize jumpKeyMapView;

@synthesize currentKeyMapView;
@synthesize currentNextButton;
@synthesize currentEventToArchive;

// Special buttons
@synthesize goButton;

/*
@synthesize resultTableView;
@synthesize resultProgress;
@synthesize resultFinishedLabel;
@synthesize resultController;
@synthesize dontSearchCommonTypes;
*/

- (id)init
{
    NSLog(@"Init Assistant");
    
    self = [self initWithNibName:[self nibName] bundle:[NSBundle mainBundle]];
    if (self)
    {
        // TODO: need to fail gracefully if we have no internet connection.
        [[OECoreUpdater sharedUpdater] checkForUpdates];
        
        self.selectedGamePadDeviceNum = 0;
        self.gotNewEvent = 0;

        // set default prefs
        [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithBool:YES] forKey:@"organizeLibrary"];
        [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithBool:YES] forKey:@"copyToLibrary"];
        [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithBool:YES] forKey:@"automaticallyGetInfo"];
                
        [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self selector:@selector(reload) name:NSWorkspaceDidMountNotification object:nil];
        [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self selector:@selector(reload) name:NSWorkspaceDidUnmountNotification object:nil];
    }
    
    return self;
}

- (void)dealloc 
{
    NSLog(@"Dealloc Assistant");
    
    [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];

    [super dealloc];
}

- (NSString*)nibName
{
    return @"OESetupAssistant";
}

- (BOOL) acceptsFirstResponder
{
    return YES;
}

- (void) awakeFromNib
{    
    self.currentKeyMapView = self.upKeyMapView;

    // setup table views;
    [self.installCoreTableView setDelegate:self];
    [self.installCoreTableView setDataSource:self];
        
    [self.gamePadTableView setDelegate:self];
    [self.gamePadTableView setDataSource:self];

    [self.mountedVolumes setDelegate:self];
    [self.mountedVolumes setDataSource:self];

    // setup buttons
    [self.gamePadSelectionNextButton setEnabled:NO];
    [self.gamePadUpNextButton setEnabled:NO];
    [self.gamePadDownNextButton setEnabled:NO];
    [self.gamePadLeftNextButton setEnabled:NO];
    [self.gamePadRightNextButton setEnabled:NO];
    [self.gamePadRunNextButton setEnabled:NO];
    [self.gamePadJumpNextButton setEnabled:NO];
    
    [(OEGlossButton*)[self goButton] setButtonColor:OEGlossButtonColorGreen];

    [self.replaceView setWantsLayer:YES];
    
    [self resetKeyViews];
    
    // setup default transition proerties
    self.transition = [CATransition animation];
    self.transition.type = kCATransitionFade;
    self.transition.subtype = kCATransitionFromRight;
    self.transition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault];
    self.transition.duration = 1.0;
    [self.replaceView setAnimations: [NSDictionary dictionaryWithObject:self.transition forKey:@"subviews"]];
    
    // Time bringing in our first view to conincide with our animation
    [self performSelector:@selector(toStep1:) withObject:nil afterDelay:10.0];
}

#pragma mark -

- (void) resetKeyViews;
{
    // set up key map views
    self.upKeyMapView.key = OESetupAssistantKeyUp;
    self.downKeyMapView.key = OESetupAssistantKeyDown;
    self.leftKeyMapView.key = OESetupAssistantKeyLeft;
    self.rightKeyMapView.key = OESetupAssistantKeyRight;
    self.runKeyMapView.key = OESetupAssistantKeyQuestionMark;
    self.jumpKeyMapView.key = OESetupAssistantKeyQuestionMark;
}

- (IBAction)toStep1:(id)sender
{     
    self.transition.type = kCATransitionFade;

    [step1 setFrame:[self.replaceView frame]];
  
    [[self.replaceView animator] addSubview:step1];
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

- (IBAction) toStep3aOr4:(id)sender
{
    if([self.allowScanForGames state] == NSOnState)
        [self goForwardToView:self.step3a];
    else
        [self goForwardToView:self.step4];
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
    // monitor Gamepad inputs from now on, since we selected a game pad.
    [[[self view] window] makeFirstResponder:self];
    self.currentKeyMapView = self.upKeyMapView;
    self.currentNextButton = self.gamePadUpNextButton;
    [self goForwardToView:self.step5];
}

- (IBAction) backToStep5:(id)sender
{
    self.currentKeyMapView = self.upKeyMapView;
    self.currentNextButton = self.gamePadUpNextButton;
    [self goBackToView:self.step5];
}

- (IBAction) toStep6:(id)sender
{
    [self archiveEventForKey:@"userDefaultUp"];

    self.currentKeyMapView = self.downKeyMapView;
    self.currentNextButton = self.gamePadDownNextButton;
    [self goForwardToView:self.step6];
}

- (IBAction) backToStep6:(id)sender
{
    self.currentKeyMapView = self.downKeyMapView;
    self.currentNextButton = self.gamePadDownNextButton;
    [self goBackToView:self.step6];
}

- (IBAction) toStep7:(id)sender
{    
    [self archiveEventForKey:@"userDefaultDown"];
    self.currentKeyMapView = self.leftKeyMapView;
    self.currentNextButton = self.gamePadLeftNextButton;
    [self goForwardToView:self.step7];
}

- (IBAction) backToStep7:(id)sender
{
    self.currentKeyMapView = self.leftKeyMapView;
    self.currentNextButton = self.gamePadLeftNextButton;
    [self goBackToView:self.step7];
}

- (IBAction) toStep8:(id)sender
{
    [self archiveEventForKey:@"userDefaultLeft"];
   self.currentKeyMapView = self.rightKeyMapView;
    self.currentNextButton = self.gamePadRightNextButton;
    [self goForwardToView:self.step8];
}

- (IBAction) backToStep8:(id)sender
{
    self.currentKeyMapView = self.rightKeyMapView;
    self.currentNextButton = self.gamePadRightNextButton;
   [self goBackToView:self.step8];
}

- (IBAction) toStep9:(id)sender
{
    [self archiveEventForKey:@"userDefaultRight"];
   self.currentKeyMapView = self.runKeyMapView;
    self.currentNextButton = self.gamePadRunNextButton;
    [self goForwardToView:self.step9];
}

- (IBAction) backToStep9:(id)sender
{
    self.currentKeyMapView = self.runKeyMapView;
    self.currentNextButton = self.gamePadRunNextButton;
    [self goBackToView:self.step9];
}

- (IBAction) toStep10:(id)sender
{
    [self archiveEventForKey:@"userDefaultPrimary"];

    self.currentKeyMapView = self.jumpKeyMapView;
    self.currentNextButton = self.gamePadJumpNextButton;
    [self goForwardToView:self.step10];
}

- (IBAction) backToStep10:(id)sender
{
    self.currentKeyMapView = self.jumpKeyMapView;
    self.currentNextButton = self.gamePadJumpNextButton;
    [self goBackToView:self.step10];
}

- (IBAction) toLastStep:(id)sender
{
    [self archiveEventForKey:@"userDefaultSecondary"];

    [self goForwardToView:self.lastStep];
}

- (IBAction) finishAndRevealLibrary:(id)sender
{    
    OELibraryController *controller = (OELibraryController*)[[[NSApp delegate] mainWindowController] defaultContentController];
    
    if([self.allowScanForGames state] == NSOnState)
    {
        [controller discoverRoms];   
    }

    // mark setup done.
    [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithBool:YES] forKey:UDSetupAssistantHasRun];
    
    // clean up
    [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];

    // switch up main content.
    [[[NSApp delegate] mainWindowController] setCurrentContentController:[[[NSApp delegate] mainWindowController] defaultContentController]];
}

#pragma mark -
#pragma mark View Switching Helpers

- (void) goBackToView:(NSView*)view
{
    self.gotNewEvent = NO;
    [self resetKeyViews];

    [view setFrame:[self.replaceView frame]];

    self.transition.type = kCATransitionPush;
    self.transition.subtype = kCATransitionFromLeft;
    
    NSView *subview  = [[replaceView subviews] objectAtIndex:0];
    
    [[self.replaceView animator] replaceSubview:subview with:view];
}

- (void) goForwardToView:(NSView*)view
{
    self.gotNewEvent = NO;
    
    [view setFrame:[self.replaceView frame]];

    self.transition.type = kCATransitionPush;
    self.transition.subtype = kCATransitionFromRight;
    
    NSView *subview  = [[self.replaceView subviews] objectAtIndex:0];
    
    [[self.replaceView animator] replaceSubview:subview with:view];
}

- (void) dissolveToView:(NSView*)view
{
    self.gotNewEvent = NO;

    [view setFrame:[self.replaceView frame]];

    self.transition.type = kCATransitionFade;
    
    NSView *subview  = [[replaceView subviews] objectAtIndex:0];
    
    [[self.replaceView animator] replaceSubview:subview with:view];
}

#pragma mark -
#pragma mark Table View Data Protocol

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
    if(aTableView == self.installCoreTableView)
    {
        return [[[OECoreUpdater sharedUpdater] coreList] count];
    }
    
    if(aTableView == self.mountedVolumes)
    {
        NSArray* keys = [NSArray arrayWithObject:NSURLLocalizedNameKey];
        return [[[NSFileManager defaultManager] mountedVolumeURLsIncludingResourceValuesForKeys:keys options:NSVolumeEnumerationSkipHiddenVolumes] count];  
    }
    
    if(aTableView == self.gamePadTableView)
    {
       return [[[(OEApplicationDelegate*)[[NSApplication sharedApplication] delegate] hidManager] deviceHandlers] count];
    }
    
    return 0;
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    if(aTableView == self.installCoreTableView)
    {
        NSString *identifier = [aTableColumn identifier];
        if([identifier isEqualTo:@"enabled"])
        {
            return [NSNumber numberWithBool:YES];
        }
        else if([identifier isEqualToString:@"emulatorName"])
        {
            return[(OECoreDownload*)[[[OECoreUpdater sharedUpdater] coreList] objectAtIndex:rowIndex] name];
        }
        else if([identifier isEqualToString:@"emulatorSystem"])
        {            
            return [(OECoreDownload*)[[[OECoreUpdater sharedUpdater] coreList] objectAtIndex:rowIndex] description];
        }
    }
    
    if(aTableView == self.mountedVolumes)
    {
        NSString *identifier = [aTableColumn identifier];
        if([identifier isEqualTo:@"enabled"])
        {
            return [NSNumber numberWithBool:YES];
        }
        else if([identifier isEqualToString:@"mountName"])
        {
            NSArray* keys = [NSArray arrayWithObject:NSURLLocalizedNameKey];
            NSURL* mountUrl = [[[NSFileManager defaultManager] mountedVolumeURLsIncludingResourceValuesForKeys:keys options:NSVolumeEnumerationSkipHiddenVolumes] objectAtIndex:rowIndex];
            
            NSString* volumeName = @"";
            if([mountUrl getResourceValue:&volumeName forKey:NSURLVolumeLocalizedNameKey error:nil])
                return volumeName;
            else 
                return @"Unnamed Volume";
        }
    }
    
    if(aTableView == self.gamePadTableView)
    {
        OEHIDDeviceHandler* handler = [[[(OEApplicationDelegate*)[[NSApplication sharedApplication] delegate] hidManager] deviceHandlers] objectAtIndex:rowIndex];

        NSString *identifier = [aTableColumn identifier];
        if([identifier isEqualTo:@"usbPort"])
        {
            return [NSString stringWithFormat:@"Device %i", handler.deviceNumber, nil];
        }
        if([identifier isEqualTo:@"gamePadName"])
        {
            return handler.product;
        }
    }

    return nil;
}

#pragma mark -
#pragma mark Table View Delegate Protocol

- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
    if([aNotification object] == self.gamePadTableView)
    {
        [self.gamePadSelectionNextButton setEnabled:(BOOL)[self.gamePadTableView numberOfSelectedRows]];
        
        self.selectedGamePadDeviceNum = [self.gamePadTableView selectedRow] + 1;
    }    
}

- (void) reload
{
    [self.mountedVolumes reloadData];
}

#pragma mark -
#pragma mark HID event handling

- (void) gotEvent:(OEHIDEvent*)event
{
    self.currentEventToArchive = event;
    self.gotNewEvent = YES;
    self.currentKeyMapView.key = OESetupAssistantKeySucess;
    self.currentNextButton.enabled = YES;
}

- (void)axisMoved:(OEHIDEvent *)anEvent
{
    if(self.selectedGamePadDeviceNum == [anEvent padNumber])
        [self gotEvent:anEvent];
}

- (void)buttonDown:(OEHIDEvent *)anEvent
{
    if(self.selectedGamePadDeviceNum == [anEvent padNumber])
        [self gotEvent:anEvent];
}

- (void)buttonUp:(OEHIDEvent *)anEvent
{
    if(self.selectedGamePadDeviceNum == [anEvent padNumber])
        [self gotEvent:anEvent];
}

- (void)hatSwitchChanged:(OEHIDEvent *)anEvent
{
    if(self.selectedGamePadDeviceNum == [anEvent padNumber])
        [self gotEvent:anEvent];
}

- (void)HIDKeyDown:(OEHIDEvent *)anEvent
{
    if(self.selectedGamePadDeviceNum == [anEvent padNumber])
        [self gotEvent:anEvent];
}

- (void)HIDKeyUp:(OEHIDEvent *)anEvent
{
    if(self.selectedGamePadDeviceNum == [anEvent padNumber])
        [self gotEvent:anEvent];
}

#pragma mark -
#pragma mark Preference Saving

- (void) archiveEventForKey:(NSString*)key
{
    [[NSUserDefaults standardUserDefaults] setValue:[NSKeyedArchiver archivedDataWithRootObject:self.currentEventToArchive] forKey:key];
}

@end
