//
//  INWindowButton.h
//
//  Copyright 2013-2014 Vladislav Alexeev. All rights reserved.
//
//  Licensed under the BSD 2-clause License. See LICENSE file distributed in the source
//  code of this project.
//

#import <Cocoa/Cocoa.h>

/**
 A concrete NSButton subclass that allows to mimic standard window title bar "traffic light" buttons
 and replace their graphics with custom ones.
 */
@interface INWindowButton : NSButton

/**
 The group identifier the receiver was initialized with.
 */
@property (nonatomic, copy, readonly) NSString *groupIdentifier;

/**
 An image for the normal state.
 */
@property (nonatomic, strong) NSImage *activeImage;

/**
 An image for the normal state, but displayed when receiver's window in not a key.
 */
@property (nonatomic, strong) NSImage *activeNotKeyWindowImage;

/**
 An image used in disabled state.
 */
@property (nonatomic, strong) NSImage *inactiveImage;

/**
 An image used when user hovers receiver with mouse pointer.
 */
@property (nonatomic, strong) NSImage *rolloverImage;

/**
 An image for the pressed state.
 */
@property (nonatomic, strong) NSImage *pressedImage;

/**
 @param size Designated size of the button. System size is 14x16 and you are considered to use it.
 @param groupIdentifier ID of the group which will apply rollover effect to its members.
 You may pass \c nil.
 @see initWithSize:groupIdentifier:
 */
+ (instancetype)windowButtonWithSize:(NSSize)size groupIdentifier:(NSString *)groupIdentifier;

/**
 @abstract Designated initializer.
 @discussion Initializes the receiver with the given size and includes it in the group with the
 given identifier.
 Groups are used to apply rollover effect to all buttons that are inside the same group.
 For example, close, minimize and zoom buttons should be inside the same group since they all get a
 rollover effect when the mouse pointer hovers over one of these buttons.
 @param size Designated size of the button. System size is 14x16 and you are considered to use it.
 @param groupIdentifier ID of the group which will apply rollover effect to its members.
 You may pass \c nil.
 */
- (instancetype)initWithSize:(NSSize)size groupIdentifier:(NSString *)groupIdentifier;

@end
