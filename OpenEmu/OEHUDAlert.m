/*
 Copyright (c) 2017, OpenEmu Team
 
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

#import "OEHUDAlert.h"

#import "OEButton.h"
#import "OETextField.h"
#import "OETextFieldCell.h"

#import "OEProgressIndicator.h"

#import "OEInputLimitFormatter.h"
#import "OEBackgroundImageView.h"

#import "OETheme.h"
#import "OEThemeImage.h"
#import "OEDownload.h"

#import "OpenEmu-Swift.h"

NS_ASSUME_NONNULL_BEGIN

static const CGFloat OEAlertTopInset                 = 16.0;
static const CGFloat OEAlertBottomInset              = 20.0;
static const CGFloat OEAlertLeadingInset             = 20.0;
static const CGFloat OEAlertTrailingInset            = 20.0;
static const CGFloat OEAlertMinimumWidth             = 420.0;
static const CGFloat OEAlertMaximumWidth             = 500.0;
       
static const CGFloat OEAlertImageLeadingInset        = 24.0;
static const CGFloat OEAlertImageWidth               = 64.0;
static const CGFloat OEAlertImageHeight              = 64.0;

static const CGFloat OEAlertHeadlineToMessageSpacing = 6.0;
static const CGFloat OEAlertProgressBarSpacing       = 12.0;
static const CGFloat OEAlertButtonSpacing            = 12.0;

static const CGFloat OEAlertMinimumButtonWidth       = 79.0;


@interface OEHUDAlert ()

- (void)OE_performCallback;
- (void)OE_createControls;
- (void)OE_layoutWindow;
- (void)OE_layoutWindowIfNeeded;

@property (copy, readonly) NSMutableArray <void(^)(void)> *blocks;

@end

@implementation OEHUDAlert {
    BOOL _needsRebuild;
}

@synthesize callbackHandler = _callbackHandler;

+ (OEHUDAlert *)alertWithMessageText:(nullable NSString *)msgText defaultButton:(nullable NSString *)defaultButtonLabel alternateButton:(nullable NSString *)alternateButtonLabel
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    
    alert.defaultButtonTitle = defaultButtonLabel;
    alert.alternateButtonTitle = alternateButtonLabel;
    alert.messageText = msgText;
    
    return alert;
}

#pragma mark - Memory Management

- (instancetype)init
{
    self = [super init];
    if(self)
    {
        _window = [[NSPanel alloc] initWithContentRect:NSMakeRect(0, 0, 0, 0) styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:YES];
        _window.releasedWhenClosed = NO;
        
        self.suppressOnDefaultReturnOnly = YES;
        [self OE_createControls];

        _blocks = [[NSMutableArray alloc] init];
    }
    
    return self;
}

#pragma mark -

- (NSModalResponse)runModal
{
    if(self.suppressionUDKey && [NSUserDefaults.standardUserDefaults valueForKey:self.suppressionUDKey])
    {
        NSInteger suppressionValue = [NSUserDefaults.standardUserDefaults integerForKey:self.suppressionUDKey];
        _result = (suppressionValue == 1 || self.suppressOnDefaultReturnOnly ? NSAlertFirstButtonReturn : NSAlertSecondButtonReturn);
        [self OE_performCallback];
        return _result;
    }

    [self OE_layoutWindow];
    
    _window.animationBehavior = NSWindowAnimationBehaviorAlertPanel;
    [_window makeKeyAndOrderFront:nil];
    
    void(^executeBlocks)(void) = ^{
        NSMutableArray *blocks = self.blocks;
        @synchronized(blocks)
        {
            while(blocks.count != 0)
            {
                void(^aBlock)(void) = blocks.firstObject;
                if(aBlock != nil)
                {
                    aBlock();
                }
                [blocks removeObjectAtIndex:0];
            }
        }
        [self OE_layoutWindowIfNeeded];
    };

    NSModalSession session = [NSApp beginModalSessionForWindow:_window];
    while([NSApp runModalSession:session] == NSModalResponseContinue)
    {
        [NSRunLoop.mainRunLoop runMode:NSDefaultRunLoopMode beforeDate:NSDate.distantFuture];
        executeBlocks();
        [NSRunLoop.currentRunLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];

    }
    executeBlocks();
   
    [NSApp endModalSession:session];

    [_window close];

    return _result;
}

- (void)performBlockInModalSession:(void(^)(void))block
{
    @synchronized(_blocks)
    {
        [_blocks addObject:block];
    }
}

- (void)closeWithResult:(NSInteger)res
{
    _result = res;
    [NSApp stopModalWithCode:_result];

    dispatch_async(dispatch_get_main_queue(), ^{
        [self->_window close];
        [self OE_performCallback];
    });
}

#pragma mark - Window Configuration

- (void)setTitle:(NSString *)title
{
    _window.title = title;
}

- (NSString *)title
{
    return _window.title;
}

#pragma mark - Progress Bar

- (void)setShowsProgressbar:(BOOL)showsProgressbar
{
    _showsProgressbar = showsProgressbar;
    _needsRebuild = YES;
}

- (double)progress
{
    return self.progressbar.doubleValue;
}

- (void)setProgress:(double)progress
{
    self.progressbar.indeterminate = progress == -1;
    self.progressbar.doubleValue = progress;
}

#pragma mark - Buttons

- (void)setDefaultButtonTitle:(nullable NSString *)defaultButtonTitle
{
    self.defaultButton.title = defaultButtonTitle ? : @"";
    _needsRebuild = YES;
}

- (NSString *)defaultButtonTitle
{
    return self.defaultButton.title;
}

- (void)setAlternateButtonTitle:(nullable NSString *)alternateButtonTitle
{
    self.alternateButton.title = alternateButtonTitle ? : @"";
    _needsRebuild = YES;
}

- (NSString *)alternateButtonTitle
{
    return self.alternateButton.title;
}

- (void)setOtherButtonTitle:(nullable NSString *)otherButtonTitle
{
    self.otherButton.title = otherButtonTitle ? : @"";
    _needsRebuild = YES;
}

- (NSString *)otherButtonTitle
{
    return self.otherButton.title;
}

- (void)buttonAction:(id)sender
{
    if(sender == self.defaultButton || sender == self.inputField)
        _result = NSAlertFirstButtonReturn;
    else if(sender == self.alternateButton)
        _result = NSAlertSecondButtonReturn;
    else if(sender == self.otherButton)
        _result = NSAlertThirdButtonReturn;
    else 
        _result = NSAlertFirstButtonReturn;

    if(_result != NSAlertThirdButtonReturn &&
       self.suppressionButton.state &&
       (_result == NSAlertFirstButtonReturn || !self.suppressOnDefaultReturnOnly)
       && self.suppressionUDKey)
    {
        NSInteger suppressionValue = (_result == NSAlertFirstButtonReturn ? 1 : 0);
        NSUserDefaults *standardUserDefaults = NSUserDefaults.standardUserDefaults;
        [standardUserDefaults setInteger:suppressionValue forKey:self.suppressionUDKey];
    }

    [NSApp stopModalWithCode:_result];
    [self OE_performCallback];
}

- (void)setDefaultButtonAction:(SEL)sel andTarget:(id)aTarget
{
    self.defaultButton.target = aTarget;
    self.defaultButton.action = sel;
}

- (void)setAlternateButtonAction:(SEL)sel andTarget:(id)aTarget
{
    self.alternateButton.target = aTarget;
    self.alternateButton.action = sel;
}

- (void)setOtherButtonAction:(SEL)sel andTarget:(id)aTarget
{
    self.alternateButton.target = aTarget;
    self.alternateButton.action = sel;
}

#pragma mark - Message Text

- (void)setHeadlineText:(nullable NSString *)headlineText
{
    self.headlineTextView.stringValue = headlineText ? : @"";
    _needsRebuild = YES;
}

- (NSString *)headlineText
{
    return self.headlineTextView.stringValue;
}

- (void)setMessageText:(nullable NSString *)messageText
{
    self.messageTextView.stringValue = messageText ? : @"";
    _needsRebuild = YES;
}

- (NSString *)messageText
{
    return self.messageTextView.stringValue;
}

#pragma mark - Callbacks

- (void)setCallbackHandler:(OEAlertCompletionHandler)handler
{
    _callbackHandler = [handler copy];
    
    self.alternateButton.target = self;
    self.alternateButton.action = @selector(buttonAction:);
    self.defaultButton.target = self;
    self.defaultButton.action = @selector(buttonAction:);
    self.otherButton.target = self;
    self.otherButton.action = @selector(buttonAction:);
}

- (OEAlertCompletionHandler)callbackHandler
{
    return _callbackHandler;
}

- (void)OE_performCallback
{   
    if(self.target != nil && self.callback != nil && [self.target respondsToSelector:self.callback])
    {
        [self.target performSelectorOnMainThread:self.callback withObject:self waitUntilDone:NO];
    }
    
    if(self.callbackHandler != nil)
    {
        _callbackHandler(self, self.result);
        [self callbackHandler];
        _callbackHandler = nil;
    }
}

#pragma mark - Suppression Button

- (void)setShowsSuppressionButton:(BOOL)showsSuppressionButton
{
    _showsSuppressionButton = showsSuppressionButton;
    _needsRebuild = YES;
}

- (void)showSuppressionButtonForUDKey:(NSString *)key
{
    self.showsSuppressionButton = YES;
    self.suppressionUDKey = key;
    
    NSUserDefaults *standardUserDefaults = NSUserDefaults.standardUserDefaults;
    BOOL checked = [standardUserDefaults valueForKey:self.suppressionUDKey] != nil;
    self.suppressionButton.state = checked;
}

- (void)setSuppressionLabelText:(NSString *)suppressionLabelText
{
    self.suppressionButton.title = suppressionLabelText;
}

- (NSString *)suppressionLabelText
{
    return self.suppressionButton.title;
}

- (void)suppressionButtonAction:(id)sender
{
    if(![self suppressionUDKey])
        return;
    
    NSInteger state = [(NSButton *)sender state];
    NSUserDefaults *standardUserDefaults = NSUserDefaults.standardUserDefaults;
    if(!state && self.suppressOnDefaultReturnOnly)
        [standardUserDefaults removeObjectForKey:self.suppressionUDKey];
}

#pragma mark - Input Field

- (void)setShowsInputField:(BOOL)showsInputField
{
    _showsInputField = showsInputField;
    _needsRebuild = YES;
}

- (void)setShowsOtherInputField:(BOOL)showsOtherInputField
{
    _showsOtherInputField = showsOtherInputField;
    _needsRebuild = YES;
}

- (void)setStringValue:(NSString *)stringValue
{
    self.inputField.stringValue = stringValue;
}

- (NSString *)stringValue
{
    return self.inputField.stringValue;
}

- (void)setOtherStringValue:(NSString *)otherStringValue
{
    self.otherInputField.stringValue = otherStringValue;
}

- (NSString *)otherStringValue
{
    return self.otherInputField.stringValue;
}

- (void)setInputLabelText:(NSString *)inputLabelText
{
    self.inputLabelView.stringValue = inputLabelText;
}

- (NSString *)inputLabelText
{
    return self.inputLabelView.stringValue;
}

- (void)setOtherInputLabelText:(NSString *)otherInputLabelText
{
    self.otherInputLabelView.stringValue = otherInputLabelText;
}

- (NSString *)otherInputLabelText
{
    return self.otherInputLabelView.stringValue;
}

- (NSInteger)inputLimit
{
    OEInputLimitFormatter *formatter = self.inputField.formatter;
    
    return [formatter isKindOfClass:[OEInputLimitFormatter class]] ? formatter.limit : 0;
}

- (void)setInputLimit:(NSInteger)inputLimit
{
    OEInputLimitFormatter *formatter = self.inputField.formatter;
    
    BOOL limitFormatterSet = (formatter != nil && [formatter isKindOfClass:[OEInputLimitFormatter class]]);
    
    if(inputLimit == 0 && limitFormatterSet)
        self.inputField.formatter = nil;
    else if(limitFormatterSet)
        formatter.limit = inputLimit;
    else 
    {
        formatter = [[OEInputLimitFormatter alloc] initWithLimit:inputLimit];
        self.inputField.formatter = formatter;
    }
}

#pragma mark - Private Methods

- (void)OE_createControls
{
    // Setup Button
    _defaultButton = [NSButton buttonWithTitle:@"" target:nil action:nil];
    self.defaultButton.translatesAutoresizingMaskIntoConstraints = NO;
    [self.defaultButton setTarget:self andAction:@selector(buttonAction:)];
    self.defaultButton.keyEquivalent = @"\r";
    self.defaultButton.title = @"";
    
    _alternateButton = [NSButton buttonWithTitle:@"" target:nil action:nil];
    self.alternateButton.translatesAutoresizingMaskIntoConstraints = NO;
    [self.alternateButton setTarget:self andAction:@selector(buttonAction:)];
    self.alternateButton.keyEquivalent = @"\E";
    self.alternateButton.title = @"";
    
    _otherButton = [NSButton buttonWithTitle:@"" target:nil action:nil];
    self.otherButton.translatesAutoresizingMaskIntoConstraints = NO;
    [self.otherButton setTarget:self andAction:@selector(buttonAction:)];
    self.otherButton.keyEquivalent = @"";
    self.otherButton.title = @"";

    // Setup Headline Text View
    _headlineTextView = [NSTextField wrappingLabelWithString:@""];
    self.headlineTextView.translatesAutoresizingMaskIntoConstraints = NO;
    self.headlineTextView.editable = NO;
    self.headlineTextView.selectable = YES;
    self.headlineTextView.font = [NSFont systemFontOfSize:NSFont.systemFontSize weight:NSFontWeightBold];

    // Setup Message Text View
    _messageTextView = [NSTextField wrappingLabelWithString:@""];
    self.messageTextView.translatesAutoresizingMaskIntoConstraints = NO;
    self.messageTextView.editable = NO;
    self.messageTextView.selectable = YES;
    
    // Setup Input Field
    _inputField = [NSTextField textFieldWithString:@""];
    self.inputField.translatesAutoresizingMaskIntoConstraints = NO;
    self.inputField.usesSingleLineMode = YES;
    //[self.inputField setTarget:self andAction:@selector(buttonAction:)];
    self.inputField.editable = YES;
    
    _inputLabelView = [NSTextField labelWithString:@""];
    self.inputLabelView.translatesAutoresizingMaskIntoConstraints = NO;
    self.inputLabelView.editable = NO;
    self.inputLabelView.selectable = NO;
    
    // Setup Other Input Field
    _otherInputField = [NSTextField textFieldWithString:@""];
    self.otherInputField.translatesAutoresizingMaskIntoConstraints = NO;
    self.otherInputField.usesSingleLineMode = YES;
    //[self.otherInputField setTarget:self andAction:@selector(buttonAction:)];
    self.otherInputField.editable = YES;

    _otherInputLabelView = [NSTextField labelWithString:@""];
    self.otherInputLabelView.translatesAutoresizingMaskIntoConstraints = NO;
    self.otherInputLabelView.editable = NO;
    self.otherInputLabelView.selectable = NO;
    
    // Setup Progressbar
    _progressbar = [[NSProgressIndicator alloc] init];
    _progressbar.translatesAutoresizingMaskIntoConstraints = NO;
    _progressbar.minValue = 0.0;
    _progressbar.maxValue = 1.0;
    
    // Setup Suppression Button
    _suppressionButton = [NSButton checkboxWithTitle:@"" target:nil action:nil];
    self.suppressionButton.translatesAutoresizingMaskIntoConstraints = NO;
    self.suppressionButton.buttonType = NSButtonTypeSwitch;
    self.suppressionButton.title = NSLocalizedString(@"Do not ask me again", @"");
    [self.suppressionButton setTarget:self andAction:@selector(suppressionButtonAction:)];
}

- (void)OE_layoutWindowIfNeeded
{
    if (_needsRebuild)
        [self OE_layoutWindow];
}

- (void)OE_layoutWindow
{
    NSView *contentView = self.window.contentView;
    contentView.subviews = @[];
    contentView.translatesAutoresizingMaskIntoConstraints = NO;

    NSLayoutAnchor *lastAnchor = contentView.topAnchor;
    NSLayoutAnchor *effectiveLeftBorderAnchor = contentView.leadingAnchor;
    NSImageView *image;
    
    /* create a NSAlert-style decoration image only if at least one of the messages is displayed. */
    if (self.headlineText.length != 0 || self.messageText.length != 0) {
        image = [NSImageView imageViewWithImage:[NSImage imageNamed:NSImageNameApplicationIcon]];
        image.translatesAutoresizingMaskIntoConstraints = NO;
        image.imageFrameStyle = NSImageFrameNone;
        image.imageScaling = NSImageScaleProportionallyUpOrDown;
        [contentView addSubview:image];
        [contentView addConstraints:@[
            [image.topAnchor constraintEqualToAnchor:lastAnchor constant:OEAlertTopInset],
            [image.leadingAnchor constraintEqualToAnchor:contentView.leadingAnchor constant:OEAlertImageLeadingInset],
            [image.heightAnchor constraintEqualToConstant:OEAlertImageWidth],
            [image.widthAnchor constraintEqualToConstant:OEAlertImageHeight]]];
        effectiveLeftBorderAnchor = image.trailingAnchor;
    }
    
    lastAnchor = [self OE_layoutHeadlineUnderAnchor:lastAnchor leadingAnchor:effectiveLeftBorderAnchor];
    lastAnchor = [self OE_layoutMessageUnderAnchor:lastAnchor leadingAnchor:effectiveLeftBorderAnchor];
    lastAnchor = [self OE_layoutProgressBarUnderAnchor:lastAnchor leadingAnchor:effectiveLeftBorderAnchor];
    
    /* add padding under the previous views in case they have less height than
     * the decoration image */
    if (image) {
        NSView *dummyView = [[NSView alloc] init];
        dummyView.translatesAutoresizingMaskIntoConstraints = NO;
        [contentView addSubview:dummyView];
        [contentView addConstraints:@[
            [dummyView.topAnchor constraintEqualToAnchor:lastAnchor],
            [dummyView.bottomAnchor constraintGreaterThanOrEqualToAnchor:image.bottomAnchor constant:0],
            [dummyView.widthAnchor constraintEqualToConstant:0],
            [dummyView.leftAnchor constraintEqualToAnchor:contentView.leftAnchor]]];
        lastAnchor = dummyView.bottomAnchor;
    }
    effectiveLeftBorderAnchor = contentView.leadingAnchor;
    
    lastAnchor = [self OE_layoutInputFieldsUnderAnchor:lastAnchor leadingAnchor:effectiveLeftBorderAnchor];
    lastAnchor = [self OE_layoutButtonsUnderAnchor:lastAnchor leadingAnchor:effectiveLeftBorderAnchor];
    
    NSLayoutConstraint *maxWidthConstraint = [contentView.widthAnchor constraintLessThanOrEqualToConstant:OEAlertMaximumWidth];
    maxWidthConstraint.priority = NSLayoutPriorityDefaultHigh-1;
    [contentView addConstraints:@[
        [contentView.bottomAnchor constraintEqualToAnchor:lastAnchor constant:OEAlertBottomInset],
        [contentView.widthAnchor constraintGreaterThanOrEqualToConstant:OEAlertMinimumWidth],
        maxWidthConstraint]];
        
    /* Set preferredMaxLayoutWidth on the message and the headline text fields.
     * In this way, in case the text wraps, the width of the label will be
     * exactly equal to the bounding box of the text itself.  */
    if (contentView.fittingSize.width <= OEAlertMaximumWidth) {
        CGFloat maxTextWidth = OEAlertMaximumWidth - OEAlertTrailingInset - OEAlertLeadingInset - OEAlertImageWidth - OEAlertImageLeadingInset;
        self.messageTextView.preferredMaxLayoutWidth = maxTextWidth;
        self.headlineTextView.preferredMaxLayoutWidth = maxTextWidth;
    } else {
        self.messageTextView.preferredMaxLayoutWidth = 0;
        self.headlineTextView.preferredMaxLayoutWidth = 0;
    }
    
    [self.window setContentSize:contentView.fittingSize];
    [self.window center];
    
    _needsRebuild = NO;
}

- (NSLayoutAnchor *)OE_layoutHeadlineUnderAnchor:(NSLayoutAnchor *)lastAnchor leadingAnchor:(NSLayoutAnchor *)effectiveLeftBorderAnchor
{
    if (self.headlineText.length == 0)
        return lastAnchor;
    NSView *contentView = self.window.contentView;
    
    [self.headlineTextView setContentHuggingPriority:NSLayoutPriorityDefaultHigh+1 forOrientation:NSLayoutConstraintOrientationVertical];
    [contentView addSubview:self.headlineTextView];
    [contentView addConstraints:@[
        [self.headlineTextView.topAnchor constraintEqualToAnchor:lastAnchor constant:OEAlertTopInset],
        [self.headlineTextView.leadingAnchor constraintEqualToAnchor:effectiveLeftBorderAnchor constant:OEAlertLeadingInset],
        [contentView.trailingAnchor constraintGreaterThanOrEqualToAnchor:self.headlineTextView.trailingAnchor constant:OEAlertTrailingInset]]];
    return self.headlineTextView.bottomAnchor;
}

- (NSLayoutAnchor *)OE_layoutMessageUnderAnchor:(NSLayoutAnchor *)lastAnchor leadingAnchor:(NSLayoutAnchor *)effectiveLeftBorderAnchor
{
    if (self.messageText.length == 0)
        return lastAnchor;
    NSView *contentView = self.window.contentView;
    BOOL hasHeadline = self.headlineText.length != 0;
    
    if (self.headlineText.length == 0) {
        self.messageTextView.font = [NSFont systemFontOfSize:NSFont.systemFontSize];
    } else {
        self.messageTextView.font = [NSFont systemFontOfSize:NSFont.smallSystemFontSize];
    }
    [contentView addSubview:self.messageTextView];
    [contentView addConstraints:@[
        [self.messageTextView.topAnchor constraintEqualToAnchor:lastAnchor constant:hasHeadline ? OEAlertHeadlineToMessageSpacing : OEAlertTopInset],
        [self.messageTextView.leadingAnchor constraintEqualToAnchor:effectiveLeftBorderAnchor constant:OEAlertLeadingInset],
        [contentView.trailingAnchor constraintGreaterThanOrEqualToAnchor:self.messageTextView.trailingAnchor constant:OEAlertTrailingInset]]];
    return self.messageTextView.bottomAnchor;
}

- (NSLayoutAnchor *)OE_layoutProgressBarUnderAnchor:(NSLayoutAnchor *)lastAnchor leadingAnchor:(NSLayoutAnchor *)effectiveLeftBorderAnchor
{
    if (!self.showsProgressbar)
        return lastAnchor;
    NSView *contentView = self.window.contentView;
    
    [contentView addSubview:self.progressbar];
    [contentView addConstraints:@[
        [self.progressbar.topAnchor constraintEqualToAnchor:lastAnchor constant:OEAlertProgressBarSpacing],
        [self.progressbar.leadingAnchor constraintEqualToAnchor:effectiveLeftBorderAnchor constant:OEAlertLeadingInset],
        [contentView.trailingAnchor constraintEqualToAnchor:self.progressbar.trailingAnchor constant:OEAlertTrailingInset]]];
    return self.progressbar.bottomAnchor;
}

- (NSLayoutAnchor *)OE_layoutInputFieldsUnderAnchor:(NSLayoutAnchor *)lastAnchor leadingAnchor:(NSLayoutAnchor *)effectiveLeftBorderAnchor
{
    if (!self.showsInputField && !self.showsOtherInputField)
        return lastAnchor;
    NSView *contentView = self.window.contentView;
    
    NSGridView *inputGrid = [NSGridView gridViewWithNumberOfColumns:2 rows:0];
    inputGrid.translatesAutoresizingMaskIntoConstraints = NO;
    inputGrid.rowAlignment = NSGridRowAlignmentLastBaseline;
    [inputGrid columnAtIndex:0].xPlacement = NSGridCellPlacementTrailing;
    [contentView addSubview:inputGrid];
    [contentView addConstraints:@[
        [inputGrid.topAnchor constraintEqualToAnchor:lastAnchor constant:OEAlertTopInset],
        [inputGrid.leadingAnchor constraintEqualToAnchor:effectiveLeftBorderAnchor constant:OEAlertLeadingInset],
        [contentView.trailingAnchor constraintEqualToAnchor:inputGrid.trailingAnchor constant:OEAlertTrailingInset]]];
    
    if (self.showsInputField) {
        [inputGrid addRowWithViews:@[self.inputLabelView, self.inputField]];
    }
    if (self.showsOtherInputField) {
        [inputGrid addRowWithViews:@[self.otherInputLabelView, self.otherInputField]];
    }
    
    return inputGrid.bottomAnchor;
}

- (NSLayoutAnchor *)OE_layoutButtonsUnderAnchor:(NSLayoutAnchor *)lastAnchor leadingAnchor:(NSLayoutAnchor *)effectiveLeftBorderAnchor
{
    NSView *contentView = self.window.contentView;
    
    NSStackView *buttonStackView = [[NSStackView alloc] init];
    buttonStackView.orientation = NSUserInterfaceLayoutOrientationHorizontal;
    buttonStackView.translatesAutoresizingMaskIntoConstraints = NO;
    buttonStackView.alignment = NSLayoutAttributeLastBaseline;
    [contentView addSubview:buttonStackView];
    [contentView addConstraints:@[
        [buttonStackView.topAnchor constraintEqualToAnchor:lastAnchor constant:OEAlertButtonSpacing],
        [buttonStackView.leadingAnchor constraintEqualToAnchor:contentView.leadingAnchor constant:OEAlertLeadingInset],
        [contentView.trailingAnchor constraintEqualToAnchor:buttonStackView.trailingAnchor constant:OEAlertTrailingInset]]];
        
    if (self.alternateButtonTitle.length != 0) {
        [buttonStackView addView:self.alternateButton inGravity:NSStackViewGravityTrailing];
        [contentView addConstraint:[self.alternateButton.widthAnchor constraintGreaterThanOrEqualToConstant:OEAlertMinimumButtonWidth]];
    }
    if (self.defaultButtonTitle.length != 0) {
        [buttonStackView addView:self.defaultButton inGravity:NSStackViewGravityTrailing];
        [contentView addConstraint:[self.defaultButton.widthAnchor constraintGreaterThanOrEqualToConstant:OEAlertMinimumButtonWidth]];
    }
    if (self.showsSuppressionButton) {
        [buttonStackView addView:self.suppressionButton inGravity:NSStackViewGravityLeading];
    }
    if (self.otherButtonTitle.length != 0) {
        [buttonStackView addView:self.otherButton inGravity:NSStackViewGravityLeading];
        [contentView addConstraint:[self.otherButton.widthAnchor constraintGreaterThanOrEqualToConstant:OEAlertMinimumButtonWidth]];
    }
    
    return buttonStackView.bottomAnchor;
}

@end

NS_ASSUME_NONNULL_END
