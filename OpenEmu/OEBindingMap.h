//
//  OEBindingMap.h
//  OpenEmu
//
//  Created by Remy Demarest on 18/11/2012.
//
//

#import <Foundation/Foundation.h>

@class OEHIDEvent;
@class OESystemKey;
@class OESystemController;

@interface OEBindingMap : NSObject

- (id)initWithCapacity:(NSUInteger)totalNumberOfKeys;
// Provides a hint for the number of keys
- (id)initWithSystemController:(OESystemController *)aController;

- (OESystemKey *)systemKeyForEvent:(OEHIDEvent *)anEvent;
- (void)setSystemKey:(OESystemKey *)aKey forEvent:(OEHIDEvent *)anEvent;
- (void)removeSystemKeyForEvent:(OEHIDEvent *)anEvent;

@end

@interface OESystemKey : NSObject
+ (instancetype)systemKeyWithKey:(NSUInteger)aKeyNumber player:(NSUInteger)playerNumber;
- (id)initWithKey:(NSUInteger)aKeyNumber player:(NSUInteger)playerNumber;
@property(nonatomic) NSUInteger key;
@property(nonatomic) NSUInteger player;
@end
