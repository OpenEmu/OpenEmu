//
//  ONPickerController.h
//  OpenNestopiaLeopard
//
//  Created by Josh Weinberg on 2/20/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class XADArchive;

@interface GamePickerController : NSWindowController {
	IBOutlet NSTableView *table;
	NSString *fileName;
	NSArray *files;
}

-(IBAction)unpackSelectedFile:(id)sender;
-(IBAction)cancelPicker:(id)sender;
-(IBAction)selectFile:(id)sender;
-(int)selectedIndex;
-(void)setArchive:(XADArchive*)archive;

@property(readonly) NSString* fileName;

@end
