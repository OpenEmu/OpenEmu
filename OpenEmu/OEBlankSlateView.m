//
//  OEBlankSlateView.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 21.11.12.
//
//

#import "OEBlankSlateView.h"

#import "OESystemPlugin.h"
#import "OECorePlugin.h"
#import "OECenteredTextFieldCell.h"
#import "OEImageButton.h"

#import "OEGridNoisePattern.h"
#import "OECoverGridForegroundLayer.h"

@interface OEBlankSlateView ()
@property NSDragOperation lastDragOperation;
- (void)OE_commonInit;
- (void)OE_showView:(NSView*)view;
- (void)OE_setupView:(NSView*)view withCollectionName:(NSString *)representedCollectionName;
- (void)OE_setupView:(NSView*)view withSystemPlugin:(OESystemPlugin *)systemPlugin;

- (void)OE_addLeftHeadlineWithText:(NSString*)text toView:(NSView*)view;
- (void)OE_setupBoxInView:(NSView*)view;
@end

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
    self = [super initWithFrame:NSZeroRect];
    if (self) {
        [self OE_commonInit];
    }
    return self;
}

- (void)OE_commonInit
{
    OEBackgroundNoisePatternCreate();
    
    [self setWantsLayer:YES];
    CALayer *layer = [CALayer layer];
    [self setLayer:layer];
    
    // Disable implicit animations
    [layer setActions:@{ @"onOrderIn" : [NSNull null],
                        @"onOrderOut" : [NSNull null],
                         @"sublayers" : [NSNull null],
                          @"contents" : [NSNull null],
                            @"bounds" : [NSNull null]
     }];
    
    // Set background lighting
    [layer setContentsGravity:kCAGravityResize];
    [layer setContents:[NSImage imageNamed:@"background_lighting"]];
    [layer setFrame:[self bounds]];
    
    // Setup noise
    CALayer *noiseLayer = [CALayer layer];
    [noiseLayer setFrame:[self bounds]];
    [noiseLayer setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
    [noiseLayer setGeometryFlipped:YES];
    [noiseLayer setBackgroundColor:OEBackgroundNoiseColorRef];
    [layer addSublayer:noiseLayer];
    
    // Setup foreground
    OECoverGridForegroundLayer *foregroundLayer = [[OECoverGridForegroundLayer alloc] init];
    [foregroundLayer setFrame:[self bounds]];
    [foregroundLayer setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
    [layer addSublayer:foregroundLayer];
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
    [coreSuppliedByLabel setStringValue:NSLocalizedString(@"Core Provided By...", @"")];
    [view addSubview:coreSuppliedByLabel];
    
    dictionary = [[NSDictionary alloc] initWithObjectsAndKeys:
                  [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0], NSFontAttributeName,
                  shadow, NSShadowAttributeName,
                  [NSColor colorWithDeviceWhite:0.86 alpha:1.0], NSForegroundColorAttributeName,
                  nil];
    [cell setTextAttributes:dictionary];
    
    NSFont  *font;
    NSColor *textColor;
    
    font      = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0];
    textColor = [NSColor colorWithDeviceWhite:0.80 alpha:1.0];
    shadow    = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor blackColor]];
    [shadow setShadowOffset:NSMakeSize(0, -1)];
    
    NSDictionary *normalDictionary = [[NSDictionary alloc] initWithObjectsAndKeys:
                                      font, NSFontAttributeName,
                                      shadow, NSShadowAttributeName,
                                      textColor, NSForegroundColorAttributeName,
                                      nil];
    
    font      = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0];
    textColor = [NSColor colorWithDeviceWhite:1.0 alpha:1.0];
    shadow    = [[NSShadow alloc] init];
    
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:1.0 alpha:0.4]];
    [shadow setShadowOffset:NSMakeSize(0, 0)];
    [shadow setShadowBlurRadius:5];
    
    NSDictionary *clickDictionary = [[NSDictionary alloc] initWithObjectsAndKeys:
                                     font, NSFontAttributeName,
                                     shadow, NSShadowAttributeName,
                                     textColor, NSForegroundColorAttributeName,
                                     nil];
    
    font      = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0];
    textColor = [NSColor colorWithDeviceWhite:1.0 alpha:1.0];
    shadow    = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor blackColor]];
    [shadow setShadowOffset:NSMakeSize(0, -1)];
    
    NSDictionary *hoverDictionary = [[NSDictionary alloc] initWithObjectsAndKeys:
                                     font, NSFontAttributeName,
                                     shadow, NSShadowAttributeName,
                                     textColor, NSForegroundColorAttributeName,
                                     nil];
    
    int idx = 0;
    
    // FIXME: I don't think that's the proper place to have that kind of code (spaghetti code)
    NSArray *allPlugins = [OECorePlugin allPlugins];
    for(OECorePlugin *obj in allPlugins)
    {
        if(![[obj systemIdentifiers] containsObject:[plugin systemIdentifier]]) continue;
        
        NSString *projectURL = [[obj infoDictionary] valueForKey:@"OEProjectURL"];
        NSString *name       = [obj displayName];
        
        float y = bottomTextViewHeight - 2 * 17 - 17 * idx + 1;
        
        OEImageButton                 *imageButton = [[OEImageButton alloc] initWithFrame:(NSRect){{ rightColumnX - 3, y}, { [self frame].size.width - rightColumnX + 10, 21 }}];
        OEImageButtonHoverPressedText *cell        = [[OEImageButtonHoverPressedText alloc] initTextCell:name];
        
        [cell setNormalAttributes:normalDictionary];
        [cell setHoverAttributes:hoverDictionary];
        [cell setClickAttributes:clickDictionary];
        [cell setSplitVertically:YES];
        [cell setImage:[NSImage imageNamed:@"open_weblink_arrow"]];
        [imageButton setTarget:self];
        [imageButton setAction:@selector(gotoProjectURL:)];
        [imageButton setObjectValue:projectURL];
        [cell setText:name];
        [imageButton setCell:cell];
        [view addSubview:imageButton];
        
        idx++;
    }
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
    
    const NSRect  bounds      = [self bounds];
    const NSSize  viewSize    = [view frame].size;
    const NSRect  viewFrame   = NSMakeRect(ceil((NSWidth(bounds) - viewSize.width) / 2.0), ceil((NSHeight(bounds) - viewSize.height) / 2.0), viewSize.width, viewSize.height);
    [view setFrame:viewFrame];
    [view setAutoresizingMask:NSViewMaxXMargin|NSViewMinXMargin|NSViewMinYMargin|NSViewMaxYMargin];
    [self addSubview:view];
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
    
    return _lastDragOperation;
}

- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender
{
    if([_delegate respondsToSelector:@selector(blankSlateView:draggingUpdated:)])
    {
        _lastDragOperation = [_delegate blankSlateView:self draggingUpdated:sender];
    }
    return _lastDragOperation;
}

- (void)draggingExited:(id<NSDraggingInfo>)sender
{
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    return [_delegate respondsToSelector:@selector(blankSlateView:acceptDrop:)] && [_delegate blankSlateView:self acceptDrop:sender];
}
#pragma mark - Properties
@synthesize delegate=_delegate;
@synthesize lastDragOperation=_lastDragOperation;
@synthesize representedCollectionName=_representedCollectionName;
@synthesize representedSystemPlugin=_representedSystemPlugin;
@end
