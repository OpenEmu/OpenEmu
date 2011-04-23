//
//  OEGameSystemResponder.m
//  OpenEmu
//
//  Created by Remy Demarest on 17/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import "OESystemResponder.h"
#import "OESystemResponderClient.h"
#import <objc/runtime.h>

@implementation OESystemResponder
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

+ (Protocol *)gameSystemResponderClientProtocol;
{
    return @protocol(OESystemResponderClient);
}

- (id<OESystemResponderClient>)client { return client; }
- (void)setClient:(id<OESystemResponderClient>)value;
{
    if(client != value)
    {
        Protocol *p = [[self class] gameSystemResponderClientProtocol];
        
        NSAssert1(protocol_conformsToProtocol(p, @protocol(OESystemResponderClient)), @"Client protocol %@ does not conform to protocol OEGameSystemResponderClient", NSStringFromProtocol(p));
        
        NSAssert2(value == nil || [value conformsToProtocol:p], @"Client %@ does not conform to protocol %@.", value, NSStringFromProtocol(p));
        
        client = value;
    }
}

- (void)unsetEventForKey:(bycopy NSString *)keyName withValueMask:(NSUInteger)keyMask;
{
    
}

- (void)settingWasSet:(bycopy id)aValue forKey:(bycopy NSString *)keyName;
{
    
}

- (void)keyboardEventWasSet:(bycopy id)theEvent forKey:(bycopy NSString *)keyName;
{
    
}

- (void)keyboardEventWasRemovedForKey:(bycopy NSString *)keyName;
{
    
}

- (void)HIDEventWasSet:(bycopy id)theEvent forKey:(bycopy NSString *)keyName;
{
    
}

- (void)HIDEventWasRemovedForKey:(bycopy NSString *)keyName;
{
    
}

@end
