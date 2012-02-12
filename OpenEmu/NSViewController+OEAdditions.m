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
static NSView *OE_NSViewController_view(NSViewController *self, SEL _cmd);

static void (*_old_NSViewController_setView_)(NSViewController *self, SEL _cmd, NSView *value);
static void OE_NSViewController_setView_(NSViewController *self, SEL _cmd, NSView *value);

static NSView *_OENSViewControllerView(NSViewController *self)
{
    return object_getIvar(self, class_getInstanceVariable([NSView class], "view"));
}

+ (void)load
{
    Method m = class_getInstanceMethod(self, @selector(view));
    
    _old_NSViewController_view = (NSView *(*)(NSViewController *, SEL))method_getImplementation(m);
    class_replaceMethod(self, @selector(view), (IMP)OE_NSViewController_view, method_getTypeEncoding(m));
    
    m = class_getInstanceMethod(self, @selector(setView:));
    _old_NSViewController_setView_ = (void(*)(NSViewController *, SEL, NSView *))method_getImplementation(m);
    class_replaceMethod(self, @selector(setView:), (IMP)OE_NSViewController_setView_, method_getTypeEncoding(m));
}

- (void)viewWillLoad;
{
}

- (void)viewDidLoad;
{
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

static NSView *OE_NSViewController_view(NSViewController *self, SEL _cmd)
{
    NSView *ret = _OENSViewControllerView(self);
    
    BOOL willLoad = ret == nil;
    
    if(willLoad) [self viewWillLoad];
    
    ret = _old_NSViewController_view(self, _cmd);
    
    if(willLoad) [self viewDidLoad];
    
    return ret;
}

static void OE_NSViewController_setView_(NSViewController *self, SEL _cmd, NSView *value)
{
    if([_OENSViewControllerView(self) OE_viewDelegate] == self)
        [_OENSViewControllerView(self) OE_setViewDelegate:nil];
    
    _old_NSViewController_setView_(self, _cmd, value);
    
    [value OE_setViewDelegate:self];
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
