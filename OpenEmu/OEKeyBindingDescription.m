/*
 Copyright (c) 2012, OpenEmu Team
 
 
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

#import "OEKeyBindingDescription.h"
#import "OEBindingsController_Internal.h"

@implementation OEKeyBindingDescription
@synthesize name, index, systemWide, _hatSwitchGroup, _axisGroup;

- (id)init
{
    return nil;
}

- (id)OE_initWithName:(NSString *)keyName index:(NSUInteger)keyIndex isSystemWide:(BOOL)isSystemWide
{
    if((self = [super init]))
    {
        name       = [keyName copy];
        index      = keyIndex;
        systemWide = isSystemWide;
    }
    
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    return self;
}

- (void)OE_setAxisGroup:(OEKeyBindingGroupDescription *)value
{
    NSAssert(_axisGroup == nil, @"Cannot change axisGroup after it was set, attempted to change axisGroup of %@ from %@ to %@.", self, _axisGroup, value);
    
    NSAssert([value isMemberOfClass:[OEKeyBindingGroupDescription class]], @"Expecting group of class OEKeyGroupBindingsDescription, instead got: %@", [value class]);
    
    _axisGroup = value;
}

- (void)OE_setHatSwitchGroup:(OEKeyBindingGroupDescription *)value
{
    NSAssert(_hatSwitchGroup == nil, @"Cannot change hatSwitchGroup after it was set, attempted to change hatSwitchGroup of %@ from %@ to %@.", self, _hatSwitchGroup, value);
    
    NSAssert([value isMemberOfClass:[OEKeyBindingGroupDescription class]], @"Expecting group of class OEKeyGroupBindingsDescription, instead got: %@", [value class]);
    
    _hatSwitchGroup = value;
}

- (OEKeyBindingDescription *)oppositeKey
{
    return [_axisGroup oppositeKeyOfKey:self];
}

- (NSArray *)hatSwitchKeys
{
    return [_hatSwitchGroup keys];
}

- (void)enumerateHatSwitchKeysUsingBlock:(void (^)(OEKeyBindingDescription *, BOOL *))block
{
    [_hatSwitchGroup enumerateKeysFromKey:self usingBlock:block];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p keyName: %@ index: %lu, isSystemWide: %@ axisGroup: %@ hatSwitchGroup: %@>", [self class], self, [self name], [self index], [self isSystemWide] ? @"YES" : @"NO", [self OE_axisGroup], [self OE_hatSwitchGroup]];
}

@end
