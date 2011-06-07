//
//  OEToolbarButton.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 16.04.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OEUIDrawingUtils.h"
@interface OEImageButton : NSButton {
@private
}
@end

@interface OEImageButtonCell : NSButtonCell {
@private
    NSImage* image;
}
- (NSRect)imageRectForState:(OEUIState)state;
@property (retain, readwrite) NSImage* image;
@end

@interface OEToolbarButtonPushCell : OEImageButtonCell {
@private
}
@end

@interface OEToolbarButtonSelectableCell : OEImageButtonCell {
@private
}
@end