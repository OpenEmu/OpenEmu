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

#import "GamePreferencesController.h"
#import "GameDocumentController.h"
#import "GameDocument.h"
#import "PrefsBundleCell.h"
#import "PluginInfo.h"

NSString * const OEGameControls = @"GameControls";
NSString * const OEFullScreen = @"FullScreen";
NSString * const OEFilter = @"Filter";
NSString * const OEGamepadControls = @"GamepadControls";

@implementation ControlRebindView

- (BOOL) acceptsFirstResponder
{
	return TRUE;
}

- (void)keyDown: (NSEvent*) theEvent
{
	//Shut up!
}

- (void)keyUp: (NSEvent*) theEvent
{
	[controller bindKeyFromSheet:[theEvent keyCode]];
}


@end


@implementation GamePreferencesController (Video)
- (BOOL) fullScreen
{
	return [[NSUserDefaults standardUserDefaults] boolForKey:OEFullScreen];
}

- (int) filter
{
	return [[NSUserDefaults standardUserDefaults] boolForKey:OEFilter];
}

- (IBAction) switchFilter:(id) sender
{
	[(GameDocument*)[docController currentDocument] resetFilter];
}

@end


@implementation GamePreferencesController (Controls)

- (void) bindKeyFromSheet: (int) key
{
	[NSApp endSheet: keyPanel returnCode: key];
}

- (void) doubleClickedKeyboard
{
	
	int row = [controlsTableView clickedRow];
	
	[NSApp beginSheet:keyPanel 
	   modalForWindow: [self window]
		modalDelegate: self
	   didEndSelector: @selector(keyInputDidEnd:returnCode:contextInfo:) 
		  contextInfo: (void*)row];
	
	//NSLog(@"Clicked %i", row);
}

- (void) doubleClickedGamepad
{
	
	int row = [gamepadTableView clickedRow];
	
	[NSApp beginSheet:keyPanel 
	   modalForWindow: [self window]
		modalDelegate: self
	   didEndSelector: @selector(keyInputDidEnd:returnCode:contextInfo:) 
		  contextInfo: (void*)row];
	
	//NSLog(@"Clicked %i", row);
}

- (void) keyInputDidEnd: (NSWindow *) sheet
			 returnCode: (int) returnCode
			contextInfo: (void *) contextInfo
{
	NSLog(@"Ended, set key:%i on row:%i", returnCode, (int)contextInfo);
	
	[self setControl:[self keyForIndex:(int)contextInfo] withKey:returnCode];
	[keyPanel close];
}

- (void) setControl: (NSString*) dictKey withKey: (int) keyCode
{
	NSDictionary* controls = [self gameControls];
	
	NSMutableDictionary* mutableControls = [NSMutableDictionary dictionaryWithDictionary:controls];
	[mutableControls setObject:[NSArray arrayWithObject:[NSNumber numberWithInt:keyCode]] forKey:dictKey];
	
	[[NSUserDefaults standardUserDefaults] setObject:mutableControls forKey:OEGameControls];
	
}

- (IBAction) restoreDefaultControls: (id) sender
{
	[[NSUserDefaults standardUserDefaults] setObject:[GameDocumentController defaultControls] forKey:OEGameControls];
	
	[controlsTableView reloadData];
}

- (NSString*) keyForIndex:(int) index
{
	switch (index) {
		case 0:
			return @"A";
		case 1:
			return @"B";
		case 2:
			return @"Start";
		case 3:
			return @"Select";
		case 4:
			return @"Up";
		case 5:
			return @"Down";
		case 6:
			return @"Left";
		case 7:
			return @"Right";
		default:
			NSLog(@"Key requested for non existant index");
			return @"Key not implemented";
	}
}

- (NSString*) friendlyDescription: (NSArray*) keyArray
{
	NSMutableString* string = [NSMutableString stringWithCapacity:25];
	for( int i = 0; i < [keyArray count]; i++ )
	{
		[string appendString: [[keyArray objectAtIndex:i] stringValue]];
		if(i < [keyArray count] - 1)
			[string appendString:@", "];
	}
	
	return [NSString stringWithString:string];	
}

@end

@implementation GamePreferencesController (UserDefaults)

- (NSDictionary*) gameControls
{
	return [[NSUserDefaults standardUserDefaults] dictionaryForKey:OEGameControls];
}

- (NSDictionary*) gamepadControls
{
	return [NSKeyedUnarchiver unarchiveObjectWithData:[[NSUserDefaults standardUserDefaults] dataForKey:OEGamepadControls]];
}

@end

@implementation GamePreferencesController

@synthesize bundles;

-(NSRect)newFrameForNewContentView:(NSView *)view {
    NSWindow *window = [self window];
    NSRect newFrameRect = [window frameRectForContentRect:[view frame]];
    NSRect oldFrameRect = [window frame];
    NSSize newSize = newFrameRect.size;
    NSSize oldSize = oldFrameRect.size;
    
    NSRect frame = [window frame];
    frame.size = newSize;
    frame.origin.y -= (newSize.height - oldSize.height);
    
    return frame;
}

-(NSView *)viewForTag:(int)tag {
    NSView *view = nil;
    switch(tag) {
		case 0: view = controlsView; break;
		case 1: view = videoView; break;
		case 2: view = pluginsView; break;
    }
    return view;
}

- (id) init
{
	if(![super initWithWindowNibName:@"Preferences"])
	{
		return nil;
	}
	
	docController = [GameDocumentController sharedDocumentController];
	
	NSMutableArray * mBundles = [[NSMutableArray alloc] init];
	
	for(NSBundle * bundle in [docController bundles])
	{
		NSLog(@"%@", bundle);
		[mBundles addObject:[[[PluginInfo alloc] initWithBundleAtPath:[bundle bundlePath]] retain]];  
	}
	
	bundles = [[NSArray arrayWithArray:mBundles] retain];
	NSLog(@"%@", bundles);
	[mBundles release];
	
	return self;
	
}

-(IBAction)switchView:(id)sender {
	
	// Figure out the new view, the old view, and the new size of the window
	int tag = [sender tag];
	NSView *view = [self viewForTag:tag];
	NSView *previousView = [self viewForTag: currentViewTag];
	currentViewTag = tag;
	
	NSRect newFrame = [self newFrameForNewContentView:view];
	
	// Using an animation grouping because we may be changing the duration
	[NSAnimationContext beginGrouping];
	
	// With the shift key down, do slow-mo animation
	if ([[NSApp currentEvent] modifierFlags] & NSShiftKeyMask)
	    [[NSAnimationContext currentContext] setDuration:5.0];
	
	// Call the animator instead of the view / window directly
	[[[[self window] contentView] animator] replaceSubview:previousView with:view];
	[[[self window] animator] setFrame:newFrame display:YES];
	
	[NSAnimationContext endGrouping];
	
}

- (IBAction)showWindow:(id)sender 
{
	if (![[self window] isVisible])
		[[self window] center];
	[[self window] setContentSize:[controlsView frame].size];
    [[[self window] contentView] addSubview:controlsView];
    [[[self window] contentView] setWantsLayer:YES];
	[super showWindow:sender];
}

- (void)awakeFromNib 
{
	
	[controlsTableView setTarget:self];
	[controlsTableView setDoubleAction:@selector(doubleClickedKeyboard)];
	
	[gamepadTableView setTarget:self];
	[gamepadTableView setDoubleAction:@selector(doubleClickedGamepad)];
	
	// set the new custom cell
	NSTableColumn* column = [[pluginsTableView tableColumns] objectAtIndex:0];
	
	PrefsBundleCell* cell = [[[PrefsBundleCell alloc] init] autorelease];	
	[column setDataCell: cell];		
}

	
@end


@implementation GamePreferencesController (ControlsDataSource)

- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
	switch([aTableView tag])
	{
		case 0:
			return [[self gameControls] count];	
		case 1:
			return [[self gamepadControls] count];
		default:
			return 0;
	}
}


- (id)tableView:(NSTableView *)aTableView
objectValueForTableColumn:(NSTableColumn *)aTableColumn
					  row:(int)rowIndex

{	
	//NSLog(@"%@",[aTableColumn identifier]);
	NSParameterAssert(rowIndex >= 0 && rowIndex < [[self gameControls] count]);	
	
	//Match the index to a button
	NSString* key = [self keyForIndex: rowIndex];

	switch( [aTableView tag] )
	{
		case 0:
		{
			if([[aTableColumn identifier] isEqual: @"Keys"])
			{
				NSArray* keyArray = [[self gameControls] objectForKey:key];
				return [self friendlyDescription: keyArray];
			}
		}
		case 1:
		{
			if([[aTableColumn identifier] isEqual: @"Keys"])
			{
				return @"test";
				//NSArray* keyArray = [[self gamepadControls] objectForKey:key];
				//	NSLog([keyArray description]);
				//return [self friendlyDescription: keyArray];
			}
		}
	}
	
	return key;
}


@end
