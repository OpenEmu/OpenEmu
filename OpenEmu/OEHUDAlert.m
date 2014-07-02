/*
 Copyright (c) 2011, OpenEmu Team
 
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
#import "OECenteredTextFieldCell.h"

#import "OEProgressIndicator.h"

#import "NSImage+OEDrawingAdditions.h"
#import "NSControl+OEAdditions.h"
#import "NSWindow+OECustomWindow.h"

#import "OEInputLimitFormatter.h"
#import "OEBackgroundImageView.h"

#import "OETheme.h"

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

@interface OEAlertWindow : NSWindow <OECustomWindow>
@end

@interface OEHUDAlert ()
{
    NSWindow *_window;
    NSUInteger result;
    
    OEAlertCompletionHandler callbackHandler;
}

- (void)OE_performCallback;
- (void)OE_layoutButtons;
- (void)OE_setupWindow;
- (void)OE_autosizeWindow;
- (NSUInteger)OE_countLinesOfTextView:(NSTextView *)textView;

@property (strong) NSMutableArray *blocks;
@end

@implementation OEHUDAlert
#pragma mark Properties
@synthesize result, suppressionUDKey;

@synthesize target, callback;
@synthesize suppressOnDefaultReturnOnly;

@synthesize defaultButton = _defaultButton, alternateButton = _alternateButton, otherButton=_otherButton;
@synthesize progressbar = _progressbar;
@synthesize messageTextView = _messageTextView, headlineTextView= _headlineTextView;
@synthesize suppressionButton = _suppressionButton;
@synthesize inputField = _inputField, inputLabelView = _inputLabelView, otherInputField = _otherInputField, otherInputLabelView = _otherInputLabelView;
@synthesize boxView = _boxView;
@synthesize window;

@dynamic inputLimit, progress;

#pragma mark -

- (void)show
{
    [_window makeKeyAndOrderFront:self];
    [_window center];
}

+ (id)alertWithMessageText:(NSString *)msgText defaultButton:(NSString *)defaultButtonLabel alternateButton:(NSString *)alternateButtonLabel
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    
    alert.defaultButtonTitle = defaultButtonLabel;
    alert.alternateButtonTitle = alternateButtonLabel;
    [alert setMessageText:msgText];
    
    return alert;
}

#pragma mark -
#pragma mark Memory Management

- (id)init
{
    self = [super init];
    if(self)
    {
        _window = [[OEAlertWindow alloc] init];
        [_window setReleasedWhenClosed:NO];
        
        _suppressionButton = [[OEButton alloc] init];
        [_suppressionButton setButtonType:NSSwitchButton];
        [(OEButton *)_suppressionButton setThemeKey:@"hud_checkbox"];
        
        _defaultButton = [[OEButton alloc] init];
        _alternateButton = [[OEButton alloc] init];
        _otherButton = [[OEButton alloc] init];
        
        _progressbar = [[OEProgressIndicator alloc] init];
        [_progressbar setMinValue:0.0];
        [_progressbar setMaxValue:1.0];
        [_progressbar setThemeKey:@"hud_progress"];
        
        _messageTextView = [[NSTextView alloc] init];
        _headlineTextView = [[NSTextView alloc] init];
        
        _inputField = [[OETextField alloc] init];
        _inputLabelView = [[NSTextView alloc] init];
        _otherInputField = [[OETextField alloc] init];
        _otherInputLabelView = [[NSTextView alloc] init];

        OEBackgroundImageView *box = [[OEBackgroundImageView alloc] initWithThemeKey:@"dark_inset_box"];
        [box setShouldFlipCoordinates:YES];
        _boxView = box;
        
        [self setSuppressOnDefaultReturnOnly:YES];
        [self OE_setupWindow];
    }
    
    return self;
}

- (void)dealloc
{    
    _progressbar = nil;
    _suppressionButton = nil;
        
    // Remove Callbacks
    self.target = nil;
    self.callbackHandler = nil;    
}

#pragma mark -

- (NSUInteger)runModal
{
    if([self suppressionUDKey] && [[NSUserDefaults standardUserDefaults] valueForKey:[self suppressionUDKey]])
    {
        result = [[NSUserDefaults standardUserDefaults] integerForKey:[self suppressionUDKey]];
        [self OE_performCallback];
        return result;
    }

    [self setBlocks:[NSMutableArray array]];
    
    [self OE_autosizeWindow];
    
    NSModalSession session = [NSApp beginModalSessionForWindow:_window];
    if([self window])
    {
        NSPoint p = (NSPoint){
            [[self window] frame].origin.x + ([[self window] frame].size.width-[_window frame].size.width)/2,
            [[self window] frame].origin.y + ([[self window] frame].size.height-[_window frame].size.height)/2
        };
        [_window setFrameOrigin:p];
    }
    else 
    {
        [_window center];
    }
    
    void(^executeBlocks)(void) = ^{
        NSMutableArray *blocks = [self blocks];
        @synchronized(blocks)
        {
            while([blocks count] != 0)
            {
                void(^aBlock)(void) = [blocks objectAtIndex:0];
                if(aBlock != nil) aBlock();
                [blocks removeObjectAtIndex:0];
            }
        }
    };
    
    while([NSApp runModalSession:session] == NSRunContinuesResponse)
    {
        executeBlocks();
        [[NSRunLoop mainRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
    }
    executeBlocks();
   
    [NSApp endModalSession:session];
    [_window close];
    
    [[self window] makeKeyAndOrderFront:self];
    
    return result;
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
    result = res;
    [NSApp stopModalWithCode:result];

    dispatch_async(dispatch_get_main_queue(), ^{
        [_window close];
        [self OE_performCallback];
    });
}
#pragma mark -
#pragma mark Window Configuration
- (void)setTitle:(NSString *)title
{
    [_window setTitle:title];
}
- (NSString *)title
{
    return [_window title];
}

- (CGFloat)height
{
    return [_window frame].size.height; 
}

- (void)setHeight:(CGFloat)height
{
    NSRect frame = [_window frame];
    frame.size.height = height;
    [_window setFrame:frame display:YES];
}

- (CGFloat)width
{
    return [_window frame].size.width; 
}

- (void)setWidth:(CGFloat)width
{
    NSRect frame = [_window frame];
    frame.size.width = width;
    [_window setFrame:frame display:YES];
}
#pragma mark -
#pragma mark Progress Bar

- (void)setShowsProgressbar:(BOOL)showsProgressbar
{
    [[self progressbar] setHidden:!showsProgressbar];
}

- (BOOL)showsProgressbar
{
    return [[self progressbar] isHidden];
}

- (double)progress
{
    return [[self progressbar] doubleValue];
}

- (void)setProgress:(double)progress
{
    [[self progressbar] setDoubleValue:progress];
}

#pragma mark -
#pragma mark Buttons
- (void)setDefaultButtonTitle:(NSString *)defaultButtonTitle
{
    [[self defaultButton] setTitle:defaultButtonTitle ? : @""];
    
    [self OE_layoutButtons];
}

- (NSString *)defaultButtonTitle
{
    return [[self defaultButton] title];
}

- (void)setAlternateButtonTitle:(NSString *)alternateButtonTitle
{
    [[self alternateButton] setTitle:alternateButtonTitle ? : @""];
    
    [self OE_layoutButtons];
}

- (NSString *)alternateButtonTitle
{
    return [[self alternateButton] title];
}

- (void)setOtherButtonTitle:(NSString *)otherButtonTitle
{
    [[self otherButton] setTitle:otherButtonTitle ? : @""];
    [self OE_layoutButtons];
}

- (NSString*)otherButtonTitle
{
    return [[self otherButton] title];
}

- (void)buttonAction:(id)sender
{
    if(sender == [self defaultButton] || sender == [self inputField])
        result = NSAlertDefaultReturn;
    else if(sender == [self alternateButton])
        result = NSAlertAlternateReturn;
    else if(sender == [self otherButton])
        result = NSAlertOtherReturn;
    else 
        result = NSAlertDefaultReturn;

    if(result != NSAlertOtherReturn && [[self suppressionButton] state] && (result || ![self suppressOnDefaultReturnOnly]) && [self suppressionUDKey])
    {
        NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
        [standardUserDefaults setInteger:result forKey:[self suppressionUDKey]];
    }

    [NSApp stopModalWithCode:result];
    [self OE_performCallback];
}

- (void)OE_layoutButtons
{
    BOOL showsDefaultButton   = [[self defaultButtonTitle] length] != 0;
    BOOL showsAlternateButton = [[self alternateButtonTitle] length] != 0;
    BOOL showsOtherButton = [[self otherButtonTitle] length] != 0;
    
    NSRect defaultButtonRect = NSMakeRect(304, 14, _OEHUDAlertButtonLength, _OEHUDAlertButtonHeight);
    
    if(showsDefaultButton) [[self defaultButton] setFrame:defaultButtonRect];
    
    if(showsAlternateButton)
    {
        NSRect alternateButtonRect = showsDefaultButton ? NSMakeRect(190, 14, _OEHUDAlertButtonLength, _OEHUDAlertButtonHeight) : defaultButtonRect;
        [[self alternateButton] setFrame:alternateButtonRect];
    }
    
    if(showsOtherButton)
    {
        NSRect otherButtonRect = NSMakeRect(16, 14, _OEHUDAlertButtonLength, _OEHUDAlertButtonHeight);
        [[self otherButton] setFrame:otherButtonRect];
    }
    
    [[self defaultButton] setHidden:!showsDefaultButton];
    [[self alternateButton] setHidden:!showsAlternateButton];
    [[self otherButton] setHidden:!showsOtherButton];
}

- (void)setDefaultButtonAction:(SEL)sel andTarget:(id)aTarget
{
    [[self defaultButton] setTarget:aTarget];
    [[self defaultButton] setAction:sel];
}

- (void)setAlternateButtonAction:(SEL)sel andTarget:(id)aTarget
{
    [[self alternateButton] setTarget:aTarget];
    [[self alternateButton] setAction:sel];
}


- (void)setOtherButtonAction:(SEL)sel andTarget:(id)aTarget
{
    [[self alternateButton] setTarget:aTarget];
    [[self alternateButton] setAction:sel];
}

#pragma mark -
#pragma mark Message Text

- (void)setHeadlineText:(NSString *)headlineText
{
    [[self headlineTextView] setString:headlineText ? : @""];
    [[self headlineTextView] setHidden:[headlineText length] == 0];
}

- (NSString *)headlineText
{
    return [[self headlineTextView] string];
}

- (void)setMessageText:(NSString *)messageText
{
    [[self messageTextView] setString:messageText ? : @""];
    [[self messageTextView] setHidden:[messageText length] == 0];
    [[self messageTextView] sizeToFit];
}

- (NSString *)messageText
{
    return [[self messageTextView] string];
}

#pragma mark -
#pragma mark Callbacks

- (void)setCallbackHandler:(OEAlertCompletionHandler)handler
{
    callbackHandler = [handler copy];
    
    [[self alternateButton] setTarget:self];
    [[self alternateButton] setAction:@selector(buttonAction:)];
    [[self defaultButton] setTarget:self];
    [[self defaultButton] setAction:@selector(buttonAction:)];
    [[self otherButton] setTarget:self];
    [[self otherButton] setAction:@selector(buttonAction:)];
}

- (OEAlertCompletionHandler)callbackHandler
{
    return callbackHandler;
}

- (void)OE_performCallback
{   
    if([self target] != nil && [self callback] != NULL && [[self target] respondsToSelector:[self callback]])
        [[self target] performSelectorOnMainThread:[self callback] withObject:self waitUntilDone:NO];
    
    if([self callbackHandler] != nil)
    {
        callbackHandler(self, [self result]);
        [self callbackHandler];
        callbackHandler = nil;
    }
}

#pragma mark -
#pragma mark Suppression Button

- (BOOL)showsSuppressionButton
{
    return ![[self suppressionButton] isHidden];
}

- (void)setShowsSuppressionButton:(BOOL)showsSuppressionButton
{
    [[self suppressionButton] setHidden:!showsSuppressionButton];
}

- (void)showSuppressionButtonForUDKey:(NSString *)key
{
    [self setShowsSuppressionButton:YES];
    [self setSuppressionUDKey:key];
    
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    BOOL checked = [standardUserDefaults valueForKey:[self suppressionUDKey]] != nil;
    [[self suppressionButton] setState:checked];
}

- (void)setSuppressionLabelText:(NSString *)suppressionLabelText
{
    [[self suppressionButton] setTitle:suppressionLabelText];
    [[self suppressionButton] sizeToFit];
}

- (NSString *)suppressionLabelText
{
    return [[self suppressionButton] title];
}

- (void)suppressionButtonAction:(id)sender
{
    if(![self suppressionUDKey]) return;
    
    NSUInteger       state                = [sender state];
    NSUserDefaults  *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    if(!state && [self suppressOnDefaultReturnOnly])
        [standardUserDefaults removeObjectForKey:[self suppressionUDKey]];   
}

#pragma mark -
#pragma mark Input Field

- (void)setShowsInputField:(BOOL)showsInputField
{
    [[self inputField] setHidden:!showsInputField];
    [[self inputLabelView] setHidden:!showsInputField];
    [[self boxView] setHidden:showsInputField];
}

- (BOOL)showsInputField
{
    return ![[self inputField] isHidden];
}

- (void)setShowsOtherInputField:(BOOL)showsOtherInputField
{
    [[self otherInputField] setHidden:!showsOtherInputField];
    [[self otherInputLabelView] setHidden:!showsOtherInputField];
    [[self boxView] setHidden:showsOtherInputField];
}

- (BOOL)showsOtherInputField
{
    return ![[self otherInputField] isHidden];
}

- (void)setStringValue:(NSString *)stringValue
{
    [[self inputField] setStringValue:stringValue];
}

- (NSString *)stringValue
{
    return [[self inputField] stringValue];
}

- (void)setOtherStringValue:(NSString *)otherStringValue
{
    [[self otherInputField] setStringValue:otherStringValue];
}

- (NSString *)otherStringValue
{
    return [[self otherInputField] stringValue];
}

- (void)setInputLabelText:(NSString *)inputLabelText
{
    [[self inputLabelView] setString:inputLabelText];
}

- (NSString *)inputLabelText
{
    return [[self inputLabelView] string];
}

- (void)setOtherInputLabelText:(NSString *)otherInputLabelText
{
    [[self otherInputLabelView] setString:otherInputLabelText];
}

- (NSString *)otherInputLabelText
{
    return [[self otherInputLabelView] string];
}

- (NSInteger)inputLimit
{
    OEInputLimitFormatter *formatter = [[self inputField] formatter];
    
    return [formatter isKindOfClass:[OEInputLimitFormatter class]] ? [formatter limit] : 0;
}

- (void)setInputLimit:(NSInteger)inputLimit
{
    OEInputLimitFormatter *formatter = [[self inputField] formatter];
    
    BOOL limitFormatterSet = (formatter != nil && [formatter isKindOfClass:[OEInputLimitFormatter class]]);
    
    if(inputLimit == 0 && limitFormatterSet)
        [[self inputField] setFormatter:nil];
    else if(limitFormatterSet)
        [formatter setLimit:inputLimit];
    else 
    {
        formatter = [[OEInputLimitFormatter alloc] initWithLimit:inputLimit];
        [[self inputField] setFormatter:formatter];
    }
}
#pragma mark -
#pragma mark Private Methods

- (void)OE_setupWindow
{    
    NSRect frame = [_window frame];
    frame.size = (NSSize){ _OEHUDAlertBoxSideMargin + _OEHUDAlertDefaultBoxWidth + _OEHUDAlertBoxSideMargin, 1 };
    [_window setFrame:frame display:NO];

    NSFont *defaultFont = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0];
    NSFont *boldFont = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:0 size:11.0];

    NSColor *defaultColor = [NSColor colorWithDeviceWhite:0.859 alpha:1.0];

    // Setup Button
    [[self defaultButton] setThemeKey:@"hud_button_blue"];
    [[self defaultButton] setTarget:self andAction:@selector(buttonAction:)];
    [[self defaultButton] setKeyEquivalent:@"\r"];
    [[self defaultButton] setAutoresizingMask:NSViewMinXMargin | NSViewMaxYMargin];
    [[self defaultButton] setTitle:@""];
    [[self defaultButton] setHidden:YES];
    [[_window contentView] addSubview:[self defaultButton]];
    
    [[self alternateButton] setThemeKey:@"hud_button"];
    [[self alternateButton] setTarget:self andAction:@selector(buttonAction:)];
    [[self alternateButton] setKeyEquivalent:@"\E"];
    [[self alternateButton] setAutoresizingMask:NSViewMinXMargin | NSViewMaxYMargin];
    [[self alternateButton] setTitle:@""];
    [[self alternateButton] setHidden:YES];
    [[_window contentView] addSubview:[self alternateButton]];
    
    [[self otherButton] setThemeKey:@"hud_button"];
    [[self otherButton] setTarget:self andAction:@selector(buttonAction:)];
    [[self otherButton] setKeyEquivalent:@"\r"];
    [[self otherButton] setAutoresizingMask:NSViewMinXMargin | NSViewMaxYMargin];
    [[self otherButton] setTitle:@""];
    [[self otherButton] setHidden:YES];
    [[_window contentView] addSubview:[self otherButton]];

    // Setup Box
    [[self boxView] setFrame:(NSRect){{_OEHUDAlertBoxSideMargin, _OEHUDAlertBoxTopMargin},{_OEHUDAlertDefaultBoxWidth, 1}}];
    [[self boxView] setAutoresizingMask:NSViewHeightSizable | NSViewWidthSizable];
    [[_window contentView] addSubview:[self boxView]];

    // Setup Headline Text View
    [[self headlineTextView] setEditable:NO];
    [[self headlineTextView] setSelectable:NO];
    [[self headlineTextView] setFont:boldFont];
    [[self headlineTextView] setTextColor:defaultColor];
    [[self headlineTextView] setDrawsBackground:NO];
    [[self headlineTextView] setFrame:NSMakeRect(_OEHUDAlertBoxTextSideMargin, _OEHUDAlertBoxTextTopMargin, 1, _OEHUDAlertBoxTextHeight)];
    [[self headlineTextView] setHidden:YES];
    [[self boxView] addSubview:[self headlineTextView]];

    // Setup Message Text View    
    [[self messageTextView] setEditable:NO];
    [[self messageTextView] setSelectable:NO];
    [[self messageTextView] setFont:defaultFont];
    [[self messageTextView] setTextColor:defaultColor];
    [[self messageTextView] setDrawsBackground:NO];
    [[self messageTextView] setFrame:NSMakeRect(_OEHUDAlertBoxTextSideMargin, _OEHUDAlertBoxTextTopMargin + (2 * _OEHUDAlertBoxTextHeight),
                                                _OEHUDAlertDefaultBoxWidth - (2 * _OEHUDAlertBoxTextSideMargin), 1)];
    [[self messageTextView] setHidden:YES];
    [[self boxView] addSubview:[self messageTextView]];

    // Setup Input Field
    OETextFieldCell *inputCell = [[OETextFieldCell alloc] init];
    [[self inputField] setCell:inputCell];
    [[self inputField] setFrame:NSMakeRect(68, 51, 337, 23)];
    [[self inputField] setHidden:YES];
    [[self inputField] setAutoresizingMask:NSViewWidthSizable | NSViewMaxYMargin];
    [[self inputField] setTarget:self andAction:@selector(buttonAction:)];
    [[self inputField] setEditable:YES];
    [[self inputField] setThemeKey:@"hud_textfield"];
    [[_window contentView] addSubview:[self inputField]];
    
    [[self inputLabelView] setEditable:NO];
    [[self inputLabelView] setSelectable:NO];
    [[self inputLabelView] setAutoresizingMask:NSViewMaxXMargin | NSViewMaxYMargin];
    [[self inputLabelView] setFont:defaultFont];
    [[self inputLabelView] setTextColor:defaultColor];
    [[self inputLabelView] setDrawsBackground:NO];
    [[self inputLabelView] setAlignment:NSRightTextAlignment];
    [[self inputLabelView] setFrame:NSMakeRect(1, 57, 61, 23)];
    [[self inputLabelView] setHidden:YES];
    [[_window contentView] addSubview:[self inputLabelView]];
    
    // Setup Other Input Field
    OETextFieldCell *otherInputCell = [[OETextFieldCell alloc] init];
    [[self otherInputField] setCell:otherInputCell];
    [[self otherInputField] setFrame:NSMakeRect(68, 90, 337, 23)];
    [[self otherInputField] setHidden:YES];
    [[self otherInputField] setAutoresizingMask:NSViewWidthSizable | NSViewMaxYMargin];
    [[self otherInputField] setFocusRingType:NSFocusRingTypeNone];
    [[self otherInputField] setTarget:self andAction:@selector(buttonAction:)];
    [[self otherInputField] setEditable:YES];
    [[self otherInputField] setWantsLayer:YES];
    [[self otherInputField] setThemeKey:@"hud_textfield"];
    [[_window contentView] addSubview:[self otherInputField]];

    [[self otherInputLabelView] setEditable:NO];
    [[self otherInputLabelView] setSelectable:NO];
    [[self otherInputLabelView] setAutoresizingMask:NSViewMaxXMargin | NSViewMaxYMargin];
    [[self otherInputLabelView] setFont:defaultFont];
    [[self otherInputLabelView] setTextColor:defaultColor];
    [[self otherInputLabelView] setDrawsBackground:NO];
    [[self otherInputLabelView] setAlignment:NSRightTextAlignment];
    [[self otherInputLabelView] setFrame:NSMakeRect(1, 96, 61, 23)];
    [[self otherInputLabelView] setHidden:YES];
    [[_window contentView] addSubview:[self otherInputLabelView]];
    
    // Setup Progressbar
    [[self progressbar] setFrame:NSMakeRect(64, 47, 258, 14)];
    [[self progressbar] setHidden:YES];
    [[self boxView] addSubview:[self progressbar]];
    
    // Setup Suppression Button
    [[self suppressionButton] setTitle:OELocalizedString(@"Do not ask me again", @"")];
    [[self suppressionButton] setAutoresizingMask:NSViewMaxXMargin|NSViewMaxYMargin];
    [[self suppressionButton] setFrame:NSMakeRect(_OEHUDAlertBoxSideMargin, _OEHUDAlertBoxSideMargin - 1,
                                                  _OEHUDAlertSuppressionButtonLength, _OEHUDAlertSuppressionButtonHeight)];
    [[self suppressionButton] setHidden:YES];
    [[self suppressionButton] setTarget:self andAction:@selector(suppressionButtonAction:)];
    [[_window contentView] addSubview:[self suppressionButton]];
}

- (void)OE_autosizeWindow
{
    NSRect frame = [_window frame];
    
    if([[self boxView] isHidden])
    {
        if([self showsOtherInputField])
            frame.size.height = 150;
        else
            frame.size.height = 112;

        [_window setFrame:frame display:NO];
    }
    else
    {
        NSRect boxFrame = [[self boxView] frame];
        NSRect headlineTextFrame =[[self headlineTextView] frame];
        NSRect messageTextFrame = [[self messageTextView] frame];
        BOOL isMessageTextVisible = ![[self messageTextView] isHidden];
        BOOL isHeadlineTextVisible = ![[self headlineTextView] isHidden];

        // To show the whole headline text, need to add about 10, otherwise the last glyph gets clipped
        NSSize headlineTextSize = [[self headlineText] sizeWithAttributes:@{  NSFontAttributeName : [_headlineTextView font] }];
        headlineTextSize.width += 10;
        headlineTextSize.width  = MAX(headlineTextSize.width, _OEHUDAlertMinimumHeadlineLength);

        if(isMessageTextVisible && isHeadlineTextVisible)
        {
            headlineTextFrame.size.width = headlineTextSize.width;
            messageTextFrame.size.width = headlineTextFrame.size.width;
            [[self headlineTextView] setFrame:headlineTextFrame];
            [[self messageTextView] setFrame:messageTextFrame];

            NSUInteger linesInMessageTextView = [self OE_countLinesOfTextView:[self messageTextView]];

            // Add together the margins and the lines in the messageTextView + headline and empty line
            boxFrame.size.height = _OEHUDAlertBoxTextTopMargin + _OEHUDAlertBoxTextHeight * (linesInMessageTextView + 2) + _OEHUDAlertBoxTextBottomMargin;
            boxFrame.size.width = (2 * _OEHUDAlertBoxTextSideMargin) + headlineTextFrame.size.width;
        }
        else if(isHeadlineTextVisible)
        {
            headlineTextFrame.size.width = headlineTextSize.width;
            [[self headlineTextView] setFrame:headlineTextFrame];
            
            boxFrame.size.height = _OEHUDAlertBoxTextTopMargin + _OEHUDAlertBoxTextHeight + _OEHUDAlertBoxTextBottomMargin;
            boxFrame.size.width = (2 * _OEHUDAlertBoxTextSideMargin) + headlineTextFrame.size.width;
        }
        else if(isMessageTextVisible)
        {
            messageTextFrame.origin = headlineTextFrame.origin;
            [[self messageTextView] setFrame:messageTextFrame];

            NSUInteger linesInMessageTextView = [self OE_countLinesOfTextView:[self messageTextView]];
            boxFrame.size.height = _OEHUDAlertBoxTextTopMargin + (_OEHUDAlertBoxTextHeight * linesInMessageTextView) + _OEHUDAlertBoxTextBottomMargin;
        }

        frame.size.height = _OEHUDAlertBoxTopMargin + boxFrame.size.height + _OEHUDAlertBoxBottomMargin;
        frame.size.width = (2 * _OEHUDAlertBoxSideMargin) + boxFrame.size.width;
        [_window setFrame:frame display:NO];
        [[self boxView] setFrame:boxFrame];
    }
}

- (NSUInteger)OE_countLinesOfTextView:(NSTextView *)textView
{
    NSLayoutManager *layoutManager = [textView layoutManager];
    NSUInteger numberOfLines, index, numberOfGlyphs = [layoutManager numberOfGlyphs];
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

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if(self != [OEAlertWindow class]) return;

    [NSWindow registerWindowClassForCustomThemeFrameDrawing:[OEAlertWindow class]];
}

- (id)init
{
    self = [super init];
    if(self)
    {
        [self setOpaque:NO];
        [self setBackgroundColor:[NSColor clearColor]];
    }
    return self;
}

#pragma mark OECustomWindow implementation

- (BOOL)drawsAboveDefaultThemeFrame
{
    return YES;
}

- (void)drawThemeFrame:(NSRect)dirtyRect
{
    NSRect bounds = [self frame];
    bounds.origin = (NSPoint){0,0};

    [[NSColor clearColor] setFill];
    NSRectFill(bounds);

    OEThemeState state = [[(NSView*)self window] isMainWindow] ? OEThemeInputStateWindowActive : OEThemeInputStateWindowInactive;
    OEThemeImage *image = [[OETheme sharedTheme] themeImageForKey:@"hud_alert_window"];
    NSImage *nsimage = [image imageForState:state];
    [nsimage drawInRect:bounds fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
}

@end
