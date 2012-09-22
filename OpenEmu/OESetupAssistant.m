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
#import "NSViewController+OEAdditions.h"

#import "OESystemPlugin.h"

#import "OELibraryController.h"
#import "OELibraryDatabase.h"

#import "OESetupAssistantKeyMapView.h"

#import "OEApplicationDelegate.h"
#import "NSApplication+OEHIDAdditions.h"

#import "OECoreDownload.h"
#import "OEHIDDeviceHandler.h"

NSString *const OESetupAssistantHasFinishedKey = @"setupAssistantFinished";

@implementation OESetupAssistant
@synthesize completionBlock;
@synthesize deviceHandlers;
@synthesize enabledCoresForDownloading;
@synthesize enabledVolumesForDownloading;
@synthesize allowedVolumes;

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
    DLog(@"Init Assistant");
    
    if((self = [self initWithNibName:@"OESetupAssistant" bundle:[NSBundle mainBundle]]))
    {
        // TODO: need to fail gracefully if we have no internet connection.
        [[OECoreUpdater sharedUpdater] performSelectorInBackground:@selector(checkForNewCores:) withObject:[NSNumber numberWithBool:NO]];
        [[OECoreUpdater sharedUpdater] performSelectorInBackground:@selector(checkForUpdates) withObject:nil];
        
        // set default prefs
        [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithBool:YES] forKey:@"organizeLibrary"];
        [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithBool:YES] forKey:@"copyToLibrary"];
        [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithBool:YES] forKey:@"automaticallyGetInfo"];
        
        [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self selector:@selector(reload) name:NSWorkspaceDidMountNotification object:nil];
        [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self selector:@selector(reload) name:NSWorkspaceDidUnmountNotification object:nil];
        
        self.enabledCoresForDownloading = [NSMutableArray array];
        self.enabledVolumesForDownloading = [NSMutableArray array];
        
        self.deviceHandlers = [[[NSApp delegate] HIDManager] deviceHandlers];
    
        // udpate our data for our volumes
        [self reload];
    }
    
    return self;
}

- (void)dealloc
{
    DLog(@"Dealloc Assistant");
    
    self.enabledCoresForDownloading = nil;
    self.enabledVolumesForDownloading = nil;
    
    [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
    [self setCompletionBlock:nil];
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    [self setCurrentKeyMapView:[self upKeyMapView]];
    
    // setup buttons
    [[self gamePadSelectionNextButton] setEnabled:NO];
    [[self gamePadUpNextButton]        setEnabled:NO];
    [[self gamePadDownNextButton]      setEnabled:NO];
    [[self gamePadLeftNextButton]      setEnabled:NO];
    [[self gamePadRightNextButton]     setEnabled:NO];
    [[self gamePadRunNextButton]       setEnabled:NO];
    [[self gamePadJumpNextButton]      setEnabled:NO];
    
    [[self replaceView] setWantsLayer:YES];
    
    [self resetKeyViews];
    
    // setup default transition proerties
    [self setTransition:[CATransition animation]];
    [[self transition] setType:kCATransitionFade];
    [[self transition] setSubtype:kCATransitionFromRight];
    [[self transition] setTimingFunction:[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault]];
    [[self transition] setDuration:1.0];
    [[self replaceView] setAnimations:[NSDictionary dictionaryWithObject:[self transition] forKey:@"subviews"]];
    
    // Time bringing in our first view to conincide with our animation
    [self performSelector:@selector(toStep1:) withObject:nil afterDelay:10.0];
}

- (void)viewDidAppear
{
}

#pragma mark -

- (void)resetKeyViews;
{
    // set up key map views
    [[self upKeyMapView]    setKey:OESetupAssistantKeyUp];
    [[self downKeyMapView]  setKey:OESetupAssistantKeyDown];
    [[self leftKeyMapView]  setKey:OESetupAssistantKeyLeft];
    [[self rightKeyMapView] setKey:OESetupAssistantKeyRight];
    [[self runKeyMapView]   setKey:OESetupAssistantKeyQuestionMark];
    [[self jumpKeyMapView]  setKey:OESetupAssistantKeyQuestionMark];
}

- (IBAction)toStep1:(id)sender
{
    [[self transition] setType:kCATransitionFade];
    
    [step1 setFrame:[[self replaceView] frame]];
    
    [[[self replaceView] animator] addSubview:step1];
    
    // Hopefully we have the updated core list by now. Lets init an NSMutableArray with 
    for (int i = 0; i < [[[OECoreUpdater sharedUpdater] coreList] count]; i ++ )
    {
        [enabledCoresForDownloading addObject:[NSNumber numberWithBool:YES]];
    }
}

- (IBAction)backToStep1:(id)sender
{
    [self goBackToView:[self step1]];
}

- (IBAction)toStep2:(id)sender
{
    [self goForwardToView:[self step2]];
}

- (IBAction)backToStep2:(id)sender
{
    [self goBackToView:[self step2]];
}

- (IBAction)toStep3:(id)sender
{
    // install our cores    
    for (int i = 0; i < [[[OECoreUpdater sharedUpdater] coreList] count]; i ++ )
    {
        if([[enabledCoresForDownloading objectAtIndex:i] boolValue])
        {
            [self OE_updateOrInstallItemAtRow:i];
        }
    }
    
    [self goForwardToView:[self step3]];
}

- (IBAction)backToStep3:(id)sender
{
    [self goBackToView:[self step3]];
}

- (IBAction)toStep3aOr4:(id)sender
{
    if([self.allowScanForGames state] == NSOnState)
        [self goForwardToView:[self step3a]];
    else
        [self goForwardToView:[self step4]];
}

- (IBAction)toStep4:(id)sender;
{
    // If the user came from step3a, get cache the selected volume URLs for searching
    if(self.allowScanForGames)
    {
        NSArray *keys = [NSArray arrayWithObject:NSURLLocalizedNameKey];
        allowedVolumes = [[[NSFileManager defaultManager] mountedVolumeURLsIncludingResourceValuesForKeys:keys options:NSVolumeEnumerationSkipHiddenVolumes] mutableCopy];
		NSMutableIndexSet *disallowedVolumes = [NSMutableIndexSet indexSet];
		
        for(int i = 0; i < [enabledVolumesForDownloading count]; i++)
        {
            if(![[enabledVolumesForDownloading objectAtIndex:i] boolValue])
				[disallowedVolumes addIndex:i];
        }
		
		[allowedVolumes removeObjectsAtIndexes:disallowedVolumes];
    }
    
    [self goForwardToView:[self step4]];
}

- (IBAction)backToStep4:(id)sender
{
    [self goBackToView:[self step4]];
}

- (IBAction)toStep5:(id)sender;
{
    // monitor Gamepad inputs from now on, since we selected a game pad.
    [[[self view] window] makeFirstResponder:self];
    [self setCurrentKeyMapView:[self upKeyMapView]];
    [self setCurrentNextButton:[self gamePadUpNextButton]];
    [self goForwardToView:[self step5]];
}

- (IBAction)backToStep5:(id)sender
{
    [self setCurrentKeyMapView:[self upKeyMapView]];
    [self setCurrentNextButton:[self gamePadUpNextButton]];
    [self goBackToView:[self step5]];
}

- (IBAction)toStep6:(id)sender
{
    [self archiveEventForKey:@"userDefaultUp"];
    
    [self setCurrentKeyMapView:[self downKeyMapView]];
    [self setCurrentNextButton:[self gamePadDownNextButton]];
    [self goForwardToView:[self step6]];
}

- (IBAction)backToStep6:(id)sender
{
    [self setCurrentKeyMapView:[self downKeyMapView]];
    [self setCurrentNextButton:[self gamePadDownNextButton]];
    [self goBackToView:[self step6]];
}

- (IBAction)toStep7:(id)sender
{    
    [self archiveEventForKey:@"userDefaultDown"];
    [self setCurrentKeyMapView:[self leftKeyMapView]];
    [self setCurrentNextButton:[self gamePadLeftNextButton]];
    [self goForwardToView:[self step7]];
}

- (IBAction)backToStep7:(id)sender
{
    [self setCurrentKeyMapView:[self leftKeyMapView]];
    [self setCurrentNextButton:[self gamePadLeftNextButton]];
    [self goBackToView:[self step7]];
}

- (IBAction)toStep8:(id)sender
{
    [self archiveEventForKey:@"userDefaultLeft"];
    [self setCurrentKeyMapView:[self rightKeyMapView]];
    [self setCurrentNextButton:[self gamePadRightNextButton]];
    [self goForwardToView:[self step8]];
}

- (IBAction)backToStep8:(id)sender
{
    [self setCurrentKeyMapView:[self rightKeyMapView]];
    [self setCurrentNextButton:[self gamePadRightNextButton]];
    [self goBackToView:[self step8]];
}

- (IBAction)toStep9:(id)sender
{
    [self archiveEventForKey:@"userDefaultRight"];
    [self setCurrentKeyMapView:[self runKeyMapView]];
    [self setCurrentNextButton:[self gamePadRunNextButton]];
    [self goForwardToView:[self step9]];
}

- (IBAction)backToStep9:(id)sender
{
    [self setCurrentKeyMapView:[self runKeyMapView]];
    [self setCurrentNextButton:[self gamePadRunNextButton]];
    [self goBackToView:[self step9]];
}

- (IBAction)toStep10:(id)sender
{
    [self archiveEventForKey:@"userDefaultPrimary"];
    
    [self setCurrentKeyMapView:[self jumpKeyMapView]];
    [self setCurrentNextButton:[self gamePadJumpNextButton]];
    [self goForwardToView:[self step10]];
}

- (IBAction)backToStep10:(id)sender
{
    [self setCurrentKeyMapView:[self jumpKeyMapView]];
    [self setCurrentNextButton:[self gamePadJumpNextButton]];
    [self goBackToView:[self step10]];
}

- (IBAction)toLastStep:(id)sender
{
    [self archiveEventForKey:@"userDefaultSecondary"];
    
    [self goForwardToView:[self lastStep]];
}

- (IBAction)finishAndRevealLibrary:(id)sender
{
    // mark setup done.
    [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithBool:YES] forKey:OESetupAssistantHasFinishedKey];
    
    // clean up
    [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
    
    BOOL shouldScan = ([[self allowScanForGames] state] == NSOnState) && ([allowedVolumes count] > 0);
    
    if(completionBlock != nil) 
        completionBlock(shouldScan, allowedVolumes);
}

#pragma mark -
#pragma mark View Switching Helpers

- (void)goBackToView:(NSView *)view
{
    [self setGotNewEvent:NO];
    [self resetKeyViews];
    
    [view setFrame:[[self replaceView] frame]];
    
    self.transition.type = kCATransitionPush;
    self.transition.subtype = kCATransitionFromLeft;
    
    NSView *subview = [[[self replaceView] subviews] objectAtIndex:0];
    
    [[[self replaceView] animator] replaceSubview:subview with:view];
}

- (void)goForwardToView:(NSView *)view
{
    [self setGotNewEvent:NO];
    
    [view setFrame:[[self replaceView] frame]];
    
    [[self transition] setType:kCATransitionPush];
    [[self transition] setSubtype:kCATransitionFromRight];
    
    NSView *subview  = [[[self replaceView] subviews] objectAtIndex:0];
    
    [[[self replaceView] animator] replaceSubview:subview with:view];
}

- (void)dissolveToView:(NSView *)view
{
    [self setGotNewEvent:NO];
    
    [view setFrame:[[self replaceView] frame]];
    
    [[self transition] setType:kCATransitionFade];
    
    NSView *subview  = [[replaceView subviews] objectAtIndex:0];
    
    [[[self replaceView] animator] replaceSubview:subview with:view];
}

#pragma mark -
#pragma mark Table View Data Protocol

// for Core Downloader

- (void)OE_updateOrInstallItemAtRow:(NSInteger)rowIndex
{
    OECoreDownload *plugin = [self OE_coreDownloadAtRow:rowIndex];
    [plugin startDownload:self];
}

- (OECoreDownload*)OE_coreDownloadAtRow:(NSInteger)row
{
    return [[[OECoreUpdater sharedUpdater] coreList] objectAtIndex:row];
}


- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
    if(aTableView == [self installCoreTableView])
    {
        return [[[OECoreUpdater sharedUpdater] coreList] count];
    }
    else if(aTableView == [self mountedVolumes])
    {
        NSArray *keys = [NSArray arrayWithObject:NSURLLocalizedNameKey];
        return [[[NSFileManager defaultManager] mountedVolumeURLsIncludingResourceValuesForKeys:keys options:NSVolumeEnumerationSkipHiddenVolumes] count];
    }
    else if(aTableView == [self gamePadTableView])
    {
        return [[self deviceHandlers] count];
    }
    
    return 0;
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{    
    if(aTableView == [self installCoreTableView])
    {
        NSString *identifier = [aTableColumn identifier];
        
        if([identifier isEqualToString:@"enabled"])
            //return [NSNumber numberWithBool:YES];
            return [enabledCoresForDownloading objectAtIndex:rowIndex];
            
        else if([identifier isEqualToString:@"emulatorName"])
            return [(OECoreDownload *)[[[OECoreUpdater sharedUpdater] coreList] objectAtIndex:rowIndex] name];
        
        else if([identifier isEqualToString:@"emulatorSystem"])
            return [(OECoreDownload *)[[[OECoreUpdater sharedUpdater] coreList] objectAtIndex:rowIndex] description];
    }
    else if(aTableView == [self mountedVolumes])
    {
        NSString *identifier = [aTableColumn identifier];
        
        if([identifier isEqualToString:@"enabled"])
            return [enabledVolumesForDownloading objectAtIndex:rowIndex];
        
        else if([identifier isEqualToString:@"mountName"])
        {
            NSArray *keys = [NSArray arrayWithObject:NSURLLocalizedNameKey];
            NSURL *mountUrl = [[[NSFileManager defaultManager] mountedVolumeURLsIncludingResourceValuesForKeys:keys options:NSVolumeEnumerationSkipHiddenVolumes] objectAtIndex:rowIndex];
            
            NSString *volumeName = @"";
            if([mountUrl getResourceValue:&volumeName forKey:NSURLVolumeLocalizedNameKey error:nil])
                return volumeName;
            else 
                return @"Unnamed Volume";
        }
    }
    else if(aTableView == [self gamePadTableView])
    {
        OEHIDDeviceHandler *handler = [[self deviceHandlers] objectAtIndex:rowIndex];
        
        NSString *identifier = [aTableColumn identifier];
        if([identifier isEqualToString:@"usbPort"])
            return [NSString stringWithFormat:@"Device %li", [handler deviceNumber]];
        else if([identifier isEqualToString:@"gamePadName"])
            return [handler product];
    }
    
    return nil;
}

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    
    if(aTableView == [self installCoreTableView])
    {
        NSString *identifier = [aTableColumn identifier];
        
        if([identifier isEqualToString:@"enabled"])
            [enabledCoresForDownloading replaceObjectAtIndex:rowIndex withObject:anObject];
    }
    
    else if(aTableView == [self mountedVolumes])
    {
        NSString *identifier = [aTableColumn identifier];
        
        if([identifier isEqualToString:@"enabled"])
            [enabledVolumesForDownloading replaceObjectAtIndex:rowIndex withObject:anObject];
    }

}

#pragma mark -
#pragma mark Table View Delegate Protocol

- (BOOL)tableView:(NSTableView *)aTableView shouldEditTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    NSString *identifier = [aTableColumn identifier];
    
    if([identifier isEqualToString:@"enabled"])
        return YES;
    
    return NO;
}

- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
    if([aNotification object] == [self gamePadTableView])
    {
        [[self gamePadSelectionNextButton] setEnabled:[[self gamePadTableView] numberOfSelectedRows] > 0];
        
        [self setSelectedGamePadDeviceNum:[[self gamePadTableView] selectedRow] + 1];
    }    
}

- (void)reload
{
    NSArray *keys = [NSArray arrayWithObject:NSURLLocalizedNameKey];
    NSUInteger volumeCount = [[[NSFileManager defaultManager] mountedVolumeURLsIncludingResourceValuesForKeys:keys options:NSVolumeEnumerationSkipHiddenVolumes] count];
    
    [enabledVolumesForDownloading removeAllObjects];
    
    for(int i = 0; i < volumeCount; i++)
    {
        [enabledVolumesForDownloading addObject:[NSNumber numberWithBool:YES]];
    }
    
    [[self mountedVolumes] reloadData];
}

#pragma mark -
#pragma mark HID event handling

- (void)gotEvent:(OEHIDEvent *)event
{
    [self setCurrentEventToArchive:event];
    [self setGotNewEvent:YES];
    [[self currentKeyMapView] setKey:OESetupAssistantKeySucess];
    [[self currentNextButton] setEnabled:YES];
}

- (void)axisMoved:(OEHIDEvent *)anEvent
{
    if([self selectedGamePadDeviceNum] == [anEvent padNumber])
        [self gotEvent:anEvent];
}

- (void)triggerPull:(OEHIDEvent *)anEvent;
{
    if([self selectedGamePadDeviceNum] == [anEvent padNumber])
        [self gotEvent:anEvent];
}

- (void)triggerRelease:(OEHIDEvent *)anEvent;
{
    if([self selectedGamePadDeviceNum] == [anEvent padNumber])
        [self gotEvent:anEvent];
}

- (void)buttonDown:(OEHIDEvent *)anEvent
{
    if([self selectedGamePadDeviceNum] == [anEvent padNumber])
        [self gotEvent:anEvent];
}

- (void)buttonUp:(OEHIDEvent *)anEvent
{
    if([self selectedGamePadDeviceNum] == [anEvent padNumber])
        [self gotEvent:anEvent];
}

- (void)hatSwitchChanged:(OEHIDEvent *)anEvent
{
    if([self selectedGamePadDeviceNum] == [anEvent padNumber])
        [self gotEvent:anEvent];
}

- (void)HIDKeyDown:(OEHIDEvent *)anEvent
{
    if([self selectedGamePadDeviceNum] == [anEvent padNumber])
        [self gotEvent:anEvent];
}

- (void)HIDKeyUp:(OEHIDEvent *)anEvent
{
    if([self selectedGamePadDeviceNum] == [anEvent padNumber])
        [self gotEvent:anEvent];
}

#pragma mark -
#pragma mark Preference Saving

- (void)archiveEventForKey:(NSString *)key
{
    [[NSUserDefaults standardUserDefaults] setValue:[NSKeyedArchiver archivedDataWithRootObject:[self currentEventToArchive]] forKey:key];
}

@end
