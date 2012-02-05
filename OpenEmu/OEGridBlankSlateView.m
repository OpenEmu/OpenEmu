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

#import "OEGridBlankSlateView.h"
#import "OESystemPlugin.h"
#import "OECorePlugin.h"
#import "OECenteredTextFieldCell.h"
#import "OEImageButton.h"

@implementation OEGridBlankSlateView
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
- (void)_addLeftHeadlineWithText:(NSString*)text
{
    OECenteredTextFieldCell *cell = [[OECenteredTextFieldCell alloc] initTextCell:@""];
    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:1.0]];
    [shadow setShadowOffset:(NSSize){0, -1}];
    [shadow setShadowBlurRadius:0];
    
    NSDictionary *dictionary = [[NSDictionary alloc] initWithObjectsAndKeys:
                                [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:15 size:11.0], NSFontAttributeName,
                                shadow, NSShadowAttributeName,
                                [NSColor colorWithDeviceWhite:1.0 alpha:1.0], NSForegroundColorAttributeName,
                                nil];
    [cell setTextAttributes:dictionary];
    [shadow release];
    [dictionary release];
    
    NSTextField *headlineField = [[NSTextField alloc] initWithFrame:(NSRect){{-2, [self frame].size.height-21-bottomHeadlineTopToViewTop},{[self frame].size.width, 21}}];
    [headlineField setCell:cell];
    [headlineField setStringValue:text];
    [headlineField setEditable:NO];
    [headlineField setSelectable:NO];
    [headlineField setDrawsBackground:NO];
    [headlineField setBezeled:NO];
    [cell release];
    
    [self addSubview:headlineField];
    [headlineField release];
    
}
- (void)_setupBox
{
    NSImageView *boxImageView = [[NSImageView alloc] initWithFrame:(NSRect){{0,[self frame].size.height-boxHeight},{[self frame].size.width,boxHeight}}];
    [boxImageView setImage:[NSImage imageNamed:@"blank_slate_box"]];
    [self addSubview:boxImageView];
    [boxImageView release];
    
    NSImageView *arrowImageView = [[NSImageView alloc] initWithFrame:(NSRect){{(roundf([self frame].size.width-100)/2), [self frame].size.height-124-arrowTopToViewTop},{100, 124}}];
    [arrowImageView setImage:[NSImage imageNamed:@"blank_slate_arrow"]];
    [self addSubview:arrowImageView];
    [arrowImageView release];
    
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
    [dictionary release];
    [shadow release];
    
    
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
    [shadow release];
    [glowCell setTextAttributes:dictionary];
    [style release];
    [dictionary release];
    
    NSTextField *dragAndDropHereOuterGlowField = [[NSTextField alloc] initWithFrame:(NSRect){{0, [self frame].size.height-25-dndTextTopToViewTop},{[self frame].size.width, 25}}];
    [dragAndDropHereOuterGlowField setCell:glowCell];
    [dragAndDropHereOuterGlowField setStringValue:NSLocalizedString(@"Drag & Drop Games Here", @"Blank Slate DnD Here")];
    [dragAndDropHereOuterGlowField setEditable:NO];
    [dragAndDropHereOuterGlowField setSelectable:NO];
    [dragAndDropHereOuterGlowField setDrawsBackground:NO];
    [dragAndDropHereOuterGlowField setBezeled:NO];
    [glowCell release];
    
    NSTextField *dragAndDropHereField = [[NSTextField alloc] initWithFrame:(NSRect){{0, [self frame].size.height-25-dndTextTopToViewTop},{[self frame].size.width, 25}}];
    [dragAndDropHereField setCell:defaultCell];
    [dragAndDropHereField setStringValue:NSLocalizedString(@"Drag & Drop Games Here", @"Blank Slate DnD Here")];
    [dragAndDropHereField setEditable:NO];
    [dragAndDropHereField setSelectable:NO];
    [dragAndDropHereField setDrawsBackground:NO];
    [dragAndDropHereField setBezeled:NO];
    [defaultCell release];
    [self addSubview:dragAndDropHereField];
    [self addSubview:dragAndDropHereOuterGlowField];
    [dragAndDropHereField release];
    [dragAndDropHereOuterGlowField release];
}

- (id)initWithCollectionName:(NSString*)collectionName 
{
    self = [super initWithFrame:(NSRect){{0,0},{ViewWidth,ViewHeight}}];
    if (self) 
    {
        [self _setupBox];
        [self _addLeftHeadlineWithText:NSLocalizedString(@"Collections", @"")];
        
        NSRect rect = (NSRect){{0,0},{[self frame].size.width, bottomTextViewHeight}};
        NSTextView *textView = [[NSTextView alloc] initWithFrame:NSInsetRect(rect,-4, 0)];
        NSString *text = [NSString stringWithFormat:NSLocalizedString(@"Create a personal game selection. To add to a collection, select a game from your console library and drag it to ’%@’ on the left.", @""), collectionName];
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
        [shadow setShadowOffset:(NSSize){0,-1}];
        [textView setShadow:shadow];
        [shadow release];
        
        [self addSubview:textView];
        [textView release];
    }
    return self;
}

- (id)initWithSystemPlugin:(OESystemPlugin*)plugin
{
    self = [super initWithFrame:(NSRect){{0,0},{ViewWidth,ViewHeight}}];
    if (self)
    {
        [self _setupBox];
        
        [self _addLeftHeadlineWithText:plugin?[plugin systemName]:NSLocalizedString(@"System", @"")];
        
        NSRect rect = (NSRect){{0,0},{[self frame].size.width/12*7, bottomTextViewHeight}};
        NSTextView *textView = [[NSTextView alloc] initWithFrame:NSInsetRect(rect, -4, 0)];
        NSString *text = [NSString stringWithFormat:NSLocalizedString(@"%@ games you add to OpenEmu will appear in this Console Library", @""), [plugin systemName]];
        [textView setDrawsBackground:NO];
        [textView setEditable:NO];
        [textView setSelectable:NO];
        [textView setFont:[[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0]];
        [textView setTextColor:[NSColor colorWithDeviceWhite:0.86 alpha:1.0]];
        [textView setTextContainerInset:NSMakeSize(0, 0)];

        NSMutableParagraphStyle *paraStyle = [[NSMutableParagraphStyle alloc] init];
        [paraStyle setLineSpacing:2];
        [textView setDefaultParagraphStyle:paraStyle];
        [paraStyle release];
        
        [textView setString:text];
        NSShadow *shadow = [[NSShadow alloc] init];
        [shadow setShadowColor:[NSColor blackColor]];
        [shadow setShadowBlurRadius:0];
        [shadow setShadowOffset:(NSSize){0,-1}];
        [textView setShadow:shadow];
        [shadow release];
        
        [self addSubview:textView];
        [textView release];
        
        NSImageView *coreIconView = [[NSImageView alloc] initWithFrame:(NSRect){{coreIconX, [self frame].size.height-40-coreIconTopToViewTop},{40, 40}}];
        [coreIconView setImage:[NSImage imageNamed:@"blank_slate_core_icon"]];
        [self addSubview:coreIconView];
        [coreIconView release];
        
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
        [dictionary release];
        
        NSTextField *coreSuppliedByLabel = [[NSTextField alloc] initWithFrame:(NSRect){{rightColumnX, bottomTextViewHeight-16},{[self frame].size.width-rightColumnX, 17}}];
        [coreSuppliedByLabel setCell:cell];
        [cell release];
        [coreSuppliedByLabel setEditable:NO];
        [coreSuppliedByLabel setSelectable:NO];
        [coreSuppliedByLabel setDrawsBackground:NO];
        [coreSuppliedByLabel setBezeled:NO];
        [coreSuppliedByLabel setStringValue:NSLocalizedString(@"Core Provided By...", @"")];
        [self addSubview:coreSuppliedByLabel];
        [coreSuppliedByLabel release];
        
        dictionary = [[NSDictionary alloc] initWithObjectsAndKeys:
                      [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0], NSFontAttributeName,
                      shadow, NSShadowAttributeName,
                      [NSColor colorWithDeviceWhite:0.86 alpha:1.0], NSForegroundColorAttributeName,
                      nil];
        [cell setTextAttributes:dictionary];
        [shadow release];
        [dictionary release];
        
        NSFont *font;
        NSColor *textColor;
        
        font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0];
        textColor = [NSColor colorWithDeviceWhite:0.80 alpha:1.0];
        shadow = [[NSShadow alloc] init];
        [shadow setShadowColor:[NSColor blackColor]];
        [shadow setShadowOffset:NSMakeSize(0, -1)];
        
        NSDictionary *normalDictionary = [[NSDictionary alloc] initWithObjectsAndKeys:
                                          font, NSFontAttributeName,
                                          shadow, NSShadowAttributeName,
                                          textColor, NSForegroundColorAttributeName,                                          
                                          nil];
        [shadow release];
        
        font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0];
        textColor = [NSColor colorWithDeviceWhite:1.0 alpha:1.0];
        shadow = [[NSShadow alloc] init];
        
        [shadow setShadowColor:[NSColor colorWithDeviceWhite:1.0 alpha:0.4]];
        [shadow setShadowOffset:NSMakeSize(0, 0)];
        [shadow setShadowBlurRadius:5];
        NSDictionary *clickDictionary = [[NSDictionary alloc] initWithObjectsAndKeys:
                                         font, NSFontAttributeName,
                                         shadow, NSShadowAttributeName,
                                         textColor, NSForegroundColorAttributeName,                                          
                                         nil];
        [shadow release];
        
        font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0];
        textColor = [NSColor colorWithDeviceWhite:1.0 alpha:1.0];
        shadow = [[NSShadow alloc] init];
        [shadow setShadowColor:[NSColor blackColor]];
        [shadow setShadowOffset:NSMakeSize(0, -1)];
        NSDictionary *hoverDictionary = [[NSDictionary alloc] initWithObjectsAndKeys:
                                         font, NSFontAttributeName,
                                         shadow, NSShadowAttributeName,
                                         textColor, NSForegroundColorAttributeName,                                          
                                         nil];
        [shadow release];
        int idx = 0;
        NSArray *allPlugins = [OECorePlugin allPlugins];
        for(OECorePlugin *obj in allPlugins)
        {
            if(![[obj systemIdentifiers] containsObject:[plugin systemIdentifier]])
            {
                continue;
            }
            
            NSString *projectURL = [[obj infoDictionary] valueForKey:@"OEProjectURL"];
            NSString *name = [obj displayName];
            
            float y = bottomTextViewHeight-2*17 -17*idx+1;
            OEImageButton *imageButton = [[OEImageButton alloc] initWithFrame:(NSRect){{rightColumnX-3, y},{[self frame].size.width-rightColumnX+10, 21}}];
            OEImageButtonHoverPressedText *cell = [[OEImageButtonHoverPressedText alloc] initTextCell:name];
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
            [self addSubview:imageButton];
            [imageButton release];
            [cell release];
            
            idx++;
        }
        [normalDictionary release];
        [clickDictionary release];
        [hoverDictionary release];        
    }
    return self;   
}

- (void)centerInSuperview
{
    [self setFrameOrigin:(NSPoint){round(([self superview].frame.size.width-[self frame].size.width)/2),
        round(([self superview].frame.size.height-[self frame].size.height)/2),
    }];
}

#pragma mark -
- (void)gotoProjectURL:(id)sender
{
    NSString *urlString = [sender title];
    NSURL *url = [NSURL URLWithString:urlString];
    [[NSWorkspace sharedWorkspace] openURL:url];
}
@end
