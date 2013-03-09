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

#import "OEPreferencesPlainBox.h"
#import "OEHUDProgressbar.h"

#import "NSImage+OEDrawingAdditions.h"
#import "NSControl+OEAdditions.h"
#import "NSWindow+OECustomWindow.h"

#import "OEInputLimitFormatter.h"

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

@end

@implementation OEHUDAlert
#pragma mark Properties
@synthesize result, suppressionUDKey;

@synthesize target, callback;
@synthesize suppressOnDefaultReturnOnly;

@synthesize defaultButton = _defaultButton, alternateButton = _alternateButton, otherButton=_otherButton;
@synthesize progressbar = _progressbar;
@synthesize messageTextView = _messageTextView, headlineLabelField = _headlineLabelField;
@synthesize suppressionButton = _suppressionButton;
@synthesize inputField = _inputField, inputLabelField = _inputLabelField, otherInputField = _otherInputField, otherInputLabelField = _otherInputLabelField;
@synthesize boxView = _boxView;
@synthesize window;

@dynamic inputLimit, progress;

#pragma mark -

- (void)show
{
    [_window makeKeyAndOrderFront:self];
    [_window center];
}

+ (id)alertWithError:(NSError *)error
{
    return nil;
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
        
        _progressbar = [[OEHUDProgressbar alloc] init];
        
        _messageTextView = [[NSTextView alloc] init];
        _headlineLabelField = [[NSTextField alloc] init];
        
        _inputField = [[OETextField alloc] init];
        _inputLabelField = [[NSTextField alloc] init];
        _otherInputField = [[OETextField alloc] init];
        _otherInputLabelField = [[NSTextField alloc] init];
        
        _boxView = [[OEPreferencesPlainBox alloc] init];
        
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
    
    for (;;) {
        if ([NSApp runModalSession:session] != NSRunContinuesResponse)
            break;
        [[NSRunLoop mainRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
    }
    [NSApp endModalSession:session];
    [_window close];
    
    [[self window] makeKeyAndOrderFront:self];
    
    return result;
}

- (void)closeWithResult:(NSInteger)res
{
    result = res;
    [NSApp stopModalWithCode:result];
    [_window close];
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

- (CGFloat)progress
{
    return [[self progressbar] value];
}

- (void)setProgress:(CGFloat)progress
{
    [[self progressbar] setValue:progress];
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
    
    NSRect defaultButtonRect = NSMakeRect(304 - 3, 14 - 1, 103, 23);
    
    if(showsDefaultButton) [[self defaultButton] setFrame:defaultButtonRect];
    
    if(showsAlternateButton)
    {
        NSRect alternateButtonRect = showsDefaultButton ? NSMakeRect(190 - 3, 14 - 1, 103, 23) : defaultButtonRect;
        [[self alternateButton] setFrame:alternateButtonRect];
    }
    
    if(showsOtherButton)
    {
        NSRect otherButtonRect = NSMakeRect(190 - 3, 14 - 1, 103, 23);
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

- (void)setHeadlineLabelText:(NSString *)headlineLabelText
{
    [[self headlineLabelField] setStringValue:headlineLabelText ? : @""];
    [[self headlineLabelField] setHidden:[headlineLabelText length] == 0];
}

- (NSString *)headlineLabelText
{
    return [[self headlineLabelField] stringValue];
}

- (void)setMessageText:(NSString *)messageText
{
    [[self messageTextView] setString:messageText ? : @""];
    
    NSRect textViewFrame = NSInsetRect((NSRect){ .size = [self boxView].frame.size }, 46, 23);
    [[self messageTextView] setFrame:textViewFrame];
    [[self messageTextView] setHidden:[messageText length] == 0];
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
    [[self inputLabelField] setHidden:!showsInputField];
    [[self boxView] setHidden:showsInputField];
}

- (BOOL)showsInputField
{
    return ![[self inputField] isHidden];
}

- (void)setShowsOtherInputField:(BOOL)showsOtherInputField
{
    [[self otherInputField] setHidden:!showsOtherInputField];
    [[self otherInputLabelField] setHidden:!showsOtherInputField];
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
    [[self inputLabelField] setStringValue:inputLabelText];
}

- (NSString *)inputLabelText
{
    return [[self inputLabelField] stringValue];
}

- (void)setOtherInputLabelText:(NSString *)otherInputLabelText
{
    [[self otherInputLabelField] setStringValue:otherInputLabelText];
}

- (NSString *)otherInputLabelText
{
    return [[self otherInputLabelField] stringValue];
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
    NSRect f = [_window frame];
    f.size = (NSSize){ 421, 172 };
    [_window setFrame:f display:NO];
    
    // Setup Button
    [[self defaultButton] setThemeKey:@"hud_button_blue"];
    [[self defaultButton] setAutoresizingMask:NSViewMinXMargin | NSViewMaxYMargin];
    [[self defaultButton] setTarget:self andAction:@selector(buttonAction:)];
    [[self defaultButton] setHidden:YES];
    [[self defaultButton] setTitle:@""];
    [[self defaultButton] setKeyEquivalent:@"\r"];
    [[_window contentView] addSubview:[self defaultButton]];
    
    
    [[self alternateButton] setThemeKey:@"hud_button"];
    [[self alternateButton] setAutoresizingMask:NSViewMinXMargin | NSViewMaxYMargin];
    [[self alternateButton] setTarget:self andAction:@selector(buttonAction:)];
    [[self alternateButton] setHidden:YES];
    [[self alternateButton] setTitle:@""];
    [[self alternateButton] setKeyEquivalent:@"\E"];
    [[_window contentView] addSubview:[self alternateButton]];
    
    [[self otherButton] setThemeKey:@"hud_button"];
    [[self otherButton] setAutoresizingMask:NSViewMinXMargin | NSViewMaxYMargin];
    [[self otherButton] setTarget:self andAction:@selector(buttonAction:)];
    [[self otherButton] setHidden:YES];
    [[self otherButton] setTitle:@""];
    [[self otherButton] setKeyEquivalent:@"\r"];
    [[_window contentView] addSubview:[self otherButton]];

    // Setup Box
    [[self boxView] setFrame:(NSRect){{18, 51},{387, 82}}];
    [[self boxView] setAutoresizingMask:NSViewHeightSizable | NSViewWidthSizable];
    [[_window contentView] addSubview:[self boxView]];
    
    // Setup Message Text View
    NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0];
    
    [[self messageTextView] setEditable:NO];
    [[self messageTextView] setSelectable:NO];
    [[self messageTextView] setAutoresizingMask:NSViewMinYMargin | NSViewWidthSizable];
    [[self messageTextView] setFont:font];
    [[self messageTextView] setTextColor:[NSColor whiteColor]];
    [[self messageTextView] setDrawsBackground:NO];

    NSMutableParagraphStyle *style = [[NSMutableParagraphStyle alloc] init];
    [style setAlignment:NSCenterTextAlignment];
    [style setLineSpacing:7];
    [[self messageTextView] setDefaultParagraphStyle:style];
    
    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:1.0]];
    [shadow setShadowBlurRadius:0];
    [shadow setShadowOffset:(NSSize){ 0, -1 }];
    [[self messageTextView] setShadow:shadow];
    NSRect textViewFrame = NSInsetRect((NSRect){ .size = [self boxView].frame.size }, 46, 23);
    [[self messageTextView] setFrame:textViewFrame];
    [[self messageTextView] setHidden:YES];
    [[self boxView] addSubview:[self messageTextView]];
    
    // Setup Input Field
    shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:1.0]];
    [shadow setShadowBlurRadius:0];
    [shadow setShadowOffset:(NSSize){ 0, -1 }];
    
    OETextFieldCell *inputCell = [[OETextFieldCell alloc] init];
    [[self inputField] setCell:inputCell];
    [[self inputField] setFrame:NSMakeRect(68, 51, 337, 23)];
    [[self inputField] setHidden:YES];
    [[self inputField] setAutoresizingMask:NSViewWidthSizable | NSViewMaxYMargin];
    [[self inputField] setTarget:self andAction:@selector(buttonAction:)];
    [[self inputField] setEditable:YES];
//    [[self inputField] setWantsLayer:YES];
    [[self inputField] setThemeKey:@"hud_textfield"];
    [[_window contentView] addSubview:[self inputField]];
    
    
    [[self inputLabelField] setFrame:NSMakeRect(1, 51, 61, 23)];
    [[self inputLabelField] setHidden:YES];
    OECenteredTextFieldCell *labelCell = [[OECenteredTextFieldCell alloc] init];
    
    font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0];
    
    NSMutableParagraphStyle *paraStyle = [[NSMutableParagraphStyle alloc] init];
    [paraStyle setAlignment:NSRightTextAlignment];
    [labelCell setTextAttributes:[NSDictionary dictionaryWithObjectsAndKeys:
                                  [NSColor whiteColor]  , NSForegroundColorAttributeName,
                                  paraStyle             , NSParagraphStyleAttributeName,
                                  font                  , NSFontAttributeName,
                                  nil]];
    [[self inputLabelField] setAutoresizingMask:NSViewMaxXMargin|NSViewMaxYMargin];
    [[self inputLabelField] setCell:labelCell];
    [[_window contentView] addSubview:[self inputLabelField]];
    
    // Setup Other Input Field
    shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:1.0]];
    [shadow setShadowBlurRadius:0];
    [shadow setShadowOffset:(NSSize){ 0, -1 }];
    
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
    
    
    [[self otherInputLabelField] setFrame:NSMakeRect(1, 90, 61, 23)];
    [[self otherInputLabelField] setHidden:YES];
    OECenteredTextFieldCell *otherLabelCell = [[OECenteredTextFieldCell alloc] init];
    
    font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0];
    
    NSMutableParagraphStyle *otherParaStyle = [[NSMutableParagraphStyle alloc] init];
    [otherParaStyle setAlignment:NSRightTextAlignment];
    [otherLabelCell setTextAttributes:[NSDictionary dictionaryWithObjectsAndKeys:
                                  [NSColor whiteColor]  , NSForegroundColorAttributeName,
                                  otherParaStyle             , NSParagraphStyleAttributeName,
                                  font                  , NSFontAttributeName,
                                  nil]];
    [[self otherInputLabelField] setAutoresizingMask:NSViewMaxXMargin|NSViewMaxYMargin];
    [[self otherInputLabelField] setCell:otherLabelCell];
    [[_window contentView] addSubview:[self otherInputLabelField]];
    
    // setup progressbar
    NSRect progressBarRect = NSMakeRect(64, 47, 258, 16);
    [[self progressbar] setFrame:progressBarRect];
    [[self progressbar] setHidden:YES];
    [[self boxView] addSubview:[self progressbar]];
    
    NSRect progressLabelFrame = NSMakeRect(2, 21 - 16, 382, 16);
    [[self headlineLabelField] setFrame:progressLabelFrame];
    [[self headlineLabelField] setHidden:YES];
    [[self headlineLabelField] setDrawsBackground:NO];
    [[self headlineLabelField] setAutoresizingMask:NSViewMaxYMargin|NSViewWidthSizable];
    labelCell = [[OECenteredTextFieldCell alloc] init];
    
    shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor blackColor]];
    [shadow setShadowBlurRadius:0];
    [shadow setShadowOffset:(NSSize){ 0, -1 }];
    
    font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:0 size:11.0];
    
    paraStyle = [[NSMutableParagraphStyle alloc] init];
    [paraStyle setAlignment:NSCenterTextAlignment];
    [labelCell setTextAttributes:[NSDictionary dictionaryWithObjectsAndKeys:
                                  [NSColor whiteColor], NSForegroundColorAttributeName,
                                  paraStyle           , NSParagraphStyleAttributeName,
                                  shadow              , NSShadowAttributeName,
                                  font                , NSFontAttributeName,
                                  nil]];
    [labelCell setupAttributes];
    [[self headlineLabelField] setCell:labelCell];
    [[self boxView] addSubview:[self headlineLabelField]];
    
    // Setup Suppression Button
    [[self suppressionButton] setTitle:NSLocalizedString(@"Do not ask me again", @"")];
    [[self suppressionButton] setAutoresizingMask:NSViewMaxXMargin|NSViewMaxYMargin];
    [[self suppressionButton] setFrame:NSMakeRect(18, 18, 150, 18)];
    [[self suppressionButton] setHidden:YES]; 
    [[self suppressionButton] setTarget:self andAction:@selector(suppressionButtonAction:)];
    [[_window contentView] addSubview:[self suppressionButton]];
}

@end

#pragma mark -

@implementation OEAlertWindow

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if(self != [OEAlertWindow class]) return;

    if([NSImage imageNamed:@"hud_alert_window_active"]) return;
    
    NSImage *hudWindowBorder = [NSImage imageNamed:@"hud_alert_window"];    
    [hudWindowBorder setName:@"hud_alert_window_active" forSubimageInRect:(NSRect){ .size = { 29, 47 } }];
    [hudWindowBorder setName:@"hud_alert_window_inactive" forSubimageInRect:(NSRect){ .size = { 29, 47 } }];
    
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
    
    NSImage *image = [NSImage imageNamed:@"hud_alert_window"];
    [image drawInRect:bounds fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:nil leftBorder:14 rightBorder:14 topBorder:24 bottomBorder:22];
}

@end
