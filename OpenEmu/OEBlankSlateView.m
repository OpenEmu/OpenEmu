/*
 Copyright (c) 2012, OpenEmu Team
 
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

#import "OEBlankSlateView.h"

#import "OESystemPlugin.h"
#import "OECorePlugin.h"

#import "OEBlankSlateForegroundLayer.h"

#import "OEButton.h"
#import "OEPopUpButtonCell.h"
#import "OECenteredTextFieldCell.h"

#import "NSColor+OEAdditions.h"

@interface OEBlankSlateView ()
@property CALayer *dragIndicationLayer;
@property NSDragOperation lastDragOperation;
- (void)OE_commonBlankSlateInit;
- (void)OE_showView:(NSView*)view;
- (void)OE_setupView:(NSView*)view withCollectionName:(NSString *)representedCollectionName;
- (void)OE_setupView:(NSView*)view withSystemPlugin:(OESystemPlugin *)systemPlugin;

- (void)OE_addLeftHeadlineWithText:(NSString*)text toView:(NSView*)view;
- (void)OE_setupBoxInView:(NSView*)view;
@end

// TODO: convert defines to constants
#pragma mark - Sizes
#define boxHeight 261
#define arrowTopToViewTop 52
#define dndTextTopToViewTop 202

#define bottomTextViewHeight 49
#define bottomHeadlineHeight 21
#define bottomHeadlineTopToViewTop 296

#define coreIconTopToViewTop 312
#define coreIconX 263

#define rightColumnX 309

#define ViewWidth 427
#define ViewHeight 366
#pragma mark -

@implementation OEBlankSlateView

- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if (self) {
        [self OE_commonBlankSlateInit];
    }
    return self;
}

- (void)awakeFromNib
{
    [super awakeFromNib];
    [self OE_commonBlankSlateInit];
}

- (void)OE_commonBlankSlateInit
{
    CALayer *layer = [self layer];

    // Setup foreground

    OEBlankSlateForegroundLayer *foregroundLayer = [[OEBlankSlateForegroundLayer alloc] init];
    [layer addSublayer:foregroundLayer];

    // Setup drag indication layer
    _dragIndicationLayer = [[CALayer alloc] init];
    [_dragIndicationLayer setBorderColor:[[NSColor colorWithDeviceRed:0.03 green:0.41 blue:0.85 alpha:1.0] CGColor]];
    [_dragIndicationLayer setBorderWidth:2.0];
    [_dragIndicationLayer setCornerRadius:8.0];
    [_dragIndicationLayer setHidden:YES];
    [layer addSublayer:_dragIndicationLayer];
}

- (void)gotoProjectURL:(id)sender
{
    NSString *urlString = [[sender cell] representedObject];
    NSURL    *url       = [NSURL URLWithString:urlString];
    [[NSWorkspace sharedWorkspace] openURL:url];
}
#pragma mark -
- (void)setRepresentedCollectionName:(NSString *)representedCollectionName
{
    if(representedCollectionName == _representedCollectionName) return;
    _representedCollectionName = representedCollectionName;
    _representedSystemPlugin = nil;
        
    NSView *view = [[NSView alloc] initWithFrame:(NSRect){ .size = { ViewWidth, ViewHeight }}];

    [self OE_setupView:view withCollectionName:representedCollectionName];
    [self OE_showView:view];
}

- (void)OE_setupView:(NSView*)view withCollectionName:(NSString *)collectionName
{
    [self OE_setupBoxInView:view];
    [self OE_addLeftHeadlineWithText:NSLocalizedString(@"Collections", @"") toView:view];

    NSRect rect = (NSRect){ .size = { [view frame].size.width, bottomTextViewHeight }};
    
    NSTextView *textView = [[NSTextView alloc] initWithFrame:NSInsetRect(rect, -4, 0)];
    NSString   *text     = [NSString stringWithFormat:NSLocalizedString(@"Create a personal game selection. To add to a collection, select a game from your console library and drag it to ’%@’ on the left.", @""), collectionName];
    [textView setString:text];
    [textView setDrawsBackground:NO];
    [textView setEditable:NO];
    [textView setSelectable:NO];
    [textView setFont:[[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0]];
    [textView setTextColor:[NSColor colorWithDeviceWhite:0.86 alpha:1.0]];
    [textView setTextContainerInset:NSMakeSize(0, 0)];
    
    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor blackColor]];
    [shadow setShadowBlurRadius:0];
    [shadow setShadowOffset:(NSSize){0, -1}];
    [textView setShadow:shadow];
    
    [view addSubview:textView];
}

- (void)setRepresentedSystemPlugin:(OESystemPlugin *)representedSystemPlugin
{
    if(representedSystemPlugin == _representedSystemPlugin) return;
    _representedSystemPlugin = representedSystemPlugin;
    _representedCollectionName = nil;
    
    NSView *view = [[NSView alloc] initWithFrame:(NSRect){ .size = { ViewWidth, ViewHeight }}];
    [self OE_setupView:view withSystemPlugin:representedSystemPlugin];
    [self OE_showView:view];
}

- (void)OE_setupView:(NSView*)view withSystemPlugin:(OESystemPlugin *)plugin
{
    [self OE_setupBoxInView:view];
    [self OE_addLeftHeadlineWithText:(plugin ? [plugin systemName] : NSLocalizedString(@"System", @"")) toView:view];
    
    NSRect      rect     = (NSRect){ .size = {[view frame].size.width/12*7, bottomTextViewHeight}};
    NSTextView *textView = [[NSTextView alloc] initWithFrame:NSInsetRect(rect, -4, 0)];
    NSString   *text     = [NSString stringWithFormat:NSLocalizedString(@"%@ games you add to OpenEmu will appear in this Console Library", @""), [plugin systemName]];
    [textView setDrawsBackground:NO];
    [textView setEditable:NO];
    [textView setSelectable:NO];
    [textView setFont:[[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0]];
    [textView setTextColor:[NSColor colorWithDeviceWhite:0.86 alpha:1.0]];
    [textView setTextContainerInset:NSMakeSize(0, 0)];
    
    NSMutableParagraphStyle *paraStyle = [[NSMutableParagraphStyle alloc] init];
    [paraStyle setLineSpacing:2];
    [textView setDefaultParagraphStyle:paraStyle];
    [textView setString:text];
    
    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor blackColor]];
    [shadow setShadowBlurRadius:0];
    [shadow setShadowOffset:(NSSize){0,-1}];
    [textView setShadow:shadow];
    
    [view addSubview:textView];
    
    NSImageView *coreIconView = [[NSImageView alloc] initWithFrame:(NSRect){{coreIconX, [view frame].size.height-40-coreIconTopToViewTop},{40, 40}}];
    [coreIconView setImage:[NSImage imageNamed:@"blank_slate_core_icon"]];
    [view addSubview:coreIconView];
    [coreIconView unregisterDraggedTypes];
    
    OECenteredTextFieldCell *cell = [[OECenteredTextFieldCell alloc] initTextCell:@""];
    shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:1.0]];
    [shadow setShadowOffset:(NSSize){0, -1}];
    [shadow setShadowBlurRadius:0];
    
    NSDictionary *dictionary = [[NSDictionary alloc] initWithObjectsAndKeys:
                                [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:15 size:11.0], NSFontAttributeName,
                                shadow, NSShadowAttributeName,
                                [NSColor colorWithDeviceWhite:1.0 alpha:1.0], NSForegroundColorAttributeName,
                                nil];
    [cell setTextAttributes:dictionary];
    
    NSTextField *coreSuppliedByLabel = [[NSTextField alloc] initWithFrame:(NSRect){{rightColumnX, bottomTextViewHeight-16},{[view frame].size.width-rightColumnX, 17}}];
    [coreSuppliedByLabel setCell:cell];
    [coreSuppliedByLabel setEditable:NO];
    [coreSuppliedByLabel setSelectable:NO];
    [coreSuppliedByLabel setDrawsBackground:NO];
    [coreSuppliedByLabel setBezeled:NO];
    [coreSuppliedByLabel setStringValue:NSLocalizedString(@"Core Provided By…", @"")];
    [view addSubview:coreSuppliedByLabel];
    
    // Get core plugins that can handle the system
    NSPredicate *pluginFilter = [NSPredicate predicateWithBlock: ^ BOOL(OECorePlugin *evaluatedPlugin, NSDictionary *bindings) {
        return [[evaluatedPlugin systemIdentifiers] containsObject:[plugin systemIdentifier]];
    }];
    NSArray *pluginsForSystem = [[OECorePlugin allPlugins] filteredArrayUsingPredicate:pluginFilter];
    [pluginsForSystem enumerateObjectsUsingBlock:^(OECorePlugin *core, NSUInteger idx, BOOL *stop) {
        NSString *projectURL = [[core infoDictionary] valueForKey:@"OEProjectURL"];
        NSString *name       = [core displayName];
        
        // Create weblink button for current core
        OEButton *gotoButton = [[OEButton alloc] initWithFrame:(NSRect){{ rightColumnX, bottomTextViewHeight - 16 * (idx+2) -1}, { [view frame].size.width - rightColumnX, 20 }}];
        [gotoButton setAutoresizingMask:NSViewWidthSizable];
        [gotoButton setAlignment:NSLeftTextAlignment];
        [gotoButton setImagePosition:NSImageRight];
        [gotoButton setThemeKey:@"open_weblink"];
        [gotoButton setTarget:self];
        [gotoButton setAction:@selector(gotoProjectURL:)];
        [gotoButton setTitle:name];
        [gotoButton setToolTip:[NSString stringWithFormat:NSLocalizedString(@"Takes you to the %@ project website", @"Weblink tooltip"), name]];
        [[gotoButton cell] setRepresentedObject:projectURL];
        [gotoButton sizeToFit];
        
        [view addSubview:gotoButton];
    }];
}

- (void)OE_addLeftHeadlineWithText:(NSString*)text toView:(NSView*)view
{
    OECenteredTextFieldCell *cell   = [[OECenteredTextFieldCell alloc] initTextCell:@""];
    NSShadow                *shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:1.0]];
    [shadow setShadowOffset:(NSSize){0, -1}];
    [shadow setShadowBlurRadius:0];
    
    NSDictionary *dictionary = [[NSDictionary alloc] initWithObjectsAndKeys:
                                [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:15 size:11.0], NSFontAttributeName,
                                shadow, NSShadowAttributeName,
                                [NSColor colorWithDeviceWhite:1.0 alpha:1.0], NSForegroundColorAttributeName,
                                nil];
    [cell setTextAttributes:dictionary];
    
    NSTextField *headlineField = [[NSTextField alloc] initWithFrame:(NSRect){{-2, [view frame].size.height-21-bottomHeadlineTopToViewTop},{[view frame].size.width, 21}}];
    [headlineField setCell:cell];
    [headlineField setStringValue:text];
    [headlineField setEditable:NO];
    [headlineField setSelectable:NO];
    [headlineField setDrawsBackground:NO];
    [headlineField setBezeled:NO];
    
    [view addSubview:headlineField];
}

- (void)OE_setupBoxInView:(NSView*)view
{
    NSImageView *boxImageView = [[NSImageView alloc] initWithFrame:(NSRect){{0,[view frame].size.height-boxHeight},{[view frame].size.width,boxHeight}}];
    [boxImageView setImage:[NSImage imageNamed:@"blank_slate_box"]];
    [view addSubview:boxImageView];
    [boxImageView unregisterDraggedTypes];
    
    NSImageView *arrowImageView = [[NSImageView alloc] initWithFrame:(NSRect){{(round([view frame].size.width-100)/2), [view frame].size.height-124-arrowTopToViewTop},{100, 124}}];
    [arrowImageView setImage:[NSImage imageNamed:@"blank_slate_arrow"]];
    [view addSubview:arrowImageView];
    [arrowImageView unregisterDraggedTypes];
    
    OECenteredTextFieldCell *defaultCell = [[OECenteredTextFieldCell alloc] initTextCell:@""];
    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:1.0 alpha:0.1]];
    [shadow setShadowOffset:(NSSize){0, -1}];
    [shadow setShadowBlurRadius:0];
    
    NSMutableParagraphStyle *style = [[NSMutableParagraphStyle alloc] init];
    [style setAlignment:NSCenterTextAlignment];
    NSDictionary *dictionary = [[NSDictionary alloc] initWithObjectsAndKeys:
                                [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:10 size:18.0], NSFontAttributeName,
                                style, NSParagraphStyleAttributeName,
                                shadow, NSShadowAttributeName,
                                [NSColor colorWithDeviceWhite:0.11 alpha:1.0], NSForegroundColorAttributeName,
                                nil];
    [defaultCell setTextAttributes:dictionary];
    
    OECenteredTextFieldCell *glowCell = [[OECenteredTextFieldCell alloc] initTextCell:@""];
    shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:1.0 alpha:0.05]];
    [shadow setShadowOffset:(NSSize){0, 0}];
    [shadow setShadowBlurRadius:2];
    
    dictionary = [[NSDictionary alloc] initWithObjectsAndKeys:
                  [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:10 size:18.0], NSFontAttributeName,
                  style, NSParagraphStyleAttributeName,
                  shadow, NSShadowAttributeName,
                  [NSColor colorWithDeviceWhite:0.11 alpha:0.0], NSForegroundColorAttributeName,
                  nil];
    [glowCell setTextAttributes:dictionary];
    
    NSTextField *dragAndDropHereOuterGlowField = [[NSTextField alloc] initWithFrame:(NSRect){{0, [view frame].size.height-25-dndTextTopToViewTop},{[view frame].size.width, 25}}];
    [dragAndDropHereOuterGlowField setCell:glowCell];
    [dragAndDropHereOuterGlowField setStringValue:NSLocalizedString(@"Drag & Drop Games Here", @"Blank Slate DnD Here")];
    [dragAndDropHereOuterGlowField setEditable:NO];
    [dragAndDropHereOuterGlowField setSelectable:NO];
    [dragAndDropHereOuterGlowField setDrawsBackground:NO];
    [dragAndDropHereOuterGlowField setBezeled:NO];
    
    NSTextField *dragAndDropHereField = [[NSTextField alloc] initWithFrame:(NSRect){{0, [view frame].size.height-25-dndTextTopToViewTop},{[view frame].size.width, 25}}];
    [dragAndDropHereField setCell:defaultCell];
    [dragAndDropHereField setStringValue:NSLocalizedString(@"Drag & Drop Games Here", @"Blank Slate DnD Here")];
    [dragAndDropHereField setEditable:NO];
    [dragAndDropHereField setSelectable:NO];
    [dragAndDropHereField setDrawsBackground:NO];
    [dragAndDropHereField setBezeled:NO];
    [view addSubview:dragAndDropHereField];
    [view addSubview:dragAndDropHereOuterGlowField];
}


- (void)OE_showView:(NSView*)view
{    
    // Remove current blank slate subview
    [[[self subviews] lastObject] removeFromSuperview];
    
    NSRect  bounds      = [self bounds];
    NSSize  viewSize    = [view frame].size;
    NSRect  viewFrame   = NSMakeRect(ceil((NSWidth(bounds) - viewSize.width) / 2.0), ceil((NSHeight(bounds) - viewSize.height) / 2.0), viewSize.width, viewSize.height);
    [view setAutoresizingMask:NSViewMaxXMargin|NSViewMinXMargin|NSViewMaxYMargin|NSViewMinYMargin];
    [view setFrame:viewFrame];
    [self addSubview:view];
}

- (void)layoutSublayersOfLayer:(CALayer *)theLayer
{
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    
    const NSRect bounds = [self bounds];
    [[theLayer sublayers] enumerateObjectsUsingBlock:^(CALayer *obj, NSUInteger idx, BOOL *stop) {
        if(obj == _dragIndicationLayer) [obj setFrame:NSInsetRect(bounds, 1.0, 1.0)];
        else if(obj!=[[[self subviews] lastObject] layer]) [obj setFrame:bounds];
    }];
    
    [CATransaction flush];
}

#pragma mark -
#pragma mark NSDraggingDestination
- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
    // The delegate has to be able to validate and accept drops, if it can't do then then there is no need to drag anything around
    if([_delegate respondsToSelector:@selector(blankSlateView:validateDrop:)] && [_delegate respondsToSelector:@selector(blankSlateView:acceptDrop:)])
    {
        _lastDragOperation = [_delegate blankSlateView:self validateDrop:sender];
    }
    
    [_dragIndicationLayer setHidden:(_lastDragOperation==NSDragOperationNone)];
    
    return _lastDragOperation;
}

- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender
{
    if([_delegate respondsToSelector:@selector(blankSlateView:draggingUpdated:)])
    {
        _lastDragOperation = [_delegate blankSlateView:self draggingUpdated:sender];
    }
    
    [_dragIndicationLayer setHidden:(_lastDragOperation==NSDragOperationNone)];
    
    return _lastDragOperation;
}

- (void)draggingExited:(id<NSDraggingInfo>)sender
{
    [_dragIndicationLayer setHidden:YES];
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    [_dragIndicationLayer setHidden:YES];
    return [_delegate respondsToSelector:@selector(blankSlateView:acceptDrop:)] && [_delegate blankSlateView:self acceptDrop:sender];
}

#pragma mark - Properties
@synthesize delegate=_delegate;
@synthesize lastDragOperation=_lastDragOperation;
@synthesize representedCollectionName=_representedCollectionName;
@synthesize representedSystemPlugin=_representedSystemPlugin;
@synthesize dragIndicationLayer=_dragIndicationLayer;
@end
