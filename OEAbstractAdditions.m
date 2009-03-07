//
//  OEAbstractAdditions.m
//  OpenEmu
//
//  Created by Remy Demarest on 22/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OEAbstractAdditions.h"


@implementation NSObject (OEAbstractAdditions)

+ (void)doesNotImplementSelector:(SEL)aSel
{
	@throw [NSException exceptionWithName:NSInvalidArgumentException
								   reason:[NSString stringWithFormat:@"*** +%s cannot be sent to the abstract class %@: Create a concrete subclass!", sel_getName(aSel), [self class]]
								 userInfo:nil];
}

- (void)doesNotImplementSelector:(SEL)aSel
{
	@throw [NSException exceptionWithName:NSInvalidArgumentException
								   reason:[NSString stringWithFormat:@"*** -%s cannot be sent to an abstract object of class %@: Create a concrete instance!", sel_getName(aSel), [self class]]
								 userInfo:nil];
}

+ (void)doesNotImplementOptionalSelector:(SEL)aSel
{
	NSLog(@"*** +%s is an optional method and it is not implemented in %@!", sel_getName(aSel), NSStringFromClass([self class]));
}

- (void)doesNotImplementOptionalSelector:(SEL)aSel
{
	NSLog(@"*** -%s is an optional method and it is not implemented in %@!", sel_getName(aSel), NSStringFromClass([self class]));
}

@end
