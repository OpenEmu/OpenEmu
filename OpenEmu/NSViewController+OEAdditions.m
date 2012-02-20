//
//  NSViewController+OEAdditions.m
//  OpenEmu
//
//  Created by Remy Demarest on 07/02/2012.
//  Copyright (c) 2012 NuLayer Inc. All rights reserved.
//

#import "NSViewController+OEAdditions.h"
#import <objc/runtime.h>

static IMP _OEReplaceMethodInClass(Class cls, SEL sel, IMP replace)
{
    Method m = class_getInstanceMethod(cls, sel);
    IMP ret = method_getImplementation(m);
    
    class_replaceMethod(cls, sel, (IMP)replace, method_getTypeEncoding(m));
    
    return ret;
}

@implementation NSViewController (OEAdditions)

static NSView *(*_old_NSViewController_view)(NSViewController *self, SEL _cmd);
static NSView *OE_NSViewController_view(NSViewController *self, SEL _cmd);

static void (*_old_NSViewController_setView_)(NSViewController *self, SEL _cmd, NSView *value);
static void OE_NSViewController_setView_(NSViewController *self, SEL _cmd, NSView *value);

static void (*_old_NSViewController_observeValueForKeyPath_ofObject_change_context_)(NSViewController *self, SEL _cmd, NSString *keyPath, id object, NSDictionary *change, void *context);
static void _NSViewController_observeValueForKeyPath_ofObject_change_context_(NSViewController *self, SEL _cmd, NSString *keyPath, id object, NSDictionary *change, void *context);

static NSView *_OENSViewControllerView(NSViewController *self)
{
    return object_getIvar(self, class_getInstanceVariable([NSView class], "view"));
}

+ (void)load
{
    _old_NSViewController_view = (NSView *(*)(NSViewController *, SEL))_OEReplaceMethodInClass(self, @selector(view), (IMP)OE_NSViewController_view);
    
    _old_NSViewController_setView_ = (void(*)(NSViewController *, SEL, NSView *))_OEReplaceMethodInClass(self, @selector(setView:), (IMP)OE_NSViewController_setView_);
    
    _old_NSViewController_observeValueForKeyPath_ofObject_change_context_ = (void (*)(NSViewController *, SEL, NSString *, id, NSDictionary *, void *))_OEReplaceMethodInClass(self, @selector(observeValueForKeyPath:ofObject:change:context:), (IMP)_NSViewController_observeValueForKeyPath_ofObject_change_context_);
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

static const void *const _OE_NSViewControllerIsChangingNextResponderKey = &_OE_NSViewControllerIsChangingNextResponderKey;
static void *const _OE_NSViewControllerViewContext = (void *)&_OE_NSViewControllerViewContext;

static void _NSViewController_observeValueForKeyPath_ofObject_change_context_(NSViewController *self, SEL _cmd, NSString *keyPath, id object, NSDictionary *change, void *context)
{
    if(context == _OE_NSViewControllerViewContext)
    {
        if([objc_getAssociatedObject(self, _OE_NSViewControllerIsChangingNextResponderKey) boolValue]) return;
        
        objc_setAssociatedObject(self, _OE_NSViewControllerIsChangingNextResponderKey, (id)kCFBooleanTrue, OBJC_ASSOCIATION_COPY);
        
        [self setNextResponder:[object nextResponder]];
        [object setNextResponder:self];
        
        objc_setAssociatedObject(self, _OE_NSViewControllerIsChangingNextResponderKey, nil, OBJC_ASSOCIATION_COPY);
    }
    else if(_old_NSViewController_observeValueForKeyPath_ofObject_change_context_ != NULL)
        _old_NSViewController_observeValueForKeyPath_ofObject_change_context_(self, _cmd, keyPath, object, change, context);
}

static void OE_NSViewController_setView_(NSViewController *self, SEL _cmd, NSView *value)
{
    [_OENSViewControllerView(self) removeObserver:self forKeyPath:@"nextResponder" context:_OE_NSViewControllerViewContext];
    
    _old_NSViewController_setView_(self, _cmd, value);
    
    if(value != nil) [value addObserver:self forKeyPath:@"nextResponder" options:NSKeyValueObservingOptionInitial context:_OE_NSViewControllerViewContext];
}

@end
