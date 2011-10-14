//
//  OEAlert.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 14.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
@class OEAlert;
typedef void (^OEAlertCompletionHandler)(OEAlert* alert, NSUInteger result);
@class OECheckBox;
@interface OEAlert : NSObject
{
    NSWindow* _window;
    
    NSString* _messageText;
    
    NSUInteger result;
    
    // Buttons
    NSButton *_defaultButton, *_alternateButton;
    NSString *defaultButtonTitle, *alternateButtonTitle;
    
    // Suppression Button
    BOOL showsSuppressionButton;
    OECheckBox *_suppressionButton;
    NSString *suppressionUDKey;
    
    
    // Callbacks
    id target;
    SEL callback;
    
    OEAlertCompletionHandler callbackHandler;
}

+ (id)alertWithError:(NSError*)error;
+ (id)alertWithMessageText:(NSString *)msgText defaultButton:(NSString*)defaultButtonLabel alternateButton:(NSString*)alternateButtonLabel;
#pragma mark -
- (NSUInteger)runModal;
@property (readonly) NSUInteger result;

#pragma mark -
#pragma mark Buttons
@property (readonly) NSButton *defaultButton;
@property (readonly) NSButton *alternateButton;
@property (copy) NSString *defaultButtonTitle, *alternateButtonTitle;

#pragma mark -
#pragma mark Callbacks
@property (assign) id target;
@property SEL callback;
@property (copy) OEAlertCompletionHandler callbackHandler;

#pragma mark -
#pragma mark Suppression Button
- (void)showSuppressionButtonForUDKey:(NSString*)key;
@property BOOL showsSuppressionButton;
@property (readonly) NSButton* suppressionButton;
@property (copy) NSString* suppressionUDKey;
@end




