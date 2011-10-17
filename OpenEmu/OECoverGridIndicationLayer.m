//
//  CoverGridIndicationLayer.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 02.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OECoverGridIndicationLayer.h"
#import "NSColor+IKSAdditions.h"

// to do, define somewhere else
#define IndicationTypeNone 0
#define IndicationTypeFileMissing 2
#define IndicationTypeProcessing 1
#define IndicationTypeDropOn 3

#define Deg_to_Rad(X ) (X*M_PI/180.0)

@implementation OECoverGridIndicationLayer

- (id)init {
    self = [super init];
    if (self) {
		self.needsDisplayOnBoundsChange = NO;
		CALayer* sublayer = [CALayer layer];
		sublayer.delegate = self;
		
		[self addSublayer:sublayer];
    }
    return self;
}

- (void)dealloc {
    [super dealloc];
}

- (void)setType:(int)newType{
	CALayer* sublayer = [self.sublayers objectAtIndex:0];
	
	switch (newType) {
		case IndicationTypeNone:{
			sublayer.contents = nil;
			self.backgroundColor = [[NSColor clearColor] CGColor];
			[sublayer removeAllAnimations];
		};
			break;
		case IndicationTypeFileMissing:{
			self.backgroundColor = [[NSColor colorWithDeviceRed:0.992 green:0.0 blue:0.0 alpha:0.4] CGColor];
			
			NSImage* fileMissingIndicator = [self fileMissingImage];
			float width = self.bounds.size.width * 0.45;
			float height = width*0.9;
			
			
			CGRect fileMissingIndicatorRect = CGRectMake(self.bounds.origin.x+(self.bounds.size.width-width)/2, self.bounds.origin.y+(self.bounds.size.height-height)/2, width, height);
			sublayer.contents = fileMissingIndicator;
			sublayer.shadowOffset = CGSizeMake(0, -1);
			sublayer.shadowOpacity = 1.0;
			sublayer.shadowRadius = 1.0;
			sublayer.shadowColor = [[NSColor colorWithDeviceRed:0.341 green:0.0 blue:0.012 alpha:6.0] CGColor];
			sublayer.frame = fileMissingIndicatorRect;
			[sublayer removeAllAnimations];
		};
			break;
		case IndicationTypeProcessing:{
			self.backgroundColor = [[NSColor colorWithDeviceRed:0.0 green:0.0 blue:0.0 alpha:0.7] CGColor];
			
			NSImage* spinnerImage = [NSImage imageNamed:@"spinner"];
			
			if(type!=IndicationTypeProcessing){
				sublayer.contents = spinnerImage;
				sublayer.shadowOffset = CGSizeMake(0, -1);
				sublayer.shadowOpacity = 1.0;
				sublayer.shadowRadius = 1.0;
				sublayer.shadowColor = [[NSColor colorWithDeviceRed:0.0 green:0.0 blue:0.0 alpha:6.0] CGColor];
				sublayer.anchorPoint = CGPointMake(0.5, 0.5);
				sublayer.anchorPointZ = 0.0;

				
				CAKeyframeAnimation * animation; 
				animation = [CAKeyframeAnimation animationWithKeyPath:@"transform.rotation.z"]; 
				animation.calculationMode = kCAAnimationDiscrete;
				
				animation.duration = 1.0; 
				animation.repeatCount = HUGE_VALF;
				animation.removedOnCompletion = NO;
				
				float angle = 0;
				NSMutableArray *values = [NSMutableArray array];
				while(angle<360){
					[values addObject:[NSNumber numberWithFloat:Deg_to_Rad(-angle)]];
					angle += 30;
				}
				
				animation.values = values;
				
				[sublayer addAnimation:animation forKey:nil];			
			} 			
			
			sublayer.frame = CGRectMake((self.frame.size.width-spinnerImage.size.width)/2, (self.frame.size.height-spinnerImage.size.height)/2, spinnerImage.size.width, spinnerImage.size.height);
		};
			break;
		case IndicationTypeDropOn:{
			sublayer.contents = nil;

			[sublayer removeAllAnimations];
			self.backgroundColor = [[NSColor colorWithDeviceRed:0.4 green:0.361 blue:0.871 alpha:0.7] CGColor];
		};
			break;
		default:
			break;
	}
	type = newType;
	[self setNeedsDisplay];
}
#pragma mark -
#pragma mark Helper
- (id < CAAction >)actionForLayer:(CALayer *)layer forKey:(NSString *)key{
	return (id < CAAction >)[NSNull null];
}

- (NSImage*)fileMissingImage{
	static NSImage* missingromimage = nil;
	if(missingromimage == nil){
		missingromimage = [NSImage imageNamed:@"missing_rom"];
	}	
	return missingromimage;
}
@synthesize type;
@end
