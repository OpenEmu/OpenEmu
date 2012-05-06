/*
 Copyright (c) 2011, OpenEmu Team
 
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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


+ (Protocol *)gameSystemResponderClientProtocol;
{
    return @protocol(OESystemResponderClient);
}

- (void)setClient:(id<OESystemResponderClient>)value;
{
    if(client != value)
    {
        Protocol *p = [[self class] gameSystemResponderClientProtocol];
        
        NSAssert1(protocol_conformsToProtocol(p, @protocol(OESystemResponderClient)), @"Client protocol %@ does not conform to protocol OEGameSystemResponderClient", NSStringFromProtocol(p));
        
        //NSAssert2(value == nil || [value conformsToProtocol:p], @"Client %@ does not conform to protocol %@.", value, NSStringFromProtocol(p));
        
        client = value;
    }
}

- (void)settingWasSet:(id)aValue forKey:(NSString *)keyName;
{
    
}

- (void)keyboardEvent:(id)theEvent wasSetForKey:(NSString *)keyName;
{
    
}

- (void)keyboardEvent:(id)theEvent wasUnsetForKey:(NSString *)keyName;
{
    
}

- (void)HIDEvent:(id)theEvent wasSetForKey:(NSString *)keyName;
{
    
}

- (void)HIDEvent:(id)theEvent wasUnsetForKey:(NSString *)keyName;
{
    
}

@end
