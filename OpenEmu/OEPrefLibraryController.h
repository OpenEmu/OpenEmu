//
//  OEPrefLibraryController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OEPreferencePane.h"

@interface OEPrefLibraryController : NSViewController <OEPreferencePane> {
@private
    
}
- (NSImage*)icon;
- (NSString*)title;

- (NSSize)viewSize;
@end
