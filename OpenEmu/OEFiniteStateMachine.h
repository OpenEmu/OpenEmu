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

@import Foundation;

typedef NSUInteger OEFSMStateLabel;
typedef NSUInteger OEFSMEventLabel;

@interface OEFiniteStateMachine : NSObject

- (void)addState:(OEFSMStateLabel)state;
- (void)addState:(OEFSMStateLabel)state entry:(void(^)(void))entry;
- (void)addState:(OEFSMStateLabel)state exit:(void(^)(void))exit;
- (void)addState:(OEFSMStateLabel)state entry:(void(^)(void))entry exit:(void(^)(void))exit;

- (void)addTransitionFrom:(OEFSMStateLabel)fromState to:(OEFSMStateLabel)nextState event:(OEFSMEventLabel)eventLabel action:(void(^)(void))action;
- (void)setTimerTransitionFrom:(OEFSMStateLabel)fromState to:(OEFSMStateLabel)nextState delay:(NSTimeInterval)delay action:(void(^)(void))action;

- (void)setInitialState:(OEFSMStateLabel)initialState;
- (void)setActionsQueue:(dispatch_queue_t)actionsQueue;
- (void)setStateDescriptions:(NSDictionary *)stateDescriptions;
- (void)setEventDescriptions:(NSDictionary *)eventDescriptions;

- (void)start;
- (void)processEvent:(OEFSMEventLabel)event;

- (NSString *)descriptionForState:(OEFSMStateLabel)state;
- (NSString *)descriptionForEvent:(OEFSMEventLabel)event;
- (void)writeDOTRepresentationToURL:(NSURL *)URL;

@end
