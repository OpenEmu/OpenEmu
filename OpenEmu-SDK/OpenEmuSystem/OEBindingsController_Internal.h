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

#import "OEBindingsController.h"
#import "OEKeyBindingGroupDescription.h"
#import "OEKeyBindingDescription.h"
#import "OESystemBindings.h"
#import "OEPlayerBindings.h"

@interface OEBindingsController ()
- (void)OE_setRequiresSynchronization;
@end

@interface OEKeyBindingGroupDescription ()
- (id)initWithGroupType:(OEKeyGroupType)aType keys:(NSArray *)groupedKeys;
@end

@interface OEKeyBindingDescription ()

- (id)OE_initWithName:(NSString *)keyName index:(NSUInteger)keyIndex isSystemWide:(BOOL)systemWide __attribute__((objc_method_family(init)));

@property(weak, nonatomic, getter=OE_hatSwitchGroup, setter=OE_setHatSwitchGroup:) OEKeyBindingGroupDescription *_hatSwitchGroup;
@property(weak, nonatomic, getter=OE_axisGroup     , setter=OE_setAxisGroup:)      OEKeyBindingGroupDescription *_axisGroup;

@end

@interface OESystemBindings ()

- (id)OE_initWithBindingsController:(OEBindingsController *)parentController systemController:(OESystemController *)aController dictionaryRepresentation:(NSDictionary *)aDictionary __attribute__((objc_method_family(init)));

- (NSDictionary *)OE_dictionaryRepresentation;

- (void)OE_didAddDeviceHandler:(OEDeviceHandler *)aHandler;
- (void)OE_didRemoveDeviceHandler:(OEDeviceHandler *)aHandler;

- (id)OE_playerBindings:(OEPlayerBindings *)sender didAssignEvent:(id)value toKeyWithName:(NSString *)aKey;

@end

@interface OEPlayerBindings ()

- (id)OE_initWithSystemBindings:(OESystemBindings *)aController playerNumber:(NSUInteger)playerNumber __attribute__((objc_method_family(init)));

@property(readwrite, setter=OE_setPlayerNumber:) NSUInteger playerNumber;

// Keys:   NSString - All key-name for each existing bindings excluding key groups
// Values: NSString - String representation of the associated event
@property(readwrite, copy, setter=OE_setBindingDescriptions:) NSDictionary *bindingDescriptions;

- (id)OE_bindingDescriptionForKey:(NSString *)aKey;
- (void)OE_setBindingDescription:(id)value forKey:(NSString *)aKey;

// Keys:   OEKeyBindingsDescription or OEOrientedKeyGroupBindingDescription - All keys for saved bindings
// Values: OEHIDEvent - Associated event
@property(readwrite, copy, setter=OE_setBindingEvents:) NSDictionary *bindingEvents;

- (id)OE_bindingEventForKey:(id)aKey;
- (void)OE_setBindingEvent:(id)value forKey:(id)aKey;

@end

@interface OEDevicePlayerBindings ()

- (id)OE_initWithSystemBindings:(OESystemBindings *)aController playerNumber:(NSUInteger)playerNumber deviceHandler:(OEDeviceHandler *)handler __attribute__((objc_method_family(init)));

- (id)OE_playerBindingsWithDeviceHandler:(OEDeviceHandler *)aHandler playerNumber:(NSUInteger)aPlayerNumber;

- (void)OE_makeIndependent;
@property(readwrite, setter=OE_setDeviceHandler:) OEDeviceHandler *deviceHandler;
@property(readonly, getter=OE_isDependent) BOOL _isDependent;

@end
