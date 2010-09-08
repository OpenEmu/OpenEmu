//
//  OEThumbstickControl.m
//  iPhoneRemote
//
//  Created by Josh Weinberg on 9/8/10.
//  Copyright 2010 OpenEmu. All rights reserved.
//

#import "OEThumbstickControl.h"


@implementation OEThumbstickControl
@synthesize delegate;

- (id)initWithCoder:(NSCoder *)aDecoder
{
    if ((self = [super initWithCoder:aDecoder])) {
        // Initialization code
		nub = [[CALayer layer] retain];
		[nub setBackgroundColor:[[UIColor redColor] CGColor]];
		[nub setCornerRadius:25];
		nub.frame = CGRectMake(0, 0, 50, 50);
		[self.layer addSublayer:nub];
    }
    return self;
}

- (void)setLRDir:(NSInteger)newDir
{
	if (newDir != lrDir)
	{
		if (lrDir == -1)
			[self.delegate releaseLeft:self];
		else if (lrDir == 1)
			[self.delegate releaseRight:self];
		lrDir = newDir;
		if (lrDir == -1)
			[self.delegate pressLeft:self];
		else if (lrDir == 1)
			[self.delegate pressRight:self];
	}
}

- (void)setUDDir:(NSInteger)newDir
{
	if (newDir != udDir)
	{
		if (udDir == -1)
			[self.delegate releaseUp:self];
		else if (udDir == 1)
			[self.delegate releaseDown:self];
		udDir = newDir;
		if (udDir == -1)
			[self.delegate pressUp:self];
		else if (udDir == 1)
			[self.delegate pressDown:self];
	}
}

- (void)setCurrentPoint:(CGPoint)p
{
	currentPoint = p;
	
	CGPoint c = CGPointMake(self.bounds.size.width / 2.0f, self.bounds.size.height/2.0f);
	
	float dirX = p.x - c.x;
	float dirY = p.y - c.y;
	
	float len2 = dirX * dirX + dirY * dirY;
	float len = sqrt(len2);
	dirX = dirX / len;
	dirY = dirY / len;
	
	float angle = atan2(dirY, dirX);
	angle = fmodf((angle + M_PI * 2.f), M_PI * 2.0f);
	
	if (len2 > 100.f)
		
	{
		
		const float deadzone = (M_PI / 3.0f);
		if (angle > deadzone && angle < M_PI - deadzone)
		{
			[self setLRDir:1];
		}
		else if (angle >= M_PI + deadzone && angle < M_PI * 2.0f - deadzone)
		{
			[self setLRDir:-1];
		}
		else
		{
			[self setLRDir:0];
		}
		
		if (angle > 3 * M_PI_2 + deadzone || angle < M_PI_2 - deadzone)
		{
			[self setUDDir:-1];
		}
		else if (angle > M_PI_2 + deadzone && angle < 3.0f * M_PI_2 - deadzone)
		{
			[self setUDDir:1];
		}
		else
		{
			[self setUDDir:0];
		}
	}
	else
	{
		[self setLRDir:0];
		[self setUDDir:0];
	}
	if (len > 30)
	{
		dirX *= 30.0f;
		dirY *= 30.0f;
		currentPoint = CGPointMake(dirX + c.x, dirY + c.y);
	}
	
	//NSLog(@"Angle: %f len2:%f", angle, len2);

//	NSLog(@"%f", angle);
	[CATransaction begin];
	[CATransaction setDisableActions:YES];
	nub.position = currentPoint;
	[CATransaction commit];
}

// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect 
{
	CGContextRef ctx = UIGraphicsGetCurrentContext();
	CGContextAddEllipseInRect(ctx, rect);
	CGContextFillPath(ctx);
}

- (void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event;
{
	touchDown = YES;
	[self setCurrentPoint:[[touches anyObject] locationInView:self]];
}

- (void) touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event;
{
	[self setCurrentPoint:[[touches anyObject] locationInView:self]];
}

- (void) touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event;
{
	CGRect b = self.bounds;
	[self setCurrentPoint:CGPointMake(b.size.width / 2.0f, b.size.height/2.0f)];
}

- (void) touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event;
{
	CGRect b = self.bounds;
	[self setCurrentPoint:CGPointMake(b.size.width / 2.0f, b.size.height/2.0f)];
}

- (void)dealloc
{
    [super dealloc];
}


@end
