//
//  NSViewController+OEAdditions.m
//  OpenEmu
//
//  Created by Remy Demarest on 07/02/2012.
//  Copyright (c) 2012 NuLayer Inc. All rights reserved.
//

#import "NSViewController+OEAdditions.h"
#import <objc/runtime.h>

@interface NSView (OEAdditions)
@property(nonatomic, getter=OE_viewDelegate, setter=OE_setViewDelegate:, assign) NSViewController *_viewDelegate;
@end

@implementation NSViewController (OEAdditions)

static NSView *(*_old_NSViewController_view)(NSViewController *self, SEL _cmd);
static NSView *_NSViewController_view(NSViewController *self, SEL _cmd)
{
    NSView *ret = nil;
    object_getInstanceVariable(self, "view", (void **)&ret);
    
    BOOL willLoad = ret == nil;
    
    if(willLoad) [self viewWillLoad];
    
    ret = _old_NSViewController_view(self, _cmd);
    
    if(willLoad) [self viewDidLoad];
    
    return ret;
}

+ (void)load
{
    Method m = class_getInstanceMethod(self, @selector(view));
    
    _old_NSViewController_view = (NSView *(*)(NSViewController *, SEL))method_getImplementation(m);
    class_replaceMethod(self, @selector(view), (IMP)_NSViewController_view, method_getTypeEncoding(m));
}

- (void)viewWillLoad;
{
    
}

- (void)viewDidLoad;
{
    [[self view] OE_setViewDelegate:self];
}

- (void)viewWillAppear;
{
    
}

- (void)viewDidAppear;
{
    
}

- (void)viewWillDisappear;
{
    
}

- (void)viewDidDisappear;
{
    
}

@end

@implementation NSView (OEAdditions)

static const void *const _OE_NSView__viewDelegateKey = &_OE_NSView__viewDelegateKey;

- (NSViewController *)OE_viewDelegate
{
    return objc_getAssociatedObject(self, _OE_NSView__viewDelegateKey);
}

- (void)OE_setViewDelegate:(NSViewController *)value
{
    NSViewController *previous = objc_getAssociatedObject(self, _OE_NSView__viewDelegateKey);
    
    if(previous != value)
    {
        objc_setAssociatedObject(self, &_OE_NSView__viewDelegateKey, value, OBJC_ASSOCIATION_ASSIGN);
        
        if(value != nil)
        {
            [value setNextResponder:[self nextResponder]];
            [super setNextResponder:value];
        }
        else
            [super setNextResponder:[previous nextResponder]];
        
        [previous setNextResponder:nil];
    }
}

- (void)setNextResponder:(NSResponder *)aResponder
{
    NSViewController *del = [self OE_viewDelegate];
    
    if(aResponder == del) return;
    
    if(del != nil) [del   setNextResponder:aResponder];
    else           [super setNextResponder:aResponder];
}

@end
