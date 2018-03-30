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

#import "OEFiniteStateMachine.h"

#pragma mark - Private variables

static char *const _OEFSMDefaultProcessingQueuePrefix = "org.openemu.OEFiniteStateMachine.processingQueue";
static char *const _OEFSMDefaultTimersQueuePrefix     = "org.openemu.OEFiniteStateMachine.timersQueue";

#pragma mark - Private functions

static void OE_timersQueueFinalizer(void *);

#pragma mark - Private types

typedef struct
{
    BOOL cancelled;
} OETimersQueueContext;

@interface OEFiniteStateMachine ()
{
    NSMutableDictionary *_states;
    NSMutableDictionary *_transitions;      // indexed by fromState, contains a dictionary of OEFSMTransition objects indexed by event
    NSMutableDictionary *_timerTransitions; // indexed by fromState, contains an array of OEFSMTimerTransition objects

    OEFSMStateLabel      _initialState;
    OEFSMStateLabel      _currentState;

    BOOL                 _running;
    dispatch_queue_t     _processingQueue;
    dispatch_queue_t     _actionsQueue;
    dispatch_queue_t     _timersQueue;

    NSDictionary        *_stateDescriptions;
    NSDictionary        *_eventDescriptions;
}

@end

@interface OEFSMState : NSObject

@property(nonatomic, readonly) OEFSMStateLabel label;
@property(nonatomic, readonly) void(^entry)(void);
@property(nonatomic, readonly) void(^exit)(void);

+ (instancetype)stateWithLabel:(OEFSMStateLabel)label entry:(void(^)(void))entry exit:(void(^)(void))exit;

@end

@interface OEFSMTransition : NSObject

@property(nonatomic, readonly) OEFSMStateLabel fromState;
@property(nonatomic, readonly) OEFSMStateLabel nextState;
@property(nonatomic, readonly) OEFSMEventLabel event;
@property(nonatomic, readonly) void(^action)(void);

+ (instancetype)transitionWithFromState:(OEFSMStateLabel)fromState nextState:(OEFSMStateLabel)nextState event:(OEFSMEventLabel)event action:(void(^)(void))action;

@end

@interface OEFSMTimerTransition : NSObject

@property(nonatomic, readonly) OEFSMStateLabel fromState;
@property(nonatomic, readonly) OEFSMStateLabel nextState;
@property(nonatomic, readonly) NSTimeInterval delay;
@property(nonatomic, readonly) void(^action)(void);

+ (instancetype)timerTransitionWithFromState:(OEFSMStateLabel)fromState nextState:(OEFSMStateLabel)nextState delay:(NSTimeInterval)delay action:(void(^)(void))action;

@end

@implementation OEFiniteStateMachine

#pragma mark - Lifecycle

- (id)init
{
    self = [super init];
    if(!self) return nil;

    _states           = [NSMutableDictionary new];
    _transitions      = [NSMutableDictionary new];
    _timerTransitions = [NSMutableDictionary new];

    return self;
}

- (void)dealloc
{
    if(_timersQueue) [self OE_releaseTimersQueue];
}

#pragma mark - Main methods

- (void)addState:(OEFSMStateLabel)state entry:(void(^)(void))entry exit:(void(^)(void))exit
{
    OEFSMState *newState = [OEFSMState stateWithLabel:state entry:entry exit:exit];
    [_states setObject:newState forKey:@(state)];
}

- (void)addState:(OEFSMStateLabel)state
{
    [self addState:state entry:nil exit:nil];
}

- (void)addState:(OEFSMStateLabel)state entry:(void(^)(void))entry
{
    [self addState:state entry:entry exit:nil];
}

- (void)addState:(OEFSMStateLabel)state exit:(void(^)(void))exit
{
    [self addState:state entry:nil exit:exit];
}

- (void)addTransitionFrom:(OEFSMStateLabel)fromState to:(OEFSMStateLabel)nextState event:(OEFSMEventLabel)event action:(void(^)(void))action
{
    NSMutableDictionary *fromStateTransitions = [_transitions objectForKey:@(fromState)];
    if(!fromStateTransitions)
    {
        fromStateTransitions = [NSMutableDictionary new];
        [_transitions setObject:fromStateTransitions forKey:@(fromState)];
    }

    OEFSMTransition *newTransition = [OEFSMTransition transitionWithFromState:fromState nextState:nextState event:event action:action];
    [fromStateTransitions setObject:newTransition forKey:@(event)];
}

// Does it make sense to have multiple timer transitions leaving a given state? The first transition to be fired wipes out the previous timer transitions when leaving the state
- (void)setTimerTransitionFrom:(OEFSMStateLabel)fromState to:(OEFSMStateLabel)nextState delay:(NSTimeInterval)delay action:(void(^)(void))action
{
    NSMutableArray *fromStateTransitions = [_timerTransitions objectForKey:@(fromState)];
    if(!fromStateTransitions)
    {
        fromStateTransitions = [NSMutableArray new];
        [_timerTransitions setObject:fromStateTransitions forKey:@(fromState)];
    }

    OEFSMTimerTransition *newTransition = [OEFSMTimerTransition timerTransitionWithFromState:fromState nextState:nextState delay:delay action:action];
    [fromStateTransitions addObject:newTransition];
}

- (void)start
{
    NSAssert(!_running, @"OEFSMFiniteStateMachine cannot be started whilst running");

    OEFSMState *initialStateObject = [_states objectForKey:@(_initialState)];
    NSAssert(initialStateObject, @"Attempting to start OEFiniteStateMachine without a configured initial state");

    if(!_processingQueue)
    {
        char queueName[256];
        sprintf(queueName, "%s.%p", _OEFSMDefaultProcessingQueuePrefix, self);
        _processingQueue = dispatch_queue_create(queueName, DISPATCH_QUEUE_SERIAL);
    }

    _running      = YES;
    _currentState = _initialState;

    dispatch_async(_processingQueue, ^{
        [self OE_enterState:self->_initialState];
    });
}

- (void)processEvent:(OEFSMEventLabel)event
{
    dispatch_async(_processingQueue, ^{
        OEFSMTransition *transition = [[self->_transitions objectForKey:@(self->_currentState)] objectForKey:@(event)];
        if(transition)
        {
            [self OE_prepareToLeaveCurrentState];

            // Start the transition to the next state
            if([transition action]) dispatch_async(self->_actionsQueue, ^{
                [transition action]();
            });

            [self OE_enterState:[transition nextState]];
        }
    });
}

// Must be sent in the timers queue
- (void)OE_processTimerTransition:(OEFSMTimerTransition *)timerTransition
{
    OETimersQueueContext *timersQueueContext = dispatch_get_context(_timersQueue);
    if(timersQueueContext->cancelled) return;

    dispatch_async(_processingQueue, ^{
        if(self->_currentState == [timerTransition fromState])
        {
            [self OE_prepareToLeaveCurrentState];

            if([timerTransition action]) dispatch_async(self->_actionsQueue, ^{
                [timerTransition action]();
            });

            [self OE_enterState:[timerTransition nextState]];
        }
    });
}

// Must be sent in the processing queue
- (void)OE_prepareToLeaveCurrentState
{
    //NSAssert(dispatch_get_current_queue() == _processingQueue, @"This method must be executed in the processing queue only");

    OEFSMState *currentStateObject = [_states objectForKey:@(_currentState)];

    if(_timersQueue) [self OE_releaseTimersQueue];

    if([currentStateObject exit]) dispatch_async(_actionsQueue, ^{
        [currentStateObject exit]();
    });
}

// Must be sent in the processing queue
- (void)OE_enterState:(OEFSMStateLabel)nextState
{
    //NSAssert(dispatch_get_current_queue() == _processingQueue, @"This method must be executed in the processing queue only");

    OEFSMState *nextStateObject = [_states objectForKey:@(nextState)];
    if([nextStateObject entry]) dispatch_async(_actionsQueue, ^{
        [nextStateObject entry]();
    });

    // Fire timer transitions
    NSArray *timerTransitions = [_timerTransitions objectForKey:@(nextState)];

    // As is, if one state has multiple timer transitions to states different from the original one,
    // then only the first timer transition is executed
    if([timerTransitions count] > 0)
    {
        char queueName[256];
        sprintf(queueName, "%s.%p", _OEFSMDefaultTimersQueuePrefix, self);
        OETimersQueueContext *timersQueueContext = malloc(sizeof(OETimersQueueContext));
        timersQueueContext->cancelled = NO;

        _timersQueue = dispatch_queue_create(queueName, DISPATCH_QUEUE_SERIAL);
        dispatch_set_context(_timersQueue, timersQueueContext);
        dispatch_set_finalizer_f(_timersQueue, OE_timersQueueFinalizer);

        // silence analyzer: timersQueueContext is freed in OE_timersQueueFinalizer
#ifdef  __clang_analyzer__
        free(timersQueueContext);
#endif
    }
    for(OEFSMTimerTransition *timerTransition in timerTransitions)
    {
        dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, [timerTransition delay] * NSEC_PER_SEC);
        dispatch_after(popTime, _timersQueue, ^{
            [self OE_processTimerTransition:timerTransition];
        });
    }

    _currentState = nextState;
}

- (void)OE_releaseTimersQueue
{
    if(!_timersQueue) return;

    OETimersQueueContext *context = dispatch_get_context(_timersQueue);
    context->cancelled = YES;

    _timersQueue = NULL;
}

- (void)setInitialState:(OEFSMStateLabel)initialState
{
    NSAssert(!_running, @"OEFiniteStateMachine's initial state cannot be set when the machine is running");

    _initialState = initialState;
}

- (void)setActionsQueue:(dispatch_queue_t)actionsQueue
{
    if(actionsQueue != _actionsQueue)
    {
        _actionsQueue = actionsQueue;
    }
}

- (void)setStateDescriptions:(NSDictionary *)stateDescriptions
{
    _stateDescriptions = stateDescriptions;
}

- (void)setEventDescriptions:(NSDictionary *)eventDescriptions
{
    _eventDescriptions = eventDescriptions;
}

- (NSString *)descriptionForState:(OEFSMStateLabel)state
{
    NSString *s = ([_stateDescriptions objectForKey:@(state)] ? : [NSString stringWithFormat:@"%lu", (unsigned long)state]);
    s = [NSString stringWithFormat:@"\"%@\"", s];
    return s;
}

- (NSString *)descriptionForEvent:(OEFSMEventLabel)event
{
    NSString *s = ([_eventDescriptions objectForKey:@(event)] ? : [NSString stringWithFormat:@"%lu", (unsigned long)event]);
    s = [NSString stringWithFormat:@"\"%@\"", s];
    return s;
}

- (void)writeDOTRepresentationToURL:(NSURL *)URL
{
    NSMutableString *s = [NSMutableString new];

    [s appendString:@"/*\n"];
    [s appendString:@" * @command = neato\n"];
    [s appendString:@" */\n"];

    [s appendString:@"digraph FSM {\n"];

    [s appendString:@"\n\t// States\n"];
    [s appendString:@"\t\" \" [shape=none] // points to the initial state\n"];
    for(NSNumber *fromStateNumber in _states)
    {
        [s appendFormat:@"\t%@;\n",
         [self descriptionForState:[fromStateNumber unsignedIntegerValue]]];
    }

    [s appendString:@"\n\t// Transitions\n"];
    [s appendFormat:@"\t\" \" -> %@\n", [self descriptionForState:_initialState]];
    [_transitions enumerateKeysAndObjectsUsingBlock:^(NSNumber *fromStateNumber, NSDictionary *stateTransitions, BOOL *stop) {
        [stateTransitions enumerateKeysAndObjectsUsingBlock:^(NSNumber *eventNumber, OEFSMTransition *transition, BOOL *stop) {
            [s appendFormat:@"\t%@ -> %@ [label=%@];\n",
             [self descriptionForState:[fromStateNumber unsignedIntegerValue]],
             [self descriptionForState:[transition nextState]],
             [self descriptionForEvent:[transition event]]];
        }];
        [s appendString:@"\n"];
    }];

    [s appendString:@"\n\t// Timer Transitions\n"];
    [_timerTransitions enumerateKeysAndObjectsUsingBlock:^(NSNumber *fromStateNumber, NSArray *stateTimerTransitions, BOOL *stop) {
        for(OEFSMTimerTransition *timerTransition in stateTimerTransitions)
        {
            [s appendFormat:@"\t%@ -> %@ [style=dashed,label=\"%.2fs\"];\n",
             [self descriptionForState:[fromStateNumber unsignedIntegerValue]],
             [self descriptionForState:[timerTransition nextState]],
             [timerTransition delay]];
        }
    }];

    [s appendString:@"}\n"];

    [s writeToURL:URL atomically:YES encoding:NSUTF8StringEncoding error:NULL];
}

@end

@implementation OEFSMState

+ (instancetype)stateWithLabel:(OEFSMStateLabel)label entry:(void (^)(void))entry exit:(void (^)(void))exit
{
    OEFSMState *newState = [self new];
    newState->_label = label;
    newState->_entry = [entry copy];
    newState->_exit  = [exit copy];
    return newState;
}

@end

@implementation OEFSMTransition

+ (instancetype)transitionWithFromState:(OEFSMStateLabel)fromState nextState:(OEFSMStateLabel)nextState event:(OEFSMEventLabel)event action:(void(^)(void))action
{
    OEFSMTransition *newTransition = [self new];
    newTransition->_fromState = fromState;
    newTransition->_nextState = nextState;
    newTransition->_event     = event;
    newTransition->_action    = [action copy];
    return newTransition;
}

@end

@implementation OEFSMTimerTransition

+ (instancetype)timerTransitionWithFromState:(OEFSMStateLabel)fromState nextState:(OEFSMStateLabel)nextState delay:(NSTimeInterval)delay action:(void(^)(void))action
{
    OEFSMTimerTransition *newTransition = [self new];
    newTransition->_fromState = fromState;
    newTransition->_nextState = nextState;
    newTransition->_delay     = delay;
    newTransition->_action    = [action copy];
    return newTransition;
}

@end

#pragma mark - Private functions

void OE_timersQueueFinalizer(void *context)
{
    free((OETimersQueueContext *)context);
}
