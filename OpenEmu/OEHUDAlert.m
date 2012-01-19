//
//  OEAlert.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 14.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEHUDAlert.h"

#import "OECheckBox.h"
#import "OEHUDButtonCell.h"
#import "OEHUDTextFieldCell.h"
#import "OECenteredTextFieldCell.h"

#import "OEPreferencesPlainBox.h"
#import "OEHUDProgressbar.h"

#import "NSImage+OEDrawingAdditions.h"
#import "NSControl+OEAdditions.h"
#import "NSWindow+OECustomWindow.h"

@interface OEAlertWindow : NSWindow <OECustomWindow>
@end
@interface OEHUDAlert (Private)
- (void)performCallback;
- (void)layoutButtons;
- (void)_setupWindow;
@end

@implementation OEHUDAlert
#pragma mark Properties
@synthesize result, suppressionUDKey;

@synthesize defaultButton=_defaultButton, alternateButton=_alternateButton;
@synthesize progressbar=_progressbar;
@synthesize messageTextView=_messageTextView, headlineLabelField=_headlineLabelField;
@synthesize suppressionButton=_suppressionButton;
@synthesize inputField=_inputField, inputLabelField=_inputLabelField;
@synthesize boxView=_boxView;

@synthesize window;
#pragma mark -
- (void)show{
    [_window makeKeyAndOrderFront:self];
    [_window center];
}
+ (id)alertWithError:(NSError*)error
{
    return nil;
}
+ (id)alertWithMessageText:(NSString *)msgText defaultButton:(NSString*)defaultButtonLabel alternateButton:(NSString*)alternateButtonLabel
{
    OEHUDAlert* alert = [[OEHUDAlert alloc] init];
    
    alert.defaultButtonTitle = defaultButtonLabel;
    alert.alternateButtonTitle = alternateButtonLabel;
    [alert setMessageText:msgText];
    
    return [alert autorelease];
}

+ (id)autoSaveGameAlert
{
    OEHUDAlert* alert = [[OEHUDAlert alloc] init];
    
    alert.messageText = NSLocalizedString(@"OpenEmu includes a save game feature that allows you to continue playing exactly where you left off.", @"");
    alert.defaultButtonTitle = NSLocalizedString(@"Save Game", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Do Not Save", @"");
    alert.headlineLabelText = NSLocalizedString(@"Would you like to save your game before you quit?", @"");
    [alert setHeight:200.0];
    
    [alert showSuppressionButtonForUDKey:UDSaveGameWhenQuitAlertSuppressionKey];
    
    return [alert autorelease];
}

+ (id)saveGameAlertWithProposedName:(NSString*)name
{
    OEHUDAlert* alert = [[OEHUDAlert alloc] init];
    
    [alert setInputLabelText:@"Save As:"];
    [alert setDefaultButtonTitle:NSLocalizedString(@"Save Game", @"")];
    [alert setAlternateButtonTitle:NSLocalizedString(@"Cancel", @"")];
    [alert setShowsInputField:YES];
    [alert setStringValue:name];
    [alert setHeight:112.0];    
    [alert setWidth:372.0];
    
    [alert showSuppressionButtonForUDKey:UDSaveGameAlertSuppressionKey];
    [alert setSuppressionLabelText:NSLocalizedString(@"Don't ask again", @"")];
    
    return [alert autorelease];
}

+ (id)deleteGameAlertWithStateName:(NSString*)stateName
{
    OEHUDAlert* alert = [[OEHUDAlert alloc] init];
    NSString* messageText = [NSString stringWithFormat:NSLocalizedString(@"Are you sure you want to delete the save game called '%@' from your OpenEmu library?", @""), stateName];
    [alert setMessageText:messageText];
    [alert setDefaultButtonTitle:NSLocalizedString(@"Delete Game", @"")];
    [alert setAlternateButtonTitle:NSLocalizedString(@"Cancel", @"")];
    [alert setHeadlineLabelText:nil];
    [alert showSuppressionButtonForUDKey:UDDelteGameAlertSuppressionKey];
    
    return [alert autorelease];
}
#pragma mark -
#pragma mark Memory Management
- (id)init
{
    self = [super init];
    if (self)
    {
        NSLog(@"OEHUDAlert init");
        
        _window = [[OEAlertWindow alloc] init];
        [_window setReleasedWhenClosed:NO];
        
        _suppressionButton = [[OECheckBox alloc] init];
        
        _defaultButton = [[NSButton alloc] init];
        _alternateButton = [[NSButton alloc] init];
        
        _progressbar = [[OEHUDProgressbar alloc] init];
        
        _messageTextView = [[NSTextView alloc] init];
        _headlineLabelField = [[NSTextField alloc] init];
        
        _inputField = [[NSTextField alloc] init];
        _inputLabelField = [[NSTextField alloc] init];
        
        _boxView = [[OEPreferencesPlainBox alloc] init];
        
        [self _setupWindow];
    }
    
    return self;
}

- (void)dealloc
{    
    NSLog(@"OEHUDAlert dealloc");
    
    [_window release];
    
    [_defaultButton release];
    _defaultButton = nil;
    
    [_alternateButton release];
    _alternateButton = nil;
    
    [_progressbar release];
    _progressbar = nil;
    
    [_messageTextView release];
    _messageTextView = nil;
    
    [_headlineLabelField release];
    _headlineLabelField = nil;
    
    [_suppressionButton release];
    _suppressionButton = nil;
    
    [_inputLabelField release];
    _inputLabelField = nil;
    
    [_inputLabelField release];
    _inputLabelField = nil;
    
    [_boxView release];
    _boxView = nil;
    
    // Remove Callbacks
    self.target = nil;
    self.callbackHandler = nil;
    
    // Remove suppression button stuff
    self.suppressionUDKey = nil;
    
    self.window = nil;
    
    [super dealloc];
}
#pragma mark -
- (NSUInteger)runModal
{
    if(self.showsSuppressionButton && self.suppressionButton.state == NSOnState){
        result = NSAlertDefaultReturn;
        [self performCallback];
        return NSAlertDefaultReturn;
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
    
    return  result;
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
- (NSString*)title
{
    return [_window title];
}
- (float)height
{
    return [_window frame].size.height; 
}
- (void)setHeight:(float)height
{
    NSRect frame = [_window frame];
    frame.size.height = height;
    [_window setFrame:frame display:YES];
}

- (float)width
{
    return [_window frame].size.width; 
}

- (void)setWidth:(float)width
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

- (void)setProgress:(float)progress
{
    [[self progressbar] setValue:progress];
}

- (float)progress
{
    return [[self progressbar] value];
}

#pragma mark -
#pragma mark Buttons
- (void)setDefaultButtonTitle:(NSString *)defaultButtonTitle
{
    if(!defaultButtonTitle) defaultButtonTitle = @"";
    [[self defaultButton] setTitle:defaultButtonTitle];
    
    [self layoutButtons];
}
- (NSString*)defaultButtonTitle
{
    return [[self defaultButton] title];
}

- (void)setAlternateButtonTitle:(NSString *)alternateButtonTitle
{
    if(!alternateButtonTitle) alternateButtonTitle = @"";
    [[self alternateButton] setTitle:alternateButtonTitle];
    
    [self layoutButtons];
}

- (NSString*)alternateButtonTitle
{
    return [[self alternateButton] title];
}

- (void)buttonAction:(id)sender{
    if(sender == [self defaultButton] || sender == [self inputField])
    {
        result = NSAlertDefaultReturn;
        
        if([self showsSuppressionButton] && [self suppressionUDKey])
        {
            NSUserDefaults* standardUserDefaults = [NSUserDefaults standardUserDefaults];
            [standardUserDefaults setBool:[self suppressionButton].state==NSOnState forKey:[self suppressionUDKey]];            
        }
    }
    else if(sender == [self alternateButton])
        result = NSAlertAlternateReturn;
    else
        result = NSAlertDefaultReturn;
    
    [NSApp stopModalWithCode:result];
    [self performCallback];
}

- (void)layoutButtons
{
    BOOL showsDefaultButton = [[self defaultButtonTitle] length]!=0;
    BOOL showsAlternateButton = [[self alternateButtonTitle] length]!=0;
    
    NSRect defaultButtonRect = (NSRect){{304-3, 14-1},{103, 23}};
    if(showsDefaultButton)
    {
        [[self defaultButton] setFrame:defaultButtonRect];
    }
    
    if(showsAlternateButton)
    {
        NSRect alternateButtonRect = showsDefaultButton?(NSRect){{190-3, 14-1},{103, 23}}:defaultButtonRect;
        [[self alternateButton] setFrame:alternateButtonRect];
    }
    [[self defaultButton] setHidden:!showsDefaultButton];
    [[self alternateButton] setHidden:!showsAlternateButton];
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
#pragma mark -
#pragma mark Message Text
- (void)setHeadlineLabelText:(NSString *)headlineLabelText
{
    if(!headlineLabelText) headlineLabelText=@"";
    [[self headlineLabelField] setStringValue:headlineLabelText];
    [[self headlineLabelField] setHidden:[headlineLabelText length]==0];
}

- (NSString*)headlineLabelText
{
    return [[self headlineLabelField] stringValue];
}

- (void)setMessageText:(NSString *)messageText
{
    if(messageText==nil) messageText = @"";
    [[self messageTextView] setString:messageText];
    NSRect textViewFrame = NSInsetRect((NSRect){{0,0}, [self boxView].frame.size}, 46, 23);
    [[self messageTextView] setFrame:textViewFrame];
    [[self messageTextView] setHidden:[messageText length]==0];
}

- (NSString*)messageText
{
    return [[self messageTextView] string];
}

#pragma mark -
#pragma mark Callbacks
@synthesize target, callback;
- (void)setCallbackHandler:(OEAlertCompletionHandler)handler
{
    if([self callbackHandler]!=nil)
        [callbackHandler release];
    
    callbackHandler = [handler copy];
    
    [[self alternateButton] setTarget:self];
    [[self alternateButton] setAction:@selector(buttonAction:)];
    [[self defaultButton] setTarget:self];
    [[self defaultButton] setAction:@selector(buttonAction:)];
}
- (OEAlertCompletionHandler)callbackHandler
{
    return callbackHandler;
}

- (void)performCallback
{   
    if([self target] && [self callback] != NULL && [[self target] respondsToSelector:[self callback]])
        [[self target] performSelectorOnMainThread:[self callback] withObject:self waitUntilDone:NO];
    
    if([self callbackHandler]!=nil)
    {
        callbackHandler(self, [self result]);
        [[self callbackHandler] release];
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

- (void)showSuppressionButtonForUDKey:(NSString*)key
{
    [self setShowsSuppressionButton:YES];
    [self setSuppressionUDKey:key];
    
    NSUserDefaults* standardUserDefaults = [NSUserDefaults standardUserDefaults];
    BOOL checked = [standardUserDefaults boolForKey:[self suppressionUDKey]];
    [[self suppressionButton] setState:checked];
}

- (void)setSuppressionLabelText:(NSString *)suppressionLabelText
{
    [[self suppressionButton] setTitle:suppressionLabelText];
    [[self suppressionButton] sizeToFit];
}
- (NSString*)suppressionLabelText
{
    return [[self suppressionButton] title];
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
- (void)setStringValue:(NSString *)stringValue
{
    [[self inputField] setStringValue:stringValue];
}
- (NSString*)stringValue
{
    return [[self inputField] stringValue];
}

- (void)setInputLabelText:(NSString *)inputLabelText
{
    [[self inputLabelField] setStringValue:inputLabelText];
}

- (NSString*)inputLabelText
{
    return [[self inputLabelField] stringValue];
}
#pragma mark -
#pragma mark Private Methods
- (void)_setupWindow
{    
    NSRect f = [_window frame];
    f.size = (NSSize){421, 172};
    [_window setFrame:f display:NO];
    
    // Setup Button
    OEHUDButtonCell* cell = [[OEHUDButtonCell alloc] init];
    [cell setButtonColor:OEHUDButtonColorBlue];    
    [[self defaultButton] setCell:cell];
    [[self defaultButton] setAutoresizingMask:NSViewMinXMargin|NSViewMaxYMargin];
    [[self defaultButton] setTarget:self andAction:@selector(buttonAction:)];
    [[self defaultButton] setHidden:YES];
    [[self defaultButton] setKeyEquivalent:@"\r"];
    [cell release];
    [[_window contentView] addSubview:[self defaultButton]];
    
    cell = [[OEHUDButtonCell alloc] init];
    [cell setButtonColor:OEHUDButtonColorDefault];
    [[self alternateButton] setCell:cell];
    [[self alternateButton] setAutoresizingMask:NSViewMinXMargin|NSViewMaxYMargin];
    [[self alternateButton] setTarget:self andAction:@selector(buttonAction:)];
    [[self alternateButton] setHidden:YES];
    [[self alternateButton] setKeyEquivalent:@"\E"];
    [cell release];
    [[_window contentView] addSubview:[self alternateButton]];
    
    // Setup Box
    [[self boxView] setFrame:(NSRect){{18, 51},{387, 82}}];
    [[self boxView] setAutoresizingMask:NSViewHeightSizable|NSViewWidthSizable];
    [[_window contentView] addSubview:[self boxView]];
    
    // Setup Message Text View
    NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0];
    
    [[self messageTextView] setEditable:NO];
    [[self messageTextView] setSelectable:NO];
    [[self messageTextView] setDrawsBackground:NO];
    [[self messageTextView] setAutoresizingMask:NSViewMinYMargin|NSViewWidthSizable];
    [[self messageTextView] setFont:font];
    [[self messageTextView] setTextColor:[NSColor whiteColor]];

    NSMutableParagraphStyle* style = [[NSMutableParagraphStyle alloc] init];
    [style setAlignment:NSCenterTextAlignment];
    [style setLineSpacing:7];
    [[self messageTextView] setDefaultParagraphStyle:style];
    [style release];
    
    NSShadow* shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:1.0]];
    [shadow setShadowBlurRadius:0];
    [shadow setShadowOffset:(NSSize){0,-1}];
    [[self messageTextView] setShadow:shadow];
    NSRect textViewFrame = NSInsetRect((NSRect){{0,0}, [self boxView].frame.size}, 46, 23);
    [[self messageTextView] setFrame:textViewFrame];
    [[self messageTextView] setHidden:YES];
    [[self boxView] addSubview:[self messageTextView]];
    [shadow release];
    
    // Setup Input Field
    shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:1.0]];
    [shadow setShadowBlurRadius:0];
    [shadow setShadowOffset:(NSSize){0,-1}];
    
    OEHUDTextFieldCell* inputCell = [[OEHUDTextFieldCell alloc] init];
    [[self inputField] setCell:inputCell];
    [inputCell release];
    [[self inputField] setFrame:(NSRect){{68,51},{337, 23}}];
    [[self inputField] setHidden:YES];
    [[self inputField] setAutoresizingMask:NSViewWidthSizable|NSViewMaxYMargin];
    [[self inputField] setFocusRingType:NSFocusRingTypeNone];
    [[self inputField] setTarget:self andAction:@selector(buttonAction:)];
    [[self inputField] setEditable:YES];
    [self inputField].wantsLayer = YES;
    [[_window contentView] addSubview:[self inputField]];
    [shadow release];
    
    
    [[self inputLabelField] setFrame:(NSRect){{1,51},{61,23}}];
    [[self inputLabelField] setHidden:YES];
    OECenteredTextFieldCell* labelCell = [[OECenteredTextFieldCell alloc] init];
    
    font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0];
    
    NSMutableParagraphStyle* paraStyle = [[NSMutableParagraphStyle alloc] init];
    [paraStyle setAlignment:NSRightTextAlignment];
    [labelCell setTextAttributes:[NSDictionary dictionaryWithObjectsAndKeys:
                                  [NSColor whiteColor]  , NSForegroundColorAttributeName,
                                  paraStyle             , NSParagraphStyleAttributeName,
                                  font                  , NSFontAttributeName,
                                  nil]];
    [paraStyle release];
    [[self inputLabelField] setAutoresizingMask:NSViewMaxXMargin|NSViewMaxYMargin];
    [[self inputLabelField] setCell:labelCell];
    [labelCell release];
    [[_window contentView] addSubview:[self inputLabelField]];
    
    // setup progressbar
    NSRect progressBarRect = NSMakeRect(64, 47, 258, 16);
    [[self progressbar] setFrame:progressBarRect];
    [[self progressbar] setHidden:YES];
    [[self boxView] addSubview:[self progressbar]];
    
    NSRect progressLabelFrame = NSMakeRect(2, 21, 382, 16);
    [[self headlineLabelField] setFrame:progressLabelFrame];
    [[self headlineLabelField] setHidden:YES];
    [[self headlineLabelField] setAutoresizingMask:NSViewMaxYMargin|NSViewWidthSizable];
    labelCell = [[OECenteredTextFieldCell alloc] init];
    
    shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor blackColor]];
    [shadow setShadowBlurRadius:0];
    [shadow setShadowOffset:(NSSize){0,-1}];
    
    font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:0 size:11.0];
    
    paraStyle = [[NSMutableParagraphStyle alloc] init];
    [paraStyle setAlignment:NSCenterTextAlignment];
    [labelCell setTextAttributes:[NSDictionary dictionaryWithObjectsAndKeys:
                                  [NSColor whiteColor]  , NSForegroundColorAttributeName,
                                  paraStyle             , NSParagraphStyleAttributeName,
                                  shadow                , NSShadowAttributeName,
                                  font                 , NSFontAttributeName,
                                  nil]];
    [labelCell setupAttributes];
    [paraStyle release];
    [shadow release];
    [[self headlineLabelField] setCell:labelCell];
    [labelCell release];
    [[self boxView] addSubview:[self headlineLabelField]];
    
    // Setup Suppression Button
    [[self suppressionButton] setTitle:NSLocalizedString(@"Do not ask me again", @"")];
    [[self suppressionButton] setAutoresizingMask:NSViewMaxXMargin|NSViewMaxYMargin];
    [[self suppressionButton] setFrame:(NSRect){{18,18}, {150, 18}}];       
    [[self suppressionButton] setHidden:YES]; 
    [[_window contentView] addSubview:[self suppressionButton]];
}

@end
#pragma mark -
@implementation OEAlertWindow
+ (void)initialize
{
    if([NSImage imageNamed:@"hud_alert_window_active"]) return;
    
    NSImage* hudWindowBorder = [NSImage imageNamed:@"hud_alert_window"];    
    [hudWindowBorder setName:@"hud_alert_window_active" forSubimageInRect:(NSRect){{0,0},{29,47}}];
    [hudWindowBorder setName:@"hud_alert_window_inactive" forSubimageInRect:(NSRect){{0,0},{29,47}}];
    
    [NSWindow registerWindowClassForCustomThemeFrameDrawing:[OEAlertWindow class]];
}

- (id)init {
    self = [super init];
    if (self) {
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
- (void)drawThemeFrame:(NSValue *)dirtyRectValue
{   
    NSRect bounds = [self frame];
    bounds.origin = (NSPoint){0,0};
    
    NSImage* image = [NSImage imageNamed:@"hud_alert_window"];
    [image drawInRect:bounds fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:nil leftBorder:14 rightBorder:14 topBorder:24 bottomBorder:22];
}
@end