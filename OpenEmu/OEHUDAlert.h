//
//  OEAlert.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 14.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
@class OEHUDAlert;
@class OECheckBox;
@class OEHUDProgressbar;
@class OEPrefBoxPlain;
typedef void (^OEAlertCompletionHandler)(OEHUDAlert* alert, NSUInteger result);
@interface OEHUDAlert : NSObject
{
    NSWindow* _window;
    NSUInteger result;
    
    OEAlertCompletionHandler callbackHandler;
}

+ (id)saveGameAlertWithProposedName:(NSString*)name;
+ (id)autoSaveGameAlert;
+ (id)deleteGameAlertWithStateName:(NSString*)stateName;

+ (id)alertWithError:(NSError*)error;
+ (id)alertWithMessageText:(NSString *)msgText defaultButton:(NSString*)defaultButtonLabel alternateButton:(NSString*)alternateButtonLabel;
#pragma mark -
- (NSUInteger)runModal;
- (void)closeWithResult:(NSInteger)res;
@property (readonly) NSUInteger result;

#pragma mark -
#pragma mark Sizing
@property float height, width;

@property (readonly, retain) OEPrefBoxPlain* boxView;
#pragma mark -
#pragma mark Buttons
@property (readonly, retain) NSButton *defaultButton;
@property (readonly, retain) NSButton *alternateButton;

@property (readonly, retain) NSTextView* messageTextView;
@property (readonly, retain) NSTextField* headlineLabelField;

@property (assign) NSString* stringValue, *inputLabelText;
@property (assign) NSString *defaultButtonTitle, *alternateButtonTitle, *title, *messageText, *headlineLabelText;

#pragma mark -
#pragma mark Input Field
@property (readonly, retain) NSTextField *inputField, * inputLabelField;
@property BOOL showsInputField;

#pragma mark -
#pragma mark Progress Bar
@property BOOL showsProgressbar;
@property (readonly) OEHUDProgressbar *progressbar;
- (void)setProgress:(float)progress;
- (float)progress;

#pragma mark -
#pragma mark Button Actions
- (void)setDefaultButtonAction:(SEL)sel andTarget:(id)target;
- (void)setAlternateButtonAction:(SEL)sel andTarget:(id)target;

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
@property (copy) NSString* suppressionLabelText; // default is "Do not ask me again", can be changed (e.g. if alert is too small)
@end




