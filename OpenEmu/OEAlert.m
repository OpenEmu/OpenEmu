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

#import "OEAlert.h"
#import "OEInputLimitFormatter.h"
#import "OpenEmu-Swift.h"

NS_ASSUME_NONNULL_BEGIN

static const CGFloat OEAlertTopInset                 = 16.0;
static const CGFloat OEAlertBottomInset              = 18.0;
static const CGFloat OEAlertLeadingInset             = 20.0;
static const CGFloat OEAlertTrailingInset            = 20.0;
static const CGFloat OEAlertMinimumWidth             = 420.0;
static const CGFloat OEAlertMaximumWidth             = 500.0;
       
static const CGFloat OEAlertImageLeadingInset        = 24.0;
static const CGFloat OEAlertImageWidth               = 64.0;
static const CGFloat OEAlertImageHeight              = 64.0;

static const CGFloat OEAlertHeadlineToMessageSpacing = 6.0;
static const CGFloat OEAlertProgressBarSpacing       = 12.0;
static const CGFloat OEAlertButtonTopSpacing         = 10.0;
static const CGFloat OEAlertOtherButtonSpacing       = 42.0;
static const CGFloat OEAlertInterButtonSpacing       = 14.0;

static const CGFloat OEAlertMinimumButtonWidth       = 79.0;


@interface OEAlert ()

- (void)OE_performCallback;
- (void)OE_createControls;
- (void)OE_layoutWindow;
- (void)OE_layoutWindowIfNeeded;

@property (copy, readonly) NSMutableArray <void(^)(void)> *blocks;

@property (nonatomic) BOOL sheetMode;

// dialog buttons
@property (nonatomic, readonly) NSButton *defaultButton;
@property (nonatomic, readonly) NSButton *alternateButton;
@property (nonatomic, readonly) NSButton *otherButton;
// touch bar buttons
@property (nonatomic, readonly) NSButton *defaultTBButton;
@property (nonatomic, readonly) NSButton *alternateTBButton;
@property (nonatomic, readonly) NSButton *otherTBButton;

@property (nonatomic, readonly) NSTextField *messageLabel;
@property (nonatomic, readonly) NSTextField *headlineLabel;

@property (nonatomic, readonly) NSTextField *inputLabel;
@property (nonatomic, readonly) NSTextField *otherInputLabel;
@property (nonatomic, readonly) NSTextField *inputField;
@property (nonatomic, readonly) NSTextField *otherInputField;

@property (nonatomic, readonly) NSProgressIndicator *progressbar;

@property (nonatomic, readonly) NSButton *suppressionButton;

@end

@implementation OEAlert {
    BOOL _needsRebuild;
}

@synthesize callbackHandler = _callbackHandler;

- (void)addButtonWithTitle:(NSString *)title
{
    if([self.defaultButtonTitle isEqual:@""]) {
        self.defaultButtonTitle = title;
    }
    else if([self.alternateButtonTitle isEqual:@""]) {
        self.alternateButtonTitle = title;
    }
    else if([self.otherButtonTitle isEqual:@""]) {
        self.otherButtonTitle = title;
    }
}

#pragma mark - Memory Management

- (instancetype)init
{
    self = [super init];
    if(self)
    {
        _window = [[NSPanel alloc] initWithContentRect:NSMakeRect(0, 0, 0, 0) styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:YES];
        _window.releasedWhenClosed = NO;
        
        if (@available(macOS 11.0, *)) {
            _window.titlebarAppearsTransparent = YES;
            _window.titleVisibility = NSWindowTitleHidden;
            _window.styleMask |= NSWindowStyleMaskFullSizeContentView;
            _window.movableByWindowBackground = YES;
            
            NSVisualEffectView *veView = [NSVisualEffectView new];
            veView.material = NSVisualEffectMaterialPopover;
            _window.contentView = veView;
        }
        
        self.suppressOnDefaultReturnOnly = YES;
        [self OE_createControls];
        [self OE_createTouchBarControls];
        _needsRebuild = YES;

        _blocks = [[NSMutableArray alloc] init];
    }
    
    return self;
}

#pragma mark -

- (NSModalResponse)runModal
{
    self.sheetMode = false;
    
    NSModalResponse suppressedResp = [self OE_checkIfSuppressed];
    if (suppressedResp != NSModalResponseContinue)
        return suppressedResp;
    
    [self OE_layoutWindowIfNeeded];
    
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
    if (self.sheetMode) {
        block();
        return;
    }
        
    @synchronized(_blocks)
    {
        [_blocks addObject:block];
    }
}

- (void)closeWithResult:(NSInteger)res
{
    _result = res;
    
    if (self.sheetMode) {
        [_window.sheetParent endSheet:_window returnCode:res];
        return;
    }
    
    [NSApp stopModalWithCode:_result];

    dispatch_async(dispatch_get_main_queue(), ^{
        [self->_window close];
        [self OE_performCallback];
    });
}

- (void)beginSheetModalForWindow:(NSWindow *)sheetWindow completionHandler:(void (^ _Nullable)(NSModalResponse returnCode))handler
{
    self.sheetMode = YES;
    
    NSModalResponse suppressedResp = [self OE_checkIfSuppressed];
    if (suppressedResp != NSModalResponseContinue) {
        handler(suppressedResp);
        return;
    }
    
    [self OE_layoutWindowIfNeeded];
    _window.animationBehavior = NSWindowAnimationBehaviorAlertPanel;
    [sheetWindow beginSheet:_window completionHandler:^(NSModalResponse resp) {
        self->_result = resp;
        handler(resp);
        [self OE_performCallback];
    }];
}

- (NSModalResponse)OE_checkIfSuppressed
{
    if(self.suppressionUDKey && [NSUserDefaults.standardUserDefaults valueForKey:self.suppressionUDKey])
    {
        NSInteger suppressionValue = [NSUserDefaults.standardUserDefaults integerForKey:self.suppressionUDKey];
        _result = (suppressionValue == 1 || self.suppressOnDefaultReturnOnly ? NSAlertFirstButtonReturn : NSAlertSecondButtonReturn);
        [self OE_performCallback];
        return _result;
    }
    
    return NSModalResponseContinue;
}

- (void)OE_stopModal
{
    if (self.sheetMode) {
        [_window.sheetParent endSheet:_window returnCode:_result];
    } else {
        [NSApp stopModalWithCode:_result];
        [self OE_performCallback];
    }
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
    [self.progressbar stopAnimation:nil];
    if (progress < 0.0 || progress > 1.0) {
        self.progressbar.indeterminate = YES;
        [self.progressbar startAnimation:nil];
    } else {
        self.progressbar.indeterminate = NO;
        self.progressbar.doubleValue = progress;
    }
}

#pragma mark - Buttons

- (void)setDefaultButtonTitle:(nullable NSString *)defaultButtonTitle
{
    self.defaultButton.title = defaultButtonTitle ? : @"";
    self.defaultTBButton.title = self.defaultButton.title;
    _needsRebuild = YES;
}

- (NSString *)defaultButtonTitle
{
    return self.defaultButton.title;
}

- (void)setAlternateButtonTitle:(nullable NSString *)alternateButtonTitle
{
    self.alternateButton.title = alternateButtonTitle ? : @"";
    self.alternateTBButton.title = self.alternateButton.title;
    _needsRebuild = YES;
}

- (NSString *)alternateButtonTitle
{
    return self.alternateButton.title;
}

- (void)setOtherButtonTitle:(nullable NSString *)otherButtonTitle
{
    self.otherButton.title = otherButtonTitle ? : @"";
    self.otherTBButton.title = self.otherButton.title;
    _needsRebuild = YES;
}

- (NSString *)otherButtonTitle
{
    return self.otherButton.title;
}

- (void)buttonAction:(id)sender
{
    if(sender == self.defaultButton || sender == self.defaultTBButton)
        _result = NSAlertFirstButtonReturn;
    else if(sender == self.alternateButton || sender == self.alternateTBButton)
        _result = NSAlertSecondButtonReturn;
    else if(sender == self.otherButton || sender == self.otherTBButton)
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

    [self OE_stopModal];
}

- (void)setDefaultButtonAction:(nullable SEL)sel andTarget:(nullable id)aTarget
{
    self.defaultButton.target = aTarget;
    self.defaultButton.action = sel;
    self.defaultButton.enabled = sel != NULL;
    self.defaultTBButton.target = aTarget;
    self.defaultTBButton.action = sel;
    self.defaultTBButton.enabled = sel != NULL;
}

- (void)setAlternateButtonAction:(nullable SEL)sel andTarget:(nullable id)aTarget
{
    self.alternateButton.target = aTarget;
    self.alternateButton.action = sel;
    self.alternateButton.enabled = sel != NULL;
    self.alternateTBButton.target = aTarget;
    self.alternateTBButton.action = sel;
    self.alternateTBButton.enabled = sel != NULL;
}

- (void)setOtherButtonAction:(nullable SEL)sel andTarget:(nullable id)aTarget
{
    self.otherButton.target = aTarget;
    self.otherButton.action = sel;
    self.otherButton.enabled = sel != NULL;
    self.otherTBButton.target = aTarget;
    self.otherTBButton.action = sel;
    self.otherButton.enabled = sel != NULL;
}

#pragma mark - Message Text

- (void)setMessageText:(nullable NSString *)messageText
{
    self.headlineLabel.stringValue = messageText ? : @"";
    _needsRebuild = YES;
}

- (NSString *)messageText
{
    return self.headlineLabel.stringValue;
}

- (void)setMessageUsesHTML:(BOOL)messageUsesHTML
{
    _messageUsesHTML = messageUsesHTML;
    _needsRebuild = YES;
}

- (void)setInformativeText:(nullable NSString *)informativeText
{
    _informativeText = informativeText ? : @"";
    _needsRebuild = YES;
}

#pragma mark - Callbacks

- (void)setCallbackHandler:(OEAlertCompletionHandler)handler
{
    _callbackHandler = [handler copy];
    
    [self setAlternateButtonAction:@selector(buttonAction:) andTarget:self];
    [self setDefaultButtonAction:@selector(buttonAction:) andTarget:self];
    [self setOtherButtonAction:@selector(buttonAction:) andTarget:self];
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

- (BOOL)suppressionButtonState
{
    return self.suppressionButton.state == NSControlStateValueOn;
}

- (void)setSuppressionButtonState:(BOOL)state
{
    self.suppressionButton.state = state ? NSControlStateValueOn : NSControlStateValueOff;
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
    self.inputLabel.stringValue = inputLabelText;
}

- (NSString *)inputLabelText
{
    return self.inputLabel.stringValue;
}

- (void)setOtherInputLabelText:(NSString *)otherInputLabelText
{
    self.otherInputLabel.stringValue = otherInputLabelText;
}

- (NSString *)otherInputLabelText
{
    return self.otherInputLabel.stringValue;
}

- (void)setInputPlaceholderText:(NSString *)str
{
    self.inputField.placeholderString = str;
}

- (NSString *)inputPlaceholderText
{
    return self.inputField.placeholderString;
}

- (void)setOtherInputPlaceholderText:(NSString *)str
{
    self.otherInputField.placeholderString = str;
}

- (NSString *)otherInputPlaceholderText
{
    return self.otherInputField.placeholderString;
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
    _headlineLabel = [NSTextField wrappingLabelWithString:@""];
    self.headlineLabel.translatesAutoresizingMaskIntoConstraints = NO;
    self.headlineLabel.editable = NO;
    self.headlineLabel.selectable = YES;
    self.headlineLabel.font = [NSFont boldSystemFontOfSize:NSFont.systemFontSize];

    // Setup Message Text View
    _messageLabel = [NSTextField wrappingLabelWithString:@""];
    self.messageLabel.translatesAutoresizingMaskIntoConstraints = NO;
    self.messageLabel.editable = NO;
    self.messageLabel.selectable = YES;
    
    // Setup Input Field
    _inputField = [NSTextField textFieldWithString:@""];
    self.inputField.translatesAutoresizingMaskIntoConstraints = NO;
    self.inputField.usesSingleLineMode = YES;
    self.inputField.editable = YES;
    
    _inputLabel = [NSTextField labelWithString:@""];
    self.inputLabel.translatesAutoresizingMaskIntoConstraints = NO;
    self.inputLabel.editable = NO;
    self.inputLabel.selectable = NO;
    
    // Setup Other Input Field
    _otherInputField = [NSTextField textFieldWithString:@""];
    self.otherInputField.translatesAutoresizingMaskIntoConstraints = NO;
    self.otherInputField.usesSingleLineMode = YES;
    self.otherInputField.editable = YES;

    _otherInputLabel = [NSTextField labelWithString:@""];
    self.otherInputLabel.translatesAutoresizingMaskIntoConstraints = NO;
    self.otherInputLabel.editable = NO;
    self.otherInputLabel.selectable = NO;
    
    // Setup Progressbar
    _progressbar = [[NSProgressIndicator alloc] init];
    _progressbar.translatesAutoresizingMaskIntoConstraints = NO;
    _progressbar.minValue = 0.0;
    _progressbar.maxValue = 1.0;
    _progressbar.usesThreadedAnimation = YES;
    
    // Setup Suppression Button
    _suppressionButton = [NSButton checkboxWithTitle:@"" target:nil action:nil];
    self.suppressionButton.translatesAutoresizingMaskIntoConstraints = NO;
    self.suppressionButton.buttonType = NSButtonTypeSwitch;
    self.suppressionButton.title = NSLocalizedString(@"Do not ask me again", @"");
    [self.suppressionButton setTarget:self andAction:@selector(suppressionButtonAction:)];
}

- (void)OE_layoutWindowIfNeeded
{
    if (_needsRebuild) {
        [self OE_layoutWindow];
        [self OE_createTouchBar];
    }
}

- (void)OE_layoutWindow
{
    NSView *contentView = self.window.contentView;
    contentView.subviews = @[];
    contentView.translatesAutoresizingMaskIntoConstraints = NO;

    NSLayoutAnchor *lastAnchor = contentView.topAnchor;
    NSLayoutAnchor *effectiveLeadingAnchor = contentView.leadingAnchor;
    NSImageView *image;
    
    /* create a NSAlert-style decoration image only if at least one of the messages is displayed. */
    if (self.messageText.length != 0 || self.informativeText.length != 0) {
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
        effectiveLeadingAnchor = image.trailingAnchor;
    }
    
    if (self.messageText.length != 0)
        lastAnchor = [self OE_layoutHeadlineUnderAnchor:lastAnchor leadingAnchor:effectiveLeadingAnchor];
    if (self.informativeText.length != 0)
        lastAnchor = [self OE_layoutMessageUnderAnchor:lastAnchor leadingAnchor:effectiveLeadingAnchor];
    if (self.showsProgressbar)
        lastAnchor = [self OE_layoutProgressBarUnderAnchor:lastAnchor leadingAnchor:effectiveLeadingAnchor];
    
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
    
    if (self.showsInputField || self.showsOtherInputField) {
        effectiveLeadingAnchor = contentView.leadingAnchor;
        lastAnchor = [self OE_layoutInputFieldsUnderAnchor:lastAnchor leadingAnchor:effectiveLeadingAnchor];
    }
    lastAnchor = [self OE_layoutButtonsUnderAnchor:lastAnchor leadingAnchor:effectiveLeadingAnchor];
    
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
        self.messageLabel.preferredMaxLayoutWidth = maxTextWidth;
        self.headlineLabel.preferredMaxLayoutWidth = maxTextWidth;
    } else {
        self.messageLabel.preferredMaxLayoutWidth = 0;
        self.headlineLabel.preferredMaxLayoutWidth = 0;
    }
    
    [self.window setContentSize:contentView.fittingSize];
    [self.window center];
    
    _needsRebuild = NO;
}

- (NSLayoutAnchor *)OE_layoutHeadlineUnderAnchor:(NSLayoutAnchor *)lastAnchor leadingAnchor:(NSLayoutAnchor *)effectiveLeadingAnchor
{
    NSView *contentView = self.window.contentView;
    
    [self.headlineLabel setContentHuggingPriority:NSLayoutPriorityDefaultHigh+1 forOrientation:NSLayoutConstraintOrientationVertical];
    [contentView addSubview:self.headlineLabel];
    [contentView addConstraints:@[
        [self.headlineLabel.topAnchor constraintEqualToAnchor:lastAnchor constant:OEAlertTopInset],
        [self.headlineLabel.leadingAnchor constraintEqualToAnchor:effectiveLeadingAnchor constant:OEAlertLeadingInset],
        [contentView.trailingAnchor constraintGreaterThanOrEqualToAnchor:self.headlineLabel.trailingAnchor constant:OEAlertTrailingInset]]];
    return self.headlineLabel.bottomAnchor;
}

- (NSLayoutAnchor *)OE_layoutMessageUnderAnchor:(NSLayoutAnchor *)lastAnchor leadingAnchor:(NSLayoutAnchor *)effectiveLeadingAnchor
{
    NSView *contentView = self.window.contentView;
    BOOL hasHeadline = self.messageText.length != 0;
    
    NSFont *messageFont;
    if (!hasHeadline) {
        messageFont = [NSFont systemFontOfSize:NSFont.systemFontSize];
    } else {
        messageFont = [NSFont systemFontOfSize:NSFont.smallSystemFontSize];
    }
    if (self.messageUsesHTML) {
        NSString *adjustedHtml = [NSString stringWithFormat:
                @"<span style=\"font-family: '-apple-system'; font-size:%fpx\">%@</span>",
                messageFont.pointSize, self.informativeText];
        NSData *htmlData = [adjustedHtml dataUsingEncoding:NSUTF8StringEncoding];
        NSMutableAttributedString *as = [[NSMutableAttributedString alloc]
                initWithHTML:htmlData options:@{
                    NSCharacterEncodingDocumentOption: @(NSUTF8StringEncoding)}
                documentAttributes:NULL];
        [as addAttribute:NSForegroundColorAttributeName value:[NSColor labelColor] range:NSMakeRange(0, as.length)];
        self.messageLabel.attributedStringValue = as;
        /* selectable labels revert to non-attributed values as soon as they are touched, losing all formatting */
        self.messageLabel.selectable = NO;
    } else {
        self.messageLabel.stringValue = self.informativeText;
        self.messageLabel.font = messageFont;
        self.messageLabel.selectable = YES;
    }
    
    [contentView addSubview:self.messageLabel];
    [contentView addConstraints:@[
        [self.messageLabel.topAnchor constraintEqualToAnchor:lastAnchor constant:hasHeadline ? OEAlertHeadlineToMessageSpacing : OEAlertTopInset],
        [self.messageLabel.leadingAnchor constraintEqualToAnchor:effectiveLeadingAnchor constant:OEAlertLeadingInset],
        [contentView.trailingAnchor constraintGreaterThanOrEqualToAnchor:self.messageLabel.trailingAnchor constant:OEAlertTrailingInset]]];
    return self.messageLabel.bottomAnchor;
}

- (NSLayoutAnchor *)OE_layoutProgressBarUnderAnchor:(NSLayoutAnchor *)lastAnchor leadingAnchor:(NSLayoutAnchor *)effectiveLeadingAnchor
{
    NSView *contentView = self.window.contentView;
    
    [contentView addSubview:self.progressbar];
    [contentView addConstraints:@[
        [self.progressbar.topAnchor constraintEqualToAnchor:lastAnchor constant:OEAlertProgressBarSpacing],
        [self.progressbar.leadingAnchor constraintEqualToAnchor:effectiveLeadingAnchor constant:OEAlertLeadingInset],
        [contentView.trailingAnchor constraintEqualToAnchor:self.progressbar.trailingAnchor constant:OEAlertTrailingInset]]];
    return self.progressbar.bottomAnchor;
}

- (NSLayoutAnchor *)OE_layoutInputFieldsUnderAnchor:(NSLayoutAnchor *)lastAnchor leadingAnchor:(NSLayoutAnchor *)effectiveLeadingAnchor
{
    NSView *contentView = self.window.contentView;
    
    NSGridView *inputGrid = [NSGridView gridViewWithNumberOfColumns:2 rows:0];
    inputGrid.translatesAutoresizingMaskIntoConstraints = NO;
    inputGrid.rowAlignment = NSGridRowAlignmentLastBaseline;
    [inputGrid columnAtIndex:0].xPlacement = NSGridCellPlacementTrailing;
    [contentView addSubview:inputGrid];
    [contentView addConstraints:@[
        [inputGrid.topAnchor constraintEqualToAnchor:lastAnchor constant:OEAlertTopInset],
        [inputGrid.leadingAnchor constraintEqualToAnchor:effectiveLeadingAnchor constant:OEAlertLeadingInset],
        [contentView.trailingAnchor constraintEqualToAnchor:inputGrid.trailingAnchor constant:OEAlertTrailingInset]]];
    
    if (self.showsOtherInputField) {
        [inputGrid addRowWithViews:@[self.otherInputLabel, self.otherInputField]];
    }
    if (self.showsInputField) {
        [inputGrid addRowWithViews:@[self.inputLabel, self.inputField]];
    }
    
    return inputGrid.bottomAnchor;
}

- (NSLayoutAnchor *)OE_layoutButtonsUnderAnchor:(NSLayoutAnchor *)lastAnchor leadingAnchor:(NSLayoutAnchor *)effectiveLeadingAnchor
{
    NSView *contentView = self.window.contentView;
    
    NSStackView *buttonStackView = [[NSStackView alloc] init];
    buttonStackView.orientation = NSUserInterfaceLayoutOrientationHorizontal;
    buttonStackView.translatesAutoresizingMaskIntoConstraints = NO;
    buttonStackView.alignment = NSLayoutAttributeLastBaseline;
    buttonStackView.spacing = OEAlertInterButtonSpacing;
    [contentView addSubview:buttonStackView];
    [contentView addConstraints:@[
        [buttonStackView.topAnchor constraintEqualToAnchor:lastAnchor constant:OEAlertButtonTopSpacing],
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
        [buttonStackView setCustomSpacing:OEAlertOtherButtonSpacing afterView:self.otherButton];
        [contentView addConstraints:@[
            [self.otherButton.widthAnchor constraintGreaterThanOrEqualToConstant:OEAlertMinimumButtonWidth],
            [self.otherButton.leadingAnchor constraintGreaterThanOrEqualToAnchor:effectiveLeadingAnchor constant:OEAlertLeadingInset]]];
    } else if (self.showsSuppressionButton) {
        [buttonStackView setCustomSpacing:OEAlertOtherButtonSpacing afterView:self.suppressionButton];
    }
    
    return buttonStackView.bottomAnchor;
}

- (void)OE_createTouchBarControls
{
    _defaultTBButton = [NSButton buttonWithTitle:@"" target:self action:@selector(buttonAction:)];
    self.defaultTBButton.keyEquivalent = @"\r";
    
    _alternateTBButton = [NSButton buttonWithTitle:@"" target:self action:@selector(buttonAction:)];
    
    _otherTBButton = [NSButton buttonWithTitle:@"" target:self action:@selector(buttonAction:)];
}

- (void)OE_createTouchBar
{
    NSTouchBar *tb = [[NSTouchBar alloc] init];
    NSGroupTouchBarItem *childTb = [NSGroupTouchBarItem alertStyleGroupItemWithIdentifier:@"OEAlertGroup"];
    tb.templateItems = [NSSet setWithObject:childTb];
    tb.defaultItemIdentifiers = @[@"OEAlertGroup"];
    tb.principalItemIdentifier = @"OEAlertGroup";
    
    NSMutableSet *allItems = [NSMutableSet set];
    NSMutableArray *allItemIds = [NSMutableArray array];
    NSCustomTouchBarItem *defaultTbi;
    NSCustomTouchBarItem *altTbi;
    NSCustomTouchBarItem *otherTbi;
    
    if (self.otherButtonTitle.length > 0) {
        otherTbi = [[NSCustomTouchBarItem alloc] initWithIdentifier:@"OEAlertOtherButton"];
        otherTbi.view = self.otherTBButton;
        [allItems addObject:otherTbi];
        [allItemIds addObject:@"OEAlertOtherButton"];
        [allItemIds addObject:NSTouchBarItemIdentifierFixedSpaceLarge];
    }
    if (self.alternateButtonTitle.length > 0) {
        altTbi = [[NSCustomTouchBarItem alloc] initWithIdentifier:@"OEAlertAlternateButton"];
        altTbi.view = self.alternateTBButton;
        [allItems addObject:altTbi];
        [allItemIds addObject:@"OEAlertAlternateButton"];
    }
    if (self.defaultButtonTitle.length > 0) {
        defaultTbi = [[NSCustomTouchBarItem alloc] initWithIdentifier:@"OEAlertDefaultButton"];
        defaultTbi.view = self.defaultTBButton;
        [allItems addObject:defaultTbi];
        [allItemIds addObject:@"OEAlertDefaultButton"];
    }
    
    childTb.groupTouchBar.templateItems = allItems;
    childTb.groupTouchBar.defaultItemIdentifiers = allItemIds;
    self.window.touchBar = tb;
}

@end

NS_ASSUME_NONNULL_END
