//
//  OEPrefCoresController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OEPreferencePane.h"

@class OEBackgroundGradientView;
@interface OEPrefCoresController : NSViewController <OEPreferencePane> {
@private
    
}
- (NSImage*)icon;
- (NSString*)title;

- (NSSize)viewSize;
@end
