//
//  SidebarFieldEditor.m
//  SidebarViewTest
//
//  Created by Christoph Leimbrock on 16.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OESidebarFieldEditor.h"
#import "OEBackgroundColorView.h"

@implementation OESidebarFieldEditor
@synthesize container;

+ (id)fieldEditor
{
    static OESidebarFieldEditor *fieldEditor = nil;
    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        fieldEditor = [[OESidebarFieldEditor alloc] initWithFrame:NSZeroRect];
        
        [fieldEditor setFieldEditor:YES];
        [fieldEditor setEditable:YES];
        [fieldEditor setSelectable:YES];
        
        [fieldEditor setTextContainerInset:NSMakeSize(1, 2)];
    });
    
    return fieldEditor;
}

- (NSRect)superFrame
{
    return [[self superview] frame];
}

- (void)viewWillMoveToSuperview:(NSView *)newSuperview
{
    if(newSuperview == nil && [self superview] != nil)
    {
        [[self superview] removeObserver:self forKeyPath:@"frame"];
    }
    else if(newSuperview != nil)
    {
        [newSuperview addObserver:self forKeyPath:@"frame" options:0xF context:nil];
    }
}
- (void)updateContainerFrame
{
    if([self superview] != nil && [self container] != nil)
    {
        NSRect rect = NSInsetRect([[self superview] frame], -1, -1);
        
        rect.size.width -= 1;
        
        [[self container] setFrame:rect];
    }
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    [self updateContainerFrame];
}

- (void)viewDidMoveToWindow
{
    [super viewDidMoveToWindow];
    
    if([self window]    != nil &&
       [self superview] != nil &&
       [NSStringFromClass([[self superview] class]) isEqualToString:@"_NSKeyboardFocusClipView"])
    {
        OEBackgroundColorView *cont = [[OEBackgroundColorView alloc] initWithFrame:NSInsetRect([self superFrame], -1, -1)];
        cont.backgroundColor = [NSColor colorWithDeviceRed:0.09 green:0.153 blue:0.553 alpha:1.0];
        [self setContainer:cont];
        
        [self updateContainerFrame];
        
        [[[self superview] superview] addSubview:[self container] positioned:NSWindowBelow relativeTo:[self superview]];
    }
    else if([self container] != nil)
    {
        [[self container] removeFromSuperview];
        [self setContainer:nil];
    }
}

- (void)setFrameSize:(NSSize)size
{
    size.width = ceil(size.width);
    
    [super setFrameSize:size];
}

- (void)drawRect:(NSRect)dirtyRect
{
    [self setBackgroundColor:[NSColor whiteColor]];
    
    [super drawRect:dirtyRect];
}

- (void)dealloc
{
    [[self container] removeFromSuperview];
    [self setContainer:nil];    
}

@end