//
//  OEPrefLibraryController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEPrefLibraryController.h"


@implementation OEPrefLibraryController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
    }
    
    return self;
}

- (void)dealloc{
    [super dealloc];
}

- (void)awakeFromNib{
	NSString* path = [[NSUserDefaults standardUserDefaults] objectForKey:UDDatabasePathKey];
	[pathField setStringValue:[path stringByAbbreviatingWithTildeInPath]];
}
#pragma mark ViewController Overrides
- (NSString*)nibName{
	return @"OEPrefLibraryController";
}

#pragma mark OEPreferencePane Protocol
- (NSImage*)icon{
	return [NSImage imageNamed:@"library_tab_icon"];
}

- (NSString*)title{
	return @"Library";
}

- (NSSize)viewSize{
	// TODO: decide how to implement Available Libraries
	return NSMakeSize(423, 480-177);
}
#pragma mark -
#pragma mark UI Actions
- (IBAction)resetLibraryFolder:(id)sender{
	
}

- (IBAction)changeLibraryFolder:(id)sender{

}

@end
