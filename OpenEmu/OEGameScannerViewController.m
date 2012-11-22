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

#import "OEGameScannerViewController.h"

#import "OELibraryDatabase.h"
#import "OEBackgroundColorView.h"

#import "OEImportItem.h"
#import "OECoreTableButtonCell.h"

#import "OEMenu.h"
#import "OEDBSystem.h"
@interface OEGameScannerViewController ()
@end
@implementation OEGameScannerViewController

- (void)setView:(NSView *)view
{
    [super setView:view];
    [[self importer] setDelegate:self];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(viewFrameChanged:) name:NSViewFrameDidChangeNotification object:view];
    
    // Show game scanner if importer is running already
    if([[self importer] status] == OEImporterStatusRunning)
            [self OE_showView];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSViewBoundsDidChangeNotification object:[self view]];
}
#pragma mark -
- (OEROMImporter*)importer
{
    return [[OELibraryDatabase defaultDatabase] importer];
}
#pragma mark -
- (void)OE_updateProgress
{
    OEROMImporter *importer = [self importer];
    
    NSUInteger maxItems = [[self importer] totalNumberOfItems];
    [[self progressIndicator] setMinValue:0];
    [[self progressIndicator] setDoubleValue:[[self importer] numberOfProcessedItems]];
    [[self progressIndicator] setMaxValue:maxItems];
    
    NSString *status;
    if([importer status] == OEImporterStatusRunning)
    {
        [[self progressIndicator] setIndeterminate:NO];
        [[self progressIndicator] startAnimation:self];
        status = [NSString stringWithFormat:@"Game %ld of %ld.", [[self importer] numberOfProcessedItems], maxItems];
    }
    else if([importer status] == OEImporterStatusStopped || [importer status] == OEImporterStatusStopping)
    {
        [[self progressIndicator] stopAnimation:self];
        [[self progressIndicator] setIndeterminate:YES];
        status = @"Done";
    }
    else
    {
        [[self progressIndicator] stopAnimation:self];
        [[self progressIndicator] setIndeterminate:YES];
        status = @"Scanner Paused";
    }
    
    [[self statusLabel] setStringValue:status];
}

- (void)viewFrameChanged:(NSNotification*)notification
{
    NSRect bounds = [[notification object] bounds];
    CGFloat width = NSWidth(bounds);
    
    NSRect frame = [[self progressIndicator] frame];
    frame.origin.x = 16;
    frame.size.width = width-16-38;
    [[self progressIndicator] setFrame:frame];
    
    frame = [[self headlineLabel] frame];
    frame.origin.x = 17;
    frame.size.width = width-17-12;
    [[self headlineLabel] setFrame:frame];
    
    frame = [[self statusLabel] frame];
    frame.origin.x = 17;
    frame.size.width = width-17-12;
    [[self statusLabel] setFrame:frame];
}
#pragma mark - OELibrarySubviewController Protocol Implementation
- (void)setRepresentedObject:(id)representedObject
{}

- (id)representedObject
{
    return [self importer];
}

- (id)encodeCurrentState
{
    return nil;
}

- (void)restoreState:(id)state
{}

#pragma mark - OEROMImporter Delegate
- (void)romImporterDidStart:(OEROMImporter *)importer
{
    int64_t delayInSeconds = 1.0;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        if([[self importer] totalNumberOfItems] != [[self importer] numberOfProcessedItems])
        {
            [self OE_updateProgress];
            [self OE_showView];
        }
    });
    
}

- (void)romImporterDidCancel:(OEROMImporter *)importer
{
    [self OE_updateProgress];
}

- (void)romImporterDidPause:(OEROMImporter *)importer
{
    [self OE_updateProgress];
}

- (void)romImporterDidFinish:(OEROMImporter *)importer
{
    int64_t delayInSeconds = 0.5;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        if([[self importer] totalNumberOfItems] == [[self importer] numberOfProcessedItems])
            [self OE_hideView];
    });
}

- (void)romImporterChangedItemCount:(OEROMImporter*)importer
{
    [self OE_updateProgress];
}

- (void)romImporter:(OEROMImporter *)importer changedProcessingPhaseOfItem:(OEImportItem*)item
{
}

- (void)romImporter:(OEROMImporter*)importer stoppedProcessingItem:(OEImportItem*)item
{    
    [self OE_updateProgress];
}

#pragma mark - UI Methods
- (IBAction)togglePause:(id)sender
{
    [[self importer] togglePause];
    [self OE_updateProgress];
}

- (IBAction)cancel:(id)sender
{
    [[self importer] cancel];
}

- (void)resolveMultipleSystemsError:(NSMenuItem*)menuItem
{
    // TODO: TableView reload data for item at selected row
    [[self importer] startQueueIfNeeded];
}
#pragma mark - Private
- (void)OE_showView
{
    NSView *scannerView = [self view];
    NSView *sidebarView = [[[scannerView superview] subviews] objectAtIndex:0];
    NSView *superView   = [sidebarView superview];
    
    [NSAnimationContext beginGrouping];
    [[scannerView animator] setFrameOrigin:NSMakePoint(0, 0)];
    [[sidebarView animator] setFrame:(NSRect){{0,NSHeight([scannerView frame])}, {NSWidth([superView bounds]), NSHeight([superView bounds])-NSHeight([scannerView frame])}}];
    [NSAnimationContext endGrouping];
}

- (void)OE_hideView
{
    NSView *scannerView = [self view];
    NSView *sidebarView = [[[scannerView superview] subviews] objectAtIndex:0];
    NSView *superView   = [sidebarView superview];
    
    [NSAnimationContext beginGrouping];
    [[scannerView animator] setFrameOrigin:NSMakePoint(0, -NSHeight([scannerView frame]))];
    [[sidebarView animator] setFrame:[superView bounds]];
    [NSAnimationContext endGrouping];
}
@end
