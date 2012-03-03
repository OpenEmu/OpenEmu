//
//  OEPrefDebugController.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 22.02.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OEPreferencePane.h"

@interface OEPrefDebugController : NSViewController <OEPreferencePane>
@property (retain) IBOutlet NSPopUpButton* regionSelector;
@property (retain) IBOutlet NSPopUpButton* dbActionSelector;

@property (retain) IBOutlet NSView* contentView;
#pragma mark -
#pragma mark UI Methods
- (IBAction)changeRegion:(id)sender;
- (IBAction)executeDatbaseAction:(id)sender;
@end
