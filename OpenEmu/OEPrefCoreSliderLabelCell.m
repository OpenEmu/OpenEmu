//
//  OEPrefCoreSliderLabel.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 31.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEPrefCoreSliderLabelCell.h"


@implementation OEPrefCoreSliderLabelCell

- (void)setupAttributes{
	NSMutableDictionary *attributes = [[[NSMutableDictionary alloc] init] autorelease];
	
	NSMutableParagraphStyle *paragraphStyle = [[[NSMutableParagraphStyle alloc] init] autorelease];
	[paragraphStyle setAlignment:[self alignment]];
	
	NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:4.0 size:12.0];
		
	[attributes setObject:[NSColor colorWithDeviceWhite:0.89 alpha:1.0] forKey:NSForegroundColorAttributeName];
	[attributes setObject:font forKey:NSFontAttributeName];
	[attributes setObject:paragraphStyle forKey:NSParagraphStyleAttributeName];
	
	
	self.textAttributes = attributes;
	
	NSRect frame = [[self controlView] frame];
	
	frame.size.height += 5;
	frame.origin.y -= 5;
	[[self controlView] setFrame:frame];	
	
	[super setupAttributes];
}

@end
