//
//  OEControlDescription.h
//  OpenEmu
//
//  Created by Remy Demarest on 06/01/2013.
//
//

#import <Foundation/Foundation.h>
#import "OEHIDEvent.h"

@class OEControllerDescription;

@interface OEControlDescription : NSObject

@property(readonly) OEControllerDescription *controllerDescription;

@property(readonly) OEHIDEventType  type;
@property(readonly) NSString       *name;
@property(readonly) NSString       *identifier;
@property(readonly) NSArray        *controlValues;
@property(readonly) BOOL            isGenericControl;

@end

@interface OEControlValueDescription : NSObject

@property(readonly) OEControlDescription *controlDescription;

@property(readonly) NSString   *name;
@property(readonly) NSString   *identifier;
@property(readonly) NSNumber   *valueIdentifier;
@property(readonly) id          representationIdentifier;
@property(readonly) OEHIDEvent *event;

- (OEControlValueDescription *)associatedControlValueDescriptionForEvent:(OEHIDEvent *)anEvent;

@end
