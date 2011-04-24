//
//  OEGameSystemResponder.m
//  OpenEmu
//
//  Created by Remy Demarest on 17/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import "OESystemResponder.h"
#import "OESystemController.h"
#import "OESystemResponderClient.h"
#import <objc/runtime.h>

@implementation OESystemResponder
@synthesize client, controller;

- (id)init
{
    return [self initWithController:nil];
}

- (id)initWithController:(OESystemController *)value;
{
    if((self = [super init]))
    {
        controller = value;
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

- (void)settingWasSet:(id)aValue forKey:(NSString *)keyName;
{
    
}

- (void)keyboardEventWasSet:(id)theEvent forKey:(NSString *)keyName;
{
    
}

- (void)keyboardEventWasRemovedForKey:(NSString *)keyName;
{
    
}

- (void)HIDEventWasSet:(id)theEvent forKey:(NSString *)keyName;
{
    
}

- (void)HIDEventWasRemovedForKey:(NSString *)keyName;
{
    
}

@end
