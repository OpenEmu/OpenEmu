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
@import OpenEmuKit;
#import "OEDBSavedGamesMedia.h"
#import "OEDBScreenshotsMedia.h"

#import "OECollectionViewItemProtocol.h"
#import "OEGameCollectionViewItemProtocol.h"
#import "OEDBDataSourceAdditions.h"

#import "OpenEmu-Swift.h"

@interface OEBlankSlateView () <NSTextViewDelegate>

@property CALayer *dragIndicationLayer;
@property NSDragOperation lastDragOperation;

@property (nonatomic) NSString       *representedCollectionName;
@property (nonatomic) OESystemPlugin *representedSystemPlugin;
@property (readwrite, strong) NSView *containerView;

@end

#pragma mark - Sizes

const CGFloat OEBlankSlateContainerWidth   = 427.0;
const CGFloat OEBlankSlateContainerHeight  = 418.0;

// Sizes defining box
const CGFloat OEBlankSlateBoxHeight        = 261.0; // height of box
const CGFloat OEBlankSlateBoxImageToTop    =  52.0; // image top to view top distance
const CGFloat OEBlankSlateBoxImageToBottom =  70.0; // image bottom to view bottom distance
const CGFloat OEBlankSlateBoxTextToTop     = 202.0; // distance of box top to text

const CGFloat OEBlankSlateHeadlineHeight   =  41.0; // height of headline
const CGFloat OEBlankSlateHeadlineToTop    = 337.0; // space between headline and view top

const CGFloat OEBlankSlateBottomTextHeight =  67.0; // height of instructional text
const CGFloat OEBlankSlateBottomTextTop    = 357.0;

const CGFloat OEBlankSlateCoreToTop        = 357.0; // space between core icon and view top
const CGFloat OEBlankSlateCoreX            = 263.0; // x coordinate of core icon
const CGFloat OEBlankSlateRightColumnX     = 310.0;

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
    NSAppearance.currentAppearance = self.effectiveAppearance;
    
    CALayer *layer = [self layer];

    // Setup drag indication layer
    _dragIndicationLayer = [[CALayer alloc] init];
    _dragIndicationLayer.borderColor = [[NSColor controlAccentColor] CGColor];
    _dragIndicationLayer.borderWidth = 2.0;
    _dragIndicationLayer.cornerRadius = 8.0;
    _dragIndicationLayer.hidden = YES;
    
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
    if(_representedObject == representedObject) {
        return;
    }
    
    _representedObject = representedObject;

    NSRect bounds = self.bounds;
    NSSize containerSize  = NSMakeSize(OEBlankSlateContainerWidth, OEBlankSlateContainerHeight);
    
    NSRect containerFrame = NSMakeRect(ceil((NSWidth(bounds) - containerSize.width) / 2.0),
                                       ceil((NSHeight(bounds) - containerSize.height) / 2.0),
                                       containerSize.width,
                                       containerSize.height);
    
    NSView *container = [[NSView alloc] initWithFrame:containerFrame];
    container.autoresizingMask = NSViewMaxXMargin |
                                 NSViewMinXMargin |
                                 NSViewMaxYMargin |
                                 NSViewMinYMargin;
    
    self.containerView = container;

    [self setupViewForRepresentedObject];

    // Remove current blank slate subview
    [self.subviews.lastObject removeFromSuperview];
    
    [self addSubview:container];
}

- (void)setupViewForRepresentedObject
{
    id representedObject = self.representedObject;

    if([representedObject isKindOfClass:[OEDBSystem class]]) {
        
        self.representedSystemPlugin = ((OEDBSystem *)representedObject).plugin;
        
    } else if([representedObject conformsToProtocol:@protocol(OEGameCollectionViewItemProtocol)]) {
        
        self.representedCollectionName = ((id <OEGameCollectionViewItemProtocol>)representedObject).collectionViewName;
        
    } else if([representedObject isKindOfClass:[OEDBSavedGamesMedia class]] || [representedObject isKindOfClass:[OEDBScreenshotsMedia class]]) {
        
        self.representedMediaType = representedObject;
        
    } else {
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

    NSString *text = [NSString stringWithFormat:NSLocalizedString(@"Create a personal game selection. To add to a collection, select a game from your console library and drag it to ’%@’ in the sidebar.", @""), collectionName];
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
    NSView *container = self.containerView;
    NSRect containerFrame = container.frame;

    [self OE_setupDragAndDropBox];
    [self addLeftHeadlineWithText:plugin ? plugin.systemName : NSLocalizedString(@"System", @"")];
    
    NSRect rect = NSMakeRect(0.0,
                             0.0,
                             NSWidth(containerFrame) / 12.0 * 7.0,
                             OEBlankSlateBottomTextHeight);
    rect.origin.y = NSHeight(containerFrame) - NSHeight(rect) - OEBlankSlateBottomTextTop;
    
    NSTextView *textView = [[OEArrowCursorTextView alloc] initWithFrame:NSInsetRect(rect, -4, 0)];

    NSString *textFormat = nil;

    if(plugin.supportsDiscsWithDescriptorFile) {
        textFormat = NSLocalizedString(@"%@ games will appear here. Check out %@ on how to add disc-based games.", @"");
    } else {
        textFormat = NSLocalizedString(@"%@ games you add to OpenEmu will appear in this Console Library", @"");
    }

    NSString *text = [NSString stringWithFormat:textFormat, plugin.systemName, NSLocalizedString(@"this guide", @"this guide")];

    textView.drawsBackground = NO;
    textView.editable = NO;
    textView.selectionGranularity = NSSelectByCharacter;
    textView.delegate = self;
    textView.font = [NSFont systemFontOfSize:12];
    textView.textColor = NSColor.labelColor;
    textView.textContainerInset = NSMakeSize(0, 0);

    NSMutableParagraphStyle *paraStyle = [[NSMutableParagraphStyle alloc] init];
    paraStyle.lineSpacing = 2;
    
    textView.defaultParagraphStyle = paraStyle;
    textView.string = text;

    NSMutableDictionary *attributes = [[textView typingAttributes] mutableCopy];
    attributes[NSCursorAttributeName] = [NSCursor arrowCursor];
    
    textView.typingAttributes = attributes;
    textView.markedTextAttributes = attributes;
    textView.selectedTextAttributes = attributes;

    if(plugin.supportsDiscsWithDescriptorFile)
    {
        NSRange guideLinkRange = [textView.string rangeOfString:NSLocalizedString(@"this guide", @"this guide")];
        NSDictionary *guideLinkAttributes = @{ NSLinkAttributeName : [NSURL URLWithString:OECDBasedGamesUserGuideURLString] };
        [textView.textStorage addAttributes:guideLinkAttributes range:guideLinkRange];
    }
    
    NSMutableDictionary *linkAttributes = [attributes mutableCopy];
    linkAttributes[NSUnderlineStyleAttributeName] = @(NSUnderlineStyleSingle);
    linkAttributes[NSCursorAttributeName] = [NSCursor pointingHandCursor];
    textView.linkTextAttributes = linkAttributes;
    
    [container addSubview:textView];
    
    // Get core plugins that can handle the system
    NSPredicate *pluginFilter = [NSPredicate predicateWithBlock: ^BOOL(OECorePlugin *evaluatedPlugin, NSDictionary *bindings) {
        return [evaluatedPlugin.systemIdentifiers containsObject:plugin.systemIdentifier];
    }];
    
    NSArray *pluginsForSystem = [[OECorePlugin allPlugins] filteredArrayUsingPredicate:pluginFilter];
    NSInteger extraspace = MAX(0, (NSInteger)[pluginsForSystem count] - 2);

    NSRect coreIconRect = NSMakeRect(OEBlankSlateCoreX,
                                     NSHeight(containerFrame) - 40.0 - OEBlankSlateCoreToTop + 16.0 * extraspace,
                                     40.0,
                                     40.0);
    NSImageView *coreIconView = [[NSImageView alloc] initWithFrame:coreIconRect];
    coreIconView.image = [NSImage imageNamed:@"blank_slate_core_icon"];
    
    [container addSubview:coreIconView];
    
    [coreIconView unregisterDraggedTypes];
    
    NSRect labelRect = NSMakeRect(OEBlankSlateRightColumnX,
                                  NSHeight(containerFrame) - 16.0 - OEBlankSlateBottomTextTop + 16.0 * extraspace,
                                  NSWidth(containerFrame),
                                  16.0);
    NSTextField *coreSuppliedByLabel = [NSTextField labelWithString:NSLocalizedString(@"Core Provided By…", @"")];
    
    coreSuppliedByLabel.frame = labelRect;
    coreSuppliedByLabel.font = [NSFont systemFontOfSize:12];
    coreSuppliedByLabel.textColor = NSColor.labelColor;
    
    [container addSubview:coreSuppliedByLabel];
    
    [pluginsForSystem enumerateObjectsUsingBlock:^(OECorePlugin *core, NSUInteger idx, BOOL *stop) {
        
        NSString *projectURL = core.infoDictionary[@"OEProjectURL"];
        NSString *name       = core.displayName;

        // Create weblink button for current core
        NSRect frame = NSMakeRect(OEBlankSlateRightColumnX,
                                  NSHeight(containerFrame) - 2.0 * 16.0 - OEBlankSlateBottomTextTop - 16.0 * idx - 2.0 + 16.0 * extraspace + 4.0,
                                  NSWidth(containerFrame) - OEBlankSlateRightColumnX,
                                  20.0);
        
        OETextButton *gotoButton = [[OETextButton alloc] initWithFrame:frame];
        
        gotoButton.autoresizingMask = NSViewWidthSizable;
        gotoButton.alignment = NSTextAlignmentLeft;
        gotoButton.target = self;
        gotoButton.action = @selector(gotoProjectURL:);
        gotoButton.title = name;
        gotoButton.toolTip = [NSString stringWithFormat:NSLocalizedString(@"Takes you to the %@ project website", @"Weblink tooltip"), name];
        gotoButton.cell.representedObject = projectURL;
        gotoButton.font = [NSFont boldSystemFontOfSize:12];
        gotoButton.textColor = NSColor.labelColor;
        gotoButton.textColorHover = [NSColor.labelColor colorWithSystemEffect:NSColorSystemEffectRollover];
        gotoButton.showArrow = YES;
        
        [gotoButton sizeToFit];
        
        [container addSubview:gotoButton];
    }];
}

- (void)addLeftHeadlineWithText:(NSString*)text
{
    NSView *container = self.containerView;
    NSRect containerFrame = container.frame;
    
    NSRect headlineFrame = NSMakeRect(-1,
                                      NSHeight(containerFrame) - 30 - OEBlankSlateHeadlineToTop,
                                      NSWidth(containerFrame),
                                      OEBlankSlateHeadlineHeight);
    NSTextField *headlineField = [NSTextField labelWithString:text];
    headlineField.frame = headlineFrame;
    headlineField.font = [NSFont systemFontOfSize:20 weight:NSFontWeightMedium];
    headlineField.textColor = NSColor.secondaryLabelColor;
    
    [container addSubview:headlineField];
}


- (void)OE_setupDragAndDropBox
{
    NSImageView *imageView = [self OE_makeImageViewWithImageNamed:@"blank_slate_arrow"];
    [self setupBoxWithText:NSLocalizedString(@"Drag & Drop Games Here", @"Blank Slate DnD Here") andImageView:imageView];
}

- (void)setupBoxWithText:(NSString*)text andImageView:(NSView*)arrowImageView
{
    NSView *container = self.containerView;
    NSRect containerFrame = container.frame;

    NSRect boxImageRect = NSMakeRect(0.0,
                                     NSHeight(containerFrame) - OEBlankSlateBoxHeight,
                                     NSWidth(containerFrame),
                                     OEBlankSlateBoxHeight);
    
    NSImageView *boxImageView = [[NSImageView alloc] initWithFrame:boxImageRect];
    boxImageView.image = [NSImage imageNamed:@"blank_slate_box"];
    
    [container addSubview:boxImageView];
    
    [boxImageView unregisterDraggedTypes];

    CGFloat height = OEBlankSlateBoxHeight - OEBlankSlateBoxImageToBottom-OEBlankSlateBoxImageToTop;
    CGFloat width  = 300.0;
    arrowImageView.frame = NSMakeRect(round(NSWidth(containerFrame) - width) / 2.0,
                                      NSHeight(containerFrame) - height - OEBlankSlateBoxImageToTop,
                                      width,
                                      height);
    
    [container addSubview:arrowImageView];
    
    NSColor *color = [NSColor colorNamed:@"blank_slate_box_text"];
    
    NSRect dragAndDropHereRect = NSMakeRect(0.0,
                                            NSHeight(containerFrame) - 30.0 - OEBlankSlateBoxTextToTop,
                                            NSWidth(containerFrame),
                                            31.0);
    
    NSTextField *dragAndDropHereField = [NSTextField labelWithString:text];
    
    dragAndDropHereField.frame = dragAndDropHereRect;
    dragAndDropHereField.font = [NSFont systemFontOfSize:24];
    dragAndDropHereField.textColor = color;
    dragAndDropHereField.alignment = NSTextAlignmentCenter;
    
    [container addSubview:dragAndDropHereField];
}

- (void)setRepresentedMediaType:(id <OESidebarItem>)item
{
    if(_representedCollectionName == item.sidebarName)
        return;

    _representedSystemPlugin = nil;
    _representedCollectionName = item.sidebarName;

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

    NSString *text = NSLocalizedString(@"Create your personal collection of screenshots. To take a screenshot, you can use the keyboard shortcut ⌘ + T while playing a game.", @"");
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
    const NSRect contentLayoutRect = self.window.contentLayoutRect;
    
    for (CALayer *layer in theLayer.sublayers) {
        
        if (layer == _dragIndicationLayer) {
            
            NSRect dragIndicationFrame = bounds;
            dragIndicationFrame.size.height = NSHeight(contentLayoutRect);
            
            layer.frame = NSInsetRect(dragIndicationFrame, 1.0, 1.0);
            
        } else if (layer != self.subviews.lastObject.layer) {
            layer.frame = bounds;
        }
    }
    
    [CATransaction commit];
}

- (NSImageView*)OE_makeImageViewWithImageNamed:(NSString*)imageName {
    
    NSImageView *arrowImageView = [[NSImageView alloc] initWithFrame:NSZeroRect];
    arrowImageView.image = [NSImage imageNamed:imageName];
    arrowImageView.imageScaling = NSImageScaleNone;
    arrowImageView.imageAlignment = NSImageAlignTop;
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
    NSView *container = self.containerView;
    NSRect containerFrame = container.frame;

    NSRect rect = NSMakeRect(0.0,
                             0.0,
                             NSWidth(containerFrame),
                             OEBlankSlateBottomTextHeight);
    rect.origin.y = NSHeight(containerFrame) - NSHeight(rect) - OEBlankSlateBottomTextTop;

    NSTextView *textView = [[NSTextView alloc] initWithFrame:NSInsetRect(rect, -4.0, 0.0)];
    textView.string = text;
    textView.drawsBackground = NO;
    textView.editable = NO;
    textView.selectable = NO;
    textView.font = [NSFont systemFontOfSize:12];
    textView.textColor = NSColor.labelColor;
    textView.textContainerInset = NSMakeSize(0, 0);

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
    
    _dragIndicationLayer.hidden = _lastDragOperation == NSDragOperationNone;
    
    return _lastDragOperation;
}

- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender
{
    if([_delegate respondsToSelector:@selector(blankSlateView:draggingUpdated:)])
    {
        _lastDragOperation = [_delegate blankSlateView:self draggingUpdated:sender];
    }
    
    _dragIndicationLayer.hidden = _lastDragOperation == NSDragOperationNone;
    
    return _lastDragOperation;
}

- (void)draggingExited:(id <NSDraggingInfo>)sender
{
    _dragIndicationLayer.hidden = YES;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
    _dragIndicationLayer.hidden = YES;
    return [_delegate respondsToSelector:@selector(blankSlateView:acceptDrop:)] && [_delegate blankSlateView:self acceptDrop:sender];
}

#pragma mark - Properties

@synthesize delegate = _delegate;
@synthesize lastDragOperation = _lastDragOperation;
@synthesize representedCollectionName = _representedCollectionName;
@synthesize representedSystemPlugin = _representedSystemPlugin;
@synthesize dragIndicationLayer = _dragIndicationLayer;

@end
