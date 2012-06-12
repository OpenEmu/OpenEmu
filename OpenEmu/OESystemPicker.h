//
//  OESystemPicker.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 10.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OESystemPlugin.h"
@interface OESystemPicker : NSWindowController
+ (OESystemPlugin*)pickSystemFromArray:(NSArray*)plugins;

@property (strong) IBOutlet NSPopUpButton *systemSelector;
@end
