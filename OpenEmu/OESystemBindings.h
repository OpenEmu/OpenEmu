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

@protocol OESystemBindingsObserver;
@class OEBindingsController;
@class OEDeviceHandler;
@class OEDevicePlayerBindings;
@class OEHIDEvent;
@class OEKeyboardPlayerBindings;
@class OEPlayerBindings;
@class OESystemController;

/// Manages the bindings for a specific system, useful for system responders
/// Instances of this class are allocated by OEGameBindingsController
@interface OESystemBindings : NSObject

@property(readonly, weak) OEBindingsController *bindingsController;
@property(readonly, weak) OESystemController   *systemController;

@property(readonly) NSUInteger numberOfPlayers;

- (OEDevicePlayerBindings *)devicePlayerBindingsForPlayer:(NSUInteger)playerNumber;
- (OEKeyboardPlayerBindings *)keyboardPlayerBindingsForPlayer:(NSUInteger)playerNumber;

- (NSUInteger)playerForDeviceHandler:(OEDeviceHandler *)deviceHandler;
- (OEDeviceHandler *)deviceHandlerForPlayer:(NSUInteger)playerNumber;
- (OEDevicePlayerBindings *)devicePlayerBindingsForDeviceHandler:(OEDeviceHandler *)deviceHandler;
- (void)setDeviceHandler:(OEDeviceHandler *)deviceHandler forPlayer:(NSUInteger)playerNumber;

// Returns player number based on the device number of the event
// Returns 0 for any keyboard events
- (NSUInteger)playerNumberForEvent:(OEHIDEvent *)anEvent;

- (void)addBindingsObserver:(id<OESystemBindingsObserver>)observer;
- (void)removeBindingsObserver:(id<OESystemBindingsObserver>)observer;

@end

@protocol OESystemBindingsObserver <NSObject>

/// @param event OEHIDEvent object representing the event to watch
/// @param bindingDescription OEKeyBindingsDescription or OEOrientedKeyGroupBindingDescription object representing the key to trigger for the event
/// @param playerNumber NSUInteger from 0 to numberOfPlayers included, 0 indicates system bindings
- (void)systemBindings:(OESystemBindings *)sender didSetEvent:(OEHIDEvent *)event forBinding:(id)bindingDescription playerNumber:(NSUInteger)playerNumber;
- (void)systemBindings:(OESystemBindings *)sender didUnsetEvent:(OEHIDEvent *)event forBinding:(id)bindingDescription playerNumber:(NSUInteger)playerNumber;

@end
