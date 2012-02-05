//
//  OEPrefLabel.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 30.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEPrefCtrlLabelCell.h"


@implementation OEPrefCtrlLabelCell

- (void)setupAttributes{	
	NSMutableDictionary *attributes = [[[NSMutableDictionary alloc] init] autorelease];
	
	NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:8.0 size:11.0];
	NSShadow *shadow = [[[NSShadow alloc] init] autorelease];
	[shadow setShadowBlurRadius:1.0];
	[shadow setShadowColor:[NSColor colorWithDeviceWhite:1.0 alpha:0.25]];
	[shadow setShadowOffset:NSMakeSize(0, -1)];
	
	[attributes setObject:[NSColor colorWithDeviceWhite:0.0 alpha:1.0] forKey:NSForegroundColorAttributeName];
	[attributes setObject:font forKey:NSFontAttributeName];
	[attributes setObject:shadow forKey:NSShadowAttributeName];
	
	
	self.textAttributes = attributes;
	[super setupAttributes];

}

@end
