//
//  AttributedTextFieldCell.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 30.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "AttributedTextFieldCell.h"


@implementation AttributedTextFieldCell
@synthesize textAttributes;
- (id)init{
    self = [super init];
    if (self) {
		[self setupAttributes];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder{
    self = [super initWithCoder:aDecoder];
    if (self) {
		[self setupAttributes];
    }    
    return self;
}
-(id)initImageCell:(NSImage *)image{
	self = [super initImageCell:image];
    if (self) {
		[self setupAttributes];
    }    
    return self;
}
- (id)initTextCell:(NSString *)aString{
	self = [super initTextCell:aString];
    if (self) {
		[self setupAttributes];
    }    
    return self;
}

- (void)dealloc{
	self.textAttributes = nil;
    [super dealloc];
}

- (void)setStringValue:(NSString *)aString{
	[self setAttributedStringValue:[[[NSAttributedString alloc] initWithString:aString attributes:self.textAttributes] autorelease]];
}

- (void)setupAttributes{
	self.textAttributes = [NSDictionary dictionary];
}
@end
