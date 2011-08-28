//
//  OEControlsKeyLabelCell.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 25.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEControlsKeyLabelCell.h"

@implementation OEControlsKeyLabelCell

- (void)setupAttributes{
	NSMutableDictionary *attributes = [[[NSMutableDictionary alloc] init] autorelease];
	
	NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:0 size:11.0];
	
	NSShadow* shadow = [[[NSShadow alloc] init] autorelease];
	[shadow setShadowBlurRadius:1.0];
	[shadow setShadowColor:[NSColor colorWithDeviceWhite:1.0 alpha:0.25]];
	[shadow setShadowOffset:NSMakeSize(0, -1)];
	
	[attributes setObject:[NSColor colorWithDeviceWhite:0.0 alpha:1.0] forKey:NSForegroundColorAttributeName];
	[attributes setObject:font forKey:NSFontAttributeName];
	[attributes setObject:shadow forKey:NSShadowAttributeName];
	
	
	NSMutableParagraphStyle* style = [[NSMutableParagraphStyle alloc] init];
	[style setAlignment:NSRightTextAlignment];
	[attributes setObject:style forKey:NSParagraphStyleAttributeName];
	[style release];
	
	self.textAttributes = attributes;
	
	[self setStringValue:[self stringValue]];
}

@end
