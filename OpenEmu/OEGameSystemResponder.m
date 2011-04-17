//
//  OEGameSystemResponder.m
//  OpenEmu
//
//  Created by Remy Demarest on 17/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import "OEGameSystemResponder.h"


@implementation OEGameSystemResponder
@synthesize client;

- (id)init
{
    if((self = [super init]))
    {
        // Initialization code here.
    }
    
    return self;
}

- (void)dealloc
{
    [super dealloc];
}

- (id<OEGameSystemResponderClient>)client { return client; }
- (void)setClient:(id<OEGameSystemResponderClient>)value;
{
    Protocol *p = [[self class] gameSystemResponderClientProtocol];
    
    NSAssert2([value conformsToProtocol:p], @"Client %@ does not conform to protocol %@.", value, NSStringFromProtocol(p));
    
    if(client != value)
    {
        client = value;
    }
}

@end
