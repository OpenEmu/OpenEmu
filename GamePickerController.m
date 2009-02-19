//
//  ONPickerController.m
//  OpenNestopiaLeopard
//
//  Created by Josh Weinberg on 2/20/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "GamePickerController.h"
#import <XADMaster/XADArchive.h>

@implementation GamePickerController

@synthesize fileName;

- (id) init
{
	return [super initWithWindowNibName:@"GameFilePicker"];
}

- (id)initWithWindowNibName:(NSString *)aName
{
	return [self init];
}

- (void)awakeFromNib
{
	[table setTarget:self];
	[table setDoubleAction:@selector(unpackSelectedFile:)];
}

- (void)setArchive:(XADArchive*)archive
{
	int i;
	
	fileName = [archive filename];
	NSMutableArray  *muteFiles = [NSMutableArray arrayWithCapacity:[archive numberOfEntries]];
	for(i = 0; i < [archive numberOfEntries];i++)
		[muteFiles addObject:[archive nameOfEntry:i]];
	
	files = [[NSArray arrayWithArray:muteFiles] retain];
}

- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
	return [files count];
}

- (id)tableView:(NSTableView *)tableView
objectValueForTableColumn:(NSTableColumn *)tableColumn
			row:(int)row
{
	return [files objectAtIndex:row];
}

-(IBAction)selectFile:(id)sender
{
	[[NSApplication sharedApplication] stopModalWithCode:1];
	[[self window] close];
	
}

-(int)selectedIndex
{
	return [table selectedRow];
}

-(IBAction)cancelPicker:(id)sender
{
	[[NSApplication sharedApplication] stopModalWithCode:0];
	[[self window] close];
}

-(IBAction)unpackSelectedFile:(id)sender
{
	[self selectFile:nil];
}

@end
