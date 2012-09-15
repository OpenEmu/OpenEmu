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

#import <Foundation/Foundation.h>

@class OESystemBindings, OEHIDDeviceHandler, OEHIDEvent;

/// Manages the bindings for a specific player in a system, useful for preferences
/// Instances of this class are allocated by OESystemBindings
@interface OEPlayerBindings : NSObject

@property(readonly, weak) OESystemBindings *systemBindingsController;

@property(readonly) NSUInteger playerNumber;

// Keys:   NSString - All key-name for each existing bindings excluding key groups
// Values: NSString - String representation of the associated event
// There are no key-groups in this case, all keys have their own strings
@property(readonly, copy) NSDictionary *bindingDescriptions;

// Keys:   OEKeyBindingsDescription or OEOrientedKeyGroupBindingDescription - All keys for saved bindings
// Values: OEHIDEvent - Associated event
@property(readonly, copy) NSDictionary *bindingEvents;

/// @param key one of the control keys
/// @result the event value for the specific type
- (id)valueForKey:(NSString *)key;

/// @result the key or key group that got assigned
- (id)assignEvent:(OEHIDEvent *)anEvent toKeyWithName:(NSString *)aKeyName;

@end

@interface OEDevicePlayerBindings : OEPlayerBindings
@property(readonly) OEHIDDeviceHandler *deviceHandler;
@end

@interface OEKeyboardPlayerBindings : OEPlayerBindings
@end
