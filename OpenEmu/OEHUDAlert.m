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

static const CGFloat _OEHUDAlertBoxSideMargin           =  18.0;
static const CGFloat _OEHUDAlertBoxTopMargin            =  51.0;
static const CGFloat _OEHUDAlertBoxBottomMargin         =  39.0;
static const CGFloat _OEHUDAlertBoxTextSideMargin       =  48.0;
static const CGFloat _OEHUDAlertBoxTextTopMargin        =  28.0;
static const CGFloat _OEHUDAlertBoxTextBottomMargin     =  28.0;
static const CGFloat _OEHUDAlertBoxTextHeight           =  14.0;
static const CGFloat _OEHUDAlertDefaultBoxWidth         = 387.0;
static const CGFloat _OEHUDAlertSuppressionButtonLength = 150.0;
static const CGFloat _OEHUDAlertSuppressionButtonHeight =  18.0;
static const CGFloat _OEHUDAlertButtonLength            = 103.0;
static const CGFloat _OEHUDAlertButtonHeight            =  23.0;
static const CGFloat _OEHUDAlertMinimumHeadlineLength   = 291.0;

@interface OEAlertWindow : NSWindow
@end

@interface HUDAlertContentView : NSView
@property NSPoint draggingPoint;
@end

@interface OEHUDAlert ()

@property NSWindow *hudWindow;

- (void)OE_performCallback;
- (void)OE_layoutButtons;
- (void)OE_setupWindow;
- (void)OE_autosizeWindow;
- (NSUInteger)OE_countLinesOfTextView:(NSTextView *)textView;

@property (copy, readonly) NSMutableArray <void(^)(void)> *blocks;

@end

@implementation OEHUDAlert
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
        _hudWindow = [[OEAlertWindow alloc] initWithContentRect:NSMakeRect(0, 0, 0, 0) styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:YES];
        _hudWindow.releasedWhenClosed = NO;
        
        _suppressionButton = [[OEButton alloc] init];
        _suppressionButton.buttonType = NSSwitchButton;
        ((OEButton *)_suppressionButton).themeKey = @"hud_checkbox";
        
        _defaultButton = [[OEButton alloc] init];
        _alternateButton = [[OEButton alloc] init];
        _otherButton = [[OEButton alloc] init];
        
        _progressbar = [[OEProgressIndicator alloc] init];
        _progressbar.minValue = 0.0;
        _progressbar.maxValue = 1.0;
        _progressbar.themeKey = @"hud_progress";
        
        _messageTextView = [[NSTextView alloc] init];
        _headlineTextView = [[NSTextView alloc] init];
        
        _inputField = [[OETextField alloc] init];
        _inputLabelView = [[NSTextView alloc] init];
        _otherInputField = [[OETextField alloc] init];
        _otherInputLabelView = [[NSTextView alloc] init];

        OEBackgroundImageView *box = [[OEBackgroundImageView alloc] initWithThemeKey:@"dark_inset_box"];
        box.shouldFlipCoordinates = YES;
        _boxView = box;
        
        self.suppressOnDefaultReturnOnly = YES;
        [self OE_setupWindow];

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

    [self OE_autosizeWindow];
    
    if(_window)
    {
        NSRect hudWindowFrame = _hudWindow.frame;
        NSRect windowFrame = _window.frame;
        NSPoint p = NSMakePoint(
            windowFrame.origin.x + (NSWidth(windowFrame) - NSWidth(hudWindowFrame)) / 2,
            windowFrame.origin.y + (NSHeight(windowFrame) - NSHeight(hudWindowFrame)) / 2
        );
        _hudWindow.frameOrigin = p;
    }
    else 
    {
        [_hudWindow center];
    }
    [_hudWindow makeKeyAndOrderFront:nil];
    
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
    };

    NSModalSession session = [NSApp beginModalSessionForWindow:_hudWindow];
    while([NSApp runModalSession:session] == NSModalResponseContinue)
    {
        [NSRunLoop.mainRunLoop runMode:NSDefaultRunLoopMode beforeDate:NSDate.distantFuture];
        executeBlocks();
        [NSRunLoop.currentRunLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];

    }
    executeBlocks();
   
    [NSApp endModalSession:session];

    [_hudWindow close];

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
        [self->_hudWindow close];
        [self OE_performCallback];
    });
}

- (void)show
{
    [_hudWindow makeKeyAndOrderFront:self];
    [_hudWindow center];
}

#pragma mark - Window Configuration

- (void)setTitle:(NSString *)title
{
    _hudWindow.title = title;
}

- (NSString *)title
{
    return _hudWindow.title;
}

- (CGFloat)height
{
    return NSHeight(_hudWindow.frame);
}

- (void)setHeight:(CGFloat)height
{
    NSRect frame = _hudWindow.frame;
    frame.size.height = height;
    [_hudWindow setFrame:frame display:YES];
}

- (CGFloat)width
{
    return NSWidth(_hudWindow.frame);
}

- (void)setWidth:(CGFloat)width
{
    NSRect frame = _hudWindow.frame;
    frame.size.width = width;
    [_hudWindow setFrame:frame display:YES];
}

#pragma mark - Progress Bar

- (void)setShowsProgressbar:(BOOL)showsProgressbar
{
    self.progressbar.hidden = !showsProgressbar;
}

- (BOOL)showsProgressbar
{
    return self.progressbar.isHidden;
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
    
    [self OE_layoutButtons];
}

- (NSString *)defaultButtonTitle
{
    return self.defaultButton.title;
}

- (void)setAlternateButtonTitle:(nullable NSString *)alternateButtonTitle
{
    self.alternateButton.title = alternateButtonTitle ? : @"";
    
    [self OE_layoutButtons];
}

- (NSString *)alternateButtonTitle
{
    return self.alternateButton.title;
}

- (void)setOtherButtonTitle:(nullable NSString *)otherButtonTitle
{
    self.otherButton.title = otherButtonTitle ? : @"";
    [self OE_layoutButtons];
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

- (void)OE_layoutButtons
{
    BOOL showsDefaultButton   = self.defaultButtonTitle.length != 0;
    BOOL showsAlternateButton = self.alternateButtonTitle.length != 0;
    BOOL showsOtherButton = self.otherButtonTitle.length != 0;
    
    NSRect defaultButtonRect = NSMakeRect(304, 14, _OEHUDAlertButtonLength, _OEHUDAlertButtonHeight);
    
    if(showsDefaultButton)
    {
        self.defaultButton.frame = defaultButtonRect;
    }
    
    if(showsAlternateButton)
    {
        NSRect alternateButtonRect = showsDefaultButton ? NSMakeRect(190, 14, _OEHUDAlertButtonLength, _OEHUDAlertButtonHeight) : defaultButtonRect;
        self.alternateButton.frame = alternateButtonRect;
    }
    
    if(showsOtherButton)
    {
        NSRect otherButtonRect = NSMakeRect(16, 14, _OEHUDAlertButtonLength, _OEHUDAlertButtonHeight);
        self.otherButton.frame = otherButtonRect;
    }
    
    self.defaultButton.hidden = !showsDefaultButton;
    self.alternateButton.hidden = !showsAlternateButton;
    self.otherButton.hidden = !showsOtherButton;
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
    self.headlineTextView.string = headlineText ? : @"";
    self.headlineTextView.hidden = headlineText.length == 0;
}

- (NSString *)headlineText
{
    return self.headlineTextView.string;
}

- (void)setMessageText:(nullable NSString *)messageText
{
    self.messageTextView.string = messageText ? : @"";
    self.messageTextView.hidden = messageText.length == 0;
    [self.messageTextView sizeToFit];
}

- (NSString *)messageText
{
    return self.messageTextView.string;
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

- (BOOL)showsSuppressionButton
{
    return !self.suppressionButton.isHidden;
}

- (void)setShowsSuppressionButton:(BOOL)showsSuppressionButton
{
    self.suppressionButton.hidden = !showsSuppressionButton;
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
    [self.suppressionButton sizeToFit];
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
    self.inputField.hidden = !showsInputField;
    self.inputLabelView.hidden = !showsInputField;
    self.boxView.hidden = showsInputField;
}

- (BOOL)showsInputField
{
    return !self.inputField.isHidden;
}

- (void)setShowsOtherInputField:(BOOL)showsOtherInputField
{
    self.otherInputField.hidden = !showsOtherInputField;
    self.otherInputLabelView.hidden = !showsOtherInputField;
    self.boxView.hidden = showsOtherInputField;
}

- (BOOL)showsOtherInputField
{
    return !self.otherInputField.isHidden;
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
    self.inputLabelView.string = inputLabelText;
}

- (NSString *)inputLabelText
{
    return self.inputLabelView.string;
}

- (void)setOtherInputLabelText:(NSString *)otherInputLabelText
{
    self.otherInputLabelView.string = otherInputLabelText;
}

- (NSString *)otherInputLabelText
{
    return self.otherInputLabelView.string;
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

- (void)OE_setupWindow
{    
    NSRect frame = _hudWindow.frame;
    frame.size = (NSSize){ _OEHUDAlertBoxSideMargin + _OEHUDAlertDefaultBoxWidth + _OEHUDAlertBoxSideMargin, 1 };
    [_hudWindow setFrame:frame display:NO];

    NSFont *defaultFont = [NSFont systemFontOfSize:11];
    NSFont *boldFont = [NSFont boldSystemFontOfSize:11];

    NSColor *defaultColor = [NSColor colorWithDeviceWhite:0.859 alpha:1.0];

    // Setup Button
    self.defaultButton.themeKey = @"hud_button_blue";
    [self.defaultButton setTarget:self andAction:@selector(buttonAction:)];
    self.defaultButton.keyEquivalent = @"\r";
    self.defaultButton.autoresizingMask = NSViewMinXMargin | NSViewMaxYMargin;
    self.defaultButton.title = @"";
    self.defaultButton.hidden = YES;
    [_hudWindow.contentView addSubview:self.defaultButton];
    
    self.alternateButton.themeKey = @"hud_button";
    [self.alternateButton setTarget:self andAction:@selector(buttonAction:)];
    self.alternateButton.keyEquivalent = @"\E";
    self.alternateButton.autoresizingMask = NSViewMinXMargin | NSViewMaxYMargin;
    self.alternateButton.title = @"";
    self.alternateButton.hidden = YES;
    [_hudWindow.contentView addSubview:self.alternateButton];
    
    self.otherButton.themeKey = @"hud_button";
    [self.otherButton setTarget:self andAction:@selector(buttonAction:)];
    self.otherButton.keyEquivalent = @"\r";
    self.otherButton.autoresizingMask = NSViewMinXMargin | NSViewMaxYMargin;
    self.otherButton.title = @"";
    self.otherButton.hidden = YES;
    [_hudWindow.contentView addSubview:self.otherButton];

    // Setup Box
    self.boxView.frame = (NSRect){{_OEHUDAlertBoxSideMargin, _OEHUDAlertBoxTopMargin},{_OEHUDAlertDefaultBoxWidth, 1}};
    self.boxView.autoresizingMask = NSViewHeightSizable | NSViewWidthSizable;
    [_hudWindow.contentView addSubview:self.boxView];

    // Setup Headline Text View
    self.headlineTextView.editable = NO;
    self.headlineTextView.selectable = NO;
    self.headlineTextView.font = boldFont;
    self.headlineTextView.textColor = defaultColor;
    self.headlineTextView.drawsBackground = NO;
    self.headlineTextView.frame = NSMakeRect(_OEHUDAlertBoxTextSideMargin, _OEHUDAlertBoxTextTopMargin, 1, _OEHUDAlertBoxTextHeight);
    self.headlineTextView.hidden = YES;
    [self.boxView addSubview:self.headlineTextView];

    // Setup Message Text View    
    self.messageTextView.editable = NO;
    self.messageTextView.selectable = NO;
    self.messageTextView.font = defaultFont;
    self.messageTextView.textColor = defaultColor;
    self.messageTextView.drawsBackground = NO;
    self.messageTextView.frame = NSMakeRect(_OEHUDAlertBoxTextSideMargin, _OEHUDAlertBoxTextTopMargin + (2 * _OEHUDAlertBoxTextHeight),
                                                _OEHUDAlertDefaultBoxWidth - (2 * _OEHUDAlertBoxTextSideMargin), 1);
    self.messageTextView.hidden = YES;
    [self.boxView addSubview:self.messageTextView];

    // Setup Input Field
    OETextFieldCell *inputCell = [[OETextFieldCell alloc] init];
    self.inputField.cell = inputCell;
    self.inputField.frame = NSMakeRect(68, 51, 337, 23);
    self.inputField.hidden = YES;
    self.inputField.autoresizingMask = NSViewWidthSizable | NSViewMaxYMargin;
    [self.inputField setTarget:self andAction:@selector(buttonAction:)];
    self.inputField.editable = YES;
    self.inputField.themeKey = @"hud_textfield";
    [_hudWindow.contentView addSubview:self.inputField];
    
    self.inputLabelView.editable = NO;
    self.inputLabelView.selectable = NO;
    self.inputLabelView.autoresizingMask = NSViewMaxXMargin | NSViewMaxYMargin;
    self.inputLabelView.font = defaultFont;
    self.inputLabelView.textColor = defaultColor;
    self.inputLabelView.drawsBackground = NO;
    self.inputLabelView.alignment = NSRightTextAlignment;
    self.inputLabelView.frame = NSMakeRect(1, 57, 61, 23);
    self.inputLabelView.hidden = YES;
    [_hudWindow.contentView addSubview:self.inputLabelView];
    
    // Setup Other Input Field
    OETextFieldCell *otherInputCell = [[OETextFieldCell alloc] init];
    self.otherInputField.cell = otherInputCell;
    self.otherInputField.frame = NSMakeRect(68, 90, 337, 23);
    self.otherInputField.hidden = YES;
    self.otherInputField.autoresizingMask = NSViewWidthSizable | NSViewMaxYMargin;
    self.otherInputField.focusRingType = NSFocusRingTypeNone;
    [self.otherInputField setTarget:self andAction:@selector(buttonAction:)];
    self.otherInputField.editable = YES;
    self.otherInputField.wantsLayer = YES;
    self.otherInputField.themeKey = @"hud_textfield";
    [_hudWindow.contentView addSubview:self.otherInputField];

    self.otherInputLabelView.editable = NO;
    self.otherInputLabelView.selectable = NO;
    self.otherInputLabelView.autoresizingMask = NSViewMaxXMargin | NSViewMaxYMargin;
    self.otherInputLabelView.font = defaultFont;
    self.otherInputLabelView.textColor = defaultColor;
    self.otherInputLabelView.drawsBackground = NO;
    self.otherInputLabelView.alignment = NSRightTextAlignment;
    self.otherInputLabelView.frame = NSMakeRect(1, 96, 61, 23);
    self.otherInputLabelView.hidden = YES;
    [_hudWindow.contentView addSubview:self.otherInputLabelView];
    
    // Setup Progressbar
    self.progressbar.frame = NSMakeRect(64, 47, 258, 14);
    self.progressbar.hidden = YES;
    [self.boxView addSubview:self.progressbar];
    
    // Setup Suppression Button
    self.suppressionButton.title = NSLocalizedString(@"Do not ask me again", @"");
    self.suppressionButton.autoresizingMask = NSViewMaxXMargin|NSViewMaxYMargin;
    self.suppressionButton.frame = NSMakeRect(_OEHUDAlertBoxSideMargin,
                                              _OEHUDAlertBoxSideMargin - 1,
                                              _OEHUDAlertSuppressionButtonLength,
                                              _OEHUDAlertSuppressionButtonHeight);
    self.suppressionButton.hidden = YES;
    [self.suppressionButton setTarget:self andAction:@selector(suppressionButtonAction:)];
    [_hudWindow.contentView addSubview:self.suppressionButton];
}

- (void)OE_autosizeWindow
{
    NSRect frame = _hudWindow.frame;
    
    if(self.boxView.isHidden)
    {
        if(self.showsOtherInputField)
            frame.size.height = 150;
        else
            frame.size.height = 112;

        [_hudWindow setFrame:frame display:NO];
    }
    else
    {
        NSRect boxFrame = self.boxView.frame;
        NSRect headlineTextFrame = self.headlineTextView.frame;
        NSRect messageTextFrame = self.messageTextView.frame;
        BOOL isMessageTextVisible = !self.messageTextView.isHidden;
        BOOL isHeadlineTextVisible = !self.headlineTextView.isHidden;

        // To show the whole headline text, need to add about 10, otherwise the last glyph gets clipped
        NSSize headlineTextSize = [self.headlineText sizeWithAttributes:@{  NSFontAttributeName : _headlineTextView.font }];
        headlineTextSize.width += 10;
        headlineTextSize.width = MAX(headlineTextSize.width, _OEHUDAlertMinimumHeadlineLength);

        if(isMessageTextVisible && isHeadlineTextVisible)
        {
            headlineTextFrame.size.width = headlineTextSize.width;
            messageTextFrame.size.width = NSWidth(headlineTextFrame);
            self.headlineTextView.frame = headlineTextFrame;
            self.messageTextView.frame = messageTextFrame;

            NSUInteger linesInMessageTextView = [self OE_countLinesOfTextView:self.messageTextView];

            // Add together the margins and the lines in the messageTextView + headline and empty line
            boxFrame.size.height = _OEHUDAlertBoxTextTopMargin + _OEHUDAlertBoxTextHeight * (linesInMessageTextView + 2) + _OEHUDAlertBoxTextBottomMargin;
            boxFrame.size.width = (2 * _OEHUDAlertBoxTextSideMargin) + headlineTextFrame.size.width;
        }
        else if(isHeadlineTextVisible)
        {
            headlineTextFrame.size.width = headlineTextSize.width;
            self.headlineTextView.frame = headlineTextFrame;
            
            boxFrame.size.height = _OEHUDAlertBoxTextTopMargin + _OEHUDAlertBoxTextHeight + _OEHUDAlertBoxTextBottomMargin;
            boxFrame.size.width = (2 * _OEHUDAlertBoxTextSideMargin) + headlineTextFrame.size.width;
        }
        else if(isMessageTextVisible)
        {
            messageTextFrame.origin = headlineTextFrame.origin;
            self.messageTextView.frame = messageTextFrame;

            NSUInteger linesInMessageTextView = [self OE_countLinesOfTextView:self.messageTextView];
            boxFrame.size.height = _OEHUDAlertBoxTextTopMargin + (_OEHUDAlertBoxTextHeight * linesInMessageTextView) + _OEHUDAlertBoxTextBottomMargin;
        }

        frame.size.height = _OEHUDAlertBoxTopMargin + NSHeight(boxFrame) + _OEHUDAlertBoxBottomMargin;
        frame.size.width = (2 * _OEHUDAlertBoxSideMargin) + NSWidth(boxFrame);
        [_hudWindow setFrame:frame display:NO];
        self.boxView.frame = boxFrame;
    }
}

- (NSUInteger)OE_countLinesOfTextView:(NSTextView *)textView
{
    NSLayoutManager *layoutManager = textView.layoutManager;
    NSUInteger numberOfLines, index, numberOfGlyphs = layoutManager.numberOfGlyphs;
    NSRange lineRange;
    for(numberOfLines = 0, index = 0; index < numberOfGlyphs; numberOfLines++)
    {
        (void) [layoutManager lineFragmentRectForGlyphAtIndex:index effectiveRange:&lineRange];
        index = NSMaxRange(lineRange);
    }

    return numberOfLines;
}

@end

#pragma mark -

@implementation OEAlertWindow

- (instancetype)initWithContentRect:(NSRect)contentRect styleMask:(NSWindowStyleMask)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag
{
    self = [super initWithContentRect:contentRect styleMask:NSBorderlessWindowMask backing:bufferingType defer:flag];
    if(self)
    {
        self.opaque = NO;
        self.backgroundColor = NSColor.clearColor;
        self.movableByWindowBackground = YES;

        NSRect frame = contentRect;
        contentRect.origin = NSMakePoint(0, 0);
        NSView *contentView = [[HUDAlertContentView alloc] initWithFrame:frame];
        self.contentView = contentView;
    }
    return self;
}

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}

@end

@implementation HUDAlertContentView

- (BOOL)isOpaque
{
    return NO;
}

- (void)drawRect:(NSRect)dirtyRect
{
    NSRect bounds = self.bounds;

    NSWindow *window = self.window;
    OEThemeState state = window.isMainWindow ? OEThemeInputStateWindowActive : OEThemeInputStateWindowInactive;
    OEThemeImage *image = [OETheme.sharedTheme themeImageForKey:@"hud_alert_window"];
    NSImage *nsimage = [image imageForState:state];
    [nsimage drawInRect:bounds fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
}

@end

NS_ASSUME_NONNULL_END
