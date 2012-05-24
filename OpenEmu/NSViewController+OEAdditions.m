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

static NSView *_OENSViewControllerView(NSViewController *self)
{
    return object_getIvar(self, class_getInstanceVariable([NSView class], "view"));
}

+ (void)load
{
    _old_NSViewController_view = (NSView *(*)(NSViewController *, SEL))_OEReplaceMethodInClass(self, @selector(view), (IMP)OE_NSViewController_view);
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
    [self setNextResponder:[[self view] nextResponder]];
    [[self view] setNextResponder:self];
}

- (void)viewWillDisappear;
{
    [[self view] setNextResponder:[self nextResponder]];
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

@end
