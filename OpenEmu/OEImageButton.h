//
//  OEToolbarButton.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 16.04.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OEUIDrawingUtils.h"
@class OECenteredTextFieldCell;
@interface OEImageButton : NSButton 
{
}
@property BOOL isInHover;
@end

@interface OEImageButtonCell : NSButtonCell 
{
@private
    NSImage *image;
}
- (BOOL)displaysHover;
- (NSRect)imageRectForButtonState:(OEButtonState)state;
@property (retain, readwrite) NSImage *image;
@end

// displays normal, inactive, disabled, pressed
@interface OEToolbarButtonPushCell : OEImageButtonCell 
{
@private
}
@end
// displays normal, inactive, disabled, pressed for Selected / Unselected
@interface OEToolbarButtonSelectableCell : OEImageButtonCell 
{
@private
}
@end

// Displays normal, hover and pressed
@interface OEImageButtonHoverPressed : OEImageButtonCell 
{
@private
}

@property BOOL splitVertically;
@end

// Displays normal, hover and pressed for Selected / Unselected
@interface OEImageButtonHoverSelectable : OEImageButtonCell
{
}
@end

// Displays normal, pressed
@interface OEImageButtonPressed : OEImageButtonCell
{
}
@end

@interface OEImageButtonHoverPressedText : OEImageButtonHoverPressed {
    OECenteredTextFieldCell *textFieldCell;
}
@property (copy)   NSString     *text;
@property (retain) NSDictionary *normalAttributes;
@property (retain) NSDictionary *hoverAttributes;
@property (retain) NSDictionary *clickAttributes;
@end