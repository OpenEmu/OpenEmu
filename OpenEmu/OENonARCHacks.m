//
//  OENonARCHacks.m
//  OpenEmu
//

#import "OESidebarOutlineView.h"
#import "OESidebarOutlineButtonCell.h"
#import <objc/runtime.h>
#import "NSViewController+OEAdditions.h"

@implementation OESidebarOutlineView (OEPrivate)

- (void)setupOutlineCell{
    OESidebarOutlineButtonCell *sidebarOutlineCell = [[OESidebarOutlineButtonCell alloc] init];
    [sidebarOutlineCell retain];
    
    void *currentCell;
    object_getInstanceVariable(self, "_outlineCell", &currentCell);
    [(id)currentCell release];
    object_getInstanceVariable(self, "_trackingOutlineCell", &currentCell);
    [(id)currentCell release];
    
    object_setInstanceVariable(self, "_outlineCell", sidebarOutlineCell);
    object_setInstanceVariable(self, "_trackingOutlineCell", sidebarOutlineCell);
}

@end

NSView *(*_old_NSViewController_view)(NSViewController *self, SEL _cmd);

NSView *OE_NSViewController_view(NSViewController *self, SEL _cmd);
NSView *OE_NSViewController_view(NSViewController *self, SEL _cmd)
{
    NSView *ret = nil;
    object_getInstanceVariable(self, "view", (void **)&ret);
    
    BOOL willLoad = ret == nil;
    
    if(willLoad) [self viewWillLoad];
    
    ret = _old_NSViewController_view(self, _cmd);
    
    if(willLoad) [self viewDidLoad];
    
    return ret;
}