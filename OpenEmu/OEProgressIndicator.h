//
//  OEProgressIndicator.h
//  OETheme
//
//  Created by Christoph Leimbrock on 13.10.12.
//  Copyright (c) 2012 OpenEmu. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OEControl.h"
#import "OECell.h"
@interface OEProgressIndicator : NSProgressIndicator <OEControl, OECell>
@property (getter=isPaused) BOOL paused;
@end
