/*
 Copyright (c) 2011, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

@import Foundation;
#import "OEUIDrawingUtils.h"

@class OECenteredTextFieldCell;

@interface OEImageButton : NSButton
@property BOOL isInHover;
@end

@interface OEImageButtonCell : NSButtonCell

- (BOOL)displaysHover;
- (NSRect)imageRectForButtonState:(OEButtonState)state;
@property(retain, readwrite) NSImage *image;
@end

// displays normal, inactive, disabled, pressed
@interface OEToolbarButtonPushCell : OEImageButtonCell
@end

// displays normal, inactive, disabled, pressed for Selected / Unselected
@interface OEToolbarButtonSelectableCell : OEImageButtonCell
@end

// Displays normal, hover and pressed
@interface OEImageButtonHoverPressed : OEImageButtonCell
@property BOOL splitVertically;
@end

// Displays normal, hover and pressed for Selected / Unselected
@interface OEImageButtonHoverSelectable : OEImageButtonCell
@end

// Displays normal, pressed
@interface OEImageButtonPressed : OEImageButtonCell
@end

@interface OEImageButtonHoverPressedText : OEImageButtonHoverPressed
{
    OECenteredTextFieldCell *textFieldCell;
}

@property(copy) NSString     *text;
@property(copy) NSDictionary *normalAttributes;
@property(copy) NSDictionary *hoverAttributes;
@property(copy) NSDictionary *clickAttributes;
@end
