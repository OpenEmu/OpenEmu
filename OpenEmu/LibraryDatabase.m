//
//  LibraryDatabase.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 31.03.11.
//  Copyright 2011 none. All rights reserved.
//

#import "LibraryDatabase.h"

#import "OEDBRom.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation LibraryDatabase

- (id)init{
    self = [super init];
    if (self) {
        consoles = [NSArray new];
        collections = [NSArray new];
		
		romsController = [[NSArrayController alloc] init];
		
		[self loadConsoleImages];
    }
    
    return self;
}

- (void)dealloc{
	[consoles release];
	[collections release];
	
	[romsController release];
	
    [super dealloc];
}


- (void)loadConsoleImages{
	NSImage* consoleIcons = [NSImage imageNamed:@"consoles"];
	
	// Bottom Left -> top right
	// first row (bottom)
	[consoleIcons setName:@"Sega Genesis" forSubimageInRect:NSMakeRect(0, 0, 16, 16)];	
	[consoleIcons setName:@"Sega 32x" forSubimageInRect:NSMakeRect(16, 0, 16, 16)];
	[consoleIcons setName:@"Sega CD" forSubimageInRect:NSMakeRect(32, 0, 16, 16)];
	[consoleIcons setName:@"Game Gear" forSubimageInRect:NSMakeRect(48, 0, 16, 16)];
	
	// Second row:
	[consoleIcons setName:@"GameBoy Advance" forSubimageInRect:NSMakeRect(0, 16, 16, 16)];
	[consoleIcons setName:@"VirtualBoy" forSubimageInRect:NSMakeRect(16, 16, 16, 16)];
	[consoleIcons setName:@"Sega SG-1000" forSubimageInRect:NSMakeRect(32, 16, 16, 16)];
	[consoleIcons setName:@"Sega Master System" forSubimageInRect:NSMakeRect(48, 16, 16, 16)];
	
	// Third row:
	[consoleIcons setName:@"Super Nintendo (SNES)" forSubimageInRect:NSMakeRect(0, 32, 16, 16)];
	[consoleIcons setName:@"Super Nintendo (SNES)" forSubimageInRect:NSMakeRect(16, 32, 16, 16)];
	[consoleIcons setName:@"Nintendo 64" forSubimageInRect:NSMakeRect(32, 32, 16, 16)];
	[consoleIcons setName:@"GameBoy" forSubimageInRect:NSMakeRect(48, 32, 16, 16)];
	
	// Fourth row (top):
	[consoleIcons setName:@"Arcade (MAME)" forSubimageInRect:NSMakeRect(0, 48, 16, 16)];	
	[consoleIcons setName:@"Atari 2600" forSubimageInRect:NSMakeRect(16, 48, 16, 16)];	
	[consoleIcons setName:@"Nintendo (NES)" forSubimageInRect:NSMakeRect(32, 48, 16, 16)]; // eu / us, load only for locale????
	[consoleIcons setName:@"Famicom" forSubimageInRect:NSMakeRect(48, 48, 16, 16)]; // jap, load only for locale????	
}
#pragma mark -
#pragma mark For Testing...
- (void)setConsoles:(NSArray*)newConsoles{
	consoles = newConsoles;
}

- (void)addRoms:(NSArray*)newRoms{
	if(newRoms == nil || [newRoms count]==0)
		return;
	
	// for testing, mark random rom with file not found
	NSUInteger index = rand() % [newRoms count];
    [(OEDBRom*)[newRoms objectAtIndex:index] setFileStatus:1];
	NSLog(@"'%@' is missing", [(OEDBRom*)[newRoms objectAtIndex:index] title]);
	
	// for testing, mark random rom with processing
	index = rand() % [newRoms count];
    [(OEDBRom*)[newRoms objectAtIndex:index] setFileStatus:2];
	NSLog(@"'%@' is processing", [(OEDBRom*)[newRoms objectAtIndex:index] title]);
	
	// for testing, mark random rom as unplayerd;
	index = rand() % [newRoms count];
	NSDate* refDate = [NSDate dateWithTimeIntervalSince1970:0];
	[(OEDBRom*)[newRoms objectAtIndex:index] setRomLastPlayed:refDate];
	NSLog(@"'%@' is unplayed", [(OEDBRom*)[newRoms objectAtIndex:index] title]);
	
	[romsController addObjects:newRoms];
}

- (void)setCollections:(NSArray*)newCollections{
	collections = [newCollections retain];
}

#pragma mark -
#pragma mark Database queries
- (NSArray*)consoles{
	return consoles;
}

- (NSArray*)collections{
	return collections;
}

- (NSArray*)romsForPredicate:(NSPredicate*)predicate{
	[romsController setFilterPredicate:predicate];
	
	return [romsController arrangedObjects];	
}

- (NSArray*)romsInCollection:(id)collection{
	NSLog(@"asking for romsInCollection");
	return [NSArray array];
}

@synthesize lastResult;
@synthesize lastSearch;
@synthesize lastSystem;
@end
