//
//  OEFieldEditor.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 18.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface OEGridViewFieldEditor : NSView {
@private
    NSTextField *textView;
}
// value
- (void)setString:(NSString*)newString;
- (NSString*)string;
#pragma mark -
// alignment
- (void)setAlignment:(NSTextAlignment)alignment;
- (NSTextAlignment)alignment;
// font
- (void)setFont:(NSFont*)newFont;
- (NSFont*)font;
#pragma mark -
- (void)setDelegate:(id)delegate;
- (id)delegate;
@property (retain, readwrite) NSColor *borderColor;
@end
