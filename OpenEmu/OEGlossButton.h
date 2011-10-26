//
//  OEGlossButton.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 26.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <AppKit/AppKit.h>

enum _OEGlossButtonColor 
{
	OEGlossButtonColorDefault,
	OEGlossButtonColorBlue,
	OEGlossButtonColorGreen
}
typedef OEGlossButtonColor;

@interface OEGlossButton : NSButton
@property OEGlossButtonColor buttonColor;
@end

@interface OEGlossButtonCell : NSButtonCell
@property OEGlossButtonColor buttonColor;
@end
