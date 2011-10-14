//
//  OEAlert.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 14.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEAlert.h"

#import "OECheckBox.h"
#import "OEHUDButtonCell.h"
#import "OEPrefBoxPlain.h"

#import "NSImage+OEDrawingAdditions.h"
#import "OEHUDWindow.h"
#import "OEBackgroundColorView.h"
@interface OEAlertWindow : NSWindow @end
@interface OEAlert (Private)
- (void)performCallback;
- (void)_setupWindow;

- (void)setMessageText:(NSString*)messageText;
@end

@implementation OEAlert

+ (id)alertWithError:(NSError*)error
{
#warning Implement!
    return nil;
}
+ (id)alertWithMessageText:(NSString *)msgText defaultButton:(NSString*)defaultButtonLabel alternateButton:(NSString*)alternateButtonLabel
{
    OEAlert* alert = [[OEAlert alloc] init];
    
    alert.defaultButtonTitle = defaultButtonLabel;
    alert.alternateButtonTitle = alternateButtonLabel;
    [alert setMessageText:msgText];
    
    return [alert autorelease];
    
}
#pragma mark -
#pragma mark Memory Management
- (id)init
{
    self = [super init];
    if (self) 
    {
        _window = [[OEHUDWindow alloc] init];
        [_window setContentView:[[[OEBackgroundColorView alloc] init] autorelease]];
        [(OEBackgroundColorView*)_window.contentView setBackgroundColor:[NSColor colorWithDeviceWhite:0.15 alpha:.95]];
        
        _suppressionButton = [[OECheckBox alloc] init];
        
        _defaultButton = [[NSButton alloc] init];
        _alternateButton = [[NSButton alloc] init];
    }
    
    return self;
}

- (void)dealloc 
{
    NSLog(@"dealloc OEAlert");
    
    [_window release];
    
    // Free texts
    [_messageText release];
    
    self.defaultButtonTitle = nil;
    self.alternateButtonTitle = nil;
    
    // Remove Buttons
    [_defaultButton release];
    [_alternateButton release];
    
    // Remove Callbacks
    self.target = nil;
    self.callbackHandler = nil;
    
    // Remove suppression button stuff
    self.suppressionUDKey = nil;
    [_suppressionButton release];
    
    [super dealloc];
}
#pragma mark -
- (NSUInteger)runModal
{
    [self _setupWindow];
    
    if(self.showsSuppressionButton && self.suppressionButton.state == NSOnState){
        result = NSAlertDefaultReturn;
        [self performCallback];
        return NSAlertDefaultReturn;
    }
    
    return  [NSApp runModalForWindow:_window];
}
@synthesize result;

#pragma mark -
#pragma mark Buttons
@synthesize defaultButton=_defaultButton;
@synthesize alternateButton=_alternateButton;
@synthesize defaultButtonTitle, alternateButtonTitle;
- (void)buttonAction:(id)sender{
    if(sender == self.defaultButton)
    {
        result = NSAlertDefaultReturn;
        
        if(self.showsSuppressionButton && self.suppressionUDKey)
        {
            NSUserDefaults* standardUserDefaults = [NSUserDefaults standardUserDefaults];
            [standardUserDefaults setBool:self.suppressionButton.state==NSOnState forKey:self.suppressionUDKey];            
        }
    }
    else if(sender == self.alternateButton)
        result = NSAlertAlternateReturn;
    else
        result = NSAlertDefaultReturn;
    
    [NSApp stopModalWithCode:result];
    [self performCallback];
}

#pragma mark -
#pragma mark Callbacks
@synthesize target, callback, callbackHandler;
- (void)performCallback
{
    if(self.target && self.callback != NULL && [self.target respondsToSelector:self.callback])
        [self.target performSelectorOnMainThread:self.callback withObject:self waitUntilDone:NO];
    
    if(self.callbackHandler!=nil)
    {
        callbackHandler(self, self.result);
    }
}

#pragma mark -
#pragma mark Suppression Button
- (void)showSuppressionButtonForUDKey:(NSString*)key
{
    self.showsSuppressionButton = YES;
    self.suppressionUDKey = key;
}
@synthesize suppressionButton=_suppressionButton, showsSuppressionButton, suppressionUDKey;
#pragma mark -
#pragma mark Private Methods
- (void)_setupWindow
{
    [_window setContentSize:(NSSize){421, 172}];
    
    // Setup Buttons
    OEHUDButtonCell* cell = [[OEHUDButtonCell alloc] init];
    [cell setButtonColor:OEHUDButtonColorBlue];    
    [_defaultButton setCell:cell];
    [_defaultButton setFrame:(NSRect){{304, 14},{103, 23}}];
    [_defaultButton setAutoresizingMask:NSViewMinXMargin|NSViewMaxYMargin];
    [_defaultButton setTarget:self];
    [_defaultButton setAction:@selector(buttonAction:)];
    [_defaultButton setTitle:self.defaultButtonTitle];
    [cell release];
    [[_window contentView] addSubview:_defaultButton];
    
    cell = [[OEHUDButtonCell alloc] init];
    [cell setButtonColor:OEHUDButtonColorDefault];
    [_alternateButton setCell:cell];
    [_alternateButton setFrame:(NSRect){{190, 14},{103, 23}}];
    [_alternateButton setAutoresizingMask:NSViewMinXMargin|NSViewMaxYMargin];
    [_alternateButton setTarget:self];
    [_alternateButton setAction:@selector(buttonAction:)];
    [_alternateButton setTitle:self.alternateButtonTitle];
    [cell release];
    [[_window contentView] addSubview:_alternateButton];
    
    OEPrefBoxPlain* darkBox = [[OEPrefBoxPlain alloc] initWithFrame:(NSRect){{18, 51},{387, 82}}];
    [[_window contentView] addSubview:darkBox];
    [darkBox release];
    
    NSTextView* textView = [[NSTextView alloc] initWithFrame:NSInsetRect((NSRect){{0,0}, darkBox.frame.size}, 5, 5)];
    [textView setEditable:NO];
    [textView setSelectable:NO];
    [textView setDrawsBackground:NO];
    [textView setTextColor:[NSColor whiteColor]];
    
    [textView setString:_messageText];
    [darkBox addSubview:textView];
    
    // Setup suppression button
    if(self.showsSuppressionButton)
    {
        if(self.suppressionUDKey)
        {
            NSUserDefaults* standardUserDefaults = [NSUserDefaults standardUserDefaults];
            BOOL checked = [standardUserDefaults boolForKey:self.suppressionUDKey];
            [self.suppressionButton setState:checked];
        }
        
        [self.suppressionButton setTitle:NSLocalizedString(@"Do not ask me again", @"")];
        [self.suppressionButton setAutoresizingMask:NSViewMaxXMargin|NSViewMaxYMargin];
        [self.suppressionButton setFrame:(NSRect){{18,18}, {150, 18}}];       
        [[_window contentView] addSubview:self.suppressionButton];
    }
}


- (void)setMessageText:(NSString*)messageText
{
    NSString* newText = [messageText copy];
    [_messageText release];
    _messageText = newText;
}
@end
#pragma mark -
@implementation OEAlertWindow

+ (void)initialize
{
    if([NSImage imageNamed:@"hud_window_active"]) return;
    
    NSImage* hudWindowBorder = [NSImage imageNamed:@"hud_window"];    
    [hudWindowBorder setName:@"hud_window_active" forSubimageInRect:(NSRect){{0,0},{29,47}}];
    [hudWindowBorder setName:@"hud_window_inactive" forSubimageInRect:(NSRect){{0,0},{29,47}}];
}

@end