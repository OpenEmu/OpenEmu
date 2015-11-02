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

#import "OEDBSystem.h"
#import "OESystemPlugin.h"
#import "OECorePlugin.h"
#import "OEDBSavedGamesMedia.h"
#import "OEDBScreenshotsMedia.h"

#import "OEBlankSlateForegroundLayer.h"
#import "OECollectionViewItemProtocol.h"
#import "OEGameCollectionViewItemProtocol.h"
#import "OEDBDataSourceAdditions.h"

#import "OEButton.h"
#import "OEPopUpButtonCell.h"
#import "OECenteredTextFieldCell.h"
#import "OEArrowCursorTextView.h"

#import "NSColor+OEAdditions.h"

@interface OEBlankSlateView () <NSTextViewDelegate>
@property CALayer *dragIndicationLayer;
@property NSDragOperation lastDragOperation;
- (void)commonBlankSlateInit;
- (void)OE_setupViewWithCollectionName:(NSString *)representedCollectionName;
- (void)OE_setupViewWithSystemPlugin:(OESystemPlugin *)systemPlugin;

- (void)OE_setupDragAndDropBox;

@property (nonatomic) NSString       *representedCollectionName;
@property (nonatomic) OESystemPlugin *representedSystemPlugin;
@property (readwrite, strong) NSView *containerView;
@end

#pragma mark - Sizes
const CGFloat OEBlankSlateContainerWidth = 427.0;
const CGFloat OEBlankSlateContainerHeight = 382.0;

// Sizes defining box
const CGFloat OEBlankSlateBoxHeight        = 261.0; // height of box
const CGFloat OEBlankSlateBoxImageToTop    =  52.0; // image top to view top distance
const CGFloat OEBlankSlateBoxImageToBottom =  70.0; // image bottom to view bottom distance
const CGFloat OEBlankSlateBoxTextToTop     = 202.0; // distance of box top to text

const CGFloat OEBlankSlateHeadlineHeight   =  21.0; // height of headline
const CGFloat OEBlankSlateHeadlineToTop    = 296.0; // space between headline and view top

const CGFloat OEBlankSlateBottomTextHeight =  49.0; // height of instructional text
const CGFloat OEBlankSlateBottomTextTop = 317.0;

const CGFloat OEBlankSlateCoreToTop = 312.0; // space between core icon and view top
const CGFloat OEBlankSlateCoreX     = 263.0; // x coordinate of core icon
const CGFloat OEBlankSlateRightColumnX = 309.0;

NSString * const OECDBasedGamesUserGuideURLString = @"https://github.com/OpenEmu/OpenEmu/wiki/User-guide:-CD-based-games";

@implementation OEBlankSlateView

- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if (self) {
        [self commonBlankSlateInit];
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) {
        [self commonBlankSlateInit];
    }
    return self;
}

- (void)commonBlankSlateInit
{
    CALayer *layer = [self layer];

    // Setup foreground
    // TODO: cleanup
    // OEBlankSlateForegroundLayer *foregroundLayer = [[OEBlankSlateForegroundLayer alloc] init];
    // [layer addSublayer:foregroundLayer];

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
- (void)setRepresentedObject:(id)representedObject
{
    if(_representedObject == representedObject) return;
    _representedObject = representedObject;

    NSView *container = [[NSView alloc] initWithFrame:(NSRect){ .size = { OEBlankSlateContainerWidth, OEBlankSlateContainerHeight }}];
    [self setContainerView:container];

    [self setupViewForRepresentedObject];

    // Remove current blank slate subview
    [[[self subviews] lastObject] removeFromSuperview];

    NSRect  bounds   = [self bounds];
    NSSize  viewSize = [[self containerView] frame].size;

    NSRect  viewFrame   = NSMakeRect(ceil((NSWidth(bounds) - viewSize.width) / 2.0), ceil((NSHeight(bounds) - viewSize.height) / 2.0), viewSize.width, viewSize.height);
    [[self containerView] setAutoresizingMask:NSViewMaxXMargin|NSViewMinXMargin|NSViewMaxYMargin|NSViewMinYMargin];
    [[self containerView] setFrame:viewFrame];
    [self addSubview:[self containerView]];
}

- (void)setupViewForRepresentedObject
{
    id representedObject = [self representedObject];

    if([representedObject isKindOfClass:[OEDBSystem class]])
    {
        [self setRepresentedSystemPlugin:[(OEDBSystem*)representedObject plugin]];
    }
    else if([representedObject conformsToProtocol:@protocol(OEGameCollectionViewItemProtocol)])
    {
        [self setRepresentedCollectionName:[representedObject collectionViewName]];
    }
    else if([representedObject isKindOfClass:[OEDBSavedGamesMedia class]] || [representedObject isKindOfClass:[OEDBScreenshotsMedia class]])
    {
        [self setRepresentedMediaType:representedObject];
    }
    else
    {
        DLog(@"Unknown represented object: %@ %@", [representedObject className], representedObject);
    }
}

- (void)setRepresentedCollectionName:(NSString *)representedCollectionName
{
    _representedCollectionName = representedCollectionName;
    _representedSystemPlugin = nil;

    [self OE_setupViewWithCollectionName:representedCollectionName];
}

- (void)OE_setupViewWithCollectionName:(NSString *)collectionName
{
    [self OE_setupDragAndDropBox];
    [self addLeftHeadlineWithText:NSLocalizedString(@"Collections", @"")];

    NSString *text = [NSString stringWithFormat:NSLocalizedString(@"Create a personal game selection. To add to a collection, select a game from your console library and drag it to ’%@’ on the left.", @""), collectionName];
    [self addInformationalText:text];
}

- (void)setRepresentedSystemPlugin:(OESystemPlugin *)representedSystemPlugin
{
    _representedSystemPlugin = representedSystemPlugin;
    _representedCollectionName = nil;

    [self OE_setupViewWithSystemPlugin:representedSystemPlugin];
}

- (void)OE_setupViewWithSystemPlugin:(OESystemPlugin *)plugin
{
    NSView *container = [self containerView];
    NSRect containerFrame = [container frame];

    [self OE_setupDragAndDropBox];
    [self addLeftHeadlineWithText:(plugin ? [plugin systemName] : NSLocalizedString(@"System", @""))];
    
    NSRect      rect     = (NSRect){ .size = {NSWidth(containerFrame)/12*7, OEBlankSlateBottomTextHeight}};
    rect.origin.y = NSHeight(containerFrame)-NSHeight(rect)-OEBlankSlateBottomTextTop;
    NSTextView *textView = [[OEArrowCursorTextView alloc] initWithFrame:NSInsetRect(rect, -4, 0)];

    NSString *textFormat = nil;

    if([plugin supportsDiscs]) {
        textFormat = NSLocalizedString(@"%@ games will appear here. Check out %@ on how to add disc-based games.", @"");
    } else {
        textFormat = NSLocalizedString(@"%@ games you add to OpenEmu will appear in this Console Library", @"");
    }

    NSString *text = [NSString stringWithFormat:textFormat, [plugin systemName], NSLocalizedString(@"this guide", @"this guide")];

    [textView setDrawsBackground:NO];
    [textView setEditable:NO];
    [textView setSelectionGranularity:NSSelectByCharacter];
    [textView setDelegate:self];
    [textView setFont:[NSFont systemFontOfSize:11]];
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

    NSMutableDictionary *attributes = [[textView typingAttributes] mutableCopy];
    [attributes setObject:[NSCursor arrowCursor] forKey:NSCursorAttributeName];
    [textView setTypingAttributes:attributes];
    [textView setMarkedTextAttributes:attributes];
    [textView setSelectedTextAttributes:attributes];

    if([plugin supportsDiscs]) {
        NSRange guideLinkRange = [[textView string] rangeOfString:NSLocalizedString(@"this guide", @"this guide")];
        NSDictionary *guideLinkAttribtues = @{
                                              NSLinkAttributeName: [NSURL URLWithString:OECDBasedGamesUserGuideURLString],
                                              };
        [[textView textStorage] setAttributes:guideLinkAttribtues range:guideLinkRange];
    }
    NSMutableDictionary *linkAttributes = [attributes mutableCopy];
    [linkAttributes setObject:@(NSUnderlineStyleSingle) forKey:NSUnderlineStyleAttributeName];
    [linkAttributes setObject:[NSCursor pointingHandCursor] forKey:NSCursorAttributeName];
    [textView setLinkTextAttributes:linkAttributes];
    
    [container addSubview:textView];

    rect   = (NSRect){{OEBlankSlateCoreX, NSHeight(containerFrame)-40-OEBlankSlateCoreToTop},{40, 40}};
    NSImageView *coreIconView = [[NSImageView alloc] initWithFrame:rect];
    [coreIconView setImage:[NSImage imageNamed:@"blank_slate_core_icon"]];
    [container addSubview:coreIconView];
    [coreIconView unregisterDraggedTypes];
    
    OECenteredTextFieldCell *cell = [[OECenteredTextFieldCell alloc] initTextCell:@""];
    shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:1.0]];
    [shadow setShadowOffset:(NSSize){0, -1}];
    [shadow setShadowBlurRadius:0];
    
    NSDictionary *dictionary = [[NSDictionary alloc] initWithObjectsAndKeys:
                                [NSFont systemFontOfSize:11], NSFontAttributeName,
                                shadow, NSShadowAttributeName,
                                [NSColor colorWithDeviceWhite:1.0 alpha:1.0], NSForegroundColorAttributeName,
                                nil];
    [cell setTextAttributes:dictionary];

    NSTextField *coreSuppliedByLabel = [[NSTextField alloc] initWithFrame:(NSRect){{OEBlankSlateRightColumnX, NSHeight(containerFrame)-16-OEBlankSlateBottomTextTop},{NSWidth(containerFrame)-OEBlankSlateRightColumnX, 17}}];
    [coreSuppliedByLabel setCell:cell];
    [coreSuppliedByLabel setEditable:NO];
    [coreSuppliedByLabel setSelectable:NO];
    [coreSuppliedByLabel setDrawsBackground:NO];
    [coreSuppliedByLabel setBezeled:NO];
    [coreSuppliedByLabel setStringValue:NSLocalizedString(@"Core Provided By…", @"")];
    [container addSubview:coreSuppliedByLabel];
    
    // Get core plugins that can handle the system
    NSPredicate *pluginFilter = [NSPredicate predicateWithBlock: ^ BOOL(OECorePlugin *evaluatedPlugin, NSDictionary *bindings) {
        return [[evaluatedPlugin systemIdentifiers] containsObject:[plugin systemIdentifier]];
    }];
    NSArray *pluginsForSystem = [[OECorePlugin allPlugins] filteredArrayUsingPredicate:pluginFilter];
    [pluginsForSystem enumerateObjectsUsingBlock:^(OECorePlugin *core, NSUInteger idx, BOOL *stop) {
        NSString *projectURL = [[core infoDictionary] valueForKey:@"OEProjectURL"];
        NSString *name       = [core displayName];

        // Create weblink button for current core
        NSRect frame = (NSRect){{ OEBlankSlateRightColumnX, NSHeight(containerFrame)-2*16-OEBlankSlateBottomTextTop - 16 * idx}, { NSWidth(containerFrame) - OEBlankSlateRightColumnX, 20 }};
        OEButton *gotoButton = [[OEButton alloc] initWithFrame:frame];
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
        
        [container addSubview:gotoButton];
    }];
}

- (void)addLeftHeadlineWithText:(NSString*)text
{
    NSView *container = [self containerView];
    NSRect containerFrame = [container frame];

    OECenteredTextFieldCell *cell   = [[OECenteredTextFieldCell alloc] initTextCell:@""];
    NSShadow                *shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:1.0]];
    [shadow setShadowOffset:(NSSize){0, -1}];
    [shadow setShadowBlurRadius:0];
    
    NSDictionary *dictionary = [[NSDictionary alloc] initWithObjectsAndKeys:
                                [NSFont systemFontOfSize:11], NSFontAttributeName,
                                shadow, NSShadowAttributeName,
                                [NSColor colorWithDeviceWhite:1.0 alpha:1.0], NSForegroundColorAttributeName,
                                nil];
    [cell setTextAttributes:dictionary];
    
    NSTextField *headlineField = [[NSTextField alloc] initWithFrame:(NSRect){{-2, NSHeight(containerFrame)-21-OEBlankSlateHeadlineToTop},{NSWidth(containerFrame), 21}}];
    [headlineField setCell:cell];
    [headlineField setStringValue:text];
    [headlineField setEditable:NO];
    [headlineField setSelectable:NO];
    [headlineField setDrawsBackground:NO];
    [headlineField setBezeled:NO];
    
    [container addSubview:headlineField];
}


- (void)OE_setupDragAndDropBox
{
    NSImageView *imageView = [self OE_makeImageViewWithImageNamed:@"blank_slate_arrow"];
    [self setupBoxWithText:NSLocalizedString(@"Drag & Drop Games Here", @"Blank Slate DnD Here") andImageView:imageView];
}

- (void)setupBoxWithText:(NSString*)text andImageView:(NSView*)arrowImageView
{
    NSView *container = [self containerView];
    NSRect containerFrame = [container frame];

    NSImageView *boxImageView = [[NSImageView alloc] initWithFrame:(NSRect){{0,NSHeight(containerFrame)-OEBlankSlateBoxHeight},{NSWidth(containerFrame),OEBlankSlateBoxHeight}}];
    [boxImageView setImage:[NSImage imageNamed:@"blank_slate_box"]];
    [container addSubview:boxImageView];
    [boxImageView unregisterDraggedTypes];

    CGFloat height = OEBlankSlateBoxHeight-OEBlankSlateBoxImageToBottom-OEBlankSlateBoxImageToTop;
    CGFloat width  = 300;
    [arrowImageView setFrame:(NSRect){{(round(NSWidth(containerFrame)-width)/2), NSHeight(containerFrame)-height-OEBlankSlateBoxImageToTop},{width, height}}];
    [container addSubview:arrowImageView];

    OECenteredTextFieldCell *defaultCell = [[OECenteredTextFieldCell alloc] initTextCell:@""];
    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:1.0 alpha:0.1]];
    [shadow setShadowOffset:(NSSize){0, -1}];
    [shadow setShadowBlurRadius:0];

    NSMutableParagraphStyle *style = [[NSMutableParagraphStyle alloc] init];
    [style setAlignment:NSCenterTextAlignment];
    NSDictionary *dictionary = [[NSDictionary alloc] initWithObjectsAndKeys:
                                [NSFont boldSystemFontOfSize:18], NSFontAttributeName,
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
                  [NSFont boldSystemFontOfSize:18], NSFontAttributeName,
                  style, NSParagraphStyleAttributeName,
                  shadow, NSShadowAttributeName,
                  [NSColor colorWithDeviceWhite:0.11 alpha:0.0], NSForegroundColorAttributeName,
                  nil];
    [glowCell setTextAttributes:dictionary];

    NSTextField *dragAndDropHereOuterGlowField = [[NSTextField alloc] initWithFrame:(NSRect){{0, NSHeight(containerFrame)-25-OEBlankSlateBoxTextToTop},{NSWidth(containerFrame), 25}}];
    [dragAndDropHereOuterGlowField setCell:glowCell];
    [dragAndDropHereOuterGlowField setStringValue:text];
    [dragAndDropHereOuterGlowField setEditable:NO];
    [dragAndDropHereOuterGlowField setSelectable:NO];
    [dragAndDropHereOuterGlowField setDrawsBackground:NO];
    [dragAndDropHereOuterGlowField setBezeled:NO];

    NSTextField *dragAndDropHereField = [[NSTextField alloc] initWithFrame:(NSRect){{0, NSHeight(containerFrame)-25-OEBlankSlateBoxTextToTop},{NSWidth(containerFrame), 25}}];
    [dragAndDropHereField setCell:defaultCell];
    [dragAndDropHereField setStringValue:text];
    [dragAndDropHereField setEditable:NO];
    [dragAndDropHereField setSelectable:NO];
    [dragAndDropHereField setDrawsBackground:NO];
    [dragAndDropHereField setBezeled:NO];
    [container addSubview:dragAndDropHereField];
    [container addSubview:dragAndDropHereOuterGlowField];
}

- (void)setRepresentedMediaType:(id <OESidebarItem>)item
{
    if(_representedCollectionName == [item sidebarName])
        return;

    _representedSystemPlugin = nil;
    _representedCollectionName = [item sidebarName];

    if([_representedObject isKindOfClass:[OEDBScreenshotsMedia class]])
    {
        [self OE_setupScreenShotBox];
        [self OE_setupScreenShotText];
    } else {
        NSImageView *imageView = [self OE_makeImageViewWithImageNamed:@"blank_slate_arrow"];
        [self setupBoxWithText:NSLocalizedString(@"Drag & Drop Save States Here", @"Blank Slate DnD Save States Here") andImageView:imageView];

        [self OE_setupSaveStateText];
    }
}

- (void)OE_setupScreenShotBox
{
    NSImageView *imageView = [self OE_makeImageViewWithImageNamed:@"blank_slate_camera"];
    [self setupBoxWithText:NSLocalizedString(@"No Screenshots Found", @"No screen shots found") andImageView:imageView];
}

- (void)OE_setupScreenShotText
{
    [self addLeftHeadlineWithText:NSLocalizedString(@"Screenshots", @"")];

    NSString *text = NSLocalizedString(@"Create your personal collection of screenshots. To take a screenshot, you can use the keyboard shortcut ⌘ + t while playing a game.", @"");
    [self addInformationalText:text];
}

- (void)OE_setupSaveStateText
{
    [self addLeftHeadlineWithText:NSLocalizedString(@"Save States", @"")];

    NSString *text = NSLocalizedString(@"With OpenEmu you can save your progress at any time using save states. We will even make auto saves for you, when you leave a game. Come back here to get an overview of all your saves.", @"");
    [self addInformationalText:text];
}

- (void)layoutSublayersOfLayer:(CALayer *)theLayer
{
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    
    const NSRect bounds = self.bounds;
    
    for (CALayer *layer in theLayer.sublayers)
    {
        if(layer == _dragIndicationLayer)
        {
            NSRect dragIndicationFrame = bounds;
            dragIndicationFrame.size.height = NSHeight(self.window.contentLayoutRect);
            
            layer.frame = NSInsetRect(dragIndicationFrame, 1.0, 1.0);
        }
        else if(layer!=self.subviews.lastObject.layer)
        {
            layer.frame = bounds;
        }
    }
    
    [CATransaction flush];
}

- (NSImageView*)OE_makeImageViewWithImageNamed:(NSString*)imageName{
    NSImageView *arrowImageView = [[NSImageView alloc] initWithFrame:NSZeroRect];
    [arrowImageView setImage:[NSImage imageNamed:imageName]];
    [arrowImageView setImageScaling:NSImageScaleNone];
    [arrowImageView setImageAlignment:NSImageAlignTop];
    [arrowImageView unregisterDraggedTypes];

    return arrowImageView;
}
#pragma mark - NSTextView Delegate
- (BOOL)textView:(NSTextView *)textView clickedOnLink:(id)link atIndex:(NSUInteger)charIndex
{
    [[NSWorkspace sharedWorkspace] openURL:link];
    return YES;
}

- (NSRange)textView:(NSTextView *)aTextView willChangeSelectionFromCharacterRange:(NSRange)oldSelectedCharRange toCharacterRange:(NSRange)newSelectedCharRange
{
    return NSMakeRange(0, 0);
}
#pragma mark - View Setup
- (void)addInformationalText:(NSString*)text
{
    NSView *container = [self containerView];
    NSRect containerFrame = [container frame];

    NSRect rect = (NSRect){ .size = { NSWidth(containerFrame), OEBlankSlateBottomTextHeight }};
    rect.origin.y = NSHeight(containerFrame)-NSHeight(rect)-OEBlankSlateBottomTextTop;

    NSTextView *textView = [[NSTextView alloc] initWithFrame:NSInsetRect(rect, -4, 0)];
    [textView setString:text];
    [textView setDrawsBackground:NO];
    [textView setEditable:NO];
    [textView setSelectable:NO];
    [textView setFont:[NSFont systemFontOfSize:11]];
    [textView setTextColor:[NSColor colorWithDeviceWhite:0.86 alpha:1.0]];
    [textView setTextContainerInset:NSMakeSize(0, 0)];

    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor blackColor]];
    [shadow setShadowBlurRadius:0];
    [shadow setShadowOffset:(NSSize){0, -1}];
    [textView setShadow:shadow];

    [container addSubview:textView];
}

#pragma mark - NSDraggingDestination
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
